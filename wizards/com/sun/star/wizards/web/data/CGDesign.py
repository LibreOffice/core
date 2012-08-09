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

class CGDesign(ConfigGroup):

    cp_Layout = str()
    cp_Style = str()
    cp_BackgroundImage = str()
    cp_IconSet = str()
    cp_DisplayTitle = bool()
    cp_DisplayDescription = bool()
    cp_DisplayAuthor = bool()
    cp_DisplayCreateDate = bool()
    cp_DisplayUpdateDate = bool()
    cp_DisplayFilename = bool()
    cp_DisplayFileFormat = bool()
    cp_DisplayFormatIcon = bool()
    cp_DisplayPages = bool()
    cp_DisplaySize = bool()
    cp_OptimizeDisplaySize = int()

    def createDOM(self, parent):
        return XMLHelper.addElement(parent, "design", (0,), (0,))
