# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text

import time
from uitest.debug import sleep
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos

class LibreLogoTest(UITestCase):
   LIBRELOGO_PATH = "vnd.sun.star.script:LibreLogo|LibreLogo.py$%s?language=Python&location=share"

   def createMasterScriptProviderFactory(self):
       xServiceManager = self.xContext.ServiceManager
       return xServiceManager.createInstanceWithContext(
           "com.sun.star.script.provider.MasterScriptProviderFactory",
           self.xContext)

   def getScript(self, command):
       xMasterScriptProviderFactory = self.createMasterScriptProviderFactory()
       document = self.ui_test.get_component()
       xScriptProvider = xMasterScriptProviderFactory.createScriptProvider(document)
       xScript = xScriptProvider.getScript(self.LIBRELOGO_PATH %command)
       self.assertIsNotNone(xScript, "xScript was not loaded")
       return xScript

   def logo(self, command):
        self.xUITest.executeCommand(self.LIBRELOGO_PATH %command)

   def test_librelogo(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        # to check the state of LibreLogo program execution
        xIsAlive = self.getScript("__is_alive__")

        # run a program with basic drawing commands FORWARD and RIGHT
        # using their abbreviated names FD and RT
        type_text(xWriterEdit, "fd 100 rt 45 fd 100")
        self.logo("run")
        # wait for LibreLogo program termination
        while xIsAlive.invoke((), (), ())[0]:
            pass
        # check shape count for
        # a) program running:
        # - turtle shape: result of program start
        # - line shape: result of turtle drawing
        # b) continuous line drawing (the regression
        # related to the fix of tdf#106792 resulted shorter line
        # segments than the turtle path and non-continuous line
        # drawing, ie. in this example, three line shapes
        # instead of a single one. See its fix in
        # commit 502e8785085f9e8b54ee383080442c2dcaf95b15)
        self.assertEqual(document.DrawPage.getCount(), 2)

        # check formatting by "magic wand"
        self.logo("__translate__")
        # a) check expansion of abbreviated commands : fd -> FORWARD, rt -> RIGHT,
        # b) check line breaking (fix for tdf#100941: new line instead of the text "\" and "n")
        self.assertEqual(document.Text.String.replace('\r\n', '\n'), "\nFORWARD 100 RIGHT 45 FORWARD 100")
        # c) check usage of real paragraphs instead of line break (tdf#120422)
        # first paragraph is empty (for working page break)
        self.assertEqual(document.Text.createEnumeration().nextElement().String, "")

        # function definitions and calls can be in arbitrary order
        document.Text.String = """
; dragon curve
TO x n
IF n = 0 [ STOP ]
x n-1
RIGHT 90
y n-1 ; it worked only as "y(n-1)"
FORWARD 10
END

TO y n
IF n = 0 [ STOP ]
FORWARD 10
x n-1
LEFT 90
y n-1
END

PICTURE ; start new line draw
x 3 ; draw only a few levels
"""
        self.logo("run")
        # wait for LibreLogo program termination
        while xIsAlive.invoke((), (), ())[0]:
            pass
        # new shape + previous two ones = 3
        #self.assertEqual(document.DrawPage.getCount(), 3)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
