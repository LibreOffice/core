:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: list_icons.pl,v $
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
