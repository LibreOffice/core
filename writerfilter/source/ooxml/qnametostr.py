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
        self.tokens = []

    def startDocument(self):
        print("""
#include "ooxml/resourceids.hxx"
#include "ooxml/QNameToString.hxx"

namespace writerfilter
{

void QNameToString::init()
{
#ifdef DBG_UTIL
    /* ooxml */
""")

    def endDocument(self):
        print("""#endif
}

}
""")

    def startElement(self, name, attrs):
        for k, v in list(attrs.items()):
            if k == "tokenid":
                if v.startswith("ooxml:"):
                    token = v.replace('ooxml:', '')
                    if token not in self.tokens:
                        print("""    mMap[NS_ooxml::LN_%s] = "ooxml:%s";""" % (token, token))
                        self.tokens.append(token)


parser = xml.sax.make_parser()
parser.setContentHandler(ContentHandler())
parser.parse(sys.argv[1])

# vim:set shiftwidth=4 softtabstop=4 expandtab:
