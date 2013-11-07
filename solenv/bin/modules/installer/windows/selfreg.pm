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



package installer::windows::selfreg;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::worker;
use installer::windows::idtglobal;

##############################################################
# Returning the cost for the selfreg table.
##############################################################

sub get_selfreg_cost
{
    my ( $onefile ) = @_;

    return "0";
}

####################################################################################
# Creating the file SelfReg.idt dynamically
# Content:
# File_ Cost
# UpgradeCode VersionMin VersionMax Language Attributes Remove ActionProperty
####################################################################################

sub create_selfreg_table
{
    my ($filesref, $basedir) = @_;

    my @selfregtable = ();

    installer::windows::idtglobal::write_idt_header(\@selfregtable, "selfreg");

    # Registering all libraries with flag "SELFREG"

    my $selfregfiles = installer::worker::collect_all_items_with_special_flag($filesref, "SELFREG");

    for ( my $i = 0; $i <= $#{$selfregfiles}; $i++ )
    {
        my $onefile = ${$selfregfiles}[$i];

        my %selfreg = ();

        $selfreg{'File_'} = $onefile->{'uniquename'};
        $selfreg{'Cost'} = get_selfreg_cost($onefile);

        my $oneline = $selfreg{'File_'} . "\t" . $selfreg{'Cost'} . "\n";

        push(@selfregtable, $oneline);
    }

    # Saving the file

    my $selfregtablename = $basedir . $installer::globals::separator . "SelfReg.idt";
    installer::files::save_file($selfregtablename ,\@selfregtable);
    $installer::logger::Lang->printf("Created idt file: %s\n", $selfregtablename);
}

1;
