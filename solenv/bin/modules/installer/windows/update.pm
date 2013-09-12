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

package installer::windows::update;

use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::pathanalyzer;
use installer::systemactions;

#################################################################################
# Extracting all tables from an msi database
#################################################################################

sub extract_all_tables_from_msidatabase
{
    my ($fulldatabasepath, $workdir) = @_;

    my $msidb = "msidb.exe";    # Has to be in the path
    my $infoline = "";
    my $systemcall = "";
    my $returnvalue = "";
    my $extraslash = "";        # Has to be set for non-ActiveState perl

    # Export of all tables by using "*"

    if ( $^O =~ /cygwin/i ) {
        # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
        $fulldatabasepath =~ s/\//\\\\/g;
        $workdir =~ s/\//\\\\/g;
        $extraslash = "\\";
    }
    if ( $^O =~ /linux/i) {
        $extraslash = "\\";
    }

    $systemcall = $msidb . " -d " . $fulldatabasepath . " -f " . $workdir . " -e " . $extraslash . "*";
    $returnvalue = system($systemcall);

    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $systemcall !\n";
        push( @installer::globals::logfileinfo, $infoline);
        installer::exiter::exit_program("ERROR: Could not exclude tables from msi database: $fulldatabasepath !", "extract_all_tables_from_msidatabase");
    }
    else
    {
        $infoline = "Success: Executed $systemcall successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

#################################################################################
# Collecting the keys from the first line of the idt file
#################################################################################

sub collect_all_keys
{
    my ($line) = @_;

    my @allkeys = ();
    my $rownumber = 0;
    my $onekey = "";

    while ( $line =~ /^\s*(\S+?)\t(.*)$/ )
    {
        $onekey = $1;
        $line = $2;
        $rownumber++;
        push(@allkeys, $onekey);
    }

    # and the last key

    $onekey = $line;
    $onekey =~ s/^\s*//g;
    $onekey =~ s/\s*$//g;

    $rownumber++;
    push(@allkeys, $onekey);

    return (\@allkeys, $rownumber);
}

#################################################################################
# Analyzing the content of one line of an idt file
#################################################################################

sub get_oneline_hash
{
    my ($line, $allkeys, $rownumber) = @_;

    my $counter = 0;
    my %linehash = ();

    $line =~ s/^\s*//;
    $line =~ s/\s*$//;

    my $value = "";
    my $onekey = "";

    while ( $line =~ /^(.*?)\t(.*)$/ )
    {
        $value = $1;
        $line = $2;
        $onekey = ${$allkeys}[$counter];
        $linehash{$onekey} = $value;
        $counter++;
    }

    # the last column

    $value = $line;
    $onekey = ${$allkeys}[$counter];

    $linehash{$onekey} = $value;

    return \%linehash;
}

#################################################################################
# Analyzing the content of an idt file
#################################################################################

sub analyze_idt_file
{
    my ($filecontent) = @_;

    my %table = ();
    # keys are written in first line
    my ($allkeys, $rownumber) = collect_all_keys(${$filecontent}[0]);

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if (( $i == 0 ) || ( $i == 1 ) || ( $i == 2 )) { next; }

        my $onelinehash = get_oneline_hash(${$filecontent}[$i], $allkeys, $rownumber);
        my $linekey = $i - 2;  # ! : The linenumber is the unique key !? Always decrease by two, because of removed first three lines.
        $table{$linekey} = $onelinehash;
    }

    return \%table;
}

#################################################################################
# Reading all idt files in a specified directory
#################################################################################

