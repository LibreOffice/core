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

package installer::scpzipfiles;

use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::systemactions;

# Replacing all zip list variables.
# Called for setup script as well as files with flag scpzip_replace.

sub replace_all_ziplistvariables_in_file
{
    my ( $fileref, $variablesref ) = @_;

    for my $line ( @{$fileref} )
    {
        # Avoid removing variables we do not recognise.
        $line =~ s/\$\{(\w+)\}/defined $variablesref->{$1}
                                     ? $variablesref->{$1}
                                     : $&/eg;
    }
}

# Replacing all zip list variables in rtf files.

sub replace_all_ziplistvariables_in_rtffile
{
    my ( $fileref, $variablesref ) = @_;

    for my $line ( @{$fileref} )
    {
        # In rtf files the braces are backslash-escaped.
        # Avoid removing variables we do not recognise.
        $line =~ s/\$\\\{(\w+)\\\}/defined $variablesref->{$1}
                                         ? $variablesref->{$1}
                                         : $&/eg;
    }
}

#########################################################
# Analyzing files with flag SCPZIP_REPLACE
# $item can be "File" or "ScpAction"
#########################################################

sub resolving_scpzip_replace_flag
{
    my ($filesarrayref, $variableshashref, $item, $languagestringref) = @_;

    my $diritem = lc($item);

    my $replacedirbase = installer::systemactions::create_directories("replace_$diritem", $languagestringref);

    installer::logger::include_header_into_logfile("$item with flag SCPZIP:");

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $styles = "";

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( $styles =~ /\bSCPZIP_REPLACE\b/ )
        {
            # Language specific subdirectory

            my $onelanguage = $onefile->{'specificlanguage'};

            if ($onelanguage eq "")
            {
                $onelanguage = "00";    # files without language into directory "00"
            }

            my $replacedir = $replacedirbase . $installer::globals::separator . $onelanguage . $installer::globals::separator;
            installer::systemactions::create_directory($replacedir);    # creating language specific directories

            # copy files and edit them with the variables defined in the zip.lst

            my $longfilename = 0;

            my $onefilename = $onefile->{'Name'};
            my $sourcepath = $onefile->{'sourcepath'};

            if ( $onefilename =~ /^\s*\Q$installer::globals::separator\E/ ) # filename begins with a slash, for instance /registry/schema/org/openoffice/VCL.xcs
            {
                $onefilename =~ s/^\s*\Q$installer::globals::separator\E//;
                $longfilename = 1;
            }

            my $destinationpath = $replacedir . $onefilename;
            my $movepath = $destinationpath . ".orig";

            if ( $longfilename )    # the destination directory has to be created before copying
            {
                my $destdir = $movepath;
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$destdir);
                installer::systemactions::create_directory_structure($destdir);
            }

            my $copysuccess = installer::systemactions::copy_one_file($sourcepath, $movepath);

            if ( $copysuccess )
            {
                # Now the file can be edited
                # ToDo: How about binary patching?

                my $onefileref = installer::files::read_file($movepath);
                replace_all_ziplistvariables_in_file($onefileref, $variableshashref);
                installer::files::save_file($destinationpath ,$onefileref);
            }

            # Saving the original source, where the file was found
            $onefile->{'originalsourcepath'} = $onefile->{'sourcepath'};

            # Writing the new sourcepath into the hashref, even if it was no copied

            $onefile->{'sourcepath'} = $destinationpath;
        }
    }

    my $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);
}

1;
