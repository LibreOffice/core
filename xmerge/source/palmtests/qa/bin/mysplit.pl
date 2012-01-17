#!/usr/local/bin/perl
#########################################################################

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



open(TESTFILE, $ARGV[0]);

while (<TESTFILE>)
{
    if ($_[1] eq "-q")
    {
    chomp $_;
    @args = split('\|', $_);
    @filestuff = split('\.', @args[0]);
    $filename = @filestuff[0] . ".infile";
    open (TESTCASE, ">$filename") || die "Error opening $filename";
    print TESTCASE "# @args[1]\n# @args[2]\n";
    print TESTCASE "TEST|@args[1]|QUICKWORD|@filestuff[0]\n" ;
    close TESTCASE;
    }
    if ($_[1] eq "-m")
    {
    chomp $_;
    @args = split('\|', $_);
    @filestuff = split('\.', @args[0]);
    $filename = @filestuff[0] . ".infile";
    open (TESTCASE, ">$filename") || die "Error opening $filename";
    print TESTCASE "# @args[1]\n# @args[2]\n";
    print TESTCASE "TEST|@args[1]|MINICALC|@filestuff[0]\n" ;
    close TESTCASE;
    }

}
