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
import traceback
import uno

from .WWHID import *
from .FTPDialogResources import FTPDialogResources
from ..ui.WizardDialog import WizardDialog
from ..ui.UIConsts import UIConsts
from ..ui.event.DataAware import DataAware
from ..ui.event.UnoDataAware import UnoDataAware
from ..common.PropertyNames import PropertyNames
from ..common.SystemDialog import SystemDialog
from ..common.FileAccess import FileAccess
from ..common.HelpIds import HelpIds
from ..common.UCB import UCB

from com.sun.star.ucb import AuthenticationRequest
from com.sun.star.ucb import InteractiveAugmentedIOException
from com.sun.star.ucb import InteractiveNetworkConnectException
from com.sun.star.ucb import InteractiveNetworkResolveNameException
from com.sun.star.ucb import OpenCommandArgument2
from com.sun.star.ucb.OpenMode import FOLDERS

# This is the FTP Dialog. <br/>
# The Dialog enables the user:
# (*) entering FTP server and user information.
# (*) testing the connection.
# (*) choosing a directory on the server.
# If a connection was established successfully, the user may
# press OK, which will change
# the CGPublish object propertiers according the user's input.
# If no connection was established. the OK and Choose-Dir button are disabled.
# See the method "disconnect()" which disables them.
#
# I use here the DataAware concept to automatically update
# the members ip, username, and password (via the methods setXXX(...))
# for details see the ui.events.DataAware classes. <br/>
class FTPDialog(WizardDialog):
    RES_OK = 2
    RES_CANCEL = 3
    RES_HELP = 4

    # A Constant used for the setLabel(int) method to change the
    # status-display. "unknown" is the status when the user first
    # opens the dialog, or changes the servername/username/password.
    STATUS_UNKNOWN = 0
    # A Constant used for the setLabel(int) method to change the
    # status-display. (connection established)
    STATUS_OK = 1
    # A Constant used for the setLabel(int) method to change the
    # status-display.
    STATUS_USER_PWD_WRONG = 2
    # A Constant used for the setLabel(int) method to change the
    # status-display.
    STATUS_SERVER_NOT_FOUND = 3
    # A Constant used for the setLabel(int) method to change the
    # status-display.
    STATUS_NO_RIGHTS = 4
    # A Constant used for the setLabel(int) method to change the
    # status-display.
    STATUS_HOST_UNREACHABLE = 5
    # A Constant used for the setLabel(int) method to change the
    # status-display.
    STATUS_CONNECTING = 6
    # The icon url for error
    ICON_ERROR = "ftperror.gif"
    # The icon url for ok (connection ok)
    ICON_OK = "ftpconnected.gif"
    # The icon url for unknown - this is the status when
    # the user first opens the dialog
    ICON_UNKNOWN = "ftpunknown.gif"
    # The icon url for an icon representing the "connecting" state.
    ICON_CONNECTING = "ftpconnecting.gif"    # GUI Components as Class members.

    # constructor.
    # constructs the UI.
    # @param xmsf
    # @param p the publishert object that contains the data
    # for this dialog
    # @throws Exception
    def __init__(self, xmsf, p, oWizardResource):

        super(FTPDialog, self).__init__(xmsf, HID_FTP)
        self.publish = p

        templateDir = p.root.soTemplateDir
        self.imagesDirectory = FileAccess.connectURLs(templateDir, "../wizard/bitmap/")

        self.dataAware = []
        self.host = ""
        self.username = ""
        self.password = ""

        # Load Resources
        self.resources = FTPDialogResources(oWizardResource)
        self.ucb = UCB(xmsf)

        # set dialog properties...
        self.setDialogProperties(True, 210, True, 104, 52, 1, 1,
            self.resources.resFTPDialog_title, 310)

        # add controls to dialog
        self.build()
        #make the hostname, username and password textfield data-aware.
        self.configure()
        #make sure we display a disconnected status.
        self.disconnect()


    # Add controls to dialog.
    def build(self):
        PROPNAMES_LABEL = (PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH)
        PROPNAMES_BUTTON = (PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH)
        PROPNAMES_BUTTON2 = (PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "PushButtonType", PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH)

        self.ln1 = self.insertFixedLine("ln1",
                PROPNAMES_LABEL,
                (8, self.resources.resln1_value, "ln1", 6, 6, 0, 210))
        self.lblFTPAddress = self.insertLabel("lblFTPAddress",
                PROPNAMES_LABEL,
                (8, self.resources.reslblFTPAddress_value, "lblFTPAddress", 12, 20, 1, 95))
        self.txtHost = self.insertTextField("txtHost", "disconnect",
                (PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH),
                (12, HelpIds.getHelpIdString(HID_FTP_SERVER), "txtIP", 110, 18, 2, 106), self)
        self.lblUsername = self.insertLabel("lblUsername",
                PROPNAMES_LABEL,
                (8, self.resources.reslblUsername_value, "lblUsername", 12, 36, 3, 85))
        self.txtUsername = self.insertTextField("txtUsername", "disconnect",
                (PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH),
                (12, HelpIds.getHelpIdString(HID_FTP_USERNAME), "txtUsername", 110, 34, 4, 106), self)
        self.lblPassword = self.insertLabel("lblPassword",
                PROPNAMES_LABEL,
                (8, self.resources.reslblPassword_value, "lblPassword", 12, 52, 5, 85))
        self.txtPassword = self.insertTextField("txtPassword", "disconnect",
                ("EchoChar", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH),
                (42, 12, HelpIds.getHelpIdString(HID_FTP_PASS), "txtPassword", 110, 50, 6, 106), self)
        self.ln2 = self.insertFixedLine("ln2",
                PROPNAMES_LABEL,
                (8, self.resources.resln2_value, "ln2", 6, 68, 7, 210))
        self.btnTestConnection = self.insertButton("btnConnect", "connect",
                PROPNAMES_BUTTON,
                (14, HelpIds.getHelpIdString(HID_FTP_TEST), self.resources.resbtnConnect_value, "btnConnect", 12, 80, 8, 50), self)
        self.imgStatus = self.insertImage("imgStatus",
                ("Border", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", "Tabstop", PropertyNames.PROPERTY_WIDTH),
                (0, 14, 68, 80, False, False, 14))
        self.lblStatus = self.insertLabel("lblStatus",
                PROPNAMES_LABEL,
                (8, self.resources.resFTPDisconnected, "lblStatus", 86, 82, 9, 99))
        self.ln3 = self.insertFixedLine("ln3",
                PROPNAMES_LABEL,
                (8, self.resources.resln3_value, "ln3", 6, 100, 10, 210))
        self.txtDir = self.insertTextField("txtDir", None,
                (PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, "Text", PropertyNames.PROPERTY_WIDTH),
                (False, 12, HelpIds.getHelpIdString(HID_FTP_TXT_PATH), "txtDir", 12, 113, 11, self.resources.restxtDir_value, 184), self)
        self.btnDir = self.insertButton("btnDir", "chooseDirectory",
                PROPNAMES_BUTTON,
                (14, HelpIds.getHelpIdString(HID_FTP_BTN_PATH), self.resources.resbtnDir_value, "btnDir", 199, 112, 12, 16), self)
        self.btnOK = self.insertButton("btnOK", None,
                PROPNAMES_BUTTON2,
                (14, HelpIds.getHelpIdString(HID_FTP_OK), self.resources.resbtnOK_value, "btnOK", 165, 142, self.RES_OK, 13, 50), self)
        self.btnCancel = self.insertButton("btnCancel",
                None, PROPNAMES_BUTTON2,
                (14, HelpIds.getHelpIdString(HID_FTP_CANCEL), self.resources.resbtnCancel_value, "btnCancel", 113, 142, self.RES_CANCEL, 14, 50), self)
        self.btnHelp = self.insertButton("btnHelp", None,
                PROPNAMES_BUTTON2,
                (14, "", self.resources.resbtnHelp_value, "btnHelp", 57, 142, self.RES_HELP, 15, 50), self)


    # Make hostname, username and password text fields data aware.
    def configure(self):
        self.dataAware.append(UnoDataAware.attachEditControl(self, "host", self.txtHost, True))
        self.dataAware.append(UnoDataAware.attachEditControl(self, "username", self.txtUsername, True))
        self.dataAware.append(UnoDataAware.attachEditControl(self, "password", self.txtPassword, True))

    # Shows the dialog.
    # If the user clicks ok, changes the given CGPublish properties to the
    # user input.
    # @param parent a dialog to center this dialog to.
    # @return 0 for cancel, 1 for ok.
    # @throws Exception - well, if something goes wrong...
    def execute(self, parent):
        self.host = self.extractHost(self.publish.cp_URL)
        self.username = "" if (self.publish.cp_Username == None) else self.publish.cp_Username
        self.password = "" if (self.publish.password == None) else self.publish.password
        self.folder = self.extractDir(self.publish.cp_URL)
        self.setLabel(self.STATUS_UNKNOWN)

        self.enableTestButton()
        self.updateUI()
        result = self.executeDialogFromParent(parent)
        # change the CGPublish properties
        if (result == 1):
            self.publish.cp_URL = "ftp://" + self.getHost() + self.getDir()
            self.publish.cp_Username = self.username
            self.publish.password = self.password

        return result

    # updates the hostname, username, password and
    # directory text fields.
    # It's called upon initialization.
    def updateUI(self):
        DataAware.updateUIs(self.dataAware)
        self.setDir(self.folder)

    # extract the hostname out of the url used by the
    # publisher. This url does not include the username:password string.
    # @param ftpUrl
    # @return
    def extractHost(self, ftpUrl):
        if (ftpUrl is None or len(ftpUrl) < 6):
            return ""
        url = ftpUrl[6:]
        i = url.find("/")
        if (i == -1):
            return url
        else:
            return url[:i]

    # used to get data from the CGPublish object.
    # @param ftpUrl
    # @return the directory portion of the ftp-url
    def extractDir(self, ftpUrl):
        if (ftpUrl is None or len(ftpUrl) < 6):
            return "/"
        url = ftpUrl[6:]
        i = url.find("/")
        if (i == -1):
            return "/"
        else:
            return url[i:]

    # enables/disables the "test" button
    # according to the status of the hostname, username, password text fields.
    # If one of these fields is empty, the button is disabled.
    def enableTestButton(self):
        self.setEnabled(self.btnTestConnection, not self.isEmpty(self.host) or self.isEmpty(self.username) or self.isEmpty(self.password))

    # @param s
    # @return True if the string is None or "".
    def isEmpty(self, s):
        return (s is None) or (s == "")

    # @return the ftp url with username and password,
    # but without the directory portion.
    def getAccountUrl(self):
        return "ftp://" + self.username + ":" + self.password + "@" + self.getHost()

    # return the host name without the "ftp://"
    # @return
    def getHost(self):
        return self.getHost1(self.host)

    @classmethod
    def getHost1(self, s):
        return s[6:] if s.startswith("ftp://") else s

    # @return the full ftp url including username, password and directory portion.
    def getFullUrl(self):
        return self.getAccountUrl() + self.folder

    # First I try to connect to the full url, including directory.
    # If an InteractiveAugmentedIOException accures, I try again,
    # this time without the dir spec. If this works, I change the dir
    # to "/", if not I say to the user its his problem...
    def connect(self):
        self.setEnabled(self.btnTestConnection, False)
        self.setLabel(self.STATUS_CONNECTING)
        success = False
        try:
            self.connect1(self.getFullUrl())
            success = True
        except InteractiveAugmentedIOException:
            try:
                self.connect1(self.getAccountUrl())
                self.setDir("/")
                success = True
            except Exception:
                self.setLabel(self.STATUS_NO_RIGHTS)
        except InteractiveNetworkResolveNameException:
            self.setLabel(self.STATUS_SERVER_NOT_FOUND)
        except AuthenticationRequest:
            self.setLabel(self.STATUS_USER_PWD_WRONG)
        except InteractiveNetworkConnectException:
            self.setLabel(self.STATUS_HOST_UNREACHABLE)
        except Exception:
            self.setLabel(-1)
            traceback.print_exc()

        if (success):
            self.setLabel(self.STATUS_OK)
            self.setEnabled(self.btnDir, True)
            self.setEnabled(self.btnOK, True)

        self.setEnabled(self.btnTestConnection, True)


    # To try the conenction I do some actions that
    # seem logical to me: <br/>
    # I get a ucb content.
    # I list the files in this content.
    # I call the ucb "open" command.
    # I get the PropertyNames.PROPERTY_TITLE property of this content.
    # @param acountUrl
    # @throws Exception
    def connect1(self, acountUrl):
        content = self.ucb.getContent(self.getAccountUrl())

        # open the content
        aArg = OpenCommandArgument2()
        aArg.Mode = FOLDERS # FOLDER, DOCUMENTS -> simple filter
        aArg.Priority = 32768 # Ignored by most implementations

        self.ucb.executeCommand(content, "open", aArg)

    # changes the ftp subdirectory, in both
    # the UI and the data.
    # @param s the directory.
    def setDir(self, s):
        self.folder = s
        self.txtDir.Model.Text = self.folder

    # @return the ftp subdirecrtory.
    def getDir(self):
        return self.folder

    # changes the status label to disconnected status, and
    # disables the ok and choose-dir buttons.
    # This method is called also when the hostname, username
    # and passwordtext fields change.
    def disconnect(self):
        self.enableTestButton()
        self.setEnabled(self.btnOK, False)
        self.setEnabled(self.btnDir, False)
        self.setLabel(self.STATUS_UNKNOWN)

    # changes the status label and icon, according to the
    # given status
    # @param status one of the private status-constants.
    # if this param is not one of them, an "unknown error" status is displayed.
    def setLabel(self, status):
        if status == self.STATUS_UNKNOWN:
            # not connected yet
            self.setLabel1(self.resources.resFTPDisconnected, self.ICON_UNKNOWN)
        elif status == self.STATUS_OK:
            # connected!
            self.setLabel1(self.resources.resFTPConnected, self.ICON_OK)
        elif status == self.STATUS_USER_PWD_WRONG:
            # wrong password
            self.setLabel1(self.resources.resFTPUserPwdWrong, self.ICON_ERROR)
        elif status == self.STATUS_SERVER_NOT_FOUND:
            # problem resolving server name
            self.setLabel1(self.resources.resFTPServerNotFound, self.ICON_ERROR)
        elif status == self.STATUS_NO_RIGHTS:
            # rights problem
            self.setLabel1(self.resources.resFTPRights, self.ICON_ERROR)
        elif status == self.STATUS_HOST_UNREACHABLE:
            # host unreachable (firewall?)
            self.setLabel1(self.resources.resFTPHostUnreachable, self.ICON_ERROR)
        elif status ==  self.STATUS_CONNECTING:
            self.setLabel1(self.resources.resConnecting, self.ICON_CONNECTING)
        else:
            self.setLabel1(self.resources.resFTPUnknownError, self.ICON_ERROR)

    # changes the text of the status label and
    # (TODO) the status image.
    # @param label
    # @param image
    def setLabel1(self, label, image):
        setattr(self.lblStatus.Model, PropertyNames.PROPERTY_LABEL, label)
        setattr(self.imgStatus.Model, PropertyNames.PROPERTY_IMAGEURL, self.imageUrl(image))

    def imageUrl(self, s):
        return self.imagesDirectory + s

    # called when the user clicks
    # the choose-dir button. ("...")
    # Opens the pickFolder dialog.
    # checks if the returned folder is an ftp folder.
    # sets the textbox and the data to the new selected dir.
    def chooseDirectory(self):
        sd = SystemDialog.createOfficeFolderDialog(self.xMSF)
        newUrl = sd.callFolderDialog(self.resources.resFTPDirectory, "", self.getFullUrl())
        if (newUrl is not None):
            # if the user chose a local directory,
            # sI do not accept it.
            if (newUrl.startswith("ftp://")):
                self.setDir(self.extractDir(newUrl))
            else:
                AbstractErrorHandler.showMessage(self.xMSF, self.xUnoDialog.getPeer(), self.resources.resIllegalFolder, ErrorHandler.ERROR_PROCESS_FATAL)

    # practical to have such a method...
    # @param p the publisher object that contains the ftp connection info.
    # @return the full ftp url with username password and everything one needs.
    @classmethod
    def getFullURL1(self, p):
        return "ftp://" + p.cp_Username + ":" + p.password + "@" + self.getHost1(p.cp_URL)
