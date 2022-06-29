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

class tdf122780(UITestCase):

   def test_tdf122780(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf122780.docx")):
            with self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties") as xDialog:

                # Without the fix in place, this test would have hung here
                self.assertEqual("Normal_x005F", get_state_as_dict(xDialog.getChild('showtemplate'))['Text'][:12])
                self.assertEqual(32767, len(get_state_as_dict(xDialog.getChild('showtemplate'))['Text']))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
