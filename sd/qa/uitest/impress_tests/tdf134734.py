# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, select_pos
from com.sun.star.drawing.FillStyle import SOLID
import importlib

class TestClass(UITestCase):
    def test_master_page_background(self):
        self.ui_test.create_doc_in_start_center("impress")
        document = self.ui_test.get_component()
        TemplateDialog = self.xUITest.getTopFocusWindow()
        close = TemplateDialog.getChild("close")
        self.ui_test.close_dialog_through_button(close)

        # set margins and fill color
        self.ui_test.execute_dialog_through_command(".uno:PageSetup")
        DrawPageDialog = self.xUITest.getTopFocusWindow()
        xTabs = DrawPageDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        checkBackgroundFullSize = DrawPageDialog.getChild("checkBackgroundFullSize")
        self.assertEqual(get_state_as_dict(checkBackgroundFullSize)["Selected"], "false")
        spinMargLeft = DrawPageDialog.getChild("spinMargLeft")
        for _ in range(20):
            spinMargLeft.executeAction("UP",tuple())
        spinMargRight = DrawPageDialog.getChild("spinMargRight")
        for _ in range(15):
            spinMargRight.executeAction("UP",tuple())
        spinMargTop = DrawPageDialog.getChild("spinMargTop")
        for _ in range(10):
            spinMargTop.executeAction("UP",tuple())
        spinMargBot = DrawPageDialog.getChild("spinMargBot")
        for _ in range(5):
            spinMargBot.executeAction("UP",tuple())
        xTabs = DrawPageDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        btncolor = DrawPageDialog.getChild("btncolor")
        btncolor.executeAction("CLICK",tuple())
        ok = DrawPageDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(ok)

        self.assertEqual(
          document.DrawPages.getByIndex(0).Background.FillStyle, SOLID)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderLeft, 1016)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderRight, 762)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderTop, 508)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderBottom, 254)
        self.assertEqual(
          document.MasterPages.getByIndex(0).BackgroundFullSize, False)

        # check it
        self.ui_test.execute_dialog_through_command(".uno:PageSetup")
        DrawPageDialog = self.xUITest.getTopFocusWindow()
        xTabs = DrawPageDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        checkBackgroundFullSize = DrawPageDialog.getChild("checkBackgroundFullSize")
        self.assertEqual(get_state_as_dict(checkBackgroundFullSize)["Selected"], "false")
        checkBackgroundFullSize.executeAction("CLICK",tuple())
        ok = DrawPageDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(ok)

        self.assertEqual(
          document.DrawPages.getByIndex(0).Background.FillStyle, SOLID)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderLeft, 1016)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderRight, 762)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderTop, 508)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderBottom, 254)
        self.assertEqual(
          document.MasterPages.getByIndex(0).BackgroundFullSize, True)

        # uncheck it again
        self.ui_test.execute_dialog_through_command(".uno:PageSetup")
        DrawPageDialog = self.xUITest.getTopFocusWindow()
        xTabs = DrawPageDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        checkBackgroundFullSize = DrawPageDialog.getChild("checkBackgroundFullSize")
        self.assertEqual(get_state_as_dict(checkBackgroundFullSize)["Selected"], "true")
        checkBackgroundFullSize.executeAction("CLICK",tuple())
        ok = DrawPageDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(ok)

        self.assertEqual(
          document.DrawPages.getByIndex(0).Background.FillStyle, SOLID)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderLeft, 1016)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderRight, 762)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderTop, 508)
        self.assertEqual(
          document.DrawPages.getByIndex(0).BorderBottom, 254)
        self.assertEqual(
          document.MasterPages.getByIndex(0).BackgroundFullSize, False)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
