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

#test FontWorks dialog
class fontWorksDialog(UITestCase):

    def test_fontwork_selector(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:FontworkGalleryFloater", close_button="cancel") as xDialog:
                FontWorkSelector = xDialog.getChild("ctlFavoriteswin")
                # Select element with id (3)
                element3 = FontWorkSelector.getChild("2")
                element3.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(FontWorkSelector)["SelectedItemPos"], "2")
                self.assertEqual(get_state_as_dict(FontWorkSelector)["SelectedItemId"], "3")
                self.assertEqual(get_state_as_dict(FontWorkSelector)["VisibleCount"], "36")

                # Select element with id (7)
                element7 = FontWorkSelector.getChild("6")
                element7.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(FontWorkSelector)["SelectedItemPos"], "6")
                self.assertEqual(get_state_as_dict(FontWorkSelector)["SelectedItemId"], "7")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
