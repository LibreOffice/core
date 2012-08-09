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
import uno
import traceback
import wizards.ui.UIConsts
from wizards.common.PropertyNames import PropertyNames
from wizards.common.Helper import Helper
from wizards.ui.PeerConfig import PeerConfig

from com.sun.star.awt import Rectangle
from com.sun.star.awt.PosSize import POS

class UnoDialog(object):

    createDict = False
    dictProperties = None

    def __init__(self, xMSF, PropertyNames, PropertyValues):
        try:
            self.xMSF = xMSF
            self.ControlList = {}
            self.xDialogModel = xMSF.createInstance(
                "com.sun.star.awt.UnoControlDialogModel")
            self.xDialogModel.setPropertyValues(PropertyNames, PropertyValues)
            self.xUnoDialog = xMSF.createInstance(
                "com.sun.star.awt.UnoControlDialog")
            self.xUnoDialog.setModel(self.xDialogModel)
            self.m_oPeerConfig = None
            self.xWindowPeer = None
        except Exception, e:
            traceback.print_exc()

    def getControlKey(self, EventObject, ControlList):
        xControlModel = EventObject.getModel()
        try:
            sName = xControlModel.getPropertyValue(
                PropertyNames.PROPERTY_NAME)
            iKey = ControlList.get(sName).intValue()
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()
            iKey = 2000

        return iKey

    def getPeerConfiguration(self):
        if self.m_oPeerConfig is None:
            self.m_oPeerConfig = PeerConfig(self)
        return self.m_oPeerConfig

    def setControlProperty(self, ControlName, PropertyName, PropertyValue):
        try:
            if PropertyValue is not None:
                if not self.xDialogModel.hasByName(ControlName):
                    return
                xPSet = self.xDialogModel.getByName(ControlName)
                setattr(xPSet,PropertyName, PropertyValue)

        except Exception, exception:
            traceback.print_exc()

    def setControlProperties(
            self, ControlName, PropertyNames, PropertyValues):
        self.setControlProperty(ControlName, PropertyNames, PropertyValues)

    def getControlProperty(self, ControlName, PropertyName):
        try:
            xPSet = self.xDialogModel.getByName(ControlName)
            return xPSet.getPropertyValue(PropertyName)
        except Exception, exception:
            traceback.print_exc()
            return None


    def printControlProperties(self, ControlName):
        try:
            xControlModel = self.xDialogModel().getByName(ControlName)
            allProps = xControlModel.PropertySetInfo.Properties
            i = 0
            while i < allProps.length:
                sName = allProps[i].Name
                i += 1
        except Exception, exception:
            traceback.print_exc()

    def getMAPConversionFactor(self, ControlName):
        xControl2 = self.xUnoDialog.getControl(ControlName)
        aSize = xControl2.Size
        dblMAPWidth = Helper.getUnoPropertyValue(xControl2.Model,
            int(PropertyNames.PROPERTY_WIDTH))
        return (aSize.Width / dblMAPWidth)

    def getpreferredLabelSize(self, LabelName, sLabel):
        xControl2 = self.xUnoDialog.getControl(LabelName)
        OldText = xControl2.Text
        xControl2.setText(sLabel)
        aSize = xControl2.PreferredSize
        xControl2.setText(OldText)
        return aSize

    def removeSelectedItems(self, xListBox):
        SelList = xListBox.SelectedItemsPos
        Sellen = SelList.length
        i = Sellen - 1
        while i >= 0:
            xListBox.removeItems(SelList[i], 1)
            i -= 1

    def getListBoxItemCount(self, _xListBox):
        # This function may look ugly, but this is the only way to check
        # the count of values in the model,which is always right.
        # the control is only a view and could be right or not.
        fieldnames = Helper.getUnoPropertyValue(getModel(_xListBox),
            "StringItemList")
        return fieldnames.length

    def getSelectedItemPos(self, _xListBox):
        ipos = Helper.getUnoPropertyValue(getModel(_xListBox),
            "SelectedItems")
        return ipos[0]

    def isListBoxSelected(self, _xListBox):
        ipos = Helper.getUnoPropertyValue(getModel(_xListBox),
            "SelectedItems")
        return ipos.length > 0

    def addSingleItemtoListbox(self, xListBox, ListItem, iSelIndex):
        xListBox.addItem(ListItem, xListBox.getItemCount())
        if iSelIndex != -1:
            xListBox.selectItemPos(iSelIndex, True)

    '''
    The problem with setting the visibility of controls is that
    changing the current step of a dialog will automatically make
    all controls visible. The PropertyNames.PROPERTY_STEP property
    always wins against the property "visible".
    Therfor a control meant to be invisible is placed on a step far far away.
    Afterwards the step property of the dialog has to be set with
    "repaintDialogStep". As the performance of that method is very bad it
    should be used only once for all controls
    @param controlname the name of the control
    @param bIsVisible sets the control visible or invisible
    '''

    def setControlVisible(self, controlname, bIsVisible):
        try:
            iCurControlStep = int(getControlProperty(
                controlname, PropertyNames.PROPERTY_STEP))
            iCurDialogStep = int(Helper.getUnoPropertyValue(
                self.xDialogModel, PropertyNames.PROPERTY_STEP))
            if bIsVisible:
                setControlProperty(
                    controlname, PropertyNames.PROPERTY_STEP, iCurDialogStep)
            else:
                setControlProperty(
                    controlname, PropertyNames.PROPERTY_STEP,
                    UIConsts.INVISIBLESTEP)

        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()

    # repaints the currentDialogStep

    def repaintDialogStep(self):
        try:
            ncurstep = int(Helper.getUnoPropertyValue(
                self.xDialogModel, PropertyNames.PROPERTY_STEP))
            Helper.setUnoPropertyValue(
                self.xDialogModel, PropertyNames.PROPERTY_STEP, 99)
            Helper.setUnoPropertyValue(
                self.xDialogModel, PropertyNames.PROPERTY_STEP, ncurstep)
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()

    def insertControlModel(
        self, serviceName, componentName, sPropNames, oPropValues):
        try:
            xControlModel = self.xDialogModel.createInstance(serviceName)
            Helper.setUnoPropertyValues(
                xControlModel, sPropNames, oPropValues)
            self.xDialogModel.insertByName(componentName, xControlModel)
            Helper.setUnoPropertyValue(xControlModel,
                PropertyNames.PROPERTY_NAME, componentName)
        except Exception, ex:
            traceback.print_exc()

        aObj = self.xUnoDialog.getControl(componentName)
        return aObj

    def setFocus(self, ControlName):
        oFocusControl = self.xUnoDialog.getControl(ControlName)
        oFocusControl.setFocus()

    def combineListboxList(self, sFirstEntry, MainList):
        try:
            FirstList = [sFirstEntry]
            ResultList = [MainList.length + 1]
            System.arraycopy(FirstList, 0, ResultList, 0, 1)
            System.arraycopy(MainList, 0, ResultList, 1, len(MainList))
            return ResultList
        except java.lang.Exception, jexception:
            traceback.print_exc()
            return None

    def selectListBoxItem(self, xListBox, iFieldsSelIndex):
        if iFieldsSelIndex > -1:
            FieldCount = xListBox.getItemCount()
            if FieldCount > 0:
                if iFieldsSelIndex < FieldCount:
                    xListBox.selectItemPos(iFieldsSelIndex, True)
                else:
                    xListBox.selectItemPos((short)(iFieldsSelIndex - 1), True)

    # deselects a Listbox. MultipleMode is not supported

    def deselectListBox(self, _xBasisListBox):
        oListBoxModel = getModel(_xBasisListBox)
        sList = Helper.getUnoPropertyValue(oListBoxModel, "StringItemList")
        Helper.setUnoPropertyValue(oListBoxModel, "StringItemList", [[],[]])
        Helper.setUnoPropertyValue(oListBoxModel, "StringItemList", sList)

    def calculateDialogPosition(self, FramePosSize):
        # Todo:check if it would be useful or possible to create a dialog peer
        # that can be used for the messageboxes to
        # maintain modality when they pop up.
        CurPosSize = self.xUnoDialog.getPosSize()
        WindowHeight = FramePosSize.Height
        WindowWidth = FramePosSize.Width
        DialogWidth = CurPosSize.Width
        DialogHeight = CurPosSize.Height
        iXPos = ((WindowWidth / 2) - (DialogWidth / 2))
        iYPos = ((WindowHeight / 2) - (DialogHeight / 2))
        self.xUnoDialog.setPosSize(
            iXPos, iYPos, DialogWidth, DialogHeight, POS)

    '''
     @param FramePosSize
    @return 0 for cancel, 1 for ok
    @throws com.sun.star.uno.Exception
    '''

    def executeDialog(self, FramePosSize):
        if self.xUnoDialog.getPeer() == None:
            raise AttributeError(
                "Please create a peer, using your own frame")

        self.calculateDialogPosition(FramePosSize)

        if self.xWindowPeer == None:
            self.createWindowPeer()
        return self.xUnoDialog.execute()

    def setVisible(self, parent):
        self.calculateDialogPosition(parent.xWindow.getPosSize())
        if self.xWindowPeer == None:
            self.createWindowPeer()

        self.xUnoDialog.setVisible(True)

    '''
    @param parent
    @return 0 for cancel, 1 for ok
    @throws com.sun.star.uno.Exception
    '''

    def executeDialogFromParent(self, parent):
        return self.executeDialog(parent.xUnoDialog.PosSize)

    '''
    @param XComponent
    @return 0 for cancel, 1 for ok
    @throws com.sun.star.uno.Exception
    '''

    def executeDialogFromComponent(self, xComponent):
        if xComponent is not None:
            w = xComponent.ComponentWindow
            if w is not None:
                return self.executeDialog(w.PosSize)

        return self.executeDialog( Rectangle (0, 0, 640, 400))

    def modifyFontWeight(self, ControlName, FontWeight):
        oFontDesc = FontDescriptor.FontDescriptor()
        oFontDesc.Weight = FontWeight
        setControlProperty(ControlName, "FontDescriptor", oFontDesc)

    '''
    create a peer for this
    dialog, using the given
    peer as a parent.
    @param parentPeer
    @return
    @throws java.lang.Exception
    '''

    def createWindowPeer(self, parentPeer=None):
        self.xUnoDialog.setVisible(False)
        xToolkit = self.xMSF.createInstance("com.sun.star.awt.Toolkit")
        if parentPeer == None:
            parentPeer = xToolkit.getDesktopWindow()

        self.xUnoDialog.createPeer(xToolkit, parentPeer)
        self.xWindowPeer = self.xUnoDialog.getPeer()
        return self.xUnoDialog.getPeer()

    # deletes the first entry when this is equal to "DelEntryName"
    # returns true when a new item is selected

    def deletefirstListboxEntry(self, ListBoxName, DelEntryName):
        xListBox = self.xUnoDialog.getControl(ListBoxName)
        FirstItem = xListBox.getItem(0)
        if FirstItem.equals(DelEntryName):
            SelPos = xListBox.getSelectedItemPos()
            xListBox.removeItems(0, 1)
            if SelPos > 0:
                setControlProperty(ListBoxName, "SelectedItems", [SelPos])
                xListBox.selectItemPos((short)(SelPos - 1), True)

    @classmethod
    def setEnabled(self, control, enabled):
        Helper.setUnoPropertyValue(
            control.Model, PropertyNames.PROPERTY_ENABLED, enabled)

    @classmethod
    def getControlModelType(self, xServiceInfo):
        if xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlFixedTextModel"):
            return UIConsts.CONTROLTYPE.FIXEDTEXT
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlButtonModel"):
            return UIConsts.CONTROLTYPE.BUTTON
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlCurrencyFieldModel"):
            return UIConsts.CONTROLTYPE.CURRENCYFIELD
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlDateFieldModel"):
            return UIConsts.CONTROLTYPE.DATEFIELD
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlFixedLineModel"):
            return UIConsts.CONTROLTYPE.FIXEDLINE
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlFormattedFieldModel"):
            return UIConsts.CONTROLTYPE.FORMATTEDFIELD
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlRoadmapModel"):
            return UIConsts.CONTROLTYPE.ROADMAP
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlNumericFieldModel"):
            return UIConsts.CONTROLTYPE.NUMERICFIELD
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlPatternFieldModel"):
            return UIConsts.CONTROLTYPE.PATTERNFIELD
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlHyperTextModel"):
            return UIConsts.CONTROLTYPE.HYPERTEXT
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlProgressBarModel"):
            return UIConsts.CONTROLTYPE.PROGRESSBAR
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlTimeFieldModel"):
            return UIConsts.CONTROLTYPE.TIMEFIELD
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlImageControlModel"):
            return UIConsts.CONTROLTYPE.IMAGECONTROL
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlRadioButtonModel"):
            return UIConsts.CONTROLTYPE.RADIOBUTTON
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlCheckBoxModel"):
            return UIConsts.CONTROLTYPE.CHECKBOX
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlEditModel"):
            return UIConsts.CONTROLTYPE.EDITCONTROL
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlComboBoxModel"):
            return UIConsts.CONTROLTYPE.COMBOBOX
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlListBoxModel"):
            return UIConsts.CONTROLTYPE.LISTBOX
        else:
            return UIConsts.CONTROLTYPE.UNKNOWN

    @classmethod
    def getDisplayProperty(self, oControlModel):
        itype = self.getControlModelType(oControlModel)
        if not UnoDialog.createDict:
            UnoDialog.createDict = True
            UnoDialog.dictProperties = {
                UIConsts.CONTROLTYPE.FIXEDTEXT:PropertyNames.PROPERTY_LABEL,
                UIConsts.CONTROLTYPE.BUTTON:PropertyNames.PROPERTY_LABEL,
                UIConsts.CONTROLTYPE.FIXEDLINE:PropertyNames.PROPERTY_LABEL,
                UIConsts.CONTROLTYPE.NUMERICFIELD:"Value",
                UIConsts.CONTROLTYPE.CURRENCYFIELD:"Value",
                UIConsts.CONTROLTYPE.FORMATTEDFIELD:"EffectiveValue",
                UIConsts.CONTROLTYPE.DATEFIELD:"Date",
                UIConsts.CONTROLTYPE.TIMEFIELD:"Time",
                UIConsts.CONTROLTYPE.SCROLLBAR:"ScrollValue",
                UIConsts.CONTROLTYPE.PROGRESSBAR:"ProgressValue",
                UIConsts.CONTROLTYPE.IMAGECONTROL:PropertyNames.PROPERTY_IMAGEURL,
                UIConsts.CONTROLTYPE.RADIOBUTTON:PropertyNames.PROPERTY_STATE,
                UIConsts.CONTROLTYPE.CHECKBOX:PropertyNames.PROPERTY_STATE,
                UIConsts.CONTROLTYPE.EDITCONTROL:"Text",
                UIConsts.CONTROLTYPE.COMBOBOX:"Text",
                UIConsts.CONTROLTYPE.PATTERNFIELD:"Text",
                UIConsts.CONTROLTYPE.LISTBOX:"SelectedItems"
            }
        try:
            return UnoDialog.dictProperties[itype]
        except KeyError:
            return ""

    def addResourceHandler(self, _Unit, _Module):
        self.m_oResource = Resource(self.xMSF, _Unit, _Module)
