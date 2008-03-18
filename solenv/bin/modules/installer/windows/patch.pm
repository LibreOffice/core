#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: patch.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:04:23 $
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
