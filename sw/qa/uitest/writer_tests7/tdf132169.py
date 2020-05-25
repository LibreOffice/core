# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.path import get_srcdir_url
from uitest.uihelper.common import get_state_as_dict, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

class tdf132169(UITestCase):
    def test_tdf132169(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("shape.odt"))

        #set measurement to points
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
        xDialogOpt = self.xUITest.getTopFocusWindow()
        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())
        xMetric = xDialogOpt.getChild("metric")
        props = {"TEXT": "Point"}
        actionProps = mkPropertyValues(props)
        xMetric.executeAction("SELECT", actionProps)
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        #wait until the toolbar is available
        self.ui_test.wait_until_child_is_available(xWriterEdit, 'metricfield')
        xLineMetric = xWriterEdit.getChild('metricfield')
        self.assertEqual(get_state_as_dict(xLineMetric)["Text"], "0.0 pt")

        #Check changing value from dialog also works
        self.ui_test.execute_dialog_through_command(".uno:FormatLine")
        xFormatLineDlg = self.xUITest.getTopFocusWindow()
        xWidth = xFormatLineDlg.getChild('MTR_FLD_LINE_WIDTH')
        type_text(xWidth, "4.0")
        xOKBtn = xFormatLineDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.wait_until_property_is_updated(xLineMetric, "Text", "4.0 pt")
        self.assertEqual(get_state_as_dict(xLineMetric)["Text"], "4.0 pt")

        xLineMetric.executeAction("UP", tuple())

        document = self.ui_test.get_component()
        drawPage = document.getDrawPages().getByIndex(0)
        shape = drawPage.getByIndex(0)

        #Without the fix in place, it would have been 310
        self.assertEqual(shape.LineWidth, 176)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

