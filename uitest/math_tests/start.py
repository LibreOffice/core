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

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text, select_pos

import unittest
import platform

class SimpleMathTest(UITestCase):

    def test_math_unoCommand(self):
        with self.ui_test.create_doc_in_start_center("math"):

            xMathDoc = self.xUITest.getTopFocusWindow()

            # tdf#140386
            self.xUITest.executeCommand(".uno:InsertCommandText?Text:string=backepsilon")

            xEditView = xMathDoc.getChild("editview")

            self.assertEqual("backepsilon", get_state_as_dict(xEditView)["Text"])

    def test_math_edit(self):
        with self.ui_test.create_doc_in_start_center("math"):

            xMathDoc = self.xUITest.getTopFocusWindow()

            xEditView = xMathDoc.getChild("editview")

            type_text(xEditView, "E=mc^2")
            xMathEdit = xMathDoc.getChild("editview")

            self.assertEqual("E=mc^2", get_state_as_dict(xEditView)["Text"])

    def test_complete_math(self):
        with self.ui_test.create_doc_in_start_center("math"):

            xMathDoc = self.xUITest.getTopFocusWindow()

            xList = xMathDoc.getChild("listbox")
            state = get_state_as_dict(xList)
            self.assertEqual(state["SelectEntryText"], "Unary/Binary Operators")
            select_pos(xList, "1")
            state = get_state_as_dict(xList)
            self.assertEqual(state["SelectEntryText"], "Relations")

            xMathSelector = xMathDoc.getChild("elements")

            xElement = xMathSelector.getChild("1")
            xElement.executeAction("DOUBLECLICK", tuple())

            xEditView = xMathDoc.getChild("editview")
            type_text(xEditView, "1")
            xEditView.executeAction("TYPE", mkPropertyValues({"KEYCODE":"F4"}))
            type_text(xEditView, "2")

            self.assertEqual("1 <> 2 ", get_state_as_dict(xEditView)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