sub read_all_tables_from_msidatabase
{
    my ($workdir) = @_;

    my %database = ();

    my $ext = "idt";

    my $allidtfiles = installer::systemactions::find_file_with_file_extension($ext, $workdir);

    for ( my $i = 0; $i <= $#{$allidtfiles}; $i++ )
    {
        my $onefilename = ${$allidtfiles}[$i];
        my $longonefilename = $workdir . $installer::globals::separator . $onefilename;
        if ( ! -f $longonefilename ) { installer::exiter::exit_program("ERROR: Could not find idt file: $longonefilename!", "read_all_tables_from_msidatabase"); }
        my $filecontent = installer::files::read_file($longonefilename);
        my $idtcontent = analyze_idt_file($filecontent);
        my $key = $onefilename;
        $key =~ s/\.idt\s*$//;
        $database{$key} = $idtcontent;
    }

    return \%database;
}

#################################################################################
# Checking, if this is the correct database.
#################################################################################

sub correct_database
{
    my ($product, $pro, $langs, $languagestringref) = @_;

    my $correct_database = 0;

    # Comparing $product with $installer::globals::product and
    # $pro with $installer::globals::pro and
    # $langs with $languagestringref

    my $product_is_good = 0;

    my $localproduct = $installer::globals::product;
    if ( $installer::globals::languagepack ) { $localproduct = $localproduct . "LanguagePack"; }
    elsif ( $installer::globals::helppack ) { $localproduct = $localproduct . "HelpPack"; }

    if ( $product eq $localproduct ) { $product_is_good = 1; }

    if ( $product_is_good )
    {
        my $pro_is_good = 0;

        if ((( $pro eq "pro" ) && ( $installer::globals::pro )) || (( $pro eq "nonpro" ) && ( ! $installer::globals::pro ))) { $pro_is_good = 1; }

        if ( $pro_is_good )
        {
            my $langlisthash = installer::converter::convert_stringlist_into_hash(\$langs, ",");
            my $langstringhash = installer::converter::convert_stringlist_into_hash($languagestringref, "_");

            my $not_included = 0;
            foreach my $onelang ( keys %{$langlisthash} )
            {
                if ( ! exists($langstringhash->{$onelang}) )
                {
                    $not_included = 1;
                    last;
                }
            }

            if ( ! $not_included )
            {
                foreach my $onelanguage ( keys %{$langstringhash} )
                {
                    if ( ! exists($langlisthash->{$onelanguage}) )
                    {
                        $not_included = 1;
                        last;
                    }
                }

                if ( ! $not_included ) { $correct_database = 1; }
            }
        }
    }

    return $correct_database;
}

#################################################################################
# Searching for the path to the reference database for this special product.
#################################################################################

sub get_databasename_from_list
{
    my ($filecontent, $languagestringref, $filename) = @_;

    my $databasepath = "";

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        my $line = ${$filecontent}[$i];
        if ( $line =~ /^\s*$/ ) { next; } # empty line
        if ( $line =~ /^\s*\#/ ) { next; } # comment line

        if ( $line =~ /^\s*(.+?)\s*\t+\s*(.+?)\s*\t+\s*(.+?)\s*\t+\s*(.+?)\s*$/ )
        {
            my $product = $1;
            my $pro = $2;
            my $langs = $3;
            my $path = $4;

            if (( $pro ne "pro" ) && ( $pro ne "nonpro" )) { installer::exiter::exit_program("ERROR: Wrong syntax in file: $filename. Only \"pro\" or \"nonpro\" allowed in column 1! Line: \"$line\"", "get_databasename_from_list"); }

            if ( correct_database($product, $pro, $langs, $languagestringref) )
            {
                $databasepath = $path;
                last;
            }
        }
        else
        {
            installer::exiter::exit_program("ERROR: Wrong syntax in file: $filename! Line: \"$line\"", "get_databasename_from_list");
        }
    }

    return $databasepath;
}

#################################################################################
# Reading an existing database completely
#################################################################################

sub readdatabase
{
    my ($allvariables, $languagestringref, $includepatharrayref) = @_;

    my $database = "";
    my $infoline = "";

    if ( ! $allvariables->{'UPDATE_DATABASE_LISTNAME'} ) { installer::exiter::exit_program("ERROR: If \"UPDATE_DATABASE\" is set, \"UPDATE_DATABASE_LISTNAME\" is required.", "Main"); }
    my $listfilename = $allvariables->{'UPDATE_DATABASE_LISTNAME'};

    # Searching the list in the include paths
    my $listname = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$listfilename, $includepatharrayref, 1);
    if ( $$listname eq "" ) { installer::exiter::exit_program("ERROR: List file not found: $listfilename !", "readdatabase"); }
    my $completelistname = $$listname;

    # Reading list file
    my $listfile = installer::files::read_file($completelistname);

    # Get name and path of reference database
    my $databasename = get_databasename_from_list($listfile, $languagestringref, $completelistname);

    # If the correct database was not found, this is not necessarily an error. But in this case, this is not an update packaging process!
    if (( $databasename ) && ( $databasename ne "" )) # This is an update packaging process!
    {
        $installer::globals::updatedatabase = 1;
        installer::logger::print_message( "... update process, using database $databasename ...\n" );
        $infoline = "\nDatabase found in $completelistname: \"$databasename\"\n\n";
        # Saving in global variable
        $installer::globals::updatedatabasepath = $databasename;
    }
    else
    {
        $infoline = "\nNo database found in $completelistname. This is no update process!\n\n";
    }
    push( @installer::globals::logfileinfo, $infoline);

    if ( $installer::globals::updatedatabase )
    {
        if ( ! -f $databasename ) { installer::exiter::exit_program("ERROR: Could not find reference database: $databasename!", "readdatabase"); }

        my $msifilename = $databasename;
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$msifilename);

        installer::logger::include_timestamp_into_logfile("Performance Info: readdatabase start");

        # create directory for unpacking
        my $databasedir = installer::systemactions::create_directories("database", $languagestringref);

        # copy database
        my $fulldatabasepath = $databasedir . $installer::globals::separator . $msifilename;
        installer::systemactions::copy_one_file($databasename, $fulldatabasepath);

        installer::logger::include_timestamp_into_logfile("Performance Info: readdatabase: before extracting tables");

        # extract all tables from database
        extract_all_tables_from_msidatabase($fulldatabasepath, $databasedir);

        installer::logger::include_timestamp_into_logfile("Performance Info: readdatabase: before reading tables");

        # read all tables
        $database = read_all_tables_from_msidatabase($databasedir);

        # Test output:

        #   foreach my $key1 ( keys %{$database} )
        #   {
        #       print "Test1: $key1\n";
        #       foreach my $key2 ( keys %{$database->{$key1}} )
        #       {
        #           print "\tTest2: $key2\n";
        #           foreach my $key3 ( keys %{$database->{$key1}->{$key2}} )
        #           {
        #               print "\t\tTest3: $key3: $database->{$key1}->{$key2}->{$key3}\n";
        #           }
        #       }
        #   }

        # Example: File table

        # my $filetable = $database->{'File'};
        # foreach my $linenumber ( keys  %{$filetable} )
        # {
        #   print "Test Filenumber: $linenumber\n";
        #   foreach my $key ( keys %{$filetable->{$linenumber}} )
        #   {
        #       print "\t\tTest: $key: $filetable->{$linenumber}->{$key}\n";
        #   }
        # }

        # Example: Searching for ProductCode in table Property

        # my $column1 = "Property";
        # my $column2 = "Value";
        # my $searchkey = "ProductCode";
        # my $propertytable = $database->{'Property'};
        # foreach my $linenumber ( keys  %{$propertytable} )
        # {
        #   if ( $propertytable->{$linenumber}->{$column1} eq $searchkey )
        #   {
        #       print("Test: $searchkey : $propertytable->{$linenumber}->{$column2}\n");
        #   }
        # }

        installer::logger::include_timestamp_into_logfile("Performance Info: readdatabase end");
    }

    return $database;
}

