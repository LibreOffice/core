import traceback
from common.Desktop import Desktop
from WWD_Startup import WWD_Startup

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
    EMPTY_SHORT_ARRAY = range(0)
    EMPTY_STRING_ARRAY = range(0)

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
        if (old == 1):
            sessionToLoad = ""
            s = Helper.getUnoPropertyValue(lstLoadSettings.Model, "SelectedItems")
            if s.length == 0 or s[0] == 0:
                sessionToLoad = ""
            else:
                sessionToLoad = \
                    settings.cp_SavedSessions.getElementAt(s[0]).cp_Name

            if not sessionToLoad.equals(self.currentSession):
                loadSession(sessionToLoad)

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
        s = Helper.getUnoPropertyValue(getModel(lstLoadSettings), "SelectedItems")
        setEnabled(btnDelSession, s.length > 0 and s[0] > 0)

    '''
    Ha ! the session should be loaded :-)
    '''

    def loadSession(self, sessionToLoad):
        try:
            sd = self.getStatusDialog()
            task = Task("LoadDocs", "", 10)
            sd.execute(this, task, resources.resLoadingSession)
            task.start()
            setSelectedDoc(WWD_Events.EMPTY_SHORT_ARRAY)
            Helper.setUnoPropertyValue(
                lstDocuments.Model, "SelectedItems", WWD_Events.EMPTY_SHORT_ARRAY)
            Helper.setUnoPropertyValue(
                lstDocuments.Model, "StringItemList", WWD_Events.EMPTY_STRING_ARRAY)
            if not sessionToLoad:
                view = Configuration.getConfigurationRoot(
                    xMSF, CONFIG_PATH + "/DefaultSession", False)
            else:
                view = Configuration.getConfigurationRoot(
                    xMSF, CONFIG_PATH + "/SavedSessions", False)
                view = Configuration.getNode(sessionToLoad, view)

            session = CGSession.CGSession()
            session.setRoot(settings)
            session.readConfiguration(view, CONFIG_READ_PARAM)
            task.setMax(session.cp_Content.cp_Documents.getSize() * 5 + 7)
            task.advance(True)
            if sessionToLoad.equals(""):
                setSaveSessionName(session)

            mount(session, task, False, sd.xControl)
            checkSteps()
            self.currentSession = sessionToLoad
            while task.getStatus() <= task.getMax():
                task.advance(False)
            task.removeTaskListener(sd)
        except Exception, ex:
            unexpectedError(ex)

        try:
            refreshStylePreview()
            updateIconsetText()
        except Exception, e:
            # TODO Auto-generated catch block
            e.printStackTrace()

    '''
    hmm. the user clicked the delete button.
    '''

    def delSession(self):
        selected = Helper.getUnoPropertyValue(
            lstLoadSettings.Model, "SelectedItems")
        if selected.length == 0:
            return

        if selected[0] == 0:
            return

        confirm = AbstractErrorHandler.showMessage(
            self.xMSF, xControl.Peer, resources.resDelSessionConfirm,
            ErrorHandler.ERROR_QUESTION_NO)
        if confirm:
            try:
                name = settings.cp_SavedSessions.getKey(selected[0])
                # first delete the session from the registry/configuration.
                Configuration.removeNode(
                    self.xMSF, CONFIG_PATH + "/SavedSessions", name)
                # then delete settings.cp_SavedSessions
                settings.cp_SavedSessions.remove(selected[0])
                settings.savedSessions.remove(selected[0] - 1)
                nextSelected = [0]
                # We try to select the same item index again, if possible
                if settings.cp_SavedSessions.getSize() > selected[0]:
                    nextSelected[0] = selected[0]
                else:
                    # this will always be available because
                    # the user can not remove item 0.
                    nextSelected[0] = (short)(selected[0] - 1)
                    # if the <none> session will
                    # be selected, disable the remove button...
                    if nextSelected[0] == 0:
                        Helper.setUnoPropertyValue(
                            btnDelSession.Model,
                            PropertyNames.PROPERTY_ENABLED, False)
                        # select...

                    Helper.setUnoPropertyValue(
                        lstLoadSettings.Model, "SelectedItems", nextSelected)

            except Exception, ex:
                ex.printStackTrace()
                unexpectedError(ex)

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
        oldDoc = getDoc(selectedDoc)
        doc = getDoc(s)
        if doc == None:
            fillExportList(WWD_Events.EMPTY_STRING_ARRAY)
            #I try to avoid refreshing the export list if
            #the same type of document is chosen.
        elif oldDoc == None or (not oldDoc.appType.equals(doc.appType)):
            fillExportList(settings.getExporters(doc.appType))
        else:

            # do nothing
            selectedDoc = s
            mount(doc, docAware)
            disableDocUpDown()

    '''
    The user clicks the "Add" button.
    This will open a "FileOpen" dialog,
    and, if the user chooses more than one file,
    will open a status dialog, when validating each document.
    '''

    def addDocument(self):
        files = getDocAddDialog().callOpenDialog(
            True, settings.cp_DefaultSession.cp_InDirectory)
        if files is None:
            return

        task = Task.Task_unknown("", "", files.length * 5)
        '''
        If more than a certain number
        of documents have been added,
        open the status dialog.
        '''
        if (files.length > MIN_ADD_FILES_FOR_DIALOG):
            sd = getStatusDialog()
            sd.setLabel(resources.resValidatingDocuments)
            sd.execute(this, task, resources.prodName)
            # new LoadDocs( sd.xControl, files, task )
            oLoadDocs = LoadDocs.LoadDocs_unknown(self.xControl, files, task)
            oLoadDocs.loadDocuments()
            task.removeTaskListener(sd)
        else:
            '''
            When adding a single document, do not use a
            status dialog...
            '''
            oLoadDocs = LoadDocs.LoadDocs_unknown(self.xControl, files, task)
            oLoadDocs.loadDocuments()

    '''
    The user clicked delete.
    '''

    def removeDocument(self):
        if selectedDoc.length == 0:
            return

        settings.cp_DefaultSession.cp_Content.cp_Documents.remove(
            selectedDoc[0])
        # update the selected document
        while selectedDoc[0] >= getDocsCount():
            selectedDoc[0] -= 1
            # if there are no documents...
        if selectedDoc[0] == -1:
            selectedDoc = WWD_Events.EMPTY_SHORT_ARRAY
            # update the list to show the right selection.

        docListDA.updateUI()
        # disables all the next steps, if the list of docuemnts
        # is empty.
        checkSteps()

    '''
    doc up.
    '''

    def docUp(self):
        doc = settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(
            selectedDoc[0])
        settings.cp_DefaultSession.cp_Content.cp_Documents.remove(
            selectedDoc[0])
        settings.cp_DefaultSession.cp_Content.cp_Documents.add(
            selectedDoc[0] - 1, doc)
        docListDA.updateUI()
        disableDocUpDown()

    '''
    doc down
    '''

    def docDown(self):
        doc = settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(
            selectedDoc[0])
        settings.cp_DefaultSession.cp_Content.cp_Documents.remove(
            selectedDoc[0])
        settings.cp_DefaultSession.cp_Content.cp_Documents.add(
            (selectedDoc[0] + 1), doc)
        docListDA.updateUI()
        disableDocUpDown()

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
            setEnabled(btnBackgrounds, False)
            if self.bgDialog == None:
                self.bgDialog = BackgroundsDialog(
                    self.xMSF, settings.cp_BackgroundImages, resources)
                self.bgDialog.createWindowPeer(xControl.Peer)

            self.bgDialog.setSelected(
                settings.cp_DefaultSession.cp_Design.cp_BackgroundImage)
            i = self.bgDialog.executeDialog(WWD_Events.this)
            if i == 1:
                #ok
                setBackground(self.bgDialog.getSelected())
        except Exception, ex:
            ex.printStackTrace()
        finally:
            setEnabled(btnBackgrounds, True)

    '''
    invoked when the BackgorundsDialog is "OKed".
    '''

    def setBackground(self, background):
        if background == None:
            background = ""

        settings.cp_DefaultSession.cp_Design.cp_BackgroundImage = background
        refreshStylePreview()

    '''
    is called when the user clicks "Icon sets" button.
    '''

    def chooseIconset(self):
        try:
            setEnabled(btnIconSets, False)
            if self.iconsDialog == None:
                self.iconsDialog = IconsDialog(
                    self.xMSF, settings.cp_IconSets, resources)
                self.iconsDialog.createWindowPeer(xControl.Peer)

            self.iconsDialog.setIconset(
                settings.cp_DefaultSession.cp_Design.cp_IconSet)
            i = self.iconsDialog.executeDialog(WWD_Events.this)
            if i == 1:
                #ok
                setIconset(self.iconsDialog.getIconset())
        except Exception, ex:
            ex.printStackTrace()
        finally:
            setEnabled(btnIconSets, True)

    '''
    invoked when the Iconsets Dialog is OKed.
    '''

    def setIconset(self, icon):
        settings.cp_DefaultSession.cp_Design.cp_IconSet = icon
        updateIconsetText()

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
        if url == None:
            return None

        p = getPublisher(publisher)
        p.cp_URL = url
        p.cp_Publish = True
        updatePublishUI(number)
        p.overwriteApproved = True
        return p

    '''
    updates the ui of a certain publisher
    (the text box url)
    @param number
    '''

    def updatePublishUI(self, number):
        (pubAware.get(number)).updateUI()
        (pubAware.get(number + 1)).updateUI()
        checkPublish()

    '''
    The user clicks the local "..." button.
    '''

    def setPublishLocalDir(self):
        dir = showFolderDialog(
            "Local destination directory", "",
            settings.cp_DefaultSession.cp_OutDirectory)
        #if ok was pressed...
        setPublishUrl(LOCAL_PUBLISHER, dir, 0)

    '''
    The user clicks the "Configure" FTP button.
    '''

    def setFTPPublish(self):
        if showFTPDialog(getPublisher(FTP_PUBLISHER)):
            getPublisher
            (FTP_PUBLISHER).cp_Publish = True
            updatePublishUI(2)

    '''
    show the ftp dialog
    @param pub
    @return true if OK was pressed, otherwise false.
    '''

    def showFTPDialog(self, pub):
        try:
            return getFTPDialog(pub).execute(this) == 1
        except Exception, ex:
            ex.printStackTrace()
            return False

    '''
    the user clicks the zip "..." button.
    Choose a zip file...
    '''

    def setZipFilename(self):
        sd = getZipDialog()
        zipFile = sd.callStoreDialog(
            settings.cp_DefaultSession.cp_OutDirectory,
            resources.resDefaultArchiveFilename)
        setPublishUrl(ZIP_PUBLISHER, zipFile, 4)
        getPublisher
        (ZIP_PUBLISHER).overwriteApproved = True

    '''
    the user clicks the "Preview" button.
    '''

    def documentPreview(self):
        try:
            if self.docPreview == None:
                self.docPreview = TOCPreview(
                    self.xMSF, settings, resources,
                    stylePreview.tempDir, myFrame)

            self.docPreview.refresh(settings)
        except Exception, ex:
            unexpectedError(ex)

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
        p = getPublisher(LOCAL_PUBLISHER)
        # should publish ?
        if (p.cp_Publish):
            path = getFileAccess().getPath(p.url, None)
            # target exists?
            if getFileAccess().exists(p.url, False):
                #if its a directory
                if getFileAccess().isDirectory(p.url):
                    #check if its empty
                    files = getFileAccess().listFiles(p.url, True)
                    if files.length > 0:
                        '''
                        it is not empty :-(
                        it either a local publisher or an ftp
                        (zip uses no directories as target...)
                        '''
                        message = resources.resLocalTragetNotEmpty.replace(
                            "%FILENAME", path)
                        result = AbstractErrorHandler.showMessage(
                            self.xMSF, xControl.Peer, message,
                            ErrorHandler.MESSAGE_WARNING,
                            ErrorHandler.BUTTONS_YES_NO, ErrorHandler.DEF_NO,
                            ErrorHandler.RESULT_YES)
                        if not result:
                            return result

                else:
                    #not a directory, but still exists
                    message = resources.resLocalTargetExistsAsfile.replace(
                        "%FILENAME", path)
                    AbstractErrorHandler.showMessage(
                        self.xMSF, xControl.Peer, message,
                        ErrorHandler.ERROR_PROCESS_FATAL)
                    return False

                # try to write to the path...
            else:
                # the local target directory does not exist.
                message = resources.resLocalTargetCreate.replace(
                    "%FILENAME", path)
                try:
                    result = AbstractErrorHandler.showMessage(
                        self.xMSF, xControl.Peer, message,
                        ErrorHandler.ERROR_QUESTION_YES)
                except Exception, ex:
                    ex.printStackTrace()

                if not result:
                    return result
                    # try to create the directory...

                try:
                    getFileAccess().fileAccess.createFolder(p.cp_URL)
                except Exception, ex:
                    message = resources.resLocalTargetCouldNotCreate.replace(
                        "%FILENAME", path)
                    AbstractErrorHandler.showMessage(
                        self.xMSF, xControl.Peer, message,
                        ErrorHandler.ERROR_PROCESS_FATAL)
                    return False

        p = getPublisher(ZIP_PUBLISHER)
        if p.cp_Publish:
            path = getFileAccess().getPath(p.cp_URL, None)
            # target exists?
            if getFileAccess().exists(p.cp_URL, False):
                #if its a directory
                if getFileAccess().isDirectory(p.cp_URL):
                    message = resources.resZipTargetIsDir.replace(
                        "%FILENAME", path)
                    AbstractErrorHandler.showMessage(
                        self.xMSF, xControl.Peer, message,
                        ErrorHandler.ERROR_PROCESS_FATAL)
                    return False
                else:
                    #not a directory, but still exists ( a file...)
                    if not p.overwriteApproved:
                        message = resources.resZipTargetExists.replace(
                            "%FILENAME", path)
                        result = AbstractErrorHandler.showMessage(
                            self.xMSF, xControl.Peer, message,
                            ErrorHandler.ERROR_QUESTION_YES)
                        if not result:
                            return False
        # 3. check FTP

        p = getPublisher(FTP_PUBLISHER)
        # should publish ?
        if p.cp_Publish:
            path = getFileAccess().getPath(p.cp_URL, None)
            # target exists?
            if getFileAccess().exists(p.url, False):
                #if its a directory
                if getFileAccess().isDirectory(p.url):
                    #check if its empty
                    files = getFileAccess().listFiles(p.url, True)
                    if files.length > 0:
                        '''
                        it is not empty :-(
                        it either a local publisher or an ftp
                        (zip uses no directories as target...)
                        '''
                        message = resources.resFTPTargetNotEmpty.replace(
                            "%FILENAME", path)
                        result = AbstractErrorHandler.showMessage(
                            self.xMSF, xControl.Peer, message,
                            ErrorHandler.ERROR_QUESTION_CANCEL)
                        if not result:
                            return result

                else:
                    #not a directory, but still exists (as a file)
                    message = resources.resFTPTargetExistsAsfile.replace(
                        "%FILENAME", path)
                    AbstractErrorHandler.showMessage(
                        self.xMSF, xControl.Peer, message,
                        ErrorHandler.ERROR_PROCESS_FATAL)
                    return False

                # try to write to the path...
            else:
                # the ftp target directory does not exist.
                message = resources.resFTPTargetCreate.replace(
                    "%FILENAME", path)
                result = AbstractErrorHandler.showMessage(
                    self.xMSF, xControl.Peer, message,
                    ErrorHandler.ERROR_QUESTION_YES)
                if not result:
                    return result
                    # try to create the directory...

                try:
                    getFileAccess().fileAccess.createFolder(p.url)
                except Exception, ex:
                    message = resources.resFTPTargetCouldNotCreate.replace(
                        "%FILENAME", path)
                    AbstractErrorHandler.showMessage(
                        self.xMSF, xControl.Peer, message,
                        ErrorHandler.ERROR_PROCESS_FATAL)
                    return False

        return True

    '''
    return false if "create" should be aborted. true if everything is fine.
    '''

    def saveSession(self):
        try:
            node = None
            name = getSessionSaveName()
            #set documents index field.
            docs = settings.cp_DefaultSession.cp_Content.cp_Documents
            i = 0
            while i < docs.getSize():
                (docs.getElementAt(i)).cp_Index = i
                i += 1
            conf = Configuration.getConfigurationRoot(
                self.xMSF, CONFIG_PATH + "/SavedSessions", True)
            # first I check if a session with the given name exists
            try:
                node = Configuration.getNode(name, conf)
                if node != None:
                    if not AbstractErrorHandler.showMessage(
                            self.xMSF, xControl.Peer,
                            resources.resSessionExists.replace("${NAME}", name),
                            ErrorHandler.ERROR_NORMAL_IGNORE):
                        return False
                        #remove the old session

                Configuration.removeNode(conf, name)
            except NoSuchElementException, nsex:
                traceb

            settings.cp_DefaultSession.cp_Index = 0;
            node = Configuration.addConfigNode(conf, name)
            settings.cp_DefaultSession.cp_Name = name;
            settings.cp_DefaultSession.writeConfiguration(
                node, CONFIG_READ_PARAM)
            settings.cp_SavedSessions.reindexSet(conf, name, "Index")
            Configuration.commit(conf)
            # now I reload the sessions to actualize the list/combo
            # boxes load/save sessions.
            settings.cp_SavedSessions.clear()
            confView = Configuration.getConfigurationRoot(
                self.xMSF, CONFIG_PATH + "/SavedSessions", False)
            settings.cp_SavedSessions.readConfiguration(
                confView, CONFIG_READ_PARAM)
            settings.cp_LastSavedSession = name;
            self.currentSession = name
            # now save the name of the last saved session...
            settings.cp_LastSavedSession = name;
            # TODO add the <none> session...
            prepareSessionLists()
            ListModelBinder.fillList(
                lstLoadSettings, settings.cp_SavedSessions.items(), None)
            ListModelBinder.fillComboBox(
                cbSaveSettings, settings.savedSessions.items(), None)
            selectSession()
            self.currentSession = settings.cp_LastSavedSession
            return True
        except Exception, ex:
            ex.printStackTrace()
            return False

    def targetStringFor(self, publisher):
        p = getPublisher(publisher)
        if p.cp_Publish:
            return "\n" + getFileAccess().getPath(p.cp_URL, None)
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
            targets = targetStringFor(LOCAL_PUBLISHER) + \
                targetStringFor(ZIP_PUBLISHER) + \
                targetStringFor(FTP_PUBLISHER)
            message = resources.resFinishedSuccess.replace(
                "%FILENAME", targets)
            AbstractErrorHandler.showMessage(
                self.xMSF, xControl.Peer, message, ErrorHandler.ERROR_MESSAGE)
            if self.exitOnCreate:
                self.xDialog.endExecute()

        else:
            AbstractErrorHandler.showMessage(
                self.xMSF, xControl.Peer, resources.resFinishedNoSuccess,
                ErrorHandler.ERROR_WARNING)

    def cancel(self):
        xDialog.endExecute()

    '''
    the user clicks the finish/create button.
    '''

    def finishWizard(self):
        finishWizard(True)
        return True

    '''
    finish the wizard
    @param exitOnCreate_ should the wizard close after
    a successfull create.
    Default is true,
    I have a hidden feature which enables false here
    '''

    def finishWizard(self, exitOnCreate_):
        self.exitOnCreate = exitOnCreate_
        '''
        First I check if ftp password was set, if not - the ftp dialog pops up
        This may happen when a session is loaded, since the
        session saves the ftp url and username, but not the password.
        '''
        p = getPublisher(FTP_PUBLISHER)
        # if ftp is checked, and no proxies are set, and password is empty...
        if p.cp_Publish and not proxies and p.password == None or \
                p.password == "":
            if showFTPDialog(p):
                updatePublishUI(2)
                #now continue...
                finishWizard2()

        else:
            finishWizard2()

    '''
    this method is only called
    if ftp-password was eather set, or
    the user entered one in the FTP Dialog which
    popped up when clicking "Create".
    '''

    def finishWizard2(self):
        p = getPublisher(LOCAL_PUBLISHER)
        p.url = p.cp_URL
        '''
        zip publisher is using another url form...
        '''
        p = getPublisher(ZIP_PUBLISHER)
        #replace the '%' with '%25'
        url1 = JavaTools.replaceSubString(p.cp_URL, "%25", "%")
        #replace all '/' with '%2F'
        url1 = JavaTools.replaceSubString(url1, "%2F", "/")
        p.url = "vnd.sun.star.zip:#" + url1 + "/";
        '''
        and now ftp...
        '''
        p = getPublisher(FTP_PUBLISHER)
        p.url = FTPDialog.getFullURL(p);
        ''' first we check the publishing targets. If they exist we warn and
        ask what to do. a False here means the user said "cancel"
        (or rather:clicked)
        '''
        if not publishTargetApproved():
            return
            '''
            In order to save the session correctly,
            I return the value of the ftp publisher cp_Publish
            property to its original value...
            '''

        p.cp_Publish = __ftp;
        #if the "save settings" checkbox is on...

        if isSaveSession():
            # if canceled by user
            if not saveSession():
                return

        else:
            settings.cp_LastSavedSession = ""

        try:
            conf = Configuration.getConfigurationRoot(xMSF, CONFIG_PATH, True)
            Configuration.set(
                settings.cp_LastSavedSession, "LastSavedSession", conf)
            Configuration.commit(conf)
        except Exception, ex:
            ex.printStackTrace()

        '''
        again, if proxies are on, I disable ftp before the creation process
        starts.
        '''
        if proxies:
            p.cp_Publish = False
            '''
            There is currently a bug, which crashes office when
            writing folders to an existing zip file, after deleting
            its content, so I "manually" delete it here...
            '''

        p = getPublisher(ZIP_PUBLISHER)
        if getFileAccess().exists(p.cp_URL, False):
            getFileAccess().delete(p.cp_URL)

        try:
            eh = ProcessErrorHandler(xMSF, xControl.Peer, resources)
            self.process = Process(settings, xMSF, eh)
            pd = getStatusDialog()
            pd.setRenderer(ProcessStatusRenderer (resources))
            pd.execute(self, self.process.myTask, resources.prodName)
            #process,
            self.process.runProcess()
            finishWizardFinished()
            self.process.myTask.removeTaskListener(pd)
        except Exception, ex:
            ex.printStackTrace()

    '''
    is called on the WindowHidden event,
    deletes the temporary directory.
    '''

    def cleanup(self):
        try:
            self.dpStylePreview.dispose()
            self.stylePreview.cleanup()

            if self.bgDialog is not None:
                self.bgDialog.xComponent.dispose()

            if self.iconsDialog is not None:
                self.iconsDialog.xComponent.dispose()

            if ftpDialog is not None:
                ftpDialog.xComponent.dispose()

            xComponent.dispose()

            if ProcessStatusRenderer is not None:
                ProcessStatusRenderer.close(False)

        except Exception, ex:
            traceback.print_exc()
