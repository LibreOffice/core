#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This script inserts the 3rd party license text of code coming
# from the online repo to the main license text that is generated
# by building LOKit core. The empty 'Extensions' section is removed.

import sys
from copy import deepcopy
from lxml import etree

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <LICENSE.html> <CODA_THIRDPARTYLICENSES.html>")
        sys.exit(1)

    license_path = sys.argv[1]
    snippet_path = sys.argv[2]

    parser = etree.XMLParser(remove_blank_text=False)

    # --- parse main LICENSE.html ---
    tree = etree.parse(license_path, parser)
    root = tree.getroot()

    # Detect default XHTML namespace (if any)
    default_ns = root.nsmap.get(None)
    if default_ns:
        ns = {"x": default_ns}
        h1_xpath = ".//x:h1[x:a[@id='a__Extensions']]"
        body_children_xpath = ".//x:body/*"
    else:
        ns = None
        h1_xpath = ".//h1[a[@id='a__Extensions']]"
        body_children_xpath = ".//body/*"

    # --- find the <h1><a id="a__Extensions">…</a></h1> ---
    h1_list = root.xpath(h1_xpath, namespaces=ns)
    if not h1_list:
        print("Error: Could not find <h1><a id='a__Extensions'>…</a></h1> in LICENSE.html")
        sys.exit(2)

    h1 = h1_list[0]

    # The <p> must be immediately after the <h1>
    p = h1.getnext()
    # Use localname so namespace won't matter
    def localname(elem):
        if elem is None:
            return None
        # elem.tag may look like '{ns}p'
        return etree.QName(elem.tag).localname

    if localname(p) != "p":
        print("Error: Expected a <p> immediately after the Extensions <h1>.")
        sys.exit(3)

    parent = h1.getparent()
    index = parent.index(h1)

    # Remove old heading + paragraph
    parent.remove(h1)
    parent.remove(p)

    # --- parse the snippet file ---
    snippet_tree = etree.parse(snippet_path, parser)
    snippet_root = snippet_tree.getroot()

    # Try to get children of <body> from the snippet
    if default_ns:
        snippet_ns = {"x": snippet_root.nsmap.get(None, default_ns)}
        snippet_elements = snippet_root.xpath(body_children_xpath, namespaces=snippet_ns)
    else:
        snippet_elements = snippet_root.xpath(".//body/*")

    # Fallback: if no <body>, treat the root's children as the snippet
    if not snippet_elements:
        snippet_elements = list(snippet_root)

    # Insert deep copies of snippet nodes at the original position
    for elem in snippet_elements:
        parent.insert(index, deepcopy(elem))
        index += 1

    # --- write back LICENSE.html ---
    tree.write(license_path, encoding="utf-8", pretty_print=True)

if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
