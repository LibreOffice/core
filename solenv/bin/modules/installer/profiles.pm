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

package installer::profiles;

use installer::converter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::remover;
use installer::systemactions;

#############################
# Profiles
#############################

#######################################################
# Sorting the content of a profile
#######################################################

sub sorting_profile {
    my ($profilesref) = @_;

    my @sections;
    my %section_content;

    for ( my $i = 0; $i < @{$profilesref}; $i++ ) {
        my $line = ${$profilesref}[$i];

        # Skip unless this is a section (every second line)
        next unless ( $line =~ /^\s*(\[.*\])\s*$/ );

        my $section = $1;
        my $next_line = ${$profilesref}[$i+1];

        if ( ! exists $section_content{$section} ) {
            push @sections, $section;
        }

        push @{ $section_content{$section} }, $next_line;
    }

    my @profile;

    for my $section (@sections) {
        push @profile, "$section\n";
        push @profile, @{ $section_content{$section} };
    }

    return \@profile;
}

#####################################################################
# Adding the newly created profile into the file list
#####################################################################

sub add_profile_into_filelist
{
    my ($filesarrayref, $oneprofile, $completeprofilename, $allvariables) = @_;

    my %profile = ();

    # Taking the base data from the "gid_File_Lib_Oox"

    my $vclgid = "gid_File_Lib_Oox";
    if ( $allvariables->{'GLOBALFILEGID'} ) { $vclgid = $allvariables->{'GLOBALFILEGID'}; }
    my ($vclfile) = grep {$_->{gid} eq $vclgid} @{$filesarrayref};
    if (! defined $vclfile) {
        die "Could not find file $vclgid in list of files!";
    }

    # copying all base data
    installer::converter::copy_item_object($vclfile, \%profile);

    # and overriding all new values

    $profile{'ismultilingual'} = 0;
    $profile{'sourcepath'} = $completeprofilename;
    $profile{'Name'} = $oneprofile->{'Name'};
    $profile{'UnixRights'} = "644";
    $profile{'gid'} = $oneprofile->{'gid'};
    $profile{'Dir'} = $oneprofile->{'Dir'};
    $profile{'destination'} = $oneprofile->{'destination'};
    $profile{'Styles'} = "";
    if ( $oneprofile->{'Styles'} ) { $profile{'Styles'} = $oneprofile->{'Styles'}; }
    $profile{'modules'} = $oneprofile->{'ModuleID'};    # Profiles can only be added completely to a module

    push(@{$filesarrayref}, \%profile);
}

###################################################
# Including Windows line ends in ini files
# Profiles on Windows shall have \r\n line ends
###################################################

sub include_windows_lineends
{
    my ($onefile) = @_;

    for ( my $i = 0; $i <= $#{$onefile}; $i++ )
    {
        ${$onefile}[$i] =~ s/\r?\n$/\r\n/;
    }
}

####################################
# Create profiles
####################################

sub create_profiles
{
    my ($profilesref, $profileitemsref, $filesarrayref, $languagestringref, $allvariables) = @_;

    my $infoline;

    my $profilesdir = installer::systemactions::create_directories("profiles", $languagestringref);

    installer::logger::include_header_into_logfile("Creating profiles:");

    # Attention: The module dependencies from ProfileItems have to be ignored, because
    # the Profile has to be installed completely with all of its content and the correct name.
    # Only complete profiles can belong to a specified module, but not ProfileItems!

    # iterating over all files

    for ( my $i = 0; $i <= $#{$profilesref}; $i++ )
    {
        my $oneprofile = ${$profilesref}[$i];
        my $dir = $oneprofile->{'Dir'};
        if ( $dir eq "PREDEFINED_CONFIGDIR" ) { next; }     # ignoring the profile sversion file

        my $profilegid = $oneprofile->{'gid'};
        my $profilename = $oneprofile->{'Name'};

        my $localprofilesdir = $profilesdir . $installer::globals::separator . $profilegid; # uniqueness guaranteed by gid
        if ( ! -d $localprofilesdir ) { installer::systemactions::create_directory($localprofilesdir); }

        my @onefile = ();
        my $profileempty = 1;

        for ( my $j = 0; $j <= $#{$profileitemsref}; $j++ )
        {
            my $oneprofileitem = ${$profileitemsref}[$j];

            my $styles = "";
            if ( $oneprofileitem->{'Styles'} ) { $styles = $oneprofileitem->{'Styles'}; }
            if ( $styles =~ /\bINIFILETABLE\b/ ) { next; }  # these values are written during installation, not during packing

            my $profileid = $oneprofileitem->{'ProfileID'};

            if ( $profileid eq $profilegid )
            {
                my $section = $oneprofileitem->{'Section'};
                my $key = $oneprofileitem->{'Key'};
                my $value = $oneprofileitem->{'Value'};
                for (my $pk = 1; $pk <= 50; $pk++)
                {
                    my $key = "ValueList" . $pk;
                    if ( $oneprofileitem->{$key} )
                        { $value = $value . " " . $oneprofileitem->{$key} }
                }
                my $order = $oneprofileitem->{'Order'}; # ignoring order at the moment

                my $line = "[" . $section . "]" . "\n";
                push(@onefile, $line);
                $line = $key . "=" . $value . "\n";
                push(@onefile, $line);

                $profileempty = 0;
            }
        }

        if ( $profileempty ) { next; }  # ignoring empty profiles

        # Sorting the array @onefile
        my $onefileref = sorting_profile(\@onefile);

        if ( $installer::globals::iswin && $^O =~ /cygwin/i)      # Windows line ends only for Cygwin
        {
            include_windows_lineends($onefileref);
        }

        # Saving the profile as a file
        $completeprofilename = $localprofilesdir . $installer::globals::separator . $profilename;

        installer::files::save_file($completeprofilename, $onefileref);

        # Adding the file to the filearray
        # Some data are set now, others are taken from the file "soffice.exe" ("soffice.bin")
        add_profile_into_filelist($filesarrayref, $oneprofile, $completeprofilename, $allvariables);

        $infoline = "Created Profile: $completeprofilename\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);
}


1;
