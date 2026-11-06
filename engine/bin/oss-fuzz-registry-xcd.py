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
# Writes one .xcd holding the schema and the data of each named configuration component, taken
# from the .xcd files of an instdir share/registry directory.
# Usage: bin/oss-fuzz-registry-xcd.py instdir/share/registry out.xcd org.openoffice.Office/Embedding

import glob
import os
import re
import sys

COMPONENT = re.compile(r'<oor:component-(schema|data) ([^>]*)>.*?</oor:component-\1>', re.S)

def main(registry_dir, out_path, components):
    wanted = {tuple(c.rsplit("/", 1)) for c in components}
    schema = {}
    data = []
    for xcd in sorted(glob.glob(os.path.join(registry_dir, "*.xcd"))):
        with open(xcd, encoding="utf-8") as f:
            text = f.read()
        for m in COMPONENT.finditer(text):
            package = re.search(r'oor:package="([^"]*)"', m.group(2))
            name = re.search(r'oor:name="([^"]*)"', m.group(2))
            key = (package.group(1), name.group(1)) if package and name else None
            if key not in wanted:
                continue
            if m.group(1) == "schema":
                schema[key] = m.group(0)
            else:
                data.append(m.group(0))
    missing = wanted - schema.keys()
    if missing:
        sys.exit("no schema in " + registry_dir + " for "
                 + ", ".join("/".join(k) for k in sorted(missing)))
    with open(out_path, "w", encoding="utf-8") as f:
        f.write('<?xml version="1.0"?>\n'
                '<oor:data xmlns:xs="http://www.w3.org/2001/XMLSchema"'
                ' xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'
                ' xmlns:oor="http://openoffice.org/2001/registry"'
                ' xmlns:install="http://openoffice.org/2004/installation">')
        f.write("".join(schema[k] for k in sorted(schema)))
        f.write("".join(data))
        f.write("</oor:data>\n")

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], sys.argv[3:])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
