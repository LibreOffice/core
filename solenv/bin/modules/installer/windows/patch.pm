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



package installer::windows::patch;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

####################################################################################
# Creating the file Upgrade.idt dynamically
# Content:
# UpgradeCode VersionMin VersionMax Language Attributes Remove ActionProperty
####################################################################################

sub update_patch_tables
{
    my ($basedir, $allvariables) = @_;

    my $reglocatfile = "";
    my $appsearchfile = "";

    my $reglocatfilename = $basedir . $installer::globals::separator . "RegLocat.idt";
    my $appsearchfilename = $basedir . $installer::globals::separator . "AppSearc.idt";
    my $signaturefilename = $basedir . $installer::globals::separator . "Signatur.idt";

    if ( -f $reglocatfilename )
    {
        $reglocatfile = installer::files::read_file($reglocatfilename);
    }
    else
    {
        my @reglocattable = ();
        $reglocatfile = \@reglocattable;
        installer::windows::idtglobal::write_idt_header($reglocatfile, "reglocat");
    }

    if ( -f $appsearchfilename )
    {
        $appsearchfile = installer::files::read_file($appsearchfilename);
    }
    else
    {
        my @appsearchtable = ();
        $appsearchfile = \@appsearchtable;
        installer::windows::idtglobal::write_idt_header($appsearchfile, "appsearch");
    }

    if ( -f $signaturefilename )
    {
        $signaturefile = installer::files::read_file($signaturefilename);
    }
    else
    {
        my @signaturetable = ();
        $signaturefile = \@signaturetable;
        installer::windows::idtglobal::write_idt_header($signaturefile, "signatur");
    }

    # Writing content into this tables

    if ( ! $allvariables->{'PATCHCODEFILE'} ) { installer::exiter::exit_program("ERROR: Variable PATCHCODEFILE must be defined for Windows patches!", "update_patch_tables"); }
    my $patchcodesfilename = $installer::globals::idttemplatepath  . $installer::globals::separator . $allvariables->{'PATCHCODEFILE'};
    my $patchcodefile = installer::files::read_file($patchcodesfilename);

    my $number = 0;

    for ( my $i = 0; $i <= $#{$patchcodefile}; $i++ )
    {
        my $oneline = ${$patchcodefile}[$i];

        if ( $oneline =~ /^\s*\#/ ) { next; }   # this is a comment line
        if ( $oneline =~ /^\s*$/ ) { next; }

        my $code = "";
        if ( $oneline =~ /^\s*(\S+)\s/ ) { $code = $1; }

        foreach my $name ( sort keys %installer::globals::installlocations )
        {
            $number++;
            my $signature = "dir" . $number . "user";
            my $rootvalue = "1";
            my $registryname = "";
            my $registryversion = "";

            if ( $allvariables->{'SEARCHPRODUCTNAME'} ) { $registryname = $allvariables->{'SEARCHPRODUCTNAME'}; }
            else { $registryname = $allvariables->{'PRODUCTNAME'}; }

            if ( $allvariables->{'SEARCHPRODUCTVERSION'} ) { $registryversion = $allvariables->{'SEARCHPRODUCTVERSION'}; }
            else { $registryversion = $allvariables->{'PRODUCTVERSION'}; }

            my $key = "Software\\" . $allvariables->{'MANUFACTURER'} . "\\" . $registryname . "\\" . $registryversion . "\\" . $code;

            my $type = 2;
            my $property = $name;

            $oneline = $signature . "\t" . $rootvalue . "\t" . $key . "\t" . $name . "\t" . $type . "\n";
            push(@{$reglocatfile}, $oneline);

            $oneline = $property . "\t" . $signature . "\n";
            push(@{$appsearchfile}, $oneline);

            $signature = "dir" . $number . "mach";
            $rootvalue = "2";

            $oneline = $signature . "\t" . $rootvalue . "\t" . $key . "\t" . $name . "\t" . $type . "\n";
            push(@{$reglocatfile}, $oneline);

            $oneline = $property . "\t" . $signature . "\n";
            push(@{$appsearchfile}, $oneline);
        }
    }

    # Saving the files

    installer::files::save_file($reglocatfilename ,$reglocatfile);
    $installer::logger::Lang->printf("Updated idt file: %s\n", $reglocatfilename);

    installer::files::save_file($appsearchfilename ,$appsearchfile);
    $installer::logger::Lang->printf("Updated idt file: %s\n", $appsearchfilename);

    installer::files::save_file($signaturefilename ,$signaturefile);
    $installer::logger::Lang->printf("Updated idt file: %s\n", $signaturefilename);
}

1;
