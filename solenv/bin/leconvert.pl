:
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
my $target_format = "";
my @filelist;
#my $debug=1;
my $debug=0;

parameter_parse(@ARGV);
print "@filelist\n" if ( $debug );
foreach my $onefile ( @filelist ) {
    convert_file( $onefile );
}


sub convert_file
{
    my $filename = shift;
    if ( $target_format eq "dos" ) {
        $lineend = "\r\n";
    } else {
        $lineend = "\n";
    }
    open INFILE, "$filename"or die "ERROR: Couldn\'t open $filename for reading.\n";
    my @lines = <INFILE>;
    close INFILE;

    foreach my $oneline ( @lines ) {
        $oneline =~ s/\r*\n*$/$lineend/;
    }

    open OUTFILE, ">$filename" or die "ERROR: Couldn\'t open $filename for writing.\n";
    syswrite OUTFILE, join "", @lines;
    close OUTFILE;

}

sub parameter_parse
{
    if ( $target_format eq "" ) {
        $target_format = shift ;
        usage() if ( $target_format ne "unix" && $target_format ne "dos" );
        usage() if ( $#_ == -1 );
    }
    foreach my $param ( @_ ) {
        if ( $param =~ "^@" ) {
            my $filename = $param;
            $filename =~ s/^@//;
            open CMDFILE, "$filename" or die "ERROR: Couldn\'t open $filename for reading.\n";
            my @filelist = <CMDFILE>;
            close CMDFILE;
            parameter_parse( @filelist );
        } else {
            push @filelist, $param;
        }
    }
}

sub usage
{
    print "Convert text files to the desired lineend convention:\n";
    print "$0 <unix|dos> <FILE|\@filelist> [more files/lists]\n";
    exit 1;
}

