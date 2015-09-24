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

class FTPDialogResources(object):

    RID_FTPDIALOG_START = 4200
    RID_COMMON_START = 500

    def __init__(self, oWizardResource):
        self.resFTPDialog_title = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 0)
        self.reslblUsername_value = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 1)
        self.reslblPassword_value = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 2)
        self.resbtnConnect_value = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 3)
        self.resln1_value = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 4)
        self.reslblFTPAddress_value = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 5)
        self.resln2_value = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 6)
        self.resln3_value = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 7)
        self.resbtnDir_value = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 8)
        self.resFTPDisconnected = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 9)
        self.resFTPConnected = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 10)
        self.resFTPUserPwdWrong = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 11)
        self.resFTPServerNotFound = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 12)
        self.resFTPRights = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 13)
        self.resFTPHostUnreachable = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 14)
        self.resFTPUnknownError = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 15)
        self.resFTPDirectory = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 16)
        self.resIllegalFolder = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 17)
        self.resConnecting = oWizardResource.getResText(
            self.RID_FTPDIALOG_START + 18)

        self.resbtnCancel_value = oWizardResource.getResText(
            self.RID_COMMON_START + 11)
        self.resbtnOK_value = oWizardResource.getResText(
            self.RID_COMMON_START + 18)
        self.resbtnHelp_value = oWizardResource.getResText(
            self.RID_COMMON_START + 15)

        self.restxtDir_value = "/"