#################################################################################
# Files can be included in merge modules. This is also important for update.
#################################################################################

sub readmergedatabase
{
    my ( $mergemodules, $languagestringref, $includepatharrayref ) = @_;

    installer::logger::include_timestamp_into_logfile("Performance Info: readmergedatabase start");

    my $mergemoduledir = installer::systemactions::create_directories("mergedatabase", $languagestringref);

    my %allmergefiles = ();

    foreach my $mergemodule ( @{$mergemodules} )
    {
        my $filename = $mergemodule->{'Name'};
        my $mergefile = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 1);

        if ( $$mergefile eq "" ) { installer::exiter::exit_program("ERROR: msm file not found: $filename !", "readmergedatabase"); }
        my $completesource = $$mergefile;

        my $mergegid = $mergemodule->{'gid'};
        my $workdir = $mergemoduledir . $installer::globals::separator . $mergegid;
        if ( ! -d $workdir ) { installer::systemactions::create_directory($workdir); }

        my $completedest = $workdir . $installer::globals::separator . $filename;
        installer::systemactions::copy_one_file($completesource, $completedest);
        if ( ! -f $completedest ) { installer::exiter::exit_program("ERROR: msm file not found: $completedest !", "readmergedatabase"); }

        # extract all tables from database
        extract_all_tables_from_msidatabase($completedest, $workdir);

        # read all tables
        my $onemergefile = read_all_tables_from_msidatabase($workdir);

        $allmergefiles{$mergegid} = $onemergefile;
    }

    foreach my $mergefilegid ( keys %allmergefiles )
    {
        my $onemergefile = $allmergefiles{$mergefilegid};
        my $filetable = $onemergefile->{'File'};

        foreach my $linenumber ( keys %{$filetable} )
        {
            # Collecting all files from merge modules in global hash
            $installer::globals::mergemodulefiles{$filetable->{$linenumber}->{'File'}} = 1;
        }
    }

    installer::logger::include_timestamp_into_logfile("Performance Info: readmergedatabase end");
}

#################################################################################
# Creating several useful hashes from old database
#################################################################################

