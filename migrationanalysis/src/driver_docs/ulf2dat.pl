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

