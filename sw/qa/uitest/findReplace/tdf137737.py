# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf137737(UITestCase):

    def test_tdf137737(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf137737.odt")) as writer_doc:

            self.assertEqual(2, writer_doc.Endnotes.Count)

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                xSearchterm = xDialog.getChild("searchterm")
                xSearchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSearchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xSearchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"[:control:]"}))

                xReplaceterm = xDialog.getChild("replaceterm")
                xReplaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"A"}))

                # Deselect similarity before selecting regex
                xSimilarity = xDialog.getChild("similarity")
                if get_state_as_dict(xSimilarity)['Selected'] == 'true':
                    xSimilarity.executeAction("CLICK", tuple())

                xRegexp = xDialog.getChild("regexp")
                xRegexp.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(xRegexp)['Selected'])

                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())

                # Deselect regex button, otherwise it might affect other tests
                xRegexp.executeAction("CLICK", tuple())
                self.assertEqual("false", get_state_as_dict(xRegexp)['Selected'])

            # Without the fix in place, this test would have failed with
            # AssertionError: 0 != 1
            self.assertEqual(0, writer_doc.Endnotes.Count)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
