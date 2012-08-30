#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

package installer::setupscript;

use base 'Exporter';

use installer::exiter;
use installer::globals;
use installer::logger qw(globallog);
use installer::remover;
use installer::scriptitems;
use installer::ziplist;

our @EXPORT_OK = qw(
    add_forced_properties
    add_installationobject_to_variables
    add_lowercase_productname_setupscriptvariable
    add_predefined_folder
    get_all_items_from_script
    get_all_scriptvariables_from_installation_object
    prepare_non_advertised_files
    replace_all_setupscriptvariables_in_script
    replace_preset_properties
    resolve_lowercase_productname_setupscriptvariable
    set_setupscript_name
);

#######################################################
# Set setup script name, if not defined as parameter
#######################################################

sub set_setupscript_name
{
    my ( $allsettingsarrayref, $includepatharrayref ) = @_;

    my $scriptnameref = installer::ziplist::getinfofromziplist($allsettingsarrayref, "script");

    my $scriptname = $$scriptnameref;

    if ( $scriptname eq "" )    # not defined on command line and not in product list
    {
        installer::exiter::exit_program("ERROR: Setup script not defined on command line (-l) and not in product list!", "set_setupscript_name");
    }

    if ( $installer::globals::compiler =~ /wnt/ )
    {
        $scriptname .= ".inf";
    }
    else
    {
        $scriptname .= ".ins";
    }

    # and now the complete path for the setup script is needed
    # The log file cannot be used, because this is the language independent section

    $scriptnameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$scriptname, $includepatharrayref, 1);

    $installer::globals::setupscriptname = $$scriptnameref;

    if ( $installer::globals::setupscriptname eq "" )
    {
        installer::exiter::exit_program("ERROR: Script $scriptname not found!", "set_setupscript_name");
    }
}

#####################################################################
# Reading script variables from installation object of script file
#####################################################################

