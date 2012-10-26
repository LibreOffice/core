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
from .LetterWizardDialog import LetterWizardDialog, Helper, \
    PropertyNames, uno, HelpIds, HID
from .LetterDocument import LetterDocument, BusinessPaperObject
from .CGLetterWizard import CGLetterWizard
from ..common.NoValidPathException import NoValidPathException
from ..common.FileAccess import FileAccess
from ..common.Configuration import Configuration
from ..common.SystemDialog import SystemDialog
from ..common.Desktop import Desktop
from ..ui.PathSelection import PathSelection
from ..ui.event.UnoDataAware import UnoDataAware
from ..ui.event.RadioDataAware import RadioDataAware
from ..text.TextDocument import TextDocument
from ..text.ViewHandler import ViewHandler
from ..text.TextFieldHandler import TextFieldHandler
from ..document.OfficeDocument import OfficeDocument

from com.sun.star.awt.VclWindowPeerAttribute import YES_NO, DEF_NO
from com.sun.star.uno import RuntimeException
from com.sun.star.util import CloseVetoException
from com.sun.star.view.DocumentZoomType import OPTIMAL
from com.sun.star.document.UpdateDocMode import FULL_UPDATE
from com.sun.star.document.MacroExecMode import ALWAYS_EXECUTE


class LetterWizardDialogImpl(LetterWizardDialog):

    RM_TYPESTYLE = 1
    RM_BUSINESSPAPER = 2
    RM_ELEMENTS = 3
    RM_SENDERRECEIVER = 4
    RM_FOOTER = 5
    RM_FINALSETTINGS = 6

    def enterStep(self, OldStep, NewStep):
        pass

    def leaveStep(self, OldStep, NewStep):
        pass

    def __init__(self, xmsf):
        super(LetterWizardDialogImpl, self).__init__(xmsf)
        self.lstBusinessStylePos = None
        self.lstPrivateStylePos = None
        self.lstPrivOfficialStylePos = None
        self.xmsf = xmsf
        self.bSaveSuccess = False
        self.filenameChanged = False
        self.BusCompanyLogo = None
        self.BusCompanyAddress = None
        self.BusCompanyAddressReceiver = None
        self.BusFooter = None

    @classmethod
    def main(self):
        #Call the wizard remotely(see README)
        try:
            ConnectStr = \
                "uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext"
            xLocMSF = Desktop.connect(ConnectStr)
            lw = LetterWizardDialogImpl(xLocMSF)
            lw.startWizard(xLocMSF)
        except Exception as e:
            print ("Wizard failure exception " + str(type(e)) +
                   " message " + str(e) + " args " + str(e.args) +
                   traceback.format_exc())

    def startWizard(self, xMSF):
        self.running = True
        try:
            #Number of steps on WizardDialog
            self.nMaxStep = 6

            #instatiate The Document Frame for the Preview
            self.myLetterDoc = LetterDocument(xMSF, self)

            #create the dialog
            self.drawNaviBar()
            self.buildStep1()
            self.buildStep2()
            self.buildStep3()
            self.buildStep4()
            self.buildStep5()
            self.buildStep6()
            self.__initializePaths()
            self.initializeSalutation()
            self.initializeGreeting()

            #special Control fFrameor setting the save Path:
            self.insertPathSelectionControl()

            self.myConfig = CGLetterWizard()

            self.initializeTemplates(xMSF)

            #load the last used settings
            #from the registry and apply listeners to the controls:
            self.initConfiguration()

            if self.myConfig.cp_BusinessLetter.cp_Greeting :
                self.myConfig.cp_BusinessLetter.cp_Greeting = \
                    self.resources.GreetingLabels[0]

            if self.myConfig.cp_BusinessLetter.cp_Salutation:
                self.myConfig.cp_BusinessLetter.cp_Salutation = \
                    self.resources.SalutationLabels[0]

            if self.myConfig.cp_PrivateOfficialLetter.cp_Greeting:
                self.myConfig.cp_PrivateOfficialLetter.cp_Greeting = \
                    self.resources.GreetingLabels[1]

            if self.myConfig.cp_PrivateOfficialLetter.cp_Salutation:
                self.myConfig.cp_PrivateOfficialLetter.cp_Salutation = \
                    self.resources.SalutationLabels[1]

            if self.myConfig.cp_PrivateLetter.cp_Greeting:
                self.myConfig.cp_PrivateLetter.cp_Greeting = \
                    self.resources.GreetingLabels[2]

            if self.myConfig.cp_PrivateLetter.cp_Salutation:
                self.myConfig.cp_PrivateLetter.cp_Salutation = \
                    self.resources.SalutationLabels[2]

            if self.myPathSelection.xSaveTextBox.Text.lower():
                self.myPathSelection.initializePath()

            xContainerWindow = self.myLetterDoc.xFrame.ContainerWindow
            self.createWindowPeer(xContainerWindow)
            self.insertRoadmap()
            self.setConfiguration()
            self.setDefaultForGreetingAndSalutation()
            self.initializeElements()
            self.myLetterDoc.xFrame.ComponentWindow.Enable = False
            self.executeDialogFromComponent(self.myLetterDoc.xFrame)
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
        endWizard = True
        try:
            fileAccess = FileAccess(self.xMSF)
            self.sPath = self.myPathSelection.getSelectedPath()
            if not self.sPath:
                self.myPathSelection.triggerPathPicker()
                self.sPath = self.myPathSelection.getSelectedPath()

            self.sPath = fileAccess.getURL(self.sPath)
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

            self.myLetterDoc.setWizardTemplateDocInfo(
                self.resources.resLetterWizardDialog_title,
                self.resources.resTemplateDescription)
            self.myLetterDoc.killEmptyUserFields()
            self.myLetterDoc.keepLogoFrame = self.chkUseLogo.State != 0
            if self.chkBusinessPaper.State != 0 \
                    and self.chkPaperCompanyLogo.State != 0:
                self.myLetterDoc.keepLogoFrame = False

            self.myLetterDoc.keepBendMarksFrame = \
                self.chkUseBendMarks.State != 0
            self.myLetterDoc.keepLetterSignsFrame = \
                self.chkUseSigns.State != 0
            self.myLetterDoc.keepSenderAddressRepeatedFrame = \
                self.chkUseAddressReceiver.State != 0
            if self.optBusinessLetter.State:
                if self.chkBusinessPaper.State != 0 \
                        and self.chkCompanyReceiver.State != 0:
                    self.myLetterDoc.keepSenderAddressRepeatedFrame = False

                if self.chkBusinessPaper.State != 0 \
                        and self.chkPaperCompanyAddress.State != 0:
                    self.myLetterDoc.keepAddressFrame = False

            self.myLetterDoc.killEmptyFrames()
            self.bSaveSuccess = \
                OfficeDocument.store(
                    self.xMSF, TextDocument.xTextDocument,
                    self.sPath, "writer8_template")
            if self.bSaveSuccess:
                self.saveConfiguration()
                xIH = self.xMSF.createInstance(
                    "com.sun.star.comp.uui.UUIInteractionHandler")
                loadValues = range(4)
                loadValues[0] = uno.createUnoStruct( \
                    'com.sun.star.beans.PropertyValue')
                loadValues[0].Name = "AsTemplate"
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

                oDoc = OfficeDocument.load(
                    Desktop.getDesktop(self.xMSF),
                    self.sPath, "_default", loadValues)
                myViewHandler = ViewHandler(self.xMSF, oDoc)
                myViewHandler.setViewSetting("ZoomType", OPTIMAL)
            else:
                pass

        except Exception, e:
            traceback.print_exc()
        finally:
            if endWizard:
                self.xUnoDialog.endExecute()
                self.running = False

        return True;

    def closeDocument(self):
        try:
            xCloseable = self.myLetterDoc.xFrame
            xCloseable.close(False)
        except CloseVetoException, e:
            traceback.print_exc()

    def optBusinessLetterItemChanged(self):
        self.lstPrivateStylePos = None
        self.lstPrivOfficialStylePos = None
        self.setControlProperty(
            "lblBusinessStyle", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "lstBusinessStyle", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "chkBusinessPaper", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "lblPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lstPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lblPrivateStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lstPrivateStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.lstBusinessStyleItemChanged()
        self.enableSenderReceiver()
        self.setPossibleFooter(True)
        if self.myPathSelection.xSaveTextBox.Text.lower():
            self.myPathSelection.initializePath()

    def optPrivOfficialLetterItemChanged(self):
        self.lstBusinessStylePos = None
        self.lstPrivateStylePos = None
        self.setControlProperty(
            "lblBusinessStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lstBusinessStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "chkBusinessPaper", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lblPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "lstPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "lblPrivateStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lstPrivateStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.lstPrivOfficialStyleItemChanged()
        self.disableBusinessPaper()
        self.enableSenderReceiver()
        self.setPossibleFooter(True)
        if self.myPathSelection.xSaveTextBox.Text.lower():
            self.myPathSelection.initializePath()

    def optPrivateLetterItemChanged(self):
        self.lstBusinessStylePos = None
        self.lstPrivOfficialStylePos = None
        self.setControlProperty(
            "lblBusinessStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lstBusinessStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "chkBusinessPaper", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lblPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lstPrivOfficialStyle", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lblPrivateStyle", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "lstPrivateStyle", PropertyNames.PROPERTY_ENABLED, True)
        self.lstPrivateStyleItemChanged()
        self.disableBusinessPaper()
        self.disableSenderReceiver()
        self.setPossibleFooter(False)
        if self.myPathSelection.xSaveTextBox.Text.lower():
            self.myPathSelection.initializePath()

    def optSenderPlaceholderItemChanged(self):
        self.setControlProperty(
            "lblSenderName", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lblSenderStreet", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "lblPostCodeCity", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "txtSenderName", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "txtSenderStreet", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "txtSenderPostCode", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "txtSenderState", PropertyNames.PROPERTY_ENABLED, False)
        self.setControlProperty(
            "txtSenderCity", PropertyNames.PROPERTY_ENABLED, False)
        self.myLetterDoc.fillSenderWithUserData()

    def optSenderDefineItemChanged(self):
        self.setControlProperty(
            "lblSenderName", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "lblSenderStreet", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "lblPostCodeCity", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "txtSenderName", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "txtSenderStreet", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "txtSenderPostCode", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "txtSenderState", PropertyNames.PROPERTY_ENABLED, True)
        self.setControlProperty(
            "txtSenderCity", PropertyNames.PROPERTY_ENABLED, True)
        self.txtSenderNameTextChanged()
        self.txtSenderStreetTextChanged()
        self.txtSenderPostCodeTextChanged()
        self.txtSenderStateTextChanged()
        self.txtSenderCityTextChanged()

    def lstBusinessStyleItemChanged(self):
        selectedItemPos = self.lstBusinessStyle.SelectedItemPos
        if self.lstBusinessStylePos != selectedItemPos:
            self.lstBusinessStylePos = selectedItemPos
            TextDocument.xTextDocument = \
                self.myLetterDoc.loadAsPreview(
                    self.BusinessFiles.values()[selectedItemPos], False)
            self.initializeElements()
            self.chkBusinessPaperItemChanged()
            self.setElements(False)
            
    def lstPrivOfficialStyleItemChanged(self):
        selectedItemPos = self.lstPrivOfficialStyle.SelectedItemPos
        if self.lstPrivOfficialStylePos != selectedItemPos:
            self.lstPrivOfficialStylePos = selectedItemPos
            TextDocument.xTextDocument = \
                self.myLetterDoc.loadAsPreview(
                    self.OfficialFiles.values()[selectedItemPos], False)
            self.initializeElements()
            self.setPossibleSenderData(True)
            self.setElements(False)

    def lstPrivateStyleItemChanged(self):
        selectedItemPos = self.lstPrivateStyle.SelectedItemPos
        if self.lstPrivateStylePos != selectedItemPos:
            self.lstPrivateStylePos = selectedItemPos
            TextDocument.xTextDocument = \
                self.myLetterDoc.loadAsPreview(
                    self.PrivateFiles.values()[selectedItemPos], False)
            self.initializeElements()
            self.setElements(True)

    def numLogoHeightTextChanged(self):
        self.BusCompanyLogo.iHeight = int(self.numLogoHeight.Value * 1000)
        self.BusCompanyLogo.setFramePosition()

    def numLogoWidthTextChanged(self):
        self.BusCompanyLogo.iWidth = int(self.numLogoWidth.Value * 1000)
        self.BusCompanyLogo.setFramePosition()

    def numLogoXTextChanged(self):
        self.BusCompanyLogo.iXPos = int(self.numLogoX.Value * 1000)
        self.BusCompanyLogo.setFramePosition()

    def numLogoYTextChanged(self):
        self.BusCompanyLogo.iYPos = int(self.numLogoY.Value * 1000)
        self.BusCompanyLogo.setFramePosition()

    def numAddressWidthTextChanged(self):
        self.BusCompanyAddress.iWidth = int(self.numAddressWidth.Value * 1000)
        self.BusCompanyAddress.setFramePosition()

    def numAddressXTextChanged(self):
        self.BusCompanyAddress.iXPos = int(self.numAddressX.Value * 1000)
        self.BusCompanyAddress.setFramePosition()

    def numAddressYTextChanged(self):
        self.BusCompanyAddress.iYPos = int(self.numAddressY.Value * 1000)
        self.BusCompanyAddress.setFramePosition()

    def numAddressHeightTextChanged(self):
        self.BusCompanyAddress.iHeight = int(self.numAddressHeight.Value * 1000)
        self.BusCompanyAddress.setFramePosition()

    def numFooterHeightTextChanged(self):
        self.BusFooter.iHeight = int(self.numFooterHeight.Value * 1000)
        self.BusFooter.iYPos = \
            self.myLetterDoc.DocSize.Height - self.BusFooter.iHeight
        self.BusFooter.setFramePosition()

    def chkPaperCompanyLogoItemChanged(self):
        if self.chkPaperCompanyLogo.State != 0:
            if self.numLogoWidth.Value == 0:
                self.numLogoWidth.Value = 0.1

            if self.numLogoHeight.Value == 0:
                self.numLogoHeight.Value = 0.1
            self.BusCompanyLogo = BusinessPaperObject(
                "Company Logo", int(self.numLogoWidth.Value * 1000),
                int(self.numLogoHeight.Value * 1000),
                int(self.numLogoX.Value * 1000),
                self.numLogoY.Value * 1000)
            self.setControlProperty(
            "numLogoHeight", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "lblCompanyLogoHeight", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "numLogoWidth", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "lblCompanyLogoWidth", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "numLogoX", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "lblCompanyLogoX", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "numLogoY", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "lblCompanyLogoY", PropertyNames.PROPERTY_ENABLED, True)
            self.setPossibleLogo(False)
        else:
            if self.BusCompanyLogo != None:
                self.BusCompanyLogo.removeFrame()

            self.setControlProperty(
            "numLogoHeight", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "lblCompanyLogoHeight", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "numLogoWidth", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "lblCompanyLogoWidth", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "numLogoX", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "lblCompanyLogoX", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "numLogoY", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "lblCompanyLogoY", PropertyNames.PROPERTY_ENABLED, False)
            self.setPossibleLogo(True)

    def chkPaperCompanyAddressItemChanged(self):
        if self.chkPaperCompanyAddress.State != 0:
            if self.numAddressWidth.Value == 0:
                self.numAddressWidth.Value = 0.1

            if self.numAddressHeight.Value == 0:
                self.numAddressHeight.Value = 0.1

            self.BusCompanyAddress = BusinessPaperObject(
                "Company Address", int(self.numAddressWidth.Value * 1000),
                int(self.numAddressHeight.Value * 1000),
                int(self.numAddressX.Value * 1000),
                int(self.numAddressY.Value * 1000))
            self.setControlProperty(
            "numAddressHeight", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "lblCompanyAddressHeight", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "numAddressWidth", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "lblCompanyAddressWidth", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "numAddressX", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "lblCompanyAddressX", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "numAddressY", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "lblCompanyAddressY", PropertyNames.PROPERTY_ENABLED, True)
            if self.myLetterDoc.hasElement("Sender Address"):
                self.myLetterDoc.switchElement(
                "Sender Address", False)

            if self.chkCompanyReceiver.State != 0:
                self.setPossibleSenderData(False)

        else:
            if self.BusCompanyAddress is not None:
                self.BusCompanyAddress.removeFrame()
            self.setControlProperty(
            "numAddressHeight", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "lblCompanyAddressHeight", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "numAddressWidth", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "lblCompanyAddressWidth", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "numAddressX", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "lblCompanyAddressX", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "numAddressY", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "lblCompanyAddressY", PropertyNames.PROPERTY_ENABLED, False)
            if self.myLetterDoc.hasElement("Sender Address"):
                self.myLetterDoc.switchElement("Sender Address", True)

            self.setPossibleSenderData(True)
            if self.optSenderDefine.State:
                self.optSenderDefineItemChanged()

            if self.optSenderPlaceholder.State:
                self.optSenderPlaceholderItemChanged()

    def chkCompanyReceiverItemChanged(self):
        xReceiverFrame = None
        if self.chkCompanyReceiver.State != 0:
            try:
                xReceiverFrame = TextDocument.getFrameByName(
                    "Receiver Address", TextDocument.xTextDocument)
                iFrameWidth = int(Helper.getUnoPropertyValue(
                    xReceiverFrame, PropertyNames.PROPERTY_WIDTH))
                iFrameX = int(Helper.getUnoPropertyValue(
                    xReceiverFrame, "HoriOrientPosition"))
                iFrameY = int(Helper.getUnoPropertyValue(
                    xReceiverFrame, "VertOrientPosition"))
                iReceiverHeight = int(0.5 * 1000)
                self.BusCompanyAddressReceiver = BusinessPaperObject(
                    " ", iFrameWidth, iReceiverHeight, iFrameX,
                    iFrameY - iReceiverHeight)
                self.setPossibleAddressReceiver(False)
            except NoSuchElementException:
                traceback.print_exc()
            except WrappedTargetException:
                traceback.print_exc()

            if self.chkPaperCompanyAddress.State != 0:
                self.setPossibleSenderData(False)

        else:
            if self.BusCompanyAddressReceiver != None:
                self.BusCompanyAddressReceiver.removeFrame()

            self.setPossibleAddressReceiver(True)
            self.setPossibleSenderData(True)
            if self.optSenderDefine.State:
                self.optSenderDefineItemChanged()

            if self.optSenderPlaceholder.State:
                self.optSenderPlaceholderItemChanged()

    def chkPaperFooterItemChanged(self):
        if self.chkPaperFooter.State != 0:
            if self.numFooterHeight.Value == 0:
                self.numFooterHeight.Value = 0.1

            self.BusFooter = BusinessPaperObject(
                "Footer", self.myLetterDoc.DocSize.Width,
                int(self.numFooterHeight.Value * 1000), 0,
                int(self.myLetterDoc.DocSize.Height - \
                    (self.numFooterHeight.Value * 1000)))
            self.setControlProperty(
            "numFooterHeight", PropertyNames.PROPERTY_ENABLED, True)
            self.setControlProperty(
            "lblFooterHeight", PropertyNames.PROPERTY_ENABLED, True)
            self.setPossibleFooter(False)
        else:
            if self.BusFooter != None:
                self.BusFooter.removeFrame()

            self.setControlProperty(
            "numFooterHeight", PropertyNames.PROPERTY_ENABLED, False)
            self.setControlProperty(
            "lblFooterHeight", PropertyNames.PROPERTY_ENABLED, False)
            self.setPossibleFooter(True)

    def chkUseLogoItemChanged(self):
        try:
            if self.myLetterDoc.hasElement("Company Logo"):
                logostatus = \
                    bool(self.getControlProperty(
                        "chkUseLogo", PropertyNames.PROPERTY_ENABLED)) \
                    and (self.chkUseLogo.State != 0)
                self.myLetterDoc.switchElement(
                "Company Logo", logostatus)
        except IllegalArgumentException, e:
            traceback.print_exc()

    def chkUseAddressReceiverItemChanged(self):
        try:
            if self.myLetterDoc.hasElement("Sender Address Repeated"):
                rstatus = \
                    bool(self.getControlProperty(
                        "chkUseAddressReceiver",
                        PropertyNames.PROPERTY_ENABLED)) \
                    and (self.chkUseAddressReceiver.State != 0)
                self.myLetterDoc.switchElement(
                    "Sender Address Repeated", rstatus)

        except IllegalArgumentException, e:
            traceback.print_exc()

    def chkUseSignsItemChanged(self):
        if self.myLetterDoc.hasElement("Letter Signs"):
            self.myLetterDoc.switchElement(
                "Letter Signs", self.chkUseSigns.State != 0)

    def chkUseSubjectItemChanged(self):
        if self.myLetterDoc.hasElement("Subject Line"):
            self.myLetterDoc.switchElement(
                "Subject Line", self.chkUseSubject.State != 0)

    def chkUseBendMarksItemChanged(self):
        if self.myLetterDoc.hasElement("Bend Marks"):
            self.myLetterDoc.switchElement(
                "Bend Marks", self.chkUseBendMarks.State != 0)

    def chkUseFooterItemChanged(self):
        try:
            bFooterPossible = (self.chkUseFooter.State != 0) \
                and bool(self.getControlProperty(
                    "chkUseFooter", PropertyNames.PROPERTY_ENABLED))
            if self.chkFooterNextPages.State != 0:
                self.myLetterDoc.switchFooter(
                    "First Page", False, self.chkFooterPageNumbers.State != 0,
                    txtFooter.Text)
                self.myLetterDoc.switchFooter("Standard", bFooterPossible,
                    self.chkFooterPageNumbers.State != 0, self.txtFooter.Text)
            else:
                self.myLetterDoc.switchFooter(
                    "First Page", bFooterPossible,
                    self.chkFooterPageNumbers.State != 0, self.txtFooter.Text)
                self.myLetterDoc.switchFooter(
                    "Standard", bFooterPossible,
                    self.chkFooterPageNumbers.State != 0, self.txtFooter.Text)

            BPaperItem = \
                self.getRoadmapItemByID(LetterWizardDialogImpl.RM_FOOTER)
            Helper.setUnoPropertyValue(
                BPaperItem, PropertyNames.PROPERTY_ENABLED, bFooterPossible)
        except Exception, exception:
            traceback.print_exc()

    def chkFooterNextPagesItemChanged(self):
        self.chkUseFooterItemChanged()

    def chkFooterPageNumbersItemChanged(self):
        self.chkUseFooterItemChanged()

    def setPossibleFooter(self, bState):
        self.setControlProperty(
            "chkUseFooter", PropertyNames.PROPERTY_ENABLED, bState)
        self.chkUseFooterItemChanged()

    def setPossibleAddressReceiver(self, bState):
        if self.myLetterDoc.hasElement("Sender Address Repeated"):
            self.setControlProperty(
            "chkUseAddressReceiver", PropertyNames.PROPERTY_ENABLED, bState)
            self.chkUseAddressReceiverItemChanged()

    def setPossibleLogo(self, bState):
        if self.myLetterDoc.hasElement("Company Logo"):
            self.setControlProperty(
            "chkUseLogo", PropertyNames.PROPERTY_ENABLED, bState)
            self.chkUseLogoItemChanged()

    def txtFooterTextChanged(self):
        self.chkUseFooterItemChanged()

    def txtSenderNameTextChanged(self):
        myFieldHandler = TextFieldHandler(
            self.myLetterDoc.xMSF, TextDocument.xTextDocument)
        myFieldHandler.changeUserFieldContent(
            "Company", self.txtSenderName.Text)

    def txtSenderStreetTextChanged(self):
        myFieldHandler = TextFieldHandler(
            self.myLetterDoc.xMSF, TextDocument.xTextDocument)
        myFieldHandler.changeUserFieldContent(
            "Street", self.txtSenderStreet.Text)

    def txtSenderCityTextChanged(self):
        myFieldHandler = TextFieldHandler(
            self.myLetterDoc.xMSF, TextDocument.xTextDocument)
        myFieldHandler.changeUserFieldContent(
            "City", self.txtSenderCity.Text)

    def txtSenderPostCodeTextChanged(self):
        myFieldHandler = TextFieldHandler(
            self.myLetterDoc.xMSF, TextDocument.xTextDocument)
        myFieldHandler.changeUserFieldContent(
            "PostCode", self.txtSenderPostCode.Text)

    def txtSenderStateTextChanged(self):
        myFieldHandler = TextFieldHandler(
            self.myLetterDoc.xMSF, TextDocument.xTextDocument)
        myFieldHandler.changeUserFieldContent(
            PropertyNames.PROPERTY_STATE, self.txtSenderState.Text)

    def txtTemplateNameTextChanged(self):
        xDocProps = TextDocument.xTextDocument.DocumentProperties
        TitleName = self.txtTemplateName.Text
        xDocProps.Title = TitleName

    def chkUseSalutationItemChanged(self):
        self.myLetterDoc.switchUserField(
            "Salutation", self.lstSalutation.Text,
            self.chkUseSalutation.State != 0)
        self.setControlProperty(
            "lstSalutation", PropertyNames.PROPERTY_ENABLED,
            self.chkUseSalutation.State != 0)

    def lstSalutationItemChanged(self):
        self.myLetterDoc.switchUserField(
            "Salutation", self.lstSalutation.Text,
            self.chkUseSalutation.State != 0)

    def chkUseGreetingItemChanged(self):
        self.myLetterDoc.switchUserField(
            "Greeting", self.lstGreeting.Text, self.chkUseGreeting.State != 0)
        self.setControlProperty(
            "lstGreeting", PropertyNames.PROPERTY_ENABLED,
            self.chkUseGreeting.State != 0)

    def setDefaultForGreetingAndSalutation(self):
        if self.lstSalutation.Text:
            self.lstSalutation.Text = self.resources.SalutationLabels[0]

        if self.lstGreeting.Text:
            self.lstGreeting.Text = self.resources.GreetingLabels[0]

    def lstGreetingItemChanged(self):
        self.myLetterDoc.switchUserField(
            "Greeting", self.lstGreeting.Text, self.chkUseGreeting.State != 0)

    def chkBusinessPaperItemChanged(self):
        if self.chkBusinessPaper.State != 0:
            self.enableBusinessPaper()
        else:
            self.disableBusinessPaper()
            self.setPossibleSenderData(True)

    def setPossibleSenderData(self, bState):
        self.setControlProperty(
            "optSenderDefine", PropertyNames.PROPERTY_ENABLED, bState)
        self.setControlProperty(
            "optSenderPlaceholder", PropertyNames.PROPERTY_ENABLED, bState)
        self.setControlProperty(
            "lblSenderAddress", PropertyNames.PROPERTY_ENABLED, bState)
        if not bState:
            self.setControlProperty(
            "txtSenderCity", PropertyNames.PROPERTY_ENABLED, bState)
            self.setControlProperty(
            "txtSenderName", PropertyNames.PROPERTY_ENABLED, bState)
            self.setControlProperty(
            "txtSenderPostCode", PropertyNames.PROPERTY_ENABLED, bState)
            self.setControlProperty(
            "txtSenderStreet", PropertyNames.PROPERTY_ENABLED, bState)
            self.setControlProperty(
            "txtSenderCity", PropertyNames.PROPERTY_ENABLED, bState)
            self.setControlProperty(
            "txtSenderState", PropertyNames.PROPERTY_ENABLED, bState)
            self.setControlProperty(
            "lblSenderName", PropertyNames.PROPERTY_ENABLED, bState)
            self.setControlProperty(
            "lblSenderStreet", PropertyNames.PROPERTY_ENABLED, bState)
            self.setControlProperty(
            "lblPostCodeCity", PropertyNames.PROPERTY_ENABLED, bState)

    def enableSenderReceiver(self):
        BPaperItem = self.getRoadmapItemByID(
            LetterWizardDialogImpl.RM_SENDERRECEIVER)
        Helper.setUnoPropertyValue(
            BPaperItem, PropertyNames.PROPERTY_ENABLED, True)

    def disableSenderReceiver(self):
        BPaperItem = self.getRoadmapItemByID(
            LetterWizardDialogImpl.RM_SENDERRECEIVER)
        Helper.setUnoPropertyValue(
            BPaperItem, PropertyNames.PROPERTY_ENABLED, False)

    def enableBusinessPaper(self):
        try:
            BPaperItem = self.getRoadmapItemByID(
                LetterWizardDialogImpl.RM_BUSINESSPAPER)
            Helper.setUnoPropertyValue(
                BPaperItem, PropertyNames.PROPERTY_ENABLED, True)
            self.chkPaperCompanyLogoItemChanged()
            self.chkPaperCompanyAddressItemChanged()
            self.chkPaperFooterItemChanged()
            self.chkCompanyReceiverItemChanged()
        except Exception:
            traceback.print_exc()

    def disableBusinessPaper(self):
        try:
            BPaperItem = self.getRoadmapItemByID(
                LetterWizardDialogImpl.RM_BUSINESSPAPER)
            Helper.setUnoPropertyValue(
                BPaperItem, PropertyNames.PROPERTY_ENABLED, False)
            if self.BusCompanyLogo != None:
                self.BusCompanyLogo.removeFrame()

            if self.BusCompanyAddress != None:
                self.BusCompanyAddress.removeFrame()

            if self.BusFooter != None:
                self.BusFooter.removeFrame()

            if self.BusCompanyAddressReceiver != None:
                self.BusCompanyAddressReceiver.removeFrame()

            self.setPossibleAddressReceiver(True)
            self.setPossibleFooter(True)
            self.setPossibleLogo(True)
            if self.myLetterDoc.hasElement("Sender Address"):
                self.myLetterDoc.switchElement(
                "Sender Address", True)
        except Exception:
            traceback.print_exc()

    def initializeSalutation(self):
        self.setControlProperty(
            "lstSalutation", "StringItemList",
            tuple(self.resources.SalutationLabels))

    def initializeGreeting(self):
        self.setControlProperty(
            "lstGreeting", "StringItemList",
            tuple(self.resources.GreetingLabels))

    def getCurrentLetter(self):
        if self.myConfig.cp_LetterType == 0:
            return self.myConfig.cp_BusinessLetter
        elif self.myConfig.cp_LetterType == 1:
            return self.myConfig.cp_PrivateOfficialLetter
        elif self.myConfig.cp_LetterType == 2:
            return self.myConfig.cp_PrivateLetter
        else:
            return None

    def __initializePaths(self):
        try:
            self.sTemplatePath = \
                FileAccess.getOfficePath2(
                    self.xMSF, "Template", "share", "/wizard")
            self.sUserTemplatePath = \
                FileAccess.getOfficePath2(self.xMSF, "Template", "user", "")
        except NoValidPathException, e:
            traceback.print_exc()

    def initializeTemplates(self, xMSF):
        sLetterPath = FileAccess.combinePaths(
            xMSF, self.sTemplatePath, "/../common/wizard/letter")
        self.BusinessFiles = \
            FileAccess.getFolderTitles(
                xMSF, "bus", sLetterPath, self.resources.dictBusinessTemplate)
        self.OfficialFiles = \
            FileAccess.getFolderTitles(
                xMSF, "off", sLetterPath, self.resources.dictOfficialTemplate)
        self.PrivateFiles = \
            FileAccess.getFolderTitles(
                xMSF, "pri", sLetterPath, self.resources.dictPrivateTemplate)
        self.setControlProperty(
            "lstBusinessStyle", "StringItemList",
            tuple(self.BusinessFiles.keys()))
        self.setControlProperty(
            "lstPrivOfficialStyle", "StringItemList",
            tuple(self.OfficialFiles.keys()))
        self.setControlProperty(
            "lstPrivateStyle", "StringItemList",
            tuple(self.PrivateFiles.keys()))
        self.setControlProperty(
            "lstBusinessStyle", "SelectedItems", (0,))
        self.setControlProperty(
            "lstPrivOfficialStyle", "SelectedItems", (0,))
        self.setControlProperty(
            "lstPrivateStyle", "SelectedItems", (0,))
        return True

    def initializeElements(self):
        self.setControlProperty(
            "chkUseLogo", PropertyNames.PROPERTY_ENABLED,
            self.myLetterDoc.hasElement("Company Logo"))
        self.setControlProperty(
            "chkUseBendMarks", PropertyNames.PROPERTY_ENABLED,
            self.myLetterDoc.hasElement("Bend Marks"))
        self.setControlProperty(
            "chkUseAddressReceiver", PropertyNames.PROPERTY_ENABLED,
            self.myLetterDoc.hasElement("Sender Address Repeated"))
        self.setControlProperty(
            "chkUseSubject", PropertyNames.PROPERTY_ENABLED,
            self.myLetterDoc.hasElement("Subject Line"))
        self.setControlProperty(
            "chkUseSigns", PropertyNames.PROPERTY_ENABLED,
            self.myLetterDoc.hasElement("Letter Signs"))
        self.myLetterDoc.updateDateFields()

    def setConfiguration(self):
        if self.optBusinessLetter.State:
            self.optBusinessLetterItemChanged()

        elif self.optPrivOfficialLetter.State:
            self.optPrivOfficialLetterItemChanged()

        elif self.optPrivateLetter.State:
            self.optPrivateLetterItemChanged()

    def setElements(self, privLetter):
        if self.optSenderDefine.State:
            self.optSenderDefineItemChanged()

        if self.optSenderPlaceholder.State:
            self.optSenderPlaceholderItemChanged()

        self.chkUseSignsItemChanged()
        self.chkUseSubjectItemChanged()
        self.chkUseSalutationItemChanged()
        self.chkUseGreetingItemChanged()
        self.chkUseBendMarksItemChanged()
        self.chkUseAddressReceiverItemChanged()
        self.txtTemplateNameTextChanged()
        if self.optReceiverDatabase.State and not privLetter:
            self.optReceiverDatabaseItemChanged()

        if self.optReceiverPlaceholder.State and not privLetter:
            self.optReceiverPlaceholderItemChanged()

        if self.optCreateLetter.State:
            self.optCreateFromTemplateItemChanged()

        if self.optMakeChanges.State:
            self.optMakeChangesItemChanged()

    def insertRoadmap(self):
        self.addRoadmap()

        self.insertRoadMapItems(
                [True, False, True, True, False, True],
                self.resources.RoadmapLabels)

        self.setRoadmapInteractive(True)
        self.setRoadmapComplete(True)
        self.setCurrentRoadmapItemID(1)

    def insertPathSelectionControl(self):
        self.myPathSelection = \
            PathSelection(self.xMSF, self, PathSelection.TransferMode.SAVE,
                PathSelection.DialogTypes.FILE)
        self.myPathSelection.insert(
            6, 97, 70, 205, 45, self.resources.reslblTemplatePath_value,
            True, HelpIds.getHelpIdString(HID + 47),
            HelpIds.getHelpIdString(HID + 48))
        self.myPathSelection.sDefaultDirectory = self.sUserTemplatePath
        self.myPathSelection.sDefaultName = "myLetterTemplate.ott"
        self.myPathSelection.sDefaultFilter = "writer8_template"
        self.myPathSelection.addSelectionListener(self)

    def initConfiguration(self):
        try:
            root = Configuration.getConfigurationRoot(
                self.xMSF, "/org.openoffice.Office.Writer/Wizards/Letter",
                False)
            self.myConfig.readConfiguration(root, "cp_")
            RadioDataAware.attachRadioButtons(self.myConfig, "cp_LetterType",
                (self.optBusinessLetter, self.optPrivOfficialLetter,
                    self.optPrivateLetter), True).updateUI()
            UnoDataAware.attachListBox(
                self.myConfig.cp_BusinessLetter, "cp_Style",
                self.lstBusinessStyle, True).updateUI()
            UnoDataAware.attachListBox(
                self.myConfig.cp_PrivateOfficialLetter, "cp_Style",
                self.lstPrivOfficialStyle, True).updateUI()
            UnoDataAware.attachListBox(
                self.myConfig.cp_PrivateLetter, "cp_Style",
                self.lstPrivateStyle, True).updateUI()
            UnoDataAware.attachCheckBox(
                self.myConfig.cp_BusinessLetter, "cp_BusinessPaper",
                self.chkBusinessPaper, True).updateUI()
            cgl = self.myConfig.cp_BusinessLetter
            cgpl = self.myConfig.cp_BusinessLetter.cp_CompanyLogo
            cgpa = self.myConfig.cp_BusinessLetter.cp_CompanyAddress
            UnoDataAware.attachCheckBox(
                cgpl, "cp_Display", self.chkPaperCompanyLogo, True).updateUI()
            UnoDataAware.attachNumericControl(
                cgpl, "cp_Width", self.numLogoWidth, True).updateUI()
            UnoDataAware.attachNumericControl(
                cgpl, "cp_Height", self.numLogoHeight, True).updateUI()
            UnoDataAware.attachNumericControl(
                cgpl, "cp_X", self.numLogoX, True).updateUI()
            UnoDataAware.attachNumericControl(
                cgpl, "cp_Y", self.numLogoY, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgpa, "cp_Display", self.chkPaperCompanyAddress, True).updateUI()
            UnoDataAware.attachNumericControl(
                cgpa, "cp_Width", self.numAddressWidth, True).updateUI()
            UnoDataAware.attachNumericControl(
                cgpa, "cp_Height", self.numAddressHeight, True).updateUI()
            UnoDataAware.attachNumericControl(
                cgpa, "cp_X", self.numAddressX, True).updateUI()
            UnoDataAware.attachNumericControl(
                cgpa, "cp_Y", self.numAddressY, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_PaperCompanyAddressReceiverField",
                self.chkCompanyReceiver, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_PaperFooter", self.chkPaperFooter, True).updateUI()
            UnoDataAware.attachNumericControl(
                cgl, "cp_PaperFooterHeight", self.numFooterHeight, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_PrintCompanyLogo", self.chkUseLogo, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_PrintCompanyAddressReceiverField",
                self.chkUseAddressReceiver, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_PrintLetterSigns", self.chkUseSigns, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_PrintSubjectLine", self.chkUseSubject, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_PrintSalutation", self.chkUseSalutation, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_PrintBendMarks", self.chkUseBendMarks, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_PrintGreeting", self.chkUseGreeting, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_PrintFooter", self.chkUseFooter, True).updateUI()
            UnoDataAware.attachEditControl(
                cgl, "cp_Salutation", self.lstSalutation, True).updateUI()
            UnoDataAware.attachEditControl(
                cgl, "cp_Greeting", self.lstGreeting, True).updateUI()
            RadioDataAware.attachRadioButtons(
                cgl, "cp_SenderAddressType",
                (self.optSenderDefine, self.optSenderPlaceholder), True).updateUI()
            UnoDataAware.attachEditControl(
                cgl, "cp_SenderCompanyName", self.txtSenderName, True).updateUI()
            UnoDataAware.attachEditControl(
                cgl, "cp_SenderStreet", self.txtSenderStreet, True).updateUI()
            UnoDataAware.attachEditControl(
                cgl, "cp_SenderPostCode", self.txtSenderPostCode, True).updateUI()
            UnoDataAware.attachEditControl(
                cgl, "cp_SenderState", self.txtSenderState, True).updateUI()
            UnoDataAware.attachEditControl(
                cgl, "cp_SenderCity", self.txtSenderCity, True).updateUI()
            RadioDataAware.attachRadioButtons(
                cgl, "cp_ReceiverAddressType",
                (self.optReceiverDatabase, self.optReceiverPlaceholder),
                True).updateUI()
            UnoDataAware.attachEditControl(
                cgl, "cp_Footer", self.txtFooter, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_FooterOnlySecondPage",
                self.chkFooterNextPages, True).updateUI()
            UnoDataAware.attachCheckBox(
                cgl, "cp_FooterPageNumbers",
                self.chkFooterPageNumbers, True).updateUI()
            RadioDataAware.attachRadioButtons(
                cgl, "cp_CreationType",
                (self.optCreateLetter, self.optMakeChanges), True).updateUI()
            UnoDataAware.attachEditControl(
                cgl, "cp_TemplateName", self.txtTemplateName, True).updateUI()
            UnoDataAware.attachEditControl(
                cgl, "cp_TemplatePath", self.myPathSelection.xSaveTextBox,
                True).updateUI()
        except Exception, exception:
            traceback.print_exc()

    def saveConfiguration(self):
        try:
            root = Configuration.getConfigurationRoot(self.xMSF,
                "/org.openoffice.Office.Writer/Wizards/Letter", True)
            self.myConfig.writeConfiguration(root, "cp_")
            root.commitChanges()
        except Exception, e:
            traceback.print_exc()

    def validatePath(self):
        if self.myPathSelection.usedPathPicker:
                self.filenameChanged = True
        self.myPathSelection.usedPathPicker = False
