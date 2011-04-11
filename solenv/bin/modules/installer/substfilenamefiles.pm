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

package installer::substfilenamefiles;

use installer::exiter;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::systemactions;

#########################################################
# Analyzing files with flag SUBST_FILENAME
#########################################################

sub resolving_subst_filename_flag
{
    my ($filesarrayref, $variableshashref, $languagestringref) = @_;

    my $replacedirbase = installer::systemactions::create_directories("change_filename", $languagestringref);

    installer::logger::include_header_into_logfile("Files with flag SUBST_FILENAME:");

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $styles = "";

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( $styles =~ /\bSUBST_FILENAME\b/ )
        {
            # Files with flag SUBST_FILENAME must have a "Substitute" key
            if (( ! $onefile->{'Substitute'} ) && ( ! $onefile->{'InstallName'} ))
            {
                installer::exiter::exit_program("ERROR: SUBST_FILENAME is set, but no Substitute and no InstallName defined at file $onefile->{'gid'}!", "resolving_subst_filename_flag");
            }

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

            if ( $onefilename =~ /\Q$installer::globals::separator\E/ ) # filename begins with a slash, for instance /registry/schema/org/openoffice/VCL.xcs
            {
                $onefilename =~ s/^\s*\Q$installer::globals::separator\E//;
                $longfilename = 1;
            }

            my $destinationpath = $replacedir . $onefilename;
            my $movepath = $destinationpath . ".orig";
            my $destdir = $replacedir;

            if ( $longfilename )    # the destination directory has to be created before copying
            {
                $destdir = $movepath;
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$destdir);
                installer::systemactions::create_directory_structure($destdir);
            }

            my $copysuccess = installer::systemactions::copy_one_file($sourcepath, $movepath);

            if ( $copysuccess )
            {
                if ( $onefile->{'Substitute'} )
                {
                    my $substitute = $onefile->{'Substitute'};

                    my $newfilename = $destinationpath;
                    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$newfilename);
                    eval '$newfilename =~ ' . "$substitute";

                    my $longnewfilename = $destdir . $newfilename;

                    $copysuccess = installer::systemactions::copy_one_file($movepath, $longnewfilename);

                    # Saving the new file name
                    $onefile->{'Name'} = $newfilename;

                    # Saving the new destination
                    my $newdest = $onefile->{'destination'};
                    installer::pathanalyzer::get_path_from_fullqualifiedname(\$newdest);
                    $onefile->{'destination'} = $newdest . $newfilename;

                    # Saving the original source, where the file was found
                    $onefile->{'originalsourcepath'} = $onefile->{'sourcepath'};

                    # Writing the new sourcepath into the hashref, even if it was not copied
                    $onefile->{'sourcepath'} = $longnewfilename;
                }
                else
                {
                    if ( $onefile->{'InstallName'} )
                    {
                        my $installname = $onefile->{'InstallName'};

                        my $newfilename = $destinationpath;
                        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$newfilename);

                        my $longnewfilename = $destdir . $installname;

                        $copysuccess = installer::systemactions::copy_one_file($movepath, $longnewfilename);

                        # Saving the new file name
                        $onefile->{'Name'} = $installname;

                        # Saving the new destination
                        my $newdest = $onefile->{'destination'};
                        installer::pathanalyzer::get_path_from_fullqualifiedname(\$newdest);
                        $onefile->{'destination'} = $newdest . $installname;

                        # Saving the original source, where the file was found
                        $onefile->{'originalsourcepath'} = $onefile->{'sourcepath'};

                        # Writing the new sourcepath into the hashref, even if it was not copied
                        $onefile->{'sourcepath'} = $longnewfilename;
                    }
                }
            }
        }
    }

    my $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);
}

1;
