: # -*- perl -*-
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
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
# create setup self extracting script

if( $#ARGV < 2 )
  {
    print <<ENDHELP;
USAGE: $0 <inputshellscript> <libraryfile> <outputshellscript>
    <inputshellscript>: the start shell script, located next to this perl script
    <libraryfile>: the library file, that is included into the shell script
    <outfile>: the target shellscript

ENDHELP
  exit;
  }

$infile     = $ARGV[0];
$library    = $ARGV[1];
$outfile    = $ARGV[2];

# read script header
open( SCRIPT, "<$infile" ) || die "cannot open $infile";
open( OUTFILE, ">$outfile$$.tmp" ) || die "cannot open $outfile";
@scriptlines = <SCRIPT>;
$linenum = $#scriptlines+2;
foreach (@scriptlines)
{
  # lineend conversion (be on the safe side)
  chomp;
  $_ =~ tr/\r//;
  s/^\s*linenum=.*$/linenum=$linenum/;
  print OUTFILE "$_\n";
}
close( SCRIPT );
close( OUTFILE );

system( "cat $library >>$outfile$$.tmp" );
rename "$outfile$$.tmp", "$outfile";

chmod 0775, $outfile;

exit;