sub get_all_scriptvariables_from_installation_object
{
    my ($scriptref) = @_;

    my @installobjectvariables;

    for ( my $i = 0; $i <= $#{$scriptref}; $i++ )
    {
        my $line = ${$scriptref}[$i];

        if ( $line =~ /^\s*Installation\s+\w+\s*$/ )    # should be the first line
        {
            my $counter = $i+1;
            my $installline = ${$scriptref}[$counter];

            while (!($installline =~ /^\s*End\s*$/ ))
            {
                if ( $installline =~ /^\s*(\w+)\s+\=\s*(.*?)\s*\;\s*$/ )
                {
                    my $key = $1;
                    my $value = $2;

                    # removing leading and ending " in $value

                    if ( $value =~ /^\s*\"(.*)\"\s*$/ )
                    {
                        $value = $1;
                    }

                    $key = "\%" . uc($key);  # $key is %PRODUCTNAME

                    my $input = $key . " " . $value . "\n";   # $key can only be the first word

                    push(@installobjectvariables ,$input);
                }

                $counter++;
                $installline = ${$scriptref}[$counter];
            }
        }

        last;   # not interesting after installation object
    }

    return \@installobjectvariables;
}

######################################################################
# Including LCPRODUCTNAME into the array
######################################################################

sub add_lowercase_productname_setupscriptvariable
{
    my ( $variablesref ) = @_;

    for ( my $j = 0; $j <= $#{$variablesref}; $j++ )
    {
        my $variableline = ${$variablesref}[$j];

        my ($key, $value);

        if ( $variableline =~ /^\s*\%(\w+?)\s+(.*?)\s*$/ )
        {
            $key = $1;
            $value = $2;

            if ( $key eq "PRODUCTNAME" )
            {
                my $newline = "\%LCPRODUCTNAME " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
                my $original = $value;
                $value =~ s/\s*//g;
                $newline = "\%ONEWORDPRODUCTNAME " . $value . "\n";
                push(@{$variablesref} ,$newline);
                $newline = "\%LCONEWORDPRODUCTNAME " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
                $value = $original;
                $value =~ s/\s*$//g;
                $value =~ s/^\s*//g;
                $value =~ s/ /\%20/g;
                $newline = "\%MASKEDPRODUCTNAME " . $value . "\n";
                push(@{$variablesref} ,$newline);
                $value = $original;
                $value =~ s/\s/\_/g;
                $newline = "\%UNIXPRODUCTNAME " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
                $newline = "\%SYSTEMINTUNIXPACKAGENAME " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
                $newline = "\%UNIXPACKAGENAME " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
                $value = $original;
                $value =~ s/\s/\_/g;
                $value =~ s/\.//g;
                $newline = "\%WITHOUTDOTUNIXPRODUCTNAME " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
                $newline = "\%WITHOUTDOTUNIXPACKAGENAME " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
                $newline = "\%SOLARISBRANDPACKAGENAME " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
                $value = $original;
            }
            elsif  ( $key eq "PRODUCTEXTENSION" )
            {
                my $newline = "\%LCPRODUCTEXTENSION " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
            }
            elsif  ( $key eq "PRODUCTVERSION" )
            {
                $value =~ s/\.//g;
                my $newline = "\%WITHOUTDOTPRODUCTVERSION " . $value . "\n";
                push(@{$variablesref} ,$newline);
            }
        }
    }
}

######################################################################
# Resolving the new introduced lowercase script variables
######################################################################

sub resolve_lowercase_productname_setupscriptvariable
{
    my ( $variablesref ) = @_;

    my %variables = ();

    # First step: Collecting variables

    for ( my $j = 0; $j <= $#{$variablesref}; $j++ )
    {
        my $variableline = ${$variablesref}[$j];

        my ($key, $value);

        if ( $variableline =~ /^\s*\%(\w+?)\s+(.*?)\s*$/ )
        {
            $key = $1;
            $value = $2;
            $variables{$key} = $value;
        }
    }

    # Second step: Resolving variables

    for ( my $j = 0; $j <= $#{$variablesref}; $j++ )
    {
        if ( ${$variablesref}[$j] =~ /\$\{(.*?)\}/ )
        {
            my $key = $1;
            ${$variablesref}[$j] =~ s/\$\{\Q$key\E\}/$variables{$key}/g;
        }
    }

}

######################################################################
# Replacing all setup script variables inside the setup script file
######################################################################

sub replace_all_setupscriptvariables_in_script
{
    my ( $scriptref, $variablesref ) = @_;

    globallog("Replacing variables in setup script (start)");

    # make hash of variables to be substituted if they appear in the script
    my %subs;
    for ( my $j = 0; $j <= $#{$variablesref}; $j++ )
    {
        my $variableline = ${$variablesref}[$j];

        if ( $variableline =~ /^\s*(\%\w+?)\s+(.*?)\s*$/ )
        {
            $subs{$1}= $2;
        }
    }

    # This is far faster than running a regexp for each line
    my $bigstring = '';
    for my $line (@{$scriptref}) { $bigstring = $bigstring . $line; }

    foreach my $key (sort { length ($b) <=> length ($a) } keys %subs)
    {
        # Attention: It must be possible to substitute "%PRODUCTNAMEn", "%PRODUCTNAME%PRODUCTVERSIONabc"
        my $value = $subs{$key};
        $bigstring =~ s/$key/$value/g;
    }

    my @newlines = split /\n/, $bigstring;
    $scriptref = \@newlines;

    # now check for any mis-named '%' variables that we have left
    my $num = 0;
    for my $check (@newlines)
    {
        $num++;
        if ( $check =~ /^.*\%\w+.*$/ )
        {
            if (( $check =~ /%1/ ) || ( $check =~ /%2/ ) || ( $check =~ /%verify/ )) { next; }
            my $infoline = "WARNING: mis-named or un-known '%' variable in setup script at line $num:\n$check\n";
            push( @installer::globals::globallogfileinfo, $infoline);
        }
    }

    globallog("Replacing variables in setup script (end)");

    return $scriptref;
}

#######################################################################
# Collecting all items of the type "searchitem" from the setup script
#######################################################################

sub get_all_items_from_script
{
    my ($scriptref, $searchitem) = @_;

    my @allitemarray = ();

    my ($itemkey, $itemvalue);

    for ( my $i = 0; $i <= $#{$scriptref}; $i++ )
    {
        my $line = ${$scriptref}[$i];

        next unless ($line =~ /^\s*\Q$searchitem\E\s+(\S+)\s*$/);
        my $gid = $1;

        my %oneitemhash = ();
        my $ismultilang = 0;

        $oneitemhash{'gid'} = $gid;

        while (!( $line =~ /^\s*End\s*$/ ))
        {
            if ( $i >= $#{$scriptref} ) {
                installer::exiter::exit_program("Invalid setup script file. End of file reached before 'End' line of '$searchitem' section.", "get_all_items_from_script");
            }
            $line = ${$scriptref}[++$i];

            if ( $line =~ /^\s*(.+?)\=\s*(.+?)\;\s*$/ ) # only oneliner!
            {
                $itemkey = $1;
                $itemvalue = $2;

                $itemkey =~ s/\s+$//;
                $itemvalue =~ s/\s+$//;

                installer::remover::remove_leading_and_ending_quotationmarks(\$itemvalue);

                $oneitemhash{$itemkey} = $itemvalue;

                $ismultilang ||= $itemkey =~ /^\S+\s+\(\S+\)$/;
            }
            elsif (($searchitem eq "Module") &&
                   ($line =~ /^\s*.+?\s*\=\s*\(/) &&
                   (!($line =~ /\)\;\s*$/)))    # more than one line, for instance files at modules!
            {
                $line =~ /^\s*(.+?)\s*\=\s*(.+?)\s*$/;  # the first line
                $itemkey = $1;
                $itemvalue = $2;

                # collecting the complete itemvalue
                do
                {
                    if ( $i >= $#{$scriptref} ) {
                        installer::exiter::exit_program("Invalid setup script file. Premature end of file.", "get_all_items_from_script");
                    }
                    $line = ${$scriptref}[++$i];
                    installer::remover::remove_leading_and_ending_whitespaces(\$line);
                    $itemvalue .= $line;
                } while (!($line =~ /\)\;\s*$/));

                # removing ending ";"
                $itemvalue =~ s/\;\s*$//;

                $oneitemhash{$itemkey} = $itemvalue;

                $ismultilang ||= $itemkey =~ /^\S+\s+\(\S+\)$/;
            }
        }

        $oneitemhash{'ismultilingual'} = $ismultilang+0;

        push(@allitemarray, \%oneitemhash);
    }

    return \@allitemarray;
}

######################################################################
# Collecting all folder at folderitems, that are predefined values
# For example: PREDEFINED_AUTOSTART
######################################################################

sub add_predefined_folder
{
    my ( $folderitemref, $folderref ) = @_;

    for my $folderid ( map { $_->{FolderID} } @{$folderitemref} ) {
        # FIXME: Anchor to start of line?
        next unless ( $folderid =~ /PREDEFINED_/ );
        next if grep { $_->{gid} eq $folderid } @{$folderref};

        push @{$folderref}, {
            ismultilingual => 0,
            Name => "",
            gid => $folderid,
        };
    }
}

#####################################################################################
# If folderitems are non-advertised, the component needs to have a registry key
# below HKCU as key path. Therefore it is required, to mark the file belonging
# to a non-advertised shortcut, that a special userreg_xxx registry key can be
# created during packing process.
#####################################################################################

sub prepare_non_advertised_files
{
    my ( $folderitemref, $filesref ) = @_;

    for ( my $i = 0; $i <= $#{$folderitemref}; $i++ )
    {
        my $folderitem = ${$folderitemref}[$i];
        my $styles = "";
        if ( $folderitem->{'Styles'} ) { $styles = $folderitem->{'Styles'}; }

        if ( $styles =~ /\bNON_ADVERTISED\b/ )
        {
            my $fileid = $folderitem->{'FileID'};
            if ( $folderitem->{'ComponentIDFile'} ) { $fileid = $folderitem->{'ComponentIDFile'}; }
            my $onefile = installer::worker::find_file_by_id($filesref, $fileid);

            # Attention: If $onefile with "FileID" is not found, this is not always an error.
            # FileID can also contain an executable file, for example msiexec.exe.
            if ( $onefile ne "" ) { $onefile->{'needs_user_registry_key'} = 1; }
        }
    }
}

#####################################################################################
# Adding all variables defined in the installation object into the hash
# of all variables from the zip list file.
# This is needed if variables are defined in the installation object,
# but not in the zip list file.
# If there is a definition in the zip list file and in the installation
# object, the installation object is more important
#####################################################################################

sub add_installationobject_to_variables
{
    my ($allvariables, $allscriptvariablesref) = @_;

    for ( my $i = 0; $i <= $#{$allscriptvariablesref}; $i++ )
    {
        my $line = ${$allscriptvariablesref}[$i];

        if ( $line =~ /^\s*\%(\w+)\s+(.*?)\s*$/ )
        {
            my $key = $1;
            my $value = $2;

            $allvariables->{$key} = $value; # overwrite existing values from zip.lst
        }
    }
}

#####################################################################################
# Adding all variables, that must be defined, but are not defined until now.
#####################################################################################

sub add_forced_properties
{
    my ($allvariables) = @_;

    my @forced_properties = qw(
        SERVICETAG_PRODUCTNAME
        SERVICETAG_PRODUCTVERSION
        SERVICETAG_PARENTNAME
        SERVICETAG_SOURCE
        SERVICETAG_URN
    );

    for my $property (@forced_properties)
    {
        if ( ! exists($allvariables->{$property}) ) {
            $allvariables->{$property} = "";
        }
    }
}

#####################################################################################
# Some properties are created automatically. It should be possible to
# overwrite them, with PRESET properties. For example UNIXPRODUCTNAME
# with PRESETUNIXPRODUCTNAME, if this is defined and the automatic process
# does not deliver the desired results.
#####################################################################################

sub replace_preset_properties
{
    my ($allvariables) = @_;

    # SOLARISBRANDPACKAGENAME
    # needs to be replaced by
    # PRESETSOLARISBRANDPACKAGENAME

    my @presetproperties = ();
    push(@presetproperties, "SOLARISBRANDPACKAGENAME");
    push(@presetproperties, "SYSTEMINTUNIXPACKAGENAME");


    foreach $property ( @presetproperties )
    {
        my $presetproperty = "PRESET" . $property;
        if (( exists($allvariables->{$presetproperty}) ) && ( $allvariables->{$presetproperty} ne "" ))
        {
            $allvariables->{$property} = $allvariables->{$presetproperty};
        }
    }
}

1;
