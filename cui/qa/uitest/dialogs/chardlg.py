#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_by_text

# Test for cui/source/tabpages/chardlg.cxx.
class Test(UITestCase):

    def testSvxCharEffectsPage(self):
        # Start Impress.
        with self.ui_test.create_doc_in_start_center("impress") as component:
            template = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(template.getChild("close"))
            doc = self.xUITest.getTopFocusWindow()
            editWin = doc.getChild("impress_win")
            # Select the title shape.
            editWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            editWin.executeAction("TYPE", mkPropertyValues({"TEXT": "t"}))
            self.xUITest.executeCommand(".uno:SelectAll")

            # Now use Format -> Character.
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                # Select RID_SVXPAGE_CHAR_EFFECTS.
                select_pos(xTabs, "1")
                xFontTransparency = xDialog.getChild("fonttransparencymtr")
                for _ in range(5):
                    xFontTransparency.executeAction("UP", tuple())

            # Verify the result.
            drawPage = component.getDrawPages()[0]
            shape = drawPage[0]

            # Without the accompanying fix in place, this test would have failed with:
            # AssertionError: 100 != 5
            # i.e. the dialog did not set transparency to 5%, instead it left the character color at
            # COL_AUTO.
            self.assertEqual(shape.CharTransparence, 5)

    def testSvxCharEffectsPageTheme(self):
        # Given a document with a document theme:
        # Start Impress.
        with self.ui_test.create_doc_in_start_center("impress") as component:
            template = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(template.getChild("close"))
            doc = self.xUITest.getTopFocusWindow()
            editWin = doc.getChild("impress_win")

            # Set theme colors.
            drawPage = component.getDrawPages()[0]
            master = drawPage.MasterPage
            theme = mkPropertyValues({
                "Name": "nameA",
                "ColorSchemeName": "colorSetA",
                "ColorScheme": tuple([
                    0x000000,  # dk1
                    0x000000,  # lt1
                    0x000000,  # dk2
                    0x000000,  # lt2
                    0x000000,  # accent1
                    0x000000,  # accent2
                    0x000000,  # accent3
                    0x000000,  # accent4
                    0x000000,  # accent5
                    0x000000,  # accent6
                    0x000000,  # hlink
                    0x000000,  # folHlink
                ])
            })
            master.ThemeUnoRepresentation = theme

            # Select the title shape.
            editWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            editWin.executeAction("TYPE", mkPropertyValues({"TEXT": "t"}))
            self.xUITest.executeCommand(".uno:SelectAll")

            # Now use Format -> Character.
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                # Select RID_SVXPAGE_CHAR_EFFECTS.
                select_pos(xTabs, "1")

                # When setting the shape text color to accent 1:
                accent1 = xDialog.getChild("fontcolorlb")
                accent1.executeAction("OPENLIST", tuple())
                floatWindow = self.xUITest.getFloatWindow()
                paletteSelector = floatWindow.getChild("palette_listbox")
                select_by_text(paletteSelector, "Theme colors")
                colorIconView = floatWindow.getChild("colorwindow_iv_colors")
                # 4 would be accent1, +12 is the first from the effect variants.
                color_element = colorIconView.getChild("16")
                color_element.executeAction("SELECT", mkPropertyValues({}))
                color_element.executeAction("DOUBLECLICK", tuple())
                self.assertEqual(get_state_as_dict(colorIconView)["SelectEntryText"], "Accent 1, 50% Lighter")

            # Then make sure the doc model has the correct color theme index:
            drawPage = component.getDrawPages()[0]
            shape = drawPage[0]
            paragraphs = shape.createEnumeration()
            paragraph = paragraphs.nextElement()
            portions = paragraph.createEnumeration()
            portion = portions.nextElement()

            # Without the accompanying fix in place, this test would have failed with:
            # AssertionError: -1 != 4
            # i.e. no theme index was set, instead of accent1 (index into the above color scheme).
            self.assertEqual(portion.CharColorTheme, 4)

            # Then make sure that '80% lighter' is lum-mod=2000 and lum-off=8000:
            # Without the accompanying fix in place, this test would have failed with:
            # AssertionError: 10000 != 2000
            # i.e. the effects where not applied, luminance modulation was the default instead of a
            # custom value.
            self.assertEqual(portion.CharColorLumMod, 5000)
            self.assertEqual(portion.CharColorLumOff, 5000)

    def testSvxCharEffectsPageWriter(self):
        # Start Writer.
        with self.ui_test.create_doc_in_start_center("writer") as component:
            doc = self.xUITest.getTopFocusWindow()
            editWin = doc.getChild("writer_edit")
            # Type a character and select it.
            editWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            editWin.executeAction("TYPE", mkPropertyValues({"TEXT": "t"}))
            self.xUITest.executeCommand(".uno:SelectAll")

            # Now use Format -> Character.
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
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

            # Verify the result.
            paragraph = component.Text.createEnumeration().nextElement()

            self.assertEqual(paragraph.CharTransparence, 5)

    def testSvxCharEffectsPageWriterAutomatic(self):
        # Start Writer.
        with self.ui_test.create_doc_in_start_center("writer"):
            # Use Format -> Character.
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                # Select RID_SVXPAGE_CHAR_EFFECTS.
                select_pos(xTabs, "1")
                xFontColorLB = xDialog.getChild("fontcolorlb")
                # Without the accompanying fix in place, this test would have failed with:
                # AssertionError: 'White' != 'Automatic'
                # i.e. the auto color lost its alpha component and appeared as white.
                self.assertEqual(get_state_as_dict(xFontColorLB)["Text"], "Automatic")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
