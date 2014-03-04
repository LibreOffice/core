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
from abc import ABCMeta, abstractmethod
from .UnoDialog2 import UnoDialog2, Desktop, PropertyNames, UIConsts, \
    ItemListenerProcAdapter
from ..common.Resource import Resource
from ..common.HelpIds import HelpIds
from ..common.FileAccess import FileAccess

from com.sun.star.lang import NoSuchMethodException
from com.sun.star.frame import TerminationVetoException
from com.sun.star.awt.PushButtonType import HELP, STANDARD
from com.sun.star.awt.FontWeight import BOLD

class WizardDialog(UnoDialog2):

    __metaclass__ = ABCMeta

    __NEXT_ACTION_PERFORMED = "gotoNextAvailableStep"
    __BACK_ACTION_PERFORMED = "gotoPreviousAvailableStep"
    __FINISH_ACTION_PERFORMED = "finishWizard_1"
    __CANCEL_ACTION_PERFORMED = "cancelWizard_1"
    __HELP_ACTION_PERFORMED = None

    '''
    Creates a new instance of WizardDialog
    the hid is used as following :
    "HID:(hid)"   - the dialog
    "HID:(hid+1)  - the help button
    "HID:(hid+2)" - the back button
    "HID:(hid+3)" - the next button
    "HID:(hid+4)" - the create button
    "HID:(hid+5)" - the cancel button
    @param xMSF
    @param hid_
    '''

    def __init__(self, xMSF, hid_):
        super(WizardDialog,self).__init__(xMSF)
        self.__hid = hid_
        self.iButtonWidth = 50
        self.nNewStep = 1
        self.nOldStep = 1
        self.nMaxStep = 1
        self.bTerminateListenermustberemoved = True
        self.oWizardResource = Resource(xMSF, "dbw")
        self.oRoadmap = None
        self.terminateListener = None

    def activate(self):
        try:
            self.xUnoDialog.toFront()
        except Exception:
            pass
            # do nothing;

    def itemStateChanged(self, itemEvent):
        try:
            self.nNewStep = itemEvent.ItemId
            self.nOldStep = int(self.xDialogModel.Step)
            if self.nNewStep != self.nOldStep:
                self.switchToStep()

        except Exception:
            traceback.print_exc()

    def setDialogProperties(self, closeable, height, moveable, position_x,
        position_Y, step, tabIndex, title, width):
        uno.invoke(self.xDialogModel, "setPropertyValues",
             ((PropertyNames.PROPERTY_CLOSEABLE, 
                PropertyNames.PROPERTY_HEIGHT, 
                PropertyNames.PROPERTY_MOVEABLE,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP, 
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_TITLE,
                PropertyNames.PROPERTY_WIDTH),
             (closeable, height, moveable, position_x, position_Y, step, 
                tabIndex, title, width)))     
        
    def setRoadmapInteractive(self, _bInteractive):
        self.oRoadmap.Activated = _bInteractive

    def setRoadmapComplete(self, bComplete):
        self.oRoadmap.Complete = bComplete

    def isRoadmapComplete(self):
        try:
            return bool(self.oRoadmap.Complete)
        except Exception:
            traceback.print_exc()
            return False

    def setCurrentRoadmapItemID(self, ID):
        if self.oRoadmap is not None:
            nCurItemID = self.getCurrentRoadmapItemID()
            if nCurItemID != ID:
                self.oRoadmap.CurrentItemID = ID

    def getCurrentRoadmapItemID(self):
        try:
            return int(self.oRoadmap.CurrentItemID)
        except Exception:
            traceback.print_exc()
            return -1


    def initializePaths(self):
        xPropertySet = \
            self.xMSF.createInstance("com.sun.star.util.PathSettings")
        self.sUserTemplatePath = \
            xPropertySet.getPropertyValue("Template_writable")
        myFA = FileAccess(self.xMSF)
        aInternalPaths = xPropertySet.getPropertyValue("Template_internal")
        self.sTemplatePath = ""
        for path in aInternalPaths:
            if myFA.exists(path + "/wizard", False):
                self.sTemplatePath = path
                break
        if self.sTemplatePath == "":
            raise Exception("could not find wizard templates")
            
    def addRoadmap(self):
        try:
            iDialogHeight = self.xDialogModel.Height
            # the roadmap control has got no real TabIndex ever
            # that is not correct, but changing this would need time,
            # so it is used without TabIndex as before

            xRoadmapControl = self.insertControlModel(
                "com.sun.star.awt.UnoControlRoadmapModel",
                "rdmNavi",
                (PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_TABINDEX, "Tabstop",
                    PropertyNames.PROPERTY_WIDTH),
                ((iDialogHeight - 26), 0, 0, 0,
                    0, True, 85))
            self.oRoadmap = xRoadmapControl.Model
            method = getattr(self, "itemStateChanged")
            xRoadmapControl.addItemListener(
                ItemListenerProcAdapter(method))

            self.oRoadmap.Text = \
                self.oWizardResource.getResText(UIConsts.RID_COMMON + 16)
        except NoSuchMethodException:
            Resource.showCommonResourceError(xMSF)
        except Exception:
            traceback.print_exc()

    def getRoadmapItemByID(self, _ID):
        try:
            getByIndex = self.oRoadmap.getByIndex
            for i in list(range(self.oRoadmap.Count)):
                CurRoadmapItem = getByIndex(i)
                CurID = int(CurRoadmapItem.ID)
                if CurID == _ID:
                    return CurRoadmapItem

            return None
        except Exception:
            traceback.print_exc()
            return None

    def switchToStep(self,_nOldStep=None, _nNewStep=None):
        if _nOldStep is not None and _nNewStep is not None:
            self.nOldStep = _nOldStep
            self.nNewStep = _nNewStep

        self.leaveStep(self.nOldStep, self.nNewStep)
        if self.nNewStep != self.nOldStep:
            if self.nNewStep == self.nMaxStep:
                self.xDialogModel.btnWizardNext.DefaultButton = False
                self.xDialogModel.btnWizardFinish.DefaultButton = True
            else:
                self.xDialogModel.btnWizardNext.DefaultButton = True
                self.xDialogModel.btnWizardFinish.DefaultButton = False

            self.changeToStep(self.nNewStep)
            self.enterStep(self.nOldStep, self.nNewStep)
            return True

        return False

    @abstractmethod
    def leaveStep(self, nOldStep, nNewStep):
        pass

    @abstractmethod
    def enterStep(self, nOldStep, nNewStep):
        pass

    def changeToStep(self, nNewStep):
        self.xDialogModel.Step = nNewStep
        self.setCurrentRoadmapItemID(nNewStep)
        self.enableNextButton(self.getNextAvailableStep() > 0)
        self.enableBackButton(nNewStep != 1)

    def drawNaviBar(self):
        try:
            curtabindex = UIConsts.SOFIRSTWIZARDNAVITABINDEX
            iButtonWidth = self.iButtonWidth
            iButtonHeight = 14
            iCurStep = 0
            iDialogHeight = self.xDialogModel.Height
            iDialogWidth = self.xDialogModel.Width
            iHelpPosX = 8
            iBtnPosY = iDialogHeight - iButtonHeight - 6
            iCancelPosX = iDialogWidth - self.iButtonWidth - 6
            iFinishPosX = iCancelPosX - 6 - self.iButtonWidth
            iNextPosX = iFinishPosX - 6 - self.iButtonWidth
            iBackPosX = iNextPosX - 3 - self.iButtonWidth
            self.insertControlModel(
                "com.sun.star.awt.UnoControlFixedLineModel",
                "lnNaviSep",
                (PropertyNames.PROPERTY_HEIGHT, "Orientation",
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_WIDTH),
                (1, 0, 0, iDialogHeight - 26, iCurStep, iDialogWidth))
            self.insertControlModel(
                "com.sun.star.awt.UnoControlFixedLineModel",
                "lnRoadSep",
                (PropertyNames.PROPERTY_HEIGHT,
                    "Orientation",
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_WIDTH),
                (iBtnPosY - 6, 1, 85, 0, iCurStep, 1))
            propNames = (PropertyNames.PROPERTY_ENABLED,
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                "PushButtonType",
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH)
            self.xDialogModel.HelpURL = HelpIds.getHelpIdString(self.__hid)
            self.insertButton("btnWizardHelp",
                WizardDialog.__HELP_ACTION_PERFORMED,
                (PropertyNames.PROPERTY_ENABLED,
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_LABEL,
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    "PushButtonType",
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_TABINDEX,
                    PropertyNames.PROPERTY_WIDTH),
                (True, iButtonHeight,
                    self.oWizardResource.getResText(UIConsts.RID_COMMON + 15),
                    iHelpPosX, iBtnPosY,
                    uno.Any("short",HELP), iCurStep,
                    uno.Any("short",(curtabindex + 1)), iButtonWidth), self)
            self.insertButton("btnWizardBack",
                WizardDialog.__BACK_ACTION_PERFORMED, propNames,
                (False, iButtonHeight, HelpIds.getHelpIdString(self.__hid + 2),
                    self.oWizardResource.getResText(UIConsts.RID_COMMON + 13),
                    iBackPosX, iBtnPosY, uno.Any("short",STANDARD), iCurStep,
                    uno.Any("short",(curtabindex + 1)), iButtonWidth), self)
            self.insertButton("btnWizardNext",
                WizardDialog.__NEXT_ACTION_PERFORMED, propNames,
                (True, iButtonHeight, HelpIds.getHelpIdString(self.__hid + 3),
                    self.oWizardResource.getResText(UIConsts.RID_COMMON + 14),
                    iNextPosX, iBtnPosY, uno.Any("short",STANDARD), iCurStep,
                    uno.Any("short",(curtabindex + 1)), iButtonWidth), self)
            self.insertButton("btnWizardFinish",
                WizardDialog.__FINISH_ACTION_PERFORMED, propNames,
                (True, iButtonHeight, HelpIds.getHelpIdString(self.__hid + 4),
                    self.oWizardResource.getResText(UIConsts.RID_COMMON + 12),
                        iFinishPosX, iBtnPosY, uno.Any("short",STANDARD),
                        iCurStep,
                        uno.Any("short",(curtabindex + 1)),
                        iButtonWidth), self)
            self.insertButton("btnWizardCancel",
                WizardDialog.__CANCEL_ACTION_PERFORMED, propNames,
                (True, iButtonHeight, HelpIds.getHelpIdString(self.__hid + 5),
                    self.oWizardResource.getResText(UIConsts.RID_COMMON + 11),
                    iCancelPosX, iBtnPosY, uno.Any("short",STANDARD), iCurStep,
                    uno.Any("short",(curtabindex + 1)),
                    iButtonWidth), self)
            self.xDialogModel.btnWizardNext.DefaultButton = True
        except Exception:
            traceback.print_exc()

    def insertRoadMapItems(self, items, enabled):
        for index, item in enumerate(items):
            try:
                oRoadmapItem = self.oRoadmap.createInstance()
                oRoadmapItem.Label = item
                oRoadmapItem.Enabled = enabled[index]
                oRoadmapItem.ID = index + 1
                self.oRoadmap.insertByIndex(index, oRoadmapItem)
            except Exception:
                traceback.print_exc()
            
    def setStepEnabled(self, _nStep, bEnabled, enableNextButton=None):
        xRoadmapItem = self.getRoadmapItemByID(_nStep)
        if xRoadmapItem is not None:
            xRoadmapItem.Enabled = bEnabled
        if enableNextButton is not None:
            if self.getNextAvailableStep() > 0:
                self.enableNextButton(bEnabled)

    def enableNavigationButtons(
            self, _bEnableBack, _bEnableNext, _bEnableFinish):
        self.enableBackButton(_bEnableBack)
        self.enableNextButton(_bEnableNext)
        self.enableFinishButton(_bEnableFinish)

    def enableBackButton(self, enabled):
        self.xDialogModel.btnWizardBack.Enabled = enabled

    def enableNextButton(self, enabled):
        self.xDialogModel.btnWizardNext.Enabled = enabled

    def enableFinishButton(self, enabled):
        self.xDialogModel.btnWizardFinish.Enabled = enabled

    def enablefromStep(self, _iStep, _bDoEnable):
        if _iStep <= self.nMaxStep:
            for i in list(range(_iStep, self.nMaxStep)):
                self.setStepEnabled(i, _bDoEnable)
            enableFinishButton(_bDoEnable)
            if not _bDoEnable:
                enableNextButton(_iStep > getCurrentStep() + 1)
            else:
                enableNextButton(not (getCurrentStep() == self.nMaxStep))

    def isStepEnabled(self, _nStep):
        try:
            xRoadmapItem = self.getRoadmapItemByID(_nStep)
            # Todo: In this case an exception should be thrown
            if xRoadmapItem is None:
                return False
            bIsEnabled = bool(xRoadmapItem.Enabled)
            return bIsEnabled
        except Exception:
            traceback.print_exc()
            return False

    def gotoPreviousAvailableStep(self):
        try:
            if self.nNewStep > 1:
                self.nOldStep = self.nNewStep
                self.nNewStep -= 1
                while self.nNewStep > 0:
                    bIsEnabled = self.isStepEnabled(self.nNewStep)
                    if bIsEnabled:
                        break;

                    self.nNewStep -= 1
                if (self.nNewStep == 0):
                    self.nNewStep = self.nOldStep
                self.switchToStep()
        except Exception:
            traceback.print_exc()

    def getNextAvailableStep(self):
        if self.isRoadmapComplete():
            i = self.nNewStep + 1
            while i <= self.nMaxStep:
                if self.isStepEnabled(i):
                    return i
                i += 1
        return -1

    def gotoNextAvailableStep(self):
        try:
            self.nOldStep = self.nNewStep
            self.nNewStep = self.getNextAvailableStep()
            if self.nNewStep > -1:
                self.switchToStep()
        except Exception:
            traceback.print_exc()

    @abstractmethod
    def finishWizard(self):
        pass

    def finishWizard_1(self):
        '''This function will call
        if the finish button is pressed on the UI'''
        try:
            self.enableFinishButton(False)
            success = False
            try:
                success = self.finishWizard()
            finally:
                if not success:
                    self.enableFinishButton(True)

            if success:
                self.removeTerminateListener()
        except Exception:
             traceback.print_exc()

    def getMaximalStep(self):
        return self.nMaxStep

    def getCurrentStep(self):
        try:
            return int(self.xDialogModel.Step)
        except Exception:
            traceback.print_exc()
            return -1

    def setCurrentStep(self, _nNewstep):
        self.nNewStep = _nNewstep
        changeToStep(self.nNewStep)

    def setRightPaneHeaders(self, _sRightPaneHeaders):
        self.nMaxStep = len(_sRightPaneHeaders)
        self.sRightPaneHeaders = _sRightPaneHeaders
        oFontDesc = uno.createUnoStruct('com.sun.star.awt.FontDescriptor')
        oFontDesc.Weight = BOLD
        for i in range(self.nMaxStep):
            self.insertLabel("lblQueryTitle" + str(i),
                ("FontDescriptor",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_MULTILINE,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),(
                    oFontDesc, 16, _sRightPaneHeaders[i],
                    True, 91, 8, i + 1, 12, 212))

    def cancelWizard(self):
        #can be overwritten by extending class
        self.xUnoDialog.endExecute()

    def removeTerminateListener(self):
        if self.bTerminateListenermustberemoved:
            Desktop.getDesktop(self.xMSF).removeTerminateListener(self.terminateListener)
            self.bTerminateListenermustberemoved = False

    '''
    called by the cancel button and
    by the window hidden event.
    if this method was not called before,
    perform a cancel.
    '''

    def cancelWizard_1(self):
        try:
            self.cancelWizard()
            self.removeTerminateListener()
        except Exception:
            traceback.print_exc()

    def windowHidden():
        self.cancelWizard_1()

    def queryTermination(self):
        self.activate()
        raise TerminationVetoException()

    def disposing(arg0):
        self.cancelWizard_1()

    def optCreateFromTemplateItemChanged(self):
        self.bEditTemplate = False

    def optMakeChangesItemChanged(self):
        self.bEditTemplate = True
