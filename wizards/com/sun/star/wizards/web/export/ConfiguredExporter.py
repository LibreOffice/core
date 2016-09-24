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
from ..data.CGArgument import CGArgument
from ..data.CGExporter import CGExporter
from .FilterExporter import FilterExporter

class ConfiguredExporter(FilterExporter):

    # (non-Javadoc)
    # @see com.sun.star.wizards.web.export.Exporter#init(com.sun.star.wizards.web.data.CGExporter)
    def __init(exporter):
        super(ConfiguredExporter, self).__init__(exporter)
        for key in exporter.cp_Arguments.childrenMap.keys():
            if (not key == "Filter"):
                value = exporter.cp_Arguments.getElement(key)
                self.props[key] = self.cast(value.cp_Value)

    def cast(s):
        s1 = s[1]
        c = s[0]
        if (c == "$"):
            return s1
        elif (c == "%"):
            return int(s1)
        elif (c == "#"):
            return int(s1)
        elif (c == "&"):
            return float(s1)
        elif (c == "f"):
            if (s == "false"):
                return False
        elif (c == "t"):
            if (s == "true"):
                return True
        return None
