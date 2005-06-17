#*************************************************************************
#
#   $RCSfile: createfolder.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-06-17 14:07:47 $
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

package installer::windows::createfolder;

use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

##############################################################
# Returning directory for createfolder table.
##############################################################

sub get_createfolder_directory
{
    my ($onedir) = @_;

    my $uniquename = $onedir->{'uniquename'};

    return $uniquename;
}

##############################################################
# Searching the correct file for language pack directories.
##############################################################

sub get_languagepack_file
{
    my ($filesref, $onedir) = @_;

    my $language = $onedir->{'specificlanguage'};
    my $foundfile = 0;
    my $onefile = "";

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];

        if ( $onefile->{'specificlanguage'} eq $onedir->{'specificlanguage'} )
        {
            $foundfile = 1;
            last;
        }
    }

    if ( ! $foundfile ) { installer::exiter::exit_program("ERROR: No file with correct language found (language pack build)!", "get_languagepack_file"); }

    return $onefile;
}

##############################################################
# Returning component for createfolder table.
##############################################################

sub get_createfolder_component
{
    my ($onedir, $filesref, $allvariableshashref) = @_;

    # Directories do not belong to a module.
    # Therefore they can only belong to the root module and
    # will be added to a component at the root module.
    # All directories will be added to the component
    # containing the file $allvariableshashref->{'GLOBALFILEGID'}

    if ( ! $allvariableshashref->{'GLOBALFILEGID'} ) { installer::exiter::exit_program("ERROR: GLOBALFILEGID must be defined in list file!", "get_createfolder_component"); }
    if (( $installer::globals::patch ) && ( ! $allvariableshashref->{'GLOBALFILEGID'} )) { installer::exiter::exit_program("ERROR: GLOBALPATCHFILEGID must be defined in list file!", "get_createfolder_component"); }

    my $globalfilegid = $allvariableshashref->{'GLOBALFILEGID'};
    if ( $installer::globals::patch ) { $globalfilegid = $allvariableshashref->{'GLOBALPATCHFILEGID'}; }

    my $onefile = "";
    if ( $installer::globals::languagepack ) { $onefile = get_languagepack_file($filesref, $onedir); }
    else { $onefile = installer::existence::get_specified_file($filesref, $globalfilegid); }

    return $onefile->{'componentname'};
}

####################################################################################
# Creating the file CreateFo.idt dynamically for creation of empty directories
# Content:
# Directory_ Component_
####################################################################################

sub create_createfolder_table
{
    my ($dirref, $filesref, $basedir, $allvariableshashref) = @_;

    my @createfoldertable = ();

    my $infoline;

    installer::windows::idtglobal::write_idt_header(\@createfoldertable, "createfolder");

    for ( my $i = 0; $i <= $#{$dirref}; $i++ )
    {
        my $onedir = ${$dirref}[$i];

        # language packs get only language dependent directories
        if (( $installer::globals::languagepack ) && ( $onedir->{'specificlanguage'} eq "" )) { next };

        my $styles = "";

        if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }

        if ( $styles =~ /\bCREATE\b/ )
        {
            my %directory = ();

            $directory{'Directory_'} = get_createfolder_directory($onedir);
            $directory{'Component_'} = get_createfolder_component($onedir, $filesref, $allvariableshashref);

            my $oneline = $directory{'Directory_'} . "\t" . $directory{'Component_'} . "\n";

            push(@createfoldertable, $oneline);
        }
    }

    # Saving the file

    my $createfoldertablename = $basedir . $installer::globals::separator . "CreateFo.idt";
    installer::files::save_file($createfoldertablename ,\@createfoldertable);
    $infoline = "Created idt file: $createfoldertablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;