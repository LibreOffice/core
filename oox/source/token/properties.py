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
import re

infile_name = sys.argv[1]
id_out_name = sys.argv[2]
name_out_name = sys.argv[3]

# parse input file

props = {}
with open(infile_name) as infile:
    for line in infile:
        line = line.strip()
        # check for valid characters
        if not re.match(r'[A-Z][a-zA-Z0-9]*$', line):
            sys.exit("Error: invalid character in property '{}'".format(line))
        props[line] = "PROP_" + line

# generate output files

idfile = open(id_out_name, 'w')
namefile = open(name_out_name, 'w')

i = 0
for token in sorted(props.keys()):
    idfile.write("const sal_Int32 {}  = {};\n".format(props[token], i))
    namefile.write("/* {} */ \"{}\",\n".format(i, token))
    i += 1

idfile.write("const sal_Int32 PROP_COUNT = {};\n".format(i))
idfile.write("const sal_Int32 PROP_INVALID = -1;\n" )

idfile.close()
namefile.close()
