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
        self.tokens = {}

    def startDocument(self):
        print("""
#ifndef INCLUDED_OOXML_NAMESPACESIDS_HXX
#define INCLUDED_OOXML_NAMESPACESIDS_HXX

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <string>

#include <resourcemodel/WW8ResourceModel.hxx>

namespace writerfilter {
namespace ooxml {
using namespace ::std;
using namespace ::com::sun::star;
""")

    def endDocument(self):
        for alias in sorted(self.tokens.keys()):
            print(self.tokens[alias])
        print("""
}}
#endif //INCLUDED_OOXML_NAMESPACESIDS_HXX""")

    def startElement(self, name, attrs):
        if name == "namespace-alias":
            token = """const sal_Int32 NMSP_%s = %s;""" % (attrs["alias"], attrs["id"])
            if token not in self.tokens:
                self.tokens[attrs["alias"]] = token

parser = xml.sax.make_parser()
parser.setContentHandler(ContentHandler())
parser.parse(sys.argv[1])

# vim:set shiftwidth=4 softtabstop=4 expandtab:
