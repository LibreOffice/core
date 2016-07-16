# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.uno.eventlistener import EventListener

from uitest.framework import UITestCase

from uitest.debug import sleep

import time

class HandleFiles(UITestCase):

    def test_load_file(self):

        calc_file = self.ui_test.load_file("file:///home/moggi/devel/libo9/uitest/data/test.ods")

        calc_file2 = self.ui_test.load_file("file:///home/moggi/devel/libo9/uitest/data/test2.ods")

        frames = self.ui_test.get_frames()
        self.assertEqual(len(frames), 2)

        self.ui_test.close_doc()

        frames = self.ui_test.get_frames()
        self.assertEqual(len(frames), 1)

        # this is currently still necessary as otherwise
        # the command is not forwarded to the correct frame
        # TODO: provide an additional event that we can use
        #       and get rid of the sleep
        time.sleep(1)

        self.ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
