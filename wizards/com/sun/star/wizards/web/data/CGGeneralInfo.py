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
from ...common.ConfigGroup import ConfigGroup
from ...common.XMLHelper import XMLHelper

class CGGeneralInfo(ConfigGroup):

    def __init__(self):
        self.cp_Title = str()
        self.cp_Description = str()
        self.cp_Author = str()
        self.cp_CreationDate = int()
        self.cp_UpdateDate = int()
        self.cp_Email = str()
        self.cp_Copyright = str()

    def createDOM(self, parent):
        return XMLHelper.addElement(
            parent, "general-info",
            ["title", "author", "description", "creation-date",
                "update-date", "email", "copyright"],
            [self.cp_Title, self.cp_Author, self.cp_Description,
                str(self.cp_CreationDate), str(self.cp_UpdateDate),
                self.cp_Email, self.cp_Copyright])
