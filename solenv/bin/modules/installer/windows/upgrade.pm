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



package installer::windows::upgrade;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

####################################################################################
# Creating the file Upgrade.idt dynamically
# Content:
# UpgradeCode VersionMin VersionMax Language Attributes Remove ActionProperty
####################################################################################

sub create_upgrade_table
{
    my ($basedir, $allvariableshashref) = @_;

    my @upgradetable = ();

    # fix for problematic OOo 1.9 versions
    my $include_ooo_fix = 0;
    my $ooomaxnew = "";
    if (($installer::globals::product =~ /^\s*OpenOffice/i ) && ( ! ( $installer::globals::product =~ /SDK/i )) && ( ! $installer::globals::languagepack ))
    {
        $include_ooo_fix = 1;
        $ooomaxnew = "34.0.0";
    }

    installer::windows::idtglobal::write_idt_header(\@upgradetable, "upgrade");

    # Setting also $installer::globals::msimajorproductversion, that is for example "3.0.0", to differ between old products for OOo 2.x and
    # older products from OOo 3.x. The latter must be removed always, the removal of the first is controlled with a checkbox.
    my $newline = $installer::globals::upgradecode . "\t" . "\t" . $installer::globals::msimajorproductversion . "\t" . "\t" . "0" . "\t" . "\t" . "OLDPRODUCTS" . "\n";
    push(@upgradetable, $newline);

    # Setting all products, that must be removed.
    $newline = $installer::globals::upgradecode . "\t" . $installer::globals::msimajorproductversion . "\t" . $installer::globals::msiproductversion . "\t" . "\t" . "257" . "\t" . "\t" . "OLDPRODUCTSSAMEMAJOR" . "\n";
    push(@upgradetable, $newline);

    if ( ! $installer::globals::patch )
    {
        # preventing downgrading
        $newline = $installer::globals::upgradecode . "\t" . $installer::globals::msiproductversion . "\t" . $ooomaxnew . "\t" . "\t" . "2" . "\t" . "\t" . "NEWPRODUCTS" . "\n";
        push(@upgradetable, $newline);

        $newline = $installer::globals::upgradecode . "\t" . $installer::globals::msiproductversion . "\t" . $ooomaxnew . "\t" . "\t" . "258" . "\t" . "\t" . "SAMEPRODUCTS" . "\n";
        push(@upgradetable, $newline);

        if ( $include_ooo_fix )
        {
            $newline = $installer::globals::upgradecode . "\t" . "35.0.0" . "\t" . "36.0.0" . "\t" . "\t" . "1" . "\t" . "\t" . "OLDPRODUCTS2" . "\n";
            push(@upgradetable, $newline);
        }

        # if (( $allvariableshashref->{'PATCHUPGRADECODE'} ) && ( ! $installer::globals::languagepack ))
        # {
        #   $newline = $allvariableshashref->{'PATCHUPGRADECODE'} . "\t" . "\t" . $installer::globals::msiproductversion . "\t" . "\t" . "1" . "\t" . "\t" . "OLDPRODUCTSPATCH" . "\n";
        #   push(@upgradetable, $newline);
        #
        #   $newline = $allvariableshashref->{'PATCHUPGRADECODE'} . "\t" . $installer::globals::msiproductversion . "\t" . "\t" . "\t" . "2" . "\t" . "\t" . "NEWPRODUCTSPATCH" . "\n";
        #   push(@upgradetable, $newline);
        #
        #   $newline = $allvariableshashref->{'PATCHUPGRADECODE'} . "\t" . $installer::globals::msiproductversion . "\t" . "\t" . "\t" . "258" . "\t" . "\t" . "SAMEPRODUCTSPATCH" . "\n";
        #   push(@upgradetable, $newline);
        # }

        # also searching for the beta

        if (( $allvariableshashref->{'BETAUPGRADECODE'} ) && ( ! $installer::globals::languagepack ))
        {
            $newline = $allvariableshashref->{'BETAUPGRADECODE'} . "\t" . "1.0" . "\t" . "\t" . "\t" . "1" . "\t" . "\t" . "BETAPRODUCTS" . "\n";
            push(@upgradetable, $newline);
        }

        # also searching for the stub

        if (( $allvariableshashref->{'STUBUPGRADECODE'} ) && ( ! $installer::globals::languagepack ))
        {
            $newline = $allvariableshashref->{'STUBUPGRADECODE'} . "\t" . "1.0" . "\t" . "\t" . "\t" . "1" . "\t" . "\t" . "STUBPRODUCTS" . "\n";
            push(@upgradetable, $newline);
        }

        # searching for all older patches and languagepacks (defined in a extra file)

        if (( $allvariableshashref->{'REMOVE_UPGRADE_CODE_FILE'} ) && ( ! $installer::globals::languagepack ))
        {
            my $filename = $allvariableshashref->{'REMOVE_UPGRADE_CODE_FILE'};
            my $langpackcodefilename = $installer::globals::idttemplatepath  . $installer::globals::separator . $filename;
            if ( ! -f $langpackcodefilename ) { installer::exiter::exit_program("ERROR: Could not find file \"$langpackcodefilename\".", "create_upgrade_table"); }

            my $filecontent = installer::files::read_file($langpackcodefilename);
            my $newlines = analyze_file_for_upgrade_table($filecontent);

            for ( my $i = 0; $i <= $#{$newlines}; $i++ ) { push(@upgradetable, ${$newlines}[$i]); }
        }
    }

    # No upgrade for Beta versions!

    if (( $allvariableshashref->{'PRODUCTEXTENSION'} eq "Beta" ) && ( ! $installer::globals::patch ) && ( ! $installer::globals::languagepack ))
    {
        @upgradetable = ();
        installer::windows::idtglobal::write_idt_header(\@upgradetable, "upgrade");
        $installer::logger::Lang->printf("Beta product -> empty Upgrade table\n");
    }

    # Saving the file

    my $upgradetablename = $basedir . $installer::globals::separator . "Upgrade.idt";
    installer::files::save_file($upgradetablename ,\@upgradetable);
    $installer::logger::Lang->printf("Created idt file: %s\n", $upgradetablename);
}

##############################################################
# Reading the file with UpgradeCodes of old products,
# that can be removed, if the user wants to remove them.
##############################################################

sub analyze_file_for_upgrade_table
{
    my ($filecontent) = @_;

    my @allnewlines = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        my $line = ${$filecontent}[$i];
        if ( $line =~ /^\s*$/ ) { next; } # empty lines can be ignored
        if ( $line =~ /^\s*\#/ ) { next; } # comment lines starting with a hash

        if ( $line =~ /^(.*)\t(.*)\t(.*)\t(.*)\t(.*)\t(.*)\t(.*)$/ ) { push(@allnewlines, $line); }
        else { installer::exiter::exit_program("ERROR: Wrong syntax in file for upgrade table", "analyze_file_for_upgrade_table"); }
    }

    return \@allnewlines;
}

1;
