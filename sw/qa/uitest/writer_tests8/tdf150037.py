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
from libreoffice.uno.propertyvalue import mkPropertyValues
from com.sun.star.text.TextContentAnchorType import AT_PAGE, AT_PARAGRAPH

class tdf150037(UITestCase):

    def test_tdf150037(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf150037.docx")) as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            xWriterEdit.executeAction("CLICK", mkPropertyValues({"START_POS": "14", "END_POS": "14"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Replacement"}))
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "22"}))
            windowState = xWriterEdit.getState();
            self.assertEqual(windowState[14].Value, "Fieldmark: Replacement")

    def test_tdf150037_protected(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf150037.odt")) as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            xWriterEdit.executeAction("CLICK", mkPropertyValues({"START_POS": "14", "END_POS": "14"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Replacement"}))
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "23"}))
            windowState = xWriterEdit.getState();
            self.assertEqual(windowState[14].Value, "Fieldmark: Replacement")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
