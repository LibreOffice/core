#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
