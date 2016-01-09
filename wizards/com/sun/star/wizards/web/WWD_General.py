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

from .WebWizardDialog import WebWizardDialog
from .StatusDialog import StatusDialog
from .AbstractErrorHandler import AbstractErrorHandler
from .ErrorHandler import ErrorHandler
from .WebWizardConst import *
from .WWHID import *
from .FTPDialog import FTPDialog
from ..common.SystemDialog import SystemDialog
from ..common.FileAccess import FileAccess
from ..common.HelpIds import HelpIds
from ..common.PropertyNames import PropertyNames
from ..ui.event.ListModelBinder import ListModelBinder

'''
This class implements general methods, used by different sub-classes
(either WWD_Sturtup, or WWD_Events) or both.
'''

class WWD_General(WebWizardDialog):

    def __init__(self, xmsf):
        super(WWD_General, self).__init__(xmsf)
        self.xStringSubstitution = SystemDialog.createStringSubstitution(xmsf)
        self.settings = None
        self.folderDialog = None
        self.ftpDialog = None
        self.zipDialog = None
        self.docAddDialog = None
        self.fileAccess = None
        self.proxies = None

    def getStatusDialog(self):
        statusDialog = StatusDialog(
            self.xMSF, StatusDialog.STANDARD_WIDTH,
            self.resources.resLoadingSession, False,
            [self.resources.prodName, "", "", "", "", ""],
            HelpIds.getHelpIdString(HID0_STATUS_DIALOG))
        try:
            statusDialog.createWindowPeer(self.xUnoDialog.Peer)
        except Exception:
            traceback.print_exc()

        return statusDialog

    '''
     File Dialog methods
    '''

    def getDocAddDialog(self):
        self.docAddDialog = SystemDialog.createOpenDialog(self.xMSF)
        for i in range(self.settings.cp_Filters.getSize()):
            f = self.settings.cp_Filters.getElementAt(i)
            if f is not None:
                self.docAddDialog.addFilter(
                    f.cp_Name.replace("%PRODNAME", self.resources.prodName),
                    f.cp_Filter, i == 0)
        return self.docAddDialog

    def getZipDialog(self):
        if self.zipDialog is None:
            self.zipDialog = SystemDialog.createStoreDialog(self.xMSF)
            self.zipDialog.addFilter(self.resources.resZipFiles, "*.zip", True)

        return self.zipDialog

    def getFTPDialog(self, pub):
        if self.ftpDialog is None:
            try:
                self.ftpDialog = FTPDialog(self.xMSF, pub, self.oWizardResource)
                self.ftpDialog.createWindowPeer(self.xUnoDialog.getPeer())
            except Exception:
                traceback.print_exc()
        return self.ftpDialog

    def showFolderDialog(self, title, description, folder):
        if self.folderDialog is None:
            self.folderDialog = SystemDialog.createFolderDialog(self.xMSF)
        return self.folderDialog.callFolderDialog(title, description, folder)

    def getFileAccess(self):
        if (self.fileAccess is None):
            try:
                self.fileAccess = FileAccess(self.xMSF)
            except Exception:
                traceback.print_exc()
        return self.fileAccess

    '''
    returns the document specified
    by the given short array.
    @param s
    @return
    '''

    def getDoc(self, s):
        if (len(s) == 0):
            return None
        elif (self.settings.cp_DefaultSession.cp_Content.cp_Documents.getSize() <= s[0]):
            return None
        else:
            return self.settings.cp_DefaultSession.cp_Content.cp_Documents.getElementAt(s[0])

    '''
    how many documents are in the list?
    @return the number of documents in the docs list.
    '''

    def getDocsCount(self):
        return self.settings.cp_DefaultSession.cp_Content.cp_Documents.getSize()

    '''
    fills the export listbox.
    @param listContent
    '''

    def fillExportList(self, listContent):
        ListModelBinder.fillList(self.lstDocTargetType, listContent, None)

    '''
    returns a publisher object for the given name
    @param name one of the WebWizardConst constants : FTP
    @return
    '''

    def getPublisher(self, name):
        return self.settings.cp_DefaultSession.cp_Publishing.getElement(name)

    '''
    @return true if the checkbox "save session" is checked.
    '''

    def isSaveSession(self):
        return int(self.chkSaveSettings.Model.State == 1)

    '''
    @return the name to save the session (step 7)
    '''

    def getSessionSaveName(self):
        return self.cbSaveSettings.Model.Text

    '''
    This method checks the status of the wizards and
    enables or disables the 'next' and the 'create' button.
    '''

    def checkSteps(self):
        '''
        first I check the document list.
        If it is empty, then step3 and on are disabled.
        '''
        if self.checkDocList():
            self.changeLocalDirState(self.chkLocalDir.Model.State)
            self.changeZipState(self.chkZip.Model.State)
            self.checkPublish()

    '''
    enables/disables the steps 3 to 7)
    @param enabled true = enabled, false = disabled.
    '''

    def enableSteps(self, enabled):
        if not enabled and not self.isStepEnabled(3):
            #disable steps 3-7
            return

        for i in range(3,8):
            self.setStepEnabled(i, enabled, True)
            '''
            in this place i just disable the finish button.
            later, in the checkPublish, which is only performed if
            this one is true, it will be enabled (if the check
            is positive)
            '''

        if not enabled:
            self.enableFinishButton(False)

    '''
    Checks if the documents list is
    empty. If it is, disables the steps 3-7, and the
    create button.
    @return
    '''

    def checkDocList(self):
        if self.settings.cp_DefaultSession.cp_Content.cp_Documents.getSize() \
                == 0:
            self.enableSteps(False)
            return False
        else:
            self.enableSteps(True)
            return True

    '''
    check if the save-settings input is ok.
    (eather the checkbox is unmarked, or,
    if it is marked, a session name exists.
    '''

    def checkSaveSession(self):
        return (not self.isSaveSession() or not self.getSessionSaveName() == "")

    '''
    @return false if this publisher is not active, or, if it
    active, returns true if the url is not empty...
    if the url is empty, throws an exception
    '''

    def checkPublish2(self, s, text, _property):
        p = self.getPublisher(s)
        if p.cp_Publish:
            url = getattr(text.Model, _property)
            if url is None or url == "":
                return False
            else:
                return True
        else:
            return False

    '''

    @return false either if publishing input is wrong or there
    are no publishing targets chosen. returns true when at least
    one target is chosen, *and* all
    which are chosen are legal.
    If proxies are on, ftp publisher is ignored.
    '''

    def checkPublish_(self):
        return \
        self.checkPublish2(LOCAL_PUBLISHER, self.txtLocalDir, "Text") \
        or (not self.proxies and self.checkPublish2(
            FTP_PUBLISHER, self.lblFTP, PropertyNames.PROPERTY_LABEL) \
        or self.checkPublish2(ZIP_PUBLISHER, self.txtZip, "Text")) \
        and self.checkSaveSession()

    '''
    This method checks if the publishing
    input is ok, and enables and disables
    the 'create' button.
    public because it is called from
    an event listener object.
    '''

    def checkPublish(self):
        self.enableFinishButton(self.checkPublish_())

    def chkLocalDirItemChanged(self):
        self.changeLocalDirState(self.chkLocalDir.Model.State)
        self.checkPublish()

    def chkZipItemChanged(self):
        self.changeZipState(self.chkZip.Model.State)
        self.checkPublish()

    def changeLocalDirState(self, enable):
        self.btnLocalDir.Model.Enabled = enable

    def changeZipState(self, enable):
        self.btnZip.Model.Enabled = enable
        
    '''
    shows a message box "Unexpected Error... " :-)
    @param ex
    '''

    def unexpectedError(self, ex):
        traceback.print_exc()
        peer = self.xUnoDialog.getPeer()
        AbstractErrorHandler.showMessage1(
            self.xMSF, peer, self.resources.resErrUnexpected,
            ErrorHandler.ERROR_PROCESS_FATAL)

    '''
    substitutes path variables with the corresponding values.
    @param path a path, which might contain OOo path variables.
    @return the path, after substituting path variables.
    '''

    def substitute(self, path):
        try:
            return self.xStringSubstitution.substituteVariables(path, False)
        except Exception:
            traceback.print_exc()
            return path
