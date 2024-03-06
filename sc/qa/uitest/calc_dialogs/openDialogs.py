# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import unittest
from uitest.uihelper.testDialog import testAppDialog

dialogs = [
    {"command": ".uno:OpenRemote", "closeButton": "cancel"},
    {"command": ".uno:SaveAsTemplate", "closeButton": "cancel"},
    {"command": ".uno:SaveAsRemote", "closeButton": "cancel"},
    # {"command": ".uno:ExportToPDF", "closeButton": "cancel", "skipTestOK": True},
        # tested in sc/qa/uitest/calc_tests4/exportToPDF.py
    {"command": ".uno:Print", "closeButton": "cancel", "skipTestOK": True},
        # no printer in CI
    {"command": ".uno:PrinterSetup", "closeButton": "cancel"},
    # {"command": ".uno:SetDocumentProperties", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests/documentProperties.py
    # {"command": ".uno:PasteSpecial", "closeButton": "cancel"},
        # would need to copy first something into the clipboard
    # {"command": ".uno:SearchDialog", "closeButton": "close"},
        # tested in sc/qa/uitest/search_replace/*
    # {"command": ".uno:InsertObjectChart", "closeButton": "cancel"},
        # tested in uitest/calc_tests/create_chart.py
    # {"command": ".uno:DataDataPilotRun", "closeButton": "cancel", "skipTestOK": True},
        # tested in sc/qa/uitest/calc_tests8/tdf137726.py
    # {"command": ".uno:FunctionDialog", "closeButton": "cancel"},
        # Cancel button tested in uitest/calc_tests/function_wizard.py
        # OK button tested in sc/qa/uitest/calc_tests7/tdf123479.py
    # {"command": ".uno:InsertName", "closeButton": "close"},
        # tested in sc/qa/uitest/range_name/create_range_name.py
    {"command": ".uno:InsertObjectFloatingFrame", "closeButton": "cancel"},
    {"command": ".uno:FontworkGalleryFloater", "closeButton": "cancel"},
    # {"command": ".uno:HyperlinkDialog", "closeButton": "ok"},
        # dialog opens but is not recognised by execute_dialog_through_command
    #{"command": ".uno:InsertSymbol", "closeButton": "cancel"},
        # Tested in uitest/demo_ui/char_dialog.py
    {"command": ".uno:EditHeaderAndFooter", "closeButton": "cancel"},
    # {"command": ".uno:InsertSignatureLine", "closeButton": "cancel"},
        # tested in sc/qa/uitest/signatureLine/insertSignatureLine.py
    # {"command": ".uno:FormatCellDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests/formatCells.py
    # {"command": ".uno:RowHeight", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests/rows.py
    # {"command": ".uno:SetOptimalRowHeight", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests9/tdf126673.py
    # {"command": ".uno:ColumnWidth", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests/columns.py
    # {"command": ".uno:SetOptimalColumnWidth", "closeButton": "cancel"},
        # tested in sc/qa/uitest/textToColumns/tdf89907.py
    # {"command": ".uno:PageFormatDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/pageFormat/tdf123508.py
    {"command": ".uno:EditPrintArea", "closeButton": "cancel", "skipTestOK": True},
        # tested in sc/qa/uitest/calc_tests/printRange.py
    # {"command": ".uno:ConditionalFormatMenu", "closeButton": "cancel"},
        # dialog does not open
    # {"command": ".uno:AutoFormat", "closeButton": "cancel"},
        # dialog does not open
    # {"command": ".uno:ChooseDesign", "closeButton": "cancel"},
        # dialog opens but is not recognised by execute_dialog_through_command
    {"command": ".uno:StyleNewByExample", "closeButton": "cancel"},
    {"command": ".uno:InsertCell", "closeButton": "cancel"},
    {"command": ".uno:DeleteCell", "closeButton": "cancel"},
    # {"command": ".uno:Insert", "closeButton": "cancel", "skipTestOK": True},
        # OK button tested in uitest/calc_tests/gridwin.py
    # {"command": ".uno:Add", "closeButton": "cancel"},
        # tested in sc/qa/uitest/range_name/tdf148072.py
    # {"command": ".uno:InsertExternalDataSource", "closeButton": "cancel"},
        # dialog opens and closes, but is not recognized as closed by close_dialog_through_button
    # {"command": ".uno:Delete", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests3/clearCells.py
    # {"command": ".uno:RandomNumberGeneratorDialog", "closeButton": "close"},
        # tested in sc/qa/uitest/calc_tests/fillRandomNumber.py
    # {"command": ".uno:AddName", "closeButton": "cancel"},
        # Cancel button tested in uitest/demo_ui/edit.py
        # "add" button tested in uitest/calc_tests/create_range_name.py
    #{"command": ".uno:DefineName", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests9/tdf130371.py
    # {"command": ".uno:SheetInsertName", "closeButton": "close"},
        # dialog does not open
    #{"command": ".uno:CreateNames", "closeButton": "cancel", "skipTestOK": True},
        # tested in sc/qa/uitest/range_name/tdf137617.py
        # OK button triggers a new dialog
    #{"command": ".uno:DefineLabelRange", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests4/tdf131170.py
    # {"command": ".uno:RenameTable", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests/sheetRename.py
    # {"command": ".uno:Move", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests6/moveCopySheet.py
    {"command": ".uno:SetTabBgColor", "closeButton": "cancel"},
    {"command": ".uno:TableEvents", "closeButton": "cancel"},
    # {"command": ".uno:DataSort", "closeButton": "cancel"},
        # cancel button tested in sc/qa/uitest/calc_tests/sorting.py
        # OK button tested in sc/qa/uitest/calc_tests/naturalSort.py
    # {"command": ".uno:DataFilterAutoFilter", "closeButton": "ok", "skipTestOK": True},
        # tested in sc/qa/uitest/autofilter/autofilterBugs.py
    # {"command": ".uno:DataFilterStandardFilter", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests2/standardFilter.py
    #{"command": ".uno:DataFilterSpecialFilter", "closeButton": "cancel", "skipTestOK": True},
        # tested in sc/qa/uitest/calc_tests6/tdf147830.py
    # {"command": ".uno:DefineDBName", "closeButton": "cancel"},
        # tested in sc/qa/uitest/range_name/tdf119954.py
    {"command": ".uno:SelectDB", "closeButton": "cancel"},
    # {"command": ".uno:InsertPivotTable", "closeButton": "cancel"},
        # dialog does not open
    # {"command": ".uno:Validation", "closeButton": "cancel"},
        # tested in sc/qa/uitest/validity/validity.py
    # {"command": ".uno:DataSubTotals", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests2/subtotals.py
    # {"command": ".uno:DataForm", "closeButton": "close"},
        # tested in sc/qa/uitest/calc_tests2/dataform.py
    # {"command": ".uno:DataConsolidate", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests2/consolidate.py
    {"command": ".uno:Group", "closeButton": "cancel"},
    # {"command": ".uno:SamplingDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/sampling.py
    # {"command": ".uno:DescriptiveStatisticsDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/descriptiveStatistics.py
    # {"command": ".uno:AnalysisOfVarianceDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/anova.py
    # {"command": ".uno:CorrelationDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/correlation.py
    # {"command": ".uno:CovarianceDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/covariance.py
    # {"command": ".uno:ExponentialSmoothingDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/exponentialSmoothing.py
    # {"command": ".uno:MovingAverageDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/movingAverage.py
    # {"command": ".uno:RegressionDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/regression.py
    # {"command": ".uno:TTestDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/tTest.py
    # {"command": ".uno:FTestDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/fTest.py
    # {"command": ".uno:ZTestDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/zTest.py
    # {"command": ".uno:ChiSquareTestDialog", "closeButton": "cancel"},
        # tested in sc/qa/uitest/statistics/chiSquare.py:53
    {"command": ".uno:FourierAnalysisDialog", "closeButton": "cancel"},
    # {"command": ".uno:SpellDialog", "closeButton": "close"},
        # an extra dialog appears
    {"command": ".uno:Hyphenate", "closeButton": "cancel"},
    #{"command": ".uno:ChineseConversion", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests6/tdf104378.py
    # {"command": ".uno:AutoCorrectDlg", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests6/autocorrectOptions.py
    {"command": ".uno:GoalSeekDialog", "closeButton": "cancel", "skipTestOK": True},
        # OK button triggers a new dialog
    {"command": ".uno:SolverDialog", "closeButton": "close", "skipTestOK": True},
        # OK button triggers a new dialog
    # {"command": ".uno:ShareDocument", "closeButton": "cancel"},
        # tested in sc/qa/uitest/calc_tests6/shareSpreadsheet.py
    # {"command": ".uno:Protect", "closeButton": "cancel"},
        # tested in sc/qa/uitest/protect/protectSheet.py
    # {"command": ".uno:ToolProtectionDocument", "closeButton": "cancel"},
        # tested in sc/qa/uitest/protect/protectSpreadsheet.py
    {"command": ".uno:RunMacro", "closeButton": "cancel"},
    {"command": ".uno:ScriptOrganizer", "closeButton": "close"},
    {"command": ".uno:ShowLicense", "closeButton": "close", "skipTestOK": True},
    # {"command": ".uno:About", "closeButton": "close"},
        # tested in sc/qa/uitest/calc_tests7/tdf106667.py
]


def load_tests(loader, tests, pattern):
    return unittest.TestSuite(openDialogs(dialog)
                              for dialog in dialogs)

# Test to open all listed dialogs one by one, close it with the given close button
# and if there is an "OK" button open the dialog again and close it by using the OK button
# the test only checks if calc crashes by opening the dialog, see e.g. tdf#120227, tdf#125985, tdf#125982
class openDialogs(UITestCase):
    def check(self, dialog):
        testAppDialog(self, "calc", dialog)

dialogCount = 0
for dialog in dialogs:
    dialogCount = dialogCount + 1


    def ch(dialog):
        return lambda self: self.check(dialog)


    setattr(openDialogs, "test_%02d_%s" % (dialogCount, dialog["command"]), ch(dialog))
# vim: set shiftwidth=4 softtabstop=4 expandtab:
