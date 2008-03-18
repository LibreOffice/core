#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: setupscript.pm,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:01:43 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

package installer::setupscript;

use installer::existence;
use installer::exiter;
use installer::globals;
use installer::remover;
use installer::scriptitems;
use installer::ziplist;

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
                $value =~ s/\s/\_/g;
                $newline = "\%UNIXPRODUCTNAME " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
                $value = $original;
                $value =~ s/\s/\_/g;
                $value =~ s/\.//g;
                $newline = "\%WITHOUTDOTUNIXPRODUCTNAME " . lc($value) . "\n";
                push(@{$variablesref} ,$newline);
            }
            elsif  ( $key eq "PRODUCTEXTENSION" )
            {
                my $newline = "\%LCPRODUCTEXTENSION " . lc($value) . "\n";
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

    for ( my $i = 0; $i <= $#{$scriptref}; $i++ )
    {
        my $line = ${$scriptref}[$i];

        if ( $line =~ /^.*\%\w+.*$/ )   # only oif "%" occurs
        {
            # Attention: It must be possible to substitute "%PRODUCTNAMEn", "%PRODUCTNAME%PRODUCTVERSIONabc"

            for ( my $j = 0; $j <= $#{$variablesref}; $j++ )
            {
                my $variableline = ${$variablesref}[$j];

                my ($key, $value);

                if ( $variableline =~ /^\s*(\%\w+?)\s+(.*?)\s*$/ )
                {
                    $key = $1;
                    $value = $2;
                }

                $line =~ s /$key/$value/g;

                ${$scriptref}[$i] = $line;
            }
        }
    }
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

1;
