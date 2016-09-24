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
from .CGContent import CGContent
from .CGDesign import CGDesign
from .CGGeneralInfo import CGGeneralInfo
from .CGPublish import CGPublish

from com.sun.star.beans import StringPair

class CGSession(ConfigGroup):

    def __init__(self):
        self.cp_Index = -1
        self.cp_InDirectory = str()
        self.cp_OutDirectory = str()
        self.cp_Name = str()
        self.cp_Content = CGContent()
        self.cp_Design = CGDesign()
        self.cp_GeneralInfo = CGGeneralInfo()
        self.cp_Publishing = WebConfigSet(CGPublish)
        self.valid = False

    def createDOM(self, doc):
        root = XMLHelper.addElement(doc, "session",
                                    ["name", "screen-size"],
                                    [self.cp_Name, self.getScreenSize()])
        self.cp_GeneralInfo.createDOM(root)
        self.cp_Content.createDOM(root)
        return root

    def serializeNode(self, node):
        xBuffer = self.root.xmsf.createInstance("com.sun.star.io.Pipe")
        xTextInputStream = self.root.xmsf.createInstance("com.sun.star.io.TextInputStream")
        xSaxWriter = self.root.xmsf.createInstance( "com.sun.star.xml.sax.Writer" )
        xSaxWriter.setOutputStream(xBuffer)
        xTextInputStream.setInputStream(xBuffer)
        node.serialize(xSaxWriter, tuple([StringPair()]))
        result = ""
        while (not xTextInputStream.isEOF()):
            sLine = xTextInputStream.readLine()
            if (not sLine == "") and (not sLine.startswith("<?xml")):
                result = result + sLine + "\n"

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
        return self.root.cp_Layouts.getElement(self.cp_Design.cp_Layout)

    def getStyle(self):
        return self.root.cp_Styles.getElement(self.cp_Design.cp_Style)

    def createDOM1(self):
        factory = self.root.xmsf.createInstance("com.sun.star.xml.dom.DocumentBuilder")
        doc = factory.newDocument()
        self.createDOM(doc)
        return doc
