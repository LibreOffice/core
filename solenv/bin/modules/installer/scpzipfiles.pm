#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: scpzipfiles.pm,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:01:03 $
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

sub replace_all_ziplistvariables_in_rtffile
{
    my ( $fileref, $variablesref, $onelanguage, $loggingdir ) = @_;

    # installer::files::save_file($loggingdir . "license_" . $onelanguage . "_before.rtf", $fileref);

    for ( my $i = 0; $i <= $#{$fileref}; $i++ )
    {
        my $line = ${$fileref}[$i];

        if ( $line =~ /^.*\$\\\{\w+\\\}.*$/ )   # only occurence of $\{abc\}
        {
            for ( my $j = 0; $j <= $#{$variablesref}; $j++ )
            {
                my $variableline = ${$variablesref}[$j];

                my ($key, $value);

                if ( $variableline =~ /^\s*([\w-]+?)\s+(.*?)\s*$/ )
                {
                    $key = $1;
                    $value = $2;
                    $key = '$\{' . $key . '\}';
                }

                $line =~ s/\Q$key\E/$value/g;

                ${$fileref}[$i] = $line;
            }
        }
    }

    # installer::files::save_file($loggingdir . "license_" . $onelanguage . "_after.rtf", $fileref);
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

            if (!( $styles =~ /\bARCHIVE\b/ ))      # The simple case
            {
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

                # Saving the original source, where the file was found
                $onefile->{'originalsourcepath'} = $onefile->{'sourcepath'};

                # Saving the original source, where the file was found
                $onefile->{'originalsourcepath'} = $onefile->{'sourcepath'};

                # Writing the new sourcepath into the hashref, even if it was no copied

                $onefile->{'sourcepath'} = $destinationpath;
            }
            else
            {
                # ToDo: How about SCPZIP_REPLACE and ARCHIVE?
                # Requires that the zip file was unpacked in resolving_archive_flag
                # and that $installer::globals::dounzip is set (Parameter -unzip).
            }
        }
    }

    my $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);
}

1;
