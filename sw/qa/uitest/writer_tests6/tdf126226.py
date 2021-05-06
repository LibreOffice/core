# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class Tdf126226(UITestCase):

   def test_tdf126226(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf126226.odt"))

        self.xUITest.executeCommand(".uno:SelectAll")

        # Without the fix in place, this test would have crashed here
        self.ui_test.execute_dialog_through_command(".uno:CommentChangeTracking")

        xDialog = self.xUITest.getTopFocusWindow()

        self.assertEqual("Hello\n", get_state_as_dict(xDialog.getChild("edit"))["Text"])
        self.assertEqual("Autor desconocido, 07/04/2019 13:43:52",
                get_state_as_dict(xDialog.getChild("lastedit"))["Text"])

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()
