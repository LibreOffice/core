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

package installer::strip;

use strict;
use warnings;

use base 'Exporter';

use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::systemactions;

our @EXPORT_OK = qw(strip_libraries);

#####################################################################
# Checking whether a file has to be stripped
#####################################################################

sub _need_to_strip
{
    my ( $filename ) = @_;

    my $strip = 0;

    # Check using the "file" command

    open (FILE, "file $filename |");
    my $fileoutput = <FILE>;
    close (FILE);

    if (( $fileoutput =~ /not stripped/i ) && ( $fileoutput =~ /\bELF\b/ )) { $strip = 1; }

    return $strip
}

#####################################################################
# Checking whether a file has to be stripped
#####################################################################

sub _do_strip
{
    my ( $filename ) = @_;

    my $systemcall = "strip" . " " . $filename;

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not strip $filename!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "SUCCESS: Stripped library $filename!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

#####################################################################
# Resolving all variables in the packagename.
#####################################################################

sub strip_libraries
{
    my ( $filelist, $languagestringref ) = @_;

    installer::logger::include_header_into_logfile("Stripping files:");

    my $strippeddirbase = installer::systemactions::create_directories("stripped", $languagestringref);

    if (! grep {$_ eq $strippeddirbase} @installer::globals::removedirs)
    {
        push(@installer::globals::removedirs, $strippeddirbase);
    }

    for ( my $i = 0; $i <= $#{$filelist}; $i++ )
    {
        my $sourcefilename = ${$filelist}[$i]->{'sourcepath'};

        if ( _need_to_strip($sourcefilename) )
        {
            my $shortfilename = $sourcefilename;
            installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$shortfilename);

            my $infoline = "Strip: $shortfilename\n";
            push( @installer::globals::logfileinfo, $infoline);

            # copy file into directory for stripped libraries

            my $onelanguage = ${$filelist}[$i]->{'specificlanguage'};

            # files without language into directory "00"

            if ($onelanguage eq "") { $onelanguage = "00"; }

            my $strippeddir = $strippeddirbase . $installer::globals::separator . $onelanguage;
            installer::systemactions::create_directory($strippeddir);   # creating language specific subdirectories

            my $destfilename = $strippeddir . $installer::globals::separator . $shortfilename;
            installer::systemactions::copy_one_file($sourcefilename, $destfilename);

            # change sourcepath in files collector

            ${$filelist}[$i]->{'sourcepath'} = $destfilename;

            # strip file

            _do_strip($destfilename);
        }
    }
}

1;
