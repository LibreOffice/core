#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

#
# Dieses awk-script generiert ein cxx-file, das alle PoolIds der Vorlage dumpt.
# wird fuer den HelpPI gebraucht.
# Aufruf:  awk -f poolid.awk poolfmt.hxx > poolid.cxx
#          cl poolid.cxx
#          poolid.exe > ???.hrc
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
    print  "    int nSize = (sizeof(ppPoolIds) / sizeof(PoolFmtIds)) - 1;"
    print  "    for( int n = 0; n < nSize; n++ )"
    print  "        printf( \"" sStr " %s\\t%8d\\n\", ppPoolIds[ n ].pStr, ppPoolIds[ n ].nId );"
    print  "}"
}

function TableHead() {
	print
    print "struct PoolFmtIds { int nId; const char* pStr; };"
    print "static PoolFmtIds ppPoolIds[] = {"
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

