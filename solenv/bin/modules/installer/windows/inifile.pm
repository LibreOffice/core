#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



package installer::windows::inifile;

use installer::existence;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

use strict;

####################################################
# Setting the profile for a special profileitem
####################################################

sub get_profile_for_profileitem
{
    my ($profileid, $filesref) = @_;

    my $profile = installer::existence::get_specified_file($filesref, $profileid);

    return $profile;
}

####################################################
# Checking whether profile is included in patch
####################################################

sub profile_has_patch_flag
{
    my ($profile) = @_;

    my $in_patch = 0;

    my $styles = "";
    if ( $profile->{'Styles'} ) { $styles = $profile->{'Styles'}; }
    if ( $styles =~ /\bPATCH\b/ ) { $in_patch = 1; }

    return $in_patch;
}

####################################################
# Checking whether profile is part of product
####################################################

sub file_is_part_of_product
{
    my ($profilegid, $filesref) = @_;

    foreach my $file (@$filesref)
    {
        my $filegid = $file->{'gid'};
        next unless defined $filegid;

        if ( $filegid eq $profilegid )
        {
            return 1;
        }
    }

    return 0;
}

###########################################################################################################
# Creating the file IniFile.idt dynamically
# Content:
# IniFile\tFileName\tDirProperty\tSection\tKey\tValue\tAction\tComponent_
###########################################################################################################

sub create_inifile_table
{
    my ($inifiletableentries, $filesref, $basedir) = @_;

    my @inifiletable = ();

    installer::windows::idtglobal::write_idt_header(\@inifiletable, "inifile");

    for ( my $i = 0; $i <= $#{$inifiletableentries}; $i++ )
    {
        my $profileitem = ${$inifiletableentries}[$i];

        my $profileid = $profileitem->{'ProfileID'};

        # Is this profile part of the product? This is not sure, for example in patch process.
        # If the profile is not part of the product, this ProfileItem must be ignored.

        if ( ! file_is_part_of_product($profileid, $filesref) ) { next; }

        my $profile = get_profile_for_profileitem($profileid, $filesref);

        if (( $installer::globals::patch ) && ( ! profile_has_patch_flag($profile) )) { next; }

        my %inifile = ();

        $inifile{'IniFile'} = $profileitem->{'Inifiletablekey'};
        $inifile{'FileName'} = $profile->{'Name'};
        $inifile{'DirProperty'} = $profile->{'uniquedirname'};
        $inifile{'Section'} = $profileitem->{'Section'};
        $inifile{'Key'} = $profileitem->{'Key'};
        $inifile{'Value'} = $profileitem->{'Value'};
        $inifile{'Action'} = $profileitem->{'Inifiletableaction'};
        $inifile{'Component_'} = $profile->{'componentname'};

        my $oneline = $inifile{'IniFile'} . "\t" . $inifile{'FileName'} . "\t" . $inifile{'DirProperty'} . "\t"
                 . $inifile{'Section'} . "\t" . $inifile{'Key'} . "\t" . $inifile{'Value'} . "\t"
                 . $inifile{'Action'} . "\t" . $inifile{'Component_'} . "\n";

        push(@inifiletable, $oneline);
    }

    # Saving the file

    my $inifiletablename = $basedir . $installer::globals::separator . "IniFile.idt";
    installer::files::save_file($inifiletablename ,\@inifiletable);
    $installer::logger::Lang->printf("Created idt file: %s\n", $inifiletablename);
}

1;
