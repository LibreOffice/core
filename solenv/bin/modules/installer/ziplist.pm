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

package installer::ziplist;

use base 'Exporter';

use File::Spec::Functions qw(rel2abs);

use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::remover;
use installer::systemactions;

our @EXPORT_OK = qw(read_ziplist);

sub read_ziplist {
    my $ziplistname = shift;

    installer::logger::globallog("zip list file: $ziplistname");

    my $ziplistref = installer::files::read_file($ziplistname);

    installer::logger::print_message( "... analyzing $ziplistname ... \n" );

    my ($productblockref, $parent) = getproductblock($ziplistref, $installer::globals::product, 1);     # product block from zip.lst

    my ($settingsblockref, undef) = getproductblock($productblockref, "Settings", 0);       # settings block from zip.lst
    $settingsblockref = analyze_settings_block($settingsblockref);              # select data from settings block in zip.lst

    my $allsettingsarrayref = get_settings_from_ziplist($settingsblockref);
    my $allvariablesarrayref = get_variables_from_ziplist($settingsblockref);

    my ($globalproductblockref, undef) = getproductblock($ziplistref, $installer::globals::globalblock, 0);     # global product block from zip.lst

    while (defined $parent) {
        my $parentproductblockref;
        ($parentproductblockref, $parent) = getproductblock($ziplistref, $parent, 1);
        my ($parentsettingsblockref, undef) = getproductblock($parentproductblockref, "Settings", 0);
        $parentsettingsblockref = analyze_settings_block($parentsettingsblockref);
        my $allparentsettingsarrayref = get_settings_from_ziplist($parentsettingsblockref);
        my $allparentvariablesarrayref = get_variables_from_ziplist($parentsettingsblockref);
        $allsettingsarrayref =
            installer::converter::combine_arrays_from_references_first_win(
                $allsettingsarrayref, $allparentsettingsarrayref)
            if $#{$allparentsettingsarrayref} > -1;
        $allvariablesarrayref =
            installer::converter::combine_arrays_from_references_first_win(
                $allvariablesarrayref, $allparentvariablesarrayref)
            if $#{$allparentvariablesarrayref} > -1;
    }

    if ( @{$globalproductblockref} ) {
        my ($globalsettingsblockref, undef) = getproductblock($globalproductblockref, "Settings", 0);       # settings block from zip.lst

        $globalsettingsblockref = analyze_settings_block($globalsettingsblockref);              # select data from settings block in zip.lst

        my $allglobalsettingsarrayref = get_settings_from_ziplist($globalsettingsblockref);

        my $allglobalvariablesarrayref = get_variables_from_ziplist($globalsettingsblockref);

        if ( @{$allglobalsettingsarrayref} ) {
            $allsettingsarrayref = installer::converter::combine_arrays_from_references_first_win($allsettingsarrayref, $allglobalsettingsarrayref);
        }
        if ( @{$allglobalvariablesarrayref} ) {
            $allvariablesarrayref = installer::converter::combine_arrays_from_references_first_win($allvariablesarrayref, $allglobalvariablesarrayref);
        }
    }

    $allsettingsarrayref = remove_multiples_from_ziplist($allsettingsarrayref);
    $allvariablesarrayref = remove_multiples_from_ziplist($allvariablesarrayref);

    replace_variables_in_ziplist_variables($allvariablesarrayref);

    my $allvariableshashref = installer::converter::convert_array_to_hash($allvariablesarrayref);

    set_default_productversion_if_required($allvariableshashref);
    add_variables_to_allvariableshashref($allvariableshashref);
    overwrite_branding( $allvariableshashref );

    return $allsettingsarrayref, $allvariableshashref;
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

        if (! grep {$_ eq $itemname} @itemarray)
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

sub replace_all_variables_in_paths
{
    my ( $patharrayref, $variableshashref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        my $key;

        foreach $key (sort { length ($b) <=> length ($a) } keys %{$variableshashref})
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

sub replace_minor_in_paths
{
    my ( $patharrayref ) = @_;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];

        $line =~ s/\.\{minor\}//g;
        $line =~ s/\.\{minornonpre\}//g;

        ${$patharrayref}[$i] = $line;
    }
}

####################################################
# Replacing packagetype in include path
####################################################

sub replace_packagetype_in_paths
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
# Removing ending separators in paths
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

sub replace_languages_in_paths
{
    my ( $patharrayref, $languagesref ) = @_;

    installer::logger::include_header_into_logfile("Replacing languages in include paths:");

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
                $newdir = $line;
                $line = $originalline;

                installer::remover::remove_leading_and_ending_whitespaces(\$newline);

                # Is it necessary to refresh the global array, containing all files of all include paths?
                if ( -d $newdir )
                {
                    # Checking if $newdir is empty
                    if ( ! installer::systemactions::is_empty_dir($newdir) )
                    {
                        $installer::globals::refresh_includepaths = 1;
                        $infoline = "Directory $newdir exists and is not empty. Refreshing global file array is required.\n";
                        push( @installer::globals::logfileinfo, $infoline);
                    }
                    else
                    {
                        $infoline = "Directory $newdir is empty. No refresh of global file array required.\n";
                        push( @installer::globals::logfileinfo, $infoline);
                    }
                }
                else
                {
                    $infoline = "Directory $newdir does not exist. No refresh of global file array required.\n";
                    push( @installer::globals::logfileinfo, $infoline);
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

    installer::logger::include_header_into_logfile("Include paths:");

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $path = ${$patharrayref}[$i];
        installer::remover::remove_leading_and_ending_whitespaces(\$path);
        my $infoline = "$path\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    push( @installer::globals::logfileinfo, "\n");

    return \@filesarray;
}

#####################################################
# Collecting all files from all include paths
#####################################################

sub set_manufacturer
{
    my ($allvariables) = @_;
    my $manufacturer;

    if( defined $ENV{'OOO_VENDOR'} && $ENV{'OOO_VENDOR'} ne "" )
    {
        $manufacturer = $ENV{'OOO_VENDOR'};
    }
    elsif( defined $ENV{'USERNAME'} && $ENV{'USERNAME'} ne "" )
    {
        $manufacturer = $ENV{'USERNAME'};
    }
    elsif( defined $ENV{'USER'} && $ENV{'USER'} ne "" )
    {
        $manufacturer = $ENV{'USER'};
    }
    else
    {
        $manufacturer = "default";
    }

    $installer::globals::manufacturer = $manufacturer;
    $installer::globals::longmanufacturer = $manufacturer;

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
# Removing .. in paths
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
# Removing ending separators in paths
####################################################

sub resolve_relative_paths
{
    my ( $patharrayref ) = @_;

    for my $path ( @{$patharrayref} )
    {
        $path = rel2abs($path);
        simplify_path(\$path);
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
    }
}

###########################################################
# Overwrite branding data in openoffice.lst that is defined in configure
###########################################################

sub overwrite_branding
{
    my ($variableshashref) = @_;
    $variableshashref->{'OOOVENDOR'} = $ENV{'OOO_VENDOR'} , if( defined $ENV{'OOO_VENDOR'}  && $ENV{'OOO_VENDOR'} ne "" );
    $variableshashref->{'PROGRESSBARCOLOR'} = $ENV{'PROGRESSBARCOLOR'} , if( defined $ENV{'PROGRESSBARCOLOR'}  && $ENV{'PROGRESSBARCOLOR'} ne "" );
    $variableshashref->{'PROGRESSSIZE'} = $ENV{'PROGRESSSIZE'} , if( defined $ENV{'PROGRESSSIZE'}  && $ENV{'PROGRESSSIZE'} ne "" );
    $variableshashref->{'PROGRESSPOSITION'} = $ENV{'PROGRESSPOSITION'} , if( defined $ENV{'PROGRESSPOSITION'}  && $ENV{'PROGRESSPOSITION'} ne "" );
    $variableshashref->{'PROGRESSFRAMECOLOR'} = $ENV{'PROGRESSFRAMECOLOR'} , if( defined $ENV{'PROGRESSFRAMECOLOR'}  && $ENV{'PROGRESSFRAMECOLOR'} ne "" );
    $variableshashref->{'PROGRESSTEXTCOLOR'} = $ENV{'PROGRESSTEXTCOLOR'} , if( defined $ENV{'PROGRESSTEXTCOLOR'}  && $ENV{'PROGRESSTEXTCOLOR'} ne "" );
    $variableshashref->{'PROGRESSTEXTBASELINE'} = $ENV{'PROGRESSTEXTBASELINE'} , if( defined $ENV{'PROGRESSTEXTBASELINE'}  && $ENV{'PROGRESSTEXTBASELINE'} ne "" );
}

###########################################################
# Adding the lowercase variables into the variableshashref
###########################################################

sub add_variables_to_allvariableshashref
{
    my ($variableshashref) = @_;

    my $lcvariable = lc($variableshashref->{'PRODUCTNAME'});
    $variableshashref->{'LCPRODUCTNAME'} = $lcvariable;

    if ($variableshashref->{'PRODUCTEXTENSION'})
    {
        $variableshashref->{'LCPRODUCTEXTENSION'} = "\-" . lc($variableshashref->{'PRODUCTEXTENSION'}); # including the "-" !
    }
    else
    {
        $variableshashref->{'LCPRODUCTEXTENSION'} = "";
    }

    if ( $installer::globals::languagepack ) { $variableshashref->{'PRODUCTADDON'} = $installer::globals::languagepackaddon; }
    elsif ( $installer::globals::helppack ) { $variableshashref->{'PRODUCTADDON'} = $installer::globals::helppackpackaddon; }
    else { $variableshashref->{'PRODUCTADDON'} = ""; }

    my $localbuild = $installer::globals::build;
    if ( $localbuild =~ /^\s*(\w+?)(\d+)\s*$/ ) { $localbuild = $2; }   # using "680" instead of "src680"
    $variableshashref->{'PRODUCTMAJOR'} = $localbuild;

    my $localminor = "";
    if ( $installer::globals::minor ne "" ) { $localminor = $installer::globals::minor; }
    else { $localminor = $installer::globals::lastminor; }
    if ( $localminor =~ /^\s*\w(\d+)\w*\s*$/ ) { $localminor = $1; }
    $variableshashref->{'PRODUCTMINOR'} = $localminor;

    $variableshashref->{'PRODUCTBUILDID'} = $installer::globals::buildid;
    $variableshashref->{'SYSTEM_LIBEXTTEXTCAT_DATA'} = $ENV{'SYSTEM_LIBEXTTEXTCAT_DATA'} , if( defined $ENV{'SYSTEM_LIBEXTTEXTCAT_DATA'} && $ENV{'SYSTEM_LIBEXTTEXTCAT_DATA'} ne "" );
}

1;
