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

class tdf159797(UITestCase):

    def test_tdf159797(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf159797.odt")) as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            xWriterEdit.executeAction("CLICK", mkPropertyValues({"START_POS": "26", "END_POS": "26"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": " "}))
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "31"}))
            windowState = xWriterEdit.getState();
            self.assertEqual(windowState[14].Value, "This is a sentence. \u2013 Here")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
