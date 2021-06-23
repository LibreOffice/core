# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

import json

class CheckBoxTest(UITestCase):

    def test_get_json(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_dialog_through_command(".uno:About")

        xAboutDlg = self.xUITest.getTopFocusWindow()

        json_string = xAboutDlg.getHierarchy()
        print(json_string)
        json_content = json.loads(json_string)
        print(json_content)
        print(json.dumps(json_content, indent=4))

        closeBtn = xAboutDlg.getChild("btnClose")
        self.ui_test.close_dialog_through_button(closeBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
