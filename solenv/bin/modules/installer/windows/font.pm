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

package installer::windows::font;

use installer::files;
use installer::globals;
use installer::windows::idtglobal;


#################################################################################
# Creating the file Font.idt dynamically
# Content:
# File_ FontTitle
#################################################################################

sub create_font_table
{
    my ($filesref, $basedir) = @_;

    my @fonttable = ();

    installer::windows::idtglobal::write_idt_header(\@fonttable, "font");

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $styles = "";

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( $styles =~ /\bFONT\b/ )
        {
            my %font = ();

            $font{'File_'} = $onefile->{'uniquename'};
            $font{'FontTitle'} = "";

            my $oneline = $font{'File_'} . "\t" . $font{'FontTitle'} . "\n";

            push(@fonttable, $oneline);
        }
    }

    # Saving the file

    my $fonttablename = $basedir . $installer::globals::separator . "Font.idt";
    installer::files::save_file($fonttablename ,\@fonttable);
    my $infoline = "Created idt file: $fonttablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

#################################################################################
# Reading the Font version from the ttf file, to avoid installation
# of older files over newer files.
#################################################################################

sub get_font_version
{
    my ( $fontfile ) = @_;

    if ( ! -f $fontfile ) { installer::exiter::exit_program("ERROR: Font file does not exist: \"$fontfile\"", "get_font_version"); }

    my $fontversion = 0;
    my $infoline = "";

    my $onefile = installer::files::read_binary_file($fontfile);

    if ( $onefile =~ /Version\s+(\d+\.\d+\.*\d*)/ )
    {
        $fontversion = $1;
        $infoline = "FONT: Font \"$fontfile\" version: $fontversion\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "FONT: Could not determine font version: \"$fontfile\"\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    return $fontversion;
}

1;
