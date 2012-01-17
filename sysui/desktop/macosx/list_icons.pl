:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



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
