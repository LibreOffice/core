#!/usr/bin/env python
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

localeNames = {'fr': 'French', 'hu': 'Hungarian', 'de': 'German'}
def getLocaleName (code):
    global localeNames
    if code in localeNames:
        return localeNames[code]
    else:
        return "(unknown locale)"

def getAscii (ords):
    ascii = ''
    for c in ords:
        ascii += chr(c)
    return ascii

class LocaleData(object):
    def __init__ (self, locale):
        self.locale = locale
        self.funcList = {}

    def addKeywordMap (self, funcName, localeName, engName):
        if funcName not in self.funcList:
            self.funcList[funcName] = []

        self.funcList[funcName].append([localeName, engName])

    def getLocaleFuncVarName (self, func, pair):
        return func.lower() + "_" + getAscii(pair[1]).lower() + "_" + self.locale

    def dumpCode (self):
        chars = ""

        # locale output
        chars += "// " + "-"*75 + "\n"
        chars += "// %s language locale (automatically generated)\n"%getLocaleName(self.locale)
        chars += "// " + "-"*75 + "\n"
        chars += "static const Locale a" + self.locale.capitalize() + "(OUString(\""
        chars += self.locale
        chars += "\"), OUString(), OUString());\n\n"

        # pre instantiations of localized function names.
        funcs = sorted(self.funcList.keys())
        chars += "// pre instantiations of localized function names\n"
        for func in funcs:
            for item in self.funcList[func]:
                chars += "static const sal_Unicode " + self.getLocaleFuncVarName(func, item) + "[] = {\n"
                chars += "    "
                isFirst = True
                # Dump the UTF-16 bytes.
                for uval in item[0]:
                    if isFirst:
                        isFirst = False
                    else:
                        chars += ", "
                    chars += "0x%.4X"%uval

                # Don't forget to null-terminate the string.
                if not isFirst:
                    chars += ", "
                chars += "0x0000"

                chars += "};\n"

        # map item instantiations
        chars += "\n"
        chars += "static const TransItem p" + self.locale.capitalize() + "[] = {\n"
        for func in funcs:
            for item in self.funcList[func]:
                chars += "    "
                chars += "{%s, \"%s\", %s},\n"%(self.getLocaleFuncVarName(func, item),
                                                getAscii(item[1]),
                                                "oc"+func.capitalize())

        chars += "    {NULL, NULL, ocNone}\n"
        chars += "};\n\n"

        # addToMap call
        chars += "addToMap(%s, %s);\n"%(
            "p"+self.locale.capitalize(), "a"+self.locale.capitalize())

        return chars

class Parser(object):

    def __init__ (self, args):
        # default input & output files.
        self.infile = "./keywords_utf16.txt"
        self.outfile = "../../source/core/tool/cellkeywords.inl"

        if len(args) >= 2:
            self.infile = args[1]
        if len(args) >= 3:
            self.outfile = args[2]

    def getDByte (self):
        # Assume little endian.
        bh = self.bytes[self.i]
        bl = self.bytes[self.i+1]
        try:
            dbyte = ord(bl)*256 + ord(bh)
        except Exception:
            dbyte = bl*256 + bh
        self.i += 2
        return dbyte

    def parseLine (self):
        buf = []
        while self.i < self.size:
            dbyte = self.getDByte()
            if dbyte == 0x000A:
                break
            buf.append(dbyte)
        return buf

    def dumpBuf (self, buf, linefeed=True):
        for item in buf:
            sys.stdout.write(chr(item))
        if linefeed:
            print ('')

    def parse (self):

        file = open(self.infile, 'rb')
        self.bytes = file.read()
        file.close()

        self.size = len(self.bytes)
        self.i = 0

        localeList = []  # stores an array of locale data objects.
        funcName = None
        word = []
        wordPair = []

        while self.i < self.size:
            dbyte = self.getDByte()
            if dbyte == 0xFEFF and self.i == 2:
                # unicode signature - ignore it.
                pass
            elif dbyte == 0x0024:
                # $ - locale name
                buf = self.parseLine()
                locale = getAscii(buf)
                localeList.append(LocaleData(locale))

            elif dbyte == 0x0040:
                # @ - function name
                buf = self.parseLine()
                funcName = getAscii(buf)

            elif dbyte == 0x002C:
                # , - comma separator
                if len(word) > 0:
                    wordPair.append(word)
                    word = []
            elif dbyte == 0x000A:
                # linefeed
                if len(word) > 0:
                    wordPair.append(word)
                    word = []
                if len(wordPair) >= 2:
                    localeList[-1].addKeywordMap(funcName, wordPair[0], wordPair[1])
                wordPair = []
            elif dbyte in [0x0009, 0x0020]:
                # whitespace - ignore it.
                pass
            else:
                word.append(dbyte)

        chars = "// This file has been automatically generated.  Do not hand-edit this!\n"
        for obj in localeList:
            chars += "\n" + obj.dumpCode()
    
        # Write to output file.
        file = open(self.outfile, 'w')
        file.write(chars)
        file.close()

if __name__=='__main__':
    parser = Parser(sys.argv)
    parser.parse()

