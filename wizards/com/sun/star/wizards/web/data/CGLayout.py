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
from ...common.FileAccess import FileAccess
from ...ui.UIConsts import UIConsts

class CGLayout(ConfigGroup):

    def __init__(self):
        self.cp_Index = -1
        self.cp_Name = str()
        self.cp_FSName = str()

    def getSettings(self):
        return self.root

    def createTemplates(self, xmsf):
        self.templates = {}

        fa = FileAccess(xmsf)
        stylesheetPath = fa.getURL(
            self.getSettings().workPath, "layouts/" + self.cp_FSName)
        files = fa.listFiles(stylesheetPath, False)
        i = 0
        while i < len(files):
            ext = FileAccess.getExtension(files[i])
            fileName = FileAccess.getFilename(files[i])
            if ext == "xsl":
                self.templates[fileName] = files[i]
            i += 1

    def getImageUrls(self):
        ResId = UIConsts.RID_IMG_WEB + (self.cp_Index * 2)
        return [ResId, ResId + 1]

    def getTemplates(self, xmsf):
        self.createTemplates(xmsf)
        return self.templates
