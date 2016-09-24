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
