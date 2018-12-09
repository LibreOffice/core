# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time
import datetime
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

class insertField(UITestCase):

    def test_insert_field_document_date(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #Date
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "1")
        xSelectListbox = xCrossRef.getChild("select")
        sleep(1)
        select_pos(xSelectListbox, "0")
        sleep(1)
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "0")  # format
        xOffset = xCrossRef.getChild("offset")
        xOffset.executeAction("UP", tuple())  # offset date
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field Date
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        actual_date = datetime.datetime.now()
        actual_day = actual_date.strftime("%d")
        actual_day_plus = str(int(actual_date.strftime("%d")) + 1)
        if len(actual_day_plus) == 1:
            actual_day_plus = "0" + actual_day_plus
        actual_month = actual_date.strftime("%m")
        actual_year = actual_date.strftime("%Y")
        print(actual_day + "." + actual_month + "." + actual_year)
        self.assertEqual(document.Text.String[0:10], actual_day_plus + "." + actual_month + "." + actual_year)
        self.xUITest.executeCommand(".uno:Undo")   #undo
        self.assertEqual(document.Text.String[0:10], "")

        #negative offset
        self.xUITest.executeCommand(".uno:InsertField")
        xCrossRef = self.xUITest.getTopFocusWindow()
        xTypeListbox = xCrossRef.getChild("type")
        select_pos(xTypeListbox, "1")
        xSelectListbox = xCrossRef.getChild("select")
        sleep(1)
        select_pos(xSelectListbox, "0")
        sleep(1)
        xFormatListbox = xCrossRef.getChild("format")
        select_pos(xFormatListbox, "0")  # format
        xOffset = xCrossRef.getChild("offset")
        xOffset.executeAction("DOWN", tuple())  # offset date
        xAddBtn = xCrossRef.getChild("ok")
        xAddBtn.executeAction("CLICK", tuple())   # insert the field Date
        xCloseBtn = xCrossRef.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())   # close the dialog
        actual_date = datetime.datetime.now()
        actual_day = actual_date.strftime("%d")
        actual_day_minus = str(int(actual_date.strftime("%d")) - 1)
        if len(actual_day_minus) == 1:
            actual_day_minus = "0" + actual_day_minus
        actual_month = actual_date.strftime("%m")
        actual_year = actual_date.strftime("%Y")
        print(actual_day + "." + actual_month + "." + actual_year)
        self.assertEqual(document.Text.String[0:10], actual_day_minus + "." + actual_month + "." + actual_year)
        self.xUITest.executeCommand(".uno:Undo")   #undo
        self.assertEqual(document.Text.String[0:10], "")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
