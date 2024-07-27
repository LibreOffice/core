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
txt_out_name = sys.argv[4]
instrict_name = sys.argv[5]
strict_out_name = sys.argv[6]

# parse input file

namespaces = {}

with open(infile_name) as infile:
    for line in infile:
        line = line.strip()
        # trim comments
        line = line.split('#')[0]
        # skip empty lines
        if line:
            # check for valid characters
            m = re.match(r'([a-zA-Z][a-zA-Z0-9]*)\s+([a-zA-Z0-9-.:\/]+)\s*$', line)
            if not m:
                sys.exit('Invalid character in input data: ' + line)
            namespaces[m.group(1)] = m.group(2)

# OOXML strict namespaces

namespaces_strict = {}
with open(instrict_name) as infile_strict:
    for line in infile_strict:
        line = line.strip()
        # trim comments
        line = line.split('#')[0]
        # skip empty lines
        if line:
            # check for valid characters
            m = re.match(r'([a-zA-Z][a-zA-Z0-9]*)\s+([a-zA-Z0-9-.:\/]+)\s*$', line)
            if not m:
                sys.exit("Error: invalid character in input data: " + line)
            namespaces_strict[m.group(1)] = m.group(2)

# generate output files

idfile = open(id_out_name, 'w')
namefile = open(name_out_name, 'w')
txtfile = open(txt_out_name, 'w')
namefile_strict = open(strict_out_name, 'w')

# number of bits to shift the namespace identifier
shift = 16

idfile.write("const size_t NMSP_SHIFT = {};\n".format(shift))

i = 1
for token in sorted(namespaces.keys()):
    idfile.write("const sal_Int32 NMSP_{} = {} << NMSP_SHIFT;\n".format(token, i))
    cur_id = i << shift
    namefile.write("{{ {}, \"{}\" }},\n".format(cur_id, namespaces[token]))
    namefile_strict.write("{{ {}, \"{}\" }},\n".format(cur_id, namespaces_strict[token]))
    txtfile.write("{} {} {}\n".format(cur_id, token, namespaces[token]))
    txtfile.write("{} {} {}\n".format(cur_id, token, namespaces_strict[token]))
    i += 1

idfile.close()
namefile.close()
namefile_strict.close()
txtfile.close()
