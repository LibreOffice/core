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


class test_tdf174211_copy_paste_arrow_to_writer(UITestCase):

    def test_tdf174211(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf174211.odp")) as document:

            xShape = document.DrawPages[0][0]
            self.assertEqual(12250, xShape.getSize().Width)
            self.assertEqual(0, xShape.getSize().Height)
            self.assertEqual(1764, xShape.LineWidth)

            xImpressDoc = self.xUITest.getTopFocusWindow()
            xEditWin = xImpressDoc.getChild("impress_win")
            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Shape 1"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            self.xUITest.executeCommand('.uno:Copy')

        with self.ui_test.load_empty_file("writer") as writer_doc:

            self.xUITest.executeCommand('.uno:Paste')

            xShape = writer_doc.DrawPages[0][0]
            self.assertEqual(12250, xShape.getSize().Width)
            self.assertEqual(0, xShape.getSize().Height)

            # Without the fix in place, this test would have failed with
            # AssertionError: 1764 != 0
            self.assertEqual(1764, xShape.LineWidth)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
