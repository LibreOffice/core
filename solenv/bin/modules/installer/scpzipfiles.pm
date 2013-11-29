#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



package installer::scpzipfiles;

use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::systemactions;

########################################################################################
# Replacing all zip list variables in setup script and files with flag scpzip_replace
########################################################################################

sub replace_all_ziplistvariables_in_file
{
    my ( $fileref, $variableshashref ) = @_;

    for ( my $i = 0; $i <= $#{$fileref}; $i++ )
    {
        my $line = ${$fileref}[$i];

        if ( $line =~ /^.*\$\{\w+\}.*$/ )   # only occurence of ${abc}
        {
            my $key;

            foreach $key (keys %{$variableshashref})
            {
                my $value = $variableshashref->{$key};
                $key = '${' . $key . '}';
                $line =~ s/\Q$key\E/$value/g;
                ${$fileref}[$i] = $line;
            }
        }
    }
}

########################################################################################
# Replacing all zip list variables in rtf files. In rtf files
# the brackets are masked.
########################################################################################

sub replace_all_ziplistvariables_in_rtffile ($$)
{
    my ($lines, $variablesref) = @_;

    my $line_count = scalar @$lines;
    for (my $i=0; $i<=$line_count; ++$i)
    {
        my $line = $lines->[$i];

        if ($line =~ /\$\\\{/) # early rejection of lines without variable references
        {
            while (my ($key, $value) = each (%$variables))
            {
                my $pattern = '$\{' . $key . '\}';
                $line =~ s/\Q$key\E/$value/g;

            }
            $lines->[$i] = $line;
        }
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

    $installer::logger::Lang->printf("\n");
}

1;
