from common.PropertyNames import *
from abc import ABCMeta, abstractmethod
import traceback
from ui.event.CommonListener import *

'''
@author rpiterman
DataAware objects are used to live-synchronize UI and DataModel/DataObject.
It is used as listener on UI events, to keep the DataObject up to date.
This class, as a base abstract class, sets a frame of functionality,
delegating the data Object get/set methods to a Value object,
and leaving the UI get/set methods abstract.
Note that event listenning is *not* a part of this model.
the updateData() or updateUI() methods should be porogramatically called.
in child classes, the updateData() will be binded to UI event calls.
<br><br>
This class holds references to a Data Object and a Value object.
The Value object "knows" how to get and set a value from the
Data Object.
'''

class DataAware(object):
    __metaclass__ = ABCMeta

    '''
    creates a DataAware object for the given data object and Value object.
    @param dataObject_
    @param value_
    '''

    def __init__(self, dataObject_, value_):
        self._dataObject = dataObject_
        self._value = value_

    '''
    sets the given value to the UI control
    @param newValue the value to set to the ui control.
    '''
    @abstractmethod
    def setToUI (self,newValue):
        pass

    '''
    gets the current value from the UI control.
    @return the current value from the UI control.
    '''

    @abstractmethod
    def getFromUI (self):
        pass

    '''
    updates the UI control according to the
    current state of the data object.
    '''

    def updateUI(self):
        data = self._value.get(self._dataObject)
        ui = self.getFromUI()
        if data is not ui:
            try:
                self.setToUI(data)
            except Exception, ex:
                traceback.print_exc()
                #TODO tell user...

    '''
    updates the DataObject according to
    the current state of the UI control.
    '''

    def updateData(self):
        try:
            data = self._value.get(self._dataObject)
            ui = self.getFromUI()
            if data is not ui:
                self._value.Set(ui, self._dataObject)
        except Exception:
            traceback.print_exc()

    '''
    compares the two given objects.
    This method is null safe and returns true also if both are null...
    If both are arrays, treats them as array of short and compares them.
    @param a first object to compare
    @param b second object to compare.
    @return true if both are null or both are equal.
    '''

    def equals(self, a, b):
        if not a and not b :
            return True

        if not a or not b:
            return False

        if a.getClass().isArray():
            if b.getClass().isArray():
                return Arrays.equals(a, b)
            else:
                return False

        return a.equals(b)

    '''
    given a collection containing DataAware objects,
    calls updateUI() on each memebr of the collection.
    @param dataAwares a collection containing DataAware objects.
    '''
    @classmethod
    def updateUIs(self, dataAwares):
        for i in dataAwares:
            i.updateUI()

    '''
    Given a collection containing DataAware objects,
    sets the given DataObject to each DataAware object
    in the given collection
    @param dataAwares a collection of DataAware objects.
    @param dataObject new data object to set to the DataAware
    objects in the given collection.
    @param updateUI if true, calls updateUI() on each DataAware object.
    '''

    def setDataObject(self, dataObject, updateUI):
        if dataObject is not None:
            if not (type(self._value) is not
                type(dataObject)):
                raise ClassCastException (
                    "can not cast new DataObject to original Class")
        self._dataObject = dataObject
        if updateUI:
            self.updateUI()

    '''
    Given a collection containing DataAware objects,
    sets the given DataObject to each DataAware object
    in the given collection
    @param dataAwares a collection of DataAware objects.
    @param dataObject new data object to set to the DataAware objects
    in the given collection.
    @param updateUI if true, calls updateUI() on each DataAware object.
    '''

    @classmethod
    def setDataObjects(self, dataAwares, dataObject, updateUI):
        for i in dataAwares:
            i.setDataObject(dataObject, updateUI)
