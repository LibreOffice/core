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
import time

class HandleFiles(UITestCase):

    def test_load_file(self):

        with self.ui_test.load_file(get_url_for_data_file("test.ods")):

            with self.ui_test.load_file(get_url_for_data_file("test2.ods")):

                frames = self.ui_test.get_frames()
                self.assertEqual(len(frames), 2)

            frames = self.ui_test.get_frames()
            self.assertEqual(len(frames), 1)

            # this is currently still necessary as otherwise
            # the command is not forwarded to the correct frame
            # TODO: provide an additional event that we can use
            #       and get rid of the sleep
            time.sleep(1)

    def test_select_frame(self):
        with self.ui_test.load_file(get_url_for_data_file("test.ods")):

            with self.ui_test.load_file(get_url_for_data_file("test2.ods")):
                frames = self.ui_test.get_frames()
                self.assertEqual(len(frames), 2)
                frames[0].activate()

            frames = self.ui_test.get_frames()
            self.assertEqual(len(frames), 1)

            self.assertTrue(frames[0].getTitle().startswith("test2.ods"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
