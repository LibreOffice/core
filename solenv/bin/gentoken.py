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

gperf_header = """%language=C++
%global-table
%null-strings
%struct-type
struct xmltoken
{
  const char *name; XMLTokenEnum nToken;
};
%%
"""

token_input_file = sys.argv[1]
gperf_output_file = sys.argv[2]

tokens = {}

with open(token_input_file) as ifile:
    for line in ifile:
        line = line.strip()
        if line:
            token = "XML_" + line
            token = token.replace("-", "_").replace(".", "_").replace(":", "_")
            token = token.replace("+", "PLUS")
            tokens[line] = token.upper()

with open(gperf_output_file, "wb") as gperf:
    gperf.write(gperf_header.encode("utf-8"))

    for token in sorted(tokens.keys()):
        gperf.write("{},{}\n".format(token, tokens[token]).encode("utf-8"))

    gperf.write("%%\n".encode("utf-8"))

# vim: set noet sw=4 ts=4:
