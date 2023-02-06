# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import change_measurement_unit, select_pos
from uitest.framework import UITestCase

class textColumnsDialog(UITestCase):

    def test_textColumnsDialog(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            with change_measurement_unit(self, 'Centimeter'):
                xImpressDoc = self.xUITest.getTopFocusWindow()

                xEditWin = xImpressDoc.getChild("impress_win")
                xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
                self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

                # Test defaults and set some values
                with self.ui_test.execute_dialog_through_command(".uno:TextAttributes") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "2")
                    colNumber = xDialog.getChild('FLD_COL_NUMBER')
                    colSpacing = xDialog.getChild('MTR_FLD_COL_SPACING')
                    self.assertEqual('1', get_state_as_dict(colNumber)['Text'])
                    self.assertEqual('0.00 cm', get_state_as_dict(colSpacing)['Text'])
                    colNumber.executeAction("SET", mkPropertyValues({"TEXT": "3"}))
                    colSpacing.executeAction("SET", mkPropertyValues({"TEXT": "1.5"}))

                # Test that settings persist
                with self.ui_test.execute_dialog_through_command(".uno:TextAttributes") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "2")
                    colNumber = xDialog.getChild('FLD_COL_NUMBER')
                    colSpacing = xDialog.getChild('MTR_FLD_COL_SPACING')
                    self.assertEqual('3', get_state_as_dict(colNumber)['Text'])
                    self.assertEqual('1.50 cm', get_state_as_dict(colSpacing)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
