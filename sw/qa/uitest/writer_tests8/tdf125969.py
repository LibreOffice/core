# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.common import select_pos, get_state_as_dict

# bug 125969: make in-use bitmap-area-fill available for re-use, but ONLY IN THE SAME DOCUMENT
class tdf125969(UITestCase):

    number_of_images = 0

    def test_tdf125969(self):
        with self.ui_test.load_file(get_url_for_data_file("paragraphAreaFill.odt")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:Sidebar") #turn on sidebar
            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "PageStylesPanel"}))

            # Get baseline from sidebar: count number of initially available bitmaps by default
            backgroundType = xWriterEdit.getChild('bgselect') #type of background: color, gradient, ...
            self.ui_test.wait_until_property_is_updated(backgroundType, "SelectEntryText", "Bitmap")

            imageCollection = xWriterEdit.getChild("lbbitmap") #listbox containing image names
            number_of_images = int(get_state_as_dict(imageCollection)["EntryCount"])

            # The paragraph area has a custom background logo - which we want to become available
            # for re-use everywhere as a background fill

            # visit the paragraph background property - which now auto-adds it to the collection
            with self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog", close_button="cancel") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "8") # area tab

            # test: the paragraph's wasta-offline logo was added and the list box was refreshed
            self.ui_test.wait_until_property_is_updated(imageCollection, "EntryCount", str(number_of_images + 1))
            self.assertEqual(number_of_images + 1, int(get_state_as_dict(imageCollection)["EntryCount"]))
            self.assertEqual("Painted White", get_state_as_dict(imageCollection)["SelectEntryText"])

        # A new document must not have access to the collected images from another document
        with self.ui_test.load_empty_file("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            # because I don't know how to change the sidebar to bitmap mode, use the page dialog
            with self.ui_test.execute_dialog_through_command(".uno:PageDialog", close_button="ok") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "2") # area tab
                xButton = xDialog.getChild('btnbitmap')
                xButton.executeAction("CLICK", tuple())

            backgroundType = xWriterEdit.getChild('bgselect')
            imageCollection = xWriterEdit.getChild("lbbitmap")
            self.ui_test.wait_until_property_is_updated(backgroundType, "SelectEntryText", "Bitmap")
            # This number MUST NOT be higher than the initial state.
            # We must not allow document images to leak into the user profile
            self.assertEqual(number_of_images, int(get_state_as_dict(imageCollection)["EntryCount"]))

            self.xUITest.executeCommand(".uno:Sidebar") # good idea to turn off sidebar again
# vim: set shiftwidth=4 softtabstop=4 expandtab:
