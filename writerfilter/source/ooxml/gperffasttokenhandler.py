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
import sys


class ContentHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        self.inFasttoken = False
        self.chars = []

    def startElement(self, name, attrs):
        if name == "fasttoken":
            self.inFasttoken = True

    def endElement(self, name):
        if name == "fasttoken":
            chars = "".join(self.chars)
            token = chars.replace('-', '_')
            print("%s, oox::XML_%s" % (token, token))
            self.chars = []
            self.inFasttoken = False

    def characters(self, characters):
        if self.inFasttoken:
            self.chars.append(characters)

print("""
%{
#include "oox/token/tokens.hxx"

namespace writerfilter { namespace ooxml { namespace tokenmap {
%}
struct token { const char * name; Token_t nToken; };
%%""")

parser = xml.sax.make_parser()
parser.setContentHandler(ContentHandler())
parser.parse(sys.argv[1])

print("""FAST_TOKENS_END, oox::XML_TOKEN_COUNT
%%

}}}""")

# vim:set shiftwidth=4 softtabstop=4 expandtab:
