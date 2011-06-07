from DataAware import *
import unohelper
from com.sun.star.awt import XItemListener
from com.sun.star.awt import XTextListener
from DataAwareFields import *
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
        self.unoModel = self.getModel(self.unoControl)
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

    def stringof(self, value):
        if value.getClass().isArray():
            sb = StringBuffer.StringBuffer_unknown("[")
            i = 0
            while i < (value).length:
                sb.append((value)[i]).append(" , ")
                i += 1
            sb.append("]")
            return sb.toString()
        else:
            return value.toString()

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
    def __attachTextControl(self, data, prop, unoText, listener, unoProperty, field, value):
        if field:
            aux = DataAwareFields.getFieldValueFor(data, prop, value)
        else:
            aux = DataAware.PropertyValue (prop, data)
        uda = UnoDataAware(data, aux, unoText, unoProperty)
        unoText.addTextListener(self.TextListener(uda,listener))
        return uda

    class TextListener(unohelper.Base, XTextListener):

        def __init__(self, uda,listener):
            self.uda = uda
            self.listener = listener

        def textChanged(te):
            self.uda.updateData()
            if self.listener:
                self.listener.eventPerformed(te)

        def disposing(self,eo):
            pass

    @classmethod
    def attachEditControl(self, data, prop, unoControl, listener, field):
        return self.__attachTextControl(data, prop, unoControl, listener, "Text", field, "")

    @classmethod
    def attachDateControl(self, data, prop, unoControl, listener, field):
        return self.__attachTextControl(data, prop, unoControl, listener, "Date", field, 0)

    def attachTimeControl(self, data, prop, unoControl, listener, field):
        return self.__attachTextControl(data, prop, unoControl, listener, "Time", field, 0)

    def attachNumericControl(self, data, prop, unoControl, listener, field):
        return self.__attachTextControl(data, prop, unoControl, listener, "Value", field, float(0))

    @classmethod
    def attachCheckBox(self, data, prop, checkBox, listener, field):
        if field:
            aux = DataAwareFields.getFieldValueFor(data, prop, uno.Any("short",0))
        else:
            aux = DataAware.PropertyValue (prop, data)
        uda = UnoDataAware(data, aux , checkBox, PropertyNames.PROPERTY_STATE)
        checkBox.addItemListener(self.ItemListener(uda, listener))
        return uda

    class ItemListener(unohelper.Base, XItemListener):

        def __init__(self, da, listener):
            self.da = da
            self.listener = listener

        def itemStateChanged(self, ie):
            self.da.updateData()
            if self.listener != None:
                self.listener.eventPerformed(ie)

        def disposing(self, eo):
            pass

    def attachLabel(self, data, prop, label, listener, field):
        if field:
            aux = DataAwareFields.getFieldValueFor(data, prop, "")
        else:
            aux = DataAware.PropertyValue (prop, data)
        return UnoDataAware(data, aux, label, PropertyNames.PROPERTY_LABEL)

    @classmethod
    def attachListBox(self, data, prop, listBox, listener, field):
        if field:
            aux = DataAwareFields.getFieldValueFor(data, prop, 0)
        else:
            aux = DataAware.PropertyValue (prop, data)
        uda = UnoDataAware(data, aux, listBox, "SelectedItems")
        listBox.addItemListener(self.ItemListener(uda,listener))
        return uda

    def getModel(self, control):
        return control.getModel()

    def setEnabled(self, control, enabled):
        setEnabled(control, enabled)

    def setEnabled(self, control, enabled):
        Helper.setUnoPropertyValue(getModel(control), PropertyNames.PROPERTY_ENABLED, enabled)
