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

