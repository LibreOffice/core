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
from .FaxWizardDialog import FaxWizardDialog, Helper, PropertyNames, uno
from .CGFaxWizard import CGFaxWizard
from .FaxDocument import FaxDocument
from .FaxWizardDialogConst import HID
from ..ui.PathSelection import PathSelection
from ..ui.event.UnoDataAware import UnoDataAware
from ..ui.event.RadioDataAware import RadioDataAware
from ..text.TextFieldHandler import TextFieldHandler
from ..text.TextDocument import TextDocument
from ..text.ViewHandler import ViewHandler
from ..common.Configuration import Configuration
from ..common.SystemDialog import SystemDialog
from ..common.NoValidPathException import NoValidPathException
from ..common.HelpIds import HelpIds
from ..common.FileAccess import FileAccess
from ..common.Desktop import Desktop
from ..document.OfficeDocument import OfficeDocument

from com.sun.star.awt.VclWindowPeerAttribute import YES_NO, DEF_NO
from com.sun.star.uno import RuntimeException
from com.sun.star.util import CloseVetoException
from com.sun.star.view.DocumentZoomType import OPTIMAL
from com.sun.star.document.UpdateDocMode import FULL_UPDATE
from com.sun.star.document.MacroExecMode import ALWAYS_EXECUTE

