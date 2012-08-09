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
from common.ConfigGroup import ConfigGroup

class CGGeneralInfo(ConfigGroup):

    cp_Title = str()
    cp_Description = str()
    cp_Author = str()
    cp_CreationDate = int()
    cp_UpdateDate = int()
    cp_Email = str()
    cp_Copyright = str()

    def createDOM(self, parent):
        return XMLHelper.addElement(
            parent, "general-info",
            ["title", "author", "description", "creation-date",
                "update-date", "email", "copyright"],
            [self.cp_Title, self.cp_Author, self.cp_Description,
                str(self.cp_CreationDate), str(self.cp_UpdateDate),
                self.cp_Email, self.cp_Copyright])

    def getCreationDate(self):
        if self.cp_CreationDate == 0:
            self.cp_CreationDate = currentDate()

        return self.cp_CreationDate

    def getUpdateDate(self):
        if self.cp_UpdateDate == 0:
            self.cp_UpdateDate = currentDate()

        return self.cp_UpdateDate

    def setCreationDate(self, i):
        self.cp_CreationDate = i.intValue()

    def setUpdateDate(self, i):
        self.cp_UpdateDate = i.intValue()

    def currentDate(self):
        dt = JavaTools.getDateTime(System.currentTimeMillis())
        return dt.Day + dt.Month * 100 + dt.Year * 10000
