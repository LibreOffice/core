#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
import traceback
from ..common.ConfigGroup import ConfigGroup
from ..common.Configuration import Configuration
from ..common.XMLProvider import XMLProvider

class WebConfigSet(ConfigGroup):
    '''
    After reading the configuration set items,
    the ConfigSet checks this field.
    If it is true, it will remove any nulls from
    the vector.
    subclasses can change this field in the constructor
    to avoid this "deletion" of nulls.
    '''

    def __init__(self, childType):
        print ("DEBUG !!! childType: ", childType)
        self.childClass = childType
        self.childrenMap = {}
        self.childrenList = []
        self.noNulls = False

    def add(self, name, o):
        print ("DEBUG !!! WebConfigSet.add -- name: ", name)
        if (o is None):
            print ("DEBUG !!! WebConfigSet.add -- Received None object as argument.")
        oldO = None
        if (name in self.childrenMap):
            oldO = self.childrenMap[name]
        self.childrenMap[name] = o
        try:
            i = int(name)
            print ("DEBUG !!! WebConfigSet.add -- name IS an integer.")
            self.childrenList.insert(i, o)
        except Exception:
            print ("DEBUG !!! WebConfigSet.add -- name IS NOT an integer.")
            try:
                i = o.cp_Index
                print ("DEBUG !!! WebConfigSet.add -- index: ", i)
                oldSize = self.getSize()
                print ("DEBUG !!! WebConfigSet.add -- oldSize: ", oldSize)
                if oldSize < i:
                    newSize = i - oldSize
                    self.childrenList += [None] * newSize
                    self.noNulls |= True
                else:
                    self.noNulls |= False
                print ("DEBUG !!! WebConfigSet.add -- inserting object o: ", o)
                self.childrenList.insert(i, o)
                if oldSize > i:
                    oldSize = i
            except Exception:
                if (oldO is not None):
                    print ("DEBUG !!! WebConfigSet.add -- No cp_Index attribute, but element already present, so replace it.")
                    i = self.childrenList.index(oldO)
                    self.childrenList[i] = o
                else:
                    print ("DEBUG !!! WebConfigSet.add -- No cp_Index attribute, so just append it.")
                    self.childrenList.append(o)


    def writeConfiguration(self, configView, param):
        print ("DEBUG !!! writeConfiguration --")
        names = self.childrenMap.keys()
        #first I remove all the children from the configuration.
        children = configView.ElementNames
        print ("DEBUG !!! writeConfiguration -- children length: ", len(children))
        if children:
            print ("DEBUG !!! writeConfiguration -- removing childrens.")
            for i in children:
                try:
                    Configuration.removeNode(configView, i)
                except Exception:
                    traceback.print_exc()

        # and add them new.
        for i in names:
            try:
                child = self.getElement(i)
                childView = Configuration.addConfigNode(configView, i)
                child.writeConfiguration(childView, param)
            except Exception:
                traceback.print_exc()

    def readConfiguration(self, configurationView, param):
        names = configurationView.ElementNames
        if names:
            for i in names:
                print ("DEBUG !!! readConfiguration -- name: ", i)
                try:
                    child = self.childClass()
                    child.root = self.root
                    child.readConfiguration(
                        configurationView.getByName(i), param)
                    self.add(i, child)
                except Exception:
                    traceback.print_exc()
        #remove any nulls from the list
        if self.noNulls:
            i = 0
            while i < len(self.childrenList):
                if self.childrenList[i] is None:
                    del self.childrenList[i]
                    i -= 1
                i += 1

    def remove(self, obj):
        key = getKey(obj)
        self.childrenMap.remove(key)
        i = self.childrenList.indexOf(obj)
        self.childrenList.remove(obj)
        #fireListDataListenerIntervalRemoved(i, i)

    def remove(self, i):
        o = getElementAt(i)
        remove(o)

    def clear(self):
        self.childrenMap.clear()
        del self.childrenList[:]

    def createDOM(self, parent):
        items = self.childrenList
        i = 0
        while i < len(items):
            item = items[i]
            if isinstance(item, XMLProvider):
                item.createDOM(parent)

            i += 1
        return parent

    def getKey(self, _object):
        for k,v in self.childrenMap.items():
            if v is _object:
                return k
        return None

    def getElementAt(self, i):
        return self.childrenList[i]

    def getElement(self, o):
        return self.childrenMap[o]

    def getSize(self):
        return len(self.childrenList)

    def getIndexOf(self, item):
        return self.childrenList.index(item)

    '''
    Set members might include a property
    which orders them.
    This method reindexes the given member to be
    the index number 0
    Do not forget to call commit() after calling this method.
    @param confView
    @param memebrName
    '''

    def reindexSet(self, confView, memberName, indexPropertyName):
        '''
        First I read all memebrs of the set,
        except the one that should be number 0
        to a vector, ordered by there index property
        '''
        names = Configuration.getChildrenNames(confView)
        v = []
        member = None
        index = 0
        i = 0
        while i < len(names):
            if not names[i] == memberName:
                member = Configuration.getNode(names[i], confView)
                index = Configuration.getInt(indexPropertyName, member)
                while index >= v.size():
                    v.append(None)
                v[index] = member
            '''
            Now I reindex them
            '''
            i += 1
        index = 1
        i = 0
        while i < len(v):
            member = v[i]
            if member != None:
                Configuration.set((index + 1), indexPropertyName, member)
            i += 1

    def sort(self, comparator):
        self.childrenList.sort(comparator)
