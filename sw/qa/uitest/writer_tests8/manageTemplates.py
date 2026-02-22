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

class ManageTemplates(UITestCase):

    def test_EditTemplate(self):

        try:
            # Open "Manage Templates" dialog from the start center
            with self.ui_test.execute_dialog_through_command(".uno:NewDoc", close_button="") as xDialog:
                xTemplateView = xDialog.getChild("template_view")
                xTemplateView.executeAction("SELECT", mkPropertyValues({"POS": "1"}))
                print(get_state_as_dict(xTemplateView))
                self.assertEqual("1", get_state_as_dict(xTemplateView)["SelectedIndex"])
                aName = get_state_as_dict(xTemplateView)["SelectedItemTitle"]

                # tdf#170399: Without the fix in place, this test would have crashed here
                xTemplateView.executeAction("EDIT", tuple())
        except:
            # if it fails, close the dialog before disposing
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)
            raise

        document = self.ui_test.get_component()
        self.assertEqual(aName.replace(" ", "_").replace("-", "_") + ".ott", document.Title)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
