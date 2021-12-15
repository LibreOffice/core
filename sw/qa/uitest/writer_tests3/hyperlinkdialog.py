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
from uitest.uihelper.common import get_state_as_dict, select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues

#test Hyperlink dialog
class HyperlinkDialog(UITestCase):

    def test_hyperlink_dialog_vertical_tab(self):

        with self.ui_test.create_doc_in_start_center("writer"):
            MainWindow = self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:HyperlinkDialog", close_button="cancel") as xDialog:

                # Test the vertical tab
                xtab=xDialog.getChild("tabcontrol")
                self.assertEqual(get_state_as_dict(xtab)["PageCount"], "4")

                select_pos(xtab, "0")
                self.assertEqual(get_state_as_dict(xtab)["CurrPageTitel"], "~Internet")
                self.assertEqual(get_state_as_dict(xtab)["CurrPagePos"], "0")

                select_pos(xtab, "1")
                self.assertEqual(get_state_as_dict(xtab)["CurrPageTitel"], "~Mail")
                self.assertEqual(get_state_as_dict(xtab)["CurrPagePos"], "1")

                select_pos(xtab, "2")
                self.assertEqual(get_state_as_dict(xtab)["CurrPageTitel"], "~Document")
                self.assertEqual(get_state_as_dict(xtab)["CurrPagePos"], "2")

                select_pos(xtab, "3")
                self.assertEqual(get_state_as_dict(xtab)["CurrPageTitel"], "~New Document")
                self.assertEqual(get_state_as_dict(xtab)["CurrPagePos"], "3")



    def test_insert_hyperlink(self):

        with self.ui_test.create_doc_in_start_center("writer"):
            xMainWindow = self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:HyperlinkDialog") as xDialog:

                # insert link
                xtab=xDialog.getChild("tabcontrol")
                select_pos(xtab, "0")

                xtarget = xDialog.getChild("target")
                xtarget.executeAction("TYPE", mkPropertyValues({"TEXT": "http://www.libreoffice.org/"}))
                self.assertEqual(get_state_as_dict(xtarget)["Text"], "http://www.libreoffice.org/")

                xindication = xDialog.getChild("indication")
                xindication.executeAction("TYPE", mkPropertyValues({"TEXT": "link"}))
                self.assertEqual(get_state_as_dict(xindication)["Text"], "link")


            # Check that the link is added
            xMainWindow = self.xUITest.getTopFocusWindow()
            xedit = xMainWindow.getChild("writer_edit")
            xedit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "4"}))
            self.assertEqual(get_state_as_dict(xedit)["SelectedText"], "link")


    def test_tdf141166(self):
        # Skip this test for --with-help=html and --with-help=online, as that would fail with a
        # DialogNotExecutedException("did not execute a dialog for a blocking action") thrown from
        # the below execute_blocking_action call (and would leave behind the relevant HTML page
        # opened in the user's default browser):
        if os.getenv('ENABLE_HTMLHELP') == 'TRUE':
            return
        # Skip this test for --enable-xmlhelp, as that would fail with a
        # "uno.com.sun.star.uno.RuntimeException: Could not find child with id: cancel" thrown from
        # the below execute_blocking_action call, as it would open the "LibreOffice Help" window
        # instead of the apparently expected "LibreOffice Help Not Installed" dialog that has a
        # "Cancel" button:
        if re.compile(r'XMLHELP\b').search(os.getenv('BUILD_TYPE')):
            return

        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            with self.ui_test.execute_dialog_through_command(".uno:HyperlinkDialog", close_button="") as xDialog:
                xHelp = xDialog.getChild("help")
                xHelp.executeAction('FOCUS', tuple())

                # Without the fix in place, this test would have crashed here
                with self.ui_test.execute_blocking_action(xHelp.executeAction,
                        args=("CLICK", tuple()), close_button="cancel"):
                    pass

# vim: set shiftwidth=4 softtabstop=4 expandtab:
