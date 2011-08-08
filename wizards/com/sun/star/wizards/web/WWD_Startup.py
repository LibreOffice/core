from WWD_General import *
from common.Configuration import Configuration
from common.FileAccess import FileAccess
from data.CGSettings import CGSettings
from data.CGSessionName import CGSessionName
from ui.event.ListModelBinder import ListModelBinder
from ui.event.UnoDataAware import UnoDataAware
from ui.event.RadioDataAware import RadioDataAware
from data.CGDocument import CGDocument
from ui.DocumentPreview import DocumentPreview
from StylePreview import StylePreview

'''
Web Wizard Dialog implementation : Startup.
This class contains the startup implementation and session mounting methods.
<h2> Data Aware Concept </h2>
<p> I use DataAware objects, which synchronize
UI and Data. A DataAware object a kind of interface
between a Value and a UI element.
</P>
<p>
A Value is either a JavaBean property with
a Getter and a Setter or a public class Memeber.
</P>
When the UI Control changes, the Value changes correspondingly.
This depends on WWD_Startup.settings a Listener which calls the updateData()
method od the DataAware object.
When the Value changes, the UI Control changes respopndingly.
This also depends on WWD_Startup.settings a Listener which calls the updateUI
method of the DataAware object.
</P>
<P>
here I use only UI Listeners, and keep calling
the updateUI() method whenever I change the Value.
</P>
To contain the Data, I use the Configuration Data Objects
which read themselfs out of the Configuration. they are all located under
the data package.
<p/>
Different groups of DataAware objects are grouped into vectors.
Each group (all elements of a vector) uses the same DataObject,
so when I change the underlying dataObject, I change the whole vector's
DataObject, thus actualizing UI.<br/>
This is what happends when a session loads: I Load the session into
a tree of DataObjects, and change the DataAware's DataObject. <br/>
One could also look at this as a kind of "View-Model" relationship.
The controls are the View, The DataObjects are the Model, containing the
Data rad directly from the configuration, and the DataAware objects
are the controller inbetween.

@author rpiterman
'''

