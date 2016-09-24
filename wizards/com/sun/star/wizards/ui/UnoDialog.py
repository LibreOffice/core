#
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
from .PeerConfig import PeerConfig
from .UIConsts import UIConsts
from ..common.PropertyNames import PropertyNames

from com.sun.star.awt import Rectangle
from com.sun.star.awt.PosSize import POS

class UnoDialog(object):

    createDict = False
    dictProperties = None
    BisHighContrastModeActivated = None
    xVclWindowPeer = None

    def __init__(self, xMSF, PropertyNames, PropertyValues):
        try:
            self.xMSF = xMSF
            self.ControlList = {}
            self.xDialogModel = xMSF.createInstance(
                "com.sun.star.awt.UnoControlDialogModel")
            self.xUnoDialog = xMSF.createInstance(
                "com.sun.star.awt.UnoControlDialog")
            self.xUnoDialog.setModel(self.xDialogModel)
            self.m_oPeerConfig = None
            self.xWindowPeer = None
        except Exception:
            traceback.print_exc()

    def getPeerConfiguration(self):
        if self.m_oPeerConfig is None:
            self.m_oPeerConfig = PeerConfig(self)
        return self.m_oPeerConfig

    def getMAPConversionFactor(self, ControlName):
        xControl2 = self.xUnoDialog.getControl(ControlName)
        aSize = xControl2.Size
        dblMAPWidth = xControl2.Model.Width
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
        fieldnames = getModel(_xListBox).StringItemList
        return fieldnames.length

    def getSelectedItemPos(self, _xListBox):
        ipos = getModel(_xListBox).SelectedItems
        return ipos[0]

    def isListBoxSelected(self, _xListBox):
        ipos = getModel(_xListBox).SelectedItems
        return ipos.length > 0

    '''
    The problem with setting the visibility of controls is that
    changing the current step of a dialog will automatically make
    all controls visible. The PropertyNames.PROPERTY_STEP property
    always wins against the property "visible".
    Therefore a control meant to be invisible is placed on a step far far away.
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
            iCurDialogStep = int(self.xDialogModel.Step)
            if bIsVisible:
                setControlProperty(
                    controlname, PropertyNames.PROPERTY_STEP, iCurDialogStep)
            else:
                setControlProperty(
                    controlname, PropertyNames.PROPERTY_STEP,
                    UIConsts.INVISIBLESTEP)

        except Exception:
            traceback.print_exc()

    # repaints the currentDialogStep

    def repaintDialogStep(self):
        try:
            ncurstep = int(self.xDialogModel.Step)
            self.xDialogModel.Step = 99
            self.xDialogModel.Step = ncurstep
        except Exception:
            traceback.print_exc()

    def insertControlModel(self, serviceName, componentName, sPropNames, oPropValues):
        try:
            xControlModel = self.xDialogModel.createInstance(serviceName)
            uno.invoke(xControlModel, "setPropertyValues",
                    (sPropNames, oPropValues))
            self.xDialogModel.insertByName(componentName, xControlModel)
            xControlModel.Name = componentName
        except Exception:
            traceback.print_exc()

        aObj = self.xUnoDialog.getControl(componentName)
        return aObj

    def setFocus(self, ControlName):
        oFocusControl = self.xUnoDialog.getControl(ControlName)
        oFocusControl.setFocus()

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
        sList = oListBoxModel.StringItemList
        oListBoxModel.StringItemList = [[],[]]
        oListBoxModel.StringItemList = sList

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
        if self.xUnoDialog.getPeer() is None:
            raise AttributeError(
                "Please create a peer, using your own frame")

        self.calculateDialogPosition(FramePosSize)

        if self.xWindowPeer is None:
            self.createWindowPeer()

        self.xVclWindowPeer = self.xWindowPeer
        self.BisHighContrastModeActivated = self.isHighContrastModeActivated()
        return self.xUnoDialog.execute()

    def setVisible(self, parent):
        self.calculateDialogPosition(parent.xUnoDialog.getPosSize())
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
        if parentPeer is None:
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
        control.Model.Enabled = enabled

    @classmethod
    def getModel(self, control):
        return control.getModel()

    @classmethod
    def getDisplayProperty(self, xServiceInfo):
        if xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlFixedTextModel"):
            return PropertyNames.PROPERTY_LABEL
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlButtonModel"):
            return PropertyNames.PROPERTY_LABEL
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlCurrencyFieldModel"):
            return "Value"
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlDateFieldModel"):
            return "Date"
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlFixedLineModel"):
            return PropertyNames.PROPERTY_LABEL
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlFormattedFieldModel"):
            return "EffectiveValue"
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlNumericFieldModel"):
            return "Value"
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlPatternFieldModel"):
            return "Text"
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlProgressBarModel"):
            return "ProgressValue"
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlTimeFieldModel"):
            return "Time"
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlImageControlModel"):
            return PropertyNames.PROPERTY_IMAGEURL
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlRadioButtonModel"):
            return PropertyNames.PROPERTY_STATE
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlCheckBoxModel"):
            return PropertyNames.PROPERTY_STATE
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlEditModel"):
            return "Text"
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlComboBoxModel"):
            return "Text"
        elif xServiceInfo.supportsService(
                "com.sun.star.awt.UnoControlListBoxModel"):
            return "SelectedItems"
        else:
            return ""
        
    def isHighContrastModeActivated(self):
        if (self.xVclWindowPeer is not None):
            if (self.BisHighContrastModeActivated is None):
                nUIColor = 0
                try:
                    nUIColor = self.xVclWindowPeer.getProperty("DisplayBackgroundColor")
                except IllegalArgumentException:
                    traceback.print_exc()
                    return False

                # TODO: The following methods could be wrapped in an own class implementation
                nRed = self.getRedColorShare(nUIColor)
                nGreen = self.getGreenColorShare(nUIColor)
                nBlue = self.getBlueColorShare(nUIColor)
                nLuminance = ((nBlue * 28 + nGreen * 151 + nRed * 77) / 256)
                bisactivated = (nLuminance <= 25)
                self.BisHighContrastModeActivated = bool(bisactivated)
                return bisactivated;
            else:
                return self.BisHighContrastModeActivated
        else:
            return False


    def getRedColorShare(self, _nColor):
        nRed = _nColor / 65536
        nRedModulo = _nColor % 65536
        nGreen = nRedModulo / 256
        nGreenModulo = (nRedModulo % 256)
        nBlue = nGreenModulo
        return nRed

    def getGreenColorShare(self, _nColor):
        nRed = _nColor / 65536
        nRedModulo = _nColor % 65536
        nGreen = nRedModulo / 256
        return nGreen

    def getBlueColorShare(self, _nColor):
        nRed = _nColor / 65536
        nRedModulo = _nColor % 65536
        nGreen = nRedModulo / 256
        nGreenModulo = (nRedModulo % 256)
        nBlue = nGreenModulo
        return nBlue

    def getWizardImageUrl(self, _nResId, _nHCResId):
        if (self.isHighContrastModeActivated()):
            return "private:resource/wzi/image/" + str(_nHCResId)
        else:
            return "private:resource/wzi/image/" + str(_nResId)

    def getImageUrl(self, _surl, _shcurl):
        if (self.isHighContrastModeActivated()):
            return _shcurl
        else:
            return _surl
