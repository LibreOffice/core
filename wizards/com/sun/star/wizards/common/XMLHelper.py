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
class XMLHelper:

    @classmethod
    def addElement(self, parent, name, attNames, attValues):
        doc = parent.getOwnerDocument()
        if (doc is None):
            doc = parent
        e = doc.createElement(name)
        for i in range(len(attNames)):
            if (not (attValues[i] is None or (attValues[i] == ""))):
                e.setAttribute(attNames[i], attValues[i])
        parent.appendChild(e)
        return e

    @classmethod
    def addElement1(self, parent, name, attName, attValue):
        return self.addElement(parent, name, [attName], [attValue])
