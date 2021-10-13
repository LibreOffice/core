# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import type_text
from uitest.uihelper.common import get_url_for_data_file

class tdf145093(UITestCase):
    def test_tdf145093(self):
        # load the sample file
        with self.ui_test.load_file(get_url_for_data_file("tdf39721.fodt")) as document:

            # redlining should be on
            self.xUITest.executeCommand(".uno:TrackChanges")

            # Move Up/Down a list item over a tracked paragraph insertion resulted a crash
            # (Note: not only at the end of the document, but check the original
            # bug report temporarily, where the problem is triggered by an other problem).

            self.xUITest.executeCommand(".uno:MoveDown")
            self.xUITest.executeCommand(".uno:MoveDown")
            # Note: Move list item from the end of the document creates an extra insertion.
            # TODO: fix this other problem, and improve the test with an extra paragraph insertion
            self.xUITest.executeCommand(".uno:MoveUp")

            # This was a crash (using invalid pointer) both in Show Changes and Hide Changes modes
            self.xUITest.executeCommand(".uno:MoveDown")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
