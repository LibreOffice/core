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

open ( TOKENS, $ARGV0 ) || die "can't open token file: $!";
my %tokens;

while ( defined ($line = <TOKENS>) )
{
    if( !($line =~ /^#/) )
    {
        chomp($line);
        @token = split(/\s+/,$line);
        if ( not defined ($token[1]) )
        {
            $token[1] = "XML_".$token[0];
            $token[1] =~ tr/\-\.\:/___/;
            $token[1] =~ s/\+/PLUS/g;
            $token[1] =~ s/\-/MINUS/g;
        }

        $tokens{$token[0]} = uc($token[1]);
    }
}
close ( TOKENS );

open ( GPERF, ">$ARGV1" ) || die "can't open tokens.gperf file: $!";

print ( GPERF "%language=C++\n" );
print ( GPERF "%global-table\n" );
print ( GPERF "%null-strings\n" );
print ( GPERF "%struct-type\n" );
print ( GPERF "struct xmltoken\n" );
print ( GPERF "{\n" );
print ( GPERF "  const sal_Char *name; XMLTokenEnum nToken; \n" );
print ( GPERF "};\n" );
print ( GPERF "%%\n" );

$i = 0;
foreach( sort(keys(%tokens)) )
{
    $i = $i + 1;
    print( GPERF "$_,$tokens{$_}\n" );
}
print ( GPERF "%%\n" );
close ( GPERF );

# vim: set noet sw=4 ts=4:
