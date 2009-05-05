:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: ulf2dat.pl,v $
# $Revision: 1.4.108.1 $
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
#***********************************************************************/


#### script id #####

( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

$id_str = ' $Revision: 1.4.108.1 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

#########################
#                       #
#   Globale Variablen   #
#                       #
#########################

use Encode;
use Encode::Alias;


my ($in_name, $ID, $new_ID);
my ($help);

print STDERR "$script_name -- Version: $script_rev\n" ;

&get_options;

if ( $help ) {
    &usage();
    exit 0;
};


open IN, "<$in_name" or die "Could not open $in_name for reading $! $^E";

foreach $lang ( keys %files ) 
{
    open "F_$lang",">$files{$lang}" or die "Could not open $files{$lang} for writing $! $^E";
    binmode "F_$lang";
    $files{$lang} = "F_$lang";
}

%transunit = ();

while ( <IN> )
{
    chomp;
    $line = $_;
    $line =~ s/\r$//;
    # [RID_RESXLS_COST_Print_Area]
    if ( $line =~ /^\[(.*)\]$/ )
    {
        $new_ID = $1;
        
        write_transunit();
        $ID = $new_ID;
        %transunit = ();
    }
    # de = "Druckbereich"
    elsif ( $line =~ /^(\S*)\s*=\s*\"(.*)\"$/ )
    {
        $lang = $1;
        $string = $2;
        $transunit{ $lang } = $string;
    }
    elsif ( $line !~ /^\s*$/ )
    {
        die "unknown lineformat in $in_name: $line\n";
    }
}
write_transunit();


sub write_transunit
{
    if ( ! $ID )
    {
        return;
    }
    foreach $lang ( keys %files )
    {
        my $string;
        if ( defined $transunit{ $lang } )
        {
            $string = $transunit{ $lang };
        }
        else
        {
            $string = $transunit{ "en-US" };
        }
        
        my $dat_line = "$ID=$string";
        Encode::from_to( $dat_line, "utf8", "UTF-16LE");
        print { $files{$lang} } "$dat_line\015\000\012\000";
    }       
}


sub get_options {
    my ($arg,$lang);

    while ($arg = shift @ARGV) {
        $arg =~ /^-i$/  and $in_name = shift @ARGV and next;
        $arg =~ /^-help$/  and $help = 1 and next; #show help
        
        $arg =~ /.*[\/\\]([^\/\\]*)\.dat$/;
#       $arg =~ /.*[/\]([^/\]*)\.dat$/;
        $lang = $1;
        print "got $lang = $arg\n";
        $files{ $lang } = $arg;
    }
}



sub usage {
    print STDERR "\n\n";
    print STDERR "Syntax:   $script_name [-help|-i <ulf-filename>] <dat-filename> ... \n";
    print STDERR "Example:  $script_name -i strings.ulf en-US.dat de.dat\n";
    print STDERR "Options:\n\n";
    print STDERR "    -i      input ulf file\n";
    print STDERR "    -help   print this help info\n\n";
};

