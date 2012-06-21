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
