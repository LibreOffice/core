# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.uihelper.common import select_pos

class Tdf140257(UITestCase):

  def change_outline_level(self, sText):

    self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
    xDialog = self.xUITest.getTopFocusWindow()
    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "4")

    xOutline = xDialog.getChild("comboLB_OUTLINE_LEVEL")

    props = {"TEXT": sText}
    actionProps = mkPropertyValues(props)
    xOutline.executeAction("SELECT", actionProps)

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

  def test_tdf140257(self):
    self.ui_test.create_doc_in_start_center("writer")
    xMainWindow = self.xUITest.getTopFocusWindow()
    xWriterEdit = xMainWindow.getChild("writer_edit")

    self.change_outline_level("Level 1")

    for i in range(4):
        type_text(xWriterEdit, 'P' + str(i + 1))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

    type_text(xWriterEdit, 'P5')

    self.xUITest.executeCommand(".uno:Sidebar")

    xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

    # wait until the navigator panel is available
    xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanelParent')

    xContentTree = xNavigatorPanel.getChild("contenttree")
    xHeadings = xContentTree.getChild('0')
    self.assertEqual('Headings', get_state_as_dict(xHeadings)['Text'])

    xHeadings.executeAction("EXPAND", tuple())

    self.assertEqual(5, len(xHeadings.getChildren()))

    for i in range(5):
        self.assertEqual('P' + str(i + 1), get_state_as_dict(xHeadings.getChild(str(i)))['Text'])

    self.xUITest.executeCommand(".uno:Sidebar")

    document = self.ui_test.get_component()
    cursor = document.getCurrentController().getViewCursor()

    # Use Adding Selection
    selectionProperty = mkPropertyValues({"SelectionMode": 2})
    self.xUITest.executeCommandWithParameters(".uno:SelectionMode", selectionProperty)

    # Go to P2 and select it
    cursor.goUp(3, False)
    cursor.gotoStartOfLine(False)
    cursor.gotoEndOfLine(True)

    # Go to P4 and select it
    cursor.goDown(2, False)
    cursor.gotoStartOfLine(False)
    cursor.gotoEndOfLine(True)

    self.change_outline_level("Level 2")

    self.xUITest.executeCommand(".uno:Sidebar")

    xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

    # wait until the navigator panel is available
    xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanelParent')

    xContentTree = xNavigatorPanel.getChild("contenttree")
    xHeadings = xContentTree.getChild('0')
    self.assertEqual('Headings', get_state_as_dict(xHeadings)['Text'])

    xHeadings.executeAction("EXPAND", tuple())

    # Without the fix in place, this test would have failed with
    # AssertionError: 3 != 4
    self.assertEqual(3, len(xHeadings.getChildren()))
    xChild1 = xHeadings.getChild('0')
    self.assertEqual('P1', get_state_as_dict(xChild1)['Text'])
    xChild1.executeAction("EXPAND", tuple())
    self.assertEqual(1, len(xChild1.getChildren()))
    self.assertEqual('P2', get_state_as_dict(xChild1.getChild('0'))['Text'])

    xChild2 = xHeadings.getChild('1')
    self.assertEqual('P3', get_state_as_dict(xChild2)['Text'])
    xChild2.executeAction("EXPAND", tuple())
    self.assertEqual(1, len(xChild2.getChildren()))
    self.assertEqual('P4', get_state_as_dict(xChild2.getChild('0'))['Text'])

    self.assertEqual('P5', get_state_as_dict(xHeadings.getChild('2'))['Text'])

    self.xUITest.executeCommand(".uno:Sidebar")

    self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
