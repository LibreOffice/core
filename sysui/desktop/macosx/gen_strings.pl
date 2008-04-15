:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: gen_strings.pl,v $
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

use warnings;
use strict 'vars';

my $my_lang = 'en-US';
my $plist = 'Info.plist';
my $outfile;

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

# open input file (Info.plist)
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

    if ( $lang ne $_ && $lang eq $my_lang && exists $documents{$last_section} ) {
      # replacing product variable doesn't work inside zip files and also not for UTF-16
      next if /%PRODUCTNAME/;
#      s/%PRODUCTNAME/\${FILEFORMATNAME} \${FILEFORMATVERSION}/g;
      s/$lang/"$documents{$last_section}"/;
      s/\n/;\n/;
      print;
    }
  }
}

close (SOURCE);
