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
from ..WebConfigSet import WebConfigSet
from ...common.ConfigGroup import ConfigGroup
from .CGArgument import CGArgument

class CGExporter(ConfigGroup):

    def __init__(self):
        self.cp_Index = -1
        self.cp_Name = str()
        self.cp_ExporterClass = str()
        self.cp_OwnDirectory = bool()
        self.cp_SupportsFilename = bool()
        self.cp_DefaultFilename = str()
        self.cp_Extension = str()
        self.cp_SupportedMimeTypes = str()
        self.cp_Icon = str()
        self.cp_TargetType = str()
        self.cp_Binary = bool()
        self.cp_PageType = int()
        self.targetTypeName = ""
        self.cp_Arguments = WebConfigSet(CGArgument)

    def toString(self):
        return self.cp_Name

    def supports(self, mime):
        return self.cp_SupportedMimeTypes == "" or \
            self.cp_SupportedMimeTypes.find(mime) > -1
