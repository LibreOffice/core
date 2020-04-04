#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import xml.sax
import sys


class ContentHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        self.tokens = []
        self.counter = 90001

    def startDocument(self):
        print("""
/*

    THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT!

*/


#ifndef INCLUDED_OOXML_RESOURCEIDS_HXX
#define INCLUDED_OOXML_RESOURCEIDS_HXX

#include <dmapper/resourcemodel.hxx>

namespace writerfilter {

namespace NS_ooxml
{""")

    def endDocument(self):
        print("""}


}
#endif // INCLUDED_OOXML_RESOURCEIDS_HXX""")

    def startElement(self, name, attrs):
        for k, v in attrs.items():
            if k in ("tokenid", "sendtokenid"):
                if v.startswith("ooxml:"):
                    token = v.replace('ooxml:', '')
                    if token not in self.tokens:
                        print("    const Id LN_%s = %s;" % (token, self.counter))
                        self.tokens.append(token)
                self.counter += 1


parser = xml.sax.make_parser()
parser.setContentHandler(ContentHandler())
parser.parse(sys.argv[1])

# vim:set shiftwidth=4 softtabstop=4 expandtab:
