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




package par2script::files;

use par2script::exiter;

############################################
# File Operations
############################################

sub check_file
{
    my ($arg) = @_;

    if(!( -f $arg ))
    {
        par2script::exiter::exit_program("ERROR: Cannot find file $arg", "check_file");
    }
}

sub read_file
{
    my ($localfile) = @_;

    my @localfile = ();

    open( IN, "<$localfile" ) || par2script::exiter::exit_program("ERROR: Cannot open file: $localfile", "read_file");
    while ( <IN> ) { push(@localfile, $_); }
    close( IN );

    return \@localfile;
}

###########################################
# Saving files, arrays and hashes
###########################################

sub save_file
{
    my ($savefile, $savecontent) = @_;
    open( OUT, ">$savefile" );
    print OUT @{$savecontent};
    close( OUT);
    if (! -f $savefile) { pre2par::exiter::exit_program("ERROR: Cannot write file: $savefile", "save_file"); }
}

sub save_hash
{
    my ($savefile, $hashref) = @_;

    my @printcontent = ();

    my ($itemkey, $itemvalue, $line);

    foreach $itemkey ( keys %{$hashref} )
    {
        $line = "";
        $itemvalue = $hashref->{$itemkey};
        $line = $itemkey . "=" . $itemvalue . "\n";
        push(@printcontent, $line);
    }

    open( OUT, ">$savefile" );
    print OUT @printcontent;
    close( OUT);
}

sub save_array_of_hashes
{
    my ($savefile, $arrayref) = @_;

    my @printcontent = ();

    my ($itemkey, $itemvalue, $line, $hashref);

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        $line = "";
        $hashref = ${$arrayref}[$i];

        foreach $itemkey ( keys %{$hashref} )
        {
            $itemvalue = $hashref->{$itemkey};

            $line = $line . $itemkey . "=" . $itemvalue . "\t";
        }

        $line = $line . "\n";

        push(@printcontent, $line);
    }

    open( OUT, ">$savefile" );
    print OUT @printcontent;
    close( OUT);
}

1;
