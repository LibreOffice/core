#*************************************************************************
#
#   $RCSfile: scppatchsoname.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:16:46 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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
    my ($sourcepath, $destpath, $variableshashref) = @_;

    my $onefile = installer::files::read_binary_file($sourcepath);

    my $onestring = "x" . chr(0);
    my $replacestring = "";
    for ( my $i = 1; $i <= 80; $i++ ) { $replacestring .= $onestring; }

    my $productname = $variableshashref->{'PRODUCTNAME'};
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

            # if (!(-f $destinationpath))   # do nothing if the file already exists
            # {

            my $copysuccess = installer::systemactions::copy_one_file($sourcepath, $movepath);

            if ( $copysuccess )
            {
                # Now the file can be patch (binary!)
                my $found = replace_productname_in_file($movepath, $destinationpath, $variableshashref);

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

            # Writing the new sourcepath into the hashref, even if it was no copied

            $onefile->{'sourcepath'} = $destinationpath;
        }
    }

    my $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);
}

1;
