#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict

# Test for cui/source/tabpages/chardlg.cxx.
class Test(UITestCase):

    def testSvxCharEffectsPage(self):
        # Start Impress.
        self.ui_test.create_doc_in_start_center("impress")
        template = self.xUITest.getTopFocusWindow()
        self.ui_test.close_dialog_through_button(template.getChild("close"))
        doc = self.xUITest.getTopFocusWindow()
        editWin = doc.getChild("impress_win")
        # Select the title shape.
        editWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        editWin.executeAction("TYPE", mkPropertyValues({"TEXT": "t"}))
        self.xUITest.executeCommand(".uno:SelectAll")

        # Now use Format -> Character.
        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        # Select RID_SVXPAGE_CHAR_EFFECTS.
        select_pos(xTabs, "1")
        xFontTransparency = xDialog.getChild("fonttransparencymtr")
        for _ in range(5):
            xFontTransparency.executeAction("UP", tuple())
        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))

        # Verify the result.
        component = self.ui_test.get_component()
        drawPage = component.getDrawPages().getByIndex(0)
        shape = drawPage.getByIndex(0)

        # Without the accompanying fix in place, this test would have failed with:
        # AssertionError: 100 != 5
        # i.e. the dialog did not set transparency to 5%, instead it left the character color at
        # COL_AUTO.
        self.assertEqual(shape.CharTransparence, 5)
        self.ui_test.close_doc()

    def testSvxCharEffectsPageWriter(self):
        # Start Writer.
        self.ui_test.create_doc_in_start_center("writer")
        doc = self.xUITest.getTopFocusWindow()
        editWin = doc.getChild("writer_edit")
        # Type a character and select it.
        editWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        editWin.executeAction("TYPE", mkPropertyValues({"TEXT": "t"}))
        self.xUITest.executeCommand(".uno:SelectAll")

        # Now use Format -> Character.
        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        # Select RID_SVXPAGE_CHAR_EFFECTS.
        select_pos(xTabs, "1")
        xFontTransparency = xDialog.getChild("fonttransparencymtr")
        # Without the accompanying fix in place, this test would have failed with:
        # AssertionError: 'false' != 'true'
        # i.e. the transparency widget was hidden.
        self.assertEqual(get_state_as_dict(xFontTransparency)["Visible"], "true")
        for _ in range(5):
            xFontTransparency.executeAction("UP", tuple())
        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))

        # Verify the result.
        component = self.ui_test.get_component()
        paragraph = component.Text.createEnumeration().nextElement()

        self.assertEqual(paragraph.CharTransparence, 5)
        self.ui_test.close_doc()

    def testSvxCharEffectsPageWriterAutomatic(self):
        # Start Writer.
        self.ui_test.create_doc_in_start_center("writer")
        doc = self.xUITest.getTopFocusWindow()
        editWin = doc.getChild("writer_edit")

        # Use Format -> Character.
        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        # Select RID_SVXPAGE_CHAR_EFFECTS.
        select_pos(xTabs, "1")
        xFontColorLB = xDialog.getChild("fontcolorlb")
        # Without the accompanying fix in place, this test would have failed with:
        # AssertionError: 'White' != 'Automatic'
        # i.e. the auto color lost its alpha component and appeared as white.
        self.assertEqual(get_state_as_dict(xFontColorLB)["Text"], "Automatic")

        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
