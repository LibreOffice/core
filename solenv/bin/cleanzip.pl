#!/usr/bin/perl -w
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



sub usage
{
    print "Cleanup unwanted unix attributes in zip archives\n\n";
    print "Usage:\n";
    print "$0 archive\n\n";
    exit(1);
}

usage() if ! defined $ARGV[0];

my $filename = $ARGV[0];
use Archive::Zip qw(:ERROR_CODES :CONSTANTS);
my $zip = Archive::Zip->new();

unless ( $zip->read( $filename ) == AZ_OK ) {
    die "$0: ERROR reading $filename\n";
}
my @members = $zip ->members();

foreach my $member ( @members ) {
#   printf ( "%o\n",$member->unixFileAttributes());
#   printf ( "%o\n",$member->unixFileAttributes() & 0b111111111111);
    my $attribs = $member->unixFileAttributes();
    if ( $member->isDirectory ) {
        $attribs = $attribs & 0b101111111111;
        $member->unixFileAttributes($attribs)
    }
#   printf ( "%o\n",$member->unixFileAttributes());
#   printf ( "%o\n",$member->unixFileAttributes() & 0b111111111111);
#   print ( $member->fileName()."\n");
}
unless ( $zip->writeToFileNamed( ${filename}."_new" ) == AZ_OK ) {
    die "$0: ERROR reading ${filename}_new\n";
}
rename($filename."_new", $filename);

