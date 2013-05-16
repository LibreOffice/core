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
# pushids - push HID.LST and *.win files for userexperience feedback
#

use lib ("$ENV{SOLARENV}/bin/modules", "$ENV{COMMON_ENV_TOOLS}/modules");

use Carp;

sub parse_info($$);

if ( @ARGV != 3 )
{
  print "usage:   $ARGV[0] <path tp hid.lst> <path to *.win files> <output file>\n";
  print "example: $ARGV[0] ./hid.lst global/win common/misc/UserFeedbackNames.csv\n\n";
  die "invalid params";
}

my ($hid, $winpath, $outfile) = @ARGV;

my @names;

open HID, "<$hid" or die "can't open file $filename $! $^E";
for (<HID>) {
  chop;
  my ($longname, $ID) = split " +";
  next if ( ! $ID );
  $upperlongname = $longname;
  $upperlongname =~ tr/a-z/A-Z/;
  $undeclared_hids{$upperlongname} = $longname;

  if ( exists $hids{$upperlongname} && ( $hids{$upperlongname} != $ID ) )
  {
    print STDERR "warn: unclear definition of longname: $longname = $hids{$upperlongname} or $ID\n";
  }
  $hids{$upperlongname} = $ID;

  if ( exists $revhids{ $ID } && ( $revhids{ $ID } ne $upperlongname ) )
  {
    print STDERR "warn: two longnames have the same ID: $longname and $revhids{$ID} share ID $ID\n";
  }
  $revhids{$ID} = $upperlongname;
}

close HID;

undef @revhids;

#Add Active
$hids{"ACTIVE"} = 0;

my %dialogs = ();

foreach ( glob("$winpath/*win") ) {
  $filename = $_;
  open WIN, "< $filename" or die "can't open file $filename $! $^E";
  my $parentinfo = "";
  my @dialog = ();
  my $parentshortname = "";

  for ( <WIN> ) {
    chop;

    s/^ +//;
    s/ +/ /g;

    next if /^ *'/;
    next if /^ *$/;

    my $ID = "";
    my $iteminfo;
    my ($shortname, $longname) = split " +";

    $shortname = "" if ( !$shortname );
    $longname = "" if ( !$longname );

    # fake a correct entry if only *active is given and overwrite the attempt to declare it differently
    if ( $shortname =~ /\*active/i )
    {
      $longname = "Active";
    }


# find UNO Names
    if ( $longname =~ /^(.uno:|http|private:factory|service:|macro:|.HelpId:)/i || $longname =~ s/^sym://i )
    {
      $ID = $longname;
      $longname = "";
    }
    else
    {
      my $upperlongname = $longname;
      $upperlongname =~ tr/a-z/A-Z/;
      if ( $shortname !~ /^[\+\*]/ && !exists $hids{$upperlongname} )
      {
        print STDERR "warn: Longname not in hid.lst: $filename $longname\n";
      }
      if ( exists $hids{$upperlongname} )
      {
          $ID = $hids{$upperlongname};
      }
      delete $undeclared_hids{$upperlongname};
    }

    $iteminfo = "$shortname $longname $ID";
    $iteminfo =~ s/^\*//;
    $iteminfo =~ s/^\+//;

# find start of deklaration
    if ( $shortname =~ s/^\+// )
    {
      # copy existing dialog
      if ( exists $dialogs{ $longname } )
      {
        my @old = @{$dialogs{ $longname }};
        my ($oldshort, $oldlong, $oldID ) = split ( " ", shift @old );
        $iteminfo = "$shortname $oldlong $oldID";

        $parentinfo = $iteminfo;
        $parentshortname = $shortname;
        $dialogs{ $parentshortname } = \@dialog;
        @dialog = ();        # break the link
        push ( @{$dialogs{ $parentshortname }}, $iteminfo );
        push @names, "   $parentinfo";

        for ( @old )
        {
          push @names, "$parentinfo $_";
        }
      }
      else
      {  # fake new dialog instead
        $shortname = "*".$shortname;
      }
    }
    if ( $shortname =~ s/^\*// )
    {
      $parentinfo = $iteminfo;
      $parentshortname = $shortname;
      $dialogs{ $parentshortname } = \@dialog;
      @dialog = ();        # break the link
      push ( @{$dialogs{ $parentshortname }}, $iteminfo );
      push @names, "   $parentinfo";
    }
    else
    {
      push ( @{$dialogs{ $parentshortname }}, $iteminfo );
      push @names, "$parentinfo $iteminfo";
    }

  }
  close WIN;
}

for ( keys %undeclared_hids ) {
  $iteminfo = "$undeclared_hids{$_} $undeclared_hids{$_} $hids{$_}";
  push @names, "   $iteminfo";
}

#----------------------------------------------------------------------------
# write to files

open HIDS, ">$outfile" or die "can't open file $filename $! $^E";
print HIDS join "\n", @names;
print HIDS "\n";
close HIDS;

