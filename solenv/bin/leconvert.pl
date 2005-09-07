:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: leconvert.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:09:53 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

