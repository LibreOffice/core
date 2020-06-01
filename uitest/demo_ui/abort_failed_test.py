import unittest

from uitest.framework import UITestCase

class TestDeadlocked(UITestCase):

    @unittest.expectedFailure
    def test_failed_test(self):

        self.ui_test.create_doc_in_start_center("calc")
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xCellsDlg = self.xUITest.getTopFocusWindow()
        xNegativeNumRedCB = xCellsDlg.getChild("negnumred")
        xNegativeNumRedCB.executeAction("CLICK",tuple())
        assert False
