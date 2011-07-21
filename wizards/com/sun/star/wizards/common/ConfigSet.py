import traceback
from ConfigNode import *

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
            self.fireListDataListenerIntervalAdded(i, i)
        else:
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
            #COMMENTED
            #self.fireListDataListenerIntervalAdded(oldSize, i)

    def writeConfiguration(self, configView, param):
        names = self.childrenMap.keySet().toArray()
        if isinstance(self.childClass, ConfigNode):
            #first I remove all the children from the configuration.
            children = Configuration.getChildrenNames(configView)
            i = 0
            while i < children.length:
                try:
                    Configuration.removeNode(configView, children[i])
                except Exception, ex:
                    ex.printStackTrace()

                # and add them new.
                i += 1
            for i in names:
                try:
                    child = getElement(i)
                    childView = Configuration.addConfigNode(configView, i)
                    child.writeConfiguration(childView, param)
                except Exception, ex:
                    ex.printStackTrace()
        else:
            raise AttributeError (
            "Unable to write primitive sets to configuration (not implemented)")

    def readConfiguration(self, configurationView, param):
        names = configurationView.ElementNames
        if isinstance(self.childClass, ConfigNode):
            for i in names:
                try:
                    child = type(self.childClass)()
                    child.setRoot(self.root)
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

    def update(self, i):
        fireListDataListenerContentsChanged(i, i)

    def createDOM(self, parent):
        items = items()
        i = 0
        while i < items.length:
            item = items[i]
            if item.instanceof.XMLProvider:
                item.createDOM(parent)

            i += 1
        return parent

    def items(self):
        return self.childrenList.toArray()

    def getKey(self, object):
        i = self.childrenMap.entrySet().iterator()
        while i.hasNext():
            me = i.next()
            if me.getValue() == object:
                return me.getKey()

        return None

    def getKey(self, i):
        c = 0
        while i > -1:
            if getElementAt(c) != None:
                i -= 1

            c += 1
        if c == 0:
            return None
        else:
            return getKey(getElementAt(c - 1))

    def setRoot(self, newRoot):
        self.root = newRoot

    '''
    Notifies all registered listeners about the event.
    @param event The event to be fired
    '''

    def fireListDataListenerIntervalAdded(self, i0, i1):
        event = ListDataEvent(self, ListDataEvent.INTERVAL_ADDED, i0, i1)
        if self.listenerList == None:
            return

        listeners = self.listenerList.getListenerList()
        i = listeners.length - 2
        while i >= 0:
            if listeners[i] == javax.swing.event.ListDataListener:
                (listeners[i + 1]).intervalAdded(event)

            i -= 2

    def getElementAt(self, i):
        return self.childrenList[i]

    def getElement(self, o):
        return self.childrenMap[o]

    def getSize(self):
        return len(self.childrenList)

    def keys(self):
        return self.childrenMap.keySet()

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
        Collections.sort(self.childrenList, comparator)
