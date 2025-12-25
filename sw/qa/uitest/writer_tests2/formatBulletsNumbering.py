# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

from libreoffice.uno.propertyvalue import mkPropertyValues, convert_property_values_to_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import change_measurement_unit

class formatBulletsNumbering(UITestCase):

    def test_bullets_and_numbering_change_bullet(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            # Set the "black down-pointing triangle" bullet in the third item
            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                # Select the BulletPage's selector
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xBulletPage = xDialog.getChild("PickBulletPage")
                xSelector = xBulletPage.getChild("valueset")

                # Select element number 3
                xSelector.executeAction("CHOOSE", mkPropertyValues({"POS": "3"}))
                self.assertEqual(get_state_as_dict(xSelector)["SelectedItemId"], "3")
                xChangeBulletBtn = xBulletPage.getChild("changeBulletBtn")
                with self.ui_test.execute_blocking_action(xChangeBulletBtn.executeAction, args=('CLICK', ())) as xCharSetDialog:
                    xCharSet = xCharSetDialog.getChild("showcharset")
                    xCharSet.executeAction("SELECT", mkPropertyValues({"COLUMN": "21", "ROW": "1"}))

            # Check that the "black down-pointing triangle" bullet is the third item
            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                # Select the BulletPage's selector
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xBulletPage = xDialog.getChild("PickBulletPage")
                xSelector = xBulletPage.getChild("valueset")

                # Select element number 3
                xSelector.executeAction("CHOOSE", mkPropertyValues({"POS": "3"}))
                self.assertEqual(get_state_as_dict(xSelector)["SelectedItemId"], "3")
                xChangeBulletBtn = xBulletPage.getChild("changeBulletBtn")
                with self.ui_test.execute_blocking_action(xChangeBulletBtn.executeAction, args=('CLICK', ())) as xCharSetDialog:
                    xHexText = xCharSetDialog.getChild("hexvalue")
                    xDecText = xCharSetDialog.getChild("decimalvalue")
                    # Check the "black down-pointing triangle" bullet Hex and Decimal value
                    self.assertEqual(get_state_as_dict(xHexText)["Text"], "25BC")
                    self.assertEqual(get_state_as_dict(xDecText)["Text"], "9660")


    def test_bullets_and_numbering_dialog_tab_position(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with change_measurement_unit(self, "Millimeter"):

                with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")
                    xalignedatmf = xDialog.getChild("alignedatmf")
                    xnum2alignlb = xDialog.getChild("num2alignlb")
                    xatmf = xDialog.getChild("atmf")
                    xindentatmf = xDialog.getChild("indentatmf")

                    xalignedatmf.executeAction("UP", tuple())
                    select_by_text(xnum2alignlb, "Centered")
                    xatmf.executeAction("UP", tuple())
                    xindentatmf.executeAction("UP", tuple())


                with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog", close_button="ok") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")
                    xalignedatmf = xDialog.getChild("alignedatmf")
                    xnum2alignlb = xDialog.getChild("num2alignlb")
                    xatmf = xDialog.getChild("atmf")
                    xindentatmf = xDialog.getChild("indentatmf")

                    self.assertEqual(get_state_as_dict(xalignedatmf)["Text"], "6.5 mm")
                    self.assertEqual(get_state_as_dict(xnum2alignlb)["SelectEntryText"], "Centered")
                    self.assertEqual(get_state_as_dict(xatmf)["Text"], "12.8 mm")
                    self.assertEqual(get_state_as_dict(xindentatmf)["Text"], "12.8 mm")



    def test_bullets_and_numbering_dialog_tab_position2(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")
                xnumfollowedbylb = xDialog.getChild("numfollowedbylb")
                select_by_text(xnumfollowedbylb, "Space")


            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog", close_button="ok") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")
                xnumfollowedbylb = xDialog.getChild("numfollowedbylb")
                self.assertEqual(get_state_as_dict(xnumfollowedbylb)["SelectEntryText"], "Space")



    def test_bullets_and_numbering_dialog_tab_customize(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")
                xnumfmtlb = xDialog.getChild("numfmtlb")
                xstartat = xDialog.getChild("startat")
                xcharstyle = xDialog.getChild("charstyle")
                xprefix = xDialog.getChild("prefix")
                xsuffix = xDialog.getChild("suffix")
                xallsame = xDialog.getChild("allsame")

                select_by_text(xnumfmtlb, "A, B, C, ...")
                xstartat.executeAction("UP", tuple())
                select_by_text(xcharstyle, "Bullets")
                xprefix.executeAction("TYPE", mkPropertyValues({"TEXT":"o"}))
                xsuffix.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xsuffix.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
                xallsame.executeAction("CLICK", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog", close_button="ok") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")
                xnumfmtlb = xDialog.getChild("numfmtlb")
                xstartat = xDialog.getChild("startat")
                xcharstyle = xDialog.getChild("charstyle")
                xprefix = xDialog.getChild("prefix")
                xsuffix = xDialog.getChild("suffix")
                xallsame = xDialog.getChild("allsame")

                self.assertEqual(get_state_as_dict(xnumfmtlb)["SelectEntryText"], "A, B, C, ...")
                self.assertEqual(get_state_as_dict(xstartat)["Text"], "2")
                self.assertEqual(get_state_as_dict(xcharstyle)["SelectEntryText"], "Bullets")
                self.assertEqual(get_state_as_dict(xprefix)["Text"], "o")
                self.assertEqual(get_state_as_dict(xsuffix)["Text"], "a")
                self.assertEqual(get_state_as_dict(xallsame)["Selected"], "true")


            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog", close_button="user") as xDialog:
                pass

            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog", close_button="ok") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")
                xnumfmtlb = xDialog.getChild("numfmtlb")
                xstartat = xDialog.getChild("startat")
                xcharstyle = xDialog.getChild("charstyle")
                xprefix = xDialog.getChild("prefix")
                xsuffix = xDialog.getChild("suffix")
                xallsame = xDialog.getChild("allsame")

                self.assertEqual(get_state_as_dict(xnumfmtlb)["SelectEntryText"], "1, 2, 3, ...")
                self.assertEqual(get_state_as_dict(xstartat)["Text"], "1")
                self.assertEqual(get_state_as_dict(xcharstyle)["SelectEntryText"], "None")
                self.assertEqual(get_state_as_dict(xprefix)["Text"], "")
                self.assertEqual(get_state_as_dict(xsuffix)["Text"], ".")
                self.assertEqual(get_state_as_dict(xallsame)["Selected"], "false")



    def test_bullets_and_numbering_tab_move(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with change_measurement_unit(self, "Millimeter"):
                xWriterDoc = self.xUITest.getTopFocusWindow()
                xWriterEdit = xWriterDoc.getChild("writer_edit")

                with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")
                    xindentatmf = xDialog.getChild("indentatmf")
                    indentValue = get_state_as_dict(xindentatmf)["Text"][0:len(get_state_as_dict(xindentatmf)["Text"])-3]


                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"})) #TAB to move indent right
                with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog", close_button="ok") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")
                    xindentatmf = xDialog.getChild("indentatmf")
                    indentValue2 = get_state_as_dict(xindentatmf)["Text"][0:len(get_state_as_dict(xindentatmf)["Text"])-3]
                    self.assertEqual(indentValue < indentValue2 , True)

                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
                with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog", close_button="ok") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")
                    xindentatmf = xDialog.getChild("indentatmf")
                    indentValue3 = get_state_as_dict(xindentatmf)["Text"][0:len(get_state_as_dict(xindentatmf)["Text"])-3]
                    self.assertEqual(indentValue == indentValue3 , True)


    def test_bullets_and_numbering_button_move(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with change_measurement_unit(self, "Millimeter"):

                with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")
                    xindentatmf = xDialog.getChild("indentatmf")
                    indentValue = get_state_as_dict(xindentatmf)["Text"][0:len(get_state_as_dict(xindentatmf)["Text"])-3]


                self.xUITest.executeCommand(".uno:DecrementSubLevels")
                with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog", close_button="ok") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")
                    xindentatmf = xDialog.getChild("indentatmf")
                    indentValue2 = get_state_as_dict(xindentatmf)["Text"][0:len(get_state_as_dict(xindentatmf)["Text"])-3]
                    self.assertEqual(indentValue < indentValue2 , True)

                self.xUITest.executeCommand(".uno:IncrementLevel")
                with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog", close_button="ok") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")
                    xindentatmf = xDialog.getChild("indentatmf")
                    indentValue3 = get_state_as_dict(xindentatmf)["Text"][0:len(get_state_as_dict(xindentatmf)["Text"])-3]
                    self.assertEqual(indentValue == indentValue3 , True)


    def test_bullets_and_numbering_selection(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            # Test Bullet Page
            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:

                # Select the BulletPage's Selector
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xBulletPage = xDialog.getChild("PickBulletPage")
                xselector = xBulletPage.getChild("valueset")
                self.assertEqual(get_state_as_dict(xselector)["ItemsCount"], "8")
                # Select element num 3
                xselector.executeAction("CHOOSE", mkPropertyValues({"POS": "3"}))
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemPos"], "2")
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemId"], "3")
                # Select element num 7
                xselector.executeAction("CHOOSE", mkPropertyValues({"POS": "7"}))
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemPos"], "6")
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemId"], "7")

            # Test other Pages
            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                # Select the NumberingPage's Selector
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")
                xNumberingPage = xDialog.getChild("PickNumberingPage")
                xselector = xNumberingPage.getChild("valueset")
                self.assertEqual(get_state_as_dict(xselector)["ItemsCount"], "8")
                # Select element num 5
                xselector.executeAction("CHOOSE", mkPropertyValues({"POS": "5"}))
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemPos"], "4")
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemId"], "5")
                self.assertEqual(get_state_as_dict(xselector)["ItemText"], "Uppercase letter A) B) C)")
                # Select element num 8
                xselector.executeAction("CHOOSE", mkPropertyValues({"POS": "8"}))
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemPos"], "7")
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemId"], "8")
                self.assertEqual(get_state_as_dict(xselector)["ItemText"], "Lowercase Roman number i. ii. iii.")

                # Select the OutlinePage's Selector
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")
                xOutlinePage = xDialog.getChild("PickOutlinePage")
                xselector = xOutlinePage.getChild("valueset")
                self.assertEqual(get_state_as_dict(xselector)["ItemsCount"], "8")
                # Select element num 1
                xselector.executeAction("CHOOSE", mkPropertyValues({"POS": "1"}))
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemPos"], "0")
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemId"], "1")
                self.assertEqual(get_state_as_dict(xselector)["ItemText"], "Uppercase Roman, uppercase letters, numeric, lowercase letters, lowercase Roman, solid small circular bullet")

                # Select the GraphicPage's Selector
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")
                xGraphicPage = xDialog.getChild("PickGraphicPage")
                xselector = xGraphicPage.getChild("valueset")
                self.assertEqual(get_state_as_dict(xselector)["ItemsCount"], "25")
                # Select element num 12
                xselector.executeAction("CHOOSE", mkPropertyValues({"POS": "12"}))
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemPos"], "11")
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemId"], "12")
                # Select element num 23
                xselector.executeAction("CHOOSE", mkPropertyValues({"POS": "23"}))
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemPos"], "22")
                self.assertEqual(get_state_as_dict(xselector)["SelectedItemId"], "23")


    def test_bullets_and_numbering_document_bullet_list(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            self.xUITest.executeCommand(".uno:DefaultBullet")
            # Without the fix in place, this test would have crashed here
            self.xUITest.executeCommand(".uno:DocumentBulletList")

    def test_bullets_and_numbering_bullet_from_smp(self):
        with self.ui_test.create_doc_in_start_center("writer") as xComponent:

            # Change the bullet to U+1F431 CAT FACE. This is a character outside of the basic
            # multilingual plane which means that it can’t be stored in a single sal_Unicode
            # variable. Verifies tdf#166488
            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                # Select the BulletPage's selector
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xBulletPage = xDialog.getChild("PickBulletPage")
                xSelector = xBulletPage.getChild("valueset")

                # Select element number 1
                xSelector.executeAction("CHOOSE", mkPropertyValues({"POS": "1"}))
                self.assertEqual(get_state_as_dict(xSelector)["SelectedItemId"], "1")
                xChangeBulletBtn = xBulletPage.getChild("changeBulletBtn")
                with self.ui_test.execute_blocking_action(xChangeBulletBtn.executeAction, args=('CLICK', ())) as xCharSetDialog:
                    # Select the DejaVu Sans font because that should contain the character
                    xFontName = xCharSetDialog.getChild("fontlb")
                    xFontName.executeAction("SET", mkPropertyValues({"TEXT": "DejaVu Sans"}))
                    # Use the hex entry to select the character
                    xHexText = xCharSetDialog.getChild("hexvalue")
                    xHexText.executeAction("SET", mkPropertyValues({"TEXT": "1F431"}))
                    # Check that the character was selected by checking the name
                    xCharName = xCharSetDialog.getChild("charname")
                    self.assertEqual(get_state_as_dict(xCharName)["Text"], "CAT FACE")

            # Check that the numbering level was updated for the paragraph in the document
            xDocCursor = xComponent.getText().createTextCursor()
            xNumberingRules = xDocCursor.getPropertyValue("NumberingRules")
            xNumberingLevel = xNumberingRules.getByIndex(0)
            self.assertEqual(convert_property_values_to_dict(xNumberingLevel)["BulletChar"],
                             "\U0001F431")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
