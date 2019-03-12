# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text

import time, re
from uitest.debug import sleep
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos

class LibreLogoCompileTest(UITestCase):
   LIBRELOGO_PATH = "vnd.sun.star.script:LibreLogo|LibreLogo.py$%s?language=Python&location=share"
   LS = "#_@L_i_N_e@_#" # LibreLogo line separator for debug feature "jump to the bad LibreLogo program line"

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

   def test_compile_librelogo(self):
        # XScript interface to LibreLogo Python compiler
        xCompile = self.getScript("__compil__")

        for test in (
                # BASE COMMANDS
                ("FORWARD 1 forward 1 fd 1", "forward(1)\nforward(1)\nforward(1)"),
                ("BACK 1 back 1 bk 1", "backward(1)\nbackward(1)\nbackward(1)"),
                ("RIGHT 1 right 1 rt 1", "turnright(1)\nturnright(1)\nturnright(1)"),
                ("LEFT 1 left 1 lt 1", "turnleft(1)\nturnleft(1)\nturnleft(1)"),
                # COMMENTS
                ("fd 1; comment\n; full-line comment", "forward(1)\n" + self.LS),
                # MULTI-LINE COMMAND
                ("LABEL ~\n10 + 10", "label(10 + 10)\n" + self.LS),
                # UNIT SPECIFIERS
                # check specifier "pt" (point)
                ("fd 1pt", "forward(1)"),
                # check specifier "pt" (point)
                ("fd 0.5pt", "forward(0.5)"),
                # check "in" (inch)
                ("fd 1in", "forward(72.0)"),
                # check "cm" (centimeter)
                ("fd 1cm", "forward(%s)" % (1/(2.54/72))),
                # check "mm" (millimeter)
                ("fd 10mm", "forward(%s)" % (1/(2.54/72))),
                # angle
                ("rt 90°", "turnright(90)"),
                # clock-position
                ("rt 3h", "turnright(90.0)"),
                # CONDITION
                ("if a = 1 [ ]", "if a == 1 :\n __checkhalt__()"),
                # with else
                ("if a == 1 [ ] [ ]", "if a == 1 :\n __checkhalt__()\nelse:\n __checkhalt__()"),
                # LOOPS
                ("repeat 10 [ ]", "for REPCOUNT in range(1, 1+int(10 )):\n __checkhalt__()"),
                # endless loop
                ("repeat [ ]", "REPCOUNT0 = 1\nwhile True:\n __checkhalt__()\n REPCOUNT = REPCOUNT0\n REPCOUNT0 += 1"),
                # loop in loop
                ("repeat 10 [ repeat REPCOUNT [ a=1 ] ]", "for REPCOUNT in range(1, 1+int(10 )):\n __checkhalt__()\n for REPCOUNT in range(1, 1+int(REPCOUNT )):\n  __checkhalt__()\n  a=1"),
                # while
                ("WHILE REPCOUNT < 10 [ ]", "REPCOUNT2 = 1\nwhile REPCOUNT2 < 10 :\n __checkhalt__()\n REPCOUNT = REPCOUNT2\n REPCOUNT2 += 1"),
                # for
                ("FOR i in [1, 2, 3] [ ]", "REPCOUNT4 = 1\nfor i in [1, 2, 3] :\n __checkhalt__()\n REPCOUNT = REPCOUNT4\n REPCOUNT4 += 1"),
                # PROCEDURE
                ("TO x\nLABEL 2\nEND", "global x\ndef x():\n __checkhalt__()\n %s\n label(2)\n %s" % (((self.LS),)*2)),
                # FUNCTION
                ("TO x\nOUTPUT 3\nEND", "global x\ndef x():\n __checkhalt__()\n %s\n return 3\n %s" % (((self.LS),)*2)),
                # PROCEDURE WITH ARGUMENTS
                ("TO x y\nLABEL y\nEND\nx 25", "global x\ndef x(y):\n __checkhalt__()\n %s\n label(y)\n %s\n%s\nx(25)" % (((self.LS),)*3)),
                ("TO x :y :z\nLABEL :y + :z\nEND\nx 25 26", "global x\ndef x(_y, _z):\n __checkhalt__()\n %s\n label(_y + _z)\n %s\n%s\nx(25, 26)" % (((self.LS),)*3)),
                ("TO x :y :z\nLABEL :y + :z\nEND\nx 25 :w", "global x\ndef x(_y, _z):\n __checkhalt__()\n %s\n label(_y + _z)\n %s\n%s\nx(25, _w)" % (((self.LS),)*3)),
                # UNICODE VARIABLE NAMES
                ("Erdős=1", "Erd__u__0151s=1"),
                # STRING CONSTANTS
                ("LABEL \"label", "label(u'label')"),
                ("LABEL “label”", "label(u'label')"),
                ("LABEL 'label'", "label(u'label')"),
                ("LABEL ‘label’", "label(u'label')"),
                # check apostrophe and quote usage within strings
                ("LABEL “label\’s”", "label(u'label’s')"),
                ("LABEL ““It\’s quote\’s...\””", "label(u'“It’s quote’s...”')"),
                ("LABEL ““It\\'s quote\\'s...\””", "label(u'“It\\'s quote\\'s...”')"),
                # CONVERSION OF LOGO EXPRESSIONS
                ("a=SIN 100 + COS 100", "a=sin(100 + cos(100))"),
                ("a=SIN(101) + COS(101)", "a=sin(101) + cos(101)"),
                ("a=(SIN 102) + (COS 102)", "a=(sin(102)) + (cos(102))"),
                ("a=SIN 103 + COS 103 - SQRT 103", "a=sin(103 + cos(103 - sqrt(103)))"),
                ("a=(SIN 104 + COS 104) - SQRT 104", "a=(sin(104 + cos(104))) - sqrt(104)"),
                # SIN(x) is Python-like, SIN (x) is Logo-like syntax
                ("a=SIN(105) + COS (105) - SQRT 105", "a=sin(105) + cos((105) - sqrt(105))"),
                ("a=COUNT [1, 2, 3]", "a=len([1, 2, 3])"),
                ("PRINT COUNT [1, 2, 3]", "Print(len([1, 2, 3]))"),
                ("PRINT 'TEXT: ' + 'CHAR'[0] + ' TEXT2: ' + variable[-1]", "Print(u'TEXT: ' + u'CHAR'[0] + u' TEXT2: ' + variable[-1])"),
                ("PRINT 'TEXT: ' + 'CHAR'[0][n] + ' TEXT2: ' + varia[len k][i+1]", "Print(u'TEXT: ' + u'CHAR'[0][n] + u' TEXT2: ' + varia[len(k)][i+1])"),
                ("a=SQRT COUNT [1, 2, 3]", "a=sqrt(len([1, 2, 3]))"),
                ("a=RANGE 1", "a=range(1,)"),
                ("a=RANGE 1 10", "a=range(1, 10,)"),
                ("a=RANGE 1 10 5", "a=range(1, 10, 5)"),
                ("a=RANDOM 40 + 120", "a=Random(40 + 120)"),
                ("a=RANDOM(40) + 120", "a=Random(40) + 120"),
                ("a=RANDOM [1, 2, 3]", "a=Random([1, 2, 3])"),
                ("a=[sin 90 + cos 15, cos 100 * x, sqrt 25 * 25]", "a=[sin(90 + cos(15)), cos(100 * x), sqrt(25 * 25)]"),
                ("a=[sin 90 + cos 15, cos 100 * x, sqrt 25 * 25]", "a=[sin(90 + cos(15)), cos(100 * x), sqrt(25 * 25)]"),
                ("a=[sin(90) + cos 15, cos(100) * x, sqrt(25) * 25]", "a=[sin(90) + cos(15), cos(100) * x, sqrt(25) * 25]"),
                ("TO x y z\nOUTPUT 3\nEND", "global x\ndef x(y, z):\n __checkhalt__()\n %s\n return 3\n %s" % (((self.LS),)*2)),
                ("TO x\nOUTPUT 3\nEND", "global x\ndef x():\n __checkhalt__()\n %s\n return 3\n %s" % (((self.LS),)*2)),
                ("TO f x y z\nOUTPUT x+y+z\nEND\na = [-sin -len f [-cos 45, 6] -len [1, 2, 3] -sin -90", "global f\ndef f(x, y, z):\n __checkhalt__()\n %s\n return x+y+z\n %s\n%s\na = [-sin(-len(f([-cos(45), 6], -len([1, 2, 3]), -sin(-90))))" % (((self.LS),)*3)),
                ("TO f x y z\nOUTPUT x+y+z\nEND\na = [sin len f [cos 45, 6] [1, 2, 3] sin 90", "global f\ndef f(x, y, z):\n __checkhalt__()\n %s\n return x+y+z\n %s\n%s\na = [sin(len(f([cos(45), 6], [1, 2, 3], sin(90))))" % (((self.LS),)*3)),
                ("TO f x y z\nLABEL x+y+z\nEND\nf len [1, cos 2, [65]] sqrt len [1, 2, 3, 4] sin 90 * cos 270", "global f\ndef f(x, y, z):\n __checkhalt__()\n %s\n label(x+y+z)\n %s\n%s\nf(len([1, cos(2), [65]]), sqrt(len([1, 2, 3, 4])), sin(90 * cos(270)))" % (((self.LS),)*3)),
                ("TO f x y z\nLABEL x+y+z\nEND\nf len([1, cos 2, [65]]) sqrt(len [1, 2, 3, 4]) sin(90) * cos 270", "global f\ndef f(x, y, z):\n __checkhalt__()\n %s\n label(x+y+z)\n %s\n%s\nf(len([1, cos(2), [65]]), sqrt(len([1, 2, 3, 4])), sin(90) * cos(270))" % (((self.LS),)*3)),
                ("TO f x y z\nLABEL x+y+z\nEND\nf (len [1, cos 2, [65]]) (sqrt len [1, 2, 3, 4]) (sin 90) * (cos 270)", "global f\ndef f(x, y, z):\n __checkhalt__()\n %s\n label(x+y+z)\n %s\n%s\nf((len([1, cos(2), [65]])), (sqrt(len([1, 2, 3, 4]))), (sin(90)) * (cos(270)))" % (((self.LS),)*3)),
                # arbitrary order of function definitions and calls
                ("f 1 1 f 2 2\nTO f x y\nPRINT x + y\nEND", "global f\nf(1, 1)\nf(2, 2)\n%s\ndef f(x, y):\n __checkhalt__()\n %s\n Print(x + y)\n %s" % (((self.LS),)*3)),
                ):
            compiled = xCompile.invoke((test[0],), (), ())[0]
            self.assertEqual(test[1], re.sub(r'(\n| +\n)+', '\n', re.sub(r'\( ', '(', compiled)).strip())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
