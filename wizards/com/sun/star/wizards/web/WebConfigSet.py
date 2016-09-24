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
from ..ui.event.EventListenerList import EventListenerList
from ..ui.event.ListDataEvent import ListDataEvent
from ..ui.event.ListDataListener import ListDataListener

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
        self.childClass = childType
        self.childrenMap = {}
        self.childrenList = []
        self.noNulls = True
        self.listenerList = None

    def add(self, name, o):
        oldO = None
        if (name in self.childrenMap):
            oldO = self.childrenMap[name]
        self.childrenMap[name] = o
        try:
            i = int(name)
            self.childrenList.insert(i, o)
            self.fireListDataListenerIntervalAdded(i, i);
        except ValueError:
            if (hasattr(o, "cp_Index")):
                i = o.cp_Index
                oldSize = self.getSize()
                while (self.getSize() <= i):
                    self.childrenList.append(None)
                self.childrenList[i] = o
                if oldSize > i:
                    oldSize = i
                self.fireListDataListenerIntervalAdded(oldSize, i);
            else:
                if (oldO is not None):
                    i = self.childrenList.index(oldO)
                    self.childrenList[i] = o
                else:
                    self.childrenList.append(o)
                    self.fireListDataListenerIntervalAdded(self.getSize() - 1, self.getSize() - 1);


    def writeConfiguration(self, configView, param):
        names = self.childrenMap.keys()
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
                childView = Configuration.addConfigNode(configView, i)
                child.writeConfiguration(childView, param)
            except Exception:
                traceback.print_exc()

    def readConfiguration(self, configurationView, param):
        names = configurationView.ElementNames
        if names:
            for i in names:
                try:
                    child = self.childClass()
                    child.setRoot(self.root)
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

    def remove1(self, obj):
        key = self.getKey(obj)
        del self.childrenMap[key]
        i = self.childrenList.index(obj)
        self.childrenList.remove(obj)
        self.fireListDataListenerIntervalRemoved(i, i)

    def remove(self, i):
        o = self.getElementAt(i)
        self.remove1(o)

    def clear(self):
        self.childrenMap.clear()
        del self.childrenList[:]

    def update(self, i):
        self.fireListDataListenerContentsChanged(i, i)

    def createDOM(self, parent):
        items = self.childrenList
        i = 0
        while i < len(items):
            item = items[i]
            if hasattr(item, "createDOM"):
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
        try:
            return self.childrenMap[o]
        except KeyError:
            return None

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


    # Registers ListDataListener to receive events.
    # @param listener The listener to register.
    def addListDataListener(self, listener):
        if (self.listenerList is None):
            self.listenerList = EventListenerList()
        self.listenerList.add(listener)

    # Removes ListDataListener from the list of listeners.
    #  @param listener The listener to remove.
    def removeListDataListener(self, listener):
        self.listenerList.remove(listener)

    # Notifies all registered listeners about the event.
    #
    # @param event The event to be fired
    def fireListDataListenerIntervalAdded(self, i0, i1):
        event = ListDataEvent(self, ListDataEvent.INTERVAL_ADDED, i0, i1)
        if (self.listenerList is None):
            return
        for listener in self.listenerList.getListenerList():
            if isinstance(listener, ListDataListener):
                listener.intervalAdded(event)

    # Notifies all registered listeners about the event.
    #
    # @param event The event to be fired
    def fireListDataListenerIntervalRemoved(self, i0, i1):
        event = ListDataEvent(self, ListDataEvent.INTERVAL_REMOVED, i0, i1)
        if (self.listenerList is None):
            return
        for listener in self.listenerList.getListenerList():
            if isinstance(listener, ListDataListener):
                listener.intervalRemoved(event)

    # Notifies all registered listeners about the event.
    #
    # @param event The event to be fired
    def fireListDataListenerContentsChanged(self, i0, i1):
        event = ListDataEvent(self, ListDataEvent.CONTENTS_CHANGED, i0, i1)
        if (self.listenerList is None):
            return
        for listener in self.listenerList.getListenerList():
            if isinstance(listener, ListDataListener):
                listener.contentsChanged(event)