sub create_database_hashes
{
    my ( $database ) = @_;

    # 1. Hash ( Component -> UniqueFileName ), required in File table.
    # Read from File table.

    my %uniquefilename = ();
    my %allupdatesequences = ();
    my %allupdatecomponents = ();
    my %allupdatefileorder = ();
    my %allupdatecomponentorder = ();
    my %revuniquefilename = ();
    my %revshortfilename = ();
    my %shortdirname = ();
    my %componentid = ();
    my %componentidkeypath = ();
    my %alloldproperties = ();
    my %allupdatelastsequences = ();
    my %allupdatediskids = ();

    my $filetable = $database->{'File'};

    foreach my $linenumber ( keys  %{$filetable} )
    {
        my $comp = $filetable->{$linenumber}->{'Component_'};
        my $uniquename = $filetable->{$linenumber}->{'File'};
        my $filename = $filetable->{$linenumber}->{'FileName'};
        my $sequence = $filetable->{$linenumber}->{'Sequence'};

        my $shortname = "";
        if ( $filename =~ /^\s*(.*?)\|\s*(.*?)\s*$/ )
        {
            $shortname = $1;
            $filename = $2;
        }

        # unique is the combination of $component and $filename
        my $key = "$comp/$filename";

        if ( exists($uniquefilename{$key}) ) { installer::exiter::exit_program("ERROR: Component/FileName \"$key\" is not unique in table \"File\" !", "create_database_hashes"); }

        my $value = $uniquename;
        if ( $shortname ne "" ) { $value = "$uniquename;$shortname"; }
        $uniquefilename{$key} = $value; # saving the unique keys and short names in hash

        # Saving reverse keys too
        $revuniquefilename{$uniquename} = $key;
        if ( $shortname ne "" ) { $revshortfilename{$shortname} = $key; }

        # Saving Sequences for unique names (and also components)
        $allupdatesequences{$uniquename} = $sequence;
        $allupdatecomponents{$uniquename} = $comp;

        # Saving unique names and components for sequences
        $allupdatefileorder{$sequence} = $uniquename;
        $allupdatecomponentorder{$sequence} = $comp;
    }

    # 2. Hash, required in Directory table.

    my $dirtable = $database->{'Directory'};

    foreach my $linenumber ( keys  %{$dirtable} )
    {
        my $dir = $dirtable->{$linenumber}->{'Directory'}; # this is a unique name
        my $defaultdir = $dirtable->{$linenumber}->{'DefaultDir'};

        my $shortname = "";
        if ( $defaultdir =~ /^\s*(.*?)\|\s*(.*?)\s*$/ )
        {
            $shortname = $1;
            $shortdirname{$dir} = $shortname;   # collecting only the short names
        }
    }

    # 3. Hash, collecting info from Component table.
    # ComponentID and KeyPath have to be reused.

    my $comptable = $database->{'Component'};

    foreach my $linenumber ( keys  %{$comptable} )
    {
        my $comp = $comptable->{$linenumber}->{'Component'};
        my $compid = $comptable->{$linenumber}->{'ComponentId'};
        my $keypath = $comptable->{$linenumber}->{'KeyPath'};

        $componentid{$comp} = $compid;
        $componentidkeypath{$comp} = $keypath;
    }

    # 4. Hash, property table, required for ProductCode and Installlocation.

    my $proptable = $database->{'Property'};

    foreach my $linenumber ( keys  %{$proptable} )
    {
        my $prop = $proptable->{$linenumber}->{'Property'};
        my $value = $proptable->{$linenumber}->{'Value'};

        $alloldproperties{$prop} = $value;
    }

    # 5. Media table, getting last sequence

    my $mediatable = $database->{'Media'};
    $installer::globals::updatelastsequence = 0;

    foreach my $linenumber ( keys  %{$mediatable} )
    {
        my $cabname = $mediatable->{$linenumber}->{'Cabinet'};
        my $lastsequence = $mediatable->{$linenumber}->{'LastSequence'};
        my $diskid = $mediatable->{$linenumber}->{'DiskId'};
        $allupdatelastsequences{$cabname} = $lastsequence;
        $allupdatediskids{$cabname} = $diskid;

        if ( $lastsequence > $installer::globals::updatelastsequence ) { $installer::globals::updatelastsequence = $lastsequence; }
    }

    $installer::globals::updatesequencecounter = $installer::globals::updatelastsequence;

    return (\%uniquefilename, \%revuniquefilename, \%revshortfilename, \%allupdatesequences, \%allupdatecomponents, \%allupdatefileorder, \%allupdatecomponentorder, \%shortdirname, \%componentid, \%componentidkeypath, \%alloldproperties, \%allupdatelastsequences, \%allupdatediskids);
}


1;
