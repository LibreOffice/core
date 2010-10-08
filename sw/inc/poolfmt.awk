#*************************************************************************
#*
#*    $Workfile:        dbgxtor.awk
#*
#*    Ersterstellung    JP  05.10.95
#*    Letzte Aenderung  $Author: hr $ $Date: 2000-09-18 17:14:27 $
#*    $Revision: 1.1.1.1 $
#*
#*    $Logfile:   T:/sw/inc/poolfmt.awv  $
#*
#*    Copyright (c) 1990-1996, STAR DIVISION
#*
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

