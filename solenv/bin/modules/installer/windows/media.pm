#*************************************************************************
#
#   $RCSfile: media.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:19:42 $
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

package installer::windows::media;

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
# Returning the lastsequence for the media table.
##############################################################

sub get_media_lastsequence
{
    my ($fileref) = @_;

    return $fileref->{'sequencenumber'};
}

##############################################################
# Returning the diskprompt for the media table.
##############################################################

sub get_media_diskprompt
{
    return 1;
}

##############################################################
# Returning the cabinet file name for the media table.
##############################################################

sub get_media_cabinet
{
    my ($id) = @_;

    my $number = 1000 + $id;
    my $filename = "f_" . $number . ".cab";

    return $filename;
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

##############################################################
# Saving the cabinet file name in the files collector.
# This is useful for making a list to connect the
# source of each file with the destination cabinet file.
##############################################################

sub set_cabinetfilename_for_component_in_file_collector
{
    my ($cabinetfilename, $filesref, $componentname, $max) = @_;

    for ( my $i = 0; $i <= $max; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $component = $onefile->{'componentname'};

        if ( $component eq $componentname )
        {
            my $cabinet = "";

            if ( $onefile->{'cabinet'} ) { $cabinet = $onefile->{'cabinet'}; }

            if ( $cabinet eq "" )
            {
                $onefile->{'cabinet'} = $cabinetfilename;
            }
        }
    }
}

#################################################
# Creating the cab file name dynamically
#################################################

sub generate_cab_filename
{
    my ( $allvariables ) = @_;

    my $name = $allvariables->{'PRODUCTNAME'};

    $name = lc($name);
    $name =~ s/\.//g;
    $name =~ s/\s//g;

    $name = $name . ".cab";

    return $name;
}

#################################################################################
# Creating the file Media.idt dynamically
# Content:
# DiskId LastSequence DiskPrompt Cabinet VolumeLabel Source
# Idea: Every component is packed into each own cab file
#################################################################################

sub create_media_table
{
    my ($filesref, $basedir, $allvariables) = @_;

    my @mediatable = ();

    my $diskid = 0;

    installer::windows::idtglobal::write_idt_header(\@mediatable, "media");

    if ( $installer::globals::many_cab_files )
    {
        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            my $onefile = ${$filesref}[$i];
            my $nextfile = ${$filesref}[$i+1];

            my $filecomponent = "";
            my $nextcomponent = "";

            if ( $onefile->{'componentname'} ) { $filecomponent = $onefile->{'componentname'}; }
            if ( $nextfile->{'componentname'} ) { $nextcomponent = $nextfile->{'componentname'}; }

            if ( $filecomponent eq $nextcomponent )
            {
                next;       # nothing to do, this is not the last file of a component
            }

            my %media = ();
            $diskid++;

            $media{'DiskId'} = get_media_diskid($diskid);
            $media{'LastSequence'} = get_media_lastsequence($onefile);
            $media{'DiskPrompt'} = get_media_diskprompt();
            $media{'Cabinet'} = get_media_cabinet($diskid);
            $media{'VolumeLabel'} = get_media_volumelabel();
            $media{'Source'} = get_media_source();

            my $oneline = $media{'DiskId'} . "\t" . $media{'LastSequence'} . "\t" . $media{'DiskPrompt'} . "\t"
                    . $media{'Cabinet'} . "\t" . $media{'VolumeLabel'} . "\t" . $media{'Source'} . "\n";

            push(@mediatable, $oneline);

            set_cabinetfilename_for_component_in_file_collector($media{'Cabinet'}, $filesref, $filecomponent, $i);
        }
    }

    if ( $installer::globals::one_cab_file )
    {

        my %media = ();
        $diskid++;

        my $maximumfile = $#{$filesref};

        $media{'DiskId'} = get_media_diskid($diskid);
        $media{'LastSequence'} = ${$filesref}[$maximumfile]->{'sequencenumber'};    # sequence number of the last file
        $media{'DiskPrompt'} = get_media_diskprompt();
        $media{'Cabinet'} = generate_cab_filename($allvariables);
        $media{'VolumeLabel'} = get_media_volumelabel();
        $media{'Source'} = get_media_source();

        my $oneline = $media{'DiskId'} . "\t" . $media{'LastSequence'} . "\t" . $media{'DiskPrompt'} . "\t"
                    . $media{'Cabinet'} . "\t" . $media{'VolumeLabel'} . "\t" . $media{'Source'} . "\n";

        push(@mediatable, $oneline);

        # Saving the cabinet file name in the file collector

        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            my $onefile = ${$filesref}[$i];
            $onefile->{'cabinet'} = $media{'Cabinet'};
        }
    }

    # Saving the file

    my $mediatablename = $basedir . $installer::globals::separator . "Media.idt";
    installer::files::save_file($mediatablename ,\@mediatable);
    my $infoline = "Created idt file: $mediatablename\n";
    push(@installer::globals::logfileinfo, $infoline);
}

1;