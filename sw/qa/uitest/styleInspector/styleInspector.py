# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class styleNavigator(UITestCase):

    def test_listbox_is_updated(self):
        self.ui_test.load_file(get_url_for_data_file("styles.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "InspectorTextPanel"}))

        xListBox = xWriterEdit.getChild('listbox_fonts')

        # The cursor is on text without formatting and default style
        self.assertEqual(1, len(xListBox.getChild('0').getChildren()))
        self.assertEqual("Default Paragraph Style", get_state_as_dict(xListBox.getChild('0').getChild('0'))['Text'])
        self.assertEqual(136, len(xListBox.getChild('0').getChild('0').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('1').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('2').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('3').getChildren()))

        self.xUITest.executeCommand(".uno:GoDown")

        # The cursor is on text with direct formatting
        self.assertEqual(1, len(xListBox.getChild('0').getChildren()))
        self.assertEqual("Default Paragraph Style", get_state_as_dict(xListBox.getChild('0').getChild('0'))['Text'])
        self.assertEqual(136, len(xListBox.getChild('0').getChild('0').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('1').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('2').getChildren()))

        xDirFormatting = xListBox.getChild('3')
        self.assertEqual(5, len(xDirFormatting.getChildren()))
        self.assertEqual("Char Back Color\t0xffff00", get_state_as_dict(xDirFormatting.getChild('0'))['Text'])
        self.assertEqual("Char Back Transparent\tfalse", get_state_as_dict(xDirFormatting.getChild('1'))['Text'])
        self.assertEqual("Char Color\t0xc9211e", get_state_as_dict(xDirFormatting.getChild('2'))['Text'])
        self.assertEqual("Char Shading Value\t0", get_state_as_dict(xDirFormatting.getChild('3'))['Text'])
        self.assertEqual("Char Transparence\t0", get_state_as_dict(xDirFormatting.getChild('4'))['Text'])


        self.xUITest.executeCommand(".uno:GoDown")

        # The cursor is on text with paragraph direct formatting
        self.assertEqual(1, len(xListBox.getChild('0').getChildren()))
        self.assertEqual("Default Paragraph Style", get_state_as_dict(xListBox.getChild('0').getChild('0'))['Text'])
        self.assertEqual(136, len(xListBox.getChild('0').getChild('0').getChildren()))

        xParDirFormatting = xListBox.getChild('1')
        self.assertEqual(7, len(xParDirFormatting.getChildren()))
        self.assertEqual("Fill Color\t0xff0000", get_state_as_dict(xParDirFormatting.getChild('0'))['Text'])
        self.assertEqual("Para Back Color\t0xff0000", get_state_as_dict(xParDirFormatting.getChild('1'))['Text'])
        self.assertEqual("Para Back Transparent\tfalse", get_state_as_dict(xParDirFormatting.getChild('2'))['Text'])
        self.assertEqual("Para First Line Indent\t0", get_state_as_dict(xParDirFormatting.getChild('3'))['Text'])
        self.assertEqual("Para is Auto First Line Indent\tfalse", get_state_as_dict(xParDirFormatting.getChild('4'))['Text'])
        self.assertEqual("Para Left Margin\t1482", get_state_as_dict(xParDirFormatting.getChild('5'))['Text'])
        self.assertEqual("Para Right Margin\t0", get_state_as_dict(xParDirFormatting.getChild('6'))['Text'])

        self.assertEqual(0, len(xListBox.getChild('2').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('3').getChildren()))

        self.xUITest.executeCommand(".uno:GoDown")

        # The cursor is on text with 'Title' style
        xParStyle = xListBox.getChild('0')
        self.assertEqual(3, len(xParStyle.getChildren()))
        self.assertEqual("Default Paragraph Style", get_state_as_dict(xParStyle.getChild('0'))['Text'])
        self.assertEqual(136, len(xParStyle.getChild('0').getChildren()))
        self.assertEqual("Heading", get_state_as_dict(xParStyle.getChild('1'))['Text'])
        self.assertEqual(28, len(xParStyle.getChild('1').getChildren()))

        xTitleStyle = xParStyle.getChild('2')
        self.assertEqual("Title", get_state_as_dict(xTitleStyle)['Text'])
        self.assertEqual(16, len(xTitleStyle.getChildren()))
        self.assertEqual("Char Difference Height\t0", get_state_as_dict(xTitleStyle.getChild('0'))['Text'])
        self.assertEqual("Char Difference Height Asian\t0", get_state_as_dict(xTitleStyle.getChild('1'))['Text'])
        self.assertEqual("Char Difference Height Complex\t0", get_state_as_dict(xTitleStyle.getChild('2'))['Text'])
        self.assertEqual("Char Height\t28", get_state_as_dict(xTitleStyle.getChild('3'))['Text'])
        self.assertEqual("Char Height Asian\t28", get_state_as_dict(xTitleStyle.getChild('4'))['Text'])
        self.assertEqual("Char Height Complex\t28", get_state_as_dict(xTitleStyle.getChild('5'))['Text'])
        self.assertEqual("Char Property Height\t100", get_state_as_dict(xTitleStyle.getChild('6'))['Text'])
        self.assertEqual("Char Property Height Asian\t100", get_state_as_dict(xTitleStyle.getChild('7'))['Text'])
        self.assertEqual("Char Property Height Complex\t100", get_state_as_dict(xTitleStyle.getChild('8'))['Text'])
        self.assertEqual("Char Weight\tbold", get_state_as_dict(xTitleStyle.getChild('9'))['Text'])
        self.assertEqual("Char Weight Asian\tbold", get_state_as_dict(xTitleStyle.getChild('10'))['Text'])
        self.assertEqual("Char Weight Complex\tbold", get_state_as_dict(xTitleStyle.getChild('11'))['Text'])
        self.assertEqual("Follow Style\tText body", get_state_as_dict(xTitleStyle.getChild('12'))['Text'])
        self.assertEqual("Para Adjust\t3", get_state_as_dict(xTitleStyle.getChild('13'))['Text'])
        self.assertEqual("Para Expand Single Word\tfalse", get_state_as_dict(xTitleStyle.getChild('14'))['Text'])
        self.assertEqual("Para Last Line Adjust\t0", get_state_as_dict(xTitleStyle.getChild('15'))['Text'])

        self.assertEqual(0, len(xListBox.getChild('1').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('2').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('3').getChildren()))

        self.xUITest.executeCommand(".uno:GoDown")

        # The cursor is on text with custom style
        xParStyle = xListBox.getChild('0')
        self.assertEqual(3, len(xParStyle.getChildren()))
        self.assertEqual("Default Paragraph Style", get_state_as_dict(xParStyle.getChild('0'))['Text'])
        self.assertEqual(136, len(xParStyle.getChild('0').getChildren()))
        self.assertEqual("Text Body", get_state_as_dict(xParStyle.getChild('1'))['Text'])
        self.assertEqual(6, len(xParStyle.getChild('1').getChildren()))

        xCustomStyle = xParStyle.getChild('2')
        self.assertEqual("Custom_Style", get_state_as_dict(xCustomStyle)['Text'])
        self.assertEqual(7, len(xCustomStyle.getChildren()))
        self.assertEqual("Char Color\t0xff5429", get_state_as_dict(xCustomStyle.getChild('0'))['Text'])
        self.assertEqual("Char Difference Height\t0", get_state_as_dict(xCustomStyle.getChild('1'))['Text'])
        self.assertEqual("Char Height\t20", get_state_as_dict(xCustomStyle.getChild('2'))['Text'])
        self.assertEqual("Char Property Height\t100", get_state_as_dict(xCustomStyle.getChild('3'))['Text'])
        self.assertEqual("Char Transparence\t0", get_state_as_dict(xCustomStyle.getChild('4'))['Text'])
        self.assertEqual("Char Weight\tbold", get_state_as_dict(xCustomStyle.getChild('5'))['Text'])
        self.assertEqual("Follow Style\tCustom_Style", get_state_as_dict(xCustomStyle.getChild('6'))['Text'])

        self.assertEqual(7, len(xListBox.getChild('0').getChild('2').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('1').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('2').getChildren()))
        self.assertEqual(0, len(xListBox.getChild('3').getChildren()))

        self.xUITest.executeCommand(".uno:Sidebar")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
