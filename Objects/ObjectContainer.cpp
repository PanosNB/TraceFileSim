/*
 * ObjectContainer.cpp
 *
 *  Created on: 2013-09-03
 *      Author: kons
 */

#include "ObjectContainer.h"
#include <stdio.h>
#include <stdlib.h>
#include "../defines.h"

extern int gLineInTrace;

namespace traceFileSimulator {

ObjectContainer::ObjectContainer() {
	int i;
	rootset.resize(NUM_THREADS);
	for (i = 0; i < NUM_THREADS; i++) {
		rootset.at(i).resize(ROOTSET_SIZE);
	}

	objectList.resize(1);
	remSet.resize(1);
	rootCount = 0;
	remCount = 0;
}

void ObjectContainer::addToGenRoot(Object* object) {
	int remSetSlot = getRemSetSlot();
	if (remSetSlot == -1) {
		fprintf(stderr, "ERROR(Line %d): no remSet slot found\n", gLineInTrace);
		exit(1);
	}
	remSet[remSetSlot] = object;
	remCount++;
}



int ObjectContainer::removeFromGenRoot(Object* object){
	int i;
	for(i = 0 ; (unsigned int) i<remSet.size() ; i++){
		if(remSet.at(i) == object){
			remSet.at(i) = NULL;
			remCount--;
			return 0;
		}
	}
	return -1;
}

int ObjectContainer::add(Object* newObject) {
	int listSlot = getListSlot();
	if (listSlot == -1) {
		fprintf(stderr, "ERROR(line %d): no slot found\n", gLineInTrace);
		return -1;
	}

	objectList[listSlot] = newObject;
	return 0;
}

int ObjectContainer::removeFromRoot(int thread, int root ){
	rootset[thread][root] = NULL;
	rootCount--;
	return -1;
}

int ObjectContainer::addToRoot(Object* newObject, int thread, int rootSlot) {
	int listSlot = getListSlot();
	if (listSlot == -1) {
		fprintf(stderr, "ERROR(line %d): no slot found\n", gLineInTrace);
		return -1;
	}
	objectList[listSlot] = newObject;
	rootset[thread][rootSlot] = newObject;
	rootCount++;

	return 0;
}

Object* ObjectContainer::getByID(int id) {
	unsigned int i;
	for (i = 0; i < objectList.size(); i++) {
		if (objectList[i]) {
			int currentId = objectList[i]->getID();
			if (id == currentId) {
				return objectList[i];
			}
		}
	}
	fprintf(stderr, "ERROR(Line %d): object with this id (%d) was not found\n",
			gLineInTrace, id);
	return NULL;
}

Object* ObjectContainer::getbySlotNr(int slot) {
	return objectList.at(slot);
}

Object* ObjectContainer::getRoot(int thread, int rootSlot) {
	return rootset[thread][rootSlot];
}

int ObjectContainer::deleteObject(Object* object) {
	if (!object)
		fprintf(stderr, "ERROR(Line %d): trying to delete a non existing object\n", gLineInTrace);
	int id = object->getID();
	return deleteObject(id);
}

int ObjectContainer::deleteObject(int objectID) {
	unsigned int i;
	Object* temp;
	for (i = 0; i < objectList.size(); i++) {
		temp = objectList[i];
		if (temp && temp->getID() == objectID) {
			objectList[i] = NULL;
			delete (temp);
			return 0;
		}
	}
	fprintf(stderr, "ERROR(Line %d): object to delete not found. id: %d\n", gLineInTrace, objectID);
	return -1;
}

int ObjectContainer::getSize() {
	return objectList.size();
}

int ObjectContainer::removeReferenceTo(Object* object) {
	int i;
	for(i = 0 ; (unsigned int)i < objectList.size() ;i++){
		if(object == objectList.at(i)){
			objectList.at(i) = NULL;
			return 0;
		}
	}
	return -1;
}

int ObjectContainer::getGenRootCount() {
	return remCount;
}
int ObjectContainer::getGenRootSize() {
	return remSet.size();
}
int ObjectContainer::getRootSize() {
	return rootCount;
}

Object* ObjectContainer::getGenRoot(int slot) {
	if(slot < 0 && (unsigned int)slot > remSet.size()){
		fprintf(stderr,"ERROR(Line%d):Request illegal gen root. slot:%d\n",
				gLineInTrace,slot);
		exit(1);
	}
	return remSet.at(slot);
}

int ObjectContainer::getRemSetSlot() {
	unsigned int i;
	//fprintf(stderr, "(%d) size: %d\n", gLineInTrace,remSet.size());
	for (i = 0; i < remSet.size(); i++) {
		if(gLineInTrace==77156)
			fprintf(stderr, "(%d) size: %ld, i: %u\n", gLineInTrace,remSet.size(),i);
		if (!remSet.at(i)) {
			return i;
		}

		if (i + 1 == remSet.size()) {
			int old = remSet.size();
			remSet.resize(remSet.size() * 2);
			fprintf(stderr,"(%d)old:%d new:%d\n",gLineInTrace,old,(int)remSet.size());
			/*if(remSet.size()>i+1 && !remSet.at(i+1)){
				return i+1;
			}*/
		}
	}
	fprintf(stderr, "remSetSlotERror: no list slot found\n");
	return -1;
}

int ObjectContainer::getListSlot() {
	unsigned int i;
	for (i = 0; i < objectList.size(); i++) {
		if (!objectList.at(i)) {
			return i;
		}
		if (i + 1 == objectList.size()) {
			objectList.resize(objectList.size() * 2);
		}
	}
	fprintf(stderr, "listSlotERror: no list slot found\n");
	return -1;
}

void ObjectContainer::clearRemSet(){
	remSet.clear();
	remSet.resize(1);
	remCount = 0;
}
//void ObjectContainer::visualizeState(char* filename){
//	FILE* file = fopen(filename, "w");
//	fprintf(file, "digraph G { \n");
//	int i;
//	int j;
//	unsigned int k;
//	//create threads and init pointers
//	for(i = 0 ; i < NUM_THREADS ; i++){
//		fprintf(file, "T%d;\n", i);
//		for(j = 0 ; j < ROOTSET_SIZE ; j++){
//			if(rootset.at(i).at(j)){
//				fprintf(file, "T%d -> %d;\n", i, rootset.at(i).at(j)->getID());
//			}
//		}
//	}
//	for(k = 0 ; k < objectList.size() ; k++){
//		parent = objectList.at(k);
//		if(objectList.at(k)){
//			for(i = 0; i < objectList.at(k)->getPointersMax() ;i++){
//				Object* child = objectList.at(k)->getReferenceTo(i);
//				if(child){
//					fprintf(file, "%d -> %d;\n",objectList.at(k)->getID(), child->getID());
//				}
//			}
//		}
//	}
//	fprintf(file, "}\n");
//	fclose(file);
//}

int ObjectContainer::countElements() {
	int result = 0;
	unsigned int i;
	for (i = 0; i < objectList.size(); i++) {
		if (objectList[i]) {
			result++;
		}
	}
	return result;
}

ObjectContainer::~ObjectContainer() {
}

} /* namespace gcKons */
