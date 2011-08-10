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

    def __init__(self, dataObject_, field_):
        self._dataObject = dataObject_
        self._field = field_

    def enableControls(self, value):
        pass

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
        data = getattr(self._dataObject, self._field)
        ui = self.getFromUI()
        if data is not ui:
            try:
                self.setToUI(data)
            except Exception, ex:
                traceback.print_exc()
                #TODO tell user...
        self.enableControls(data)

    '''
    sets a new data object. Optionally
    update the UI.
    @param obj the new data object.
    @param updateUI if true updateUI() will be called.
    '''

    def setDataObject(self, obj, updateUI):
        if obj is not None and not isinstance(obj, type(self._field)):
            return

        self._dataObject = obj

        if updateUI:
            self.updateUI()

    '''
    updates the DataObject according to
    the current state of the UI control.
    '''

    def updateData(self):
        try:
            data = getattr(self._dataObject, self._field)
            ui = self.getFromUI()
            if data is not ui:
                if isinstance(ui,tuple):
                    #Selected Element listbox
                    ui = ui[0]
                setattr(self._dataObject, self._field, ui)
            self.enableControls(ui)
        except Exception:
            traceback.print_exc()
    '''
    given a collection containing DataAware objects,
    calls updateUI() on each memebr of the collection.
    @param dataAwares a collection containing DataAware objects.
    '''

    @classmethod
    def updateUIs(self, dataAwares):
        for i in dataAwares:
            i.updateUI()
