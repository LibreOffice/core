# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class Tdf118883(UITestCase):

   def test_tdf118883(self):
        with self.ui_test.create_doc_in_start_center("writer") as writer_document:

            # Insert shape with Ctrl key
            xArgs = mkPropertyValues({"KeyModifier": 8192})
            self.xUITest.executeCommandWithParameters(".uno:BasicShapes.rectangle", xArgs)

            self.assertEqual(1, writer_document.DrawPage.getCount())

            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("calc") as calc_document:

            self.xUITest.executeCommand(".uno:Paste")

            # Without the fix in place, this test would have failed with
            # AssertionError: 1 != 0
            self.assertEqual(1, calc_document.DrawPages[0].getCount())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
