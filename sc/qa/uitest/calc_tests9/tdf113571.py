# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class Tdf113571(UITestCase):

   def test_tdf113571(self):
        self.ui_test.load_file(get_url_for_data_file("tdf113571.ods"))

        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        self.xUITest.executeCommand(".uno:Copy")

        # Close the document
        self.xUITest.executeCommand(".uno:CloseDoc")

        self.ui_test.create_doc_in_start_center("writer")

        self.xUITest.executeCommand(".uno:PasteUnformatted")

        document = self.ui_test.get_component()

        # Without the fix in place, this test would have failed with
        # AssertionError: '<?xml version="1.0" encoding="UTF-8"?>\n<[34 chars]est>' !=
        #                 '"<?xml version=""1.0"" encoding=""UTF-8""[40 chars]st>"'
        self.assertEqual('<?xml version="1.0" encoding="UTF-8"?>\n<test>\n  <hello>world</hello>\n</test>',
                document.Text.String)

        self.ui_test.close_doc()
