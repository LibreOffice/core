#*************************************************************************
#
#   $RCSfile: download.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2004-12-16 10:43:29 $
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

package installer::download;

use installer::globals;
use installer::pathanalyzer;
use installer::systemactions;

# use installer::control;
# use installer::converter;
# use installer::files;
# use installer::logger;
# use installer::mail;
# use installer::worker;

##################################################################
# Including the lowercase product name into the script template
##################################################################

sub put_productname_into_script
{
    my ($scriptfile, $variableshashref) = @_;

    my $productname = $variableshashref->{'PRODUCTNAME'};
    $productname = lc($productname);
    $productname =~ s/\.//g;    # openoffice.org -> openofficeorg

    my $infoline = "Adding productname $productname into language pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/PRODUCTNAMEPLACEHOLDER/$productname/;
    }
}

#########################################################
# Including the linenumber into the script template
#########################################################

sub put_linenumber_into_script
{
    my ( $scriptfile ) = @_;

    my $linenumber =  $#{$scriptfile} + 2;

    my $infoline = "Adding linenumber $linenumber into language pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/LINENUMBERPLACEHOLDER/$linenumber/;
    }
}

#########################################################
# Determining the name of the new scriptfile
#########################################################

sub determine_scriptfile_name
{
    my ( $filename ) = @_;

    $filename = $filename . "\.sh";

    my $infoline = "Setting download shell script file name to $filename\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $filename;
}

#########################################################
# Saving the script file in the installation directory
#########################################################

sub save_script_file
{
    my ($directory, $newscriptfilename, $scriptfile) = @_;

    $newscriptfilename = $directory . $installer::globals::separator . $newscriptfilename;
    installer::files::save_file($newscriptfilename, $scriptfile);

    my $infoline = "Saving script file $newscriptfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $newscriptfilename;
}

#########################################################
# Including the binary package into the script
#########################################################

sub include_package_into_script
{
    my ( $installdir, $scriptfilename) = @_;

    my $systemcall = "cd $installdir; tar -cf - * >> $scriptfilename";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    my $localcall = "chmod 775 $scriptfilename \>\/dev\/null 2\>\&1";
    system($localcall);
}

#########################################################
# Setting the variables in the download name
#########################################################

sub resolve_variables_in_downloadname
{
    my ($allvariables, $downloadname, $languagestringref) = @_;

    # Typical name: soa-{productversion}-{extension}-bin-{os}-{languages}

    my $productversion = "";
    if ( $allvariables->{'PRODUCTVERSION'} ) { $productversion = $allvariables->{'PRODUCTVERSION'}; }
    $downloadname =~ s/\{productversion\}/$productversion/;

    my $extension = "";
    if ( $allvariables->{'SHORT_PRODUCTEXTENSION'} ) { $extension = $allvariables->{'SHORT_PRODUCTEXTENSION'}; }
    $extension = lc($extension);
    $downloadname =~ s/\{extension\}/$extension/;

    my $os = "";
    if ( $installer::globals::iswindowsbuild ) { $os = "windows"; }
    elsif ( $installer::globals::issolarissparcbuild ) { $os = "solsparc"; }
    elsif ( $installer::globals::issolarisx86build ) { $os = "solia"; }
    elsif ( $installer::globals::islinuxbuild ) { $os = "linux"; }
    else { $os = ""; }
    $downloadname =~ s/\{os\}/$os/;

    my $languages = $$languagestringref;
    $downloadname =~ s/\{languages\}/$languages/;

    $downloadname =~ s/\-\-\-/\-/g;
    $downloadname =~ s/\-\-/\-/g;
    $downloadname =~ s/\-\s*$//;

    return $downloadname;
}

####################################################
# Creating download installation sets
####################################################

sub create_download_sets
{
    my ($installationdir, $includepatharrayref, $allvariableshashref, $downloadname, $languagestringref) = @_;

    print "\n******************************************\n";
    print "... creating download installation set ...\n";
    print "******************************************\n";

    installer::logger::include_header_into_logfile("Creating download installation sets:");

    my $firstdir = $installationdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$firstdir);

    my $lastdir = $installationdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$lastdir);

    if ( $lastdir =~ /\./ ) { $downloaddir =~ s/\./_download_inprogress\./ }
    else { $lastdir = $lastdir . "_download_inprogress"; }

    # removing existing directory "_native_packed_inprogress" and "_native_packed_witherror" and "_native_packed"

    my $downloaddir = $firstdir . $lastdir;
    if ( -d $downloaddir ) { installer::systemactions::remove_complete_directory($downloaddir); }

    my $olddir = $downloaddir;
    $olddir =~ s/_inprogress/_witherror/;
    if ( -d $olddir ) { installer::systemactions::remove_complete_directory($olddir); }

    $olddir = $downloaddir;
    $olddir =~ s/_inprogress//;
    if ( -d $olddir ) { installer::systemactions::remove_complete_directory($olddir); }

    # creating the new directory

    installer::systemactions::create_directory($downloaddir);

    # evaluating the name of the download file

    $downloadname = resolve_variables_in_downloadname($allvariableshashref, $downloadname, $languagestringref);

    if ( ! $installer::globals::iswindowsbuild )    # Unix specific part
    {
        # find and read setup script template
        my $scriptfilename = "downloadscript.sh";
        my $scriptref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$scriptfilename, $includepatharrayref, 0);
        if ($$scriptref eq "") { installer::exiter::exit_program("ERROR: Could not find script file $scriptfilename!", "create_download_sets"); }
        my $scriptfile = installer::files::read_file($$scriptref);

        my $infoline = "Found  script file $scriptfilename: $$scriptref \n";
        push( @installer::globals::logfileinfo, $infoline);

        # add product name into script template
        put_productname_into_script($scriptfile, $allvariableshashref);

        # replace linenumber in script template
        put_linenumber_into_script($scriptfile);

        # saving the script file
        my $newscriptfilename = determine_scriptfile_name($downloadname);

        print "... including installation set into $newscriptfilename ... \n";

        $newscriptfilename = save_script_file($downloaddir, $newscriptfilename, $scriptfile);
        include_package_into_script($installationdir, $newscriptfilename);
    }
    else    # Windows specific part
    {
        # copying the installation set into a sub directory

        # print "... copying installation set ... \n";
        # my $subdirname = "packingdir";
        # my $subdir = $downloaddir . $installer::globals::separator . $subdirname;
        # installer::systemactions::create_directory($subdir);
        # installer::systemactions::copy_complete_directory($installationdir, $subdir);

        # remove old directory
        # installer::systemactions::remove_complete_directory($subdir);


    }

    return $downloaddir;
}

1;