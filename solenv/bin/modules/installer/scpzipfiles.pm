#*************************************************************************
#
#   $RCSfile: scpzipfiles.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:16:59 $
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

package installer::scpzipfiles;

use installer::files;
use installer::globals;
use installer::logger;
use installer::systemactions;

########################################################################################
# Replacing all zip list variables in setup script and files with flag scpzip_replace
########################################################################################

sub replace_all_ziplistvariables_in_file
{
    my ( $fileref, $variablesref ) = @_;

    for ( my $i = 0; $i <= $#{$fileref}; $i++ )
    {
        my $line = ${$fileref}[$i];

        if ( $line =~ /^.*\$\{\w+\}.*$/ )   # only occurence of ${abc}
        {
            for ( my $j = 0; $j <= $#{$variablesref}; $j++ )
            {
                my $variableline = ${$variablesref}[$j];

                my ($key, $value);

                if ( $variableline =~ /^\s*(\w+?)\s+(.*?)\s*$/ )
                {
                    $key = $1;
                    $value = $2;
                    $key = '${' . $key . '}';
                }

                $line =~ s/\Q$key\E/$value/g;

                ${$fileref}[$i] = $line;
            }
        }
    }
}

#########################################################
# Analyzing files with flag SCPZIP_REPLACE
# $item can be "File" or "ScpAction"
#########################################################

sub resolving_scpzip_replace_flag
{
    my ($filesarrayref, $variablesref, $item) = @_;

    my $diritem = lc($item);

    my $replacedirbase = installer::systemactions::create_directories("replace_$diritem", "");

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

                my $onefilename = $onefile->{'Name'};
                my $sourcepath = $onefile->{'sourcepath'};
                my $destinationpath = $replacedir . $onefilename;
                my $movepath = $destinationpath . ".orig";

            #   if (!(-f $movepath))    # do nothing if the file already exists
            #   {
                my $copysuccess = installer::systemactions::copy_one_file($sourcepath, $movepath);

                if ( $copysuccess )
                {
                    # Now the file can be edited
                    # ToDo: How about binary patching?

                    my $onefileref = installer::files::read_file($movepath);
                    replace_all_ziplistvariables_in_file($onefileref, $variablesref);
                    installer::files::save_file($destinationpath ,$onefileref);
                }
            #   }

                # Writing the new sourcepath into the hashref, even if it was no copied

                $onefile->{'sourcepath'} = $destinationpath;
            }
            else
            {
                # ToDo: How about SCPZIP_REPALCE and ARCHIVE?
                # Requires that the zip file was unpacked in resolving_archive_flag
                # and that $installer::globals::dounzip is set (Parameter -unzip).
            }
        }
    }

    my $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);
}

1;
