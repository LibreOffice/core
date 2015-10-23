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

#
# Translates multiple .desktop files at once with strings from .ulf
# files; if you add new translateable .ulf files please add them to
# l10ntools/source/localize.cxx
#

my ($prefix, $ext, $key);
my $productname = "LibreOffice";
my $workdir = ".";
my $template_dir;

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
    if (/^--template-dir/) {
        $template_dir = $ARGV[0];
        shift;
    }
}

if (!defined $template_dir) {
    $template_dir = "$workdir/$prefix";
}

# hack for unity section
my $outkey = $key;
if ( $outkey eq "UnityQuickList" ) {
    $outkey = "Name";
}

my %templates;

# open input file
unless (open(SOURCE, $ARGV[0])) {
    die "Can't open $ARGV[0] file: $!\n";
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
        $entry{'outfile'} = "$template_dir$template.$ext";
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

my $processed = 0;
# process templates
foreach $template (keys %templates) {
    my $outfile = $templates{$template}->{'outfile'};

    # open the template file - ignore sections for which no
    # templates exist
    if (open(TEMPLATE, $outfile)) {
       $processed++;
    } elsif ($ext eq 'str') { # string files processed one by one
       next;
    } else {
        die "Warning: No template found for item '$template' : '$outfile' : '$_': $!\n";
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
                # print "locale is $locale\n";
                # print "value is $value\n";
                if ($value) {
                    if ($ext eq "desktop" || $ext eq "str") {
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

if ($ext eq 'str' && $processed == 0) {
    die "Warning: No matching templates processed";
}
