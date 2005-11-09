#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: java.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-11-09 09:11:38 $
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

package installer::windows::java;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

####################################################################################
# Writing content into RegLocat.idt and AppSearc.idt to find Java on system
####################################################################################

sub update_java_tables
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
    # Java version is saved in scp project
    # $installer::globals::javafile was defined in installer::windows::idtglobal::add_childprojects

    if ( ! $installer::globals::javafile->{'Javaversion'} ) { installer::exiter::exit_program("ERROR: \"Javaversion\" has to be defined in $installer::globals::javafile->{'gid'} in scp project!", "update_java_tables"); }

    my $javastring = $installer::globals::javafile->{'Javaversion'};

    my $signature = "JavaReg";
    my $rootvalue = "2";
    my $key = "Software\\JavaSoft\\Java Runtime Environment\\" . $javastring;
    my $name = "JavaHome";
    my $type = 2;
    my $property = "JAVAPATH";

    my $oneline = $signature . "\t" . $rootvalue . "\t" . $key . "\t" . $name . "\t" . $type . "\n";
    push(@{$reglocatfile}, $oneline);

    $oneline = $property . "\t" . $signature . "\n";
    push(@{$appsearchfile}, $oneline);

    # Saving the files

    installer::files::save_file($reglocatfilename ,$reglocatfile);
    my $infoline = "Updated idt file for Java: $reglocatfilename\n";
    push(@installer::globals::logfileinfo, $infoline);

    installer::files::save_file($appsearchfilename ,$appsearchfile);
    $infoline = "Updated idt file for Java: $appsearchfilename\n";
    push(@installer::globals::logfileinfo, $infoline);

    installer::files::save_file($signaturefilename ,$signaturefile);
    $infoline = "Updated idt file: $signaturefilename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;
