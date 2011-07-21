from DataAware import *
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
            for i in self.radioButtons:
                i.State = False
        else:
            self.radioButtons[selected].State = True

    def getFromUI(self):
        for index, workwith in enumerate(self.radioButtons):
            if workwith.State:
                return index

        return -1

    @classmethod
    def attachRadioButtons(self, data, prop, buttons, field):
        da = RadioDataAware(data, prop, buttons)
        method = getattr(da,"updateData")
        for i in da.radioButtons:
            i.addItemListener(ItemListenerProcAdapter(method))
        return da
