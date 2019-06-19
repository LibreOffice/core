# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import unittest
from uitest.uihelper.common import get_state_as_dict

dialogs = [
    {"command": ".uno:OpenRemote", "closeButton": "cancel"},
    {"command": ".uno:SaveAsTemplate", "closeButton": "cancel"},
    {"command": ".uno:SaveAsRemote", "closeButton": "cancel"},
    {"command": ".uno:ExportToPDF", "closeButton": "cancel"},
    {"command": ".uno:Print", "closeButton": "cancel"},
    {"command": ".uno:PrinterSetup", "closeButton": "cancel"},
    {"command": ".uno:SetDocumentProperties", "closeButton": "cancel"},
    {"command": ".uno:PasteSpecial", "closeButton": "cancel"},
    {"command": ".uno:SearchDialog", "closeButton": "close"},
    {"command": ".uno:InsertObjectChart", "closeButton": "cancel"},
    {"command": ".uno:DataDataPilotRun", "closeButton": "cancel", "skipTestOK": True},
        # OK button triggers a new dialog
    {"command": ".uno:FunctionDialog", "closeButton": "cancel"},
    {"command": ".uno:InsertName", "closeButton": "close"},
    {"command": ".uno:InsertObjectFloatingFrame", "closeButton": "cancel"},
    {"command": ".uno:FontworkGalleryFloater", "closeButton": "cancel"},
    # {"command": ".uno:HyperlinkDialog", "closeButton": "ok"},
        # dialog opens but is not recognised by execute_dialog_through_command
    {"command": ".uno:InsertSymbol", "closeButton": "cancel"},
    {"command": ".uno:EditHeaderAndFooter", "closeButton": "cancel"},
    {"command": ".uno:InsertSignatureLine", "closeButton": "cancel"},
    {"command": ".uno:FormatCellDialog", "closeButton": "cancel"},
    {"command": ".uno:RowHeight", "closeButton": "cancel"},
    {"command": ".uno:SetOptimalRowHeight", "closeButton": "cancel"},
    {"command": ".uno:ColumnWidth", "closeButton": "cancel"},
    {"command": ".uno:SetOptimalColumnWidth", "closeButton": "cancel"},
    {"command": ".uno:PageFormatDialog", "closeButton": "cancel"},
    {"command": ".uno:EditPrintArea", "closeButton": "cancel"},
    # {"command": ".uno:ConditionalFormatMenu", "closeButton": "cancel"},
        # dialog does not open
    # {"command": ".uno:AutoFormat", "closeButton": "cancel"},
        # dialog does not open
    # {"command": ".uno:ChooseDesign", "closeButton": "cancel"},
        # dialog opens but is not recognised by execute_dialog_through_command
    {"command": ".uno:StyleNewByExample", "closeButton": "cancel"},
    {"command": ".uno:InsertCell", "closeButton": "cancel"},
    {"command": ".uno:DeleteCell", "closeButton": "cancel"},
    {"command": ".uno:Insert", "closeButton": "cancel"},
    {"command": ".uno:Add", "closeButton": "cancel"},
    # {"command": ".uno:InsertExternalDataSource", "closeButton": "cancel"},
        # dialog opens and closes, but is not recognized as closed by close_dialog_through_button
    {"command": ".uno:Delete", "closeButton": "cancel"},
    {"command": ".uno:RandomNumberGeneratorDialog", "closeButton": "close"},
    {"command": ".uno:Delete", "closeButton": "cancel"},
    {"command": ".uno:AddName", "closeButton": "cancel"},
    {"command": ".uno:DefineName", "closeButton": "cancel"},
    # {"command": ".uno:SheetInsertName", "closeButton": "close"},
        # dialog does not open
    {"command": ".uno:CreateNames", "closeButton": "cancel", "skipTestOK": True},
        # OK button triggers a new dialog
    {"command": ".uno:DefineLabelRange", "closeButton": "cancel"},
    {"command": ".uno:RenameTable", "closeButton": "cancel"},
    {"command": ".uno:Move", "closeButton": "cancel"},
    {"command": ".uno:SetTabBgColor", "closeButton": "cancel"},
    {"command": ".uno:TableEvents", "closeButton": "cancel"},
    {"command": ".uno:DefineLabelRange", "closeButton": "cancel"},
    {"command": ".uno:DataSort", "closeButton": "cancel"},
    {"command": ".uno:DataFilterAutoFilter", "closeButton": "ok", "skipTestOK": True},
        # don't test OK button twice
    {"command": ".uno:DataFilterStandardFilter", "closeButton": "cancel"},
    {"command": ".uno:DataFilterSpecialFilter", "closeButton": "cancel", "skipTestOK": True},
        # OK button triggers a new dialog
    {"command": ".uno:DefineDBName", "closeButton": "cancel"},
    {"command": ".uno:SelectDB", "closeButton": "cancel"},
    # {"command": ".uno:InsertPivotTable", "closeButton": "cancel"},
        # dialog does not open
    {"command": ".uno:Validation", "closeButton": "cancel"},
    {"command": ".uno:DataSubTotals", "closeButton": "cancel"},
    {"command": ".uno:DataForm", "closeButton": "close"},
    {"command": ".uno:DataConsolidate", "closeButton": "cancel"},
    {"command": ".uno:Group", "closeButton": "cancel"},
    {"command": ".uno:SamplingDialog", "closeButton": "cancel"},
    {"command": ".uno:DescriptiveStatisticsDialog", "closeButton": "cancel"},
    {"command": ".uno:AnalysisOfVarianceDialog", "closeButton": "cancel"},
    {"command": ".uno:CorrelationDialog", "closeButton": "cancel"},
    {"command": ".uno:CovarianceDialog", "closeButton": "cancel"},
    {"command": ".uno:ExponentialSmoothingDialog", "closeButton": "cancel"},
    {"command": ".uno:MovingAverageDialog", "closeButton": "cancel"},
    {"command": ".uno:RegressionDialog", "closeButton": "cancel"},
    {"command": ".uno:TTestDialog", "closeButton": "cancel"},
    {"command": ".uno:FTestDialog", "closeButton": "cancel"},
    {"command": ".uno:ZTestDialog", "closeButton": "cancel"},
    {"command": ".uno:ChiSquareTestDialog", "closeButton": "cancel"},
    {"command": ".uno:FourierAnalysisDialog", "closeButton": "cancel"},
    # {"command": ".uno:SpellDialog", "closeButton": "close"},
        # an extra dialog appears
    {"command": ".uno:Hyphenate", "closeButton": "cancel"},
    # {"command": ".uno:ChineseConversion", "closeButton": "cancel"},
        # tdf#125985
    # {"command": ".uno:HangulHanjaConversion", "closeButton": "cancel"},
        # tdf#125982
    {"command": ".uno:AutoCorrectDlg", "closeButton": "cancel"},
    {"command": ".uno:GoalSeekDialog", "closeButton": "cancel", "skipTestOK": True},
        # OK button triggers a new dialog
    {"command": ".uno:SolverDialog", "closeButton": "close", "skipTestOK": True},
        # OK button triggers a new dialog
    {"command": ".uno:ShareDocument", "closeButton": "cancel"},
    {"command": ".uno:Protect", "closeButton": "cancel"},
    {"command": ".uno:ToolProtectionDocument", "closeButton": "cancel"},
    {"command": ".uno:RunMacro", "closeButton": "cancel"},
    {"command": ".uno:ScriptOrganizer", "closeButton": "close"},
    {"command": ".uno:ShowLicense", "closeButton": "close"},
    {"command": ".uno:About", "closeButton": "close"},
]


