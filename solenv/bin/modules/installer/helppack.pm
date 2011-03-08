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

package installer::helppack;

use installer::converter;
use installer::existence;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::scpzipfiles;
use installer::scriptitems;
use installer::systemactions;
use installer::worker;

sub select_help_items
{
    my ( $itemsref, $languagesarrayref, $itemname ) = @_;

    installer::logger::include_header_into_logfile("Selecting items for help pack. Item: $itemname");

    my @itemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];

        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }

        if (( $styles =~ /\bHELPPACK\b/ ) || ( $styles =~ /\bFORCEHELPPACK\b/ ))
        {
            # Files with style "HELPPACK" and "FORCEHELPPACK" also have to be included into the help pack.
            # Files with style "HELPPACK" are only included into help packs.
            # Files with style "FORCEHELPPACK" are included into help packs and non help packs. They are
            # forced, because otherwise they not not be included into helppacks.

            my $ismultilingual = $oneitem->{'ismultilingual'};

            if ($ismultilingual)
            {
                my $specificlanguage = "";
                if ( $oneitem->{'specificlanguage'} ) { $specificlanguage = $oneitem->{'specificlanguage'}; }

            for ( my $j = 0; $j <= $#{$languagesarrayref}; $j++ )   # iterating over all languages
                {
                    my $onelanguage = ${$languagesarrayref}[$j];
                    my $locallang = $onelanguage;
                    $locallang =~ s/-/_/;

                    if ( $specificlanguage eq $onelanguage )
                    {
                    push(@itemsarray, $oneitem);
                    }
                }
            }
            else
            {
                push(@itemsarray, $oneitem);
            }
        }
    }

    return \@itemsarray;
}

sub replace_languagestring_variable
{
    my ($onepackageref, $languagestringref) = @_;

    my $key;

    foreach $key (keys %{$onepackageref})
    {
        my $value = $onepackageref->{$key};
        $value =~ s/\%LANGUAGESTRING/$$languagestringref/g;
        $onepackageref->{$key} = $value;
    }
}

#########################################################
# Including the license text into the script template
#########################################################

sub put_license_file_into_script
{
    my ($scriptfile, $licensefile) = @_;

    my $infoline = "Adding licensefile into help pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $includestring = "";

    for ( my $i = 0; $i <= $#{$licensefile}; $i++ )
    {
        $includestring = $includestring . ${$licensefile}[$i];
    }

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/LICENSEFILEPLACEHOLDER/$includestring/;
    }
}

#########################################################
# Creating a tar.gz file from a Solaris package
#########################################################

sub create_tar_gz_file
{
    my ($installdir, $packagename, $packagestring) = @_;

    $packagename =~ s/\.rpm\s*$//;
    my $targzname = $packagename . ".tar.gz";
    $systemcall = "cd $installdir; tar -cf - $packagestring | gzip > $targzname";
    installer::logger::print_message( "... $systemcall ...\n" );

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

    return $targzname;
}

#########################################################
# Determining the name of the package file
#########################################################

sub get_packagename_from_packagelist
{
    my ( $alldirs, $allvariables, $languagestringref ) = @_;

    # my $packagename = "";

    # for ( my $i = 0; $i <= $#{$alldirs}; $i++ )
    # {
    #   if ( ${$alldirs}[$i] =~ /-fonts/ ) { next; }
    #   if ( ${$alldirs}[$i] =~ /-help/ ) { next; }
    #   if ( ${$alldirs}[$i] =~ /-res/ ) { next; }
    #
    #   $packagename = ${$alldirs}[$i];
    #   last;
    # }

    # if ( $packagename eq "" ) { installer::exiter::exit_program("ERROR: Could not find base package in directory $installdir!", "get_packagename_from_packagelist"); }

    my $localproductname = $allvariables->{'PRODUCTNAME'};
    $localproductname = lc($localproductname);
    $localproductname =~ s/ //g;
    $localproductname =~ s/-/_/g;

    my $packagename = $localproductname . "_" . $$languagestringref;

    return $packagename;
}

#########################################################
# Determining the name of the package file or the rpm
# in the installation directory. For help packs
# there is only one file in this directory
#########################################################

