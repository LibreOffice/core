# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_by_text

class tdf144691(UITestCase):

    def test_tdf144691(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:
                xPages = xDialog.getChild("pages")
                xLanguageEntry = xPages.getChild('2')
                xLanguageEntry.executeAction("EXPAND", tuple())
                xxLanguageEntryGeneralEntry = xLanguageEntry.getChild('0')
                xxLanguageEntryGeneralEntry.executeAction("SELECT", tuple())

                xWesternLanguage = xDialog.getChild("westernlanguage")

                defaultLanguage = get_state_as_dict(xWesternLanguage)['SelectEntryText']

                # Select another language
                select_by_text(xWesternLanguage, "Tajik")
                self.assertEqual("Tajik", get_state_as_dict(xWesternLanguage)['SelectEntryText'])

                xApply = xDialog.getChild("apply")
                xApply.executeAction("CLICK", tuple())

                # Without the fix in place, this test would have failed with
                # AssertionError: 'Tajik' != 'English (USA)'
                self.assertEqual("Tajik", get_state_as_dict(xWesternLanguage)['SelectEntryText'])

                # Select the default language
                select_by_text(xWesternLanguage, defaultLanguage)
                self.assertEqual(defaultLanguage, get_state_as_dict(xWesternLanguage)['SelectEntryText'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
