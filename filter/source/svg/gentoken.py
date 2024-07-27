# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys

tokenfile_name = sys.argv[1]
hxx_name = sys.argv[2]
gperf_name = sys.argv[3]

gperf_header = r"""%language=C++
%global-table
%null-strings
%struct-type
struct xmltoken
{
  const char *name; sal_Int32 nToken;
}
%%
"""

tokens = {}

with open(tokenfile_name) as tokenfile:
   for line in tokenfile:
       line = line.strip()
       if line:
           arr = line.split()
           if len(arr) < 2:
               t = "XML_" + arr[0]
               t = t.replace('-', '_').replace('.', '_').replace(':', '_')
               t = t.replace('+', 'PLUS')
               arr.append(t)
           tokens[arr[0]] = arr[1].upper()

hxx = open(hxx_name, 'w')
gperf = open(gperf_name, 'w', newline='\n')

gperf.write(gperf_header)

hxx.write("#ifndef INCLUDED_AUTOGEN_TOKEN_HXX\n")
hxx.write("#define INCLUDED_AUTOGEN_TOKEN_HXX\n\n")
hxx.write("#include <sal/types.h>\n\n" )

i = 0
for token in sorted(tokens.keys()):
    i += 1
    hxx.write("const sal_Int32 {} = {};\n".format(tokens[token], i))
    gperf.write("{},{}\n".format(token, tokens[token]))

gperf.write("%%\n")
hxx.write("const sal_Int32 XML_TOKEN_COUNT = {};\n".format(i))
hxx.write("const sal_Int32 XML_TOKEN_INVALID = -1;\n\n")
hxx.write("#endif\n")

hxx.close()
gperf.close()
