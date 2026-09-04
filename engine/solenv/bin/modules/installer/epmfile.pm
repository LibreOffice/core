#
# This file is part of the Collabora Office project.
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

package installer::epmfile;

use strict;
use warnings;

use Cwd qw();
use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::packagelist;
use installer::pathanalyzer;
use installer::remover;
use installer::scpzipfiles;
use installer::scriptitems;
use installer::systemactions;
use POSIX;

# please Debian packaging, fdo#53341
sub debian_rewrite($)
{
    my $dep = shift;
    if ( $installer::globals::debian ) {
	$dep =~ s/_/-/g;  # Debian allows no underline in package name
	$dep = lc ($dep);
    }
    return $dep;
}

##########################################################
# Filling the epm file with directories, files and links
##########################################################

sub put_directories_into_epmfile
{
    my ($directoriesarrayref, $epmfileref, $allvariables, $packagerootpath) = @_;
    my $group = "bin";

    if ( $installer::globals::islinuxbuild )
    {
        $group = "root";
    }

    for ( my $i = 0; $i <= $#{$directoriesarrayref}; $i++ )
    {
        my $onedir = ${$directoriesarrayref}[$i];
        my $dir = "";

        if ( $onedir->{'Dir'} ) { $dir = $onedir->{'Dir'}; }

        if ((!($dir =~ /\bPREDEFINED_/ )) || ( $dir =~ /\bPREDEFINED_PROGDIR\b/ ))
        {
            my $hostname = $onedir->{'HostName'};

            my $line = "d 755 root $group $hostname -\n";

            push(@{$epmfileref}, $line)
        }
    }
}

sub put_files_into_epmfile
{
    my ($filesinproductarrayref, $epmfileref) = @_;

    for ( my $i = 0; $i <= $#{$filesinproductarrayref}; $i++ )
    {
        my $onefile = ${$filesinproductarrayref}[$i];

        my $unixrights = $onefile->{'UnixRights'};
        my $destination = $onefile->{'destination'};
        my $sourcepath = $onefile->{'sourcepath'};

        my $filetype = "f";
        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
        if ( $styles =~ /\bCONFIGFILE\b/ ) { $filetype = "c"; }

        my $group = "bin";
        if ( $installer::globals::islinuxbuild ) { $group = "root"; }

        my $line = "$filetype $unixrights root $group $destination $sourcepath\n";

        push(@{$epmfileref}, $line);
    }
}

sub put_files_into_dbgepmfile
{
    my ($filesinproductarrayref, $epmfileref) = @_;

    for ( my $i = 0; $i <= $#{$filesinproductarrayref}; $i++ )
    {
        my $onefile = ${$filesinproductarrayref}[$i];

        my $unixrights = $onefile->{'UnixRights'};
        my $destination = $onefile->{'destination'} . ".dbg";
        my $sourcepath = $onefile->{'sourcepath'} . ".dbg";
        my $filetype = "f";
        my $group = "root";

        my $line = "$filetype $unixrights root $group $destination $sourcepath\n";

        push(@{$epmfileref}, $line);

        # Add the build id lookup link. Tools that resolve debug info by
        # build id search /usr/lib/debug/.build-id/<first two hex>/<rest>.debug,
        # so point that path at the installed debug file.

        my $buildid = $onefile->{'buildid'} // "";
        if ( $buildid ne "" )
        {
            my $iddir = substr($buildid, 0, 2);
            my $idfile = substr($buildid, 2);
            my $linkdestination = "/usr/lib/debug/.build-id/$iddir/$idfile.debug";

            my $linkline = "l 000 root $group $linkdestination $destination\n";

            push(@{$epmfileref}, $linkline);
        }
    }
}

sub put_links_into_epmfile
{
    my ($linksinproductarrayref, $epmfileref) = @_;
    my $group = "bin";

    if ( $installer::globals::islinuxbuild )
    {
        $group = "root";
    }


    for ( my $i = 0; $i <= $#{$linksinproductarrayref}; $i++ )
    {
        my $onelink = ${$linksinproductarrayref}[$i];
        my $destination = $onelink->{'destination'};
        my $destinationfile = $onelink->{'destinationfile'};

        my $line = "l 000 root $group $destination $destinationfile\n";

        push(@{$epmfileref}, $line)
    }
}

sub put_unixlinks_into_epmfile
{
    my ($unixlinksinproductarrayref, $epmfileref) = @_;
    my $group = "bin";

    if ( $installer::globals::islinuxbuild ) { $group = "root"; }

    for ( my $i = 0; $i <= $#{$unixlinksinproductarrayref}; $i++ )
    {
        my $onelink = ${$unixlinksinproductarrayref}[$i];
        my $destination = $onelink->{'destination'};
        my $target = $onelink->{'Target'};

        my $line = "l 000 root $group $destination $target\n";

        push(@{$epmfileref}, $line)
    }
}

###############################################
# Creating epm header file
###############################################

sub create_epm_header
{
    my ($variableshashref, $filesinproduct, $languagesref, $onepackage) = @_;

    my @epmheader = ();

    my $licensefilename;

    my $foundlicensefile = 0;

    my $line = "";
    my $infoline = "";

    # %product LibreOffice Software
    # %version 2.0
    # %description A really great software
    # %copyright 1999-2003 by OOo
    # %vendor LibreOffice
    # %license /test/replace/01/LICENSE01
    # %requires foo
    # %provides bar
    # %replaces bar
    # %incompat bar

    # using the description for the %product line in the epm list file

    my $productnamestring = $onepackage->{'description'};
    installer::packagelist::resolve_packagevariables(\$productnamestring, $variableshashref, 0);

    $line = "%product" . " " . $productnamestring . "\n";
    push(@epmheader, $line);

    # Determining the release version
    # This release version has to be listed in the line %version : %version versionnumber releasenumber

    if ( ! $onepackage->{'packageversion'} ) { installer::exiter::exit_program("ERROR: No packageversion defined for package: $onepackage->{'module'}!", "create_epm_header"); }
    $installer::globals::packageversion = $onepackage->{'packageversion'};
    installer::packagelist::resolve_packagevariables(\$installer::globals::packageversion, $variableshashref, 0);
    if ( $variableshashref->{'PACKAGEREVISION'} ) { $installer::globals::packagerevision = $variableshashref->{'PACKAGEREVISION'}; }

    $line = "%version" . " " . $installer::globals::packageversion . "\n";
    push(@epmheader, $line);

    $line = "%release" . " " . $installer::globals::packagerevision . "\n";
    #if ( $installer::globals::isrpmbuild ) { $line = "%release" . " " . $installer::globals::buildid . "\n"; }
    push(@epmheader, $line);

    # Description, Copyright and Vendor are multilingual and are defined in
    # the string file for the header file ($headerfileref)

    my $descriptionstring = $onepackage->{'description'};
    installer::packagelist::resolve_packagevariables(\$descriptionstring, $variableshashref, 0);
    $line = "%description" . " " . $descriptionstring . "\n";
    push(@epmheader, $line);

    my $copyrightstring = $onepackage->{'copyright'};
    installer::packagelist::resolve_packagevariables(\$copyrightstring, $variableshashref, 0);
    $line = "%copyright" . " " . $copyrightstring . "\n";
    push(@epmheader, $line);

    my $vendorstring = $onepackage->{'vendor'};
    installer::packagelist::resolve_packagevariables(\$vendorstring, $variableshashref, 0);
    $line = "%vendor" . " " . $vendorstring . "\n";
    push(@epmheader, $line);

    # License file can be included automatically from the file list

    $licensefilename = "LICENSE";

    my $license_in_package_defined = 0;

    # Process for Linux packages, in which only a very basic license file is
    # included into the package.

    if ( $installer::globals::islinuxbuild )
    {
        if ( $variableshashref->{'COPYRIGHT_INTO_LINUXPACKAGE'} )
        {
            $licensefilename = "linuxcopyrightfile";
            $license_in_package_defined = 1;
        }
    }

    # searching for and license file

    if ( $license_in_package_defined )
    {
        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$licensefilename, "" , 0);

        if ( $$fileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find license file $licensefilename (A)!", "create_epm_header"); }

        $infoline = "Using license file: \"$$fileref\"!\n";
        push(@installer::globals::logfileinfo, $infoline);

        $foundlicensefile = 1;
        $line = "%license" . " " . $$fileref . "\n";
        push(@epmheader, $line);
    }
    else
    {
    for my $onefile (@{$filesinproduct})
    {
        if ($onefile->{'Name'} =~ /$licensefilename$/)
            {
                push @epmheader, "%license" . " " . $onefile->{'sourcepath'} . "\n";
                $foundlicensefile = 1;
                last;
            }
        }

        # the license file need not be packaged more times in the langpacks
        # they need to be installed in parallel with the main package anyway
        # try to find the LICENSE file between all available files (not only between the packaged)
        if (!($foundlicensefile) && ($installer::globals::languagepack || $installer::globals::helppack))
        {
            my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$licensefilename, "" , 0);
            if($$fileref ne "" )
            {
                $infoline = "Fallback to license file: \"$$fileref\"!\n";
                push(@installer::globals::logfileinfo, $infoline);

                $foundlicensefile = 1;
                $line = "%license" . " " . $$fileref . "\n";
                push(@epmheader, $line);
            }
        }
    }

    if (!($foundlicensefile))
    {
        installer::exiter::exit_program("ERROR: Could not find license file $licensefilename (B)", "create_epm_header");
    }

    # The patched epm in external/epm rejects list files without %readme,
    # even though deb/rpm/pkg targets ignore the value. Emit a placeholder.
    push(@epmheader, "%readme DUMMY\n");

    # including %replaces

    my $replaces = "";

    if ( $installer::globals::islinuxbuild )
    {
        $replaces = "linuxreplaces";    # the name in the packagelist
    }

    if ( $replaces )
    {
        if ( $onepackage->{$replaces} )
        {
            my $replacesstring = $onepackage->{$replaces};

            my $allreplaces = installer::converter::convert_stringlist_into_array(\$replacesstring, ",");

            for ( my $i = 0; $i <= $#{$allreplaces}; $i++ )
            {
                my $onereplaces = ${$allreplaces}[$i];
                $onereplaces =~ s/\s*$//;
                installer::packagelist::resolve_packagevariables(\$onereplaces, $variableshashref, 1);
		$onereplaces = debian_rewrite($onereplaces);
                $line = "%replaces" . " " . $onereplaces . "\n";
                push(@epmheader, $line);
            }
        }
    }

    # including %incompat

    my $incompat = "";

    if (( $installer::globals::islinuxbuild ) && ( ! $installer::globals::patch ))
    {
        $incompat = "linuxincompat";    # the name in the packagelist
    }

    if (( $incompat ) && ( ! $installer::globals::patch ))
    {
        if ( $onepackage->{$incompat} )
        {
            my $incompatstring = $onepackage->{$incompat};

            my $allincompat = installer::converter::convert_stringlist_into_array(\$incompatstring, ",");

            for ( my $i = 0; $i <= $#{$allincompat}; $i++ )
            {
                my $oneincompat = ${$allincompat}[$i];
                $oneincompat =~ s/\s*$//;
                installer::packagelist::resolve_packagevariables(\$oneincompat, $variableshashref, 1);
                $oneincompat = debian_rewrite($oneincompat);
                $line = "%incompat" . " " . $oneincompat . "\n";
                push(@epmheader, $line);
            }
        }
    }

    # including the directives for %requires and %provides

    my $provides = "";
    my $requires = "";

    if ( $installer::globals::isfreebsdpkgbuild )
    {
        $provides = "freebsdprovides";   # the name in the packagelist
        $requires = "freebsdrequires";   # the name in the packagelist
    }
    else
    {
        $provides = "provides";         # the name in the packagelist
        $requires = "requires";         # the name in the packagelist
    }

        my $isdict = 0;
        if ( $onepackage->{'packagename'} =~ /-dict-/ ) { $isdict = 1;  }

    if ( $onepackage->{$provides} )
    {
        my $providesstring = $onepackage->{$provides};

        my $allprovides = installer::converter::convert_stringlist_into_array(\$providesstring, ",");

        for ( my $i = 0; $i <= $#{$allprovides}; $i++ )
        {
            my $oneprovides = ${$allprovides}[$i];
            $oneprovides =~ s/\s*$//;
            installer::packagelist::resolve_packagevariables(\$oneprovides, $variableshashref, 1);
	    $oneprovides = debian_rewrite($oneprovides);
            $line = "%provides" . " " . $oneprovides . "\n";
            push(@epmheader, $line);
        }
    }

    if ( $onepackage->{$requires} )
    {
        my $requiresstring = $onepackage->{$requires};

        # The requires string can contain the separator "," in the names (descriptions) of the packages.
        # Therefore "," inside such a description has to be masked with a backslash.
        # This masked separator need to be found and replaced, before the stringlist is converted into an array.
        # This replacement has to be turned back after the array is created.

        my $replacementstring = "COMMAREPLACEMENT";
        $requiresstring = installer::converter::replace_masked_separator($requiresstring, ",", "$replacementstring");

        my $allrequires = installer::converter::convert_stringlist_into_array(\$requiresstring, ",");

        installer::converter::resolve_masked_separator($allrequires, ",", $replacementstring);

        for ( my $i = 0; $i <= $#{$allrequires}; $i++ )
        {
            my $onerequires = ${$allrequires}[$i];
            $onerequires =~ s/\s*$//;
            installer::packagelist::resolve_packagevariables2(\$onerequires, $variableshashref, 0, $isdict);
	    $onerequires = debian_rewrite($onerequires);
            $line = "%requires" . " " . $onerequires . "\n";
            push(@epmheader, $line);
        }
    }

    return \@epmheader;
}

#######################################
# Adding header to epm file
#######################################

sub adding_header_to_epm_file
{
    my ($epmfileref, $epmheaderref) = @_;

    for ( my $i = 0; $i <= $#{$epmheaderref}; $i++ )
    {
        push( @{$epmfileref}, ${$epmheaderref}[$i] );
    }

    push( @{$epmfileref}, "\n\n" );
}

#####################################################
# Replace one in shell scripts ( ${VARIABLENAME} )
#####################################################

sub replace_variable_in_shellscripts
{
    my ($scriptref, $variable, $searchstring) = @_;

    for ( my $i = 0; $i <= $#{$scriptref}; $i++ )
    {
        ${$scriptref}[$i] =~ s/\$\{$searchstring\}/$variable/g;
    }
}

################################################
# Replacing many variables in shell scripts
################################################

sub replace_many_variables_in_shellscripts
{
    my ($scriptref, $variableshashref) = @_;

    my $key;

    foreach $key (keys %{$variableshashref})
    {
        my $value = $variableshashref->{$key};
        replace_variable_in_shellscripts($scriptref, $value, $key);
    }
}

#######################################
# Adding shell scripts to epm file
#######################################

sub adding_shellscripts_to_epm_file
{
    my ($epmfileref, $shellscriptsfilename, $localrootpath, $allvariableshashref, $filesinpackage) = @_;

    push( @{$epmfileref}, "\n\n" );

    my $shellscriptsfileref = installer::files::read_file($shellscriptsfilename);

    replace_variable_in_shellscripts($shellscriptsfileref, $localrootpath, "rootpath");

    replace_many_variables_in_shellscripts($shellscriptsfileref, $allvariableshashref);

    for ( my $i = 0; $i <= $#{$shellscriptsfileref}; $i++ )
    {
        push( @{$epmfileref}, ${$shellscriptsfileref}[$i] );
    }

    push( @{$epmfileref}, "\n" );
}

#################################################
# Determining the epm on the system
#################################################

sub find_epm_on_system
{
    my ($includepatharrayref) = @_;

    installer::logger::include_header_into_logfile("Check epm on system");

    my $epmname = "epm";

    # epm should be defined through the configure script but we need to
    # check for it to be defined because of the Sun environment.
    # Check the environment variable first and if it is not defined,
    # or if it is but the location is not executable, search further.
    # It has to be found in the solver or it has to be in the path
    # (saved in $installer::globals::epm_in_path) or we get the specified
    # one through the environment (i.e. when --with-epm=... is specified)

    if ($ENV{'EPM'})
    {
        if (($ENV{'EPM'} ne "") && (-x "$ENV{'EPM'}"))
        {
            $epmname = $ENV{'EPM'};
        }
        else
        {
            installer::exiter::exit_program("Environment variable EPM set (\"$ENV{'EPM'}\"), but file does not exist or is not executable!", "find_epm_on_system");
        }
    }
    else
    {
        my $epmfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$epmname, $includepatharrayref, 0);

        if (($$epmfileref eq "") && (!($installer::globals::epm_in_path))) { installer::exiter::exit_program("ERROR: Could not find program $epmname!", "find_epm_on_system"); }
        if (($$epmfileref eq "") && ($installer::globals::epm_in_path)) { $epmname = $installer::globals::epm_path; }
        if (!($$epmfileref eq "")) { $epmname = $$epmfileref; }
    }

    my $infoline = "Using epmfile: $epmname\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $epmname;
}

#################################################
# Determining the epm patch state
# saved in $installer::globals::is_special_epm
#################################################

sub set_patch_state
{
    my ($epmexecutable) = @_;

    my $infoline = "";

    my $systemcall = "$epmexecutable |";
    open (EPMPATCH, "$systemcall");

    while (<EPMPATCH>)
    {
        chop;
        if ( $_ =~ /Patched for .*Office/ ) { $installer::globals::is_special_epm = 1; }
    }

    close (EPMPATCH);

    if ( $installer::globals::is_special_epm )
    {
        $infoline = "\nPatch state: This is a patched version of epm!\n\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "\nPatch state: This is an unpatched version of epm!\n\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    if ( ( $installer::globals::is_special_epm ) && ( $installer::globals::isrpmbuild ) )
    {
        # Special postprocess handling only for Linux RPM packages
        $installer::globals::postprocess_specialepm = 1;
        $installer::globals::postprocess_standardepm = 0;
    }
    else
    {
        $installer::globals::postprocess_specialepm = 0;
        $installer::globals::postprocess_standardepm = 1;
    }
}

#################################################
# Calling epm to create the installation sets
#################################################

sub call_epm
{
    my ($epmname, $epmlistfilename, $packagename, $includepatharrayref) = @_;

    installer::logger::include_header_into_logfile("epm call for $packagename");

    my $packageformat = $installer::globals::packageformat;

    my $localpackagename = $packagename;
    # Debian allows only lowercase letters in package name
    if ( $installer::globals::debian ) { $localpackagename = lc($localpackagename); }

    my $outdirstring = "";
    if ( $installer::globals::epmoutpath ne "" ) { $outdirstring = " --output-dir $installer::globals::epmoutpath"; }

    # Debian package build needs to be run with fakeroot for correct ownerships/permissions

    my $fakerootstring = "";

    if ( $installer::globals::debian ) { $fakerootstring = "fakeroot "; }

    my $extraflags = "";
        if ($ENV{'EPM_FLAGS'}) { $extraflags = $ENV{'EPM_FLAGS'}; }

    $extraflags .= ' -g' unless $installer::globals::strip;
    $extraflags .= ' --keep-files' if $installer::globals::debian;

    my $verboseflag = "-v";
    if ( ! $installer::globals::quiet ) { $verboseflag = "-v2"; };

    my $systemcall = $fakerootstring . $epmname . " -f " . $packageformat . " " . $extraflags . " " . $localpackagename . " " . $epmlistfilename . $outdirstring . " " . $verboseflag . " " . " 2\>\&1 |";

    installer::logger::print_message( "... $systemcall ...\n" );

    my $maxepmcalls = 3;

    for ( my $i = 1; $i <= $maxepmcalls; $i++ )
    {
        my @epmoutput = ();

        open (EPM, "$systemcall");
        while (<EPM>) {push(@epmoutput, $_); }
        close (EPM);

        my $returnvalue = $?;   # $? contains the return value of the systemcall

        my $infoline = "Systemcall  (Try $i): $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        for ( my $j = 0; $j <= $#epmoutput; $j++ )
        {
            if ( $i < $maxepmcalls ) { $epmoutput[$j] =~ s/\bERROR\b/PROBLEM/ig; }
            push( @installer::globals::logfileinfo, "$epmoutput[$j]");
        }

        if ($returnvalue)
        {
            $infoline = "Try $i : Could not execute \"$systemcall\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
            if ( $i == $maxepmcalls ) { installer::exiter::exit_program("ERROR: \"$systemcall\"!", "call_epm"); }
        }
        else
        {
            installer::logger::print_message( "Success (Try $i): \"$systemcall\"\n" );
            $infoline = "Success: Executed \"$systemcall\" successfully!\n";
            push( @installer::globals::logfileinfo, $infoline);
            last;
        }
    }
}

#####################################################################
# Adding the new line for relocatables into the spec file (Linux)
# created by epm
#####################################################################

sub add_one_line_into_file
{
    my ($file, $insertline, $filename) = @_;

    if ( $installer::globals::isrpmbuild )
    {
        # Adding behind the line beginning with: Group:

        my $inserted_line = 0;

        for ( my $i = 0; $i <= $#{$file}; $i++ )
        {
            if ( ${$file}[$i] =~ /^\s*Group\:\s*/ )
            {
                splice(@{$file},$i+1,0,$insertline);
                $inserted_line = 1;
                last;
            }
        }

        if (! $inserted_line) { installer::exiter::exit_program("ERROR: Did not find string \"Group:\" in file: $filename", "add_one_line_into_file"); }
    }

    $insertline =~ s/\s*$//;    # removing line end for correct logging
    my $infoline = "Success: Added line $insertline into file $filename!\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#####################################################################
# Adding a new line for topdir into specfile, removing old
# topdir if set.
#####################################################################

sub set_topdir_in_specfile
{
    my ($changefile, $filename, $newepmdir) = @_;

    $newepmdir = Cwd::cwd() . $installer::globals::separator . $newepmdir; # only absolute path allowed

    # removing "%define _topdir", if existing

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        if ( ${$changefile}[$i] =~ /^\s*\%define _topdir\s+/ )
        {
            my $removeline = ${$changefile}[$i];
            $removeline =~ s/\s*$//;
            splice(@{$changefile},$i,1);
            my $infoline = "Info: Removed line \"$removeline\" from file $filename!\n";
            push( @installer::globals::logfileinfo, $infoline);
            last;
        }
    }

    # Adding "topdir" behind the line beginning with: Group:

    my $inserted_line = 0;

    my $topdirline = "\%define _topdir $newepmdir\n";

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        if ( ${$changefile}[$i] =~ /^\s*Group\:\s*/ )
        {
            splice(@{$changefile},$i+1,0,$topdirline);
            $inserted_line = 1;
            $topdirline =~ s/\s*$//;
            my $infoline = "Success: Added line $topdirline into file $filename!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    if (! $inserted_line) { installer::exiter::exit_program("ERROR: Did not find string \"Group:\" in file: $filename", "set_topdir_in_specfile"); }

}

#####################################################################
# Setting the packager in the spec file
# Syntax: Packager: abc@def
#####################################################################

sub set_packager_in_specfile
{
    my ($changefile) = @_;

    my $packager = $installer::globals::longmanufacturer;

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        if ( ${$changefile}[$i] =~ /^\s*Packager\s*:\s*(.+?)\s*$/ )
        {
            my $oldstring = $1;
            ${$changefile}[$i] =~ s/\Q$oldstring\E/$packager/;
            my $infoline = "Info: Changed Packager in spec file from $oldstring to $packager!\n";
            push( @installer::globals::logfileinfo, $infoline);
            last;
        }
    }
}

#####################################################################
# Setting the requirements in the spec file (i81494)
# Syntax: PreReq: "requirements" (only for shared extensions)
#####################################################################

sub set_prereq_in_specfile
{
    my ($changefile) = @_;

    my $prereq = "PreReq:";

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        if ( ${$changefile}[$i] =~ /^\s*Requires:\s*(.+?)\s*$/ )
        {
            my $oldstring = ${$changefile}[$i];
            ${$changefile}[$i] =~ s/Requires:/$prereq/;
            my $infoline = "Info: Changed requirements in spec file from $oldstring to ${$changefile}[$i]!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

#####################################################################
# Setting the Auto[Req]Prov line and __find_requires
#####################################################################

sub set_autoprovreq_in_specfile
{
    my ($changefile, $findrequires, $bindir) = @_;

    my $autoreqprovline = "AutoReqProv\: no\n";

    if ( $findrequires )
    {
        # don't let rpm invoke it, we never want to use AutoReq because
        # rpm will generate Requires: config(packagename)
        open (FINDREQUIRES, "echo | $bindir/$findrequires |");
        while (<FINDREQUIRES>) { $autoreqprovline .= "Requires: $_\n"; }
        close (FINDREQUIRES);
    }

    $autoreqprovline .= "%define _binary_filedigest_algorithm 8\n%define _binary_payload w2.xzdio\n";

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        # Adding "autoreqprov" behind the line beginning with: Group:
        if ( ${$changefile}[$i] =~ /^\s*Group\:\s*/ )
        {
            splice(@{$changefile},$i+1,0,$autoreqprovline);
            $autoreqprovline =~ s/\s*$//;
            my $infoline = "Success: Added line $autoreqprovline into spec file!\n";
            push( @installer::globals::logfileinfo, $infoline);

            last;
        }
    }
}

#####################################################################
# Replacing Copyright with License in the spec file
# Syntax: License: LGPLv3 (or MPLv2 on ALv2, older usages were LGPL, SISSL)
#####################################################################

sub set_license_in_specfile
{
    my ($changefile, $variableshashref) = @_;

    my $license = $variableshashref->{'LICENSENAME'};

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        if ( ${$changefile}[$i] =~ /^\s*Copyright\s*:\s*(.+?)\s*$/ )
        {
            ${$changefile}[$i] = "License: $license\n";
            my $infoline = "Info: Replaced Copyright with License: $license !\n";
            push( @installer::globals::logfileinfo, $infoline);
            last;
        }
    }
}

#########################################################################
# Replacing the variables in the shell scripts or in the epm list file
# Linux: spec file
# If epm is used in the original version (not relocatable)
# the variables have to be exchanged in the list file,
# created for epm.
#########################################################################

sub replace_variables_in_shellscripts
{
    my ($scriptfile, $scriptfilename, $oldstring, $newstring) = @_;

    my $debug = 0;
    if ( $oldstring eq "PRODUCTDIRECTORYNAME" ) { $debug = 1; }

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        if ( ${$scriptfile}[$i] =~ /\Q$oldstring\E/ )
        {
            my $oldline = ${$scriptfile}[$i];
            ${$scriptfile}[$i] =~ s/\Q$oldstring\E/$newstring/g;
            ${$scriptfile}[$i] =~ s/\/\//\//g;  # replacing "//" by "/" , if path $newstring is empty!
            my $infoline = "Info: Substituting in $scriptfilename $oldstring by $newstring\n";
            push(@installer::globals::logfileinfo, $infoline);
            if ( $debug )
            {
                $infoline = "Old Line: $oldline";
                push(@installer::globals::logfileinfo, $infoline);
                $infoline = "New Line: ${$scriptfile}[$i]";
                push(@installer::globals::logfileinfo, $infoline);
            }
        }
    }
}

############################################################
# Determining the directory created by epm, in which the
# RPMS are created.
############################################################

sub determine_installdir_ooo
{
    # A simple "ls" command returns the directory name

    my $dirname = "";

    my $systemcall = "ls |";
    open (LS, "$systemcall");
    $dirname = <LS>;
    close (LS);

    $dirname =~ s/\s*$//;

    my $infoline = "Info: Directory created by epm: $dirname\n";
    push(@installer::globals::logfileinfo, $infoline);

    return $dirname;
}

############################################################
# Setting the tab content into the file container
############################################################

sub set_tab_into_datafile
{
    my ($changefile, $filesref) = @_;

    if ( $installer::globals::isrpmbuild )
    {
        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            my $onefile = ${$filesref}[$i];

            if ( $onefile->{'SpecFileContent'} )
            {
                my $destination = $onefile->{'destination'};

                for ( my $j = 0; $j <= $#{$changefile}; $j++ )
                {
                    if ( ${$changefile}[$j] =~ /^\s*(\%attr\(.*\))\s+(\".*?\Q$destination\E\"\s*)$/ )
                    {
                        my $begin = $1;
                        my $end = $2;

                        my $oldline = ${$changefile}[$j];
                        ${$changefile}[$j] = $begin . " " . $onefile->{'SpecFileContent'} . " " . $end;
                        my $newline = ${$changefile}[$j];

                        $oldline =~ s/\s*$//;
                        $newline =~ s/\s*$//;

                        my $infoline = "TAB: Changing content from \"$oldline\" to \"$newline\" .\n";
                        push(@installer::globals::logfileinfo, $infoline);

                        last;
                    }
                }
            }
        }
    }

}

##########################################################################################
# Checking, if an extension is included into the package (Linux).
# All extension files have to be installed into directory
# share/extension/install
# %attr(0444,root,root) "/opt/staroffice8/share/extension/install/SunSearchToolbar.oxt"
##########################################################################################

sub is_extension_package
{
    my ($specfile) = @_;

    my $is_extension_package = 0;

    for ( my $i = 0; $i <= $#{$specfile}; $i++ )
    {
        my $line = ${$specfile}[$i];
        if ( $line =~ /share\/extension\/install\/.*?\.oxt\"\s*$/ )
        {
            $is_extension_package = 1;
            last;
        }
    }

    return $is_extension_package;
}

############################################################
# Including the relocatable directory into the
# spec file and setting "topdir" for Linux
############################################################

sub prepare_packages
{
    my ($loggingdir, $packagename, $staticpath, $relocatablepath, $onepackage, $variableshashref, $filesref, $languagestringref) = @_;

    my $filename = "";
    my $newline = "";
    my $newepmdir = $installer::globals::epmoutpath . $installer::globals::separator;

    my $localrelocatablepath = $relocatablepath;
    if ( $localrelocatablepath ne "/" ) { $localrelocatablepath =~ s/\/\s*$//; }

    if ( $installer::globals::isrpmbuild )
    {
        $filename =  $packagename . ".spec";
        $newline = "Prefix\:\ " . $localrelocatablepath . "\n";
    }

    my $completefilename = $newepmdir . $filename;

    if ( ! -f $completefilename) { installer::exiter::exit_program("ERROR: Did not find file: $completefilename", "prepare_packages"); }
    my $changefile = installer::files::read_file($completefilename);
    if ( $newline ne "" )
    {
        add_one_line_into_file($changefile, $newline, $filename);
        installer::files::save_file($completefilename, $changefile);
    }

    # adding new "topdir" and removing old "topdir" in specfile

    if ( $installer::globals::isrpmbuild )
    {
        set_topdir_in_specfile($changefile, $filename, $newepmdir);
        set_autoprovreq_in_specfile($changefile, $onepackage->{'findrequires'}, "$installer::globals::workpath" . "/bin");
        set_packager_in_specfile($changefile);
        if ( is_extension_package($changefile) ) { set_prereq_in_specfile($changefile); }
        set_license_in_specfile($changefile, $variableshashref);
        set_tab_into_datafile($changefile, $filesref);
        installer::files::save_file($completefilename, $changefile);
    }

    return $newepmdir;
}

###############################################################################
# Replacement of PRODUCTINSTALLLOCATION and PRODUCTDIRECTORYNAME in the
# epm list file.
# The complete rootpath is stored in $installer::globals::rootpath
# or for each package in $onepackage->{'destpath'}
# The static rootpath is stored in $staticpath
# The relocatable path is stored in $relocatablepath
# PRODUCTINSTALLLOCATION is the relocatable part ("/opt") and
# PRODUCTDIRECTORYNAME the static path ("openofficeorg20").
# In standard epm process:
# No usage of package specific variables like $BASEDIR, because
# 1. These variables would be replaced in epm process
# 2. epm version 3.7 does not support relocatable packages
###############################################################################

sub resolve_path_in_epm_list_before_packaging
{
    my ($listfile, $listfilename, $variable, $path) = @_;

    installer::logger::include_header_into_logfile("Replacing variables in epm list file:");

    $path =~ s/\/\s*$//;
    replace_variables_in_shellscripts($listfile, $listfilename, $variable, $path);

}

#################################################################
# Determining the rpm version. Beginning with rpm version 4.0
# the tool to create RPMs is "rpmbuild" and no longer "rpm"
#################################################################

sub determine_rpm_version
{
    my $rpmversion = 0;
    my $rpmout = "";
    my $systemcall = "";

    # "rpm --version" has problems since LD_LIBRARY_PATH was removed. Therefore the content of $RPM has to be called.
    # "rpm --version" and "rpmbuild --version" have the same output. Therefore $RPM can be used. Its value
    # is saved in $installer::globals::rpm

    if ( $installer::globals::rpm ne "" )
    {
        $systemcall = "$installer::globals::rpm --version |";
    }
    else
    {
        $systemcall = "rpm --version |";
    }

    open (RPM, "$systemcall");
    $rpmout = <RPM>;
    close (RPM);

    if ( $rpmout ne "" )
    {
        $rpmout =~ s/\s*$//g;

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ( $rpmout eq "" ) { $infoline = "ERROR: Could not find file \"rpm\" !\n"; }
        else { $infoline = "Success: rpm version: $rpmout\n"; }

        push( @installer::globals::logfileinfo, $infoline);

        if ( $rpmout =~ /(\d+)\.(\d+)\.(\d+)/ ) { $rpmversion = $1; }
        elsif ( $rpmout =~ /(\d+)\.(\d+)/ ) { $rpmversion = $1; }
        elsif ( $rpmout =~ /(\d+)/ ) { $rpmversion = $1; }
        else { installer::exiter::exit_program("ERROR: Unknown format: $rpmout ! Expected: \"a.b.c\", or \"a.b\", or \"a\"", "determine_rpm_version"); }
    }

    return $rpmversion;
}

####################################################
# Writing some info about rpm into the log file
####################################################

sub log_rpm_info
{
    my $systemcall = "";
    my $infoline = "";

    $infoline = "\nLogging rpmrc content using --showrc\n\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( $installer::globals::rpm ne "" )
    {
        $systemcall = "$installer::globals::rpm --showrc |";
    }
    else
    {
        $systemcall = "rpm --showrc |";
    }

    my @fullrpmout = ();

    open (RPM, "$systemcall");
    while (<RPM>) {push(@fullrpmout, $_); }
    close (RPM);

    if ( $#fullrpmout > -1 )
    {
        for ( my $i = 0; $i <= $#fullrpmout; $i++ )
        {
            my $rpmout = $fullrpmout[$i];
            $rpmout =~ s/\s*$//g;

            $infoline = "$rpmout\n";
            $infoline =~ s/error/e_r_r_o_r/gi;  # avoiding log problems
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
    else
    {
        $infoline = "Problem in systemcall: $systemcall : No return value\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    $infoline = "End of logging rpmrc\n\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#################################################
# Systemcall to start the packaging process
#################################################

sub create_packages_without_epm
{
    my ($epmdir, $packagename, $includepatharrayref, $allvariables, $languagestringref) = @_;

    # Linux: rpm -bb so8m35.spec    ( -> dependency check abklemmen? )

    if ( $installer::globals::isrpmbuild )
    {
        my $specfilename = $epmdir . $packagename . ".spec";
        if (! -f $specfilename) { installer::exiter::exit_program("ERROR: Did not find file: $specfilename", "create_packages_without_epm"); }

        my $rpmcommand = $installer::globals::rpm;
        my $rpmversion = determine_rpm_version();

        my $target = "";
        if ( $installer::globals::platformid eq 'linux_x86')
        {
            $target = "i586";
        }
        elsif ( $installer::globals::os eq 'LINUX')
        {
            $target = (POSIX::uname())[4];
        }

        # rpm 4.6 ignores buildroot tag in spec file

        my $buildrootstring = "";

        if ( $rpmversion >= 4 )
        {
            my $dir = Cwd::getcwd;
            my $buildroot = $dir . "/" . $epmdir . "buildroot/";
            $buildrootstring = "--buildroot=$buildroot";
            mkdir($buildroot = $dir . "/" . $epmdir . "BUILD/");
        }

        if ( ! $installer::globals::rpminfologged )
        {
            log_rpm_info();
            $installer::globals::rpminfologged = 1;
        }

        my $systemcall = "$rpmcommand -bb --define \"_unpackaged_files_terminate_build  0\" --define \"_build_id_links none\" $specfilename --target $target $buildrootstring 2\>\&1 |";

        installer::logger::print_message( "... $systemcall ...\n" );

        my $maxrpmcalls = 3;
        my $rpm_failed = 0;

        for ( my $i = 1; $i <= $maxrpmcalls; $i++ )
        {
            my @rpmoutput = ();

            open (RPM, "$systemcall");
            while (<RPM>) {push(@rpmoutput, $_); }
            close (RPM);

            my $returnvalue = $?;   # $? contains the return value of the systemcall

            my $infoline = "Systemcall (Try $i): $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);

            for ( my $j = 0; $j <= $#rpmoutput; $j++ )
            {
                $rpmoutput[$j] =~ s/\bERROR\b/PROBLEM/ig;
                push( @installer::globals::logfileinfo, "$rpmoutput[$j]");
            }

            if ($returnvalue)
            {
                $infoline = "Try $i : Could not execute \"$systemcall\"!\n";
                push( @installer::globals::logfileinfo, $infoline);
                $rpm_failed = 1;
            }
            else
            {
                installer::logger::print_message( "Success (Try $i): \"$systemcall\"\n" );
                $infoline = "Success: Executed \"$systemcall\" successfully!\n";
                push( @installer::globals::logfileinfo, $infoline);
                $rpm_failed = 0;
                last;
            }
        }

        if ( $rpm_failed )
        {
            # Because of the problems with LD_LIBRARY_PATH, a direct call of local "rpm" or "rpmbuild" might be successful
            my $rpmprog = "";
            if ( -f "/usr/bin/rpmbuild" ) { $rpmprog = "/usr/bin/rpmbuild"; }
            elsif ( -f "/usr/bin/rpm" ) { $rpmprog = "/usr/bin/rpm"; }

            if ( $rpmprog ne "" )
            {
                installer::logger::print_message( "... $rpmprog ...\n" );

                my $helpersystemcall = "$rpmprog -bb $specfilename --target $target $buildrootstring 2\>\&1 |";

                my @helperrpmoutput = ();

                open (RPM, "$helpersystemcall");
                while (<RPM>) {push(@helperrpmoutput, $_); }
                close (RPM);

                my $helperreturnvalue = $?; # $? contains the return value of the systemcall

                my $infoline = "\nLast try: Using $rpmprog directly (problem with LD_LIBRARY_PATH)\n";
                push( @installer::globals::logfileinfo, $infoline);

                $infoline = "\nSystemcall: $helpersystemcall\n";
                push( @installer::globals::logfileinfo, $infoline);

                for ( my $j = 0; $j <= $#helperrpmoutput; $j++ ) { push( @installer::globals::logfileinfo, "$helperrpmoutput[$j]"); }

                if ($helperreturnvalue)
                {
                    $infoline = "Could not execute \"$helpersystemcall\"!\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }
                else
                {
                    installer::logger::print_message( "Success: \"$helpersystemcall\"\n" );
                    $infoline = "Success: Executed \"$helpersystemcall\" successfully!\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    $rpm_failed = 0;
                }
            }

            # Now it is really time to exit this packaging process, if the error still occurs
            if ( $rpm_failed ) { installer::exiter::exit_program("ERROR: \"$systemcall\"!", "create_packages_without_epm"); }
        }
    }
}

#################################################
# Removing all temporary files created by epm
#################################################

sub remove_temporary_epm_files
{
    my ($epmdir, $loggingdir, $packagename) = @_;

    # saving the files into the loggingdir

    if ( $installer::globals::isrpmbuild )
    {
        my $removefile = $epmdir . $packagename . ".spec";
        my $destfile = $loggingdir . $packagename . ".spec.log";

        my $systemcall = "mv -f $removefile $destfile";
        system($systemcall);     # ignoring the return value
        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        # removing the directory "buildroot"

        my $removedir = $epmdir . "buildroot";

        $systemcall = "rm -rf $removedir";

        installer::logger::print_message( "... $systemcall ...\n" );

        my $returnvalue = system($systemcall);

        $removedir = $epmdir . "BUILD";

        $systemcall = "rm -rf $removedir";

        installer::logger::print_message( "... $systemcall ...\n" );

        $returnvalue = system($systemcall);


        $infoline = "Systemcall: $systemcall\n";
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
    }
}

###########################################################
# Creating a better directory structure in the solver.
###########################################################

sub create_new_directory_structure
{
    my ($newepmdir) = @_;

    my $newdir = $installer::globals::epmoutpath;

    if ( $installer::globals::isrpmbuild )
    {
        my $rpmdir;
                my $machine = "";
        if ( $installer::globals::platformid eq 'linux_x86')
        {
            $rpmdir = "$installer::globals::epmoutpath/RPMS/i586";
        }
        elsif ( $installer::globals::os eq 'LINUX')
        {
            $machine = (POSIX::uname())[4];
            $rpmdir = "$installer::globals::epmoutpath/RPMS/$machine";
        }
        else
        {
            installer::exiter::exit_program("ERROR: rpmdir undefined !", "create_new_directory_structure");
        }

        my $systemcall = "mv $rpmdir/* $newdir";    # moving the rpms into the directory "RPMS"

        my $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not move content of \"$rpmdir\" to \"$newdir\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Moved content of \"$rpmdir\" to \"$newdir\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }

        # and removing the empty directory

        if ( $machine ne "" )
        {
            rmdir "$installer::globals::epmoutpath/RPMS/$machine";
        }
        rmdir "$installer::globals::epmoutpath/RPMS/powerpc";
        rmdir "$installer::globals::epmoutpath/RPMS/x86_64";
        rmdir "$installer::globals::epmoutpath/RPMS/i586";
        rmdir "$installer::globals::epmoutpath/RPMS/i386";
        rmdir "$installer::globals::epmoutpath/RPMS"
            or warn "Could not remove RPMS dir: $!";
    }

    # Setting unix rights to "775" for $newdir ("RPMS" or "packages")
    chmod 0775, $newdir;
}

######################################################
# Analyzing the Unix installation path.
# From the installation path /opt/openofficeorg20
# is the part /opt relocatable and the part
# openofficeorg20 static.
######################################################

sub analyze_rootpath
{
    my ($rootpath, $staticpathref, $relocatablepathref, $allvariables) = @_;

    $rootpath =~ s/\/\s*$//;    # removing ending slash

    ##############################################################
    # Version 3: "/" is variable and "/opt/openofficeorg20" fixed
    ##############################################################

    $$relocatablepathref = "/";
    # Static path has to contain the office directory name. This is replaced in shellscripts.
    $$staticpathref = $rootpath . $installer::globals::separator . $installer::globals::officedirhostname;
    # For RPM version 3.x it is required, that Prefix is not "/" in spec file. In this case --relocate will not work,
    # because RPM 3.x says, that the package is not relocatable. Therefore we have to use Prefix=/opt and for
    # all usages of --relocate this path has to be on both sides of the "=": --relocate /opt=<myselectdir>/opt .
    if ( $installer::globals::isrpmbuild )
    {
        $$relocatablepathref = $rootpath . "\/"; # relocatable path must end with "/", will be "/opt/"
        $$staticpathref = $installer::globals::officedirhostname; # to be used as replacement in shell scripts
    }

    if ( $installer::globals::isdebbuild )
    {
        $$relocatablepathref = "";
        # $$staticpathref is already "/opt/libreoffice", no additional $rootpath required.
    }

}

1;
