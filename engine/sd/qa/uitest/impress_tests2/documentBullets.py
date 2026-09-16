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
from libreoffice.uno.propertyvalue import mkPropertyValues

# The test presentation carries one bullet character that only its master slide uses, and one
# bullet image. Both are offered again wherever a bullet can be picked.
class documentBullets(UITestCase):

    def selectOutlinePlaceholder(self):
        # Tab reaches the placeholders in turn, and the outline one is the second. Picking it by
        # name is not possible because a placeholder carries none.
        xImpressDoc = self.xUITest.getTopFocusWindow()
        xEditWin = xImpressDoc.getChild("impress_win")
        xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))
        xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

    def test_bullets_and_numbering_offers_the_document_images(self):
        with self.ui_test.load_file(get_url_for_data_file("documentBullets.odp")):
            self.selectOutlinePlaceholder()

            with self.ui_test.execute_dialog_through_command(".uno:OutlineBullet") as xDialog:
                xSelectImage = xDialog.getChild("bitmap")
                aVisibleItems = get_state_as_dict(xSelectImage)["VisibleItems"]

                # The presentation's own images are offered beside the gallery and the file picker.
                self.assertIn("fromdocument", aVisibleItems)
                self.assertIn("gallery", aVisibleItems)
                self.assertIn("fromfile", aVisibleItems)

    def test_bullets_and_numbering_hides_the_entry_without_document_images(self):
        with self.ui_test.create_doc_in_start_center("impress"):
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(xTemplateDlg.getChild("close"))

            self.selectOutlinePlaceholder()

            with self.ui_test.execute_dialog_through_command(".uno:OutlineBullet") as xDialog:
                xSelectImage = xDialog.getChild("bitmap")
                aVisibleItems = get_state_as_dict(xSelectImage)["VisibleItems"]

                # A presentation with no images of its own says so by leaving the entry out.
                self.assertNotIn("fromdocument", aVisibleItems)
                self.assertIn("gallery", aVisibleItems)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
