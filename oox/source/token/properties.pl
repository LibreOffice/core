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

# parse input file

open( INFILE, $ARGV0 ) or die "Error: cannot open input file: $!";
my %props;
while( <INFILE> )
{
    # trim newline
    chomp( $_ );
    # trim leading/trailing whitespace
    $_ =~ s/^\s*//g;
    $_ =~ s/\s*$//g;
    # check for valid characters
    $_ =~ /^[A-Z][a-zA-Z0-9]*$/ or die "Error: invalid character in property '$_'";
    $id = "PROP_$_";
    $props{$_} = $id;
}
close( INFILE );

# generate output files

open( IDFILE, ">$ARGV1" ) or die "Error: cannot open output file: $!";
open( NAMEFILE, ">$ARGV2" ) or die "Error: cannot open output file: $!";

$i = 0;
foreach( sort( keys( %props ) ) )
{
    print( IDFILE "const sal_Int32 $props{$_} = $i;\n" );
    print( NAMEFILE "/* $i */ \"$_\",\n" );
    ++$i;
}

print( IDFILE "const sal_Int32 PROP_COUNT = $i;\n" );
print( IDFILE "const sal_Int32 PROP_INVALID = -1;\n" );
print( IDFILE "const sal_Int32 PROP_END_LIST = -2;\n" );

close( IDFILE );
close( NAMEFILE );
