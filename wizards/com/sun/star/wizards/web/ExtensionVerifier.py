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

from ..common.UCB import UCB

# Verifies all String that do not end with
# the given extension.
# This is used to exclude from a copy all the
# xsl files, so I copy from a layout directory
# all the files that do *not* end with xsl.
class ExtensionVerifier(UCB.Verifier):

    extension = ""

    def __init__(self, extension_):
        self.extension = "." + extension_

    # @return true if the given object is
    # a String which does not end with the
    # given extension.
    def verify(self, obj):
        if (isinstance(obj, str)):
            return not obj.endswith(self.extension)
        return False
