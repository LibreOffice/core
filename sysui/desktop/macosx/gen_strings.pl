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
        s/$lang/"$documents{$last_section}"/;
        s/\n/;\n/;
        print;
        $lines += 1;
      }
    }
  }

  close (SOURCE);
}
