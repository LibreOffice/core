# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time
from uitest.uihelper.common import get_state_as_dict, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep

#test Find Bar
class FindBar(UITestCase):

    def test_find_bar(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        # Type some lines to search for words on them
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "LibreOffice"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "LibreOffice Writer"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "LibreOffice Calc"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "The Document Foundation"}))

        # open the Find Bar
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+f"}))

        # Type the Word that we want to search for it
        xfind = xWriterDoc.getChild("find")
        xfind.executeAction("TYPE", mkPropertyValues({"TEXT": "Libre"}))

        # Select the Find Bar
        xfind_bar = xWriterDoc.getChild("FindBar")
        self.assertEqual(get_state_as_dict(xfind_bar)["ItemCount"], "14")

        # Press on FindAll in the Find Bar
        xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "4"}))
        self.assertEqual(get_state_as_dict(xfind_bar)["CurrSelectedItemID"], "5") # 5 is FindAll id for Pos 4
        self.assertEqual(get_state_as_dict(xfind_bar)["CurrSelectedItemText"], "Find All")
        self.assertEqual(get_state_as_dict(xfind_bar)["CurrSelectedItemCommand"], ".uno:FindAll")
        self.assertEqual(get_state_as_dict(xWriterEdit)["SelectedText"], "LibreLibreLibre")

        # Press on Find Next in the Find Bar
        xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "3"}))  # 3 is Find Next pos
        self.assertEqual(get_state_as_dict(xfind_bar)["CurrSelectedItemID"], "4")
        self.assertEqual(get_state_as_dict(xfind_bar)["CurrSelectedItemText"], "Find Next")
        self.assertEqual(get_state_as_dict(xfind_bar)["CurrSelectedItemCommand"], ".uno:DownSearch")
        self.assertEqual(get_state_as_dict(xWriterEdit)["SelectedText"], "Libre")

        # Press on Find Previous in the Find Bar
        xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "2"}))  # 2 is Find Previous pos
        self.assertEqual(get_state_as_dict(xfind_bar)["CurrSelectedItemID"], "3")
        self.assertEqual(get_state_as_dict(xfind_bar)["CurrSelectedItemText"], "Find Previous")
        self.assertEqual(get_state_as_dict(xfind_bar)["CurrSelectedItemCommand"], ".uno:UpSearch")
        self.assertEqual(get_state_as_dict(xWriterEdit)["SelectedText"], "Libre")

        # Close the Find Bar
        xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "0"}))  # 0 is pos for close

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
