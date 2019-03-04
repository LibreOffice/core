# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class regression(UITestCase):
    def test_regression_row(self):
        self._regression_check(data_groupedby_column = False)

    def test_regression_column(self):
        self._regression_check(data_groupedby_column = True)

    def _regression_check(self, data_groupedby_column = True):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("regression.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        # Initially the final check status is "FALSE" (failed).
        self.assertEqual(get_cell_by_position(document, 14, 1, 6).getString(), "FALSE",
                         "Check status must be FALSE before the test")
        self._do_regression(regression_type = "LINEAR", data_groupedby_column = data_groupedby_column, calc_intercept = True)
        self._do_regression(regression_type = "LINEAR", data_groupedby_column = data_groupedby_column, calc_intercept = False)
        self._do_regression(regression_type = "LOG", data_groupedby_column = data_groupedby_column)
        self._do_regression(regression_type = "POWER", data_groupedby_column = data_groupedby_column)
        self.assertEqual(get_cell_by_position(document, 14, 1, 6).getString(), "TRUE",
                         "One of more of the checks failed for data_groupedby_column = {}, manually try with the document".
                         format(data_groupedby_column))
        self.ui_test.close_doc()

    def _do_regression(self, regression_type, data_groupedby_column = True, calc_intercept = True):
        assert(regression_type == "LINEAR" or regression_type == "LOG" or regression_type == "POWER")
        self.ui_test.execute_modeless_dialog_through_command(".uno:RegressionDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xvariable1rangeedit = xDialog.getChild("variable1-range-edit")
        xvariable2rangeedit = xDialog.getChild("variable2-range-edit")
        xoutputrangeedit = xDialog.getChild("output-range-edit")
        xwithlabelscheck = xDialog.getChild("withlabels-check")
        xgroupedbyrowsradio = xDialog.getChild("groupedby-rows-radio")
        xgroupedbycolumnsradio = xDialog.getChild("groupedby-columns-radio")
        xlinearradio = xDialog.getChild("linear-radio")
        xlogarithmicradio = xDialog.getChild("logarithmic-radio")
        xpowerradio = xDialog.getChild("power-radio")
        xnointerceptcheck = xDialog.getChild("nointercept-check")

        ## Set the X, Y and output ranges
        xvariable1rangeedit.executeAction("FOCUS", tuple()) # Without this the range parser does not kick in somehow
        xvariable1rangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvariable1rangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        if data_groupedby_column:
            xvariable1rangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$DataInColumns.$A$1:$C$11"}))
        else:
            xvariable1rangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$DataInRows.$A$1:$K$3"}))

        xvariable2rangeedit.executeAction("FOCUS", tuple()) # Without this the range parser does not kick in somehow
        xvariable2rangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvariable2rangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        if data_groupedby_column:
            xvariable2rangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$DataInColumns.$D$1:$D$11"}))
        else:
            xvariable2rangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$DataInRows.$A$4:$K$4"}))
        # The data ranges have labels in them
        if (get_state_as_dict(xwithlabelscheck)["Selected"]) == "false":
            xwithlabelscheck.executeAction("CLICK", tuple())

        xoutputrangeedit.executeAction("FOCUS", tuple()) # Without this the range parser does not kick in somehow
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        if regression_type == "LINEAR":
            if calc_intercept:
                xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$ActualLinear.$A$1"}))
            else:
                xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$ActualLinearNoIntercept.$A$1"}))
        elif regression_type == "LOG":
            xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$ActualLog.$A$1"}))
        else:
            xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$ActualPower.$A$1"}))

        if data_groupedby_column:
            xgroupedbycolumnsradio.executeAction("CLICK", tuple())
        else:
            xgroupedbyrowsradio.executeAction("CLICK", tuple())

        if regression_type == "LINEAR":
            xlinearradio.executeAction("CLICK", tuple())
        elif regression_type == "LOG":
            xlogarithmicradio.executeAction("CLICK", tuple())
        else:
            xpowerradio.executeAction("CLICK", tuple())

        if not calc_intercept:
            xnointerceptcheck.executeAction("CLICK", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
