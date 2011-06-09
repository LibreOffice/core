from UnoDialog2 import *
from common.Resource import Resource
from abc import ABCMeta, abstractmethod
from com.sun.star.lang import NoSuchMethodException
from com.sun.star.uno import Exception as UnoException
from com.sun.star.lang import IllegalArgumentException
from com.sun.star.frame import TerminationVetoException
from common.HelpIds import *
from com.sun.star.awt.PushButtonType import HELP, STANDARD
from event.MethodInvocation import *
from event.EventNames import EVENT_ITEM_CHANGED

class WizardDialog(UnoDialog2):

    __metaclass__ = ABCMeta

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
        self.__iButtonWidth = 50
        self.__nNewStep = 1
        self.__nOldStep = 1
        self.__nMaxStep = 1
        self.__bTerminateListenermustberemoved = True
        self.__oWizardResource = Resource(xMSF, "dbw")
        self.sMsgEndAutopilot = self.__oWizardResource.getResText(UIConsts.RID_DB_COMMON + 33)
        self.oRoadmap = None
        #self.vetos = VetoableChangeSupport.VetoableChangeSupport_unknown(this)

    def getResource(self):
        return self.__oWizardResource

    def activate(self):
        try:
            if self.xUnoDialog != None:
                self.xUnoDialog.toFront()

        except UnoException, ex:
            pass
            # do nothing;

    def setMaxStep(self, i):
        self.__nMaxStep = i

    def getMaxStep(self):
        return self.__nMaxStep

    def setOldStep(self, i):
        self.__nOldStep = i

    def getOldStep(self):
        return self.__nOldStep

    def setNewStep(self, i):
        self.__nNewStep = i

    def getNewStep(self):
        return self.__nNewStep

    #@see java.beans.VetoableChangeListener#vetoableChange(java.beans.PropertyChangeEvent)


    def vetoableChange(self, arg0):
        self.__nNewStep = self.__nOldStep

    def itemStateChanged(self, itemEvent):
        try:
            self.__nNewStep = itemEvent.ItemId
            self.__nOldStep = int(Helper.getUnoPropertyValue(self.xDialogModel, PropertyNames.PROPERTY_STEP))
            if self.__nNewStep != self.__nOldStep:
                switchToStep()

        except IllegalArgumentException, exception:
            traceback.print_exc()

    def setRoadmapInteractive(self, _bInteractive):
        Helper.setUnoPropertyValue(self.oRoadmap, "Activated", _bInteractive)

    def setRoadmapComplete(self, bComplete):
        Helper.setUnoPropertyValue(self.oRoadmap, "Complete", bComplete)

    def isRoadmapComplete(self):
        try:
            return bool(Helper.getUnoPropertyValue(self.oRoadmap, "Complete"))
        except IllegalArgumentException, exception:
            traceback.print_exc()
            return False

    def setCurrentRoadmapItemID(self, ID):
        if self.oRoadmap != None:
            nCurItemID = self.getCurrentRoadmapItemID()
            if nCurItemID != ID:
                Helper.setUnoPropertyValue(self.oRoadmap, "CurrentItemID", uno.Any("short",ID))

    def getCurrentRoadmapItemID(self):
        try:
            return int(Helper.getUnoPropertyValue(self.oRoadmap, "CurrentItemID"))
        except UnoException, exception:
            traceback.print_exc()
            return -1

    def addRoadmap(self):
        try:
            iDialogHeight = Helper.getUnoPropertyValue(self.xDialogModel, PropertyNames.PROPERTY_HEIGHT)
            # the roadmap control has got no real TabIndex ever
            # that is not correct, but changing this would need time, so it is used
            # without TabIndex as before
            self.oRoadmap = self.insertControlModel("com.sun.star.awt.UnoControlRoadmapModel", "rdmNavi", (PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Tabstop", PropertyNames.PROPERTY_WIDTH),((iDialogHeight - 26), 0, 0, 0, uno.Any("short",0), True, uno.Any("short",85)))
            self.oRoadmap.setPropertyValue(PropertyNames.PROPERTY_NAME, "rdmNavi")

            mi = MethodInvocation("itemStateChanged", self)
            self.xRoadmapControl = self.xUnoDialog.getControl("rdmNavi")
            self.xRoadmapControl.addItemListener(ItemListenerProcAdapter(None))

            Helper.setUnoPropertyValue(self.oRoadmap, "Text", self.__oWizardResource.getResText(UIConsts.RID_COMMON + 16))
        except NoSuchMethodException, ex:
            Resource.showCommonResourceError(xMSF)
        except UnoException, jexception:
            traceback.print_exc()

    def setRMItemLabels(self, _oResource, StartResID):
        self.sRMItemLabels = _oResource.getResArray(StartResID, self.__nMaxStep)

    def getRMItemLabels(self):
        return self.sRMItemLabels

    def insertRoadmapItem(self, _Index, _bEnabled, _LabelID, _CurItemID):
        return insertRoadmapItem(_Index, _bEnabled, self.sRMItemLabels(_LabelID), _CurItemID)

    def insertRoadmapItem(self, Index, _bEnabled, _sLabel, _CurItemID):
        try:
            oRoadmapItem = self.oRoadmap.createInstance()
            Helper.setUnoPropertyValue(oRoadmapItem, PropertyNames.PROPERTY_LABEL, _sLabel)
            Helper.setUnoPropertyValue(oRoadmapItem, PropertyNames.PROPERTY_ENABLED, _bEnabled)
            Helper.setUnoPropertyValue(oRoadmapItem, "ID", _CurItemID)
            self.oRoadmap.insertByIndex(Index, oRoadmapItem)
            NextIndex = Index + 1
            return NextIndex
        except UnoException, exception:
            traceback.print_exc()
            return -1

    def getRMItemCount(self):
        return self.oRoadmap.getCount()

    def getRoadmapItemByID(self, _ID):
        try:
            i = 0
            while i < self.oRoadmap.getCount():
                CurRoadmapItem = self.oRoadmap.getByIndex(i)
                CurID = int(Helper.getUnoPropertyValue(CurRoadmapItem, "ID"))
                if CurID == _ID:
                    return CurRoadmapItem

                i += 1
            return None
        except UnoException, exception:
            traceback.print_exc()
            return None

    def switchToStep(self,_nOldStep=None, _nNewStep=None):
        if _nOldStep is not None and _nNewStep is not None:
            self.__nOldStep = _nOldStep
            self.__nNewStep = _nNewStep

        self.leaveStep(self.__nOldStep, self.__nNewStep)
        if self.__nNewStep != self.__nOldStep:
            if self.__nNewStep == self.__nMaxStep:
                self.setControlProperty("btnWizardNext", "DefaultButton", False)
                self.setControlProperty("btnWizardFinish", "DefaultButton", True)
            else:
                self.setControlProperty("btnWizardNext", "DefaultButton", True)
                self.setControlProperty("btnWizardFinish", "DefaultButton", False)

            self.changeToStep(self.__nNewStep)
            self.enterStep(self.__nOldStep, self.__nNewStep)
            return True

        return False

    @abstractmethod
    def leaveStep(self, nOldStep, nNewStep):
        pass

    @abstractmethod
    def enterStep(self, nOldStep, nNewStep):
        pass

    def changeToStep(self, nNewStep):
        Helper.setUnoPropertyValue(self.xDialogModel, PropertyNames.PROPERTY_STEP, nNewStep)
        self.setCurrentRoadmapItemID(nNewStep)
        self.enableNextButton(self.getNextAvailableStep() > 0)
        self.enableBackButton(nNewStep != 1)

    def iscompleted(self, _ndialogpage):
        return False

    def ismodified(self, _ndialogpage):
        return False

    def drawNaviBar(self):
        try:
            curtabindex = UIConsts.SOFIRSTWIZARDNAVITABINDEX
            iButtonWidth = self.__iButtonWidth
            iButtonHeight = 14
            iCurStep = 0
            iDialogHeight = Helper.getUnoPropertyValue(self.xDialogModel, PropertyNames.PROPERTY_HEIGHT)
            iDialogWidth = Helper.getUnoPropertyValue(self.xDialogModel, PropertyNames.PROPERTY_WIDTH)
            iHelpPosX = 8
            iBtnPosY = iDialogHeight - iButtonHeight - 6
            iCancelPosX = iDialogWidth - self.__iButtonWidth - 6
            iFinishPosX = iCancelPosX - 6 - self.__iButtonWidth
            iNextPosX = iFinishPosX - 6 - self.__iButtonWidth
            iBackPosX = iNextPosX - 3 - self.__iButtonWidth
            self.insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "lnNaviSep", (PropertyNames.PROPERTY_HEIGHT, "Orientation", PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH), (1, 0, 0, iDialogHeight - 26, iCurStep, iDialogWidth))
            self.insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "lnRoadSep",(PropertyNames.PROPERTY_HEIGHT, "Orientation", PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH),(iBtnPosY - 6, 1, 85, 0, iCurStep, 1))
            propNames = (PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "PushButtonType", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH)
            Helper.setUnoPropertyValue(self.xDialogModel, PropertyNames.PROPERTY_HELPURL, HelpIds.getHelpIdString(self.__hid))
            self.insertButton("btnWizardHelp", "",(PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "PushButtonType", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH),(True, iButtonHeight, self.__oWizardResource.getResText(UIConsts.RID_COMMON + 15), iHelpPosX, iBtnPosY, uno.Any("short",HELP), iCurStep, uno.Any("short",(curtabindex + 1)), iButtonWidth))
            self.insertButton("btnWizardBack", self.gotoPreviousAvailableStep, propNames,(False, iButtonHeight, HelpIds.getHelpIdString(self.__hid + 2), self.__oWizardResource.getResText(UIConsts.RID_COMMON + 13), iBackPosX, iBtnPosY, uno.Any("short",STANDARD), iCurStep, uno.Any("short",(curtabindex + 1)), iButtonWidth))
            self.insertButton("btnWizardNext", self.gotoNextAvailableStep, propNames,(True, iButtonHeight, HelpIds.getHelpIdString(self.__hid + 3), self.__oWizardResource.getResText(UIConsts.RID_COMMON + 14), iNextPosX, iBtnPosY, uno.Any("short",STANDARD), iCurStep, uno.Any("short",(curtabindex + 1)), iButtonWidth))
            self.insertButton("btnWizardFinish", self.finishWizard_1, propNames,(True, iButtonHeight, HelpIds.getHelpIdString(self.__hid + 4), self.__oWizardResource.getResText(UIConsts.RID_COMMON + 12), iFinishPosX, iBtnPosY, uno.Any("short",STANDARD), iCurStep, uno.Any("short",(curtabindex + 1)), iButtonWidth))
            self.insertButton("btnWizardCancel", self.cancelWizard_1, propNames,(True, iButtonHeight, HelpIds.getHelpIdString(self.__hid + 5), self.__oWizardResource.getResText(UIConsts.RID_COMMON + 11), iCancelPosX, iBtnPosY, uno.Any("short",STANDARD), iCurStep, uno.Any("short",(curtabindex + 1)), iButtonWidth))
            self.setControlProperty("btnWizardNext", "DefaultButton", True)
            # add a window listener, to know
            # if the user used "escape" key to
            # close the dialog.
            windowHidden = MethodInvocation("windowHidden", self)
            self.xUnoDialog.addWindowListener(WindowListenerProcAdapter(None))
            dialogName = Helper.getUnoPropertyValue(self.xDialogModel, PropertyNames.PROPERTY_NAME)
        except Exception, exception:
            traceback.print_exc()

    def insertRoadMapItems(self, items, steps, enabled):
        i = 0
        while i < items.length:
            insertRoadmapItem(i, enabled(i), items(i), steps(i))
            i += 1

    def setStepEnabled(self, _nStep, bEnabled, enableNextButton):
        setStepEnabled(_nStep, bEnabled)
        if self.getNextAvailableStep() > 0:
            self.enableNextButton(bEnabled)

    def enableNavigationButtons(self, _bEnableBack, _bEnableNext, _bEnableFinish):
        self.enableBackButton(_bEnableBack)
        self.enableNextButton(_bEnableNext)
        self.enableFinishButton(_bEnableFinish)

    def enableBackButton(self, enabled):
        self.setControlProperty("btnWizardBack", PropertyNames.PROPERTY_ENABLED, enabled)

    def enableNextButton(self, enabled):
        self.setControlProperty("btnWizardNext", PropertyNames.PROPERTY_ENABLED, enabled)

    def enableFinishButton(self, enabled):
        self.setControlProperty("btnWizardFinish", PropertyNames.PROPERTY_ENABLED, enabled)

    def setStepEnabled(self, _nStep, bEnabled):
        xRoadmapItem = getRoadmapItemByID(_nStep)
        if xRoadmapItem != None:
            Helper.setUnoPropertyValue(xRoadmapItem, PropertyNames.PROPERTY_ENABLED, bEnabled)

    def enablefromStep(self, _iStep, _bDoEnable):
        if _iStep <= self.__nMaxStep:
            i = _iStep
            while i <= self.__nMaxStep:
                setStepEnabled(i, _bDoEnable)
                i += 1
            enableFinishButton(_bDoEnable)
            if not _bDoEnable:
                enableNextButton(_iStep > getCurrentStep() + 1)
            else:
                enableNextButton(not (getCurrentStep() == self.__nMaxStep))

    def isStepEnabled(self, _nStep):
        try:
            xRoadmapItem = self.getRoadmapItemByID(_nStep)
            # Todo: In this case an exception should be thrown
            if (xRoadmapItem == None):
                return False
            bIsEnabled = bool(Helper.getUnoPropertyValue(xRoadmapItem, PropertyNames.PROPERTY_ENABLED))
            return bIsEnabled
        except UnoException, exception:
            traceback.print_exc()
            return False

    def gotoPreviousAvailableStep(self, oActionEvent):
        try:
            if self.__nNewStep > 1:
                self.__nOldStep = self.__nNewStep
                self.__nNewStep -= 1
                while self.__nNewStep > 0:
                    bIsEnabled = self.isStepEnabled(self.__nNewStep)
                    if bIsEnabled:
                        break;

                    self.__nNewStep -= 1
                if (self.__nNewStep == 0):
                    self.__nNewStep = self.__nOldStep
                self.switchToStep()
        except Exception, e:
            traceback.print_exc()

    #TODO discuss with rp

    def getNextAvailableStep(self):
        if self.isRoadmapComplete():
            i = self.__nNewStep + 1
            while i <= self.__nMaxStep:
                if self.isStepEnabled(i):
                    return i

                i += 1

        return -1

    def gotoNextAvailableStep(self, oActionEvent):
        try:
            self.__nOldStep = self.__nNewStep
            self.__nNewStep = self.getNextAvailableStep()
            if self.__nNewStep > -1:
                self.switchToStep()
        except Exception, e:
            traceback.print_exc()

    @abstractmethod
    def finishWizard(self):
        pass

    def finishWizard_1(self, oActionEvent):
        '''This function will call if the finish button is pressed on the UI'''
        try:
            self.enableFinishButton(False)
            success = False
            try:
                success = self.finishWizard()
            finally:
                if not success:
                    self.enableFinishButton(True)

            if success:
                removeTerminateListener()
        except Exception, e:
             traceback.print_exc()

    def getMaximalStep(self):
        return self.__nMaxStep

    def getCurrentStep(self):
        try:
            return int(Helper.getUnoPropertyValue(self.xDialogModel, PropertyNames.PROPERTY_STEP))
        except UnoException, exception:
            traceback.print_exc()
            return -1

    def setCurrentStep(self, _nNewstep):
        self.__nNewStep = _nNewstep
        changeToStep(self.__nNewStep)

    def setRightPaneHeaders(self, _oResource, StartResID, _nMaxStep):
        self.sRightPaneHeaders = _oResource.getResArray(StartResID, _nMaxStep)
        setRightPaneHeaders(self.sRightPaneHeaders)

    def setRightPaneHeaders(self, _sRightPaneHeaders):
        self.__nMaxStep = _sRightPaneHeaders.length
        self.sRightPaneHeaders = _sRightPaneHeaders
        oFontDesc = FontDescriptor.FontDescriptor()
        oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD
        i = 0
        while i < self.sRightPaneHeaders.length:
            insertLabel("lblQueryTitle" + String.valueOf(i),("FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH),(oFontDesc, 16, self.sRightPaneHeaders(i), True, 91, 8, i + 1, uno.Any("short",12), 212))
            i += 1

    def cancelWizard(self):
        #can be overwritten by extending class
        xDialog.endExecute()

    def removeTerminateListener(self):
        if self.__bTerminateListenermustberemoved:
            #COMMENTED
            #Desktop.getDesktop(self.xMSF).removeTerminateListener( \
            #    ActionListenerProcAdapter(self))
            self.__bTerminateListenermustberemoved = False

    '''
    called by the cancel button and
    by the window hidden event.
    if this method was not called before,
    perform a cancel.
    '''

    def cancelWizard_1(self, oActionEvent):
        try:
            self.cancelWizard()
            self.removeTerminateListener()
        except Exception,e:
            traceback.print_exc()


    def windowHidden(self):
        cancelWizard_1()

    def notifyTermination(self, arg0):
        cancelWizard_1()

    def queryTermination(self, arg0):
        activate()
        raise TerminationVetoException ();

    def disposing(self, arg0):
        cancelWizard_1()
