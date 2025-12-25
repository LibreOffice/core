# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

class tdf159707(UITestCase):

    def test_tdf159707(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf159707.xls")) as document:

            self.assertFalse(document.isReadonly())

            # Without the fix in place, this test would have crashed here
            self.xUITest.executeCommand(".uno:EditDoc")

            document = self.ui_test.get_component()
            self.assertTrue(document.isReadonly())

            self.xUITest.executeCommand(".uno:EditDoc")

            self.assertFalse(document.isReadonly())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
