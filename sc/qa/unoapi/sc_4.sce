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
-o sc.ScDataPilotItemObj
-o sc.ScDatabaseRangeObj
-o sc.ScDocumentConfiguration
-o sc.ScDrawPageObj
# ported to cppunit -o sc.ScHeaderFieldObj
-o sc.ScHeaderFieldsObj
# The two HeaderFooter tests seem to fail randomly
# -o sc.ScHeaderFooterTextCursor
# SHF_TextObj is composed of SHF_TextData, which has a weak reference to
# SHF_ContentObj, which itself has three references to SHF_TextObj.
# The css::text::XTextRange test fails often when the weak SHF_ContentObj is
# already gone. If just this test is disabled, later tests of this object fail
# too, so this disables the whole interface.
# -o sc.ScHeaderFooterTextObj
