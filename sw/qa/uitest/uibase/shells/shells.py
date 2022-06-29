# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import get_url_for_data_file
import time


class TestSwGrfShell(UITestCase):
    def testFormatGraphicDlg(self):
        # Given a document with an image with a relative size:
        with self.ui_test.load_file(get_url_for_data_file("image-rel-size.fodt")):
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            # 120 ms in the SwView ctor
            time.sleep(0.2)
            # When opening the properties dialog for the image:
            with self.ui_test.execute_dialog_through_command(".uno:GraphicDialog") as dialog:
                # Then make sure the width is 80% as in the file:
                widthField = dialog.getChild('width')
                # Without the accompanying fix in place, this test would have failed with:
                # AssertionError: '237%' != '80%'
                # i.e. the percent width of the image was wrong.
                self.assertEqual(get_state_as_dict(widthField)["Text"], "80%")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
