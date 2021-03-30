# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import os
import re
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

#test Hyperlink dialog
class HyperlinkDialog(UITestCase):

    def test_hyperlink_dialog_vertical_tab(self):

        self.ui_test.create_doc_in_start_center("writer")
        MainWindow = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:HyperlinkDialog")
        xDialog  = self.xUITest.getTopFocusWindow()

        # Test the vertical tab
        xtab=xDialog.getChild("tabcontrol")
        self.assertEqual(get_state_as_dict(xtab)["PageCount"], "4")

        xtab.executeAction("SELECT", mkPropertyValues({"POS": "0"}))
        self.assertEqual(get_state_as_dict(xtab)["CurrPageTitel"], "~Internet")
        self.assertEqual(get_state_as_dict(xtab)["CurrPagePos"], "0")

        xtab.executeAction("SELECT", mkPropertyValues({"POS": "1"}))
        self.assertEqual(get_state_as_dict(xtab)["CurrPageTitel"], "~Mail")
        self.assertEqual(get_state_as_dict(xtab)["CurrPagePos"], "1")

        xtab.executeAction("SELECT", mkPropertyValues({"POS": "2"}))
        self.assertEqual(get_state_as_dict(xtab)["CurrPageTitel"], "~Document")
        self.assertEqual(get_state_as_dict(xtab)["CurrPagePos"], "2")

        xtab.executeAction("SELECT", mkPropertyValues({"POS": "3"}))
        self.assertEqual(get_state_as_dict(xtab)["CurrPageTitel"], "~New Document")
        self.assertEqual(get_state_as_dict(xtab)["CurrPagePos"], "3")

        xcancel = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancel)

        self.ui_test.close_doc()

    def test_insert_hyperlink(self):

        self.ui_test.create_doc_in_start_center("writer")
        xMainWindow = self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:HyperlinkDialog")
        xDialog  = self.xUITest.getTopFocusWindow()

        # insert link
        xtab=xDialog.getChild("tabcontrol")
        xtab.executeAction("SELECT", mkPropertyValues({"POS": "0"}))

        xtarget = xDialog.getChild("target")
        xtarget.executeAction("TYPE", mkPropertyValues({"TEXT": "http://www.libreoffice.org/"}))
        self.assertEqual(get_state_as_dict(xtarget)["Text"], "http://www.libreoffice.org/")

        xindication = xDialog.getChild("indication")
        xindication.executeAction("TYPE", mkPropertyValues({"TEXT": "link"}))
        self.assertEqual(get_state_as_dict(xindication)["Text"], "link")

        xok = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xok)

        # Check that the link is added
        xMainWindow = self.xUITest.getTopFocusWindow()
        xedit = xMainWindow.getChild("writer_edit")
        xedit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "4"}))
        self.assertEqual(get_state_as_dict(xedit)["SelectedText"], "link")

        self.ui_test.close_doc()

    def test_tdf141166(self):
        # Skip this test for --with-help=html and --with-help=online, as that would fail with a
        # DialogNotExecutedException("did not execute a dialog for a blocking action") thrown from
        # the below execute_blocking_action call (and would leave behind the relevant HTML page
        # opened in the user's default browser):
        if os.getenv('ENABLE_HTMLHELP') == 'TRUE':
            return
        # Skip this test for --with-help, as that would fail with a
        # "uno.com.sun.star.uno.RuntimeException: Could not find child with id: cancel" thrown from
        # the below execute_blocking_action call, as it would open the "LibreOffice Help" window
        # instead of the apparently expected "LibreOffice Help Not Installed" dialog that has a
        # "Cancel" button:
        if re.compile(r'-DWITH_HELP\b').search(os.getenv('SCPDEFS')):
            return

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.ui_test.execute_dialog_through_command(".uno:HyperlinkDialog")
        xDialog  = self.xUITest.getTopFocusWindow()
        xHelp = xDialog.getChild("help")
        xHelp.executeAction('FOCUS', tuple())

        # Without the fix in place, this test would have crashed here
        self.ui_test.execute_blocking_action(xHelp.executeAction,
                args=("CLICK", tuple()), dialog_element="cancel")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
