# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import type_text
from com.sun.star.awt.FontSlant import ITALIC as __Slant_ITALIC__
from com.sun.star.awt.FontUnderline import NONE as __Underline_NONE__
from com.sun.star.awt.FontUnderline import SINGLE as __Underline_SINGLE__
from com.sun.star.awt.FontStrikeout import NONE as __Strikeout_NONE__
from com.sun.star.awt.FontStrikeout import SINGLE as __Strikeout_SINGLE__
from com.sun.star.script.provider import theMasterScriptProviderFactory
import time

class LibreLogoTest(UITestCase):
    LIBRELOGO_PATH = "vnd.sun.star.script:LibreLogo|LibreLogo.py$%s?language=Python&location=share"
    PROGRAM_FOR = "FOR N IN [1, 2, 3, 4] [ FORWARD 100 RIGHT 45 ]"
    PROGRAM_REPEAT = "REPEAT 4 [ FORWARD 100 RIGHT 45 ]"
    PROGRAM_WHILE = "N = 0 WHILE N < 4 [ N = N + 1 FORWARD 100 RIGHT 45 ]"

    def getScript(self, command):
        xMasterScriptProviderFactory = theMasterScriptProviderFactory.get(self.xContext)
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

    def run_and_get_message(self, program):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            # to check the state of LibreLogo program execution
            xIsAlive = self.getScript("__is_alive__")

            # to check the last dialog message presented by LibreLogo
            xLastDialogMessage = self.getScript("__last_dialog_message__")

            # write the given program in the document
            type_text(xWriterEdit, program)

            # run the written program
            self.logo("run")

            # wait for LibreLogo program termination closing every opened dialog
            while xIsAlive.invoke((), (), ())[0]:
                xCurrentTopWindow = self.xUITest.getTopFocusWindow()
                if get_state_as_dict(xCurrentTopWindow)['WindowType'] == '130':
                    xDialogOk = xCurrentTopWindow.getChild('ok')
                    xDialogOk.executeAction("CLICK", tuple())
                time.sleep(self.ui_test.get_default_sleep())

            return xLastDialogMessage.invoke((), (), ())[0]

    def test_print_log10(self):
        self.assertEqual(self.run_and_get_message("print log10 1000"), "3.0")

    def test_print_sqrt(self):
        self.assertEqual(self.run_and_get_message("print sqrt 16"), "4.0")


    def set_document_default_language(self, language):
        with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:
            xPages = xDialog.getChild("pages")
            xLanguageEntry = xPages.getChild('2')
            xLanguageEntry.executeAction("EXPAND", tuple())
            xxLanguageEntryGeneralEntry = xLanguageEntry.getChild('0')
            xxLanguageEntryGeneralEntry.executeAction("SELECT", tuple())
            # mark "For the current document only" to prevent the persistence of the selection
            xCurrentDoc = xDialog.getChild("currentdoc")
            if get_state_as_dict(xCurrentDoc)['Selected'] != "true":
                xCurrentDoc.executeAction("CLICK", tuple())
            # try to select the given language sequentially in the three lists
            xWesternLanguage = xDialog.getChild("westernlanguage")
            previousWesternLanguage = get_state_as_dict(xWesternLanguage)['SelectEntryText']
            select_by_text(xWesternLanguage, language)
            if get_state_as_dict(xWesternLanguage)['SelectEntryText'] != language:
                select_by_text(xWesternLanguage, "[None]")
                xAsianLanguage = xDialog.getChild("asianlanguage")
                previousAsianLanguage = get_state_as_dict(xAsianLanguage)['SelectEntryText']
                select_by_text(xAsianLanguage, language)
                if get_state_as_dict(xAsianLanguage)['SelectEntryText'] != language:
                    select_by_text(xAsianLanguage, "[None]")
                    xComplexLanguage = xDialog.getChild("complexlanguage")
                    select_by_text(xComplexLanguage, language)
                    if get_state_as_dict(xComplexLanguage)['SelectEntryText'] != language:
                        # if not found, then restore the previous languages
                        select_by_text(xAsianLanguage, previousAsianLanguage)
                        select_by_text(xWesternLanguage, previousWesternLanguage)
            # apply the change
            xApply = xDialog.getChild("apply")
            xApply.executeAction("CLICK", tuple())

    def check_localization(self, language, program):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            self.set_document_default_language(language)
            # to check the state of LibreLogo program execution
            xIsAlive = self.getScript("__is_alive__")
            # write the given program in the document
            type_text(xWriterEdit, program)
            # translate the program to the current document language by "magic wand"
            self.logo("__translate__")
            # check that the program was changed
            self.assertNotEqual(document.Text.String.replace('\r\n', '\n'), "\n" + program)
            # run the translated program
            self.logo("run")
            # wait for LibreLogo program termination or the opening of an error dialog
            while xIsAlive.invoke((), (), ())[0]:
                xCurrentTopWindow = self.xUITest.getTopFocusWindow()
                if get_state_as_dict(xCurrentTopWindow)['WindowType'] == '130':
                    xDialogOk = xCurrentTopWindow.getChild('ok')
                    xDialogOk.executeAction("CLICK", tuple())
                    break
                time.sleep(self.ui_test.get_default_sleep())
            # turtle + line shape (if program is not executed then no lines are found)
            self.assertGreater(len(document.DrawPage), 1)

    # Tests for languages having a REPCOUNT translation with special characters
    # or with the REPEAT or COUNT translation as a substring of REPCOUNT:
    def test_localization_afrikaans_for(self): # af REPCOUNT: space + REPEAT substring
        self.check_localization("Afrikaans (South Africa)", self.PROGRAM_FOR)
    def test_localization_afrikaans_repeat(self): # af REPCOUNT: space + REPEAT substring
        self.check_localization("Afrikaans (South Africa)", self.PROGRAM_REPEAT)
    def test_localization_afrikaans_while(self): # af REPCOUNT: space + REPEAT substring
        self.check_localization("Afrikaans (South Africa)", self.PROGRAM_WHILE)
    def test_localization_catalan_for(self): # ca REPCOUNT: dot + REPEAT substring
        self.check_localization("Catalan", self.PROGRAM_FOR)
    def test_localization_catalan_repeat(self): # ca REPCOUNT: dot + REPEAT substring
        self.check_localization("Catalan", self.PROGRAM_REPEAT)
    def test_localization_catalan_while(self): # ca REPCOUNT: dot + REPEAT substring
        self.check_localization("Catalan", self.PROGRAM_WHILE)
    def test_localization_french_for(self): # fr REPCOUNT: no special condition
        self.check_localization("French (France)", self.PROGRAM_FOR)
    def test_localization_french_repeat(self): # fr REPCOUNT: no special condition
        self.check_localization("French (France)", self.PROGRAM_REPEAT)
    def test_localization_french_while(self): # fr REPCOUNT: no special condition
        self.check_localization("French (France)", self.PROGRAM_WHILE)
    def test_localization_guarani_for(self): # gug REPCOUNT: apostrophe + dot + REPEAT substring
        self.check_localization("Guarani (Paraguay)", self.PROGRAM_FOR)
    def test_localization_guarani_repeat(self): # gug REPCOUNT: apostrophe + dot + REPEAT substring
        self.check_localization("Guarani (Paraguay)", self.PROGRAM_REPEAT)
    def test_localization_guarani_while(self): # gug REPCOUNT: apostrophe + dot + REPEAT substring
        self.check_localization("Guarani (Paraguay)", self.PROGRAM_WHILE)
    def test_localization_spanish_for(self): # es REPCOUNT: dot + COUNT substring
        self.check_localization("Spanish (Spain)", self.PROGRAM_FOR)
    def test_localization_spanish_repeat(self): # es REPCOUNT: dot + COUNT substring
        self.check_localization("Spanish (Spain)", self.PROGRAM_REPEAT)
    def test_localization_spanish_while(self): # es REPCOUNT: dot + COUNT substring
        self.check_localization("Spanish (Spain)", self.PROGRAM_WHILE)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
