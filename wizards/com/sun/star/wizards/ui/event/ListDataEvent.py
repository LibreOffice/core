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

class ListDataEvent:

    INTERVAL_ADDED = 1
    INTERVAL_REMOVED = 2
    CONTENTS_CHANGED = 3

    # general constructor -
    # @param source
    # @param type_
    def __init__(self, source_, type_, i0, i1):
        #super(TaskEvent, self).__init__(source)
        self.index0 = i0
        self.index1 = i1

    def getIndex0(self):
        return self.index0

    def getIndex1(self):
        return self.index1
