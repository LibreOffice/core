from UnoDialog import *
from ui.event.CommonListener import *
from common.Desktop import Desktop
from UIConsts import *

'''
This class contains convenience methods for inserting components to a dialog.
It was created for use with the automatic conversion of Basic XML Dialog
description files to a Java class which builds the same dialog through the UNO API.<br/>
It uses an Event-Listener method, which calls a method through reflection
wenn an event on a component is trigered.
see the classes AbstractListener, CommonListener, MethodInvocation for details.
'''

class UnoDialog2(UnoDialog):

    '''
    Override this method to return another listener.
    @return
    '''

    def __init__(self, xmsf):
        super(UnoDialog2,self).__init__(xmsf,(), ())

    def insertButton(self, sName, actionPerformed, sPropNames, oPropValues, eventTarget=None):
        xButton = self.insertControlModel2("com.sun.star.awt.UnoControlButtonModel", sName, sPropNames, oPropValues)
        if actionPerformed is not None:
            xButton.addActionListener(ActionListenerProcAdapter(actionPerformed))

        return xButton

    def insertImageButton(self, sName, actionPerformed, sPropNames, oPropValues):
        xButton = self.insertControlModel2("com.sun.star.awt.UnoControlButtonModel", sName, sPropNames, oPropValues)
        if actionPerformed != None:
            xButton.addActionListener(ActionListenerProcAdapter(actionPerformed))

        return xButton

    def insertCheckBox(self, sName, itemChanged, sPropNames, oPropValues, eventTarget=None):
        xCheckBox = self.insertControlModel2("com.sun.star.awt.UnoControlCheckBoxModel", sName, sPropNames, oPropValues)
        if itemChanged != None:
            if eventTarget is None:
                eventTarget = self
            xCheckBox.addItemListener(ItemListenerProcAdapter(None))

        return xCheckBox

    def insertComboBox(self, sName, actionPerformed, itemChanged, textChanged, sPropNames, oPropValues, eventTarget=None):
        xComboBox = self.insertControlModel2("com.sun.star.awt.UnoControlComboBoxModel", sName, sPropNames, oPropValues)
        if eventTarget is None:
            eventTarget = self
        if actionPerformed != None:
            xComboBox.addActionListener(None)

        if itemChanged != None:
            xComboBox.addItemListener(ItemListenerProcAdapter(None))

        if textChanged != None:
            xComboBox.addTextListener(TextListenerProcAdapter(None))

        return xComboBox

    def insertListBox(self, sName, actionPerformed, itemChanged, sPropNames, oPropValues, eventTarget=None):
        xListBox = self.insertControlModel2("com.sun.star.awt.UnoControlListBoxModel",
            sName, sPropNames, oPropValues)

        if eventTarget is None:
            eventTarget = self

        if actionPerformed != None:
            xListBox.addActionListener(None)

        if itemChanged != None:
            xListBox.addItemListener(ItemListenerProcAdapter(None))

        return xListBox

    def insertRadioButton(self, sName, itemChanged, sPropNames, oPropValues, eventTarget=None):
        xRadioButton = self.insertControlModel2("com.sun.star.awt.UnoControlRadioButtonModel",
            sName, sPropNames, oPropValues)
        if itemChanged != None:
            if eventTarget is None:
                eventTarget = self
            xRadioButton.addItemListener(ItemListenerProcAdapter(None))


        return xRadioButton

    def insertTitledBox(self, sName, sPropNames, oPropValues):
        oTitledBox = self.insertControlModel2("com.sun.star.awt.UnoControlGroupBoxModel", sName, sPropNames, oPropValues)
        return oTitledBox

    def insertTextField(self, sName, sTextChanged, sPropNames, oPropValues, eventTarget=None):
        return self.insertEditField(sName, sTextChanged, eventTarget,
            "com.sun.star.awt.UnoControlEditModel", sPropNames, oPropValues)

    def insertImage(self, sName, sPropNames, oPropValues):
        return self.insertControlModel2("com.sun.star.awt.UnoControlImageControlModel", sName, sPropNames, oPropValues)

    def insertInfoImage(self, _posx, _posy, _iStep):
        xImgControl = self.insertImage(Desktop.getUniqueName(self.xDialogModel, "imgHint"),("Border", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH),(uno.Any("short",0), 10, UIConsts.INFOIMAGEURL, _posx, _posy, False, _iStep, 10))
        self.getPeerConfiguration().setImageUrl(self.getModel(xImgControl), UIConsts.INFOIMAGEURL, UIConsts.INFOIMAGEURL_HC)
        return xImgControl

    '''
    This method is used for creating Edit, Currency, Date, Formatted, Pattern, File
    and Time edit components.
    '''

    def insertEditField(self, sName, sTextChanged, eventTarget, sModelClass, sPropNames, oPropValues):
        xField = self.insertControlModel2(sModelClass, sName, sPropNames, oPropValues)
        if sTextChanged != None:
            if eventTarget is None:
                eventTarget = self
            xField.addTextListener(TextListenerProcAdapter(None))

        return xField

    def insertFileControl(self, sName, sTextChanged, sPropNames, oPropValues, eventTarget=None):
        return self.insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlFileControlModel", sPropNames, oPropValues)

    def insertCurrencyField(self, sName, sTextChanged, sPropNames, oPropValues, eventTarget=None):
        return self.insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlCurrencyFieldModel", sPropNames, oPropValues)

    def insertDateField(self, sName, sTextChanged, sPropNames, oPropValues, eventTarget=None):
        return self.insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlDateFieldModel", sPropNames, oPropValues)

    def insertNumericField(self, sName, sTextChanged, sPropNames, oPropValues, eventTarget=None):
        return self.insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlNumericFieldModel", sPropNames, oPropValues)

    def insertTimeField(self, sName, sTextChanged, sPropNames, oPropValues, eventTarget=None):
        return self.insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlTimeFieldModel", sPropNames, oPropValues)

    def insertPatternField(self, sName, sTextChanged, oPropValues, eventTarget=None):
        return self.insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlPatternFieldModel", sPropNames, oPropValues)

    def insertFormattedField(self, sName, sTextChanged, sPropNames, oPropValues, eventTarget=None):
        return self.insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlFormattedFieldModel", sPropNames, oPropValues)

    def insertFixedLine(self, sName, sPropNames, oPropValues):
        oLine = self.insertControlModel2("com.sun.star.awt.UnoControlFixedLineModel", sName, sPropNames, oPropValues)
        return oLine

    def insertScrollBar(self, sName, sPropNames, oPropValues):
        oScrollBar = self.insertControlModel2("com.sun.star.awt.UnoControlScrollBarModel", sName, sPropNames, oPropValues)
        return oScrollBar

    def insertProgressBar(self, sName, sPropNames, oPropValues):
        oProgressBar = self.insertControlModel2("com.sun.star.awt.UnoControlProgressBarModel", sName, sPropNames, oPropValues)
        return oProgressBar

    def insertGroupBox(self, sName, sPropNames, oPropValues):
        oGroupBox = self.insertControlModel2("com.sun.star.awt.UnoControlGroupBoxModel", sName, sPropNames, oPropValues)
        return oGroupBox

    def insertControlModel2(self, serviceName, componentName, sPropNames, oPropValues):
        try:
            xControlModel = self.insertControlModel(serviceName, componentName, (), ())
            Helper.setUnoPropertyValues(xControlModel, sPropNames, oPropValues)
            Helper.setUnoPropertyValue(xControlModel, PropertyNames.PROPERTY_NAME, componentName)
        except Exception, ex:
            traceback.print_exc()

        aObj = self.xUnoDialog.getControl(componentName)
        return aObj

    def setControlPropertiesDebug(self, model, names, values):
        i = 0
        while i < names.length:
            print "   Settings: ", names[i]
            Helper.setUnoPropertyValue(model, names[i], values[i])
            i += 1

    def translateURL(self, relativeURL):
        return ""

    def getControlModel(self, unoControl):
        obj = unoControl.getModel()
        return obj

    def showMessageBox(self, windowServiceName, windowAttribute, MessageText):
        return SystemDialog.showMessageBox(xMSF, self.xControl.getPeer(), windowServiceName, windowAttribute, MessageText)

