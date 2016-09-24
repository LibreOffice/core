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

class CGStyle(ConfigGroup):
    cp_Index = -1
    cp_Name = str()
    cp_CssHref = str()
    cp_BackgroundImage = str()
    cp_IconSet = str()

    def toString(self):
        return self.cp_Name

    def getBackgroundUrl(self):
        if CGStyle.cp_BackgroundImage is None \
                or CGStyle.cp_BackgroundImage == "":
            return None
        else:
            return CGStyle.cp_BackgroundImage