def load_tests(loader, tests, pattern):
    return unittest.TestSuite(openDialogs(dialog)
                              for dialog in dialogs)

# Test to open all listed dialogs one by one, close it with the given close button
# and if there is an "OK" button open the dialog again and close it by using the OK button
# the test only checks if calc crashes by opening the dialog, see e.g. tdf#120227, tdf#125985, tdf#125982
class openDialogs(UITestCase):
    def check(self, dialog):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        self.ui_test.execute_dialog_through_command(dialog['command'])
        xDialog = self.xUITest.getTopFocusWindow()
        xCloseBtn = xDialog.getChild(dialog['closeButton'])
        if 'skipTestOK' in dialog and dialog['skipTestOK'] == True:
            xOKBtn = None
        else:
            try:
                xOKBtn = xDialog.getChild("ok")
                if (get_state_as_dict(xOKBtn)["Enabled"] != "true"):
                    xOKBtn = None
            except:
                xOKBtn = None

        self.ui_test.close_dialog_through_button(xCloseBtn)
        if (xOKBtn != None):
            print("check also OK button")
            self.ui_test.execute_dialog_through_command(dialog['command'])
            xDialog = self.xUITest.getTopFocusWindow()
            xOKBtn = xDialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOKBtn)
        self.ui_test.close_doc()


dialogCount = 0
for dialog in dialogs:
    dialogCount = dialogCount + 1


    def ch(dialog):
        return lambda self: self.check(dialog)


    setattr(openDialogs, "test_%02d_%s" % (dialogCount, dialog["command"]), ch(dialog))
# vim: set shiftwidth=4 softtabstop=4 expandtab:
