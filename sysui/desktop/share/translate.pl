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

my ($prefix, $ext, $key);
my $productname = "LibreOffice";
my $workdir = ".";

while ($_ = $ARGV[0], /^-/) {
    shift;
    last if /^--$/;
    if (/^-p/) {
        $productname = $ARGV[0];
        shift;
    }
    if (/^-d/) {
        $workdir = $ARGV[0];
        shift;
    }
    if (/^--key/) {
        $key = $ARGV[0];
        shift;
    }
    if (/^--prefix/) {
        $prefix = $ARGV[0];
        shift;
    }
    if (/^--ext/) {
        $ext = $ARGV[0];
        shift;
    }
}

# hack for unity section
my $outkey = $key;
if ( $outkey eq "UnityQuicklist" ) {
    $outkey = "Name";
}

my %templates;

# open input file
unless (open(SOURCE, $ARGV[0])) {
    print STDERR "Can't open $ARGV[0] file: $!\n";
    return;
}

# currently read template
my $template;

# read ulf file
while (<SOURCE>) {
    my $line = $_;

    if ( "[" eq substr($line, 0, 1) ) {
        $template = substr($line, 1, index($line,"]")-1);
        my %entry;
        # For every section in the specified ulf file there should exist
        # a template file in $workdir ..
        $entry{'outfile'} = "$workdir/$prefix$template.$ext";
        my %translations;
        $entry{'translations'} = \%translations;
        $templates{$template} = \%entry;
    } else {
        # split locale = "value" into 2 strings
        my ($locale, $value) = split(' = ', $line);

        if ( $locale ne $line ) {
            # replace en-US with en
            $locale=~s/en-US/en/;

            # use just anything inside the ""
            $value = substr($value, index($value, "\"") + 1, rindex($value, "\"") - 1);

            # replace resource placeholder
            $value=~s/%PRODUCTNAME/$productname/g;

            $locale=~s/-/_/;

            $templates{$template}->{'translations'}->{$locale} = $value;
        }
    }
}

close(SOURCE);

# process templates
foreach $template (keys %templates) {
    my $outfile = $templates{$template}->{'outfile'};
    print "processing template $template in $outfile\n";

    # open the template file - ignore sections for which no
    # templates exist
    unless(open(TEMPLATE, $outfile)) {
        print STDERR "Warning: No template found for item $_: $outfile: $!\n";
        next;
    }

    # open output file
    unless (open(OUTFILE, "> $outfile.tmp")) {
        print STDERR "Can't create output file $outfile.tmp: $!\n";
        exit -1;
    }

    # emit the template to the output file
    while (<TEMPLATE>) {
        my $keyline = $_;
        $keyline =~ s/^$key/$outkey/;
        print OUTFILE $keyline;
        if (/$key/) {
            my $translations = $templates{$template}->{'translations'};
            foreach my $locale (keys %{$translations}) {
                my $value = $translations->{$locale};
                print "locale is $locale\n";
                print "value is $value\n";
                if ($value) {
                    if ($ext eq "desktop") {
                        print OUTFILE "$outkey\[$locale\]=$value\n";
                    } else {
                        print OUTFILE "\t\[$locale\]$outkey=$value\n";
                    }
                }
            }
        }
    }

    close(TEMPLATE);

    if (close(OUTFILE)) {
        system "mv -f $outfile.tmp $outfile\n";
    }
}
