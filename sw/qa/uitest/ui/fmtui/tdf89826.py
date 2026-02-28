# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
"""Tests for tdf#89826 - interactive property chips on Organizer tab."""

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import type_text
from libreoffice.uno.propertyvalue import mkPropertyValues


class tdf89826(UITestCase):

    def _find_and_click_first_chip(self, xDialog):
        """Navigate into propbox and click the remove button on the first chip.
        Returns True if a chip was found and clicked."""
        xPropBox = xDialog.getChild("propbox")
        for sCatName in xPropBox.getChildren():
            xCatRow = xPropBox.getChild(sCatName)
            if "chipsbox" not in xCatRow.getChildren():
                continue
            xChipsBox = xCatRow.getChild("chipsbox")
            for sRowName in xChipsBox.getChildren():
                xChipRow = xChipsBox.getChild(sRowName)
                for sChipName in xChipRow.getChildren():
                    xChip = xChipRow.getChild(sChipName)
                    if "removebar" in xChip.getChildren():
                        xRemoveBar = xChip.getChild("removebar")
                        xRemoveBar.executeAction("CLICK",
                            mkPropertyValues({"POS": "0"}))
                        return True
        return False

    def test_property_chip_reset(self):
        """Test that removing a property via chip resets it to parent value,
        and that undo restores it."""
        with self.ui_test.create_doc_in_start_center("writer"):
            document = self.ui_test.get_component()

            # Create a new style
            with self.ui_test.execute_dialog_through_command(
                    ".uno:StyleNewByExample") as xDialog:
                xStyleName = xDialog.getChild("stylename")
                type_text(xStyleName, "Test Chip Style")

            # Set parent and change font size in one dialog opening
            with self.ui_test.execute_dialog_through_command(
                    ".uno:EditStyle") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                # Tab 0 = Organizer: set parent
                select_pos(xTabs, "0")
                xLinkedWith = xTabs.getChild("linkedwith")
                xLinkedWith.executeAction("SELECT",
                    mkPropertyValues({"TEXT": "Default Paragraph Style"}))
                # Tab 1 = Character: change font size
                select_pos(xTabs, "1")
                xSizeWest = xTabs.getChild("cbWestSize")
                xSizeWest.executeAction("CLEAR", tuple())
                type_text(xSizeWest, "20pt")

            # Verify font size was applied
            xParaStyles = document.StyleFamilies.ParagraphStyles
            self.assertEqual(
                xParaStyles.getByName("Test Chip Style").getPropertyValue(
                    "CharHeight"), 20)

            # Open style dialog, switch to Edit mode, remove first chip
            with self.ui_test.execute_dialog_through_command(
                    ".uno:EditStyle") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xEditBtn = xDialog.getChild("editprops")
                xEditBtn.executeAction("CLICK", tuple())
                self.assertTrue(self._find_and_click_first_chip(xDialog),
                    "No property chip found to remove")

            # Property should now be inherited from parent
            xParent = xParaStyles.getByName("Standard")
            self.assertEqual(
                xParaStyles.getByName("Test Chip Style").getPropertyValue(
                    "CharHeight"),
                xParent.getPropertyValue("CharHeight"))

            # Undo should restore the removed property
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(
                xParaStyles.getByName("Test Chip Style").getPropertyValue(
                    "CharHeight"), 20)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
