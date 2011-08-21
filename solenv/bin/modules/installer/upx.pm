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

package installer::upx;

use installer::converter;
use installer::existence;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::scriptitems;
use installer::systemactions;

#####################################################################
# Checking whether a file has to be stripped
#####################################################################

sub is_upx_candidate
{
    my ( $filename, $onefile ) = @_;

    my $useupx = 0;

    if (( $filename =~ /\.so\s*$/ ) ||
        ( $filename =~ /\.dll\s*$/ ) ||
        ( $filename =~ /\.exe\s*$/ ) ||
        ( $filename =~ /\.bin\s*$/ ))
    {
        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
        if ( ! ( $styles =~ /\bDONT_UPX\b/ )) { $useupx = 1; }
    }

    return $useupx;
}

#####################################################################
# Checking whether a file has to be stripped
#####################################################################

sub do_upx
{
    my ( $filename ) = @_;

    my $compression = "9";
    my $systemcall = $installer::globals::upxfile . " -" . $compression . " " . $filename;

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "WARNING: Could not successfully upx $filename! Using original file.\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "SUCCESS: upx $filename!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $returnvalue;
}

#####################################################################
# Using upx to decrease file size
#####################################################################

sub upx_on_libraries
{
    my ( $filelist, $languagestringref) = @_;

    installer::logger::include_header_into_logfile("UPX'ing files:");
    my $infoline = "";

    if ( ! $installer::globals::upx_in_path )
    {
        $infoline = "\n\nWarning: This is an UPX product, but upx was not found in PATH!\n\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Using upx: $installer::globals::upxfile\n";
        push( @installer::globals::logfileinfo, $infoline);

        my $upxdirbase = installer::systemactions::create_directories("upx", $languagestringref);

        if (! installer::existence::exists_in_array($upxdirbase, \@installer::globals::removedirs))
        {
            push(@installer::globals::removedirs, $upxdirbase);
        }

        for ( my $i = 0; $i <= $#{$filelist}; $i++ )
        {
            my $sourcefilename = ${$filelist}[$i]->{'sourcepath'};

            if ( is_upx_candidate($sourcefilename, ${$filelist}[$i]) )
            {
                my $shortfilename = $sourcefilename;
                installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$shortfilename);

                $infoline = "\nUpx: $shortfilename";
                push( @installer::globals::logfileinfo, $infoline);

                # copy file into directory for stripped libraries
                my $onelanguage = ${$filelist}[$i]->{'specificlanguage'};

                # files without language into directory "00"
                if ($onelanguage eq "") { $onelanguage = "00"; }

                my $upxdir = $upxdirbase . $installer::globals::separator . $onelanguage;
                installer::systemactions::create_directory($upxdir);    # creating language specific subdirectories

                my $destfilename = $upxdir . $installer::globals::separator . $shortfilename;
                installer::systemactions::copy_one_file($sourcefilename, $destfilename);

                # change sourcepath in files collector
                ${$filelist}[$i]->{'sourcepath'} = $destfilename;

                # do upx on file
                my $return = do_upx($destfilename);

                # Using original file, if upx was not successful (no reason for error)
                if ( $return ) { ${$filelist}[$i]->{'sourcepath'} = $sourcefilename; }
            }
        }
    }
}

1;
