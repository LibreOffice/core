# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import select_pos
import unohelper
from com.sun.star.util import URL
from com.sun.star.frame import XStatusListener

class ValueStatusListener(XStatusListener, unohelper.Base):
    def __init__(self, xContext, xModel, url):
        self.State = None
        self.url = URL()
        self.url.Complete = url

        ut = xContext.ServiceManager.createInstance("com.sun.star.util.URLTransformer")
        _, self.url = ut.parseStrict(self.url)

        self.xController = xModel.CurrentController
        self.xDispatcher = self.xController.queryDispatch(self.url, "_self", 0)
        self.xDispatcher.addStatusListener(self, self.url)

    def remove_listener(self):
        self.xDispatcher.removeStatusListener(self, self.url)

    def statusChanged(self, event):
        self.State = event.State

def GetCommandStatus(xContext, xModel, url):
    xListener = ValueStatusListener(xContext, xModel, url)
    xListener.remove_listener()
    return xListener.State

class tdf134166(UITestCase):
    def test_tdf134166(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf134166.fodt")) as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            # Document should open with LTR page
            self.assertEqual(0, document.StyleFamilies.PageStyles.Standard.WritingMode);

            # Command state should reflect LTR
            self.assertEqual(True, GetCommandStatus(self.xContext, document, ".uno:ParaLeftToRight"))
            self.assertEqual(False, GetCommandStatus(self.xContext, document, ".uno:ParaRightToLeft"))

            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as PageDialog:
                TabControl = PageDialog.getChild("tabcontrol")
                select_pos(TabControl, "1")
                FlowCombo = PageDialog.getChild("comboTextFlowBox")
                select_by_text(FlowCombo, "Right-to-left (horizontal)")

            # Page should now be RTL
            self.assertEqual(1, document.StyleFamilies.PageStyles.Standard.WritingMode);

            # Command state should reflect RTL
            self.assertEqual(False, GetCommandStatus(self.xContext, document, ".uno:ParaLeftToRight"))
            self.assertEqual(True, GetCommandStatus(self.xContext, document, ".uno:ParaRightToLeft"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
