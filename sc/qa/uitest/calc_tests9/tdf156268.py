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

class Tdf156268(UITestCase):

   def test_tdf156268(self):
        # In order to simulate copy&paste from external source,
        # import the text in Writer, copy it and paste it in Calc
        with self.ui_test.load_file(get_url_for_data_file("tdf156268.fods")):

            self.xUITest.executeCommand(".uno:SelectAll")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
