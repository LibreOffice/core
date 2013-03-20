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

from .WWD_General import WWD_General
from .WebWizardConst import *
from .StylePreview import StylePreview
from ..common.Configuration import Configuration
from ..common.FileAccess import FileAccess
from ..common.Desktop import Desktop
from ..document.OfficeDocument import OfficeDocument
from .data.CGSettings import CGSettings
from .data.CGDocument import CGDocument
from .data.CGSessionName import CGSessionName
from ..ui.event.ListModelBinder import ListModelBinder
from ..ui.event.UnoDataAware import UnoDataAware
from ..ui.event.RadioDataAware import RadioDataAware
from ..ui.DocumentPreview import DocumentPreview
from ..ui.event.DataAware import DataAware
from ..ui.event.Task import Task

from com.sun.star.lang import IllegalArgumentException

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
This depends on WWD_General.settings a Listener which calls the updateData()
method od the DataAware object.
When the Value changes, the UI Control changes respopndingly.
This also depends on WWD_General.settings a Listener which calls the updateUI
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

    selectedDoc = []
    ftp = False

    '''
    He - my constructor !
    I call/do here in this order: <br/>
    Check if ftp http proxy is set, and warn user he can not use
    ftp if  it is.<br/>
    draw NaviBar and steps 1 to 7, incl. Step X,
    which is the Layouts ImageList on step 3.<br/>
    load the WWD_General.settings from the configuration. <br/>
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
        self.docAware = []
        self.designAware = []
        self.genAware = []
        self.pubAware = []
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
        self.setSaveSessionName(WWD_General.settings.cp_DefaultSession)
        self.ilLayouts.listModel = WWD_General.settings.cp_Layouts
        self.ilLayouts.create(self)
        #COMMENTED
        self.checkContent(WWD_General.settings.cp_DefaultSession.cp_Content, Task("", "", 99999), self.xUnoDialog)
        #saved sessions, styles, combobox save session.
        # also set the chosen saved session...
        self.fillLists()
        self.makeDataAware()

        self.updateUI()

        # fill the documents listbox.
        self.fillDocumentList(self.settings.cp_DefaultSession.cp_Content)

        if self.proxies:
            self.btnFTP.Model.Enabled = False
            self.chkFTP.Model.Enabled = False

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
        for i in range(WWD_General.settings.cp_SavedSessions.getSize()):
            sessionName = \
                WWD_General.settings.cp_SavedSessions.getElementAt(i).cp_Name
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
    This method goes through all
    the DataAware vectors and objects and
    calls their updateUI() method.
    In response, the UI is synchronized to
    the state of the corresponding data objects.
    '''
    def updateUI(self):
        DataAware.updateUIs(self.designAware)
        DataAware.updateUIs(self.genAware)
        DataAware.updateUIs(self.pubAware)
        self.sessionNameDA.updateUI()
        self.checkPublish()

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
            print ("WARNING !!! show() : executeDialogFromComponent")
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
                self.xMSF, WWD_General.settings.workPath)
            style = WWD_General.settings.cp_DefaultSession.getStyle()
            bg = WWD_General.settings.cp_DefaultSession.cp_Design.cp_BackgroundImage
            self.stylePreview.refresh(style, bg)
            self.dpStylePreview.setDocument(
                self.stylePreview.htmlFilename, DocumentPreview.PREVIEW_MODE)
        except Exception:
            traceback.print_exc()

    '''
    Loads the web wizard WWD_General.settings from the registry.
    '''

    def loadSettings(self, document):
        try:
            # instanciate
            WWD_General.settingsResources = \
                [self.resources.resPages, self.resources.resSlides,
                    self.resources.resCreatedTemplate,
                    self.resources.resUpdatedTemplate,
                    self.resources.resSizeTemplate]

            WWD_General.settings = CGSettings(
                self.xMSF, WWD_General.settingsResources, document)
            # get configuration view
            confRoot = Configuration.getConfigurationRoot(
                self.xMSF, CONFIG_PATH, True)
            # read
            WWD_General.settings.readConfiguration(
                confRoot, CONFIG_READ_PARAM)
            configSet = WWD_General.settings.cp_DefaultSession.cp_Publishing
            # now if path variables are used in publisher paths, they
            # are getting replaced here...
            for i in range(configSet.getSize()):
                p = configSet.getElementAt(i)
                try:
                    p.cp_URL = self.substitute(p.cp_URL)
                except Exception:
                    traceback.print_exc()
            # initialize the WWD_General.settings.
            WWD_General.settings.configure(self.xMSF)
            # set resource needed for web page.
            # sort the styles alphabetically
            #COMMENTED - FIXME: to be impemented
            #WWD_General.settings.cp_Styles.sort(None)
            self.prepareSessionLists()
            if self.proxies:
                self.__ftp = self.getPublisher(FTP_PUBLISHER).cp_Publish
                self.getPublisher(FTP_PUBLISHER).cp_Publish = False

        except Exception:
            traceback.print_exc()

    def prepareSessionLists(self):
        # now copy the sessions list...
        sessions = WWD_General.settings.cp_SavedSessions.childrenList

        WWD_General.settings.savedSessions.clear()
        for index,item in enumerate(sessions):
            WWD_General.settings.savedSessions.add(index, item)
            # add an empty session to the saved session
            # list which apears in step 1
        sn = CGSessionName()
        sn.cp_Name = self.resources.resSessionNameNone
        WWD_General.settings.cp_SavedSessions.add(0, sn)

    '''
    fills the saved session list, the styles list,
    and save session combo box.
    Also set the selected "load" session to the last session
    which was saved.
    '''

    def fillLists(self):
        # fill the saved session list.

        print ("Filling lstLoadSettings with cp_SavedSessions ...")
        ListModelBinder.fillList(self.lstLoadSettings,
            WWD_General.settings.cp_SavedSessions.childrenList, None)
        print ("lstLoadSettings filled !!!")
        # set the selected session to load. (step 1)
        self.selectSession()
        # fill the styles list.
        print ("Filling lstStyles with cp_Styles ...")
        ListModelBinder.fillList(self.lstStyles,
            WWD_General.settings.cp_Styles.childrenList, None)
        print ("lstStyles filled !!!")
        # fill the save session combobox (step 7)
        print ("Filling cbSaveSettings with savedSessions ...")
        ListModelBinder.fillComboBox(self.cbSaveSettings,
            WWD_General.settings.savedSessions.childrenList, None)
        print ("cbSaveSettings filled !!!")

    def selectSession(self):
        selectedSession = 0
        if WWD_General.settings.cp_LastSavedSession is not None \
                and not WWD_General.settings.cp_LastSavedSession == "":
            ses = WWD_General.settings.cp_SavedSessions.getElement(
                WWD_General.settings.cp_LastSavedSession)
            if ses is not None:
                selectedSession = \
                    WWD_General.settings.cp_SavedSessions.getIndexOf(ses)

        self.lstLoadSettings.Model.SelectedItems = (selectedSession,)

    '''
    attaches to each ui-data-control (like checkbox, groupbox or
    textbox, no buttons though), a DataObject's JavaBean Property,
    or class member.
    '''

    def makeDataAware(self):
        #page 1
        ListModelBinder(
            self.lstLoadSettings, WWD_General.settings.cp_SavedSessions)
        #page 2 : document properties
        self.docListDA = UnoDataAware.attachListBox(
            self, "selectedDoc", self.lstDocuments, False)
        self.docListDA.disableObjects = \
            [self.lnDocsInfo, self.btnRemoveDoc, self.lblDocTitle,
                self.txtDocTitle, self.lblDocInfo, self.txtDocInfo,
                self.lblDocAuthor, self.txtDocAuthor, self.lblDocExportFormat,
                self.lstDocTargetType]
        self.docListDA.updateUI()
        doc = CGDocument #dummy
        self.docsBinder = ListModelBinder(self.lstDocuments,
            WWD_General.settings.cp_DefaultSession.cp_Content.cp_Documents)
        self.docAware.append(UnoDataAware.attachEditControl(
            doc, "cp_Title", self.txtDocTitle, True))
        self.docAware.append(UnoDataAware.attachEditControl(
            doc, "cp_Description", self.txtDocInfo, True))
        self.docAware.append(UnoDataAware.attachEditControl(
            doc, "cp_Author", self.txtDocAuthor, True))
        self.docAware.append(UnoDataAware.attachListBox(
            doc, "Exporter", self.lstDocTargetType, False))
        #page 3 : Layout
        design = WWD_General.settings.cp_DefaultSession.cp_Design

        #COMMENTED
        #self.sda = SimpleDataAware.SimpleDataAware_unknown(design, DataAware.PropertyValue ("Layout", design), ilLayouts, DataAware.PropertyValue ("Selected", ilLayouts))

        #self.ilLayouts.addItemListener(None)
        #page 4 : layout 2
        self.designAware.append(UnoDataAware.attachCheckBox(
            design, "cp_DisplayDescription", self.chbDocDesc, True))
        self.designAware.append(UnoDataAware.attachCheckBox(
            design, "cp_DisplayAuthor", self.chbDocAuthor, True))
        self.designAware.append(UnoDataAware.attachCheckBox(
            design, "cp_DisplayCreateDate", self.chkDocCreated, True))
        self.designAware.append(UnoDataAware.attachCheckBox(
            design, "cp_DisplayUpdateDate", self.chkDocChanged, True))
        self.designAware.append(UnoDataAware.attachCheckBox(
            design, "cp_DisplayFilename", self.chkDocFilename, True))
        self.designAware.append(UnoDataAware.attachCheckBox(
            design, "cp_DisplayFileFormat", self.chkDocFormat, True))
        self.designAware.append(UnoDataAware.attachCheckBox(
            design, "cp_DisplayFormatIcon", self.chkDocFormatIcon, True))
        self.designAware.append(UnoDataAware.attachCheckBox(
            design, "cp_DisplayPages", self.chkDocPages, True))
        self.designAware.append(UnoDataAware.attachCheckBox(
            design, "cp_DisplaySize", self.chkDocSize, True))
        self.designAware.append(RadioDataAware.attachRadioButtons(
            WWD_General.settings.cp_DefaultSession.cp_Design,
            "cp_OptimizeDisplaySize",
            (self.optOptimize640x480, self.optOptimize800x600,
                self.optOptimize1024x768), True))
        #page 5 : Style
        '''
        note : on style change, i do not call here refresh ,but rather on
        a special method which will perform some display, background and Iconsets changes.
        '''
        self.designAware.append(UnoDataAware.attachListBox(
            WWD_General.settings.cp_DefaultSession.cp_Design,
            "cp_Style", self.lstStyles, False))
        #page 6 : site general props
        self.genAware.append(UnoDataAware.attachEditControl(
            WWD_General.settings.cp_DefaultSession.cp_GeneralInfo,
            "cp_Title", self.txtSiteTitle, True))
        self.genAware.append(UnoDataAware.attachEditControl(
            WWD_General.settings.cp_DefaultSession.cp_GeneralInfo,
            "cp_Description", self.txtSiteDesc, True))
        self.genAware.append(UnoDataAware.attachDateControl(
            WWD_General.settings.cp_DefaultSession.cp_GeneralInfo,
            "cp_CreationDate", self.dateSiteCreated, False))
        self.genAware.append(UnoDataAware.attachDateControl(
            WWD_General.settings.cp_DefaultSession.cp_GeneralInfo,
            "cp_UpdateDate", self.dateSiteUpdate, False))
        self.genAware.append(UnoDataAware.attachEditControl(
            WWD_General.settings.cp_DefaultSession.cp_GeneralInfo,
            "cp_Email", self.txtEmail, True))
        self.genAware.append(UnoDataAware.attachEditControl(
            WWD_General.settings.cp_DefaultSession.cp_GeneralInfo,
            "cp_Copyright", self.txtCopyright, True))
        #page 7 : publishing
        self.pubAware_(
            LOCAL_PUBLISHER, self.chkLocalDir, self.txtLocalDir, False)
        self.pubAware_(
            FTP_PUBLISHER, self.chkFTP, self.lblFTP, True)
        self.pubAware_(
            ZIP_PUBLISHER, self.chkZip, self.txtZip, False)
        self.sessionNameDA = UnoDataAware.attachEditControl(
            WWD_General.settings.cp_DefaultSession, "cp_Name",
            self.cbSaveSettings, True)

    '''
    A help method to attach a Checkbox and a TextBox to
    a CGPublish object properties/class members,
    @param publish
    @param checkbox
    @param textbox
    '''

    def pubAware_(self, publish, checkbox, textbox, isLabel):
        p = WWD_General.settings.cp_DefaultSession.cp_Publishing.getElement(publish)
        uda = UnoDataAware.attachCheckBox(p, "cp_Publish", checkbox, True)
        uda.Inverse = True
        uda.disableObjects = [textbox]
        self.pubAware.append(uda)
        if isLabel:
            aux = UnoDataAware.attachLabel(p, "cp_URL", textbox, False)
        else:
            aux = UnoDataAware.attachEditControl(p, "cp_URL", textbox, False)
        self.pubAware.append(aux)

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
        self.checkContent(session.cp_Content, task, xC)
        WWD_General.settings.cp_DefaultSession = session
        self.fillDocumentList(session.cp_Content)
        task.advance(True)
        self.mountList(session.cp_Design, self.designAware)
        self.mountList(session.cp_GeneralInfo, self.genAware)
        task.advance(True)
        self.mountDataAware(session.cp_Publishing.getElement(LOCAL_PUBLISHER), 0)
        self.mountDataAware(session.cp_Publishing.getElement(FTP_PUBLISHER), 1)
        self.mountDataAware(session.cp_Publishing.getElement(ZIP_PUBLISHER), 2)
        task.advance(True)
        self.sessionNameDA.setDataObject(session, True)
        self.chkSaveSettings.Model.State = 1
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

    def mountDataAware(self, data, i):
        self.pubAware[i * 2].setDataObject(data, True)
        self.pubAware[i * 2 + 1].setDataObject(data, True)

    '''
    Fills the documents listbox.
    @param root the CGContent object
    that contains the documents (a ListModel)
    '''

    def fillDocumentList(self, root):
        ListModelBinder.fillList(self.lstDocuments, root.cp_Documents.childrenList, None)
        #COMMENTED
        #self.docsBinder.setListModel(root.cp_Documents)
        self.disableDocUpDown()

    '''
    changes the DataAwares Objects' (in
    the gioen list) DataObject to the
    @param data
    @param list
    '''

    def mountList(self, data, _list):
        print ("DEBUG !!! mountList -- data: ", data)
        for i in _list:
            i.setDataObject(data, True)

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

    @classmethod
    def checkDocument1(self, xmsf, doc, task, xC):
        doc.validate(xmsf, task)
        return True

    def checkDocument(self, doc, task, xC):
        doc.validate(self.xMSF, task)
        return True
        '''except IOError:
            relocate = SystemDialog.showMessageBox(
                self.xMSF, xC.Peer, "WarningBox",
                VclWindowPeerAttribute.YES_NO + VclWindowPeerAttribute.DEF_NO,
                getFileAccess().getPath(doc.cp_URL, "") + "\n\n" + \
                    self.resources.resSpecifyNewFileLocation)
            if relocate == 2:
                _file = getDocAddDialog().callOpenDialog(
                    False, FileAccess.getParentDir(doc.cp_URL))
                if _file is None:
                    return False
                else:
                    doc.cp_URL = _file[0]
                    return checkDocument(doc, task, xC)

            else:
                return False

        except IllegalArgumentException, iaex:
            #file is a directory
            SystemDialog.showMessage(
                self.xMSF, xControl.Peer,
                self.resources.resErrIsDirectory.replace(
                    "%FILENAME", getFileAccess().getPath(doc.cp_URL, "")),
                ErrorHandler.ERROR_PROCESS_FATAL)
            return False
        except Exception:
            #something went wrong.
            traceback.print_exc()
            SystemDialog.showMessage(
                self.xMSF, xControl.Peer,
                self.resources.resErrDocValidate.replace(
                    "%FILENAME", getFileAccess().getPath(doc.cp_URL, "")),
                ErrorHandler.ERROR_PROCESS_FATAL)
            return False
        '''

    '''
    Checks the documents contained in this content.
    @param content
    @param task
    '''

    def checkContent(self, content, task, xC):
        i = 0
        while i < content.cp_Documents.getSize():
            if not self.checkDocument(content.cp_Documents.getElementAt(i), task, xC):
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
            if len(WWD_Startup.selectedDoc) == 0:
                aux = False
                aux2 = False
            else:
                if WWD_Startup.selectedDoc[0] == 0:
                    aux = False
                else:
                    aux = True
                if WWD_Startup.selectedDoc[0] + 1 < \
                    WWD_General.settings.cp_DefaultSession.cp_Content.cp_Documents.getSize():
                    aux2 = True
                else:
                    aux2 = False
            self.btnDocUp.Model.Enabled = aux
            self.btnDocDown.Model.Enabled = aux2
        except Exception:
            traceback.print_exc()

    def updateBackgroundText(self):
        bg = \
            WWD_General.settings.cp_DefaultSession.cp_Design.cp_BackgroundImage
        if bg is None or bg == "":
            bg = self.resources.resBackgroundNone
        else:
            bg = FileAccess.getPathFilename(self.getFileAccess().getPath(bg, None))

        self.txtBackground.Model.Label = bg

    def updateIconsetText(self):
        iconset = WWD_General.settings.cp_DefaultSession.cp_Design.cp_IconSet
        if iconset is None or iconset == "":
            iconsetName = self.resources.resIconsetNone
        else:
            IconSet = WWD_General.settings.cp_IconSets.getElement(iconset)
            if IconSet is None:
                iconsetName = self.resources.resIconsetNone
            else:
                iconsetName = IconSet.cp_Name

        self.txtIconset.Model.Label = iconsetName

    '''
    refreshes the style preview.
    I also call here "updateBackgroundtext", because always
    when the background is changed, this method
    has to be called, so I am walking on the safe side here...
    '''

    def refreshStylePreview(self):
        try:
            print ("WARNING !!! refreshStylePreview")
            self.updateBackgroundText()
            self.stylePreview.refresh(WWD_General.settings.cp_DefaultSession.getStyle(),
                                      WWD_General.settings.cp_DefaultSession.cp_Design.cp_BackgroundImage)
            self.dpStylePreview.reload(self.xMSF)
        except Exception:
            traceback.print_exc()
