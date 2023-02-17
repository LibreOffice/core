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

class tdf132714(UITestCase):
    def test_tdf132714(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf132714.odt")) as document:

            # delete second row (first data row) in the associated text table of the chart
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoDown")
            # Without the fix in place, at this point crash occurs.
            self.xUITest.executeCommand(".uno:DeleteRows")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
