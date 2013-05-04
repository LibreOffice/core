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

class CGDesign(ConfigGroup):

    def __init__(self):
        self.cp_Layout = str()
        self.cp_Style = str()
        self.cp_BackgroundImage = str()
        self.cp_IconSet = str()
        self.cp_DisplayTitle = bool()
        self.cp_DisplayDescription = bool()
        self.cp_DisplayAuthor = bool()
        self.cp_DisplayCreateDate = bool()
        self.cp_DisplayUpdateDate = bool()
        self.cp_DisplayFilename = bool()
        self.cp_DisplayFileFormat = bool()
        self.cp_DisplayFormatIcon = bool()
        self.cp_DisplayPages = bool()
        self.cp_DisplaySize = bool()
        self.cp_OptimizeDisplaySize = int()

    def createDOM(self, parent):
        return XMLHelper.addElement(parent, "design", (0,), (0,))

    def getStyle(self):
        style = self.root.cp_Styles.getElement(self.cp_Style)
        return [self.root.cp_Styles.getIndexOf(style)]

    def setStyle(self, newStyle):
        o = self.root.cp_Styles.getElementAt(newStyle[0])
        self.cp_Style = self.root.cp_Styles.getKey(o)

    def getLayout(self):
        layout = self.root.cp_Layouts.getElement(self.cp_Layout)
        return layout.cp_Index

    def setLayout(self, layoutIndex):
        layout = self.root.cp_Layouts.getElementAt(layoutIndex)
        self.cp_Layout = self.root.cp_Layouts.getKey(layout)
