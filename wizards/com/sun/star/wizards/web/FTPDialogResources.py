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

from ..common.Resource import Resource

class FTPDialogResources(Resource):

    UNIT_NAME = "dbwizres"
    MODULE_NAME = "dbw"
    RID_FTPDIALOG_START = 4200
    RID_COMMON_START = 500
    resFTPDialog_title = ""
    reslblUsername_value = ""
    reslblPassword_value = ""
    resbtnConnect_value = ""
    resbtnOK_value = ""
    resbtnHelp_value = ""
    resbtnCancel_value = ""
    resln1_value = ""
    reslblFTPAddress_value = ""
    resln2_value = ""
    resln3_value = ""
    esln3_value = ""
    restxtDir_value = ""
    resbtnDir_value = ""
    resFTPDisconnected = ""
    resFTPConnected = ""
    resFTPUserPwdWrong = ""
    resFTPServerNotFound = ""
    resFTPRights = ""
    resFTPHostUnreachable = ""
    resFTPUnknownError = ""
    resFTPDirectory = ""
    resIllegalFolder = ""
    resConnecting = ""

    def __init__(self, xmsf):
        super(FTPDialogResources, self).__init__(xmsf, self.MODULE_NAME)

        # Delete the String, uncomment the getResText method
        resFTPDialog_title = self.getResText(self.RID_FTPDIALOG_START + 0)
        reslblUsername_value = self.getResText(self.RID_FTPDIALOG_START + 1)
        reslblPassword_value = self.getResText(self.RID_FTPDIALOG_START + 2)
        resbtnConnect_value = self.getResText(self.RID_FTPDIALOG_START + 3)
        resln1_value = self.getResText(self.RID_FTPDIALOG_START + 4)
        reslblFTPAddress_value = self.getResText(self.RID_FTPDIALOG_START + 5)
        resln2_value = self.getResText(self.RID_FTPDIALOG_START + 6)
        resln3_value = self.getResText(self.RID_FTPDIALOG_START + 7)
        resbtnDir_value = self.getResText(self.RID_FTPDIALOG_START + 8)
        resFTPDisconnected = self.getResText(self.RID_FTPDIALOG_START + 9)
        resFTPConnected = self.getResText(self.RID_FTPDIALOG_START + 10)
        resFTPUserPwdWrong = self.getResText(self.RID_FTPDIALOG_START + 11)
        resFTPServerNotFound = self.getResText(self.RID_FTPDIALOG_START + 12)
        resFTPRights = self.getResText(self.RID_FTPDIALOG_START + 13)
        resFTPHostUnreachable = self.getResText(self.RID_FTPDIALOG_START + 14)
        resFTPUnknownError = self.getResText(self.RID_FTPDIALOG_START + 15)
        resFTPDirectory = self.getResText(self.RID_FTPDIALOG_START + 16)
        resIllegalFolder = self.getResText(self.RID_FTPDIALOG_START + 17)
        resConnecting = self.getResText(self.RID_FTPDIALOG_START + 18)

        resbtnCancel_value = self.getResText(self.RID_COMMON_START + 11)
        resbtnOK_value = self.getResText(self.RID_COMMON_START + 18)
        resbtnHelp_value = self.getResText(self.RID_COMMON_START + 15)

        restxtDir_value = "/"
