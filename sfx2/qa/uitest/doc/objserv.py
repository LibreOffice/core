#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file


# Test for sfx2/source/doc/objserv.cxx.
class Test(UITestCase):

    def testPdfSigning(self):
        # Start Impress.
        impress_doc = self.ui_test.load_file(get_url_for_data_file("pdf-sign.pdf"))
        doc = self.xUITest.getTopFocusWindow()

        # Now use File -> Digital signatures -> Digital signatures.
        self.ui_test.execute_dialog_through_command(".uno:Signature")
        xDialog = self.xUITest.getTopFocusWindow()
        # Without the accompanying fix in place, this test would have failed with:
        # uno.com.sun.star.uno.RuntimeException: Could not find child with id: close vcl/source/uitest/uiobject.cxx:452
        self.ui_test.close_dialog_through_button(xDialog.getChild("close"))

        self.ui_test.close_doc()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
