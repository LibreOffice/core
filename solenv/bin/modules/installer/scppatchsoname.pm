#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: scppatchsoname.pm,v $
#
# $Revision: 1.8 $
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

package installer::scppatchsoname;

use installer::files;
use installer::globals;
use installer::logger;
use installer::setupscript;
use installer::systemactions;

########################################################################################
# The length of the new string must be identical with the length of the old string
########################################################################################

sub change_length_of_string
{
    my ($newstringref, $oldstring) = @_;

    while ( length($$newstringref) < length($oldstring) )
    {
        $$newstringref = $$newstringref . chr(0);
    }
}

########################################################################################
# Converting a string to a unicode string
########################################################################################

sub convert_to_unicode
{
    my ($string) = @_;

    my $unicodestring = "";

    my $stringlength = length($string);

    for ( my $i = 0; $i < $stringlength; $i++ )
    {
        $unicodestring = $unicodestring . substr($string, $i, 1);
        $unicodestring = $unicodestring . chr(0);
    }

    return $unicodestring;
}

########################################################################################
# Replacing the so name in all files with flag PATCH_SO_NAME
########################################################################################

sub replace_productname_in_file
{
    my ($sourcepath, $destpath, $variableshashref, $filedescription) = @_;

    my $onefile = installer::files::read_binary_file($sourcepath);

    my $onestring = "x" . chr(0);
    my $replacestring = "";
    for ( my $i = 1; $i <= 80; $i++ ) { $replacestring .= $onestring; }

    my $productname = $variableshashref->{'PRODUCTNAME'} . " " . $variableshashref->{'PRODUCTVERSION'};
    if ( $filedescription ne "" ) { $productname = $filedescription; }
    my $unicode_productname = convert_to_unicode($productname);

    change_length_of_string(\$unicode_productname, $replacestring);

    my $found = $onefile =~ s/$replacestring/$unicode_productname/s;

    installer::files::save_binary_file($onefile, $destpath);

    return $found;
}

#########################################################
# Analyzing files with flag PATCH_SO_NAME
#########################################################

sub resolving_patchsoname_flag
{
    my ($filesarrayref, $variableshashref, $item, $languagestringref) = @_;

    my $diritem = lc($item);

    my $replacedirbase = installer::systemactions::create_directories("patchsoname_$diritem", $languagestringref);

    installer::logger::include_header_into_logfile("$item with flag PATCH_SO_NAME:");

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $styles = "";

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( $styles =~ /\bPATCH_SO_NAME\b/ )
        {
            # Language specific subdirectory

            my $onelanguage = $onefile->{'specificlanguage'};
            my $filedescription = "";

            if ($onelanguage eq "")
            {
                $onelanguage = "00";    # files without language into directory "00"
            }

            my $replacedir = $replacedirbase . $installer::globals::separator . $onelanguage . $installer::globals::separator;
            installer::systemactions::create_directory($replacedir);    # creating language specific directories

            # copy files and edit them with the variables defined in the zip.lst

            my $onefilename = $onefile->{'Name'};
            my $sourcepath = $onefile->{'sourcepath'};
            my $destinationpath = $replacedir . $onefilename;
            my $movepath = $destinationpath . ".orig";

            if ( exists($onefile-> {'FileDescription'}) ) { $filedescription = $onefile-> {'FileDescription'}; }

            # if (!(-f $destinationpath))   # do nothing if the file already exists
            # {

            my $copysuccess = installer::systemactions::copy_one_file($sourcepath, $movepath);

            if ( $copysuccess )
            {
                # Now the file can be patch (binary!)
                my $found = replace_productname_in_file($movepath, $destinationpath, $variableshashref, $filedescription);

                if ($found == 0)
                {
                    my $infoline = "Did not patch the file $destinationpath\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }
                else
                {
                    my $infoline = "Successfully patched $destinationpath, Count: $found\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }
            }

            # }

            # Saving the original source, where the file was found
            $onefile->{'originalsourcepath'} = $onefile->{'sourcepath'};

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
