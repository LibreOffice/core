#*************************************************************************
#
#   $RCSfile: setupscript.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2004-07-06 14:59:45 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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
                $line = ${$scriptref}[$counter];
                $counter++;

                if ( $line =~ /^\s*(.+?)\s*\=\s*(.+?)\s*\;\s*$/ )   # only oneliner!
                {
                    $itemkey = $1;
                    $itemvalue = $2;

                    installer::remover::remove_leading_and_ending_quotationmarks(\$itemvalue);

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

                            # Not including these to itemvalues, makes the module collection readable in editor
                            # if (($gid eq "gid_Module_Root") && ($itemkey eq "Files")) { $itemvalue = "rootfiles"; }
                            # if (($gid eq "gid_Module_Root") && ($itemkey eq "Dirs")) { $itemvalue = "rootdirs"; }

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
            if ( ! $installer::globals::iswindowsbuild )
            {
                $value =~ s/\s/\_/g;     # no spaces allowed for pathes on Unix
            }

            $allvariables->{$key} = $value; # overwrite existing values from zip.lst
        }
    }
}

1;
