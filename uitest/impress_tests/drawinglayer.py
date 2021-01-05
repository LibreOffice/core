# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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
        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        change_measurement_unit(self, 'Centimeter')

        xImpressDoc = self.xUITest.getTopFocusWindow()

        document = self.ui_test.get_component()
        self.assertEqual(1400, document.DrawPages[0].getByIndex(0).Position.X)
        self.assertEqual(628, document.DrawPages[0].getByIndex(0).Position.Y)
        self.assertEqual(1400, document.DrawPages[0].getByIndex(1).Position.X)
        self.assertEqual(3685, document.DrawPages[0].getByIndex(1).Position.Y)


        xEditWin = xImpressDoc.getChild("impress_win")
        xDrawinglayerObject = xEditWin.getChild("Unnamed Drawinglayer object 1")
        xDrawinglayerObject.executeAction("MOVE", mkPropertyValues({"X": "1000", "Y":"1000"}))

        self.assertEqual(1400, document.DrawPages[0].getByIndex(0).Position.X)
        self.assertEqual(628, document.DrawPages[0].getByIndex(0).Position.Y)
        self.assertEqual(2400, document.DrawPages[0].getByIndex(1).Position.X)
        self.assertEqual(4685, document.DrawPages[0].getByIndex(1).Position.Y)

        self.assertIsNone(document.CurrentSelection)

        xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
        self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

        self.ui_test.execute_dialog_through_command(".uno:Size")

        xDialog = self.xUITest.getTopFocusWindow()

        self.assertEqual('25.2', get_state_as_dict(xDialog.getChild('MTR_FLD_WIDTH'))['Value'])
        self.assertEqual('9.13', get_state_as_dict(xDialog.getChild('MTR_FLD_HEIGHT'))['Value'])
        self.assertEqual('2.4', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_X'))['Value'])
        self.assertEqual('4.69', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_Y'))['Value'])
        self.assertEqual('0', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())
        xEditWin.executeAction("DESELECT", tuple())
        self.assertIsNone(document.CurrentSelection)

        self.ui_test.close_doc()

    def test_resize_object(self):
        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        change_measurement_unit(self, 'Centimeter')

        xImpressDoc = self.xUITest.getTopFocusWindow()

        document = self.ui_test.get_component()
        self.assertEqual(25199, document.DrawPages[0].getByIndex(0).Size.Width)
        self.assertEqual(2629, document.DrawPages[0].getByIndex(0).Size.Height)
        self.assertEqual(25199, document.DrawPages[0].getByIndex(1).Size.Width)
        self.assertEqual(9134, document.DrawPages[0].getByIndex(1).Size.Height)

        xEditWin = xImpressDoc.getChild("impress_win")

        xDrawinglayerObject = xEditWin.getChild("Unnamed Drawinglayer object 1")
        xDrawinglayerObject.executeAction("RESIZE", mkPropertyValues({"X": "500", "Y":"4000", "FRAC_X": "0.5", "FRAC_Y": "0.5"}))

        self.assertEqual(25199, document.DrawPages[0].getByIndex(0).Size.Width)
        self.assertEqual(2629, document.DrawPages[0].getByIndex(0).Size.Height)
        self.assertEqual(12600, document.DrawPages[0].getByIndex(1).Size.Width)
        self.assertEqual(4568, document.DrawPages[0].getByIndex(1).Size.Height)

        self.assertIsNone(document.CurrentSelection)

        xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
        self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

        self.ui_test.execute_dialog_through_command(".uno:Size")

        xDialog = self.xUITest.getTopFocusWindow()

        self.assertEqual('12.6', get_state_as_dict(xDialog.getChild('MTR_FLD_WIDTH'))['Value'])
        self.assertEqual('4.57', get_state_as_dict(xDialog.getChild('MTR_FLD_HEIGHT'))['Value'])
        self.assertEqual('0.95', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_X'))['Value'])
        self.assertEqual('3.84', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_Y'))['Value'])
        self.assertEqual('0', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())
        xEditWin.executeAction("DESELECT", tuple())
        self.assertIsNone(document.CurrentSelection)

        self.ui_test.close_doc()

    def test_rotate_object(self):
        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        xImpressDoc = self.xUITest.getTopFocusWindow()

        document = self.ui_test.get_component()
        self.assertEqual(25199, document.DrawPages[0].getByIndex(0).Size.Width)
        self.assertEqual(2629, document.DrawPages[0].getByIndex(0).Size.Height)
        self.assertEqual(25199, document.DrawPages[0].getByIndex(1).Size.Width)
        self.assertEqual(9134, document.DrawPages[0].getByIndex(1).Size.Height)
        self.assertEqual(0, document.DrawPages[0].getByIndex(1).RotateAngle)

        xEditWin = xImpressDoc.getChild("impress_win")

        xDrawinglayerObject = xEditWin.getChild("Unnamed Drawinglayer object 1")
        xDrawinglayerObject.executeAction("ROTATE", mkPropertyValues({"X": "500", "Y":"4000", "ANGLE": "3000"}))

        self.assertEqual(25199, document.DrawPages[0].getByIndex(0).Size.Width)
        self.assertEqual(2629, document.DrawPages[0].getByIndex(0).Size.Height)
        self.assertEqual(25199, document.DrawPages[0].getByIndex(1).Size.Width)
        self.assertEqual(9134, document.DrawPages[0].getByIndex(1).Size.Height)
        self.assertEqual(3000, document.DrawPages[0].getByIndex(1).RotateAngle)

        self.assertIsNone(document.CurrentSelection)

        xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
        self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

        self.ui_test.execute_dialog_through_command(".uno:Size")

        xDialog = self.xUITest.getTopFocusWindow()

        self.assertEqual('30', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())
        xEditWin.executeAction("DESELECT", tuple())
        self.assertIsNone(document.CurrentSelection)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
