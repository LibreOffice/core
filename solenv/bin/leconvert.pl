:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
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

