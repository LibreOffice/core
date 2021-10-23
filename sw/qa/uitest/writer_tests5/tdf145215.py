# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict

class Tdf145215(UITestCase):

    def test_tdf145215(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf145215.docx"))

        # Check field value (there is only one field)
        textfields = writer_doc.getTextFields()
        textfields.refresh()
        for textfield in textfields:
            self.assertTrue(textfield.supportsService("com.sun.star.text.TextField.GetReference"))
            self.assertEqual(textfield.CurrentPresentation, "1.2.1(i)")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
