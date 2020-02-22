#!/usr/bin/env python3
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

import sys

token_input_file = sys.argv[1]
gperf_output_file = sys.argv[2]

tokens = {}

for line in open(token_input_file):
    line = line.strip()
    if line:
        token = "XML_" + line
        token = token.replace("-", "_").replace(".", "_").replace(":", "_")
        token = token.replace("+", "PLUS")
        token = token.replace("-", "MINUS")
        tokens[line] = token.upper()

gperf = open(gperf_output_file, "w")

gperf.write("%language=C++\n" )
gperf.write("%global-table\n" )
gperf.write("%null-strings\n" )
gperf.write("%struct-type\n" )
gperf.write("struct xmltoken\n" )
gperf.write("{\n" )
gperf.write("  const sal_Char *name; XMLTokenEnum nToken; \n" )
gperf.write("};\n" )
gperf.write("%%\n" )

for token in sorted(tokens.keys()):
    gperf.write("{},{}\n".format(token, tokens[token]))

gperf.write("%%\n")
gperf.close()

# vim: set noet sw=4 ts=4:
