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



package installer::setupscript;

use installer::existence;
use installer::exiter;
use installer::globals;
use installer::logger;
use installer::remover;
use installer::scriptitems;
use installer::ziplist;

use strict;

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

sub get_all_scriptvariables_from_installation_object ($$)
{
    my ($scriptref, $script_filename) = @_;

    my $installobjectvariables = {};

    my $firstline = $scriptref->[0];
    if ($firstline !~ /^\s*Installation\s+\w+\s*$/)
    {
        installer::logger::PrintError("did not find 'Installation' keyword in first line of %s\n",
            $script_filename);
    }
    foreach my $line (@$scriptref)
    {
        next if $line =~ /^\s*Installation\s+\w+\s*$/; # Already processed.
        last if $line =~ /^\s*End\s*$/;

        if ($line =~ /^\s*(\w+)\s+\=\s*\"?(.*?)\"?\s*\;\s*$/ )
        {
            my ($key, $value) = ($1, $2);

            $installobjectvariables->{uc($key)} = $value;
        }

    }

    return $installobjectvariables;
}

######################################################################
# Including LCPRODUCTNAME into the array
######################################################################

sub add_lowercase_productname_setupscriptvariable ($)
{
    my ( $variablesref ) = @_;

    my %additional_variables = ();

    while (my ($key, $value) = each %$variablesref)
    {
        if ($key eq "PRODUCTNAME")
        {
            $additional_variables{"LCPRODUCTNAME"} = lc($value);
            my $original = $value;
            $value =~ s/\s+//g;
            $additional_variables{"ONEWORDPRODUCTNAME"} = $value;
            $additional_variables{"LCONEWORDPRODUCTNAME"} = lc($value);
            $value = $original;
            $value =~ s/(^\s+|\s+$)//g;
            $value =~ s/ /\%20/g;
            $additional_variables{"MASKEDPRODUCTNAME"} = $value;
            $value = $original;
            $value =~ s/\s/\_/g;
            $additional_variables{"UNIXPRODUCTNAME"} = lc($value);
            $additional_variables{"SYSTEMINTUNIXPACKAGENAME"} = lc($value);
            $additional_variables{"UNIXPACKAGENAME"} = lc($value);
            $value = $original;
            $value =~ s/\s/\_/g;
            $value =~ s/\.//g;
            $additional_variables{"WITHOUTDOTUNIXPRODUCTNAME"} = lc($value);
            $additional_variables{"WITHOUTDOTUNIXPACKAGENAME"} = lc($value);
            $additional_variables{"SOLARISBRANDPACKAGENAME"} = lc($value);
        }
        elsif  ($key eq "PRODUCTEXTENSION")
        {
            $additional_variables{"LCPRODUCTEXTENSION"} = lc($value);
        }
        elsif  ($key eq "PRODUCTVERSION")
        {
            $value =~ s/\.//g;
            $additional_variables{"WITHOUTDOTPRODUCTVERSION"} = $value;
        }
        elsif  ($key eq "OOOBASEVERSION")
        {
            $value =~ s/\.//g;
            $additional_variables{"WITHOUTDOTOOOBASEVERSION"} = $value;
        }
    }

    while (my ($key, $value) = each %additional_variables)
    {
        $variablesref->{$key} = $value;
    }
}

######################################################################
# Resolving the new introduced lowercase script variables
######################################################################

sub resolve_lowercase_productname_setupscriptvariable ($)
{
    my ($variablesref) = @_;

    while (my ($key,$value) = each %$variablesref)
    {
        if ($value =~ /\$\{(.*?)\}/)
        {
            my $varname = $1;
            my $replacement = $variablesref->{$varname};
            my $new_value = $value;
            $new_value =~ s/\$\{\Q$varname\E\}/$replacement/g;
            $variablesref->{$key} = $new_value;
        }
    }
}




######################################################################
# Replacing all setup script variables inside the setup script file
######################################################################

sub replace_all_setupscriptvariables_in_script ($$)
{
    my ($script_lines, $variables) = @_;

    installer::logger::include_header_into_globallogfile("Replacing variables in setup script (start)");

    # This is far faster than running a regexp for each line
    my $bigstring = join("", @$script_lines);

    while (my ($key,$value) = each %$variables)
    {
        # Attention: It must be possible to substitute "%PRODUCTNAMEn", "%PRODUCTNAME%PRODUCTVERSIONabc"
        my $count = ($bigstring =~ s/%$key/$value/g);
        if ($count > 0)
        {
            $installer::logger::Lang->printf("replaced %s %d times\n", $key, $count);
        }
    }

    my @newlines = split /\n/, $bigstring;

    # now check for any mis-named '%' variables that we have left
    my $num = 0;
    foreach my $line (@newlines)
    {
        $num++;
        if ($line =~ /\%\w+/)
        {
            if (( $line =~ /%1/ ) || ( $line =~ /%2/ ) || ( $line =~ /%verify/ ))
            {
                next;
            }
            $installer::logger::Info->printf(
                "WARNING: mis-named or un-known %%-variable in setup script at line %s:\n",$num);
            $installer::logger::Info->printf("%s\n", $line);
        }
    }

    installer::logger::include_header_into_globallogfile("Replacing variables in setup script (end)");

    return \@newlines;
}

#######################################################################
# Collecting all items of the type "searchitem" from the setup script
#######################################################################

sub get_all_items_from_script
{
    my ($scriptref, $searchitem) = @_;

    my @allitemarray = ();

    my ($itemkey, $itemvalue, $valuecounter);

    for ( my $i = 0; $i <= $#{$scriptref}; $i++ )
    {
        my $line = ${$scriptref}[$i];

        if ( $line =~ /^\s*\Q$searchitem\E\s+(\S+)\s*$/ )
        {
            my $gid = $1;
            my $counter = $i + 1;

            my %oneitemhash = ();
            my $ismultilang = 0;

            $oneitemhash{'gid'} = $gid;

            while  (!( $line =~ /^\s*End\s*$/ ))
            {
                if ( $counter > $#{$scriptref} ) {
                    installer::exiter::exit_program("Invalid setup script file. End of file reached before 'End' line of '$searchitem' section.", "get_all_items_from_script");
                }
                $line = ${$scriptref}[$counter];
                $counter++;

                if ( $line =~ /^\s*(.+?)\s*\=\s*(.+?)\s*\;\s*$/ )   # only oneliner!
                {
                    $itemkey = $1;
                    $itemvalue = $2;

                    installer::remover::remove_leading_and_ending_quotationmarks(\$itemvalue);
                    $itemvalue =~ s/\s*$//; # removing ending whitespaces. Could be introduced by empty variables.

                    $oneitemhash{$itemkey} = $itemvalue;

                    if ( $itemkey =~ /^\s*\S+\s+\(\S+\)\s*$/ )
                    {
                        $ismultilang = 1;
                    }
                }
                else
                {
                    if ( $searchitem eq "Module" ) # more than one line, for instance files at modules!
                    {
                        if (( $line =~ /^\s*(.+?)\s*\=\s*\(/ ) && (!($line =~ /\)\;\s*$ / )))
                        {
                            if ( $line =~ /^\s*(.+?)\s*\=\s*(.+)/ ) # the first line
                            {
                                $itemkey = $1;
                                $itemvalue = $2;
                                $itemvalue =~ s/\s*$//;
                            }

                            # collecting the complete itemvalue

                            $valuecounter = $counter;
                            $line = ${$scriptref}[$valuecounter];
                            installer::remover::remove_leading_and_ending_whitespaces(\$line);
                            $itemvalue = $itemvalue . $line;

                            while (!( $line =~ /\)\;\s*$/ ))
                            {
                                $valuecounter++;
                                $line = ${$scriptref}[$valuecounter];
                                installer::remover::remove_leading_and_ending_whitespaces(\$line);
                                $itemvalue = $itemvalue . $line;
                            }

                            # removing ending ";"
                            $itemvalue =~ s/\;\s*$//;

                            $oneitemhash{$itemkey} = $itemvalue;

                            if ( $itemkey =~ /^\s*\S+\s+\(\S+\)\s*$/ )
                            {
                                $ismultilang = 1;
                            }
                        }
                    }
                }
            }

            $oneitemhash{'ismultilingual'} = $ismultilang;

            push(@allitemarray, \%oneitemhash);
        }
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

    for ( my $i = 0; $i <= $#{$folderitemref}; $i++ )
    {
        my $folderitem = ${$folderitemref}[$i];
        my $folderid = $folderitem->{'FolderID'};

        if ( $folderid =~ /PREDEFINED_/ )
        {
            if (! installer::existence::exists_in_array_of_hashes("gid", $folderid, $folderref))
            {
                my %folder = ();
                $folder{'ismultilingual'} = "0";
                $folder{'Name'} = "";
                $folder{'gid'} = $folderid;

                push(@{$folderref}, \%folder);
            }
        }
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

sub add_installationobject_to_variables ($$)
{
    my ($variables, $script_variables) = @_;

    while (my ($key, $value) = each %$script_variables)
    {
        $variables->{$key} = $value;    # overwrite existing values from zip.lst
    }
}

#####################################################################################
# Adding all variables, that must be defined, but are not defined until now.
# List of this varibles: @installer::globals::forced_properties
#####################################################################################

sub add_forced_properties
{
    my ($allvariables) = @_;

    my $property;
    foreach $property ( @installer::globals::forced_properties )
    {
        if ( ! exists($allvariables->{$property}) ) { $allvariables->{$property} = ""; }
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


    foreach my $property (@presetproperties)
    {
        my $presetproperty = "PRESET" . $property;
        if (( exists($allvariables->{$presetproperty}) ) && ( $allvariables->{$presetproperty} ne "" ))
        {
            $allvariables->{$property} = $allvariables->{$presetproperty};
        }
    }
}

1;
