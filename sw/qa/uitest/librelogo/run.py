# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text
from com.sun.star.awt.FontSlant import ITALIC as __Slant_ITALIC__
from com.sun.star.awt.FontUnderline import NONE as __Underline_NONE__
from com.sun.star.awt.FontUnderline import SINGLE as __Underline_SINGLE__
from com.sun.star.awt.FontStrikeout import NONE as __Strikeout_NONE__
from com.sun.star.awt.FontStrikeout import SINGLE as __Strikeout_SINGLE__

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
        with self.ui_test.create_doc_in_start_center("writer") as document:
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
            self.assertEqual(len(document.DrawPage), 2)

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
# disable unreliable test. Depending on how busy the machine is, this may produce 3 or 4
#            self.assertEqual(len(document.DrawPage), 3)

   def check_label(self, hasCustomLock):
        sLock = "CLEARSCREEN "
        if hasCustomLock:
            sLock = sLock + "SLEEP -1 "
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            # to check the state of LibreLogo program execution
            xIsAlive = self.getScript("__is_alive__")

            #1 run a program with basic LABEL command

            type_text(xWriterEdit, sLock + "LABEL 'Hello, World!'")
            self.logo("run")
            # wait for LibreLogo program termination
            while xIsAlive.invoke((), (), ())[0]:
                pass

            # turtle and text shape
            self.assertEqual(len(document.DrawPage), 2)
            textShape = document.DrawPage[1]
            # text in the text shape
            self.assertEqual(textShape.getString(), "Hello, World!")

            #2 check italic, bold, underline + red and blue formatting

            document.Text.String = sLock + "LABEL '<i><red>Hello</red>, <bold><blue>W<u>orld</blue></bold>!</i></u>'"
            self.logo("run")
            # wait for LibreLogo program termination
            while xIsAlive.invoke((), (), ())[0]:
                pass

            # turtle and text shape
            self.assertEqual(len(document.DrawPage), 2)
            textShape = document.DrawPage[1]
            # text in the text shape
            self.assertEqual(textShape.getString(), "Hello, World!")
            # check portion formatting
            c = textShape.createTextCursor()
            c.gotoStart(False)
            # before character "H"
            self.assertEqual(c.CharPosture, __Slant_ITALIC__) # cursive
            self.assertEqual(c.CharUnderline, __Underline_NONE__) # no underline
            self.assertEqual(c.CharWeight, 100) # normal weight
            self.assertEqual(c.CharColor, 0xFF0000) # red color
            # after character " "
            c.goRight(6, False)
            self.assertEqual(c.CharPosture, __Slant_ITALIC__) # cursive
            self.assertEqual(c.CharUnderline, __Underline_NONE__) # no underline
            self.assertEqual(c.CharWeight, 100) # normal weight
            self.assertEqual(c.CharColor, 0x000000) # black color
            # after character "W"
            c.goRight(2, False)
            self.assertEqual(c.CharPosture, __Slant_ITALIC__) # cursive
            self.assertEqual(c.CharUnderline, __Underline_NONE__) # no underline
            self.assertEqual(c.CharWeight, 150) # bold
            self.assertEqual(c.CharColor, 0x0000FF) # blue color
            # 9th: after character "o"
            c.goRight(1, False)
            self.assertEqual(c.CharPosture, __Slant_ITALIC__) # cursive
            self.assertEqual(c.CharUnderline, __Underline_SINGLE__) # underline
            self.assertEqual(c.CharWeight, 150) # bold
            self.assertEqual(c.CharColor, 0x0000FF) # blue color
            # last: after character "!"
            c.gotoEnd(False)
            self.assertEqual(c.CharPosture, __Slant_ITALIC__) # cursive
            self.assertEqual(c.CharUnderline, __Underline_SINGLE__) # underline
            self.assertEqual(c.CharWeight, 100) # normal weight
            self.assertEqual(c.CharColor, 0x000000) # black color

            #2 check strike out, sub, sup, font name and font size formatting

            document.Text.String = (
                sLock + "FONTFAMILY 'Linux Biolinum G' FONTSIZE 12 " +
                "LABEL '<s>x</s>, <sub>x</sub>, <sup>x</sup>, " +
                    "<FONTFAMILY Liberation Sans>x</FONTFAMILY>, " +
                    "<FONTHEIGHT 20>x</FONTHEIGHT>...'" )

            self.logo("run")
            # wait for LibreLogo program termination
            while xIsAlive.invoke((), (), ())[0]:
                pass

            # turtle and text shape
            self.assertEqual(len(document.DrawPage), 2)
            textShape = document.DrawPage[1]
            # text in the text shape
            self.assertEqual(textShape.getString(), "x, x, x, x, x...")
            # check portion formatting
            c = textShape.createTextCursor()
            c.gotoStart(False)
            # check portion formatting
            c = textShape.createTextCursor()
            c.gotoStart(False)

            # strike out
            self.assertEqual(c.CharStrikeout, __Strikeout_SINGLE__) # strike out
            c.goRight(4, False)

            # subscript
            self.assertEqual(c.CharStrikeout, __Strikeout_NONE__) # no strike out
            self.assertEqual(c.CharEscapement, -14000) # magic number for default subscript, see DFLT_ESC_AUTO_SUB
            self.assertEqual(c.CharEscapementHeight, 58) # size in percent
            c.goRight(3, False)

            # superscript
            self.assertEqual(c.CharEscapement, 14000) # magic number for default superscript, see DFLT_ESC_AUTO_SUPER
            self.assertEqual(c.CharEscapementHeight, 58) # size in percent
            c.goRight(3, False)

            # font family
            self.assertEqual(c.CharEscapement, 0) # no superscript
            self.assertEqual(c.CharEscapementHeight, 100) # no superscript
            self.assertEqual(c.CharFontName, "Liberation Sans") # new font family
            c.goRight(3, False)

            # font size
            self.assertEqual(c.CharFontName, "Linux Biolinum G") # default font family
            self.assertEqual(c.CharHeight, 20) # new font size
            c.goRight(3, False)

            # default font size
            self.assertEqual(c.CharHeight, 12)

            #3 check colors

            document.Text.String = ( sLock +
                "LABEL '<red>x</red>, <BLUE>x</BLUE>, " +  # check ignoring case
                    "<FONTCOLOR GREEN>x</FONTCOLOR>, " +   # check with command
                    "<FONTCOLOR 0x0000FF>x, " +            # check with hexa code
                    "<FILLCOLOR ORANGE>x</FILLCOLOR>, " +  # blue text with orange highlighting
                    "<FILLCOLOR 0xFF00FF>x</FILLCOLOR>" +  # blue text with purple highlighting
                    "...</FONTCOLOR>'" )

            self.logo("run")
            # wait for LibreLogo program termination
            while xIsAlive.invoke((), (), ())[0]:
                pass

            # turtle and text shape
            self.assertEqual(len(document.DrawPage), 2)
            textShape = document.DrawPage[1]
            # text in the text shape
            self.assertEqual(textShape.getString(), "x, x, x, x, x, x...")
            # check portion formatting
            c = textShape.createTextCursor()
            c.gotoStart(False)
            # check portion formatting
            c = textShape.createTextCursor()
            c.gotoStart(False)

            self.assertEqual(c.CharColor, 0xFF0000) # red
            self.assertEqual(c.CharBackColor, -1) # transparent highlight
            c.goRight(4, False)

            self.assertEqual(c.CharColor, 0x0000FF) # blue
            self.assertEqual(c.CharBackColor, -1) # transparent highlight
            c.goRight(3, False)

            self.assertEqual(c.CharColor, 0x008000) # green
            self.assertEqual(c.CharBackColor, -1) # transparent highlight
            c.goRight(3, False)

            self.assertEqual(c.CharColor, 0x0000FF) # blue
            self.assertEqual(c.CharBackColor, -1) # transparent highlight
            c.goRight(3, False)

            self.assertEqual(c.CharColor, 0x0000FF) # blue
            self.assertEqual(c.CharBackColor, 0xFFA500) # orange highlight
            c.goRight(3, False)

            self.assertEqual(c.CharColor, 0x0000FF) # blue
            self.assertEqual(c.CharBackColor, 0xFF00FF) # purple highlight
            c.goRight(3, False)

            self.assertEqual(c.CharColor, 0x0000FF) # blue
            self.assertEqual(c.CharBackColor, -1) # transparent highlight

            #4 check font features

            document.Text.String = (
                sLock + "FONTFAMILY 'Linux Biolinum G' " +
                "LABEL 'a <smcp>smcp <pnum>1<onum>1</pnum> 1</onum>1</smcp>...'" )

            self.logo("run")
            # wait for LibreLogo program termination
            while xIsAlive.invoke((), (), ())[0]:
                pass

            # turtle and text shape
            self.assertEqual(len(document.DrawPage), 2)
            textShape = document.DrawPage[1]
            # text in the text shape
            self.assertEqual(textShape.getString(), "a smcp 11 11...")
            # check portion formatting
            c = textShape.createTextCursor()
            c.gotoStart(False)
            # check portion formatting
            c = textShape.createTextCursor()
            c.gotoStart(False)

            self.assertEqual(c.CharFontName, "Linux Biolinum G")
            c.goRight(3, False)
            self.assertEqual(c.CharFontName, "Linux Biolinum G:smcp")
            c.goRight(5, False)
            self.assertEqual(c.CharFontName, "Linux Biolinum G:smcp&pnum")
            c.goRight(1, False)
            self.assertEqual(c.CharFontName, "Linux Biolinum G:smcp&pnum&onum")
            c.goRight(2, False)
            self.assertEqual(c.CharFontName, "Linux Biolinum G:smcp&onum")
            c.goRight(1, False)
            self.assertEqual(c.CharFontName, "Linux Biolinum G:smcp")

   def test_LABEL(self):
        self.check_label(False)

   def test_custom_lock(self):
        self.check_label(True)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
