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

use warnings;
use strict 'vars';

my $my_lang = 'en-US';
my $plist = 'Info.plist';
my $lines = 0;

while ($_ = $ARGV[0], /^-/) {
  shift;
  last if /^--$/;
  if (/^-l/) {
    $my_lang = $ARGV[0];
    shift;
  } elsif (/^-p/) {
    $plist = $ARGV[0];
    shift;
  }
}

# open input file (Info.plist)
unless (open(SOURCE, $plist)) {
  print STDERR "Can't open $plist file: $!\n";
  return;
}

# XML::Parser not installed by default on MacOS X
my (%documents,$key,$icon,$name);

$name = "";

while (<SOURCE>) {
  if ( /<\/dict>/ ) {
    $documents{$icon} = $name if length $name > 0;
    $key = $icon = $name = "";
  } elsif ( /<key>(.*)<\/key>/ ) {
    $key = $1;
  } elsif ( /<string>(.*)<\/string>/ ) {
    if ( $key eq 'CFBundleTypeIconFile' ) {
      $icon = $1;
      $icon =~ s/\.icns$//;
    } elsif ( $key eq 'CFBundleTypeName' ) {
      $name = $1;
    }
  }
}

close (SOURCE);

print_lang($my_lang);
print_lang('en-US') unless $lines > 0;

sub print_lang
{
  my ($this_lang) = @_;

  # open input file (documents.ulf)
  unless (open(SOURCE, $ARGV[0])) {
    print STDERR "Can't open $ARGV[0] file: $!\n";
    return;
  }

  my $last_section;

  while (<SOURCE>) {

    if ( /\[(.*)\]/ ) {
      $last_section = $1;
    } else {
      # split locale = "value" into 2 strings
      my ($lang, $value) = split ' = ';

      if ( $lang ne $_ && $lang eq $this_lang && exists $documents{$last_section} ) {
        # replacing product variable doesn't work inside zip files and also not for UTF-16
        next if /%PRODUCTNAME/;
#        s/%PRODUCTNAME/\${FILEFORMATNAME} \${FILEFORMATVERSION}/g;
        s/$lang/"$documents{$last_section}"/;
        s/\n/;\n/;
        print;
        $lines += 1;
      }
    }
  }

  close (SOURCE);
}
