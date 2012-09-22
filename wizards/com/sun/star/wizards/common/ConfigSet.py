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
from .ConfigNode import *
from .Configuration import Configuration

class ConfigSet(ConfigNode):
    '''
    After reading the configuration set items,
    the ConfigSet checks this field.
    If it is true, it will remove any nulls from
    the vector.
    subclasses can change this field in the constructor
    to avoid this "deletion" of nulls.
    '''

    def __init__(self, childType):
        self.childClass = childType
        self.childrenMap = {}
        self.childrenList = []
        self.noNulls = False

    def add(self, name, o):
        self.childrenMap[name] = o
        if isinstance(name, int):
            i = name
            self.childrenList.insert(i, o)
        else:
            try:
                i = o.cp_Index
                oldSize = self.getSize()
                if oldSize <= i:
                    newSize = i - oldSize
                    self.childrenList += [None] * newSize
                    self.noNulls = True
                else:
                    self.noNulls = False
                self.childrenList.insert(i, o);
                if oldSize > i:
                    oldSize = i
            except Exception:
                self.childrenList.append(o)

    def writeConfiguration(self, configView, param):
        names = self.childrenMap.keys()
        if isinstance(self.childClass, ConfigNode):
            #first I remove all the children from the configuration.
            children = configView.ElementNames
            if children:
                for i in children:
                    try:
                        Configuration.removeNode(configView, i)
                    except Exception:
                        traceback.print_exc()

                # and add them new.
            for i in names:
                try:
                    child = self.getElement(i)
                    childView = configView.getByName(i)
                    child.writeConfiguration(childView, param)
                except Exception:
                    traceback.print_exc()
        else:
            raise AttributeError (
            "Unable to write primitive sets to configuration (not implemented)")

    def readConfiguration(self, configurationView, param):
        names = configurationView.ElementNames
        if isinstance(self.childClass, ConfigNode):
            if names:
                for i in names:
                    try:
                        child = type(self.childClass)()
                        child.root = self.root
                        child.readConfiguration(
                            configurationView.getByName(i), param)
                        self.add(i, child)
                    except Exception, ex:
                         traceback.print_exc()
            #remove any nulls from the list
            if self.noNulls:
                i = 0
                while i < len(self.childrenList):
                    if self.childrenList[i] is None:
                        del self.childrenList[i]
                        i -= 1
                    i += 1

        else:
            for i in names:
                try:
                    child = configurationView.getByName(i)
                    self.add(i, child)
                except Exception, ex:
                    traceback.print_exc()

    def remove(self, obj):
        key = getKey(obj)
        self.childrenMap.remove(key)
        i = self.childrenList.indexOf(obj)
        self.childrenList.remove(obj)
        fireListDataListenerIntervalRemoved(i, i)

    def remove(self, i):
        o = getElementAt(i)
        remove(o)

    def clear(self):
        self.childrenMap.clear()
        del self.childrenList[:]

    def createDOM(self, parent):
        items = items()
        i = 0
        while i < items.length:
            item = items[i]
            if item.instanceof.XMLProvider:
                item.createDOM(parent)

            i += 1
        return parent

    def getKey(self, _object):
        for k,v in self.childrenMap.items():
            if v == _object:
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
        v = Vector.Vector_unknown(names.length)
        member = None
        index = 0
        i = 0
        while i < names.length:
            if not names[i].equals(memberName):
                member = Configuration.getConfigurationNode(names[i], confView)
                index = Configuration.getInt(indexPropertyName, member)
                while index >= v.size():
                    v.add(None)
                v.setElementAt(member, index)
            '''
            Now I reindex them
            '''

            i += 1
        index = 1
        i = 0
        while i < v.size():
            member = v.get(i)
            if member != None:
                Configuration.set((index + 1), indexPropertyName, member)

            i += 1

    def sort(self, comparator):
        self.childrenList.sort(comparator)
