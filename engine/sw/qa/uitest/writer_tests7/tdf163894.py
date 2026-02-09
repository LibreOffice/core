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

class tdf163894(UITestCase):

    FIELD_CONTENT = "Etiam eget dui. Aliquet pede. Donec. "
    PARA_CONTENT = "Mauris Etiam eget dui. Aliquet pede. Donec. Suspendisse dui purus. Ut nonummy."

    def test_tdf163894(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf163894.fodt")) as writer_doc:
            # check visibility of the referred character style in the dialog window
            with self.ui_test.execute_modeless_dialog_through_command(".uno:InsertReferenceField", close_button="cancel") as xDialog:
                # Select set reference type
                xTreelistType = xDialog.getChild("type-ref")
                xTreeEntry = xTreelistType.getChild('5')
                self.assertEqual(get_state_as_dict(xTreeEntry)["Text"], "Styles")
                xTreeEntry.executeAction("SELECT", tuple())

                # check character styles in the STYLES list

                # This was failed (missing character styles used in the document)
                xSelect = xDialog.getChild("select-ref")
                self.assertEqual("5", get_state_as_dict(xSelect)["Children"])
                self.assertEqual("Emphasis", get_state_as_dict(xSelect.getChild(1))["Text"])

                # select character style "Emphasis"

                xSelectEntry = xSelect.getChild("1")
                self.assertEqual(get_state_as_dict(xSelectEntry)["Text"], "Emphasis")
                xSelectEntry.executeAction("SELECT", tuple())

                # Insert a cross reference for the character style "Emphasis"

                xInsert = xDialog.getChild("ok")
                xInsert.executeAction("CLICK", tuple())

            # FIXME style-ref doesn't work in the same paragraph, yet
            # so insert a new paragraph to get a working style-ref field
            # instead of "Reference source not found"
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.waitUntilAllIdlesDispatched()
            output_text = writer_doc.Text.getString().replace('\r\n', '\n')
            # text of the document starts with the new cross-reference
            self.assertEqual(self.FIELD_CONTENT + "\n" + self.PARA_CONTENT, output_text)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
