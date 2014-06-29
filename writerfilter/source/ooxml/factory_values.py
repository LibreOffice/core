#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from __future__ import print_function
import xml.sax
import string
import sys


class ContentHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        self.inValue = False
        self.defines = []
        self.chars = []

    def __escape(self, name):
        return name.replace('-', 'm').replace('+', 'p').replace(' ', '_').replace(',', '_')

    def startDocument(self):
        print('''
#ifndef INCLUDED_FACTORY_VALUES
#include <rtl/ustring.hxx>

#define OOXMLValueString_ ""''')

    def endDocument(self):
        print("""
#endif // INCLUDED_FACTORY_VALUES""")

    def startElement(self, name, attrs):
        if name == "value":
            self.inValue = True

    def endElement(self, name):
        if name == "value":
            self.inValue = False
            characters = "".join(self.chars)
            self.chars = []
            if len(characters):
                define = '#define OOXMLValueString_%s "%s"' % (self.__escape(characters), characters)
                if not define in self.defines:
                    self.defines.append(define)
                    print(define)

    def characters(self, chars):
        if self.inValue:
            self.chars.append(chars)

parser = xml.sax.make_parser()
parser.setContentHandler(ContentHandler())
parser.parse(sys.argv[1])

# vim:set shiftwidth=4 softtabstop=4 expandtab:
