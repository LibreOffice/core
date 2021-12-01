# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos

class apply_line_cap(UITestCase):

    def test_apply_line_cap(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf127166_prstDash_Word97.docx")) as writer_doc:

            # check FLAT -> ROUND cap style change by selecting the new 'Rounded' preset line styles

            # select second line shape (dashDot)
            writer_doc.getCurrentController().select(writer_doc.getDrawPage()[1])

            # wait for available line style setting
            self.ui_test.wait_until_child_is_available('metricfield')

            # line setting dialog window
            with self.ui_test.execute_dialog_through_command(".uno:FormatLine") as xFormatLineDlg:

                # get cap style combo box
                xCapStyle = xFormatLineDlg.getChild("LB_CAP_STYLE")
                cap_style = get_state_as_dict(xCapStyle)['SelectEntryText']

                # get line style combo box
                xLineStyle = xFormatLineDlg.getChild("LB_LINE_STYLE")

                # select 'Dot (Rounded)', but store the previous value
                style = get_state_as_dict(xLineStyle)['SelectEntryText']
                select_pos(xLineStyle, "3")


            self.assertEqual(cap_style, 'Flat')
            self.assertEqual(style, 'Long Dash Dot')

            # check round cap setting, opening the line style dialog again
            writer_doc.getCurrentController().select(writer_doc.getDrawPage()[1])

            # wait for available line style setting
            self.ui_test.wait_until_child_is_available('metricfield')

            # line setting dialog window
            with self.ui_test.execute_dialog_through_command(".uno:FormatLine") as xFormatLineDlg:

                # get cap style combo box
                xCapStyle = xFormatLineDlg.getChild("LB_CAP_STYLE")
                cap_style = get_state_as_dict(xCapStyle)['SelectEntryText']

                # get line style combo box
                xLineStyle = xFormatLineDlg.getChild("LB_LINE_STYLE")

                # select 'Dot', but store the previous value
                style = get_state_as_dict(xLineStyle)['SelectEntryText']
                select_pos(xLineStyle, "2")


            # This was 'Flat' (set only dash style of the line style before)
            self.assertEqual(cap_style, 'Round')
            self.assertEqual(style, 'Dot (Rounded)')

            # 2. check ROUND -> FLAT cap style change
            writer_doc.getCurrentController().select(writer_doc.getDrawPage()[1])

            # wait for available line style setting
            self.ui_test.wait_until_child_is_available('metricfield')

            # line setting dialog window
            with self.ui_test.execute_dialog_through_command(".uno:FormatLine") as xFormatLineDlg:

                # get cap style combo box
                xCapStyle = xFormatLineDlg.getChild("LB_CAP_STYLE")
                cap_style = get_state_as_dict(xCapStyle)['SelectEntryText']

                # get line style combo box
                xLineStyle = xFormatLineDlg.getChild("LB_LINE_STYLE")

                style = get_state_as_dict(xLineStyle)['SelectEntryText']


            # This was 'Flat' (set only dash style of the line style before)
            self.assertEqual(cap_style, 'Flat')
            self.assertEqual(style, 'Dot')


# vim: set shiftwidth=4 softtabstop=4 expandtab:
