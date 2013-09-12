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


package packager::files;

use packager::exiter;

############################################
# File Operations
############################################

sub check_file
{
    my ($arg) = @_;

    if(!( -f $arg ))
    {
        packager::exiter::exit_program("ERROR: Cannot find file $arg", "check_file");
    }
}

sub read_file
{
    my ($localfile) = @_;

    if ( ! open( IN, $localfile ) ) {
        # try again - sometimes we get errors caused by race conditions in parallel builds
        sleep 5;
        open( IN, $localfile ) or packager::exiter::exit_program("ERROR: Cannot open file: $localfile", "read_file");
    }
    my @localfile = <IN>;
    close( IN );

    return \@localfile;
}

###########################################
# Saving files
###########################################

sub save_file
{
    my ($savefile, $savecontent) = @_;
    open( OUT, ">$savefile" );
    print OUT @{$savecontent};
    close( OUT);
    if (! -f $savefile) { packager::exiter::exit_program("ERROR: Cannot write file: $savefile", "save_file"); }
}

######################################################
# Creating a new direcotory
######################################################

sub create_directory
{
    my ($directory) = @_;

    my $returnvalue = 1;

    if (!(-d $directory))
    {
        $returnvalue = mkdir($directory, 0775);

        if ($returnvalue)
        {
            $infoline = "\nCreated directory: $directory\n";
            push(@packager::globals::logfileinfo, $infoline);

            if ($packager::globals::isunix)
            {
                my $localcall = "chmod 775 $directory \>\/dev\/null 2\>\&1";
                system($localcall);
            }
        }
        else
        {
            packager::exiter::exit_program("ERROR: Could not create directory: $directory", "create_directory");
        }
    }
}

######################################################
# Removing a complete directory with subdirectories
######################################################

sub remove_complete_directory
{
    my ($directory) = @_;

    my @content = ();

    $directory =~ s/\Q$packager::globals::separator\E\s*$//;

    if ( -d $directory )
    {
        opendir(DIR, $directory);
        @content = readdir(DIR);
        closedir(DIR);

        my $oneitem;

        foreach $oneitem (@content)
        {
            if ((!($oneitem eq ".")) && (!($oneitem eq "..")))
            {
                my $item = $directory . $packager::globals::separator . $oneitem;

                if ( -f $item )     # deleting files
                {
                    unlink($item);
                }

                if ( -d $item )     # recursive
                {
                    remove_complete_directory($item, 0);
                }
            }
        }

        # try to remove empty directory

        rmdir $directory;

    }
}

1;
