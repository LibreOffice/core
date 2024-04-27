# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf115871(UITestCase):

   def test_tdf115871_reset_outline_list_options_parent(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            # Create new style from selection
            with self.ui_test.execute_dialog_through_command(".uno:StyleNewByExample") as xDialog:
                xStyleName = xDialog.getChild("stylename")
                type_text(xStyleName, "Custom Heading 1")

            # Modify the created style to inherit paragraph attributes from Heading 1
            with self.ui_test.execute_dialog_through_command(".uno:EditStyle") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xLinkedWith = xTabs.getChild("linkedwith")
                xLinkedWith.executeAction("SELECT", mkPropertyValues({"TEXT": "Heading 1"}))
                self.assertEqual(get_state_as_dict(xLinkedWith)["SelectEntryText"], "Heading 1")

            # Modify outline & list settings of the created style
            with self.ui_test.execute_dialog_through_command(".uno:EditStyle") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "13")

                # Outline level
                xOutlineLevel = xTabs.getChild("comboLB_OUTLINE_LEVEL")
                xOutlineLevel.executeAction("SELECT", mkPropertyValues({"TEXT": "Level 5"}))
                self.assertEqual(get_state_as_dict(xOutlineLevel)["SelectEntryText"], "Level 5")

                # List style
                xListStyle = xTabs.getChild("comboLB_NUMBER_STYLE")
                xListStyle.executeAction("SELECT", mkPropertyValues({"TEXT": "Numbering 123"}))
                self.assertEqual(get_state_as_dict(xListStyle)["SelectEntryText"], "Numbering 123")

                # Line numbering including start with
                xRestartAtParagraph = xTabs.getChild("checkCB_RESTART_PARACOUNT")
                xRestartAtParagraph.executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xRestartAtParagraph)["Selected"], "true")
                xStartWith = xTabs.getChild("spinNF_RESTART_PARA")
                xStartWith.executeAction("UP", tuple())
                self.assertEqual(get_state_as_dict(xStartWith)["Text"], "2")

            # Open the paragraph style dialog and reset dialog to parent settings
            with self.ui_test.execute_dialog_through_command(".uno:EditStyle") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "13")

                xStandardButton = xDialog.getChild("standard")
                xStandardButton.executeAction("CLICK", tuple())

                xOutlineLevel = xTabs.getChild("comboLB_OUTLINE_LEVEL")
                # Without the fix in place, this test would have failed with
                # AssertionError: 'Level 5' != 'Level 1'
                self.assertEqual(get_state_as_dict(xOutlineLevel)["SelectEntryText"], "Level 1")

                xListStyle = xTabs.getChild("comboLB_NUMBER_STYLE")
                # Without the fix in place, this test would have failed with
                # AssertionError: 'Numbering 123' != 'Heading Numbering'
                self.assertEqual(get_state_as_dict(xListStyle)["SelectEntryText"], "Heading Numbering")

                xRestartAtParagraph = xTabs.getChild("checkCB_RESTART_PARACOUNT")
                # Without the fix in place, this test would have failed with
                # AssertionError: 'true' != 'false'
                self.assertEqual(get_state_as_dict(xRestartAtParagraph)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xListStyle)["SelectEntryText"], "Heading Numbering")

                xStartWith = xTabs.getChild("spinNF_RESTART_PARA")
                # Without the fix in place, this test would have failed with
                # AssertionError: '2' != '1'
                self.assertEqual(get_state_as_dict(xStartWith)["Text"], "1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
