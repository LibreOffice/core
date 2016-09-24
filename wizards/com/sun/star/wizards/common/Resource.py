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
#import uno before importing from VclWindowPeerAttribute
import uno
from com.sun.star.awt.VclWindowPeerAttribute import OK

class Resource(object):

    def __init__(self, _xMSF, _Module):
        self.xMSF = _xMSF
        self.Module = _Module
        try:
            xResource = self.xMSF.createInstanceWithArguments(
                "org.libreoffice.resource.ResourceIndexAccess", (self.Module,))
            if xResource is None:
                raise Exception ("could not initialize ResourceIndexAccess")

            self.xStringIndexAccess = xResource.getByName("String")

            if self.xStringIndexAccess is None:
                raise Exception ("could not initialize xStringIndexAccess")

        except Exception:
            traceback.print_exc()
            self.showCommonResourceError(self.xMSF)

    def getResText(self, nID):
        try:
            return self.xStringIndexAccess.getByIndex(nID)
        except Exception:
            traceback.print_exc()
            raise ValueError("Resource with ID not " + str(nID) + " not found")

    def getResArray(self, nID, iCount):
        try:
            resArray = []
            for i in range(iCount):
                resArray.append(self.getResText(nID + i))
            return resArray
        except Exception:
            traceback.print_exc()
            raise ValueError("Resource with ID not" + str(nID) + " not found")

    @classmethod
    def showCommonResourceError(self, xMSF):
        from .SystemDialog import SystemDialog
        sError = "The files required could not be found.\n" + \
            "Please start the LibreOffice Setup and choose 'Repair'."
        SystemDialog.showMessageBox(xMSF, "ErrorBox", OK, sError)
