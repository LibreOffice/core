# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

# Bundled by external/more_fonts, and asserted on by vcl/qa/cppunit/FontVariationTest.cxx
# and vcl/qa/cppunit/logicalfontinstance.cxx: Reem Kufi has a single wght axis, 400..700,
# with named instances at 400/500/600/700, Amiri has no axes.
VARIABLE_FONT = "Reem Kufi"
STATIC_FONT = "Amiri"
# A value that is not a named instance, so it has to be stored as an explicit setting.
OFF_INSTANCE = "650"


# Tests for the font variations popover, cui/source/dialogs/FontVariationsPopup.cxx.
class FontVariations(UITestCase):
    def hasFont(self, sName):
        xToolkit = self.xContext.ServiceManager.createInstanceWithContext(
            "com.sun.star.awt.Toolkit", self.xContext)
        xDevice = xToolkit.createScreenCompatibleDevice(100, 100)
        return any(aDescriptor.Name == sName for aDescriptor in xDevice.FontDescriptors)

    def setUp(self):
        super().setUp()
        if not self.hasFont(VARIABLE_FONT) or not self.hasFont(STATIC_FONT):
            self.skipTest("bundled fonts are not available")

    def typeText(self):
        xWriterEdit = self.xUITest.getTopFocusWindow().getChild("writer_edit")
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Hello"}))
        self.xUITest.executeCommand(".uno:SelectAll")

    def setFontName(self, xDialog, sName):
        xDialog.getChild("edWestFontName").executeAction(
            "SET", mkPropertyValues({"TEXT": sName}))

    def setComboText(self, xDialog, sId, sText):
        xCombo = xDialog.getChild(sId)
        xCombo.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
        xCombo.executeAction("TYPE", mkPropertyValues({"TEXT": sText}))

    def openPopover(self, xDialog):
        xDialog.getChild("btnWestVariations").executeAction("CLICK", tuple())
        return self.xUITest.getFloatWindow()

    def setAxisValue(self, xPopover, sValue):
        xPopover.getChild("spin-wght").executeAction(
            "VALUE", mkPropertyValues({"VALUE": sValue}))

    def test_button_follows_font(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with self.ui_test.execute_dialog_through_command(
                    ".uno:FontDialog", close_button="cancel") as xDialog:
                xButton = xDialog.getChild("btnWestVariations")

                self.setFontName(xDialog, VARIABLE_FONT)
                self.assertEqual("true", get_state_as_dict(xButton)["Enabled"])

                self.setFontName(xDialog, STATIC_FONT)
                self.assertEqual("false", get_state_as_dict(xButton)["Enabled"])

    def test_popover_shows_axis_default(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with self.ui_test.execute_dialog_through_command(
                    ".uno:FontDialog", close_button="cancel") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)

                xPopover = self.openPopover(xDialog)
                # The label carries an auto-generated mnemonic marker.
                sLabel = get_state_as_dict(xPopover.getChild("label-wght"))["Text"]
                self.assertEqual("Weight", sLabel.replace("~", ""))
                self.assertEqual("400", get_state_as_dict(xPopover.getChild("spin-wght"))["Value"])

    def test_popover_shows_named_instance_value(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with self.ui_test.execute_dialog_through_command(
                    ".uno:FontDialog", close_button="cancel") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)
                self.setComboText(xDialog, "cbWestStyle", "Bold")

                xPopover = self.openPopover(xDialog)
                self.assertEqual("700", get_state_as_dict(xPopover.getChild("spin-wght"))["Value"])

    def test_popover_sets_variations(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            self.typeText()

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)
                self.setAxisValue(self.openPopover(xDialog), OFF_INSTANCE)

            self.assertEqual('"wght" 650', component.CurrentSelection[0].CharFontVariations)

    def test_popover_value_matching_instance_picks_style(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            self.typeText()

            # wght=700 is a named instance, so it is stored as the font style
            # rather than as an explicit setting.
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)
                xPopover = self.openPopover(xDialog)
                self.setAxisValue(xPopover, "700")
                self.assertEqual("Bold", get_state_as_dict(xDialog.getChild("cbWestStyle"))["Text"])

            self.assertEqual("", component.CurrentSelection[0].CharFontVariations)
            self.assertEqual(150.0, component.CurrentSelection[0].CharWeight)

    def test_off_instance_value_clears_style(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            self.typeText()

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)
                xPopover = self.openPopover(xDialog)
                self.setAxisValue(xPopover, "700")
                self.assertEqual("Bold", get_state_as_dict(xDialog.getChild("cbWestStyle"))["Text"])

                # No named instance describes this, so the style must not go on
                # claiming Bold.
                self.setAxisValue(xPopover, OFF_INSTANCE)
                self.assertEqual("", get_state_as_dict(xDialog.getChild("cbWestStyle"))["Text"])

            self.assertEqual('"wght" 650', component.CurrentSelection[0].CharFontVariations)
            self.assertEqual(100.0, component.CurrentSelection[0].CharWeight)

    def test_popover_default_value_picks_regular(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            self.typeText()

            # The default instance reports no coordinates of its own, but wght=400
            # still describes it.
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)
                xPopover = self.openPopover(xDialog)
                self.setAxisValue(xPopover, "700")
                self.assertEqual("Bold", get_state_as_dict(xDialog.getChild("cbWestStyle"))["Text"])

                self.setAxisValue(xPopover, "400")
                self.assertEqual("Regular",
                                 get_state_as_dict(xDialog.getChild("cbWestStyle"))["Text"])

            self.assertEqual("", component.CurrentSelection[0].CharFontVariations)
            self.assertEqual(100.0, component.CurrentSelection[0].CharWeight)

    def test_style_change_drops_variations(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            self.typeText()

            # Picking a style is picking a named instance, so the popover has to
            # come back up showing that instance and not the dropped settings.
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)
                self.setAxisValue(self.openPopover(xDialog), OFF_INSTANCE)
                self.setComboText(xDialog, "cbWestStyle", "Bold")

                xPopover = self.openPopover(xDialog)
                self.assertEqual("700", get_state_as_dict(xPopover.getChild("spin-wght"))["Value"])

            self.assertEqual("", component.CurrentSelection[0].CharFontVariations)
            self.assertEqual(150.0, component.CurrentSelection[0].CharWeight)

    def test_named_instance_writes_no_variations(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            self.typeText()

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)
                self.setComboText(xDialog, "cbWestStyle", "Bold")

            self.assertEqual("", component.CurrentSelection[0].CharFontVariations)
            self.assertEqual(150.0, component.CurrentSelection[0].CharWeight)

    def test_variations_survive_size_change(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            self.typeText()

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)
                self.setAxisValue(self.openPopover(xDialog), OFF_INSTANCE)
                self.setComboText(xDialog, "cbWestSize", "18")

            self.assertEqual('"wght" 650', component.CurrentSelection[0].CharFontVariations)

    def test_untouched_font_gets_no_variations(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            self.typeText()

            # Picking a variable font without ever opening the popover must not
            # pin the axes in the document.
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)
                self.setComboText(xDialog, "cbWestSize", "18")

            self.assertEqual("", component.CurrentSelection[0].CharFontVariations)

    def test_font_change_drops_variations(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            self.typeText()

            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                self.setFontName(xDialog, VARIABLE_FONT)
                self.setAxisValue(self.openPopover(xDialog), OFF_INSTANCE)

            self.assertEqual('"wght" 650', component.CurrentSelection[0].CharFontVariations)

            # Settings of the old font don't carry over to a font without axes.
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                self.setFontName(xDialog, STATIC_FONT)

            self.assertEqual("", component.CurrentSelection[0].CharFontVariations)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
