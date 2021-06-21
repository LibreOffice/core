# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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
            document = self.ui_test.get_component()
            xWriterDoc = self.xUITest.getTopFocusWindow()

            #go to TOC
            self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
            xDialog = self.xUITest.getTopFocusWindow()
            searchterm = xDialog.getChild("searchterm")
            searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"aasasas"}))
            xsearch = xDialog.getChild("search")
            xsearch.executeAction("CLICK", tuple())
            xcloseBtn = xDialog.getChild("close")
            self.ui_test.close_dialog_through_button(xcloseBtn)
            #edit index
            self.ui_test.execute_dialog_through_command(".uno:EditCurIndex")  #open index dialog
            xDiagIndex = self.xUITest.getTopFocusWindow()
            xOKBtn = xDiagIndex.getChild("ok")
            title = xDiagIndex.getChild("title")
            title.executeAction("TYPE", mkPropertyValues({"TEXT":"aaaa"}))
            self.ui_test.close_dialog_through_button(xOKBtn)

            self.xUITest.executeCommand(".uno:Undo")

            self.ui_test.execute_dialog_through_command(".uno:EditCurIndex")  #open index dialog
            xDiagIndex = self.xUITest.getTopFocusWindow()
            xOKBtn = xDiagIndex.getChild("ok")
            title = xDiagIndex.getChild("title")
            title.executeAction("TYPE", mkPropertyValues({"TEXT":"aaaa"}))
            self.ui_test.close_dialog_through_button(xOKBtn)

            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual(document.Text.String.replace('\r\n', '\n')[1:7], "CRASHY")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