class WWD_Startup(WWD_General):

    settings = None

    '''
    He - my constructor !
    I call/do here in this order: <br/>
    Check if ftp http proxy is set, and warn user he can not use
    ftp if  it is.<br/>
    draw NaviBar and steps 1 to 7, incl. Step X,
    which is the Layouts ImageList on step 3.<br/>
    load the WWD_Startup.settings from the configuration. <br/>
    set the default save session name to the first
    available one (sessionX or something like that). <br/>
    set the Layouts ImageList ListModel, and call
    its "create()" method. <br/>
    check the content (documents) specified in the default session.<br/>
    fill the list boxes (saved sessions, styles, combobox save session). <br/>
    make data aware. <br/>
    updateUI (refreshes the display to crrespond to the data in
    the default session.)<br/>
    fill the documents listbox. <br/>
    if proxies are set, disable the ftp controls in step 7.
    '''

    def __init__(self, xmsf):
        super(WWD_Startup, self).__init__(xmsf)
        self.sda = None
        self.selectedDoc = 0
        self.proxies = self.getOOProxies()
        soTemplateDir = FileAccess.getOfficePath2(
            xmsf, "Template", "share", "/wizard")
        exclamationURL = FileAccess.connectURLs(
            soTemplateDir, "../wizard/bitmap/caution_16.png")
        self.drawNaviBar()
        self.buildStep1()
        self.buildStep2()
        self.buildStep3()
        self.buildStep4()
        self.buildStep5()
        self.buildStep6()
        self.buildStep7(self.proxies, exclamationURL)
        self.buildStepX()
        self.xMSF = xmsf
        xDesktop = Desktop.getDesktop(xmsf)
        self.myFrame = OfficeDocument.createNewFrame(xmsf, self)
        doc = OfficeDocument.createNewDocument(
            self.myFrame, "swriter", False, True)
        self.loadSettings(doc)
        self.setSaveSessionName(WWD_Startup.settings.cp_DefaultSession)
        self.ilLayouts.listModel = WWD_Startup.settings.cp_Layouts
        self.ilLayouts.create(self)
        #COMMENTED
        #self.checkContent(WWD_Startup.settings.cp_DefaultSession.cp_Content, Task ("", "", 99999), self.xControl)
        #saved sessions, styles, combobox save session.
        # also set the chosen saved session...
        self.fillLists()
        self.makeDataAware()

        self.checkPublish()

        # fill the documents listbox.
        #COMMENTED
        #self.mount(self.settings.cp_DefaultSession.cp_Content)
        if self.proxies:
            self.setEnabled(btnFTP, False)
            self.setEnabled(chkFTP, False)

    '''
    return true if http proxies or other proxies
    which do not enable ftp to function correctly are set.
    @return true if (http) proxies are on.
    @throws Exception
    '''

    def getOOProxies(self):
        node = Configuration.getConfigurationRoot(
            self.xMSF, "org.openoffice.Inet/Settings", False)
        i = int(node.getByName("ooInetProxyType"))
        if i== 0:
            #no proxies
            return False
        elif i == 2:
            #http proxies
            return True
        else:
            return True

    '''
    calculates the first available session name,
    and sets the current session name to it.
    The combobox text in step 7 will be updated
    automatically when updateUI() is called.
    '''

    def setSaveSessionName(self, session):
        maxValue = 0
        length = self.resources.resSessionName
        # traverse between the sessions and find the one that
        # has the biggest number.
        for i in xrange(WWD_Startup.settings.cp_SavedSessions.getSize()):
            sessionName = \
                WWD_Startup.settings.cp_SavedSessions.getElementAt(i).cp_Name
            if sessionName.startswith(self.resources.resSessionName):
                maxValue = max(maxValue, int(sessionName[length:]))

    '''
    fills the road map, and sets the necessary properties,
    like MaxStep, Complete, Interactive-
    Disables the finbihButton.
    '''

    def addRoadMapItems(self):
        self.insertRoadMapItems(
            [True, True, False, False, False, False, False],
            [self.resources.resStep1, self.resources.resStep2,
                self.resources.resStep3, self.resources.resStep4,
                self.resources.resStep5, self.resources.resStep6,
                self.resources.resStep7])
        self.setRoadmapInteractive(True)
        self.setRoadmapComplete(True)
        self.setCurrentRoadmapItemID(1)
        self.nMaxStep = 7
        self.enableFinishButton(False)

    '''
    create the peer, add roadmap,
    add roadmap items, add style-preview,
    disable the steps 3 to 7 if no documents are
    on the list, and... show the dialog!
    '''

    def show(self):
        try:
            xContainerWindow = self.myFrame.getComponentWindow()
            self.createWindowPeer(xContainerWindow)
            self.addRoadmap()
            self.addRoadMapItems()
            self.addStylePreview()
            self.checkSteps()
            self.executeDialogFromComponent(self.myFrame)
            self.removeTerminateListener()
        except Exception:
            traceback.print_exc()

    '''
    initializes the style preview.
    '''

    def addStylePreview(self):
        try:
            self.dpStylePreview = DocumentPreview(self.xMSF, self.imgPreview)
            self.stylePreview = StylePreview(
                self.xMSF, WWD_Startup.settings.workPath)
            self.stylePreview.refresh(
                WWD_Startup.settings.cp_DefaultSession.getStyle(
                    WWD_Startup.settings.cp_Styles),
                WWD_Startup.settings.cp_DefaultSession.cp_Design.cp_BackgroundImage)
            self.dpStylePreview.setDocument(
                self.stylePreview.htmlFilename, DocumentPreview.PREVIEW_MODE)
        except Exception:
            traceback.print_exc()

    '''
    Loads the web wizard WWD_Startup.settings from the registry.
    '''

    def loadSettings(self, document):
        try:
            # instanciate
            WWD_Startup.settingsResources = \
                [self.resources.resPages, self.resources.resSlides,
                    self.resources.resCreatedTemplate,
                    self.resources.resUpdatedTemplate,
                    self.resources.resSizeTemplate]

            WWD_Startup.settings = CGSettings(
                self.xMSF, WWD_Startup.settingsResources, document)
            # get configuration view
            confRoot = Configuration.getConfigurationRoot(
                self.xMSF, CONFIG_PATH, False)
            # read
            WWD_Startup.settings.readConfiguration(
                confRoot, CONFIG_READ_PARAM)
            configSet = WWD_Startup.settings.cp_DefaultSession.cp_Publishing
            # now if path variables are used in publisher pathes, they
            # are getting replaced here...
            for i in xrange(configSet.getSize()):
                p = configSet.getElementAt(i)
                p.cp_URL = self.substitute(p.cp_URL)
            # initialize the WWD_Startup.settings.
            WWD_Startup.settings.configure(self.xMSF)
            # set resource needed for web page.
            # sort the styles alphabetically
            #COMMENTED
            WWD_Startup.settings.cp_Styles.sort(None)
            self.prepareSessionLists()
            if self.proxies:
                self.__ftp = self.getPublisher(FTP_PUBLISHER).cp_Publish
                self.getPublisher(FTP_PUBLISHER).cp_Publish = False

        except Exception:
            traceback.print_exc()

    def prepareSessionLists(self):
        # now copy the sessions list...
        sessions = WWD_Startup.settings.cp_SavedSessions.childrenList

        WWD_Startup.settings.savedSessions.clear()
        for index,item in enumerate(sessions):
            WWD_Startup.settings.savedSessions.add(index, item)
            # add an empty session to the saved session
            # list which apears in step 1
        sn = CGSessionName()
        sn.cp_Name = self.resources.resSessionNameNone
        WWD_Startup.settings.cp_SavedSessions.add(0, sn)

    '''
    fills the saved session list, the styles list,
    and save session combo box.
    Also set the selected "load" session to the last session
    which was saved.
    '''

    def fillLists(self):
        # fill the saved session list.

        ListModelBinder.fillList(self.lstLoadSettings,
            WWD_Startup.settings.cp_SavedSessions.childrenList, None)
        # set the selected session to load. (step 1)
        self.selectSession()
        # fill the styles list.
        ListModelBinder.fillList(self.lstStyles,
            WWD_Startup.settings.cp_Styles.childrenList, None)
        # fill the save session combobox (step 7)
        ListModelBinder.fillComboBox(self.cbSaveSettings,
            WWD_Startup.settings.savedSessions.childrenList, None)

    def selectSession(self):
        selectedSession = 0
        if WWD_Startup.settings.cp_LastSavedSession is not None \
                and not WWD_Startup.settings.cp_LastSavedSession == "":
            ses = WWD_Startup.settings.cp_SavedSessions.getElement(
                settings.cp_LastSavedSession)
            if ses is not None:
                selectedSession = \
                    WWD_Startup.settings.cp_SavedSessions.getIndexOf(ses)

        Helper.setUnoPropertyValue(
            self.lstLoadSettings.Model, "SelectedItems", (selectedSession,))

    '''
    attaches to each ui-data-control (like checkbox, groupbox or
    textbox, no buttons though), a DataObject's JavaBean Property,
    or class member.
    '''

    def makeDataAware(self):
        #page 1
        ListModelBinder(
            self.lstLoadSettings, WWD_Startup.settings.cp_SavedSessions)
        #page 2 : document properties
        self.docListDA = UnoDataAware.attachListBox(
            self, "selectedDoc", self.lstDocuments, False)
        self.docListDA.disableObjects = \
            [self.lnDocsInfo, self.btnRemoveDoc, self.lblDocTitle,
                self.txtDocTitle, self.lblDocInfo, self.txtDocInfo,
                self.lblDocAuthor, self.txtDocAuthor, self.lblDocExportFormat,
                self.lstDocTargetType]
        self.docListDA.updateUI()
        doc = CGDocument
        #dummy
        self.docsBinder = ListModelBinder(self.lstDocuments,
            WWD_Startup.settings.cp_DefaultSession.cp_Content.cp_Documents)
        UnoDataAware.attachEditControl(
            doc, "cp_Title", self.txtDocTitle, True)
        UnoDataAware.attachEditControl(
            doc, "cp_Description", self.txtDocInfo, True)
        UnoDataAware.attachEditControl(
            doc, "cp_Author", self.txtDocAuthor, True)
        UnoDataAware.attachListBox(
            doc, "Exporter", self.lstDocTargetType, False)
        #page 3 : Layout
        design = WWD_Startup.settings.cp_DefaultSession.cp_Design

        #COMMENTED
        #self.sda = SimpleDataAware.SimpleDataAware_unknown(design, DataAware.PropertyValue ("Layout", design), ilLayouts, DataAware.PropertyValue ("Selected", ilLayouts))

        #self.ilLayouts.addItemListener(None)
        #page 4 : layout 2
        UnoDataAware.attachCheckBox(
            design, "cp_DisplayDescription", self.chbDocDesc, True)
        UnoDataAware.attachCheckBox(
            design, "cp_DisplayAuthor", self.chbDocAuthor, True)
        UnoDataAware.attachCheckBox(
            design, "cp_DisplayCreateDate", self.chkDocCreated, True)
        UnoDataAware.attachCheckBox(
            design, "cp_DisplayUpdateDate", self.chkDocChanged, True)
        UnoDataAware.attachCheckBox(
            design, "cp_DisplayFilename", self.chkDocFilename, True)
        UnoDataAware.attachCheckBox(
            design, "cp_DisplayFileFormat", self.chkDocFormat, True)
        UnoDataAware.attachCheckBox(
            design, "cp_DisplayFormatIcon", self.chkDocFormatIcon, True)
        UnoDataAware.attachCheckBox(
            design, "cp_DisplayPages", self.chkDocPages, True)
        UnoDataAware.attachCheckBox(
            design, "cp_DisplaySize", self.chkDocSize, True)
        RadioDataAware.attachRadioButtons(
            WWD_Startup.settings.cp_DefaultSession.cp_Design,
            "cp_OptimizeDisplaySize",
            (self.optOptimize640x480, self.optOptimize800x600,
                self.optOptimize1024x768), True)
        #page 5 : Style
        '''
        note : on style change, i do not call here refresh ,but rather on
        a special method which will perform some display, background and Iconsets changes.
        '''
        UnoDataAware.attachListBox(
            WWD_Startup.settings.cp_DefaultSession.cp_Design,
            "Style", self.lstStyles, False)
        #page 6 : site general props
        UnoDataAware.attachEditControl(
            WWD_Startup.settings.cp_DefaultSession.cp_GeneralInfo,
            "cp_Title", self.txtSiteTitle, True)
        UnoDataAware.attachEditControl(
            WWD_Startup.settings.cp_DefaultSession.cp_GeneralInfo,
            "cp_Description", self.txtSiteDesc, True)
        UnoDataAware.attachDateControl(
            WWD_Startup.settings.cp_DefaultSession.cp_GeneralInfo,
            "CreationDate", self.dateSiteCreated, False)
        UnoDataAware.attachDateControl(
            WWD_Startup.settings.cp_DefaultSession.cp_GeneralInfo,
            "UpdateDate", self.dateSiteUpdate, False)
        UnoDataAware.attachEditControl(
            WWD_Startup.settings.cp_DefaultSession.cp_GeneralInfo,
            "cp_Email", self.txtEmail, True)
        UnoDataAware.attachEditControl(
            WWD_Startup.settings.cp_DefaultSession.cp_GeneralInfo,
            "cp_Copyright", self.txtCopyright, True)
        #page 7 : publishing
        self.pubAware(
            LOCAL_PUBLISHER, self.chkLocalDir, self.txtLocalDir, False)
        self.pubAware(
            FTP_PUBLISHER, self.chkFTP, self.lblFTP, True)
        self.pubAware(
            ZIP_PUBLISHER, self.chkZip, self.txtZip, False)
        self.sessionNameDA = UnoDataAware.attachEditControl(
            WWD_Startup.settings.cp_DefaultSession, "cp_Name",
            self.cbSaveSettings, True)

    '''
    A help method to attach a Checkbox and a TextBox to
    a CGPublish object properties/class members,
    @param publish
    @param checkbox
    @param textbox
    '''

    def pubAware(self, publish, checkbox, textbox, isLabel):
        p = WWD_Startup.settings.cp_DefaultSession.cp_Publishing.getElement(publish)
        uda = UnoDataAware.attachCheckBox(p, "cp_Publish", checkbox, True)
        uda.Inverse = True
        uda.disableObjects = [textbox]
        #COMMENTED
        '''if isLabel:
            aux = UnoDataAware.attachLabel(p, "URL", textbox, False)
        else:
            aux = UnoDataAware.attachEditControl(p, "URL", textbox, False)'''

    '''
    Session load methods
    (are used both on the start of the wizard and
    when the user loads a session)

    Is called when a new session/settings is
    loaded. <br/>
    Checks the documents (validate), fills the
    documents listbox, and changes the
    DataAware data objects to the
    new session's objects.<br/>
    Task advances 4 times in the mount method,
    and ??? times for each document in the session.
    '''

    def mount(self, session, task, refreshStyle, xC):
        '''
        This checks the documents. If the user
        chooses to cancel, the session is not loaded.
        '''
        checkContent(session.cp_Content, task, xC)
        settings.cp_DefaultSession = session;
        mount(session.cp_Content)
        task.advance(True)
        mount(session.cp_Design, self.designAware)
        mount(session.cp_GeneralInfo, self.genAware)
        task.advance(True)
        mount(session.cp_Publishing.getElement(LOCAL_PUBLISHER), 0)
        mount(session.cp_Publishing.getElement(FTP_PUBLISHER), 1)
        mount(session.cp_Publishing.getElement(ZIP_PUBLISHER), 2)
        task.advance(True)
        self.sessionNameDA.setDataObject(session, True)
        Helper.setUnoPropertyValue(
            chkSaveSettings.Model, PropertyNames.PROPERTY_STATE, 1)
        self.docListDA.updateUI()
        task.advance(True)
        if refreshStyle:
            self.refreshStylePreview()
            self.updateIconsetText()

    '''
    used to mount the publishing objects. This is somehow cryptic -
    there are 6 DataAware objects, 2 for each Publishing Object (CGPublish).
    All 6 reside on one Vector (pubAware).
    Since, otherwise than other DataAware objects, different
    Objects on the same Vector need different DataObjectBinding,
    I use this method...
    @param data the CGPublish object
    @param i the number of the object (0 = local, 1 = ftp, 2 = zip)
    '''

    def mount(self, data, i):
        (self.pubAware.get(i * 2)).setDataObject(data, True)
        (self.pubAware.get(i * 2 + 1)).setDataObject(data, True)

    '''
    Fills the documents listbox.
    @param root the CGContent object
    that contains the documents (a ListModel)
    '''

    def mount(self, root):
        ListModelBinder.fillList(lstDocuments, root.cp_Documents.items(), None)
        self.docsBinder.setListModel(root.cp_Documents)
        disableDocUpDown()

    '''
    changes the DataAwares Objects' (in
    the gioen list) DataObject to the
    @param data
    @param list
    '''

    def mount(self, data, list):
        i = 0
        while i < list.size():
            (list.get(i)).setDataObject(data, True)
            i += 1

    '''
    Checks if the document specified by the given CGDocument
    object (cp_URL) exists. Calls also the validate(...) method
    of this document.
    If the document does not exist, it asks the user whether
    to specify a new URL, or forget about this document.
    @param doc the document to check.
    @param task while loading a session, a status is
    displayed, using the Task object to monitor progress.
    @return true if the document is ok (a file exists in the given url).
    '''

    def checkDocument(self, doc, task, xC):
        try:
            doc.validate(xMSF, task)
            return True
        except FileNotFoundException, ex:
            relocate = SystemDialog.showMessageBox(
                self.xMSF, xC.Peer, "WarningBox",
                VclWindowPeerAttribute.YES_NO + VclWindowPeerAttribute.DEF_NO,
                getFileAccess().getPath(doc.cp_URL, "") + "\n\n" + \
                    self.resources.resSpecifyNewFileLocation)
            if relocate == 2:
                file = getDocAddDialog().callOpenDialog(
                    False, FileAccess.getParentDir(doc.cp_URL))
                if file is None:
                    return False
                else:
                    doc.cp_URL = file[0]
                    return checkDocument(doc, task, xC)

            else:
                return False

        except IllegalArgumentException, iaex:
            #file is a directory
            AbstractErrorHandler.showMessage(
                self.xMSF, xControl.Peer,
                self.resources.resErrIsDirectory.replace(
                    "%FILENAME", getFileAccess().getPath(doc.cp_URL, "")),
                ErrorHandler.ERROR_PROCESS_FATAL)
            return False
        except Exceptionp:
            #something went wrong.
            exp.printStackTrace()
            AbstractErrorHandler.showMessage(
                self.xMSF, xControl.Peer,
                self.resources.resErrDocValidate.replace(
                    "%FILENAME", getFileAccess().getPath(doc.cp_URL, "")),
                ErrorHandler.ERROR_PROCESS_FATAL)
            return False

    '''
    Checks the documents contained in this content.
    @param content
    @param task
    '''

    def checkContent(self, content, task, xC):
        i = 0
        while i < content.cp_Documents.getSize():
            if not checkDocument(content.cp_Documents.getElementAt(i), task, xC):
                # I use here 'i--' since, when the document is removed
                # an index change accures
                content.cp_Documents.remove(i)
                i -= 1
            i += 1

    '''
    Disables/enables the docUpDown buttons (step 2)
    according to the currently selected document
    (no doc selected - both disbaled, last doc selected,
    down disabled and so on...)
    '''

    def disableDocUpDown(self):
        try:
            '''setEnabled(btnDocUp, self.selectedDoc.length == 0 ? Boolean.FALSE : (self.selectedDoc[0] == 0 ? Boolean.FALSE : Boolean.TRUE))
            setEnabled(btnDocDown, self.selectedDoc.length == 0 ? Boolean.FALSE : (self.selectedDoc[0] + 1 < WWD_Startup.settings.cp_DefaultSession.cp_Content.cp_Documents.getSize() ? Boolean.TRUE : Boolean.FALSE))'''
        except Exception, e:
            e.printStackTrace()

    def updateBackgroundText(self):
        bg = \
            WWD_Startup.settings.cp_DefaultSession.cp_Design.cp_BackgroundImage
        if bg is None or bg.equals(""):
            bg = self.resources.resBackgroundNone
        else:
            bg = FileAccess.getPathFilename(getFileAccess().getPath(bg, None))

        Helper.setUnoPropertyValue(
            txtBackground.Model, PropertyNames.PROPERTY_LABEL, bg)

    def updateIconsetText(self):
        iconset = WWD_Startup.settings.cp_DefaultSession.cp_Design.cp_IconSet
        if iconset is None or iconset.equals(""):
            iconsetName = self.resources.resIconsetNone
        else:
            IconSet = WWD_Startup.settings.cp_IconSets.getElement(iconset)
            if IconSet is None:
                iconsetName = self.resources.resIconsetNone
            else:
                iconsetName = IconSet.cp_Name

        Helper.setUnoPropertyValue(
            txtIconset.Model, PropertyNames.PROPERTY_LABEL, iconsetName)

    '''
    refreshes the style preview.
    I also call here "updateBackgroundtext", because always
    when the background is changed, this method
    has to be called, so I am walking on the safe side here...
    '''

    def refreshStylePreview(self):
        try:
            updateBackgroundText()
            self.stylePreview.refresh(settings.cp_DefaultSession.getStyle(), WWD_Startup.settings.cp_DefaultSession.cp_Design.cp_BackgroundImage)
            self.dpStylePreview.reload(xMSF)
        except Exception:
            ex.printStackTrace()
