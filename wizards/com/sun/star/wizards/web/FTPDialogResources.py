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

    def __init__(self, xmsf):
        super(FTPDialogResources, self).__init__(xmsf, self.MODULE_NAME)

        # Delete the String, uncomment the getResText method
        self.resFTPDialog_title = self.getResText(self.RID_FTPDIALOG_START + 0)
        self.reslblUsername_value = self.getResText(self.RID_FTPDIALOG_START + 1)
        self.reslblPassword_value = self.getResText(self.RID_FTPDIALOG_START + 2)
        self.resbtnConnect_value = self.getResText(self.RID_FTPDIALOG_START + 3)
        self.resln1_value = self.getResText(self.RID_FTPDIALOG_START + 4)
        self.reslblFTPAddress_value = self.getResText(self.RID_FTPDIALOG_START + 5)
        self.resln2_value = self.getResText(self.RID_FTPDIALOG_START + 6)
        self.resln3_value = self.getResText(self.RID_FTPDIALOG_START + 7)
        self.resbtnDir_value = self.getResText(self.RID_FTPDIALOG_START + 8)
        self.resFTPDisconnected = self.getResText(self.RID_FTPDIALOG_START + 9)
        self.resFTPConnected = self.getResText(self.RID_FTPDIALOG_START + 10)
        self.resFTPUserPwdWrong = self.getResText(self.RID_FTPDIALOG_START + 11)
        self.resFTPServerNotFound = self.getResText(self.RID_FTPDIALOG_START + 12)
        self.resFTPRights = self.getResText(self.RID_FTPDIALOG_START + 13)
        self.resFTPHostUnreachable = self.getResText(self.RID_FTPDIALOG_START + 14)
        self.resFTPUnknownError = self.getResText(self.RID_FTPDIALOG_START + 15)
        self.resFTPDirectory = self.getResText(self.RID_FTPDIALOG_START + 16)
        self.resIllegalFolder = self.getResText(self.RID_FTPDIALOG_START + 17)
        self.resConnecting = self.getResText(self.RID_FTPDIALOG_START + 18)

        self.resbtnCancel_value = self.getResText(self.RID_COMMON_START + 11)
        self.resbtnOK_value = self.getResText(self.RID_COMMON_START + 18)
        self.resbtnHelp_value = self.getResText(self.RID_COMMON_START + 15)

        self.restxtDir_value = "/"
