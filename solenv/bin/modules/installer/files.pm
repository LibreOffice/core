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

package installer::files;

use strict;
use warnings;

use installer::exiter;
use installer::logger;

############################################
# File Operations
############################################

sub check_file
{
    my ($arg) = @_;

    if(!( -f $arg ))
    {
        installer::exiter::exit_program("ERROR: Cannot find file $arg", "check_file");
    }
}

sub read_file
{
    my ($localfile) = @_;
    my @localfile = ();

    open( IN, "<$localfile" ) || installer::exiter::exit_program("ERROR: Cannot open file $localfile for reading", "read_file");

#   Don't use "my @localfile = <IN>" here, because
#   perl has a problem with the internal "large_and_huge_malloc" function
#   when calling perl using Mac OS X 10.5 with a perl built with Mac OS X 10.4
    while ( my $line = <IN> ) {
        push @localfile, $line;
    }

    close( IN );

    return \@localfile;
}

###########################################
# Saving files, arrays and hashes
###########################################

sub save_file
{
    my ($savefile, $savecontent) = @_;

    if ( open( OUT, ">$savefile" ) )
    {
        print OUT @{$savecontent};
        close( OUT);
    }
    else
    {
        # it is useless to save a log file, if there is no write access

        if ( $savefile =~ /\.log/ )
        {
            print "\n*************************************************\n";
            print "ERROR: Cannot write log file $savefile, $!";
            print "\n*************************************************\n";
            exit(-1);   # exiting the program to avoid endless loops
        }

        installer::exiter::exit_program("ERROR: Cannot open file $savefile for writing", "save_file");
    }
}

###########################################
# Binary file operations
###########################################

sub read_binary_file
{
    my ($filename) = @_;

    my $file;

    open( IN, "<$filename" ) || installer::exiter::exit_program("ERROR: Cannot open file $filename for reading", "read_binary_file");
    binmode IN;
    seek IN, 0, 2;
    my $length = tell IN;
    seek IN, 0, 0;
    read IN, $file, $length;
    close IN;

    return $file;
}

sub save_binary_file
{
    my ($file, $filename) = @_;

    open( OUT, ">$filename" ) || installer::exiter::exit_program("ERROR: Cannot open file $filename for writing", "save_binary_file");
    binmode OUT;
    print OUT $file;
    close OUT;
}

1;
