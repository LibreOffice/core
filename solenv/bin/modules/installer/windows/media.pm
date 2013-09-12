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

package installer::windows::media;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

##############################################################
# Returning the diskid for the media table.
##############################################################

sub get_media_diskid
{
    my ($id) = @_;

    return $id;
}

##############################################################
# Returning the diskprompt for the media table.
##############################################################

sub get_media_diskprompt
{
    return 1;
}

##############################################################
# Returning the volumelabel for the media table.
##############################################################

sub get_media_volumelabel
{
    return "DISK1";
}

##############################################################
# Returning the source for the media table.
##############################################################

sub get_media_source
{
    return "";
}

sub get_maximum_filenumber
{
    my ($allfiles, $maxcabfilenumber) = @_;

    my $maxfile = 0;

    while ( ! ( $allfiles%$maxcabfilenumber == 0 ))
    {
        $allfiles++;
    }

    $maxfile = $allfiles / $maxcabfilenumber;

    $maxfile++;                 # for securitry

    return $maxfile;
}

#################################################################################
# Creating the file Media.idt dynamically
# Content:
# DiskId LastSequence DiskPrompt Cabinet VolumeLabel Source
# Idea: Every component is packed into each own cab file
#################################################################################

sub create_media_table
{
    my ($filesref, $basedir, $allvariables, $alludpatelastsequences, $allupdatediskids) = @_;

    my @mediatable = ();

    my $diskid = 0;

    installer::windows::idtglobal::write_idt_header(\@mediatable, "media");

    if ( $installer::globals::fix_number_of_cab_files )
    {
        # number of cabfiles
        my $maxcabfilenumber = $installer::globals::number_of_cabfiles;
        if ( $allvariables->{'CABFILENUMBER'} ) { $maxcabfilenumber = $allvariables->{'CABFILENUMBER'}; }
        my $allfiles = $#{$filesref} + 1;
        my $maxfilenumber = get_maximum_filenumber($allfiles, $maxcabfilenumber);
        my $cabfilenumber = 0;
        my $cabfull = 0;
        my $counter = 0;

        # Sorting of files collector files required !
        # Attention: The order in the cab file is not guaranteed (especially in udpate process)

        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            if (( $counter >= $maxfilenumber ) || ( $i == $#{$filesref} )) { $cabfull = 1; }

            $counter++;      # counting the files in the cab file

            my $onefile = ${$filesref}[$i];
            my $nextfile = ${$filesref}[$i+1];

            my $filecomponent = "";
            my $nextcomponent = "";

            if ( $onefile->{'componentname'} ) { $filecomponent = $onefile->{'componentname'}; }
            if ( $nextfile->{'componentname'} ) { $nextcomponent = $nextfile->{'componentname'}; }

            if ( $filecomponent eq $nextcomponent ) # all files of one component have to be in one cab file
            {
                next;       # nothing to do, this is not the last file of a component
            }

            if ( $cabfull )
            {
                my %media = ();
                $cabfilenumber++;

                $media{'DiskId'} = get_media_diskid($cabfilenumber);
                $media{'LastSequence'} = $i + 1;    # This should be correct, also for unsorted files collectors
                $media{'DiskPrompt'} = get_media_diskprompt();
                $media{'Cabinet'} = generate_cab_filename_for_some_cabs($allvariables, $cabfilenumber);
                $media{'VolumeLabel'} = get_media_volumelabel();
                $media{'Source'} = get_media_source();

                my $oneline = $media{'DiskId'} . "\t" . $media{'LastSequence'} . "\t" . $media{'DiskPrompt'} . "\t"
                        . $media{'Cabinet'} . "\t" . $media{'VolumeLabel'} . "\t" . $media{'Source'} . "\n";

                push(@mediatable, $oneline);

                # Saving the cabinet file name in the file collector

                $media{'Cabinet'} =~ s/^\s*\#//;    # removing leading hash

                for ( my $j = 0; $j <= $i; $j++ )
                {
                    my $onefile = ${$filesref}[$j];
                    if ( ! $onefile->{'cabinet'} ) { $onefile->{'cabinet'} = $media{'Cabinet'}; }
                }

                $cabfull = 0;
                $counter = 0;
            }
        }
    }
    else
    {
        installer::exiter::exit_program("ERROR: No cab file specification in globals.pm !", "create_media_table");
    }

    # Saving the file

    my $mediatablename = $basedir . $installer::globals::separator . "Media.idt";
    installer::files::save_file($mediatablename ,\@mediatable);
    my $infoline = "Created idt file: $mediatablename\n";
    push(@installer::globals::logfileinfo, $infoline);
}

1;
