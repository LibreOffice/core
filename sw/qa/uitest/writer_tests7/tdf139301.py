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

class tdf139301(UITestCase):

    styles = ('Long Dash', 'Long Dash Dot', 'Long Dot', 'Double Dash', 'Double Dash Dot', 'Double Dash Dot Dot', 'Dash', 'Dash Dot', 'Dash Dot Dot', 'Dot')

    def test_tdf139301(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf127166_prstDash_Word97.docx")) as writer_doc:

            for i in range(len(self.styles)):
                # select next line shape
                writer_doc.getCurrentController().select(writer_doc.getDrawPage()[i])

                # wait for available line style setting
                self.ui_test.wait_until_child_is_available('metricfield')

                # line setting dialog window
                with self.ui_test.execute_dialog_through_command(".uno:FormatLine") as xFormatLineDlg:
                    # get line style combo box
                    xLineStyle = xFormatLineDlg.getChild("LB_LINE_STYLE")

                    # check preset line style
                    style = get_state_as_dict(xLineStyle)['SelectEntryText']


                self.assertEqual(style, self.styles[i])

    def test_round_cap(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf127166_prstDash_round_cap.docx")) as writer_doc:

            style_name_extension = ' (Rounded)'

            for i in range(len(self.styles)):
                # select next line shape
                writer_doc.getCurrentController().select(writer_doc.getDrawPage()[i])

                # wait for available line style setting
                self.ui_test.wait_until_child_is_available('metricfield')

                # line setting dialog window
                with self.ui_test.execute_dialog_through_command(".uno:FormatLine") as xFormatLineDlg:
                    # get line style combo box
                    xLineStyle = xFormatLineDlg.getChild("LB_LINE_STYLE")

                    # check preset line style
                    style = get_state_as_dict(xLineStyle)['SelectEntryText']


                self.assertEqual(style, self.styles[i] + style_name_extension)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
