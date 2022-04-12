#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# wikilinks.py:
# This throwaway program can be used to convert idl_chapter_refs.txt to a
# "fake" IDL file that can be fed to doxygen to get the DevGuide Wiki links.
#
# Usage:
#
# In core, generate a file letting us know what kinds of entities to declare:
# cat <(make -s cmd cmd='$(INSTDIR)/sdk/bin/unoidl-read --summary $(INSTDIR)/program/types.rdb') <(make -s cmd cmd='$(INSTDIR)/sdk/bin/unoidl-read --summary $(INSTDIR)/program/types.rdb $(INSTDIR)/program/types/offapi.rdb') | LC_ALL=C sort | LC_ALL=C uniq > /tmp/kinds
#
# Run the script while feeding it the chapter references and output the fake IDL:
# python wikilinks.py < idl_chapter_refs.txt > generated_idl_chapter_refs.idl

import sys

devguidewww = "https://wiki.documentfoundation.org/"
in_topic = False
link = None
description = None
allthings = {}
allkinds = {}

for line in open("/tmp/kinds") :
    (kind,_,name) = line.strip().partition(" ")
    allkinds[name] = kind

for line in sys.stdin :
    sline = line.strip()
    if sline.startswith("LINK:") :
        link = sline.partition('LINK:')[2]
    elif sline.startswith("DESCR:") :
        description = sline.partition('DESCR:')[2]
    elif sline == "TOPIC:" :
        in_topic = True
    elif in_topic :
        if sline == "" :
            in_topic = False
        elif sline in allthings :
            allthings[sline].append((link, description))
        else:
            allthings[sline] = [(link, description)]

print("""/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* this file was generated from idl_chapter_refs.txt by wikilinks.py */""")

for key in allthings:
    kind = allkinds[key]
    parts = key.split(".")
    print("\n")
    for p in parts[0:-1] :
        print("module", p, "{")
    # for enums the "{}" trick results in broken/duplicate output
    if kind == "enum" :
        print("/// @" + kind, parts[-1])
    print("/// @par Developers Guide")
    for item in allthings[key] :
        print("///      <a href=\"" + devguidewww + item[0] + "\">"
                + item[1] + "</a><br>")
    # doxygen does not have tags for e.g. @service but empty definition works
    if kind != "enum" :
        print(kind, parts[-1], "{}")
    for p in parts[0:-1] :
        print("};")

# vim:set shiftwidth=4 softtabstop=4 expandtab:
