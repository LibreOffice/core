# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

import json

class CheckBoxTest(UITestCase):

    def test_get_json(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_dialog_through_command(".uno:About", close_button="btnClose") as xAboutDlg:


                json_string = xAboutDlg.getHierarchy()
                print(json_string)
                json_content = json.loads(json_string)
                print(json_content)
                print(json.dumps(json_content, indent=4))



# vim: set shiftwidth=4 softtabstop=4 expandtab:
