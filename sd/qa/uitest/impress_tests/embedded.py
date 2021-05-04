# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position

class EmbeddedDocument(UITestCase):

  def test_open_embedded_document(self):
    self.ui_test.load_file(get_url_for_data_file("embedded.pptx"))

    xImpressDoc = self.xUITest.getTopFocusWindow()
    xEditWin = xImpressDoc.getChild("impress_win")

    xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Object 2"}))

    xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

    document = self.ui_test.get_component()
    self.assertEqual("Name", get_cell_by_position(document, 0, 0, 0).getString())
    self.assertEqual("NWell", get_cell_by_position(document, 0, 0, 1).getString())
    self.assertEqual("PWell", get_cell_by_position(document, 0, 0, 2).getString())
    self.assertEqual("Active", get_cell_by_position(document, 0, 0, 3).getString())
    self.assertEqual("NoPoly", get_cell_by_position(document, 0, 0, 4).getString())
    self.assertEqual("Poly", get_cell_by_position(document, 0, 0, 5).getString())
    self.assertEqual("Sized", get_cell_by_position(document, 0, 0, 6).getString())

    self.xUITest.executeCommand(".uno:CloseDoc")

    self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
