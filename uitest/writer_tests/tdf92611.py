#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time

class tdf92611(UITestCase):

    def test_launch_and_close_bibliography(self):

        self.ui_test.create_doc_in_start_center("writer")


        self.xUITest.executeCommand(".uno:BibliographyComponent")

        time.sleep(2)

        self.xUITest.executeCommand(".uno:CloseWin")

        time.sleep(2)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
