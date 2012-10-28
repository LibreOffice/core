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
from .AgendaWizardDialog import AgendaWizardDialog
from .AgendaWizardDialogConst import HID
from .AgendaTemplate import AgendaTemplate, TopicsControl, FileAccess
from CGAgenda import CGAgenda
from ..ui.PathSelection import PathSelection
from ..ui.event.UnoDataAware import UnoDataAware
from ..ui.event.RadioDataAware import RadioDataAware
from ..common.NoValidPathException import NoValidPathException
from ..common.SystemDialog import SystemDialog
from ..common.Desktop import Desktop
from ..common.HelpIds import HelpIds
from ..common.Configuration import Configuration

from com.sun.star.view.DocumentZoomType import OPTIMAL
from com.sun.star.awt.VclWindowPeerAttribute import YES_NO, DEF_NO
from com.sun.star.awt.VclWindowPeerAttribute import OK

class AgendaWizardDialogImpl(AgendaWizardDialog):

    fileAccess1 = None
    pageDesign = None

    def __init__(self, xmsf):
        super(AgendaWizardDialogImpl, self).__init__(xmsf)
        self.filenameChanged = False

    def enterStep(self, OldStep, NewStep):
        pass

    def leaveStep(self, OldStep, NewStep):
        pass


    @classmethod
    def main(self):
        #Call the wizard remotely(see README)
        try:
            ConnectStr = \
                "uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext"
            xLocMSF = Desktop.connect(ConnectStr)
            lw = AgendaWizardDialogImpl(xLocMSF)
            lw.startWizard()
        except Exception as e:
            print ("Wizard failure exception " + str(type(e)) +
                   " message " + str(e) + " args " + str(e.args) +
                   traceback.format_exc())

    def startWizard(self):
        self.running = True
        try:
            #Number of steps on WizardDialog
            self.nMaxStep = 6

            # initialize the agenda template
            self.agenda = CGAgenda()
            self.agendaTemplate = AgendaTemplate(
                self.xMSF, self.agenda, self.resources, self)

            # build the dialog.
            self.drawNaviBar()

            self.buildStep1()
            self.buildStep2()
            self.buildStep3()
            self.buildStep4()
            self.buildStep5()
            self.buildStep6()

            self.topicsControl = TopicsControl(self, self.xMSF, self.agenda)

            self.initializePaths()
            #special Control for setting the save Path:
            self.insertPathSelectionControl()

            self.initializeTemplates()

            # synchronize GUI and CGAgenda object.
            self.initConfiguration()

            if self.myPathSelection.xSaveTextBox.Text.lower() == "":
                self.myPathSelection.initializePath()

            # create the peer
            xContainerWindow = self.agendaTemplate.xFrame.ContainerWindow
            self.createWindowPeer(xContainerWindow)

            # initialize roadmap
            self.insertRoadmap()

            self.pageDesignChanged()

            self.executeDialogFromComponent(self.agendaTemplate.xFrame)
            self.removeTerminateListener()
            self.closeDocument()
            self.running = False
        except Exception, ex:
            self.removeTerminateListener()
            traceback.print_exc()
            self.running = False
            return

    def insertPathSelectionControl(self):
        self.myPathSelection = PathSelection(
            self.xMSF, self, PathSelection.TransferMode.SAVE,
            PathSelection.DialogTypes.FILE)
        self.myPathSelection.insert(6, 97, 70, 205, 45,
            self.resources.reslblTemplatePath_value, True,
            HelpIds.getHelpIdString(HID + 24),
            HelpIds.getHelpIdString(HID + 25))
        self.myPathSelection.sDefaultDirectory = self.sUserTemplatePath
        self.myPathSelection.sDefaultName = "myAgendaTemplate.ott"
        self.myPathSelection.sDefaultFilter = "writer8_template"
        self.myPathSelection.addSelectionListener(self)

    def initializePaths(self):
        try:
            self.sTemplatePath = FileAccess.getOfficePath2(
                self.xMSF, "Template", "share", "/wizard")
            self.sUserTemplatePath = FileAccess.getOfficePath2(
                self.xMSF, "Template", "user", "")
            self.sBitmapPath = FileAccess.combinePaths(
                self.xMSF, self.sTemplatePath, "/../wizard/bitmap")
        except NoValidPathException:
            traceback.print_exc()

    def checkSavePath(self):
        if self.agenda.cp_TemplatePath is None \
            or self.agenda.cp_TemplatePath == "" \
            or not self.getFileAccess().exists(
                FileAccess.getParentDir(self.agenda.cp_TemplatePath), False) \
            or not self.getFileAccess().isDirectory(
                FileAccess.getParentDir(self.agenda.cp_TemplatePath)):
            try:
                self.agenda.cp_TemplatePath = FileAccess.connectURLs(
                    FileAccess.getOfficePath2(self.xMSF, "Work", "", ""),
                    self.resources.resDefaultFilename)
            except Exception, ex:
                traceback.print_exc()

    '''
    bind controls to the agenda member (DataAware model)
    '''

    def initConfiguration(self):
        # read configuration data.
        root = Configuration.getConfigurationRoot(
            self.xMSF, "/org.openoffice.Office.Writer/Wizards/Agenda", False)
        self.agenda.readConfiguration(root, "cp_")

        self.setControlProperty(
            "listPageDesign", "StringItemList",
            tuple(self.agendaTemplates.keys()))
        self.checkSavePath()
        UnoDataAware.attachListBox(
            self.agenda, "cp_AgendaType", self.listPageDesign, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_IncludeMinutes", self.chkMinutes, True).updateUI()
        UnoDataAware.attachEditControl(
            self.agenda, "cp_Title", self.txtTitle, True).updateUI()
        UnoDataAware.attachDateControl(
            self.agenda, "cp_Date", self.txtDate, True).updateUI()
        UnoDataAware.attachTimeControl(
            self.agenda, "cp_Time", self.txtTime, True).updateUI()
        UnoDataAware.attachEditControl(
            self.agenda, "cp_Location", self.cbLocation, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowMeetingType", self.chkMeetingTitle,
            True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowRead", self.chkRead, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowBring", self.chkBring, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowNotes", self.chkNotes, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowCalledBy", self.chkConvenedBy,
            True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowFacilitator", self.chkPresiding,
            True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowNotetaker", self.chkNoteTaker,
            True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowTimekeeper", self.chkTimekeeper,
            True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowAttendees", self.chkAttendees,
            True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowObservers", self.chkObservers,
            True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowResourcePersons",self.chkResourcePersons,
            True).updateUI()
        UnoDataAware.attachEditControl(
            self.agenda, "cp_TemplateName", self.txtTemplateName,
            True).updateUI()
        RadioDataAware.attachRadioButtons(
            self.agenda, "cp_ProceedMethod",
                (self.optCreateAgenda, self.optMakeChanges), True).updateUI()

    def saveConfiguration(self):
        root = Configuration.getConfigurationRoot(
            self.xMSF, "/org.openoffice.Office.Writer/Wizards/Agenda", True)
        self.agenda.writeConfiguration(root, "cp_")
        root.commitChanges()

    def insertRoadmap(self):
        self.addRoadmap()

        self.insertRoadMapItems(
            [True, True, True, True, True, True],
            [self.resources.resStep1, self.resources.resStep2,
                self.resources.resStep3, self.resources.resStep4,
                self.resources.resStep5, self.resources.resStep6])

        self.setRoadmapInteractive(True)
        self.setRoadmapComplete(True)
        self.setCurrentRoadmapItemID(1)

    '''
    read the available agenda wizard templates.
    '''

    def initializeTemplates(self):
        try:
            self.sTemplatePath = FileAccess.getOfficePath2(
                self.xMSF, "Template", "share", "/wizard")
            sAgendaPath = FileAccess.combinePaths(
                self.xMSF, self.sTemplatePath, "/wizard/agenda")
            self.agendaTemplates = FileAccess.getFolderTitles(
                self.xMSF, "aw", sAgendaPath)
            return True
        except NoValidPathException:
            traceback.print_exc()
            return False

    '''
    first page, page design listbox changed.
    '''

    def pageDesignChanged(self):
        try:
            SelectedItemPos = self.listPageDesign.SelectedItemPos
            #avoid to load the same item again
            if AgendaWizardDialogImpl.pageDesign is not SelectedItemPos:
                AgendaWizardDialogImpl.pageDesign = SelectedItemPos
                self.agendaTemplate.load(
                    self.agendaTemplates.values()[SelectedItemPos],
                    self.topicsControl.scrollfields)
        except Exception:
            traceback.print_exc()

    '''
    last page, template title changed...
    '''

    def templateTitleChanged(self):
        title = Helper.getUnoPropertyValue(getModel(txtTemplateName), "Text")
        self.agendaTemplate.setTemplateTitle(title)

    #textFields listeners
    def txtTitleTextChanged(self):
        AgendaTemplate.redrawTitle("txtTitle")

    def txtDateTextChanged(self):
        AgendaTemplate.redrawTitle("txtDate")

    def txtTimeTextChanged(self):
        AgendaTemplate.redrawTitle("txtTime")

    def txtLocationTextChanged(self):
        AgendaTemplate.redrawTitle("cbLocation")

    #checkbox listeners
    def chkUseMeetingTypeItemChanged(self):
        AgendaTemplate.agenda.cp_IncludeMinutes = bool(self.chkMinutes.State)

    def chkUseMeetingTypeItemChanged(self):
        AgendaTemplate.redraw(FILLIN_MEETING_TYPE)

    def chkUseReadItemChanged(self):
        AgendaTemplate.redraw(FILLIN_READ)

    def chkUseBringItemChanged(self):
        AgendaTemplate.redraw(FILLIN_BRING)

    def chkUseNotesItemChanged(self):
        AgendaTemplate.redraw(FILLIN_NOTES)

    def chkUseCalledByItemChanged(self):
        AgendaTemplate.redraw(FILLIN_CALLED_BY)

    def chkUseFacilitatorItemChanged(self):
        AgendaTemplate.redraw(FILLIN_FACILITATOR)

    def chkUseNoteTakerItemChanged(self):
        AgendaTemplate.redraw(FILLIN_NOTETAKER)

    def chkUseTimeKeeperItemChanged(self):
        AgendaTemplate.redraw(FILLIN_TIMEKEEPER)

    def chkUseAttendeesItemChanged(self):
        AgendaTemplate.redraw(FILLIN_PARTICIPANTS)

    def chkUseObserversItemChanged(self):
        AgendaTemplate.redraw(FILLIN_OBSERVERS)

    def chkUseResourcePersonsItemChanged(self):
        AgendaTemplate.redraw(FILLIN_RESOURCE_PERSONS)

    '''
    convenience method.
    instead of creating a FileAccess object every time
    it is needed, I have a FileAccess object memeber.
    the first time it is needed it will be created, and
    then be reused...
    @return the FileAccess memeber object.
    '''

    def getFileAccess(self):
        if AgendaWizardDialogImpl.fileAccess1 is None:
            try:
                AgendaWizardDialogImpl.fileAccess1 = FileAccess(self.xMSF)
            except Exception, e:
                traceback.print_exc()
        return AgendaWizardDialogImpl.fileAccess1

    '''
    last page, "browse" ("...") button was clicked...
    '''

    def saveAs(self):
        try:
            checkSavePath()
            saveAs = SystemDialog.createStoreDialog(xMSF)
            saveAs.addFilterToDialog("ott", "writer8_template", True)
            # call the saveAs dialog.
            url = saveAs.callStoreDialog(
                FileAccess.getParentDir(self.agenda.cp_TemplatePath),
                FileAccess.getFilename(self.agenda.cp_TemplatePath))
            if url != None:
                self.agenda.cp_TemplatePath = url
                setFilename(url)
                self.filenameChanged = True

        except Exception, ex:
            traceback.print_exc()

    '''
    is called when the user
    changes the path through the "save as" dialog.
    The path displayed is a translated, user-friendly, platform dependant path.
    @param url the new save url.
    '''

    def setFilename(self, url):
        try:
            path = getFileAccess().getPath(url, "")
            Helper.setUnoPropertyValue(
                getModel(self.myPathSelection.xSaveTextBox), "Text", path)
        except Exception, ex:
            traceback.print_exc()

    def insertRow(self):
        self.topicsControl.insertRow()

    def removeRow(self):
        self.topicsControl.removeRow()

    def rowUp(self):
        self.topicsControl.rowUp()

    def rowDown(self):
        self.topicsControl.rowDown()

    def cancelWizard(self):
        self.xUnoDialog.endExecute()
        self.running = False

    def finishWizard(self):
        self.switchToStep(self.getCurrentStep(), self.nMaxStep)
        bSaveSuccess = False
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
                        # user said: no, do not overwrite
                        endWizard = False
                        return False

            xTextDocument = self.agendaTemplate.document
            bSaveSuccess = OfficeDocument.store(
                self.xMSF, AgendaTemplate.xTextDocument, self.sPath,
                "writer8_template")

            if bSaveSuccess:
                self.saveConfiguration()

                self.agendaTemplate.finish(self.topicsControl.scrollfields)

                loadValues = range(2)
                loadValues[0] = uno.createUnoStruct( \
                    'com.sun.star.beans.PropertyValue')
                loadValues[0].Name = "AsTemplate"
                if self.agenda.cp_ProceedMethod == 1:
                    loadValues[0].Value = True
                else:
                    loadValues[0].Value = False

                loadValues[1] = uno.createUnoStruct( \
                    'com.sun.star.beans.PropertyValue')
                loadValues[1].Name = "InteractionHandler"

                xIH = self.xMSF.createInstance(
                    "com.sun.star.comp.uui.UUIInteractionHandler")
                loadValues[1].Value = xIH

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
        return True

    def closeDocument(self):
        try:
            xCloseable = self.agendaTemplate.xFrame
            xCloseable.close(False)
        except CloseVetoException, e:
            traceback.print_exc()
