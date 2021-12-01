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
from uitest.uihelper.common import get_url_for_data_file

#Bug 126017 - Crash swlo!SwNode::EndOfSectionIndex

class tdf126017(UITestCase):
   def test_tdf126017_crash_after_undo(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf126017.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            #go to TOC
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"aasasas"}))
                xsearch = xDialog.getChild("search")
                xsearch.executeAction("CLICK", tuple())
            #edit index
            with self.ui_test.execute_dialog_through_command(".uno:EditCurIndex") as xDiagIndex:
                title = xDiagIndex.getChild("title")
                title.executeAction("TYPE", mkPropertyValues({"TEXT":"aaaa"}))

            self.xUITest.executeCommand(".uno:Undo")

            with self.ui_test.execute_dialog_through_command(".uno:EditCurIndex") as xDiagIndex:
                title = xDiagIndex.getChild("title")
                title.executeAction("TYPE", mkPropertyValues({"TEXT":"aaaa"}))

            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual(writer_doc.Text.String.replace('\r\n', '\n')[1:7], "CRASHY")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
