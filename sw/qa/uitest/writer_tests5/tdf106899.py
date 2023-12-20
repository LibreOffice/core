# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import org.libreoffice.unotest
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

class tdf106899(UITestCase):

    def test_tdf106899_alphabetical_index_utf8(self):
        # Copy concordance file containing an utf8 index entry
        org.libreoffice.unotest.makeCopyFromTDOC("tdf106899.sdi")
        with self.ui_test.load_file(get_url_for_data_file("tdf106899.odt")) as document:

            # Update the alphabetical index and check if it contains the utf8 index entry
            xDocumentIndexes = document.DocumentIndexes
            self.assertEqual(len(xDocumentIndexes), 1)
            self.assertEqual(xDocumentIndexes.hasByName("Alphabetical Index1"), True)
            xDocumentIndex = xDocumentIndexes.getByName("Alphabetical Index1")
            xIndexAnchor = xDocumentIndex.getAnchor()
            self.assertEqual("Nguyễn Khánh" in xIndexAnchor.getString(), False)

            # TODO Bug Report - Refresh of the index does only work using .uno:UpdateAllIndexes
            # It does not work with xDocumentIndex.refresh() nor with xDocumentIndex.update()
            self.xUITest.executeCommand(".uno:UpdateAllIndexes")

            # TODO Bug Report - Retrieving the text of the updated index only works using the cursor
            # It does not work with xIndexAnchor.getString()
            xCursor = document.getText().createTextCursor()
            xCursor.gotoRange(xDocumentIndex.getAnchor().getEnd(), False)
            xCursor.gotoStartOfParagraph(True)

            # Without the fix in place the index does not contain the utf8 index entry
            self.assertEqual("Nguyễn Khánh" in xCursor.getString(), True)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
