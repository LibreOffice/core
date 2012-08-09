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
from common.ConfigSet import ConfigSet
from CGContent import CGContent
from CGDesign import CGDesign
from CGGeneralInfo import CGGeneralInfo
from CGPublish import CGPublish

class CGSession(ConfigGroup):

    cp_Index = -1
    cp_InDirectory = str()
    cp_OutDirectory = str()
    cp_Name = str()
    cp_Content = CGContent()
    cp_Design = CGDesign()
    cp_GeneralInfo = CGGeneralInfo()
    cp_Publishing = ConfigSet(CGPublish())
    valid = False

    def createDOM(self, parent):
        root = XMLHelper.addElement(
            parent, "session", ["name", "screen-size"],
            [self.cp_Name, getScreenSize()])
        self.cp_GeneralInfo.createDOM(root)
        self.cp_Content.createDOM(root)
        return root

    def getScreenSize(self):
        tmp_switch_var1 = self.cp_Design.cp_OptimizeDisplaySize
        if tmp_switch_var1 == 0:
            return "640"
        elif tmp_switch_var1 == 1:
            return "800"
        elif tmp_switch_var1 == 2:
            return "1024"
        else:
            return "800"

    def getLayout(self):
        return self. root.cp_Layouts.getElement(self.cp_Design.cp_Layout)

    def getStyle(self):
        return self.root.cp_Styles.getElement(self.cp_Design.cp_Style)

    def createDOM(self):
        factory = DocumentBuilderFactory.newInstance()
        doc = factory.newDocumentBuilder().newDocument()
        createDOM(doc)
        return doc
