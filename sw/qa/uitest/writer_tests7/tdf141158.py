# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_url_for_data_file

class TestTdf141158(UITestCase):
    def test_tdf141158(self):
        # load the desired bugdoc
        with self.ui_test.load_file(get_url_for_data_file("TestHiddenHeadersFooters.docx")) as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            # open the page styles dialog and select the headers tab
            with self.ui_test.execute_dialog_through_command(".uno:PageStyleName") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")
                # make the same left right page header state to off  and apply the setting
                checkSameLR = xDialog.getChild("checkSameLR")
                checkSameLR.executeAction("CLICK",tuple())

            # go to the left header
            xWriterEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "2"}))
            self.xUITest.executeCommand(".uno:JumpToHeader")
            # get the text of the header
            xPageSytle = document.getStyleFamilies().getByIndex(2)
            xHeaderText = xPageSytle.getByIndex(0).HeaderText.String
            xHeaderLeftText = xPageSytle.getByIndex(0).HeaderTextLeft.String
            # without the fix in place it was "Right Header" (lost hidden left header),
            # with the fix it should pass...
            self.assertEqual("Left Header", xHeaderLeftText)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
