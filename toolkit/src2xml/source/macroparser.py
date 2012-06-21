#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

import sys
from globals import *
import srclexer

class MacroParser(object):

    def __init__ (self, buf):
        self.buffer = buf
        self.macro = None
        self.debug = False

    def parse (self):
        """
A macro with arguments must have its open paren immediately following 
its name without any whitespace.
"""
        if self.debug:
            print "-"*68
            print "parsing '%s'"%self.buffer

        i = 0
        bufSize = len(self.buffer)
        name, buf = '', ''
        while i < bufSize:
            c = self.buffer[i]
            if c in [' ', "\t"] and len(name) == 0:
                # This is a simple macro with no arguments.
                name = buf
                vars = []
                content = self.buffer[i:]
                self.setMacro(name, vars, content)
                return
            elif c == '(' and len(name) == 0:
                # This one has arguments.
                name = buf
                buf = self.buffer[i:]
                vars, content = self.parseArgs(buf)
                self.setMacro(name, vars, content)
                return
            else:
                buf += c
                i += 1

    def parseArgs (self, buffer):
        """Parse arguments.

The buffer is expected to be formatted like '(a, b, c)' where the first 
character is the open paren.
"""
        scope = 0
        buf = ''
        vars = []
        content = ''
        bufSize = len(buffer)
        i = 0
        while i < bufSize:
            c = buffer[i]
            if c == '(':
                scope += 1
            elif c == ')':
                scope -= 1
                if len(buf) > 0:
                    vars.append(buf)
                if scope == 0:
                    break
            elif c == ',':
                if len(buf) == 0:
                    raise globals.ParseError ('')
                vars.append(buf)
                buf = ''
            elif c in " \t" and scope > 0:
                pass
            else:
                buf += c

            i += 1

        if scope > 0:
            raise globals.ParseError ('')

        return vars, buffer[i+1:]


    def setMacro (self, name, vars, content):
        if self.debug:
            print "-"*68
            print "name: %s"%name
            for var in vars:
                print "var: %s"%var
            if len(vars) == 0:
                print "no vars"
            print "content: '%s'"%content

        if len(content) > 0:
            self.macro = Macro(name)
            for i in xrange(0, len(vars)):
                self.macro.vars[vars[i]] = i

            # tokinize it using lexer.
            mclexer = srclexer.SrcLexer(content)
            mclexer.expandHeaders = False
            mclexer.inMacroDefine = True
            mclexer.tokenize()
            self.macro.tokens = mclexer.getTokens()
            if self.debug:
                print self.macro.tokens
            
            if not self.isValidMacro(self.macro):
                self.macro = None

        if self.debug:
            if self.macro != None:
                print "macro registered!"
            else:
                print "macro not registered"

    def isValidMacro (self, macro):

        n = len(macro.tokens)
        if n == 0:
            return False
        elif len(macro.name) > 4 and macro.name[1:4] == 'ID_':
            # We don't want to expand macros like HID_, SID_, WID_, etc.
            return False
        return True


    def getMacro (self):
        return self.macro
