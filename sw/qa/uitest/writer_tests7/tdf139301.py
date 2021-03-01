# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf139301(UITestCase):

    def test_tdf139301(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf127166_prstDash_Word97.docx"))

        styles = ('Long Dash', 'Long Dash Dot', 'Long Dot', 'Double Dash', 'Double Dash Dot', 'Double Dash Dot Dot', 'Dash', 'Dash Dot', 'Dash Dot Dot', 'Dot')

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        for i in range(len(styles)):
            # don't check not back-ported styles
            if styles[i] in ('Dash Dot', 'Dash Dot Dot'):
                continue

            # select next line shape
            writer_doc.getCurrentController().select(writer_doc.getDrawPage()[i])

            # wait for available line style setting
            self.ui_test.wait_until_child_is_available(xWriterEdit, 'metricfield')

            # line setting dialog window
            self.ui_test.execute_dialog_through_command(".uno:FormatLine")
            xFormatLineDlg = self.xUITest.getTopFocusWindow()

            # get line style combo box
            xLineStyle = xFormatLineDlg.getChild("LB_LINE_STYLE")

            # check preset line style
            style = get_state_as_dict(xLineStyle)['SelectEntryText']

            xOKBtn = xFormatLineDlg.getChild("ok")
            self.ui_test.close_dialog_through_button(xOKBtn)

            self.assertEqual(style, styles[i])

        self.ui_test.close_doc()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
