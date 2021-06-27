# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class Tdf95554(UITestCase):

   def test_tdf95554(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf95554.ods")):

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A3"}))

            self.xUITest.executeCommand(".uno:Copy")

            with self.ui_test.load_empty_file("writer") as writer_document:

                self.xUITest.getTopFocusWindow()

                self.xUITest.executeCommand(".uno:PasteUnformatted")

                self.assertEqual('1\n2\n3',
                        writer_document.Text.String)

