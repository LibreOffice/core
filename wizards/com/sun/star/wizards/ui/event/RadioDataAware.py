from DataAware import *
from DataAwareFields import *
from UnoDataAware import *
import time
'''
@author rpiterman
To change the template for this generated type comment go to
Window>Preferences>Java>Code Generation>Code and Comments
'''

class RadioDataAware(DataAware):

    def __init__(self, data, value, radioButtons):
        super(RadioDataAware,self).__init__(data, value)
        self.radioButtons = radioButtons

    def setToUI(self, value):
        selected = int(value)
        if selected == -1:
            i = 0
            while i < self.radioButtons.length:
                self.radioButtons[i].State = False
                i += 1
        else:
            self.radioButtons[selected].State = True

    def getFromUI(self):
        for i in self.radioButtons:
            if i.State:
                return i

        return -1

    @classmethod
    def attachRadioButtons(self, data, dataProp, buttons, field):
        if field:
            aux = DataAwareFields.getFieldValueFor(data, dataProp, 0)
        else:
            aux = DataAware.PropertyValue (dataProp, data)

        da = RadioDataAware(data, aux , buttons)
        return da
