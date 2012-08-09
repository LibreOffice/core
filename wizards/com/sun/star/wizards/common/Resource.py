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
from wizards.common.Configuration import Configuration
from wizards.common.SystemDialog import SystemDialog

from com.sun.star.awt.VclWindowPeerAttribute import OK

class Resource(object):
    '''
    Creates a new instance of Resource
    @param _xMSF
    @param _Unit
    @param _Module
    '''

    @classmethod
    def __init__(self, _xMSF, _Module):
        self.xMSF = _xMSF
        self.Module = _Module
        try:
            xResource = self.xMSF.createInstanceWithArguments(
                "org.libreoffice.resource.ResourceIndexAccess", (self.Module,))
            if xResource is None:
                raise Exception ("could not initialize ResourceIndexAccess")

            self.xStringIndexAccess = xResource.getByName("String")
            self.xStringListIndexAccess = xResource.getByName("StringList")
            if self.xStringListIndexAccess is None:
                raise Exception ("could not initialize xStringListIndexAccess")

            if self.xStringIndexAccess is None:
                raise Exception ("could not initialize xStringIndexAccess")

        except Exception, exception:
            traceback.print_exc()
            self.showCommonResourceError(self.xMSF)

    def getResText(self, nID):
        try:
            return self.xStringIndexAccess.getByIndex(nID)
        except Exception, exception:
            traceback.print_exc()
            raise ValueError("Resource with ID not " + str(nID) + " not found")

    def getStringList(self, nID):
        try:
            return self.xStringListIndexAccess.getByIndex(nID)
        except Exception, exception:
            traceback.print_exc()
            raise ValueError("Resource with ID not " + str(nID) + " not found")

    def getResArray(self, nID, iCount):
        try:
            ResArray = range(iCount)
            i = 0
            while i < iCount:
                ResArray[i] = getResText(nID + i)
                i += 1
            return ResArray
        except Exception, exception:
            traceback.print_exc()
            raise ValueError("Resource with ID not" + str(nID) + " not found")

    @classmethod
    def showCommonResourceError(self, xMSF):
        ProductName = Configuration.getProductName(xMSF)
        sError = "The files required could not be found.\n" + \
            "Please start the %PRODUCTNAME Setup and choose 'Repair'."
        sError = sError.replace("%PRODUCTNAME", ProductName)
        SystemDialog.showMessageBox(xMSF, "ErrorBox", OK, sError)

