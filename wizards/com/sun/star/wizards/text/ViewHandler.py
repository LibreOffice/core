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
class ViewHandler(object):
    '''Creates a new instance of View '''

    def __init__ (self, xMSF, xTextDocument):
        self.xMSFDoc = xMSF
        self.xTextDocument = xTextDocument
        self.xTextViewCursorSupplier = xTextDocument.CurrentController

    def selectFirstPage(self, oTextTableHandler):
        try:
            xPageCursor = self.xTextViewCursorSupplier.ViewCursor
            xPageCursor.jumpToFirstPage()
            xPageCursor.jumpToStartOfPage()
            Helper.setUnoPropertyValue(
                xPageCursor, "PageDescName", "First Page")
            oPageStyles = self.xTextDocument.StyleFamilies.getByName(
                "PageStyles")
            oPageStyle = oPageStyles.getByName("First Page")
            xAllTextTables = oTextTableHandler.xTextTablesSupplier.TextTables
            xTextTable = xAllTextTables.getByIndex(0)
            xRange = xTextTable.Anchor.Text
            xPageCursor.gotoRange(xRange, False)
            if not com.sun.star.uno.AnyConverter.isVoid(XTextRange):
                xViewTextCursor.gotoRange(xHeaderRange, False)
                xViewTextCursor.collapseToStart()
            else:
                print "No Headertext available"

        except com.sun.star.uno.Exception, exception:
            exception.printStackTrace(System.out)

    def setViewSetting(self, Setting, Value):
        self.xTextViewCursorSupplier.ViewSettings.setPropertyValue(Setting, Value)

    def collapseViewCursorToStart(self):
        xTextViewCursor = self.xTextViewCursorSupplier.ViewCursor
        xTextViewCursor.collapseToStart()
