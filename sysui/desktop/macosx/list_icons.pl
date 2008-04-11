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
# $RCSfile: list_icons.pl,v $
#
# $Revision: 1.3 $
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

# package all .icns if XML::Parser module is not installed
eval 'use XML::Parser;'; if ( $@ ) { print '*.icns'; exit 0; };

my $valuetype = "none";
my $key = "none";

#
# XML handlers
#

sub start_handler {
  my ($parser,$element,%attributes) = @_;
  $valuetype = "$element";
}

sub char_handler {
  my ($parser,$string) = @_;
  if ( $key eq "CFBundleTypeIconFile" || $key eq "CFBundleIconFile" ) {
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    print "$string " if length($string) > 0;
  }
  $key = "$string" if $valuetype eq "key";
}

sub default_handler {
  my ($parser,$string) = @_;
}

sub end_handler {
  my ($parser,$element) = @_;

  $key = "none" if $valuetype ne "key";
  $valuetype = "none";
}

#
# main
#

my $parser = new XML::Parser(ErrorContext => 2,
                             Namespaces => 1);

$parser->setHandlers( Start => \&start_handler,
                      End => \&end_handler,
                      Char => \&char_handler,
                      Default => \&default_handler);
$parser->parse(STDIN);

print "\n";
