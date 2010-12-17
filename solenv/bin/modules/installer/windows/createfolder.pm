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
    elsif ( $installer::globals::helppack ) { $onefile = installer::existence::get_specified_file($filesref, 'gid_File_Help_Common_Zip'); }
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

        # language packs and help packs get only language dependent directories
        if (( $installer::globals::languagepack ) ||  ( $installer::globals::languagepack ) && ( $onedir->{'specificlanguage'} eq "" )) { next };

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
