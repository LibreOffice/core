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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.calc.csv_dialog import load_csv_file

class Td117868(UITestCase):

    def test_tdf117868(self):

        with load_csv_file(self, "tdf117868.csv", False) as xDialog:
            # Set text delimiter in case it's changed by another test
            self.assertEqual('Detected (|)', get_state_as_dict(xDialog.getChild("todetectseparator"))['Text'])
            xDetected = xDialog.getChild("todetectseparator")
            xDetected.executeAction("CLICK", tuple())

            self.assertEqual('1', get_state_as_dict(xDialog.getChild("fromrow"))['Text'])

        document = self.ui_test.get_component()

        self.assertEqual("LETTER", get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual("ANIMAL", get_cell_by_position(document, 0, 1, 1).getString())
        self.assertEqual('a', get_cell_by_position(document, 0, 0, 2).getString())
        self.assertEqual('aardvark', get_cell_by_position(document, 0, 1, 2).getString())
        self.assertEqual('b', get_cell_by_position(document, 0, 0, 3).getString())
        self.assertEqual('bear', get_cell_by_position(document, 0, 1, 3).getString())
        self.assertEqual('c', get_cell_by_position(document, 0, 0, 4).getString())
        self.assertEqual('cow', get_cell_by_position(document, 0, 1, 4).getString())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
