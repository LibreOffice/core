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
from ui.UIConsts import RID_IMG_WEB

class CGLayout(ConfigGroup):

    cp_Index = -1
    cp_Name = str()
    cp_FSName = str()

    def createTemplates(self, xmsf):
        self.templates = {}
        tf = TransformerFactory.newInstance()
        workPath = getSettings().workPath
        fa = FileAccess(xmsf)
        stylesheetPath = fa.getURL(
            getSettings().workPath, "layouts/" + self.cp_FSName)
        files = fa.listFiles(stylesheetPath, False)
        i = 0
        while i < files.length:
            if FileAccess.getExtension(files[i]).equals("xsl"):
                self.templates.put(
                    FileAccess.getFilename(files[i]),
                    tf.newTemplates(StreamSource (files[i])))
            i += 1

    def getImageUrls(self):
        sRetUrls = range(1)
        ResId = RID_IMG_WEB + (self.cp_Index * 2)
        return [ResId, ResId + 1]

    def getTemplates(self, xmsf):
        self.createTemplates(xmsf)
        return self.templates
