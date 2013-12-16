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

package installer::windows::binary;

use installer::existence;
use installer::files;
use installer::globals;

use strict;

###########################################################################################################
# Updating the table Binary dynamically with all files from $binarytablefiles
# Content:
# Name  Data
# s72   v0
# Binary    Name
###########################################################################################################

sub update_binary_table
{
    my ($languageidtdir, $filesref, $binarytablefiles) = @_;

    my $binaryidttablename = $languageidtdir . $installer::globals::separator . "Binary.idt";
    my $binaryidttable = installer::files::read_file($binaryidttablename);

    # Only the iconfiles, that are used in the shortcut table for the
    # FolderItems (entries in Windows startmenu) are added into the icon table.

    foreach my $binaryfile (@$binarytablefiles)
    {
        my $binaryfilename = $binaryfile->{'Name'};
        my $binaryfiledata = $binaryfilename;

        # removing "." in filename: "abc.dll" to "abcdll" in name column
        $binaryfilename =~ s/\.//g;

        push @$binaryidttable, $binaryfilename . "\t" . $binaryfiledata . "\n";
    }

    # Saving the file

    installer::files::save_file($binaryidttablename ,$binaryidttable);
    $installer::logger::Lang->printf("Updated idt file: %s\n", $binaryidttablename);
}

1;
