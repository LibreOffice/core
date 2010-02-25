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
    my $infoline = "Updated idt file: $reglocatfilename\n";
    push(@installer::globals::logfileinfo, $infoline);

    installer::files::save_file($appsearchfilename ,$appsearchfile);
    $infoline = "Updated idt file: $appsearchfilename\n";
    push(@installer::globals::logfileinfo, $infoline);

    installer::files::save_file($signaturefilename ,$signaturefile);
    $infoline = "Updated idt file: $signaturefilename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;
