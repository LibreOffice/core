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



package installer::ziplist;

use installer::existence;
use installer::exiter;
use installer::globals;
use installer::logger;
use installer::parameter;
use installer::remover;
use installer::systemactions;
use strict;

=head2 read_openoffice_lst_file (#loggingdir)
    Read the settings and variables from the settings file (typically 'openoffice.lst').
=cut
sub read_openoffice_lst_file ($$;$)
{
    my ($filename, $product_name, $loggingdir) = @_;

    # Read all lines from the settings file.
    my $ziplistref = installer::files::read_file($filename);

    # Extract the lines of the settings block for the current product.
    my ($productblockref, $parent) = installer::ziplist::getproductblock(
        $ziplistref, $product_name, 1);
    my ($settingsblockref, undef) = installer::ziplist::getproductblock($productblockref, "Settings", 0);
    $settingsblockref = installer::ziplist::analyze_settings_block($settingsblockref);

    # Split into settings and variables.
    my $allsettingsarrayref = installer::ziplist::get_settings_from_ziplist($settingsblockref);
    my $allvariablesarrayref = installer::ziplist::get_variables_from_ziplist($settingsblockref);

    # global product block from zip.lst
    my ($globalproductblockref, undef) = installer::ziplist::getproductblock(
        $ziplistref, $installer::globals::globalblock, 0);

    if ($installer::globals::globallogging && defined $loggingdir)
    {
        installer::files::save_file($loggingdir . "productblock.log", $productblockref);
        installer::files::save_file($loggingdir . "settingsblock.log", $settingsblockref);
        installer::files::save_file($loggingdir . "allsettings1.log", $allsettingsarrayref);
        installer::files::save_file($loggingdir . "allvariables1.log", $allvariablesarrayref);
        installer::files::save_file($loggingdir . "globalproductblock.log", $globalproductblockref);
    }

    # Integrate parent values.
    while (defined $parent)
    {
        my $parentproductblockref;
        ($parentproductblockref, $parent) = installer::ziplist::getproductblock($ziplistref, $parent, 1);
        my ($parentsettingsblockref, undef) = installer::ziplist::getproductblock(
            $parentproductblockref, "Settings", 0);
        $parentsettingsblockref = installer::ziplist::analyze_settings_block($parentsettingsblockref);
        my $allparentsettingsarrayref = installer::ziplist::get_settings_from_ziplist($parentsettingsblockref);
        my $allparentvariablesarrayref = installer::ziplist::get_variables_from_ziplist($parentsettingsblockref);
        if (scalar @$allparentsettingsarrayref > 0)
        {
            $allsettingsarrayref = installer::converter::combine_arrays_from_references_first_win(
                $allsettingsarrayref,
                $allparentsettingsarrayref)
        }
        if (scalar @$allparentvariablesarrayref)
        {
            $allvariablesarrayref = installer::converter::combine_arrays_from_references_first_win(
                $allvariablesarrayref,
                $allparentvariablesarrayref)
        }
    }

    # Integrate global values.
    if (scalar @$globalproductblockref)
    {
        # settings block from zip.lst
        my ($globalsettingsblockref, undef) = installer::ziplist::getproductblock(
            $globalproductblockref, "Settings", 0);

        # select data from settings block in zip.lst
        $globalsettingsblockref = installer::ziplist::analyze_settings_block($globalsettingsblockref);

        my $allglobalsettingsarrayref = installer::ziplist::get_settings_from_ziplist($globalsettingsblockref);
        my $allglobalvariablesarrayref = installer::ziplist::get_variables_from_ziplist($globalsettingsblockref);

        if ($installer::globals::globallogging && defined $loggingdir)
        {
            installer::files::save_file($loggingdir . "globalsettingsblock1.log", $globalsettingsblockref);
            installer::files::save_file($loggingdir . "globalsettingsblock2.log", $globalsettingsblockref);
            installer::files::save_file($loggingdir . "allglobalsettings1.log", $allglobalsettingsarrayref);
            installer::files::save_file($loggingdir . "allglobalvariables1.log", $allglobalvariablesarrayref);
        }

        if (scalar @$allglobalsettingsarrayref > 0)
        {
            $allsettingsarrayref = installer::converter::combine_arrays_from_references_first_win(
                $allsettingsarrayref, $allglobalsettingsarrayref);
        }
        if (scalar @$allglobalvariablesarrayref > 0)
        {
            $allvariablesarrayref = installer::converter::combine_arrays_from_references_first_win(
                $allvariablesarrayref, $allglobalvariablesarrayref);
        }
    }

    # Remove multiples (and the trailing ##%##).
    $allsettingsarrayref = installer::ziplist::remove_multiples_from_ziplist($allsettingsarrayref);
    $allvariablesarrayref = installer::ziplist::remove_multiples_from_ziplist($allvariablesarrayref);
    installer::ziplist::replace_variables_in_ziplist_variables($allvariablesarrayref);

    # Transform array into hash.
    my $allvariableshashref = installer::converter::convert_array_to_hash($allvariablesarrayref);

    # Postprocess the variables.
    installer::ziplist::set_default_productversion_if_required($allvariableshashref);
    installer::ziplist::add_variables_to_allvariableshashref($allvariableshashref);
    installer::ziplist::overwrite_ooovendor($allvariableshashref);

    if ($installer::globals::globallogging && defined $loggingdir)
    {
        installer::files::save_file($loggingdir . "allsettings2.log" ,$allsettingsarrayref);
        installer::files::save_file($loggingdir . "allvariables2.log" ,$allvariablesarrayref);
    }

    # Eventually we should fix this so that we don't have to return the raw arrays, only the resulting hashes.
    return ($allvariableshashref, $allsettingsarrayref);
}


#################################################
# Getting data from path file and zip list file
#################################################

sub getproductblock
{
    my ($fileref, $search, $inheritance) = @_;

    my @searchblock = ();
    my $searchexists = 0;
    my $record = 0;
    my $count = 0;
    my $line;
    my $inh = $inheritance ? '(?::\s*(\S+)\s*)?' : "";
    my $parent;

    for ( my $i = 0; $i <= $#{$fileref}; $i++ )
    {
        $line = ${$fileref}[$i];

        if ( $line =~ /^\s*\Q$search\E\s*$inh$/i )      # case insensitive
        {
            $record = 1;
            $searchexists = 1;
            $parent = $1 if $inheritance;
        }

        if ($record)
        {
            push(@searchblock, $line);
        }

        if ( ($record) && ($line =~ /\{/) )
        {
            $count++;
        }

        if ( ($record) && ($line =~ /\}/) )
        {
            $count--;
        }

        if ( ($record) && ($line =~ /\}/) && ( $count == 0 ) )
        {
            $record = 0;
        }
    }

    if (( ! $searchexists ) && ( $search ne $installer::globals::globalblock ))
    {
        if ($search eq $installer::globals::product )
        {
            installer::exiter::exit_program("ERROR: Product $installer::globals::product not defined in $installer::globals::ziplistname", "getproductblock");
        }
        elsif ($search eq $installer::globals::compiler )
        {
            installer::exiter::exit_program("ERROR: Compiler $installer::globals::compiler not defined in $installer::globals::pathfilename", "getproductblock");
        }
        else    # this is not possible
        {
            installer::exiter::exit_program("ERROR: Unknown value for $search in getproductblock()", "getproductblock");
        }
    }

    return (\@searchblock, $parent);
}

###############################################
# Analyzing the settings in the zip list file
###############################################

sub analyze_settings_block
{
    my ($blockref) = @_;

    my @newsettingsblock = ();
    my $compilerstring = "";
    my $record = 1;
    my $counter = 0;

    # Allowed values in settings block:
    # "Settings", "Variables", "unix" (for destination path and logfile)
    # Furthermore allowed values are $installer::globals::build (srx645) and $installer::globals::compiler (pro and nonpro (unxsols4.pro))

    # Comment line in settings block begin with "#" or ";"

    if ( $installer::globals::pro )
    {
        $compilerstring = $installer::globals::compiler . ".pro";
    }
    else
    {
        $compilerstring = $installer::globals::compiler;
    }

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        my $line = ${$blockref}[$i];
        my $nextline = "";

        if ( ${$blockref}[$i+1] ) { $nextline = ${$blockref}[$i+1]; }

        # removing comment lines

        if (($line =~ /^\s*\#/) || ($line =~ /^\s*\;/))
        {
            next;
        }

        # complete blocks of unknows strings are not recorded

        if ((!($line =~ /^\s*\Q$compilerstring\E\s*$/i)) &&
            (!($line =~ /^\s*\Q$installer::globals::build\E\s*$/i)) &&
            (!($line =~ /^\s*\bSettings\b\s*$/i)) &&
            (!($line =~ /^\s*\bVariables\b\s*$/i)) &&
            (!($line =~ /^\s*\bunix\b\s*$/i)) &&
            ($nextline =~ /^\s*\{\s*$/i))
        {
            $record = 0;
            next;           # continue with next $i
        }

        if (!( $record ))
        {
            if ($line =~ /^\s*\{\s*$/i)
            {
                $counter++;
            }

            if ($line =~ /^\s*\}\s*$/i)
            {
                $counter--;
            }

            if ($counter == 0)
            {
                $record = 1;
                next;   # continue with next $i
            }
        }

        if ($record)
        {
            push(@newsettingsblock, $line);
        }
    }

    return \@newsettingsblock;
}

########################################
# Settings in zip list file
########################################

sub get_settings_from_ziplist
{
    my ($blockref) = @_;

    my @allsettings = ();
    my $isvariables = 0;
    my $counter = 0;
    my $variablescounter = 0;

    # Take all settings from the settings block
    # Do not take the variables from the settings block
    # If a setting is defined more than once, take the
    # setting with the largest counter (open brackets)

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        my $line = ${$blockref}[$i];
        my $nextline = "";

        if ( ${$blockref}[$i+1] ) { $nextline = ${$blockref}[$i+1]; }

        if (($line =~ /^\s*\S+\s*$/i) &&
            ($nextline =~ /^\s*\{\s*$/i) &&
            (!($line =~ /^\s*Variables\s*$/i)))
        {
            next;
        }

        if ($line =~ /^\s*Variables\s*$/i)
        {
            # This is a block of variables

            $isvariables = 1;
            next;
        }

        if ($line =~ /^\s*\{\s*$/i)
        {
            if ($isvariables)
            {
                $variablescounter++;
            }
            else
            {
                $counter++;
            }

            next;
        }

        if ($line =~ /^\s*\}\s*$/i)
        {
            if ($isvariables)
            {
                $variablescounter--;

                if ($variablescounter == 0)
                {
                    $isvariables = 0;
                }
            }
            else
            {
                $counter--;
            }

            next;
        }

        if ($isvariables)
        {
            next;
        }

        installer::remover::remove_leading_and_ending_whitespaces(\$line);

        $line .= "\t##$counter##\n";

        push(@allsettings, $line);
    }

    return \@allsettings;
}

#######################################
# Variables from zip list file
#######################################

sub get_variables_from_ziplist
{
    my ($blockref) = @_;

    my @allvariables = ();
    my $isvariables = 0;
    my $counter = 0;
    my $variablescounter = 0;
    my $countersum = 0;

    # Take all variables from the settings block
    # Do not take the other settings from the settings block
    # If a variable is defined more than once, take the
    # variable with the largest counter (open brackets)

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        my $line = ${$blockref}[$i];
        my $nextline = ${$blockref}[$i+1];

        if ($line =~ /^\s*Variables\s*$/i)
        {
            # This is a block of variables

            $isvariables = 1;
            next;
        }

        if ($line =~ /^\s*\{\s*$/i)
        {
            if ($isvariables)
            {
                $variablescounter++;
            }
            else
            {
                $counter++;
            }

            next;
        }

        if ($line =~ /^\s*\}\s*$/i)
        {
            if ($isvariables)
            {
                $variablescounter--;

                if ($variablescounter == 0)
                {
                    $isvariables = 0;
                }
            }
            else
            {
                $counter--;
            }

            next;
        }

        if (!($isvariables))
        {
            next;
        }

        $countersum = $counter + $variablescounter;

        installer::remover::remove_leading_and_ending_whitespaces(\$line);

        $line .= "\t##$countersum##\n";

        push(@allvariables, $line);
    }

    return \@allvariables;
}

#######################################################################
# Removing multiple variables and settings, defined in zip list file
#######################################################################

sub remove_multiples_from_ziplist
{
    my ($blockref) = @_;

    # remove all definitions of settings and variables
    # that occur more than once in the zip list file.
    # Take the one with the most open brackets. This
    # number is stored at the end of the string.

    my @newarray = ();
    my @itemarray = ();
    my ($line, $itemname, $itemnumber);

    # first collecting all variables and settings names

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        $line = ${$blockref}[$i];

        if ($line =~ /^\s*\b(\S*)\b\s+.*\#\#\d+\#\#\s*$/i)
        {
            $itemname = $1;
        }

        if (! installer::existence::exists_in_array($itemname, \@itemarray))
        {
            push(@itemarray, $itemname);
        }
    }

    # and now all $items can be selected with the highest number

    for ( my $i = 0; $i <= $#itemarray; $i++ )
    {
        $itemname = $itemarray[$i];

        my $itemnumbermax = 0;
        my $printline = "";

        for ( my $j = 0; $j <= $#{$blockref}; $j++ )
        {
            $line = ${$blockref}[$j];

            if ($line =~ /^\s*\Q$itemname\E\s+.*\#\#(\d+)\#\#\s*$/)
            {
                $itemnumber = $1;

                if ($itemnumber >= $itemnumbermax)
                {
                    $printline = $line;
                    $itemnumbermax = $itemnumber;
                }
            }
        }

        # removing the ending number from the printline
        # and putting it into the array

        $printline =~ s/\#\#\d+\#\#//;
        installer::remover::remove_leading_and_ending_whitespaces(\$line);
        push(@newarray, $printline);
    }

    return \@newarray;
}

#########################################################
# Reading one variable defined in the zip list file
#########################################################

sub getinfofromziplist
{
    my ($blockref, $variable) = @_;

    my $searchstring = "";
    my $line;

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        $line = ${$blockref}[$i];

        if ( $line =~ /^\s*\Q$variable\E\s+(.+?)\s*$/ ) # "?" for minimal matching
        {
            $searchstring = $1;
            last;
        }
    }

    return \$searchstring;
}

####################################################
# Replacing variables in include path
####################################################

sub replace_all_variables_in_pathes
{
    my ( $patharrayref, $variableshashref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        my $key;

        foreach $key (keys %{$variableshashref})
        {
            my $value = $variableshashref->{$key};

            if (( $line =~ /\{$key\}/ ) && ( $value eq "" )) { $line = ".\n"; }

            $line =~ s/\{\Q$key\E\}/$value/g;
        }

        ${$patharrayref}[$i] = $line;
    }
}

####################################################
# Replacing minor in include path
####################################################

sub replace_minor_in_pathes
{
    my ( $patharrayref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        if ( ! defined $ENV{CWS_WORK_STAMP} and defined $ENV{UPDMINOR} )
#       if ( $installer::globals::minor )
        {
            $line =~ s/\{minor\}/$installer::globals::minor/g;
            # no difference for minor and minornonpre (ToDo ?)
            $line =~ s/\{minornonpre\}/$installer::globals::minor/g;
        }
        else    # building without a minor
        {
            $line =~ s/\.\{minor\}//g;
            $line =~ s/\.\{minornonpre\}//g;
        }

        ${$patharrayref}[$i] = $line;
    }
}

####################################################
# Replacing packagetype in include path
####################################################

sub replace_packagetype_in_pathes
{
    my ( $patharrayref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        if (( $installer::globals::installertypedir ) && ( $line =~ /\{pkgtype\}/ ))
        {
            $line =~ s/\{pkgtype\}/$installer::globals::installertypedir/g;
        }

        ${$patharrayref}[$i] = $line;
    }
}

####################################################
# Removing ending separators in pathes
####################################################

sub remove_ending_separator
{
    my ( $patharrayref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        installer::remover::remove_ending_pathseparator(\$line);

        $line =~ s/\s*$//;
        $line = $line . "\n";

        ${$patharrayref}[$i] = $line;
    }
}

####################################################
# Replacing languages in include path
####################################################

sub replace_languages_in_pathes
{
    my ( $patharrayref, $languagesref ) = @_;

    installer::logger::include_header_into_logfile("Replacing languages in include pathes:");

    my @patharray = ();
    my $infoline = "";

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        if ( $line =~ /\$\(LANG\)/ )
        {
            my $originalline = $line;
            my $newline = "";

            for ( my $j = 0; $j <= $#{$languagesref}; $j++ )
            {
                my $language = ${$languagesref}[$j];
                $line =~ s/\$\(LANG\)/$language/g;
                push(@patharray ,$line);
                my $newdir = $line;
                $line = $originalline;

                installer::remover::remove_leading_and_ending_whitespaces(\$newline);

                # Is it necessary to refresh the global array, containing all files of all include pathes?
                if ( -d $newdir )
                {
                    # Checking if $newdir is empty
                    if ( ! installer::systemactions::is_empty_dir($newdir) )
                    {
                        $installer::globals::refresh_includepathes = 1;
                        $infoline = "Directory $newdir exists and is not empty. Refreshing global file array is required.\n";
                        $installer::logger::Lang->print($infoline);
                    }
                    else
                    {
                        $infoline = "Directory $newdir is empty. No refresh of global file array required.\n";
                        $installer::logger::Lang->print($infoline);
                    }
                }
                else
                {
                    $infoline = "Directory $newdir does not exist. No refresh of global file array required.\n";
                    $installer::logger::Lang->print($infoline);
                }
            }
        }
        else        # not language dependent include path
        {
            push(@patharray ,$line);
        }
    }

    return \@patharray;
}

#####################################################
# Collecting all files from all include paths
#####################################################

sub list_all_files_from_include_path
{
    my ( $patharrayref) = @_;

    installer::logger::include_header_into_logfile("Include pathes:");

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $path = ${$patharrayref}[$i];
        installer::remover::remove_leading_and_ending_whitespaces(\$path);
        my $infoline = "$path\n";
        $installer::logger::Lang->print($infoline);
    }

    $installer::logger::Lang->print("\n");
}

#####################################################
# Collecting all files from all include paths
#####################################################

sub set_manufacturer
{
    my ($allvariables) = @_;

    my $openofficeproductname = "OpenOffice";
    my $sunname = "";


    if ( $allvariables->{'OPENSOURCE'} && $allvariables->{'OPENSOURCE'} == 1 )
    {
        $installer::globals::isopensourceproduct = 1;
        $installer::globals::manufacturer = $openofficeproductname;
        $installer::globals::longmanufacturer = $openofficeproductname;
    }
    else
    {
        $installer::globals::isopensourceproduct = 0;
        if (( $allvariables->{'DEFINEDMANUFACTURER'} ) && ( $allvariables->{'DEFINEDMANUFACTURER'} ne "" )) { $sunname = $allvariables->{'DEFINEDMANUFACTURER'}; }
        else { installer::exiter::exit_program("ERROR: Property DEFINEDMANUFACTURER has to be set for this product!", "set_manufacturer"); }
        $installer::globals::manufacturer = $sunname;
        $installer::globals::longmanufacturer = $sunname;
    }

    $allvariables->{'MANUFACTURER'} = $installer::globals::manufacturer;
}

##############################################################
# A ProductVersion has to be defined. If it is not set in
# zip.lst, it is set now to "1"
##############################################################

sub set_default_productversion_if_required
{
    my ($allvariables) = @_;

    if (!($allvariables->{'PRODUCTVERSION'}))
    {
        $allvariables->{'PRODUCTVERSION'} = 1;  # FAKE
    }
}

####################################################
# Removing .. in pathes
####################################################

sub simplify_path
{
    my ( $pathref ) = @_;

    my $oldpath = $$pathref;

    my $change = 0;

    while ( $oldpath =~ /(^.*)(\Q$installer::globals::separator\E.*\w+?)(\Q$installer::globals::separator\E\.\.)(\Q$installer::globals::separator\E.*$)/ )
    {
        my $part1 = $1;
        my $part2 = $4;
        $oldpath = $part1 . $part2;
        $change = 1;
    }

    if ( $change ) { $$pathref = $oldpath . "\n"; }
}

####################################################
# Removing ending separators in pathes
####################################################

sub resolve_relative_pathes
{
    my ( $patharrayref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        installer::parameter::make_path_absolute(\${$patharrayref}[$i]);
        simplify_path(\${$patharrayref}[$i]);
    }
}

####################################################
# Replacing variables inside zip list variables
# Example: {milestone} to be replaced by
# $installer::globals::lastminor
####################################################

sub replace_variables_in_ziplist_variables
{
    my ($blockref) = @_;

    my $milestonevariable = $installer::globals::lastminor;
    $milestonevariable =~ s/m//;
    $milestonevariable =~ s/s/\./;

    my $localminor = $installer::globals::lastminor;
    if ( $installer::globals::minor ) { $localminor = $installer::globals::minor; }

    my $buildidstringcws = $installer::globals::build . $localminor . "(Build:" . $installer::globals::buildid . ")";

    # the environment variable CWS_WORK_STAMP is set only in CWS
    if ( $ENV{'CWS_WORK_STAMP'} ) { $buildidstringcws = $buildidstringcws . "\[CWS\:" . $ENV{'CWS_WORK_STAMP'} . "\]"; }

    for ( my $i = 0; $i <= $#{$blockref}; $i++ )
    {
        if ($installer::globals::lastminor) { ${$blockref}[$i] =~ s/\{milestone\}/$milestonevariable/; }
        else { ${$blockref}[$i] =~ s/\{milestone\}//; }
        if ( $localminor ) { ${$blockref}[$i] =~ s/\{minor\}/$localminor/; }
        else { ${$blockref}[$i] =~ s/\{minor\}//; }
        if ( $installer::globals::buildid ) { ${$blockref}[$i] =~ s/\{buildid\}/$installer::globals::buildid/; }
        else { ${$blockref}[$i] =~ s/\{buildid\}//; }
        if ( $installer::globals::build ) { ${$blockref}[$i] =~ s/\{buildsource\}/$installer::globals::build/; }
        else { ${$blockref}[$i] =~ s/\{build\}//; }
        ${$blockref}[$i] =~ s/\{buildidcws\}/$buildidstringcws/;
    }
}

###########################################################
# Overwrite the vendor string in openoffice.lst that is defined in configure
###########################################################

sub overwrite_ooovendor
{
    my ($variableshashref) = @_;
    $variableshashref->{'OOOVENDOR'} = $ENV{'OOO_VENDOR'} , if( defined $ENV{'OOO_VENDOR'}  && $ENV{'OOO_VENDOR'} ne "" );
}

###########################################################
# Adding the lowercase variables into the variableshashref
###########################################################

sub add_variables_to_allvariableshashref
{
    my ($variableshashref) = @_;

    my $lcvariable = lc($variableshashref->{'PRODUCTNAME'});
    $variableshashref->{'LCPRODUCTNAME'} = $lcvariable;

    if ($variableshashref->{'SHORT_PRODUCTEXTENSION'})
    {
        $variableshashref->{'LCPRODUCTEXTENSION'} = "\-" . lc($variableshashref->{'SHORT_PRODUCTEXTENSION'}); # including the "-" !
    }
    else
    {
        $variableshashref->{'LCPRODUCTEXTENSION'} = "";
    }

    if ($installer::globals::patch)
    {
        $variableshashref->{'PRODUCTADDON'} = $installer::globals::patchaddon;
    }
    elsif ($installer::globals::languagepack)
    {
        $variableshashref->{'PRODUCTADDON'} = $installer::globals::languagepackaddon;
    }
    else
    {
        $variableshashref->{'PRODUCTADDON'} = "";
    }

    my $localbuild = $installer::globals::build;
    if ( $localbuild =~ /^\s*(\w+?)(\d+)\s*$/ ) { $localbuild = $2; }   # using "680" instead of "src680"
    $variableshashref->{'PRODUCTMAJOR'} = $localbuild;

    my $localminor = "";
    if ( $installer::globals::minor ne "" ) { $localminor = $installer::globals::minor; }
    else { $localminor = $installer::globals::lastminor; }
    if ( $localminor =~ /^\s*\w(\d+)\w*\s*$/ ) { $localminor = $1; }
    $variableshashref->{'PRODUCTMINOR'} = $localminor;

    $variableshashref->{'PRODUCTBUILDID'} = $installer::globals::buildid;
    $variableshashref->{'SYSTEM_LIBTEXTCAT_DATA'} = $ENV{'SYSTEM_LIBTEXTCAT_DATA'} , if( defined $ENV{'SYSTEM_LIBTEXTCAT_DATA'} && $ENV{'SYSTEM_LIBTEXTCAT_DATA'} ne "" );
}

1;
