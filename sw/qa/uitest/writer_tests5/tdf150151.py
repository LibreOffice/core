# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import type_text

class Tdf150151(UITestCase):

    def test_tdf150151(self):

        sText = "Šđčćž ŠĐČĆŽ !”#$%&/()=?*,.-;:_  ° ~ˇ^˘°˛`˙’˝”¸"

        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, sText)

            self.xUITest.executeCommand(".uno:SelectAll")

            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("impress") as document:

            xDoc = self.xUITest.getTopFocusWindow()
            xEditWin = xDoc.getChild("impress_win")

            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            self.xUITest.executeCommand(".uno:Text")
            self.xUITest.executeCommand(".uno:Paste")

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Šđčćž ŠĐČĆŽ !”#$%&/()=?*,.-;:_  ° ~ˇ^˘°˛`˙’˝”¸' != 'Š  !#$%&/()=?*,.-;:_   ~^`'
            #self.assertEqual(sText, document.DrawPages[0].getByIndex(1).String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