class FaxWizardDialogImpl(FaxWizardDialog):

    def leaveStep(self, nOldStep, nNewStep):
        pass

    def enterStep(self, nOldStep, nNewStep):
        pass

    RM_TYPESTYLE = 1
    RM_ELEMENTS = 2
    RM_SENDERRECEIVER = 3
    RM_FOOTER = 4
    RM_FINALSETTINGS = 5

    lstBusinessStylePos = None
    lstPrivateStylePos = None

    def __init__(self, xmsf):
        super(FaxWizardDialogImpl, self).__init__(xmsf)
        self.bSaveSuccess = False
        self.filenameChanged = False
        self.UserTemplatePath = ""
        self.sTemplatePath = ""

    @classmethod
    def main(self, args):
        #only being called when starting wizard remotely
        try:
            ConnectStr = \
                "uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext"
            xLocMSF = Desktop.connect(ConnectStr)
            lw = FaxWizardDialogImpl(xLocMSF)
            lw.startWizard(xLocMSF)
        except Exception, e:
            traceback.print_exc()

    def startWizard(self, xMSF):
        self.running = True
        try:
            #Number of steps on WizardDialog
            self.nMaxStep = 5

            #instatiate The Document Frame for the Preview
            self.myFaxDoc = FaxDocument(xMSF, self)

            #create the dialog:
            self.drawNaviBar()

            self.buildStep1()
            self.buildStep2()
            self.buildStep3()
            self.buildStep4()
            self.buildStep5()

            self.initializeSalutation()
            self.initializeGreeting()
            self.initializeCommunication()
            self.__initializePaths()

            #special Control fFrameor setting the save Path:
            self.insertPathSelectionControl()

            self.initializeTemplates(xMSF)

            #load the last used settings
            #from the registry and apply listeners to the controls:
            self.initConfiguration()

            if self.myPathSelection.xSaveTextBox.Text.lower() == "":
                self.myPathSelection.initializePath()

            xContainerWindow = self.myFaxDoc.xFrame.ContainerWindow
            self.createWindowPeer(xContainerWindow)

            #add the Roadmap to the dialog:
            self.insertRoadmap()

            #load the last used document and apply last used settings:
            #TODO:
            self.setConfiguration()

            #If the configuration does not define
            #Greeting/Salutation/CommunicationType yet choose a default
            self.__setDefaultForGreetingAndSalutationAndCommunication()

            #disable funtionality that is not supported by the template:
            self.initializeElements()

            #disable the document, so that the user cannot change anything:
            self.myFaxDoc.xFrame.ComponentWindow.Enable = False
            self.executeDialogFromComponent(self.myFaxDoc.xFrame)
            self.removeTerminateListener()
            self.closeDocument()
            self.running = False
        except Exception, exception:
            self.removeTerminateListener()
            traceback.print_exc()
            self.running = False
            return

    def cancelWizard(self):
        self.xUnoDialog.endExecute()
        self.running = False

    def finishWizard(self):
        self.switchToStep(self.getCurrentStep(), self.nMaxStep)
        self.myFaxDoc.setWizardTemplateDocInfo( \
            self.resources.resFaxWizardDialog_title,
            self.resources.resTemplateDescription)
        endWizard = True
        try:
            fileAccess = FileAccess(self.xMSF)
            self.sPath = self.myPathSelection.getSelectedPath()
            if self.sPath == "":
                self.myPathSelection.triggerPathPicker()
                self.sPath = self.myPathSelection.getSelectedPath()

            self.sPath = fileAccess.getURL(self.sPath)
            #first, if the filename was not changed, thus
            #it is coming from a saved session, check if the
            # file exists and warn the user.
            if not self.filenameChanged:
                if fileAccess.exists(self.sPath, True):
                    answer = SystemDialog.showMessageBox(
                        self.xMSF, "MessBox", YES_NO + DEF_NO,
                        self.resources.resOverwriteWarning,
                        self.xUnoDialog.Peer)
                    if answer == 3:
                        # user said: no, do not overwrite...
                        endWizard = False
                        return False

            self.myFaxDoc.setWizardTemplateDocInfo( \
                self.resources.resFaxWizardDialog_title,
                self.resources.resTemplateDescription)
            self.myFaxDoc.killEmptyUserFields()
            self.myFaxDoc.keepLogoFrame = (self.chkUseLogo.State is not 0)
            self.myFaxDoc.keepTypeFrame = \
                (self.chkUseCommunicationType.State is not 0)
            self.myFaxDoc.killEmptyFrames()
            self.bSaveSuccess = OfficeDocument.store(self.xMSF,
                TextDocument.xTextDocument, self.sPath, "writer8_template")
            if self.bSaveSuccess:
                self.saveConfiguration()
                xIH = self.xMSF.createInstance( \
                    "com.sun.star.comp.uui.UUIInteractionHandler")
                loadValues = range(4)
                loadValues[0] = uno.createUnoStruct( \
                    'com.sun.star.beans.PropertyValue')
                loadValues[0].Name = "AsTemplate"
                loadValues[0].Value = True
                loadValues[1] = uno.createUnoStruct( \
                    'com.sun.star.beans.PropertyValue')
                loadValues[1].Name = "MacroExecutionMode"
                loadValues[1].Value = ALWAYS_EXECUTE
                loadValues[2] = uno.createUnoStruct( \
                    'com.sun.star.beans.PropertyValue')
                loadValues[2].Name = "UpdateDocMode"
                loadValues[2].Value = FULL_UPDATE
                loadValues[3] = uno.createUnoStruct( \
                    'com.sun.star.beans.PropertyValue')
                loadValues[3].Name = "InteractionHandler"
                loadValues[3].Value = xIH
                if self.bEditTemplate:
                    loadValues[0].Value = False
                else:
                    loadValues[0].Value = True

                oDoc = OfficeDocument.load(Desktop.getDesktop(self.xMSF),
                    self.sPath, "_default", loadValues)
                myViewHandler = ViewHandler(self.xMSF, oDoc)
                myViewHandler.setViewSetting("ZoomType", OPTIMAL)
            else:
                pass
                #TODO: Error Handling

        except Exception, e:
            traceback.print_exc()
        finally:
            if endWizard:
                self.xUnoDialog.endExecute()
                self.running = False

        return True

    def closeDocument(self):
        try:
            self.myFaxDoc.xFrame.close(False)
        except CloseVetoException, e:
            traceback.print_exc()

    def insertRoadmap(self):
        self.addRoadmap()
        self.insertRoadMapItems(
                [True, True, True, False, True], self.resources.RoadmapLabels)

        self.setRoadmapInteractive(True)
        self.setRoadmapComplete(True)
        self.setCurrentRoadmapItemID(1)

    def insertPathSelectionControl(self):
        self.myPathSelection = PathSelection(self.xMSF,
            self, PathSelection.TransferMode.SAVE,
            PathSelection.DialogTypes.FILE)
        self.myPathSelection.insert(
            5, 97, 70, 205, 45, self.resources.reslblTemplatePath_value,
            True, HelpIds.getHelpIdString(HID + 34),
            HelpIds.getHelpIdString(HID + 35))
        self.myPathSelection.sDefaultDirectory = self.UserTemplatePath
        self.myPathSelection.sDefaultName = "myFaxTemplate.ott"
        self.myPathSelection.sDefaultFilter = "writer8_template"
        self.myPathSelection.addSelectionListener( \
            self.myPathSelectionListener())

    def __initializePaths(self):
        try:
            self.sTemplatePath = FileAccess.getOfficePath2(self.xMSF,
                "Template", "share", "/wizard")
            self.UserTemplatePath = FileAccess.getOfficePath2(self.xMSF,
                "Template", "user", "")
            self.sBitmapPath = FileAccess.combinePaths(self.xMSF,
                self.sTemplatePath, "/../wizard/bitmap")
        except NoValidPathException, e:
            traceback.print_exc()

    def initializeTemplates(self, xMSF):
        try:
            self.sFaxPath = FileAccess.combinePaths(xMSF, self.sTemplatePath,
                "/wizard/fax")
            self.sWorkPath = FileAccess.getOfficePath2(xMSF, "Work", "", "")
            self.BusinessFiles = FileAccess.getFolderTitles(xMSF, "bus",
                self.sFaxPath)
            self.PrivateFiles = FileAccess.getFolderTitles(xMSF, "pri",
                self.sFaxPath)
            self.setControlProperty("lstBusinessStyle", "StringItemList",
                tuple(self.BusinessFiles[0]))
            self.setControlProperty("lstPrivateStyle", "StringItemList",
                tuple(self.PrivateFiles[0]))
            self.setControlProperty("lstBusinessStyle", "SelectedItems", (0,))
            self.setControlProperty("lstPrivateStyle", "SelectedItems" , (0,))
            return True
        except NoValidPathException, e:
            # TODO Auto-generated catch block
            traceback.print_exc()
            return False

    def initializeElements(self):
        self.setControlProperty("chkUseLogo",
            PropertyNames.PROPERTY_ENABLED,
            self.myFaxDoc.hasElement("Company Logo"))
        self.setControlProperty("chkUseSubject",
            PropertyNames.PROPERTY_ENABLED,
            self.myFaxDoc.hasElement("Subject Line"))
        self.setControlProperty("chkUseDate",
            PropertyNames.PROPERTY_ENABLED,
            self.myFaxDoc.hasElement("Date"))
        self.myFaxDoc.updateDateFields()

    def initializeSalutation(self):
        self.setControlProperty("lstSalutation", "StringItemList",
            self.resources.SalutationLabels)

    def initializeGreeting(self):
        self.setControlProperty("lstGreeting", "StringItemList",
            self.resources.GreetingLabels)

    def initializeCommunication(self):
        self.setControlProperty("lstCommunicationType", "StringItemList",
            self.resources.CommunicationLabels)

    def __setDefaultForGreetingAndSalutationAndCommunication(self):
        if self.lstSalutation.Text == "":
            self.lstSalutation.setText(self.resources.SalutationLabels[0])

        if self.lstGreeting.Text == "":
            self.lstGreeting.setText(self.resources.GreetingLabels[0])

        if self.lstCommunicationType.Text == "":
            self.lstCommunicationType.setText( \
                self.resources.CommunicationLabels[0])

    def initConfiguration(self):
        try:
            self.myConfig = CGFaxWizard()
            root = Configuration.getConfigurationRoot(self.xMSF,
                "/org.openoffice.Office.Writer/Wizards/Fax", False)
            self.myConfig.readConfiguration(root, "cp_")
            RadioDataAware.attachRadioButtons(
                self.myConfig, "cp_FaxType",
                (self.optBusinessFax, self.optPrivateFax), True).updateUI()
            UnoDataAware.attachListBox(
                self.myConfig.cp_BusinessFax, "cp_Style",
                self.lstBusinessStyle, True).updateUI()
            UnoDataAware.attachListBox(
                self.myConfig.cp_PrivateFax, "cp_Style", self.lstPrivateStyle,
                True).updateUI()
            cgl = self.myConfig.cp_BusinessFax
            UnoDataAware.attachCheckBox(cgl,
                "cp_PrintCompanyLogo", self.chkUseLogo, True).updateUI()
            UnoDataAware.attachCheckBox(cgl,
                "cp_PrintSubjectLine", self.chkUseSubject, True).updateUI()
            UnoDataAware.attachCheckBox(cgl,
                "cp_PrintSalutation", self.chkUseSalutation, True).updateUI()
            UnoDataAware.attachCheckBox(cgl,
                "cp_PrintDate", self.chkUseDate, True).updateUI()
            UnoDataAware.attachCheckBox(cgl, "cp_PrintCommunicationType",
                self.chkUseCommunicationType, True).updateUI()
            UnoDataAware.attachCheckBox(cgl,
                "cp_PrintGreeting", self.chkUseGreeting, True).updateUI()
            UnoDataAware.attachCheckBox(cgl,
                "cp_PrintFooter", self.chkUseFooter, True).updateUI()
            UnoDataAware.attachEditControl(cgl,
                "cp_Salutation", self.lstSalutation, True).updateUI()
            UnoDataAware.attachEditControl(cgl,
                "cp_Greeting", self.lstGreeting, True).updateUI()
            UnoDataAware.attachEditControl(cgl, "cp_CommunicationType",
                self.lstCommunicationType, True).updateUI()
            RadioDataAware.attachRadioButtons(cgl, "cp_SenderAddressType",
                (self.optSenderDefine, self.optSenderPlaceholder),
                True).updateUI()
            UnoDataAware.attachEditControl(cgl, "cp_SenderCompanyName",
                self.txtSenderName, True).updateUI()
            UnoDataAware.attachEditControl(cgl, "cp_SenderStreet",
                self.txtSenderStreet, True).updateUI()
            UnoDataAware.attachEditControl(cgl, "cp_SenderPostCode",
                self.txtSenderPostCode, True).updateUI()
            UnoDataAware.attachEditControl(cgl, "cp_SenderState",
                self.txtSenderState, True).updateUI()
            UnoDataAware.attachEditControl(cgl, "cp_SenderCity",
                self.txtSenderCity, True).updateUI()
            UnoDataAware.attachEditControl(cgl, "cp_SenderFax",
                self.txtSenderFax, True).updateUI()
            RadioDataAware.attachRadioButtons(cgl, "cp_ReceiverAddressType",
                (self.optReceiverDatabase, self.optReceiverPlaceholder),
                True).updateUI()
            UnoDataAware.attachEditControl(cgl, "cp_Footer",
                self.txtFooter, True).updateUI()
            UnoDataAware.attachCheckBox(cgl, "cp_FooterOnlySecondPage",
                self.chkFooterNextPages, True).updateUI()
            UnoDataAware.attachCheckBox(cgl, "cp_FooterPageNumbers",
                self.chkFooterPageNumbers, True).updateUI()
            RadioDataAware.attachRadioButtons(cgl, "cp_CreationType",
                (self.optCreateFax, self.optMakeChanges), True).updateUI()
            UnoDataAware.attachEditControl(cgl,
                "cp_TemplateName", self.txtTemplateName, True).updateUI()
            UnoDataAware.attachEditControl(cgl, "cp_TemplatePath",
                self.myPathSelection.xSaveTextBox, True).updateUI()
        except Exception, exception:
            traceback.print_exc()

    def saveConfiguration(self):
        try:
            root = Configuration.getConfigurationRoot(self.xMSF,
                "/org.openoffice.Office.Writer/Wizards/Fax", True)
            self.myConfig.writeConfiguration(root, "cp_")
            root.commitChanges()
        except Exception, e:
            traceback.print_exc()

    def setConfiguration(self):
        #set correct Configuration tree:
        if self.optBusinessFax.State:
            self.optBusinessFaxItemChanged()
        elif self.optPrivateFax.State:
            self.optPrivateFaxItemChanged()

    def optBusinessFaxItemChanged(self):
        FaxWizardDialogImpl.lstPrivateStylePos = None
        self.setControlProperty("lblBusinessStyle",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("lstBusinessStyle",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("lblPrivateStyle",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("lstPrivateStyle",
            PropertyNames.PROPERTY_ENABLED, False)
        self.lstBusinessStyleItemChanged()
        self.__enableSenderReceiver()
        self.__setPossibleFooter(True)

    def lstBusinessStyleItemChanged(self):
        selectedItemPos = self.lstBusinessStyle.SelectedItemPos
        #avoid to load the same item again
        if FaxWizardDialogImpl.lstBusinessStylePos is not selectedItemPos:
            FaxWizardDialogImpl.lstBusinessStylePos = selectedItemPos
            TextDocument.xTextDocument = self.myFaxDoc.loadAsPreview(
                self.BusinessFiles[1][selectedItemPos], False)
            self.initializeElements()
            self.setElements()

    def optPrivateFaxItemChanged(self):
        FaxWizardDialogImpl.lstBusinessStylePos = None
        self.setControlProperty("lblBusinessStyle",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("lstBusinessStyle",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("lblPrivateStyle",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("lstPrivateStyle",
            PropertyNames.PROPERTY_ENABLED, True)
        self.lstPrivateStyleItemChanged()
        self.__disableSenderReceiver()
        self.__setPossibleFooter(False)

    def lstPrivateStyleItemChanged(self):
        selectedItemPos = self.lstPrivateStyle.SelectedItemPos
        #avoid to load the same item again
        if FaxWizardDialogImpl.lstPrivateStylePos is not selectedItemPos:
            FaxWizardDialogImpl.lstPrivateStylePos = selectedItemPos
            TextDocument.xTextDocument = self.myFaxDoc.loadAsPreview(
                self.PrivateFiles[1][selectedItemPos], False)
            self.initializeElements()
            self.setElements()

    def txtTemplateNameTextChanged(self):
        xDocProps = TextDocument.xTextDocument.DocumentProperties
        xDocProps.Title = self.txtTemplateName.Text

    def optSenderPlaceholderItemChanged(self):
        self.setControlProperty("lblSenderName",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("lblSenderStreet",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("lblPostCodeCity",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("lblSenderFax",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("txtSenderName",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("txtSenderStreet",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("txtSenderPostCode",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("txtSenderState",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("txtSenderCity",
            PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty("txtSenderFax",
            PropertyNames.PROPERTY_ENABLED, False)
        self.myFaxDoc.fillSenderWithUserData()

    def optSenderDefineItemChanged(self):
        self.setControlProperty("lblSenderName",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("lblSenderStreet",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("lblPostCodeCity",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("lblSenderFax",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("txtSenderName",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("txtSenderStreet",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("txtSenderPostCode",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("txtSenderState",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("txtSenderCity",
            PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty("txtSenderFax",
            PropertyNames.PROPERTY_ENABLED, True)

        self.myFieldHandler = TextFieldHandler(self.myFaxDoc.xMSF,
            TextDocument.xTextDocument)
        self.txtSenderNameTextChanged()
        self.txtSenderStreetTextChanged()
        self.txtSenderPostCodeTextChanged()
        self.txtSenderStateTextChanged()
        self.txtSenderCityTextChanged()
        self.txtSenderFaxTextChanged()

    def txtSenderNameTextChanged(self):
        self.myFieldHandler.changeUserFieldContent(
            "Company", self.txtSenderName.Text)

    def txtSenderStreetTextChanged(self):
        self.myFieldHandler.changeUserFieldContent(
            "Street", self.txtSenderStreet.Text)

    def txtSenderCityTextChanged(self):
        self.myFieldHandler.changeUserFieldContent(
            "City", self.txtSenderCity.Text)

    def txtSenderPostCodeTextChanged(self):
        self.myFieldHandler.changeUserFieldContent(
            "PostCode", self.txtSenderPostCode.Text)

    def txtSenderStateTextChanged(self):
        self.myFieldHandler.changeUserFieldContent(
            PropertyNames.PROPERTY_STATE, self.txtSenderState.Text)

    def txtSenderFaxTextChanged(self):
        self.myFieldHandler.changeUserFieldContent(
            "Fax", self.txtSenderFax.Text)

    #switch Elements on/off --------------------------------------------------

    def setElements(self):
        #UI relevant:
        if self.optSenderDefine.State:
            self.optSenderDefineItemChanged()

        if self.optSenderPlaceholder.State:
            self.optSenderPlaceholderItemChanged()

        self.chkUseLogoItemChanged()
        self.chkUseSubjectItemChanged()
        self.chkUseSalutationItemChanged()
        self.chkUseGreetingItemChanged()
        self.chkUseCommunicationItemChanged()
        self.chkUseDateItemChanged()
        self.chkUseFooterItemChanged()
        self.txtTemplateNameTextChanged()
        #not UI relevant:
        if self.optReceiverDatabase.State:
            self.optReceiverDatabaseItemChanged()

        elif self.optReceiverPlaceholder.State:
            self.optReceiverPlaceholderItemChanged()

        if self.optCreateFax.State:
            self.optCreateFromTemplateItemChanged()

        elif self.optMakeChanges.State:
            self.optMakeChangesItemChanged()

    def chkUseLogoItemChanged(self):
        if self.myFaxDoc.hasElement("Company Logo"):
            self.myFaxDoc.switchElement("Company Logo",
                (self.chkUseLogo.State is not 0))

    def chkUseSubjectItemChanged(self):
        if self.myFaxDoc.hasElement("Subject Line"):
            self.myFaxDoc.switchElement("Subject Line",
                (self.chkUseSubject.State is not 0))

    def chkUseDateItemChanged(self):
        if self.myFaxDoc.hasElement("Date"):
            self.myFaxDoc.switchElement("Date",
                (self.chkUseDate.State is not 0))

    def chkUseFooterItemChanged(self):
        try:
            bFooterPossible = (self.chkUseFooter.State is not 0) \
                and bool(self.getControlProperty("chkUseFooter",
                    PropertyNames.PROPERTY_ENABLED))
            if self.chkFooterNextPages.State is not 0:
                self.myFaxDoc.switchFooter("First Page", False,
                    (self.chkFooterPageNumbers.State is not 0),
                        self.txtFooter.Text)
                self.myFaxDoc.switchFooter("Standard", bFooterPossible,
                    (self.chkFooterPageNumbers.State is not 0),
                    self.txtFooter.Text)
            else:
                self.myFaxDoc.switchFooter("First Page", bFooterPossible,
                    (self.chkFooterPageNumbers.State is not 0),
                    self.txtFooter.Text)
                self.myFaxDoc.switchFooter("Standard", bFooterPossible,
                    (self.chkFooterPageNumbers.State is not 0),
                    self.txtFooter.Text)

            #enable/disable roadmap item for footer page
            BPaperItem = self.getRoadmapItemByID( \
                FaxWizardDialogImpl.RM_FOOTER)
            Helper.setUnoPropertyValue(BPaperItem,
                PropertyNames.PROPERTY_ENABLED, bFooterPossible)
        except Exception, exception:
            traceback.print_exc()

    def chkFooterNextPagesItemChanged(self):
        self.chkUseFooterItemChanged()

    def chkFooterPageNumbersItemChanged(self):
        self.chkUseFooterItemChanged()

    def txtFooterTextChanged(self):
        self.myFaxDoc.switchFooter("First Page", True,
                    (self.chkFooterPageNumbers.State is not 0),
                    self.txtFooter.Text)

    def chkUseSalutationItemChanged(self):
        self.myFaxDoc.switchUserField("Salutation",
            self.lstSalutation.Text, (self.chkUseSalutation.State is not 0))
        self.setControlProperty("lstSalutation",
            PropertyNames.PROPERTY_ENABLED,
            self.chkUseSalutation.State is not 0)

    def lstSalutationItemChanged(self):
        self.myFaxDoc.switchUserField("Salutation",
            self.lstSalutation.Text, (self.chkUseSalutation.State is not 0))

    def chkUseCommunicationItemChanged(self):
        self.myFaxDoc.switchUserField("CommunicationType",
            self.lstCommunicationType.Text,
            (self.chkUseCommunicationType.State is not 0))
        self.setControlProperty("lstCommunicationType",
            PropertyNames.PROPERTY_ENABLED,
            self.chkUseCommunicationType.State is not 0)

    def lstCommunicationItemChanged(self):
        self.myFaxDoc.switchUserField("CommunicationType",
            self.lstCommunicationType.Text,
            (self.chkUseCommunicationType.State is not 0))

    def chkUseGreetingItemChanged(self):
        self.myFaxDoc.switchUserField("Greeting",
            self.lstGreeting.Text, (self.chkUseGreeting.State is not 0))
        self.setControlProperty("lstGreeting",
            PropertyNames.PROPERTY_ENABLED,
            (self.chkUseGreeting.State is not 0))

    def lstGreetingItemChanged(self):
        self.myFaxDoc.switchUserField("Greeting", self.lstGreeting.Text,
            (self.chkUseGreeting.State is not 0))

    def __setPossibleFooter(self, bState):
        self.setControlProperty("chkUseFooter",
            PropertyNames.PROPERTY_ENABLED, bState)
        if not bState:
            self.chkUseFooter.State = 0

        self.chkUseFooterItemChanged()

    def __enableSenderReceiver(self):
        BPaperItem = self.getRoadmapItemByID( \
            FaxWizardDialogImpl.RM_SENDERRECEIVER)
        Helper.setUnoPropertyValue(BPaperItem,
            PropertyNames.PROPERTY_ENABLED, True)

    def __disableSenderReceiver(self):
        BPaperItem = self.getRoadmapItemByID( \
            FaxWizardDialogImpl.RM_SENDERRECEIVER)
        Helper.setUnoPropertyValue(BPaperItem,
            PropertyNames.PROPERTY_ENABLED, False)

