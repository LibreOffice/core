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
from uitest.uihelper.common import change_measurement_unit
from uitest.framework import UITestCase

class ImpressDrawinglayerTest(UITestCase):

    def test_move_object(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            change_measurement_unit(self, 'Centimeter')

            xImpressDoc = self.xUITest.getTopFocusWindow()

            self.assertIsNone(document.CurrentSelection)

            xEditWin = xImpressDoc.getChild("impress_win")
            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            with self.ui_test.execute_dialog_through_command(".uno:Size") as xDialog:
                self.assertEqual('25.2', get_state_as_dict(xDialog.getChild('MTR_FLD_WIDTH'))['Value'])
                self.assertEqual('9.13', get_state_as_dict(xDialog.getChild('MTR_FLD_HEIGHT'))['Value'])
                self.assertEqual('1.4', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_X'))['Value'])
                self.assertEqual('3.69', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_Y'))['Value'])
                self.assertEqual('0', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])

            xDrawinglayerObject = xEditWin.getChild("Unnamed Drawinglayer object 1")
            xDrawinglayerObject.executeAction("MOVE", mkPropertyValues({"X": "1000", "Y":"1000"}))

            with self.ui_test.execute_dialog_through_command(".uno:Size") as xDialog:
                self.assertEqual('25.2', get_state_as_dict(xDialog.getChild('MTR_FLD_WIDTH'))['Value'])
                self.assertEqual('9.13', get_state_as_dict(xDialog.getChild('MTR_FLD_HEIGHT'))['Value'])
                self.assertEqual('2.4', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_X'))['Value'])
                self.assertEqual('4.69', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_Y'))['Value'])
                self.assertEqual('0', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])

            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())
            xEditWin.executeAction("DESELECT", tuple())
            self.assertIsNone(document.CurrentSelection)

    def test_resize_object(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            change_measurement_unit(self, 'Centimeter')

            xImpressDoc = self.xUITest.getTopFocusWindow()

            self.assertIsNone(document.CurrentSelection)

            xEditWin = xImpressDoc.getChild("impress_win")
            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            with self.ui_test.execute_dialog_through_command(".uno:Size") as xDialog:
                self.assertEqual('25.2', get_state_as_dict(xDialog.getChild('MTR_FLD_WIDTH'))['Value'])
                self.assertEqual('9.13', get_state_as_dict(xDialog.getChild('MTR_FLD_HEIGHT'))['Value'])
                self.assertEqual('1.4', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_X'))['Value'])
                self.assertEqual('3.69', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_Y'))['Value'])
                self.assertEqual('0', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])

            xDrawinglayerObject = xEditWin.getChild("Unnamed Drawinglayer object 1")
            xDrawinglayerObject.executeAction("RESIZE", mkPropertyValues({"X": "500", "Y":"4000", "FRAC_X": "0.5", "FRAC_Y": "0.5"}))

            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            with self.ui_test.execute_dialog_through_command(".uno:Size") as xDialog:
                self.assertEqual('12.6', get_state_as_dict(xDialog.getChild('MTR_FLD_WIDTH'))['Value'])
                self.assertEqual('4.57', get_state_as_dict(xDialog.getChild('MTR_FLD_HEIGHT'))['Value'])
                self.assertEqual('0.95', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_X'))['Value'])
                self.assertEqual('3.84', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_Y'))['Value'])
                self.assertEqual('0', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])

            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())
            xEditWin.executeAction("DESELECT", tuple())
            self.assertIsNone(document.CurrentSelection)

    def test_rotate_object(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            xImpressDoc = self.xUITest.getTopFocusWindow()

            self.assertIsNone(document.CurrentSelection)

            xEditWin = xImpressDoc.getChild("impress_win")
            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            with self.ui_test.execute_dialog_through_command(".uno:Size") as xDialog:
                self.assertEqual('25.2', get_state_as_dict(xDialog.getChild('MTR_FLD_WIDTH'))['Value'])
                self.assertEqual('9.13', get_state_as_dict(xDialog.getChild('MTR_FLD_HEIGHT'))['Value'])
                self.assertEqual('1.4', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_X'))['Value'])
                self.assertEqual('3.69', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_Y'))['Value'])
                self.assertEqual('0', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])

            xDrawinglayerObject = xEditWin.getChild("Unnamed Drawinglayer object 1")
            xDrawinglayerObject.executeAction("ROTATE", mkPropertyValues({"X": "500", "Y":"4000", "ANGLE": "3000"}))

            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            with self.ui_test.execute_dialog_through_command(".uno:Size") as xDialog:
                self.assertEqual('30', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])

            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())
            xEditWin.executeAction("DESELECT", tuple())
            self.assertIsNone(document.CurrentSelection)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