sub determine_packagename
{
    my ( $installdir, $allvariables, $languagestringref ) = @_;

    my $packagename = "";
    my $allnames = "";

    if ( $installer::globals::isrpmbuild )
    {
        # determining the rpm file in directory $installdir

        my $fileextension = "rpm";
        my $rpmfiles = installer::systemactions::find_file_with_file_extension($fileextension, $installdir);
        if ( ! ( $#{$rpmfiles} > -1 )) { installer::exiter::exit_program("ERROR: Could not find package in directory $installdir!", "determine_packagename"); }
        my $rpmsav = [@{$rpmfiles}];
        for ( my $i = 0; $i <= $#{$rpmfiles}; $i++ ) { installer::pathanalyzer::make_absolute_filename_to_relative_filename(\${$rpmfiles}[$i]); }

        $packagename = get_packagename_from_packagelist($rpmfiles, $allvariables, $languagestringref);

        my $packagestring = installer::converter::convert_array_to_space_separated_string($rpmfiles);
        $packagename = create_tar_gz_file($installdir, $packagename, $packagestring);   # only one file
        for ( my $i = 0; $i <= $#{$rpmsav}; $i++ )
        {
            my $onefile = $installdir . $installer::globals::separator . ${$rpmsav}[$i];
            unlink($onefile);
        }

        $allnames = $rpmfiles;
    }

    if ( $installer::globals::issolarisbuild )
    {
        # determining the Solaris package file in directory $installdir
        my $alldirs = installer::systemactions::get_all_directories($installdir);

        if ( ! ( $#{$alldirs} > -1 )) { installer::exiter::exit_program("ERROR: Could not find package in directory $installdir!", "determine_packagename"); }
        my $alldirssav = [@{$alldirs}];
        for ( my $i = 0; $i <= $#{$alldirs}; $i++ ) { installer::pathanalyzer::make_absolute_filename_to_relative_filename(\${$alldirs}[$i]); }

        $packagename = get_packagename_from_packagelist($alldirs, $allvariables, $languagestringref);
        my $packagestring = installer::converter::convert_array_to_space_separated_string($alldirs);
        $packagename = create_tar_gz_file($installdir, $packagename, $packagestring);   # only a file (not a directory) can be included into the shell script
        for ( my $i = 0; $i <= $#{$alldirssav}; $i++ ) { installer::systemactions::remove_complete_directory(${$alldirssav}[$i], 1); }
        $allnames = $alldirs;
    }

    my $infoline = "Found package in installation directory $installdir : $packagename\n";
    push( @installer::globals::logfileinfo, $infoline);

    return ( $packagename, $allnames);
}

#########################################################
# Including the name of the package file or the rpm
# into the script template
#########################################################

sub put_packagename_into_script
{
    my ($scriptfile, $packagename, $allnames) = @_;

    my $localpackagename = $packagename;
    $localpackagename =~ s/\.tar\.gz//; # making "OOOopenoffice-it-ea.tar.gz" to "OOOopenoffice-it-ea"
    my $infoline = "Adding packagename $localpackagename into help pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $installline = "";

    if ( $installer::globals::issolarisbuild ) { $installline = "  /usr/sbin/pkgadd -d \$outdir -a \$adminfile"; }

    if ( $installer::globals::isrpmbuild ) { $installline = "  rpm --prefix \$PRODUCTINSTALLLOCATION --replacepkgs -i"; }

    for ( my $i = 0; $i <= $#{$allnames}; $i++ )
    {
        if ( $installer::globals::issolarisbuild ) { $installline = $installline . " ${$allnames}[$i]"; }

        if ( $installer::globals::isrpmbuild ) { $installline = $installline . " \$outdir/${$allnames}[$i]"; }
    }

    for ( my $j = 0; $j <= $#{$scriptfile}; $j++ )
    {
        ${$scriptfile}[$j] =~ s/INSTALLLINES/$installline/;
    }
}

##################################################################
# Including the lowercase product name into the script template
##################################################################

sub put_productname_into_script
{
    my ($scriptfile, $variableshashref) = @_;

    my $productname = $variableshashref->{'PRODUCTNAME'};
    $productname = lc($productname);
    $productname =~ s/\.//g;    # openoffice.org -> openofficeorg

    my $infoline = "Adding productname $productname into help pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/PRODUCTNAMEPLACEHOLDER/$productname/;
    }
}

##################################################################
# Including the full product name into the script template
# (name and version)
##################################################################

sub put_fullproductname_into_script
{
    my ($scriptfile, $variableshashref) = @_;

    my $productname = $variableshashref->{'PRODUCTNAME'};
    my $productversion = "";
    if ( $variableshashref->{'PRODUCTVERSION'} ) { $productversion = $variableshashref->{'PRODUCTVERSION'}; };
    my $fullproductname = $productname . " " . $productversion;

    my $infoline = "Adding full productname \"$fullproductname\" into help pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/FULLPRODUCTNAMELONGPLACEHOLDER/$fullproductname/;
    }
}

##################################################################
# Including the name of the search package (-core01)
# into the script template
##################################################################

sub put_searchpackage_into_script
{
    my ($scriptfile, $variableshashref) = @_;

    my $basispackageprefix = $variableshashref->{'BASISPACKAGEPREFIX'};
    my $basispackageversion = $variableshashref->{'OOOBASEVERSION'};

    if ( $installer::globals::issolarisbuild ) { $basispackageversion =~ s/\.//g; } # "3.0" -> "30"

    my $infoline = "Adding basis package prefix $basispackageprefix into help pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    $infoline = "Adding basis package version $basispackageversion into help pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/BASISPACKAGEPREFIXPLACEHOLDER/$basispackageprefix/;
        ${$scriptfile}[$i] =~ s/OOOBASEVERSIONPLACEHOLDER/$basispackageversion/;
    }

}

#########################################################
# Including the linenumber into the script template
#########################################################

sub put_linenumber_into_script
{
    my ( $scriptfile, $licensefile, $allnames ) = @_;

    my $linenumber =  $#{$scriptfile} + $#{$licensefile} + 3;   # also adding the content of the license file!

    my $infoline = "Adding linenumber $linenumber into help pack script\n";
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
    my ( $packagename ) = @_;

    my $scriptfilename = $packagename;

#   if ( $installer::globals::isrpmbuild ) { $scriptfilename =~ s/\.rpm\s*$/\.sh/; }
#   if ( $installer::globals::issolarisbuild ) { $scriptfilename =~ s/\.tar\.gz\s*$/\.sh/; }

    $scriptfilename =~ s/\.tar\.gz\s*$/\.sh/;

    my $infoline = "Setting help pack script file name to $scriptfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $scriptfilename;
}

#########################################################
# Saving the script file in the installation directory
#########################################################

sub save_script_file
{
    my ($installdir, $newscriptfilename, $scriptfile) = @_;

    $newscriptfilename = $installdir . $installer::globals::separator . $newscriptfilename;
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
    my ( $scriptfilename, $installdir, $packagename ) = @_;

    my $longpackagename = $installdir . $installer::globals::separator . $packagename;
    my $systemcall = "cat $longpackagename >>$scriptfilename";

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
# Removing the binary package
#########################################################

sub remove_package
{
    my ( $installdir, $packagename ) = @_;

    my $remove_package = 1;

    if ( $ENV{'DONT_REMOVE_PACKAGE'} ) { $remove_package = 0; }

    if ( $remove_package )
    {
        my $longpackagename = $installdir . $installer::globals::separator . $packagename;
        unlink $longpackagename;

        my $infoline = "Removing package: $longpackagename \n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

####################################################
# Unix help packs, that are not part of
# multilingual installation sets, need a
# shell script installer
####################################################

sub build_installer_for_helppack
{
    my ($installdir, $allvariableshashref, $includepatharrayref, $languagesarrayref, $languagestringref) = @_;

    installer::logger::print_message( "... creating shell script installer ...\n" );

    installer::logger::include_header_into_logfile("Creating shell script installer:");

    # find and read setup script template

    my $scriptfilename = "langpackscript.sh";
    my $scriptref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$scriptfilename, $includepatharrayref, 0);
    if ($$scriptref eq "") { installer::exiter::exit_program("ERROR: Could not find script file $scriptfilename!", "build_installer_for_helppack"); }
    my $scriptfile = installer::files::read_file($$scriptref);

    my $infoline = "Found  script file $scriptfilename: $$scriptref \n";
    push( @installer::globals::logfileinfo, $infoline);

    # find and read english license file
    my $licenselanguage = "en-US";                  # always english !
    my $licensefilename = "LICENSE"; # _" . $licenselanguage;
    my $licenseincludepatharrayref = installer::worker::get_language_specific_include_pathes($includepatharrayref, $licenselanguage);

    my $licenseref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$licensefilename, $licenseincludepatharrayref, 0);
    if ($$licenseref eq "") { installer::exiter::exit_program("ERROR: Could not find License file $licensefilename!", "build_installer_for_helppack"); }
    my $licensefile = installer::files::read_file($$licenseref);

    $infoline = "Found licensefile $licensefilename: $$licenseref \n";
    push( @installer::globals::logfileinfo, $infoline);

    # including variables into license file
    installer::scpzipfiles::replace_all_ziplistvariables_in_file($licensefile, $allvariableshashref);

    # add license text into script template
    put_license_file_into_script($scriptfile, $licensefile);

    # add rpm or package file name into script template
    my ( $packagename, $allnames) = determine_packagename($installdir, $allvariableshashref, $languagestringref);
    put_packagename_into_script($scriptfile, $packagename, $allnames);

    # add product name into script template
    put_productname_into_script($scriptfile, $allvariableshashref);

    # add product name into script template
    put_fullproductname_into_script($scriptfile, $allvariableshashref);

    # add product name into script template
    put_searchpackage_into_script($scriptfile, $allvariableshashref);

    # replace linenumber in script template
    put_linenumber_into_script($scriptfile, $licensefile, $allnames);

    # saving the script file
    my $newscriptfilename = determine_scriptfile_name($packagename);
    $newscriptfilename = save_script_file($installdir, $newscriptfilename, $scriptfile);

    # include rpm or package into script
    include_package_into_script($newscriptfilename, $installdir, $packagename);

    # remove rpm or package
    remove_package($installdir, $packagename);
}

1;
