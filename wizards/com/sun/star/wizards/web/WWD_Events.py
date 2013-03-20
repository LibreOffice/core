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
import uno
#from common.Desktop import Desktop
from .WWD_Startup import WWD_Startup
from .WWD_General import WWD_General
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
it is therfore sorted to steps.
not much application-logic here - just plain
methods which react to events.
The only exception are the finish methods with the save
session methods.
'''

class WWD_Events(WWD_Startup):

    iconsDialog = None
    bgDialog = None
    docPreview = None

    '''
    He - my constructor !
    I add a window listener, which, when
    the window closes, deltes the temp directory.
    '''

    def __init__(self, xmsf):
        super(WWD_Events, self).__init__(xmsf)
        self.chkFTP.addKeyListener(None)
        self.chkLocalDir.addKeyListener(None)
        self.chkZip.addKeyListener(None)
        self.currentSession = ""
        self.exitOnCreate = True
        self.time = 0
        self.count = 0

    @classmethod
    def main(self, args):
        ConnectStr = \
            "uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext"
        try:
            xmsf = Desktop.connect(ConnectStr)
            ww = WWD_Events(xmsf)
            ww.show()
            ww.cleanup()
        except Exception:
            traceback.print_exc()

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
                    WWD_Startup.settings.cp_SavedSessions.getElementAt(s[0]).cp_Name

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
        print ("DEBUG !!! loadSession -- sessionToLoad: ", sessionToLoad)
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
            session.root = WWD_Startup.settings
            print ("DEBUG !!! loadSession -- reading configuration ...")
            session.readConfiguration(view, CONFIG_READ_PARAM)
            numDocs = session.cp_Content.cp_Documents.getSize()
            print ("DEBUG !!! loadSession -- numDocs: ", numDocs)
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
                name = WWD_Startup.settings.cp_SavedSessions.getKey(selected[0])
                # first delete the session from the registry/configuration.
                Configuration.removeNode(
                    self.xMSF, CONFIG_PATH + "/SavedSessions", name)
                # then delete WWD_Startup.settings.cp_SavedSessions
                WWD_Startup.settings.cp_SavedSessions.remove(selected[0])
                WWD_Startup.settings.savedSessions.remove(selected[0] - 1)
                nextSelected = [0]
                # We try to select the same item index again, if possible
                if WWD_Startup.settings.cp_SavedSessions.getSize() > selected[0]:
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
    when the user clicks another document
    in the listbox, this method is called,
    and couses the display in
    the textboxes title,description, author and export format
    to change
    '''

    def setSelectedDoc(self, s):
        print ("DEBUG !!! setSelectedDoc -- s: ", s)
        oldDoc = self.getDoc(WWD_Startup.selectedDoc)
        doc = self.getDoc(s)
        if doc is None:
            print ("DEBUG !!! setSelectedDoc -- doc is None.")
            self.fillExportList([])
            #I try to avoid refreshing the export list if
            #the same type of document is chosen.
        elif oldDoc is None or oldDoc.appType != doc.appType:
            print ("DEBUG !!! setSelectedDoc -- oddDoc is None.")
            self.fillExportList(WWD_Startup.settings.getExporters(doc.appType))

        WWD_Startup.selectedDoc = s
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
                True, WWD_Startup.settings.cp_DefaultSession.cp_InDirectory)
            if files is None:
                print ("DEBUG !!! addDocument -- files is None")
                return
            print ("DEBUG !!! addDocument -- number of files: ", len(files))

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
                oLoadDocs = self.LoadDocs(self.xMSF, self.xUnoDialog, files, self)
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
        if len(WWD_Startup.selectedDoc) == 0:
            return

        WWD_Startup.settings.cp_DefaultSession.cp_Content.cp_Documents.remove(
            WWD_Startup.selectedDoc[0])
        # update the selected document
        while WWD_Startup.selectedDoc[0] >= self.getDocsCount():
            WWD_Startup.selectedDoc[0] -= 1
            # if there are no documents...
        if WWD_Startup.selectedDoc[0] == -1:
            WWD_Startup.selectedDoc = []
            # update the list to show the right selection.

        docListDA.updateUI()
        # disables all the next steps, if the list of docuemnts
        # is empty.
        self.checkSteps()

    '''
    doc up.
    '''

    def docUp(self):
        print ("DEBUG !!! docUp --")
        doc = WWD_Startup.settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(
            WWD_Startup.selectedDoc[0])
        WWD_Startup.settings.cp_DefaultSession.cp_Content.cp_Documents.remove(
            WWD_Startup.selectedDoc[0])
        WWD_Startup.settings.cp_DefaultSession.cp_Content.cp_Documents.add(
            WWD_Startup.selectedDoc[0] - 1, doc)
        docListDA.updateUI()
        self.disableDocUpDown()

    '''
    doc down
    '''

    def docDown(self):
        print ("DEBUG !!! docDown --")
        doc = WWD_Startup.settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(
            WWD_Startup.selectedDoc[0])
        WWD_Startup.settings.cp_DefaultSession.cp_Content.cp_Documents.remove(
            WWD_Startup.selectedDoc[0])
        WWD_Startup.settings.cp_DefaultSession.cp_Content.cp_Documents.add(
            (WWD_Startup.selectedDoc[0] + 1), doc)
        docListDA.updateUI()
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
            if WWD_Events.bgDialog is None:
                WWD_Events.bgDialog = BackgroundsDialog(
                    self.xMSF, WWD_Startup.settings.cp_BackgroundImages,
                    self.resources)
                WWD_Events.bgDialog.createWindowPeer(self.xUnoDialog.Peer)

            WWD_Events.bgDialog.setSelected(
                WWD_Startup.settings.cp_DefaultSession.cp_Design.cp_BackgroundImage)
            i = WWD_Events.bgDialog.executeDialogFromParent(self)
            if i == 1:
                #ok
                self.setBackground(WWD_Events.bgDialog.getSelected())
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

        WWD_Startup.settings.cp_DefaultSession.cp_Design.cp_BackgroundImage \
            = background
        self.refreshStylePreview()

    '''
    is called when the user clicks "Icon sets" button.
    '''

    def chooseIconset(self):
        try:
            self.setEnabled(self.btnIconSets, False)
            if WWD_Events.iconsDialog is None:
                WWD_Events.iconsDialog = IconsDialog(
                    self.xMSF, WWD_Startup.settings.cp_IconSets,
                    self.resources)
                WWD_Events.iconsDialog.createWindowPeer(self.xUnoDialog.Peer)

            WWD_Events.iconsDialog.setIconset(
                WWD_Startup.settings.cp_DefaultSession.cp_Design.cp_IconSet)
            i = WWD_Events.iconsDialog.executeDialogFromParent(self)
            if i == 1:
                #ok
                self.setIconset(WWD_Events.iconsDialog.getIconset())
        except Exception:
            traceback.print_exc()
        finally:
            self.setEnabled(self.btnIconSets, True)

    '''
    invoked when the Iconsets Dialog is OKed.
    '''

    def setIconset(self, icon):
        WWD_Startup.settings.cp_DefaultSession.cp_Design.cp_IconSet = icon
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

        print ("DEBUG !!! setPublishUrl - publisher: " + publisher + " url: " + url)

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
        (self.pubAware[number]).updateUI()
        (self.pubAware[number + 1]).updateUI()
        self.checkPublish()

    '''
    The user clicks the local "..." button.
    '''

    def setPublishLocalDir(self):
        folder = self.showFolderDialog(
            "Local destination directory", "",
            WWD_Startup.settings.cp_DefaultSession.cp_OutDirectory)
        #if ok was pressed...
        self.setPublishUrl(LOCAL_PUBLISHER, folder, 0)

    '''
    The user clicks the "Configure" FTP button.
    '''

    def setFTPPublish(self):
        if self.showFTPDialog(self.getPublisher(FTP_PUBLISHER)):
            self.getPublisher(FTP_PUBLISHER).cp_Publish = True
            self.updatePublishUI(2)

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
            WWD_Startup.settings.cp_DefaultSession.cp_OutDirectory,
            self.resources.resDefaultArchiveFilename)
        self.setPublishUrl(ZIP_PUBLISHER, zipFile, 4)
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
        # should publish ?
        if (p.cp_Publish):
            fileAccess = self.getFileAccess()
            path = fileAccess.getPath(p.url, None)
            # target exists?
            if fileAccess.exists(p.url, False):
                #if its a directory
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
                    print ("WARNING !!! publishTargetApproved -- URL: ", p.cp_URL)
                    fileAccess.xInterface.createFolder(p.cp_URL)
                except Exception as ex:
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
                #if its a directory
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
                #if its a directory
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
                except Exception as ex:
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
        print ("DEBUG !!! saveSession")
        try:
            node = None
            name = self.getSessionSaveName()
            #set documents index field.
            docs = WWD_Startup.settings.cp_DefaultSession.cp_Content.cp_Documents
            i = 0
            while i < docs.getSize():
                (docs.getElementAt(i)).cp_Index = i
                i += 1
            conf = Configuration.getConfigurationRoot(
                self.xMSF, CONFIG_PATH + "/SavedSessions", True)
            # first I check if a session with the given name exists
            try:
                print ("DEBUG !!! saveSession -- check if a session with the given name exists")
                print ("DEBUG !!! saveSession -- name: ", name)
                node = Configuration.getNode(name, conf)
                if node is not None:
                    if not AbstractErrorHandler.showMessage1(
                            self.xMSF, self.xUnoDialog.Peer,
                            self.resources.resSessionExists.replace("${NAME}", name),
                            ErrorHandler.ERROR_NORMAL_IGNORE):
                        return False  #remove the old session
                Configuration.removeNode(conf, name)
            except NoSuchElementException as nsex:
                print ("DEBUG !!! saveSession -- node not found in Configuration - name: ", name)
                pass

            WWD_Startup.settings.cp_DefaultSession.cp_Index = 0
            node = Configuration.addConfigNode(conf, name)
            WWD_Startup.settings.cp_DefaultSession.cp_Name = name
            WWD_Startup.settings.cp_DefaultSession.writeConfiguration(
                node, CONFIG_READ_PARAM)
            WWD_Startup.settings.cp_SavedSessions.reindexSet(conf, name, "cp_Index")
            Configuration.commit(conf)
            # now I reload the sessions to actualize the list/combo
            # boxes load/save sessions.
            WWD_Startup.settings.cp_SavedSessions.clear()
            confView = Configuration.getConfigurationRoot(
                self.xMSF, CONFIG_PATH + "/SavedSessions", False)
            WWD_Startup.settings.cp_SavedSessions.readConfiguration(
                confView, CONFIG_READ_PARAM)
            WWD_Startup.settings.cp_LastSavedSession = name
            self.currentSession = name
            # now save the name of the last saved session...
            WWD_Startup.settings.cp_LastSavedSession = name
            # TODO add the <none> session...
            self.prepareSessionLists()
            ListModelBinder.fillList(
                self.lstLoadSettings, WWD_Startup.settings.cp_SavedSessions.childrenList, None)
            ListModelBinder.fillComboBox(
                self.cbSaveSettings, WWD_Startup.settings.savedSessions.childrenList, None)
            self.selectSession()
            self.currentSession = WWD_Startup.settings.cp_LastSavedSession
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
    It checks if the "Process" was successfull, and if so,
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
        self.finishWizard1(True)
        return True

    '''
    finish the wizard
    @param exitOnCreate_ should the wizard close after
    a successfull create.
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
        password = getattr(p, "password", "")
        print ("FTP checked: ", p.cp_Publish)
        if p.cp_Publish and not proxies and (password is None or password == ""):
            if self.showFTPDialog(p):
                self.updatePublishUI(2)
                #now continue...
                self.finishWizard2()

        else:
            self.finishWizard2()

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
        url1 = p.cp_URL.replace("%25", "%")
        #replace all '/' with '%2F'
        url1 = url1.replace("%F", "/")
        p.url = "vnd.sun.star.zip://" + url1 + "/";

        '''
        and now ftp...
        '''
        p = self.getPublisher(FTP_PUBLISHER)
        p.url = FTPDialog.getFullURL1(p);

        ''' first we check the publishing targets. If they exist we warn and
        ask what to do. a False here means the user said "cancel"
        (or rather:clicked)
        '''
        if not self.publishTargetApproved():
            return
            '''
            In order to save the session correctly,
            I return the value of the ftp publisher cp_Publish
            property to its original value...
            '''

        p.cp_Publish = self.ftp
        #if the "save settings" checkbox is on...

        if self.isSaveSession():
            # if canceled by user
            if not self.saveSession():
                print ("DEBUG !!! finishWizard2 - saveSession canceled !!!")
                return
        else:
            WWD_Startup.settings.cp_LastSavedSession = ""

        try:
            conf = Configuration.getConfigurationRoot(self.xMSF, CONFIG_PATH, True)
            Configuration.set(
                WWD_Startup.settings.cp_LastSavedSession, "LastSavedSession", conf)
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

            if WWD_Events.bgDialog is not None:
                WWD_Events.bgDialog.xUnoDialog.dispose()

            if WWD_Events.iconsDialog is not None:
                WWD_Events.iconsDialog.xUnoDialog.dispose()

            if self.ftpDialog is not None:
                self.ftpDialog.xUnoDialog.dispose()

            #self.dispose()

            #if ProcessStatusRenderer is not None:
            #    ProcessStatusRenderer.close(False)

        except Exception as ex:
            traceback.print_exc()

    class LoadDocs(object):

        def __init__(self, xmsf, xC_, files_, task_, parent_):
            self.xMSF = xmsf
            self.xC = xC_
            self.files = files_
            self.parent = parent_
            self.task = task_

        def loadDocuments(self):
            offset = WWD_Startup.selectedDoc[0] + 1 if (len(WWD_Startup.selectedDoc) > 0) else self.parent.getDocsCount()
            print ("DEBUG !!! loadDocuments -- offset: ", offset)

            '''
            if the user chose one file, the list starts at 0,
            if he chose more than one, the first entry is a directory name,
            all the others are filenames.
            '''
            if len(self.files) > 1:
                start = 1
            else:
                start = 0
            print ("DEBUG !!! loadDocuments -- start: ", start)

            #Number of documents failed to validate.
            failed = 0

            # store the directory
            print ("DEBUG !!! loadDocuments (Store the directory) -- dir: ", self.files[0])
            if start == 1:
                WWD_General.settings.cp_DefaultSession.cp_InDirectory = self.files[0]
            else:
                WWD_General.settings.cp_DefaultSession.cp_InDirectory = \
                    FileAccess.getParentDir(self.files[0])

            '''
            Here i go through each file, and validate it.
            If its ok, I add it to the ListModel/ConfigSet
            '''

            for i in range(start, len(self.files)):
                doc = CGDocument()

                if start == 0:
                    doc.cp_URL = self.files[i]
                else:
                    doc.cp_URL = FileAccess.connectURLs(files[0], self.files[i])
                print ("DEBUG !!! loadDocuments (new Document) -- cp_URL: ", doc.cp_URL)

                '''
                so - i check each document and if it is ok I add it.
                The failed variable is used only to calculate the place to add -
                Error reporting to the user is (or should (-:  )
                done in the checkDocument(...) method
                '''
                if WWD_Startup.checkDocument1(self.xMSF, doc, self.task, self.xC):
                    index = offset + i - failed - start
                    print ("DEBUG !!! loadDocuments (checkDocument) -- index: ", index)
                    WWD_General.settings.cp_DefaultSession.cp_Content.cp_Documents.add(index, doc)
                else:
                    print ("DEBUG !!! loadDocuments (checkDocument) -- failed validation.")
                    failed += 1

            # if any documents where added,
            # set the first one to be the current-selected document.
            if len(self.files) > start + failed:
                print ("DEBUG !!! loadDocuments (setSelectedDoc) -- offset: ", offset)
                self.parent.setSelectedDoc([offset])
            else:
                print ("DEBUG !!! loadDocuments (setSelectedDoc) -- no documents were added")

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
