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

import sys, re

infile_name = sys.argv[1]
idfile_out_name = sys.argv[2]
namefile_out_name = sys.argv[3]
gperffile_out_name = sys.argv[4]

idfile = open(idfile_out_name, 'w')
namefile = open(namefile_out_name, 'w')
gperffile = open(gperffile_out_name, 'w')

gperffile.write("""%language=C++
%global-table
%null-strings
%struct-type
struct xmltoken {
    const char *name;
    sal_Int32 nToken;
};
%%
""")

token_count = 0;
tokens = {}

with open(infile_name) as infile:
    for line in infile:
        line = line.strip()
        # check for valid characters
        if not re.match(r'[a-zA-Z0-9-_]+$', line):
            sys.exit("Error: invalid character in token '{}'".format(line));
        cur_id = "XML_" + line;
        # we have two ids with similar names("cut-offs" and "cut_offs")
        if cur_id == "XML_cut_offs":
            cur_id = "cut_offs2";
        cur_id = cur_id.replace('-', '_')
        tokens[line] = cur_id
        idfile.write("const sal_Int32 {} = {};\n".format(cur_id, token_count))
        namefile.write("\"{}\",\n".format(line));
        gperffile.write("{},{}\n".format(line, cur_id));
        token_count += 1

idfile.write("const sal_Int32 XML_TOKEN_COUNT = {};\n".format(token_count))
gperffile.write("%%\n")

idfile.close()
namefile.close()
gperffile.close()

def fix_linefeeds(fname):
    # Gperf requires LF newlines, not CRLF, even on Windows.
    # Making this work on both Python 2 and 3 is difficult.
    # When Python 2 is dropped, delete this and add
    # newline = '\n' to the open() calls above.
    with open(fname, 'rb') as ifile:
        d = ifile.read()
    d = d.replace(b'\r', b'')
    with open(fname, 'wb') as ofile:
        ofile.write(d)

fix_linefeeds(idfile_out_name)
fix_linefeeds(namefile_out_name)
fix_linefeeds(gperffile_out_name)
