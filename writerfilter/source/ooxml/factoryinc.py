#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from xml.dom import minidom
import sys


def createInclude(model):
    print("""
#ifndef INCLUDED_OOXML_FACTORY_GENERATED_HXX
#define INCLUDED_OOXML_FACTORY_GENERATED_HXX

namespace writerfilter {
namespace ooxml {

/// @cond GENERATED
    """)

    # Create namespaces.
    counter = 1
    for namespace in sorted([ns.getAttribute("name") for ns in model.getElementsByTagName("namespace")]):
        print("const Id NN_%s = %s << 16;" % (namespace.replace('-', '_'), counter))
        counter += 1

    # Create defines.
    counter = 1
    defines = []
    for define in sorted([ns.getAttribute("name") for ns in model.getElementsByTagName("define")]):
        if define not in defines:
            print("const Id DEFINE_%s = %s;" % (define, counter))
            defines.append(define)
        counter += 1
    print("""/// @endcond
}}

#endif // INCLUDED_OOXML_FACTORY_GENERATED_HXX""")


modelPath = sys.argv[1]
model = minidom.parse(modelPath)
createInclude(model)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
