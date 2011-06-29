from DataAware import *
from DataAwareField import DataAwareField
from common.Helper import *

'''
@author rpiterman
This class suppoprts imple cases where a UI control can
be directly synchronized with a data property.
Such controls are: the different text controls
(synchronizing the "Text" , "Value", "Date", "Time" property),
Checkbox controls, Dropdown listbox controls (synchronizing the
SelectedItems[] property.
For those controls, static convenience methods are offered, to simplify use.
'''

class UnoDataAware(DataAware):

    def __init__(self, dataObject, value, unoObject_, unoPropName_):
        super(UnoDataAware,self).__init__(dataObject, value)
        self.unoControl = unoObject_
        self.unoModel = self.unoControl.Model
        self.unoPropName = unoPropName_
        self.disableObjects = range(0)
        self.inverse = False

    def setInverse(self, i):
        self.inverse = i

    def enableControls(self, value):
        b = getBoolean(value)
        if self.inverse:
            if b.booleanValue():
                b = False
            else:
                b = True

        i = 0
        while i < self.disableObjects.length:
            setEnabled(self.disableObjects[i], b)
            i += 1

    def setToUI(self, value):
        Helper.setUnoPropertyValue(self.unoModel, self.unoPropName, value)

    '''
    Try to get from an arbitrary object a boolean value.
    Null returns False;
    A Boolean object returns itself.
    An Array returns true if it not empty.
    An Empty String returns False.
    everything else returns a True.
    @param value
    @return
    '''

    def getBoolean(self, value):
        if value == None:
            return False

        if isinstance(value, bool):
            return value
        elif value.getClass().isArray():
            if value.length != 0:
                return True
            else:
                return False
        elif value.equals(""):
            return False
        elif isinstance(value, int):
            if value == 0:
                return True
            else:
                return False
        else:
            return True

    def disableControls(self, controls):
        self.disableObjects = controls

    def getFromUI(self):
        return Helper.getUnoPropertyValue(self.unoModel, self.unoPropName)

    @classmethod
    def __attachTextControl(
        self, data, prop, unoText, unoProperty, field, value):
        if field:
            aux = DataAwareField(prop, value)
        else:
            aux = DataAware.PropertyValue (prop, data)
        uda = UnoDataAware(data, aux, unoText, unoProperty)
        method = getattr(uda,"updateData")
        unoText.addTextListener(TextListenerProcAdapter(method))
        return uda

    @classmethod
    def attachEditControl(self, data, prop, unoControl, field):
        return self.__attachTextControl(
            data, prop, unoControl, "Text", field, "")

    @classmethod
    def attachDateControl(self, data, prop, unoControl, field):
        return self.__attachTextControl(
            data, prop, unoControl, "Date", field, 0)

    def attachTimeControl(self, data, prop, unoControl, field):
        return self.__attachTextControl(
            data, prop, unoControl, "Time", field, 0)

    @classmethod
    def attachNumericControl(self, data, prop, unoControl, field):
        return self.__attachTextControl(
            data, prop, unoControl, "Value", field, float(0))

    @classmethod
    def attachCheckBox(self, data, prop, checkBox, field):
        if field:
            aux = DataAwareField(prop, 0)
        else:
            aux = DataAware.PropertyValue (prop, data)
        uda = UnoDataAware(data, aux , checkBox, PropertyNames.PROPERTY_STATE)
        method = getattr(uda,"updateData")
        checkBox.addItemListener(ItemListenerProcAdapter(method))
        return uda

    def attachLabel(self, data, prop, label, field):
        if field:
            aux = DataAwareField(prop, "")
        else:
            aux = DataAware.PropertyValue (prop, data)
        return UnoDataAware(data, aux, label, PropertyNames.PROPERTY_LABEL)

    @classmethod
    def attachListBox(self, data, prop, listBox, field):
        if field:
            aux = DataAwareField(prop, uno.Any("short",0))
        else:
            aux = DataAware.PropertyValue (prop, data)
        uda = UnoDataAware(data, aux, listBox, "SelectedItems")
        method = getattr(uda,"updateData")
        listBox.addItemListener(ItemListenerProcAdapter(method))
        return uda

    def setEnabled(self, control, enabled):
        Helper.setUnoPropertyValue(
            getModel(control), PropertyNames.PROPERTY_ENABLED, enabled)
