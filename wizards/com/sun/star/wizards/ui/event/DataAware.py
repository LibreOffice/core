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
import uno
from abc import ABCMeta, abstractmethod

from com.sun.star.util import Date
from com.sun.star.util import Time
from datetime import datetime

'''
@author rpiterman
DataAware objects are used to live-synchronize UI and DataModel/DataObject.
It is used as listener on UI events, to keep the DataObject up to date.
This class, as a base abstract class, sets a frame of functionality,
delegating the data Object get/set methods to a Value object,
and leaving the UI get/set methods abstract.
Note that event listening is *not* a part of this model.
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
        try:
            data = getattr(self._dataObject, self._field)
        except Exception:
            data = uno.invoke(self._dataObject, "get" + self._field, ())
        ui = self.getFromUI()
        if data is not ui:
            try:
                self.setToUI(data)
            except Exception:
                traceback.print_exc()

    '''
    sets a new data object. Optionally
    update the UI.
    @param obj the new data object.
    @param updateUI if true updateUI() will be called.
    '''

    def setDataObject(self, obj, updateUI):
        #if obj is not None and not isinstance(obj, type(self._field)):
        #    return

        self._dataObject = obj

        if updateUI:
            self.updateUI()

    '''
    updates the DataObject according to
    the current state of the UI control.
    '''

    def updateData(self):
        useUno = False
        try:
            try:
                data = getattr(self._dataObject, self._field)
            except Exception:
                useUno = True
                data = uno.invoke(self._dataObject, "get" + self._field, ())
            ui = self.getFromUI()
            if data is not ui:
                if isinstance(ui,Date):
                    d = datetime(ui.Year, ui.Month, ui.Day)
                    ui = d.strftime('%d/%m/%y')
                elif isinstance(ui,Time):
                    t = datetime(1, 1, 1, ui.Hours, ui.Minutes)
                    ui = t.strftime('%H:%M')
                if useUno:
                    uno.invoke(self._dataObject, "set" + self._field, (ui,))
                else:
                    if isinstance(ui,tuple):
                        #Listbox Element
                        ui = ui[0]
                    setattr(self._dataObject, self._field, ui)
        except Exception:
            traceback.print_exc()

    '''
    given a collection containing DataAware objects,
    calls updateUI() on each member of the collection.
    @param dataAwares a collection containing DataAware objects.
    '''

    @classmethod
    def updateUIs(self, dataAwares):
        for i in dataAwares:
            i.updateUI()
