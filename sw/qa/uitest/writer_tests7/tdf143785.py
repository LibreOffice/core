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

class tdf143785(UITestCase):

    def test_tdf143785(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf137802.odt")) as document:

            self.xUITest.executeCommand(".uno:JumpToNextFrame")

            self.ui_test.wait_until_child_is_available('metricfield')

            self.assertEqual(False, document.isModified())

            with self.ui_test.execute_dialog_through_command(".uno:TransformDialog", close_button="cancel"):
                pass

            # Without the fix in place, this test would have failed with
            # AssertionError: False != True
            self.assertEqual(False, document.isModified())

            with self.ui_test.execute_dialog_through_command(".uno:FormatArea", close_button="cancel"):
                pass

            # tdf#143778: Without the fix in place, this test would have failed with
            # AssertionError: False != True
            self.assertEqual(False, document.isModified())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
