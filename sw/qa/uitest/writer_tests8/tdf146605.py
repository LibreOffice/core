# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos

class tdf146605(UITestCase):

    def test_tdf146605(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf146605.odt")) as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.assertEqual(1, len(document.TextSections))
            self.assertTrue(document.TextSections.Section1.IsVisible)
            self.assertFalse(document.TextSections.Section1.FootnoteIsCollectAtTextEnd)
            self.assertFalse(document.TextSections.Section1.EndnoteIsCollectAtTextEnd)
            self.assertEqual("2", get_state_as_dict(xWriterEdit)["Pages"])

            with self.ui_test.execute_dialog_through_command(".uno:EditRegion") as xDialog:
                xTree = xDialog.getChild("tree")
                self.assertEqual("1", get_state_as_dict(xTree)['Children'])
                self.assertEqual("Section1", get_state_as_dict(xTree.getChild('0'))['Text'])
                self.assertEqual("Section1", get_state_as_dict(xDialog.getChild('curname'))['Text'])

                xOptions = xDialog.getChild("options")
                with self.ui_test.execute_blocking_action(
                        xOptions.executeAction, args=('CLICK', ())) as xOptDialog:
                    xTabs = xOptDialog.getChild("tabcontrol")
                    select_pos(xTabs, "3")

                    xTextEnd = xOptDialog.getChild("endntattextend")
                    xNum = xOptDialog.getChild("endntnum")

                    self.assertEqual("true", get_state_as_dict(xTextEnd)['Enabled'])
                    self.assertEqual("false", get_state_as_dict(xTextEnd)['Selected'])
                    self.assertEqual("false", get_state_as_dict(xNum)['Enabled'])

                    xTextEnd.executeAction('CLICK', tuple())

                    self.assertEqual("true", get_state_as_dict(xTextEnd)['Enabled'])
                    self.assertEqual("true", get_state_as_dict(xTextEnd)['Selected'])
                    self.assertEqual("true", get_state_as_dict(xNum)['Enabled'])

            self.assertEqual(1, len(document.TextSections))
            self.assertTrue(document.TextSections.Section1.IsVisible)
            self.assertFalse(document.TextSections.Section1.FootnoteIsCollectAtTextEnd)
            self.assertTrue(document.TextSections.Section1.EndnoteIsCollectAtTextEnd)

            # Without the fix in place, this test would have failed with
            # AssertionError: '1' != '2'
            self.assertEqual("1", get_state_as_dict(xWriterEdit)["Pages"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
