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

package installer::windows::createfolder;

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
    # $allvariableshashref->{'ROOTMODULEGID'}

    if ( ! $allvariableshashref->{'ROOTMODULEGID'} ) { installer::exiter::exit_program("ERROR: ROOTMODULEGID must be defined in list file!", "get_createfolder_component"); }

    my $rootmodulegid = $allvariableshashref->{'ROOTMODULEGID'};

    my $onefile;
    if ( $installer::globals::languagepack ) { $onefile = get_languagepack_file($filesref, $onedir); }
    elsif ( $installer::globals::helppack ) { ($onefile) = grep {$_->{gid} eq 'gid_File_Help_Common_Zip'} @{$filesref} }
    else {
        foreach my $file (@{$filesref}) {
            if ($file->{'modules'} eq $rootmodulegid)
            {
                $onefile = $file;
                last;
            }
        }
    }

    if (! defined $onefile) {
        installer::exiter::exit_program("ERROR: Could not find file!", "get_createfolder_component");
    }

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
