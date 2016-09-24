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
# This awk-script generates a cxx-file, that dumps all PoolIDs of the template.
# It is needed for HelpPI.
# Usage:  awk -f poolid.awk poolfmt.hxx > poolid.cxx
#         cl poolid.cxx
#         poolid.exe > ???.hrc
#

function Header() {
    print "// This is an outputfile of an awk-script: $Workfile:   poolfmt.awk  $"
    print "#include <solar.h> "
    print
    print  "#include <stdio.h>"
    print  "#include <stdlib.h>"
    print
    print  "#pragma hdrstop"
    print
    print "#include <iostream.hxx> "
    print "#include \"poolfmt.hxx\""
    print
}

function Main() {
	print
    print  "void main( int , char *[] ) {"
    sStr = "#define"
    print  "    int nSize = (sizeof(ppPoolIds) / sizeof(PoolFormatIds)) - 1;"
    print  "    for( int n = 0; n < nSize; n++ )"
    print  "        printf( \"" sStr " %s\\t%8d\\n\", ppPoolIds[ n ].pStr, ppPoolIds[ n ].nId );"
    print  "}"
}

function TableHead() {
	print
    print "struct PoolFormatIds { int nId; const char* pStr; };"
    print "static PoolFormatIds ppPoolIds[] = {"
}

function TableTail() {
    print  " 0, \"\" };"
    print
}

BEGIN {
    Header();
    TableHead();
}

/^[ \t]*RES_/ && !index( $1, "_BEGIN" ) && !index( $1, "_END" ) && !index( $1, "_POOL_" ) {
    sStr = $1;
	split( $1, sStr, "," );
    print  "    " sStr[1] ", \"" sStr[1] "\","
}

END {
    TableTail();
    Main();
}

