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
        self.counter = 0
        self.chars = []

    def startElement(self, name, attrs):
        if name == "fasttoken":
            self.inFasttoken = True

    def endElement(self, name):
        if name == "fasttoken":
            chars = "".join(self.chars)
            print("const Token_t OOXML_%s = %s;" % (chars.replace('-', '_'), self.counter))
            self.chars = []
            self.counter += 1
            self.inFasttoken = False

    def characters(self, characters):
        if self.inFasttoken:
            self.chars.append(characters)

    def endDocument(self):
        print("const Token_t OOXML_FAST_TOKENS_END = %s;" % self.counter)


print("""
/*

    THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT!

*/


#ifndef INCLUDED_OOXML_FAST_TOKENS_HXX
#define INCLUDED_OOXML_FAST_TOKENS_HXX

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <string>

#include <resourcemodel/WW8ResourceModel.hxx>

typedef sal_Int32 Token_t;
    """)

parser = xml.sax.make_parser()
parser.setContentHandler(ContentHandler())
parser.parse(sys.argv[1])

print("""namespace writerfilter {
namespace ooxml {

#ifdef DEBUG_DOMAINMAPPER
string fastTokenToId(sal_uInt32 nToken);
#endif

}}

static const sal_uInt32 F_Attribute = 1U << 31;

#endif //INCLUDED_OOXML_FAST_TOKENS_HXX""")

# vim:set shiftwidth=4 softtabstop=4 expandtab:
