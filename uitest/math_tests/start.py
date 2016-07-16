# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

import time
import unittest

class SimpleMathTest(UITestCase):

    def test_start_math(self):

        self.ui_test.create_doc_in_start_center("math")

        xMathDoc = self.xUITest.getTopFocusWindow()

        self.ui_test.close_doc()

    def test_docking_window_listbox(self):

        self.ui_test.create_doc_in_start_center("math")

        xMathDoc = self.xUITest.getTopFocusWindow()

        xList = xMathDoc.getChild("listbox")
        state = get_state_as_dict(xList)
        self.assertEqual(state["SelectEntryText"], "Unary/Binary Operators")
        xList.executeAction("SELECT", mkPropertyValues({"POS": "1"}))
        state = get_state_as_dict(xList)
        self.assertEqual(state["SelectEntryText"], "Relations")

        self.ui_test.close_doc()


# vim:set shiftwidth=4 softtabstop=4 expandtab: */
