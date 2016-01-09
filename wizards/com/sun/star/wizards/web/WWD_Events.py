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

from .WWD_Startup import WWD_Startup

from .WebWizardConst import *
from ..common.FileAccess import FileAccess
from ..common.Configuration import Configuration
from ..ui.event.ListModelBinder import ListModelBinder
from ..ui.event.Task import Task
from .data.CGDocument import CGDocument
from .data.CGSession import CGSession
from .ProcessStatusRenderer import ProcessStatusRenderer
from .FTPDialog import FTPDialog
from .ErrorHandler import ErrorHandler
from .AbstractErrorHandler import AbstractErrorHandler
from .ProcessErrorHandler import ProcessErrorHandler
from .Process import Process
from .BackgroundsDialog import BackgroundsDialog
from .IconsDialog import IconsDialog
from .TOCPreview import TOCPreview

from com.sun.star.container import NoSuchElementException

'''
This class implements the ui-events of the
web wizard.
it is therefore sorted to steps.
not much application-logic here - just plain
methods which react to events.
The only exception are the finish methods with the save
session methods.
'''

class WWD_Events(WWD_Startup):

    '''
    He - my constructor !
    I add a window listener, which, when
    the window closes, deletes the temp directory.
    '''

    def __init__(self, xmsf):
        super(WWD_Events, self).__init__(xmsf)
        self.currentSession = ""
        self.exitOnCreate = True
        self.count = 0
        self.bgDialog = None
        self.iconsDialog = None
        self.docPreview = None

    def leaveStep(self, nOldStep, nNewStep):
        pass

    def enterStep(self, old, newStep):
        if old == 1:
            sessionToLoad = ""
            s = self.lstLoadSettings.Model.SelectedItems
            if len(s) == 0 or s[0] == 0:
                sessionToLoad = ""
            else:
                sessionToLoad = \
                    self.settings.cp_SavedSessions.getElementAt(s[0]).cp_Name

            if sessionToLoad is not self.currentSession:
                self.loadSession(sessionToLoad)

    '''
    **************
        STEP 1
    **************
    '''

    '''
    Called from the Uno event dispatcher when the
    user selects a saved session.
    '''
    def sessionSelected(self):
        s = self.getModel(self.lstLoadSettings).SelectedItems
        self.setEnabled(self.btnDelSession, len(s) > 0 and s[0] > 0)

    '''
    Ha ! the session should be loaded :-)
    '''

    def loadSession(self, sessionToLoad):
        try:
            sd = self.getStatusDialog()
            task = Task("LoadDocs", "", 10)
            sd.execute(self, task, self.resources.resLoadingSession)
            task.start()
            self.setSelectedDoc([])
            self.lstDocuments.Model.SelectedItems = tuple([])
            self.lstDocuments.Model.StringItemList = tuple([])
            if sessionToLoad == "":
                view = Configuration.getConfigurationRoot(
                    self.xMSF, CONFIG_PATH + "/DefaultSession", False)
            else:
                view = Configuration.getConfigurationRoot(
                    self.xMSF, CONFIG_PATH + "/SavedSessions", False)
                view = Configuration.getNode(sessionToLoad, view)

            session = CGSession()
            session.setRoot(self.settings)
            session.readConfiguration(view, CONFIG_READ_PARAM)
            numDocs = session.cp_Content.cp_Documents.getSize()
            task.setMax(numDocs * 5 + 7)
            task.advance(True)
            if sessionToLoad == "":
                self.setSaveSessionName(session)

            self.mount(session, task, False, sd.xUnoDialog)
            self.checkSteps()
            self.currentSession = sessionToLoad
            while task.getStatus() <= task.getMax():
                task.advance(False)
            task.removeTaskListener(sd)
        except Exception as ex:
            self.unexpectedError(ex)

        try:
            self.refreshStylePreview()
            self.updateIconsetText()
        except Exception:
           traceback.print_exc()

    '''
    hmm. the user clicked the delete button.
    '''

    def delSession(self):
        selected = self.lstLoadSettings.Model.SelectedItems
        if selected.length == 0:
            return

        if selected[0] == 0:
            return

        confirm = AbstractErrorHandler.showMessage1(
            self.xMSF, self.xUnoDialog.Peer, self.resources.resDelSessionConfirm,
            ErrorHandler.ERROR_QUESTION_NO)
        if confirm:
            try:
                name = self.settings.cp_SavedSessions.getKey(selected[0])
                # first delete the session from the registry/configuration.
                Configuration.removeNode(
                    self.xMSF, CONFIG_PATH + "/SavedSessions", name)
                # then delete WWD_Startup.settings.cp_SavedSessions
                self.settings.cp_SavedSessions.remove(selected[0])
                self.settings.savedSessions.remove(selected[0] - 1)
                nextSelected = [0]
                # We try to select the same item index again, if possible
                if self.settings.cp_SavedSessions.getSize() > selected[0]:
                    nextSelected[0] = selected[0]
                else:
                    # this will always be available because
                    # the user can not remove item 0.
                    nextSelected[0] = (short)(selected[0] - 1)
                    # if the <none> session will
                    # be selected, disable the remove button...
                    if nextSelected[0] == 0:
                        self.btnDelSession.Model.Enabled = False
                        # select...

                    self.lstLoadSettings.Model.SelectedItems = nextSelected

            except Exception as ex:
                traceback.print_exc()
                self.unexpectedError(ex)

    '''
    **************
        STEP 2
    **************
    '''

    '''
    A method used by the UnoDataAware attached
    to the Documents listbox.
    See the concept of the DataAware objects to undestand
    why it is there...
    '''
    def getSelectedDoc(self):
        return self.selectedDoc

    '''
    when the user clicks another document
    in the listbox, this method is called,
    and causes the display in
    the textboxes title, description, author and export format
    to change
    '''

    def setSelectedDoc(self, s):
        oldDoc = self.getDoc(self.selectedDoc)
        doc = self.getDoc(s)
        if doc is None:
            self.fillExportList([])
            #I try to avoid refreshing the export list if
            #the same type of document is chosen.
        elif oldDoc is None or oldDoc.appType != doc.appType:
            self.fillExportList(self.settings.getExporters(doc.appType))

        self.selectedDoc = list(s)
        if (doc is not None):
            self.mountList(doc, self.docAware)
            self.disableDocUpDown()

    '''
    The user clicks the "Add" button.
    This will open a "FileOpen" dialog,
    and, if the user chooses more than one file,
    will open a status dialog, when validating each document.
    '''

    def addDocument(self):
        try:
            files = self.getDocAddDialog().callOpenDialog(
                True, self.settings.cp_DefaultSession.cp_InDirectory)
            if files is None:
                return

            task = Task("", "", len(files) * 5)
            '''
            If more than a certain number
            of documents have been added,
            open the status dialog.
            '''
            if (len(files) > MIN_ADD_FILES_FOR_DIALOG):
                sd = self.getStatusDialog()
                sd.setLabel(self.resources.resValidatingDocuments)
                sd.execute(self, task, self.resources.prodName)
                oLoadDocs = self.LoadDocs(self.xMSF, self.xUnoDialog, files, task, self)
                oLoadDocs.loadDocuments()
                task.removeTaskListener(sd)
            else:
                '''
                When adding a single document, do not use a
                status dialog...
                '''
                oLoadDocs = self.LoadDocs(self.xMSF, self.xUnoDialog, files, task, self)
                oLoadDocs.loadDocuments()
        except Exception:
            traceback.print_exc()

    '''
    The user clicked delete.
    '''

    def removeDocument(self):
        if len(self.selectedDoc) == 0:
            return

        self.settings.cp_DefaultSession.cp_Content.cp_Documents.remove(
            self.selectedDoc[0])
        # update the selected document
        while self.selectedDoc[0] >= self.getDocsCount():
            self.selectedDoc[0] -= 1
            # if there are no documents...
        if self.selectedDoc[0] == -1:
            self.selectedDoc = []
            # update the list to show the right selection.

        self.docListDA.updateUI()
        # disables all the next steps, if the list of documents
        # is empty.
        self.checkSteps()

    '''
    doc up.
    '''

    def docUp(self):
        doc = self.settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(
            self.selectedDoc[0])
        self.settings.cp_DefaultSession.cp_Content.cp_Documents.remove(
            self.selectedDoc[0])
        self.settings.cp_DefaultSession.cp_Content.cp_Documents.add(
            self.selectedDoc[0] - 1, doc)
        self.docListDA.updateUI()
        self.disableDocUpDown()

    '''
    doc down
    '''

    def docDown(self):
        doc = self.settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(
            self.selectedDoc[0])
        self.settings.cp_DefaultSession.cp_Content.cp_Documents.remove(
            self.selectedDoc[0])
        self.settings.cp_DefaultSession.cp_Content.cp_Documents.add(
            (self.selectedDoc[0] + 1), doc)
        self.docListDA.updateUI()
        self.disableDocUpDown()

    '''
    **************
        STEP 5
    **************
    '''

    '''
    the user clicked the "backgrounds" button
    '''

    def chooseBackground(self):
        try:
            self.setEnabled(self.btnBackgrounds, False)
            if self.bgDialog is None:
                self.bgDialog = BackgroundsDialog(
                    self.xMSF, self.settings.cp_BackgroundImages,
                    self.resources)
                self.bgDialog.createWindowPeer(self.xUnoDialog.Peer)

            self.bgDialog.setSelected(
                self.settings.cp_DefaultSession.cp_Design.cp_BackgroundImage)
            i = self.bgDialog.executeDialogFromParent(self)
            if i == 1:
                #ok
                self.setBackground(self.bgDialog.getSelected())
        except Exception:
            traceback.print_exc()
        finally:
            self.setEnabled(self.btnBackgrounds, True)

    '''
    invoked when the BackgorundsDialog is "OKed".
    '''

    def setBackground(self, background):
        if background is None:
            background = ""

        self.settings.cp_DefaultSession.cp_Design.cp_BackgroundImage \
            = background
        self.refreshStylePreview()

    '''
    is called when the user clicks "Icon sets" button.
    '''

    def chooseIconset(self):
        try:
            self.setEnabled(self.btnIconSets, False)
            if self.iconsDialog is None:
                self.iconsDialog = IconsDialog(
                    self.xMSF, self.settings.cp_IconSets,
                    self.resources)
                self.iconsDialog.createWindowPeer(self.xUnoDialog.Peer)

            self.iconsDialog.setIconset(
                self.settings.cp_DefaultSession.cp_Design.cp_IconSet)
            i = self.iconsDialog.executeDialogFromParent(self)
            if i == 1:
                #ok
                self.setIconset(self.iconsDialog.getIconset())
        except Exception:
            traceback.print_exc()
        finally:
            self.setEnabled(self.btnIconSets, True)

    '''
    invoked when the Iconsets Dialog is OKed.
    '''

    def setIconset(self, icon):
        self.settings.cp_DefaultSession.cp_Design.cp_IconSet = icon
        self.updateIconsetText()

    '''
    **************
        STEP 7
    **************
    '''

    '''
    sets the publishing url of either a local/zip or ftp publisher.
    updates the ui....
    '''
    def setPublishUrl(self, publisher, url, number):
        if url is None:
            return None

        p = self.getPublisher(publisher)
        p.cp_URL = url
        p.cp_Publish = True
        self.updatePublishUI(number)
        p.overwriteApproved = True
        return p

    '''
    updates the ui of a certain publisher
    (the text box url)
    @param number
    '''

    def updatePublishUI(self, number):
        self.pubAware[number].updateUI()
        self.pubAware[number + 1].updateUI()
        self.checkPublish()

    '''
    The user clicks the local "..." button.
    '''

    def setPublishLocalDir(self):
        folder = self.showFolderDialog(
            "Local destination directory", "",
            self.settings.cp_DefaultSession.cp_OutDirectory)
        #if ok was pressed...
        self.setPublishUrl(LOCAL_PUBLISHER, folder, 0)

    '''
    The user clicks the "Configure" FTP button.
    '''

    def setFTPPublish(self):
        if self.showFTPDialog(self.getPublisher(FTP_PUBLISHER)):
            self.getPublisher(FTP_PUBLISHER).cp_Publish = True
            self.updatePublishUI(4)

    '''
    show the ftp dialog
    @param pub
    @return true if OK was pressed, otherwise false.
    '''

    def showFTPDialog(self, pub):
        try:
            return self.getFTPDialog(pub).execute(self) == 1
        except Exception:
            traceback.print_exc()
            return False

    '''
    the user clicks the zip "..." button.
    Choose a zip file...
    '''

    def setZipFilename(self):
        sd = self.getZipDialog()
        zipFile = sd.callStoreDialog(
            self.settings.cp_DefaultSession.cp_OutDirectory,
            self.resources.resDefaultArchiveFilename)
        self.setPublishUrl(ZIP_PUBLISHER, zipFile, 2)
        self.getPublisher(ZIP_PUBLISHER).overwriteApproved = True

    '''
    the user clicks the "Preview" button.
    '''

    def documentPreview(self):
        try:
            if self.docPreview is None:
                self.docPreview = TOCPreview(
                    self.xMSF, self.settings, self.resources,
                    self.stylePreview.tempDir, self.myFrame)

            self.docPreview.refresh(self.settings)
        except Exception as ex:
            self.unexpectedError(ex)

    '''
    **************
        FINISH
    **************
    '''

    '''
    This method checks if the given target's path,
    added the pathExtension argument, exists,
    and asks the user what to do about it.
    If the user says its all fine, then the target will
    be replaced.
    @return true if "create" should continue. false if "create" should abort.
    '''
    def publishTargetApproved(self):
        result = True
        # 1. check local publish target
        p = self.getPublisher(LOCAL_PUBLISHER)
        fileAccess = self.getFileAccess()
        # should publish ?
        if (p.cp_Publish):
            path = fileAccess.getPath(p.url, None)
            # target exists?
            if fileAccess.exists(p.url, False):
                #if it's a directory
                if fileAccess.isDirectory(p.url):
                    #check if its empty
                    files = fileAccess.listFiles(p.url, True)
                    if len(files) > 0:
                        '''
                        it is not empty :-(
                        it either a local publisher or an ftp
                        (zip uses no directories as target...)
                        '''
                        message = self.resources.resLocalTragetNotEmpty.replace(
                            "%FILENAME", path)
                        result = AbstractErrorHandler.showMessage2(
                            self.xMSF, self.xUnoDialog.Peer, message,
                            ErrorHandler.MESSAGE_WARNING,
                            ErrorHandler.BUTTONS_YES_NO, ErrorHandler.DEF_NO,
                            ErrorHandler.RESULT_YES)
                        if not result:
                            return result

                else:
                    #not a directory, but still exists
                    message = self.resources.resLocalTargetExistsAsfile.replace(
                        "%FILENAME", path)
                    AbstractErrorHandler.showMessage1(
                        self.xMSF, self.xUnoDialog.Peer, message,
                        ErrorHandler.ERROR_PROCESS_FATAL)
                    return False

                # try to write to the path...
            else:
                # the local target directory does not exist.
                message = self.resources.resLocalTargetCreate.replace(
                    "%FILENAME", path)
                try:
                    result = AbstractErrorHandler.showMessage1(
                        self.xMSF, self.xUnoDialog.Peer, message,
                        ErrorHandler.ERROR_QUESTION_YES)
                except Exception:
                    traceback.print_exc()

                if not result:
                    return result
                    # try to create the directory...

                try:
                    fileAccess.xInterface.createFolder(p.cp_URL)
                except Exception:
                    message = self.resources.resLocalTargetCouldNotCreate.replace(
                        "%FILENAME", path)
                    AbstractErrorHandler.showMessage1(
                        self.xMSF, self.xUnoDialog.Peer, message,
                        ErrorHandler.ERROR_PROCESS_FATAL)
                    return False

        # 2. Check ZIP
        # should publish ?
        p = self.getPublisher(ZIP_PUBLISHER)
        if p.cp_Publish:
            path = fileAccess.getPath(p.cp_URL, None)
            # target exists?
            if fileAccess.exists(p.cp_URL, False):
                #if it's a directory
                if fileAccess.isDirectory(p.cp_URL):
                    message = self.resources.resZipTargetIsDir.replace(
                        "%FILENAME", path)
                    AbstractErrorHandler.showMessage1(
                        self.xMSF, self.xUnoDialog.Peer, message,
                        ErrorHandler.ERROR_PROCESS_FATAL)
                    return False
                else:
                    #not a directory, but still exists ( a file...)
                    if not p.overwriteApproved:
                        message = self.resources.resZipTargetExists.replace(
                            "%FILENAME", path)
                        result = AbstractErrorHandler.showMessage1(
                            self.xMSF, self.xUnoDialog.Peer, message,
                            ErrorHandler.ERROR_QUESTION_YES)
                        if not result:
                            return False

        # 3. check FTP
        p = self.getPublisher(FTP_PUBLISHER)
        # should publish ?
        if p.cp_Publish:
            path = fileAccess.getPath(p.cp_URL, None)
            # target exists?
            if fileAccess.exists(p.url, False):
                #if it's a directory
                if fileAccess.isDirectory(p.url):
                    #check if its empty
                    files = fileAccess.listFiles(p.url, True)
                    if len(files) > 0:
                        '''
                        it is not empty :-(
                        it either a local publisher or an ftp
                        (zip uses no directories as target...)
                        '''
                        message = self.resources.resFTPTargetNotEmpty.replace(
                            "%FILENAME", path)
                        result = AbstractErrorHandler.showMessage1(
                            self.xMSF, self.xUnoDialog.Peer, message,
                            ErrorHandler.ERROR_QUESTION_CANCEL)
                        if not result:
                            return result

                else:
                    #not a directory, but still exists (as a file)
                    message = self.resources.resFTPTargetExistsAsfile.replace(
                        "%FILENAME", path)
                    AbstractErrorHandler.showMessage1(
                        self.xMSF, self.xUnoDialog.Peer, message,
                        ErrorHandler.ERROR_PROCESS_FATAL)
                    return False

                # try to write to the path...
            else:
                # the ftp target directory does not exist.
                message = self.resources.resFTPTargetCreate.replace(
                    "%FILENAME", path)
                result = AbstractErrorHandler.showMessage1(
                    self.xMSF, self.xUnoDialog.Peer, message,
                    ErrorHandler.ERROR_QUESTION_YES)
                if not result:
                    return result
                    # try to create the directory...

                try:
                    fileAccess.mkdir(p.url)
                except Exception:
                    message = self.resources.resFTPTargetCouldNotCreate.replace(
                        "%FILENAME", path)
                    AbstractErrorHandler.showMessage1(
                        self.xMSF, self.xUnoDialog.Peer, message,
                        ErrorHandler.ERROR_PROCESS_FATAL)
                    return False

        return True

    '''
    return false if "create" should be aborted. true if everything is fine.
    '''

    def saveSession(self):
        try:
            node = None
            name = self.getSessionSaveName()
            #set documents index field.
            docs = self.settings.cp_DefaultSession.cp_Content.cp_Documents
            i = 0
            while i < docs.getSize():
                (docs.getElementAt(i)).cp_Index = i
                i += 1
            conf = Configuration.getConfigurationRoot(
                self.xMSF, CONFIG_PATH + "/SavedSessions", True)
            # first I check if a session with the given name exists
            try:
                node = Configuration.getNode(name, conf)
                if node is not None:
                    if not AbstractErrorHandler.showMessage1(
                            self.xMSF, self.xUnoDialog.Peer,
                            self.resources.resSessionExists.replace("${NAME}", name),
                            ErrorHandler.ERROR_NORMAL_IGNORE):
                        return False  #remove the old session
                Configuration.removeNode(conf, name)
            except NoSuchElementException:
                pass

            self.settings.cp_DefaultSession.cp_Index = 0
            node = Configuration.addConfigNode(conf, name)
            self.settings.cp_DefaultSession.cp_Name = name
            self.settings.cp_DefaultSession.writeConfiguration(
                node, CONFIG_READ_PARAM)
            self.settings.cp_SavedSessions.reindexSet(conf, name, "cp_Index")
            Configuration.commit(conf)
            # now I reload the sessions to actualize the list/combo
            # boxes load/save sessions.
            self.settings.cp_SavedSessions.clear()
            confView = Configuration.getConfigurationRoot(
                self.xMSF, CONFIG_PATH + "/SavedSessions", False)
            self.settings.cp_SavedSessions.readConfiguration(
                confView, CONFIG_READ_PARAM)
            self.settings.cp_LastSavedSession = name
            self.currentSession = name
            # now save the name of the last saved session...
            self.settings.cp_LastSavedSession = name
            # TODO add the <none> session...
            self.prepareSessionLists()
            ListModelBinder.fillList(
                self.lstLoadSettings, self.settings.cp_SavedSessions.childrenList, None)
            ListModelBinder.fillComboBox(
                self.cbSaveSettings, self.settings.savedSessions.childrenList, None)
            self.selectSession()
            self.currentSession = self.settings.cp_LastSavedSession
            return True
        except Exception:
            traceback.print_exc()
            return False

    def targetStringFor(self, publisher):
        p = self.getPublisher(publisher)
        if p.cp_Publish:
            return "\n" + self.getFileAccess().getPath(p.cp_URL, None)
        else:
            return ""

    '''
    this method will be called when the Status Dialog
    is hidden.
    It checks if the "Process" was successful, and if so,
    it closes the wizard dialog.
    '''

    def finishWizardFinished(self):
        if self.process.getResult():
            targets = self.targetStringFor(LOCAL_PUBLISHER) + \
                self.targetStringFor(ZIP_PUBLISHER) + \
                self.targetStringFor(FTP_PUBLISHER)
            message = self.resources.resFinishedSuccess.replace(
                "%FILENAME", targets)
            AbstractErrorHandler.showMessage1(
                self.xMSF, self.xUnoDialog.Peer, message, ErrorHandler.ERROR_MESSAGE)
            if self.exitOnCreate:
                self.xUnoDialog.endExecute()

        else:
            AbstractErrorHandler.showMessage1(
                self.xMSF, self.xUnoDialog.Peer, self.resources.resFinishedNoSuccess,
                ErrorHandler.ERROR_WARNING)

    def cancel(self):
        self.xUnoDialog.endExecute()

    '''
    the user clicks the finish/create button.
    '''

    def finishWizard(self):
        return self.finishWizard1(True)

    '''
    finish the wizard
    @param exitOnCreate_ should the wizard close after
    a successful create.
    Default is true,
    I have a hidden feature which enables false here
    '''

    def finishWizard1(self, exitOnCreate_):
        self.exitOnCreate = exitOnCreate_
        '''
        First I check if ftp password was set, if not - the ftp dialog pops up
        This may happen when a session is loaded, since the
        session saves the ftp url and username, but not the password.
        '''
        p = self.getPublisher(FTP_PUBLISHER)
        # if ftp is checked, and no proxies are set, and password is empty...
        if p.cp_Publish and not self.proxies and (p.password is None or p.password == ""):
            if self.showFTPDialog(p):
                self.updatePublishUI(2)
                #now continue...
                return self.finishWizard2()

        else:
            return self.finishWizard2()

    '''
    this method is only called
    if ftp-password was eather set, or
    the user entered one in the FTP Dialog which
    popped up when clicking "Create".
    '''

    def finishWizard2(self):
        p = self.getPublisher(LOCAL_PUBLISHER)
        p.url = p.cp_URL

        '''
        zip publisher is using another url form...
        '''
        p = self.getPublisher(ZIP_PUBLISHER)
        #replace the '%' with '%25'
        url1 = p.cp_URL.replace("%", "%25")
        #replace all '/' with '%2F'
        url1 = url1.replace("/", "%2F")
        p.url = "vnd.sun.star.zip://" + url1 + "/"

        '''
        and now ftp...
        '''
        p = self.getPublisher(FTP_PUBLISHER)
        p.url = FTPDialog.getFullURL1(p)

        ''' first we check the publishing targets. If they exist we warn and
        ask what to do. a False here means the user said "cancel"
        (or rather:clicked)
        '''
        if not self.publishTargetApproved():
            return False
            '''
            In order to save the session correctly,
            I return the value of the ftp publisher cp_Publish
            property to its original value...
            '''
        if self.proxies:
            p.cp_Publish = self.ftp

        #if the "save settings" checkbox is on...
        if self.isSaveSession():
            # if canceled by user
            if not self.saveSession():
                return False
        else:
            self.settings.cp_LastSavedSession = ""

        try:
            conf = Configuration.getConfigurationRoot(self.xMSF, CONFIG_PATH, True)
            Configuration.set(
                self.settings.cp_LastSavedSession, "LastSavedSession", conf)
            Configuration.commit(conf)
        except Exception:
            traceback.print_exc()

        '''
        again, if proxies are on, I disable ftp before the creation process
        starts.
        '''
        if self.proxies:
            p.cp_Publish = False
            '''
            There is currently a bug, which crashes office when
            writing folders to an existing zip file, after deleting
            its content, so I "manually" delete it here...
            '''

        p = self.getPublisher(ZIP_PUBLISHER)
        if self.getFileAccess().exists(p.cp_URL, False):
            self.getFileAccess().delete(p.cp_URL)

        try:
            eh = ProcessErrorHandler(self.xMSF, self.xUnoDialog.Peer, self.resources)
            self.process = Process(self.settings, self.xMSF, eh)
            pd = self.getStatusDialog()
            pd.setRenderer(ProcessStatusRenderer (self.resources))
            pd.execute(self, self.process.myTask, self.resources.prodName)
            #process,
            self.process.runProcess()
            self.finishWizardFinished()
            self.process.myTask.removeTaskListener(pd)
            return True
        except Exception:
            traceback.print_exc()

    '''
    is called on the WindowHidden event,
    deletes the temporary directory.
    '''

    def cleanup(self):
        try:
            self.dpStylePreview.dispose()
            self.stylePreview.cleanup()

            if self.bgDialog is not None:
                self.bgDialog.xUnoDialog.dispose()

            if self.iconsDialog is not None:
                self.iconsDialog.xUnoDialog.dispose()

            if self.ftpDialog is not None:
                self.ftpDialog.xUnoDialog.dispose()

            self.xUnoDialog.dispose()
            self.myFrame.close(False);

            #if ProcessStatusRenderer is not None:
            #    ProcessStatusRenderer.close(False)

        except Exception:
            traceback.print_exc()

    class LoadDocs(object):

        def __init__(self, xmsf, xC_, files_, task_, parent_):
            self.xMSF = xmsf
            self.xC = xC_
            self.files = files_
            self.parent = parent_
            self.task = task_

        def loadDocuments(self):
            offset = self.parent.selectedDoc[0] + 1 if (len(self.parent.selectedDoc) > 0) else self.parent.getDocsCount()

            #Number of documents failed to validate.
            failed = 0

            # store the directory
            self.parent.settings.cp_DefaultSession.cp_InDirectory = \
                FileAccess.getParentDir(self.files[0])

            '''
            Here i go through each file, and validate it.
            If its ok, I add it to the ListModel/ConfigSet
            '''

            for i in range(0, len(self.files)):
                doc = CGDocument()
                doc.setRoot(self.parent.settings)

                doc.cp_URL = self.files[i]

                '''
                so - i check each document and if it is ok I add it.
                The failed variable is used only to calculate the place to add -
                Error reporting to the user is (or should (-:  )
                done in the checkDocument(...) method
                '''
                if self.parent.checkDocument(doc, self.task, self.xC):
                    index = offset + i - failed
                    self.parent.settings.cp_DefaultSession.cp_Content.cp_Documents.add(index, doc)
                else:
                    failed += 1

            # if any documents where added,
            # set the first one to be the current-selected document.
            if len(self.files) > failed:
                self.parent.setSelectedDoc([offset])

            # update the ui...
            self.parent.docListDA.updateUI()
            # this enables/disables the next steps.
            # when no documents in the list, all next steps are disabled
            self.parent.checkSteps()
            '''
            a small insurance that the status dialog will
            really close...
            '''

            while (self.task.getStatus() < self.task.getMax()):
                self.task.advance(False)
