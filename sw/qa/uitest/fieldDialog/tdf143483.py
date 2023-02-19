# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class Tdf143483(UITestCase):

    def test_tdf143483(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf143483.odt")) as document:

            self.xUITest.executeCommand(".uno:JumpToNextBookmark")

            with self.ui_test.execute_dialog_through_command(".uno:FieldDialog") as xDialog:

                xDoc = xDialog.getChild("type-doc")
                self.assertEqual("Heading", get_state_as_dict(xDoc)['SelectEntryText'])

                xFormat = xDialog.getChild("format-doc")
                self.assertEqual("Heading contents", get_state_as_dict(xFormat)['SelectEntryText'])

                xNext = xDialog.getChild("next")
                xNext.executeAction("CLICK", tuple())

                self.assertEqual("Heading", get_state_as_dict(xDoc)['SelectEntryText'])
                self.assertEqual("Heading number", get_state_as_dict(xFormat)['SelectEntryText'])

                xNext.executeAction("CLICK", tuple())

                self.assertEqual("Heading", get_state_as_dict(xDoc)['SelectEntryText'])
                self.assertEqual("Heading number and contents", get_state_as_dict(xFormat)['SelectEntryText'])

                xNext.executeAction("CLICK", tuple())

                self.assertEqual("Heading", get_state_as_dict(xDoc)['SelectEntryText'])
                self.assertEqual("Heading number without separator", get_state_as_dict(xFormat)['SelectEntryText'])

            xEnumeration = document.Text.createEnumeration()
            self.assertEqual("Another title", xEnumeration.nextElement().String)
            self.assertEqual("", xEnumeration.nextElement().String)
            self.assertEqual("1. Another title", xEnumeration.nextElement().String)

            # Without the fix in place, this test would have failed with
            # AssertionError: '2. Chapter 1 -' != '2. Another title'
            self.assertEqual("2. Chapter 1 -", xEnumeration.nextElement().String)
            self.assertEqual("3. Chapter 1 - Another title", xEnumeration.nextElement().String)
            self.assertEqual("4. 1", xEnumeration.nextElement().String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
