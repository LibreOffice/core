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
from ..WebConfigSet import WebConfigSet
from ...common.XMLHelper import XMLHelper
from .CGDocument import CGDocument

class CGContent(ConfigGroup):

    def __init__(self):
        self.cp_Index = -1
        self.dirName = str()
        self.cp_Name = str()
        self.cp_Description = str()
        self.cp_Documents = WebConfigSet(CGDocument)
        self.cp_Contents = WebConfigSet(CGContent)

    def createDOM(self, parent):
        myElement = XMLHelper.addElement(
            parent, "content",
            ["name", "directory-name", "description", "directory"],
            [self.cp_Name, self.dirName, self.cp_Description, self.dirName])
        self.cp_Documents.createDOM(myElement)
        return myElement
