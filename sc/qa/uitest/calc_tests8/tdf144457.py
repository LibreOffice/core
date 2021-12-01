# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf144457(UITestCase):

    def test_tdf144457(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:
                xPages = xDialog.getChild("pages")
                xLanguageEntry = xPages.getChild('2')
                xLanguageEntry.executeAction("EXPAND", tuple())
                xxLanguageEntryGeneralEntry = xLanguageEntry.getChild('0')
                xxLanguageEntryGeneralEntry.executeAction("SELECT", tuple())

                xDatePatterns = xDialog.getChild("datepatterns")

                xDatePatterns.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xDatePatterns.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xDatePatterns.executeAction("TYPE", mkPropertyValues({"TEXT":"m/d/y"}))

                # Without the fix in place, this test would have failed with
                # AssertionError: 'M/D/Y' != 'm/M/dM/M/d/M/M/dM/M/d/yM/M/dM/M/d/M/M/dM/M/d/y'
                self.assertEqual("M/D/Y", get_state_as_dict(xDatePatterns)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
