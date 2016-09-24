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

$ARGV0 = shift @ARGV;
$ARGV1 = shift @ARGV;
$ARGV2 = shift @ARGV;
$ARGV3 = shift @ARGV;

open ( IDFILE, ">$ARGV1" ) or die "Error: cannot open output file: $!";
open ( NAMEFILE, ">$ARGV2" ) or die "Error: cannot open output file: $!";
open ( GPERFFILE, ">$ARGV3" ) or die "Error: cannot open output file: $!";

print( GPERFFILE "%language=C++\n" );
print( GPERFFILE "%global-table\n" );
print( GPERFFILE "%null-strings\n" );
print( GPERFFILE "%struct-type\n" );
print( GPERFFILE "struct xmltoken {\n" );
print( GPERFFILE "    const sal_Char *name;\n" );
print( GPERFFILE "    sal_Int32 nToken;\n" );
print( GPERFFILE "};\n" );
print( GPERFFILE "%%\n" );

open( INFILE, $ARGV0 ) or die "Error: cannot open input file: $!";

$i = 0;
while ( <INFILE> )
{
    # trim newline
    chomp( $_ );
    # trim leading/trailing whitespace
    $_ =~ s/^\s*//g;
    $_ =~ s/\s*$//g;
    # check for valid characters
    $_ =~ /^[a-zA-Z0-9-_]+$/ or die "Error: invalid character in token '$_'";
    $id = "XML_$_";
    $id =~ s/-/_/g;
    $tokens{$_} = $id;
    print( IDFILE "const sal_Int32 $id = $i;\n" );
    print( NAMEFILE "\"$_\",\n" );
    print( GPERFFILE "$_,$id\n" );
    ++$i;
}
close ( INFILE );

print( IDFILE "const sal_Int32 XML_TOKEN_COUNT = $i;\n" );
print( GPERFFILE "%%\n" );

close( IDFILE );
close( NAMEFILE );
close( GPERFFILE );
