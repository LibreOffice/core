#!/usr/bin/env python
# *************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
# *************************************************************


import srclexer, srcparser, globals

class TestCase:

    @staticmethod
    def run (tokens, defines):
        mcExpander = srcparser.MacroExpander(tokens, defines)
        mcExpander.debug = True
        mcExpander.expand()
        tokens = mcExpander.getTokens()
        print tokens

    @staticmethod
    def simpleNoArgs ():
        tokens = ['FUNC_FOO', '(', 'left', ',', 'right', ')']
        defines = {}
        macro = globals.Macro('FUNC_FOO')
        macro.tokens = ['Here', 'comes', 'X', 'and', 'Y']
        defines['FUNC_FOO'] = macro
        TestCase.run(tokens, defines)

    @staticmethod
    def simpleArgs ():
        tokens = ['FUNC_FOO', '(', 'left', ',', 'right', ')']
        defines = {}
        macro = globals.Macro('FUNC_FOO')
        macro.tokens = ['Here', 'comes', 'X', 'and', 'Y']
        macro.vars['X'] = 0
        macro.vars['Y'] = 1
        defines['FUNC_FOO'] = macro
        TestCase.run(tokens, defines)

    @staticmethod
    def multiTokenArgs ():
        tokens = ['FUNC_FOO', '(', 'left1', 'left2', 'left3', ',', 'right', ')']
        defines = {}
        macro = globals.Macro('FUNC_FOO')
        macro.tokens = ['Here', 'comes', 'X', 'and', 'Y']
        macro.vars['X'] = 0
        macro.vars['Y'] = 1
        defines['FUNC_FOO'] = macro
        TestCase.run(tokens, defines)

    @staticmethod
    def nestedTokenArgs ():
        tokens = ['FUNC_BAA', '(', 'left', ',', 'right', ')']
        defines = {}
        macro = globals.Macro('FUNC_FOO')
        macro.tokens = ['Here', 'comes', 'X', 'and', 'Y']
        macro.vars['X'] = 0
        macro.vars['Y'] = 1
        defines['FUNC_FOO'] = macro
        macro = globals.Macro('FUNC_BAA')
        macro.tokens = ['FUNC_FOO']
        defines['FUNC_BAA'] = macro
        TestCase.run(tokens, defines)

def main ():
    print "simple expansion with no arguments"
    TestCase.simpleNoArgs()
    print "simple argument expansion"
    TestCase.simpleArgs()
    print "multi-token argument expansion"
    TestCase.multiTokenArgs()
    print "nested argument expansion"
    TestCase.nestedTokenArgs()

if __name__ ==  '__main__':
    main()
