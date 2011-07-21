from AgendaWizardDialog import *
from common.Configuration import *
from CGAgenda import CGAgenda
from AgendaTemplate import *
from common.NoValidPathException import *
from common.FileAccess import *
from ui.PathSelection import *
from ui.event.UnoDataAware import *
from ui.event.RadioDataAware import *

from com.sun.star.awt.VclWindowPeerAttribute import OK
from common.NoValidPathException import *

class AgendaWizardDialogImpl(AgendaWizardDialog):

    fileAccess1 = None

    def __init__(self, xmsf):
        super(AgendaWizardDialogImpl, self).__init__(xmsf)
        self.filenameChanged = False

    def enterStep(self, OldStep, NewStep):
        pass

    def leaveStep(self, OldStep, NewStep):
        pass

    '''
    used in developement to start the wizard
    '''

    @classmethod
    def main(self, args):
        ConnectStr = \
            "uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext"
        try:
            xLocMSF = Desktop.connect(ConnectStr)
            wizard = AgendaWizardDialogImpl(xLocMSF)
            wizard.startWizard()
        except Exception, exception:
            traceback.print_exc()

    '''
    read the configuration data, open the specified template,
    initialize the template controller (AgendaTemplate) and
    set the status of the displayed template to the one
    read from the configuration.
    build the dialog.
    Synchronize the dialog to the same status (read from
    the configuration).
    show the dialog.
    '''

    def startWizard(self):
        self.running = True
        try:
            # read configuration data.
            self.agenda = CGAgenda()
            root = Configuration.getConfigurationRoot(
                self.xMSF, "/org.openoffice.Office.Writer/Wizards/Agenda",
                False)
            self.agenda.readConfiguration(root, "cp_")
            # initialize the agenda temself.myPathSelectionListener()plate
            self.agendaTemplate = AgendaTemplate(
                self.xMSF, self.agenda, self.resources, self)
            self.initializeTemplates()
            self.agendaTemplate.load(self.agendaTemplates[1][0], [])
            # build the dialog.
            self.buildStep1()
            self.buildStep2()
            self.buildStep3()
            self.buildStep4()
            self.buildStep5()
            self.topicsControl = TopicsControl(self, self.xMSF, self.agenda)
            self.buildStep6()
            self.drawNaviBar()
            self.initializePaths()
            #special Control for setting the save Path:
            self.insertPathSelectionControl()
            # create the peer
            xw = self.agendaTemplate.xFrame.getContainerWindow()
            self.createWindowPeer(xw)
            # initialize roadmap
            self.addRoadmap()
            self.insertRoadMapItems(
                [self.resources.resStep1, self.resources.resStep2,
                    self.resources.resStep3, self.resources.resStep4,
                    self.resources.resStep5, self.resources.resStep6],
                [1, 2, 3, 4, 5, 6],[True, True, True, True, True, True])
            self.nMaxStep = 6
            self.setCurrentRoadmapItemID(1)
            # synchronize GUI and CGAgenda object.
            self.makeDA()
            if self.myPathSelection.xSaveTextBox.Text.lower() == "":
                self.myPathSelection.initializePath()

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
        self.myPathSelection.addSelectionListener(
            self.myPathSelectionListener())

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
                    FileAccess.getOfficePath(xMSF, "Work", "", ""),
                    resources.resDefaultFilename)
            except Exception, ex:
                traceback.print_exc()

    '''
    bind controls to the agenda member (DataAware model)
    '''

    def makeDA(self):
        self.setControlProperty(
            "listPageDesign", "StringItemList", tuple(self.agendaTemplates[0]))
        self.checkSavePath()
        #setFilename(agenda.cp_TemplatePath);
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
            self.agenda, "cp_ShowMeetingType", self.chkMeetingTitle, True)
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowRead", self.chkRead, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowBring", self.chkBring, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowNotes", self.chkNotes, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowCalledBy", self.chkConvenedBy, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowFacilitator", self.chkPresiding, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowNotetaker", self.chkNoteTaker, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowTimekeeper", self.chkTimekeeper, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowAttendees", self.chkAttendees, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowObservers", self.chkObservers, True).updateUI()
        UnoDataAware.attachCheckBox(
            self.agenda, "cp_ShowResourcePersons",self.chkResourcePersons, True).updateUI()
        UnoDataAware.attachEditControl(
            self.agenda, "cp_TemplateName", self.txtTemplateName, True).updateUI()
        RadioDataAware.attachRadioButtons(
            self.agenda, "cp_ProceedMethod",
                (self.optCreateAgenda, self.optMakeChanges), True).updateUI()

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

    def pageDesignChanged(self, item):
        try:
            self.agendaTemplate.load(
                self.agendaTemplates[1][item.Selected],
                self.topicsControl.scrollfields)
        except Exception:
            SystemDialog.showMessageBox(
                self.xMSF, "ErrBox", OK, self.resources.resErrOpenTemplate)
            traceback.print_exc()

    '''
    last page, template title changed...
    '''

    def templateTitleChanged(self):
        title = Helper.getUnoPropertyValue(getModel(txtTemplateName), "Text")
        self.agendaTemplate.setTemplateTitle(title)

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
        bSaveSuccess = False
        # pesimistic :(
        try:
            fileAccess = FileAccess.FileAccess_unknown(xMSF)
            self.sPath = self.myPathSelection.getSelectedPath()
            if self.sPath.equals(""):
                self.myPathSelection.triggerPathPicker()
                self.sPath = self.myPathSelection.getSelectedPath()

            self.sPath = fileAccess.getURL(self.sPath)
            #first, if the filename was not changed, thus
            #it is coming from a saved session, check if the
            # file exists and warn the user.
            if not self.filenameChanged:
                if fileAccess.exists(self.sPath, True):
                    answer = SystemDialog.showMessageBox(
                        xMSF, xControl.Peer, "MessBox",
                        VclWindowPeerAttribute.YES_NO + \
                            VclWindowPeerAttribute.DEF_NO,
                        resources.resFileExists)
                    if (answer == 3):
                    # user said: no, do not overwrite....
                        return False

            self.agendaTemplate.xTextDocument.lockControllers()
            xTextDocument = self.agendaTemplate.document
            bSaveSuccess = OfficeDocument.store(
                xMSF, xTextDocument, self.sPath, "writer8_template", False)
        except Exception, e:
            SystemDialog.showMessageBox(
                xMSF, xControl.Peer, "ErrBox", VclWindowPeerAttribute.OK,
                resources.resErrSaveTemplate)

        if bSaveSuccess:
            try:
                self.topicsControl.saveTopics(self.agenda)
                root = Configuration.getConfigurationRoot(
                    xMSF, "/org.openoffice.Office.Writer/Wizards/Agenda", True)
                self.agenda.writeConfiguration(root, "cp_")
                Configuration.commit(root)

                self.agendaTemplate.finish(self.topicsControl.getTopicsData())
                xStoreable = self.agendaTemplate.document
                xStoreable.store()

                self.agendaTemplate.xTextDocument.unlockControllers()
                loadValues = range(2)
                loadValues[0] = PropertyValue.PropertyValue()
                loadValues[0].Name = "AsTemplate"
                if self.agenda.cp_ProceedMethod == 1:
                    loadValues[0].Value = Boolean.TRUE
                else:
                    loadValues[0].Value = Boolean.FALSE

                loadValues[1] = PropertyValue.PropertyValue()
                loadValues[1].Name = "InteractionHandler"

                xIH = xMSF.createInstance(
                    "com.sun.star.comp.uui.UUIInteractionHandler")
                loadValues[1].Value = xIH

                oDoc = OfficeDocument.load(
                    Desktop.getDesktop(self.xMSF),
                    self.sPath, "_default", loadValues)
                myViewHandler = ViewHandler(self.xMSF, oDoc)
                myViewHandler.setViewSetting("ZoomType", OPTIMAL)
            except Exception, ex:
                traceback.print_exc()

        else:
            self.agendaTemplate.xTextDocument.unlockControllers()
            return False

        self.xUnoDialog.endExecute()
        self.running = False
        return True

    def closeDocument(self):
        try:
            xCloseable = self.agendaTemplate.xFrame
            xCloseable.close(False)
        except CloseVetoException, e:
            traceback.print_exc()
