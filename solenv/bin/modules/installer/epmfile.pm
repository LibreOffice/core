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

package installer::epmfile;

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

############################################################################
# Reading the package map to find Solaris package names for
# the corresponding abbreviations
############################################################################

sub read_packagemap
{
    my ($allvariables, $includepatharrayref, $languagesarrayref) = @_;

    my $packagemapname = "";
    if ( $allvariables->{'PACKAGEMAP'} ) { $packagemapname = $allvariables->{'PACKAGEMAP'}; }
    if ( $packagemapname eq "" ) { installer::exiter::exit_program("ERROR: Property PACKAGEMAP must be defined!", "read_packagemap"); }

    my $infoline = "\n\nCollected abbreviations and package names:\n";
    push(@installer::globals::logfileinfo, $infoline);

    # Can be a comma separated list. All files have to be found in include paths
    my $allpackagemapnames = installer::converter::convert_stringlist_into_hash(\$packagemapname, ",");
    foreach my $onepackagemapname ( keys %{$allpackagemapnames} )
    {
        my $packagemapref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$onepackagemapname, $includepatharrayref, 0);

        if ( $$packagemapref eq "" ) { installer::exiter::exit_program("ERROR: Could not find package map file \"$onepackagemapname\" (propery PACKAGEMAP)!", "read_packagemap"); }

        my $packagemapcontent = installer::files::read_file($$packagemapref);

        for ( my $i = 0; $i <= $#{$packagemapcontent}; $i++ )
        {
            my $line = ${$packagemapcontent}[$i];

            if ( $line =~ /^\s*\#/ ) { next; }  # comment line
            if ( $line =~ /^\s*$/ ) { next; }  # empty line

            if ( $line =~ /^\s*(.*?)\t(.*?)\s*$/ )
            {
                my $abbreviation = $1;
                my $packagename = $2;
                installer::packagelist::resolve_packagevariables(\$abbreviation, $allvariables, 0);
                installer::packagelist::resolve_packagevariables(\$packagename, $allvariables, 0);

                # Special handling for language strings %LANGUAGESTRING

                if (( $abbreviation =~ /\%LANGUAGESTRING/ ) || ( $packagename =~ /\%LANGUAGESTRING/ ))
                {
                    foreach my $onelang ( @{$languagesarrayref} )
                    {
                        my $local_abbreviation = $abbreviation;
                        my $local_packagename = $packagename;
                        $local_abbreviation =~ s/\%LANGUAGESTRING/$onelang/g;
                        $local_packagename =~ s/\%LANGUAGESTRING/$onelang/g;

                        # Logging all abbreviations and packagenames
                        $infoline = "$onelang : $local_abbreviation : $local_packagename\n";
                        push(@installer::globals::logfileinfo, $infoline);

                        if ( exists($installer::globals::dependfilenames{$local_abbreviation}) )
                        {
                            installer::exiter::exit_program("ERROR: Packagename for  Solaris package $local_abbreviation already defined ($installer::globals::dependfilenames{$local_abbreviation})!", "read_packagemap");
                        }
                        else
                        {
                            $installer::globals::dependfilenames{$local_abbreviation} = $local_packagename;
                        }
                    }
                }
                else
                {
                    # Logging all abbreviations and packagenames
                    $infoline = "$abbreviation : $packagename\n";
                    push(@installer::globals::logfileinfo, $infoline);

                    if ( exists($installer::globals::dependfilenames{$abbreviation}) )
                    {
                        installer::exiter::exit_program("ERROR: Packagename for  Solaris package $abbreviation already defined ($installer::globals::dependfilenames{$abbreviation})!", "read_packagemap");
                    }
                    else
                    {
                        $installer::globals::dependfilenames{$abbreviation} = $packagename;
                    }
                }
            }
            else
            {
                my $errorline = $i + 1;
                installer::exiter::exit_program("ERROR: Wrong syntax in file \"$onepackagemapname\" (line $errorline)!", "read_packagemap");
            }
        }
    }

    $infoline = "\n\n";
    push(@installer::globals::logfileinfo, $infoline);

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
        if (( $installer::globals::issolarisbuild ) && ( $onefile->{'SolarisGroup'} )) { $group = $onefile->{'SolarisGroup'}; }

        my $line = "$filetype $unixrights root $group $destination $sourcepath\n";

        push(@{$epmfileref}, $line);
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

    my ($licensefilename, $readmefilename, $readmefilenameen);

    my $foundlicensefile = 0;
    my $foundreadmefile = 0;

    my $line = "";
    my $infoline = "";

    # %product LibreOffice Software
    # %version 2.0
    # %description A really great software
    # %copyright 1999-2003 by OOo
    # %vendor LibreOffice
    # %license /test/replace/01/LICENSE01
    # %readme /test/replace/01/README01
    # %requires foo
    # %provides bar
    # %replaces bar
    # %incompat bar

    # The first language in the languages array determines the language of license and readme file

    my $searchlanguage = ${$languagesref}[0];

    # using the description for the %product line in the epm list file

    my $productnamestring = $onepackage->{'description'};
    installer::packagelist::resolve_packagevariables(\$productnamestring, $variableshashref, 0);
    if ( $variableshashref->{'PRODUCTEXTENSION'} ) { $productnamestring = $productnamestring . " " . $variableshashref->{'PRODUCTEXTENSION'}; }

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
    if ( $installer::globals::isrpmbuild ) { $line = "%release" . " " . $installer::globals::buildid . "\n"; }
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

    # License and Readme file can be included automatically from the file list

    if ( $installer::globals::iswindowsbuild )
    {
        $licensefilename = "license.txt";
        $readmefilename = "readme.txt";
        $readmefilenameen = "readme_en-US.txt";
    }
    else
    {
        $licensefilename = "LICENSE";
        $readmefilename = "README";
        $readmefilenameen = "README_en-US";
    }

    if (( $installer::globals::languagepack )   # in language packs and help packs the files LICENSE and README are removed, because they are not language specific
        || ( $installer::globals::helppack )
        || ( $variableshashref->{'NO_README_IN_ROOTDIR'} ))
    {
        if ( $installer::globals::iswindowsbuild )
        {
            $licensefilename = "license.txt";
            $readmefilename = "readme_$searchlanguage.txt";
        }
        else
        {
            $licensefilename = "LICENSE";
            $readmefilename = "README_$searchlanguage";
        }
    }

    my $license_in_package_defined = 0;

    if ( $installer::globals::issolarisbuild )
    {
        if ( $onepackage->{'solariscopyright'} )
        {
            $licensefilename = $onepackage->{'solariscopyright'};
            $license_in_package_defined = 1;
        }
    }

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

    # searching for and readme file;
    # URE uses special README; others use README_en-US
    # it does not matter which one is passed for epm if both are packaged
    foreach my $possiblereadmefilename ($readmefilenameen, $readmefilename)
    {
        last if ($foundreadmefile);
        for ( my $i = 0; $i <= $#{$filesinproduct}; $i++ )
        {
            my $onefile = ${$filesinproduct}[$i];
            my $filename = $onefile->{'Name'};
            # in the SDK it's in subdirectory sdk/share/readme
            if ( $filename =~ /$possiblereadmefilename$/ )
            {
                $foundreadmefile = 1;
                $line = "%readme" . " " . $onefile->{'sourcepath'} . "\n";
                push(@epmheader, $line);
                last;
            }
        }
    }

    # the readme file need not be packaged more times in the help content
    # it needs to be installed in parallel with the main package anyway
    # try to find the README file between all available files (not only between the packaged)
    if (!($foundreadmefile) && $installer::globals::helppack)
    {
        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$readmefilenameen, "" , 0);
        if($$fileref ne "" )
        {
            $infoline = "Fallback to readme file: \"$$fileref\"!\n";
            push(@installer::globals::logfileinfo, $infoline);

            $foundreadmefile = 1;
            $line = "%readme" . " " . $$fileref . "\n";
            push(@epmheader, $line);
        }
    }

    # searching for and license file

    if ( $license_in_package_defined )
    {
        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$licensefilename, "" , 0);

        if ( $$fileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find license file $licensefilename (A)!", "create_epm_header"); }

        # Special handling to add the content of the file "license_en-US" to the solaris copyrightfile. But not for all products

        if (( $installer::globals::issolarispkgbuild ) && ( ! $variableshashref->{'NO_LICENSE_INTO_COPYRIGHT'} ))
        {
            if ( ! $installer::globals::englishlicenseset ) { _set_english_license() }

            # The location for the new file
            my $languagestring = "";
            for ( my $i = 0; $i <= $#{$languagesref}; $i++ ) { $languagestring = $languagestring . "_" . ${$languagesref}[$i]; }
            $languagestring =~ s/^\s*_//;

            my $copyrightdir = installer::systemactions::create_directories("copyright", \$languagestring);

            my $copyrightfile = installer::files::read_file($$fileref);

            # Adding license content to copyright file
            push(@{$copyrightfile}, "\n");
            for ( my $i = 0; $i <= $#{$installer::globals::englishlicense}; $i++ ) { push(@{$copyrightfile}, ${$installer::globals::englishlicense}[$i]); }

            # New destination for $$fileref
            $$fileref = $copyrightdir . $installer::globals::separator . "solariscopyrightfile_" . $onepackage->{'module'};
            if ( -f $$fileref ) { unlink $$fileref; }
            installer::files::save_file($$fileref, $copyrightfile);
        }

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
        # in the SDK it's in subdirectory sdk/share/readme so try to match that
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

    if (!($foundreadmefile))
    {
        installer::exiter::exit_program("ERROR: Could not find readme file $readmefilename (C)", "create_epm_header");
    }

    # including %replaces

    my $replaces = "";

    if ( $installer::globals::issolarispkgbuild )
    {
        $replaces = "solarisreplaces";   # the name in the packagelist
    }
    elsif ( $installer::globals::islinuxbuild )
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

    if (( $installer::globals::issolarispkgbuild ) && ( ! $installer::globals::patch ))
    {
        $incompat = "solarisincompat";   # the name in the packagelist
    }
    elsif (( $installer::globals::islinuxbuild ) && ( ! $installer::globals::patch ))
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

    if ( $installer::globals::issolarispkgbuild )
    {
        $provides = "solarisprovides";   # the name in the packagelist
        $requires = "solarisrequires";   # the name in the packagelist
    }
    elsif ( $installer::globals::isfreebsdpkgbuild )
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

        # The requires string can contain the separator "," in the names (descriptions) of the packages
        # (that are required for Solaris depend files). Therefore "," inside such a description has to
        # masked with a backslash.
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
        if ( $_ =~ /Patched for OpenOffice.org/ ) { $installer::globals::is_special_epm = 1; }
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

    if ( ( $installer::globals::is_special_epm ) && (($installer::globals::isrpmbuild) || ($installer::globals::issolarispkgbuild)) )
    {
        # Special postprocess handling only for Linux RPM and Solaris packages
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
# LD_PRELOAD string for Debian packages
#################################################

sub get_ld_preload_string
{
    my $getuidlibrary = $ENV{'WORKDIR'} . '/LinkTarget/Library/libgetuid.so';
    if ( ! -e $getuidlibrary ) { installer::exiter::exit_program("File $getuidlibrary does not exist!", "get_ld_preload_string"); }
    return 'LD_PRELOAD=' . $getuidlibrary;
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

    # Debian package build needs a LD_PRELOAD for correct rights

    my $ldpreloadstring = "";

    if ( $installer::globals::debian ) { $ldpreloadstring = get_ld_preload_string($includepatharrayref) . " "; }

    my $extraflags = "";
        if ($ENV{'EPM_FLAGS'}) { $extraflags = $ENV{'EPM_FLAGS'}; }

    $extraflags .= ' -g' unless $installer::globals::strip;

    my $verboseflag = "-v";
    if ( ! $installer::globals::quiet ) { $verboseflag = "-v2"; };

    my $systemcall = $ldpreloadstring . $epmname . " -f " . $packageformat . " " . $extraflags . " " . $localpackagename . " " . $epmlistfilename . $outdirstring . " " . $verboseflag . " " . " 2\>\&1 |";

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
# Adding the new line for relocatables into pkginfo file (Solaris)
# or spec file (Linux) created by epm
#####################################################################

sub add_one_line_into_file
{
    my ($file, $insertline, $filename) = @_;

    if ( $installer::globals::issolarispkgbuild )
    {
        push(@{$file}, $insertline);        # simply adding at the end of pkginfo file
    }

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
# Setting the revision VERSION=1.9,REV=66  .
# Also adding the new line: "AutoReqProv: no"
#####################################################################

sub set_revision_in_pkginfo
{
    my ($file, $filename, $variables, $packagename) = @_;

    my $revisionstring = "\,REV\=" . $installer::globals::packagerevision;

    # Adding also a time string to the revision. Syntax: VERSION=8.0.0,REV=66.2005.01.24

    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);

    $mday = $mday;
    $mon = $mon + 1;
    $year = $year + 1900;

    if ( $mday < 10 ) { $mday = "0" . $mday; }
    if ( $mon < 10 ) { $mon = "0" . $mon; }
    $datestring = $year . "." . $mon . "." . $mday;
    $revisionstring = $revisionstring . "." . $datestring;

    for ( my $i = 0; $i <= $#{$file}; $i++ )
    {
        if ( ${$file}[$i] =~ /^\s*(VERSION\=.*?)\s*$/ )
        {
            my $oldstring = $1;
            my $newstring = $oldstring . $revisionstring;   # also adding the date string
            ${$file}[$i] =~ s/$oldstring/$newstring/;
            my $infoline = "Info: Changed in $filename file: \"$oldstring\" to \"$newstring\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
            last;
        }
    }

    # For Update and Patch reasons, this string can also be kept constant

    my $pkgversion = "SOLSPARCPKGVERSION";
    if ( $installer::globals::issolarisx86build ) { $pkgversion = "SOLIAPKGVERSION"; }

    if (( $variables->{$pkgversion} ) && ( $variables->{$pkgversion} ne "" ))
    {
        if ( $variables->{$pkgversion} ne "FINALVERSION" )
        {
            # In OOo 3.x timeframe, this string is no longer unique for all packages, because of the three layer.
            # In the string: "3.0.0,REV=9.2008.09.30" only the part "REV=9.2008.09.30" can be unique for all packages
            # and therefore be set as $pkgversion.
            # The first part "3.0.0" has to be derived from the

            my $version = $installer::globals::packageversion;
            if ( $version =~ /^\s*(\d+)\.(\d+)\.(\d+)\s*$/ )
            {
                my $major = $1;
                my $minor = $2;
                my $micro = $3;

                my $finalmajor = $major;
                my $finalminor = $minor;
                my $finalmicro = 0;

                $version = "$finalmajor.$finalminor.$finalmicro";
            }

            my $datestring = $variables->{$pkgversion};

            # Allowing some packages to have another date of creation.
            # They can be defined in product definition using a key like "SOLSPARCPKGVERSION_$packagename"

            my $additionalkey = $pkgversion . "_" . $packagename;
            if (( $variables->{$additionalkey} ) && ( $variables->{$additionalkey} ne "" )) { $datestring = $variables->{$additionalkey}; }

            my $versionstring = "$version,$datestring";

            for ( my $i = 0; $i <= $#{$file}; $i++ )
            {
                if ( ${$file}[$i] =~ /^\s*(VERSION\=).*?\s*$/ )
                {
                    my $start = $1;
                    my $newstring = $start . $versionstring . "\n"; # setting the complete new string
                    my $oldstring = ${$file}[$i];
                    ${$file}[$i] = $newstring;
                    $oldstring =~ s/\s*$//;
                    $newstring =~ s/\s*$//;
                    my $infoline = "Info: Changed in $filename file: \"$oldstring\" to \"$newstring\"!\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    last;
                }
            }
        }
    }
}

########################################################
# Setting MAXINST=1000 into the pkginfo file.
########################################################

sub set_maxinst_in_pkginfo
{
    my ($changefile, $filename) = @_;

    my $newline = "MAXINST\=1000\n";

    add_one_line_into_file($changefile, $newline, $filename);
}

#############################################################
# Setting several Solaris variables into the pkginfo file.
#############################################################

sub set_solaris_parameter_in_pkginfo
{
    my ($changefile, $filename, $allvariables) = @_;

    my $newline = "";

    # SUNW_PRODNAME
    # SUNW_PRODVERS
    # SUNW_PKGVERS
    # Not: SUNW_PKGTYPE
    # HOTLINE
    # EMAIL

    my $productname = $allvariables->{'PRODUCTNAME'};
    $newline = "SUNW_PRODNAME=$productname\n";
    add_one_line_into_file($changefile, $newline, $filename);

    my $productversion = "";
    if ( $allvariables->{'PRODUCTVERSION'} )
    {
        $productversion = $allvariables->{'PRODUCTVERSION'};
        if ( $allvariables->{'PRODUCTEXTENSION'} ) { $productversion = $productversion . "/" . $allvariables->{'PRODUCTEXTENSION'}; }
    }
    $newline = "SUNW_PRODVERS=$productversion\n";
    add_one_line_into_file($changefile, $newline, $filename);

    $newline = "SUNW_PKGVERS=1\.0\n";
    add_one_line_into_file($changefile, $newline, $filename);

    if ( $allvariables->{'SUNW_PKGTYPE'} )
    {
        $newline = "SUNW_PKGTYPE=$allvariables->{'SUNW_PKGTYPE'}\n";
        add_one_line_into_file($changefile, $newline, $filename);
    }
    else
    {
        $newline = "SUNW_PKGTYPE=\n";
        add_one_line_into_file($changefile, $newline, $filename);
    }

    $newline = "HOTLINE=Please contact your local service provider\n";
    add_one_line_into_file($changefile, $newline, $filename);

    $newline = "EMAIL=\n";
    add_one_line_into_file($changefile, $newline, $filename);

}

#####################################################################
# epm uses as archtecture for Solaris x86 "i86pc". This has to be
# changed to "i386".
#####################################################################

sub fix_architecture_setting
{
    my ($changefile) = @_;

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        if ( ${$changefile}[$i] =~ /^\s*ARCH=i86pc\s*$/ )
        {
            ${$changefile}[$i] =~ s/i86pc/i386/;
            last;
        }

    }
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

    my $autoreqprovline;

    if ( $findrequires )
    {
        $autoreqprovline = "AutoProv\: no\n%define _use_internal_dependency_generator 0\n%define __find_requires $bindir/$findrequires\n";
    }
    else
    {
        $autoreqprovline = "AutoReqProv\: no\n";
    }

    $autoreqprovline .= "%define _binary_filedigest_algorithm 1\n%define _binary_payload w9.gzdio\n";

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        # Adding "autoreqprov" behind the line beginning with: Group:
        if ( ${$changefile}[$i] =~ /^\s*Group\:\s*/ )
        {
            splice(@{$changefile},$i+1,0,$autoreqprovline);
            $autoreqprovline =~ s/\s*$//;
            $infoline = "Success: Added line $autoreqprovline into spec file!\n";
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

#########################################################
# Building relocatable Solaris packages means:
# 1. Add "BASEDIR=/opt" into pkginfo
# 2. Remove "/opt/" from all objects in prototype file
# For step2 this function exists
# Sample: d none /opt/openofficeorg20/help 0755 root other
# -> d none openofficeorg20/help 0755 root other
#########################################################

sub make_prototypefile_relocatable
{
    my ($prototypefile, $relocatablepath) = @_;

    for ( my $i = 0; $i <= $#{$prototypefile}; $i++ )
    {
        if ( ${$prototypefile}[$i] =~ /^\s*\w\s+\w+\s+\/\w+/ )  # this is an object line
        {
            ${$prototypefile}[$i] =~ s/$relocatablepath//;  # Important: $relocatablepath has a "/" at the end. Example "/opt/"
        }
    }

    # If the $relocatablepath is "/opt/openoffice20/" the line "d none /opt/openoffice20" was not changed.
    # This line has to be removed now

    if ( $relocatablepath ne "/" ) { $relocatablepath =~ s/\/\s*$//; }      # removing the ending slash

    for ( my $i = 0; $i <= $#{$prototypefile}; $i++ )
    {
        if ( ${$prototypefile}[$i] =~ /^\s*d\s+\w+\s+\Q$relocatablepath\E/ )
        {
            my $line = ${$prototypefile}[$i];
            splice(@{$prototypefile},$i,1); # removing the line
            $line =~ s/\s*$//;
            my $infoline = "Info: Removed line \"$line\" from prototype file!\n";
            push( @installer::globals::logfileinfo, $infoline);
            last;
        }
    }

    # Making "\$" to "$" in prototype file. "\$" was created by epm.

    for ( my $i = 0; $i <= $#{$prototypefile}; $i++ )
    {
        if ( ${$prototypefile}[$i] =~ /\\\$/ )
        {
            ${$prototypefile}[$i] =~ s/\\\$/\$/g;
            my $infoline2 = "Info: Changed line in prototype file: ${$prototypefile}[$i] !\n";
            push( @installer::globals::logfileinfo, $infoline2);
        }
    }
}

#########################################################################
# Replacing the variables in the shell scripts or in the epm list file
# Linux: spec file
# Solaris: preinstall, postinstall, preremove, postremove
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
# Determinig the directory created by epm, in which the
# RPMS or Solaris packages are created.
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

    my @newclasses = ();
    my $newclassesstring = "";

    if ( $installer::globals::issolarispkgbuild )
    {
        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            my $onefile = ${$filesref}[$i];

            if ( $onefile->{'SolarisClass'} )
            {
                my $sourcepath = $onefile->{'sourcepath'};

                for ( my $j = 0; $j <= $#{$changefile}; $j++ )
                {
                    if (( ${$changefile}[$j] =~ /^\s*f\s+none\s+/ ) && ( ${$changefile}[$j] =~ /\=\Q$sourcepath\E\s+/ ))
                    {
                        my $oldline = ${$changefile}[$j];
                        ${$changefile}[$j] =~ s/f\s+none/e $onefile->{'SolarisClass'}/;
                        my $newline = ${$changefile}[$j];
                        $oldline =~ s/\s*$//;
                        $newline =~ s/\s*$//;

                        my $infoline = "TAB: Changing content from \"$oldline\" to \"$newline\" .\n";
                        push(@installer::globals::logfileinfo, $infoline);

                        # collecting all new classes
                        if (! grep {$_ eq $onefile->{'SolarisClass'}} @newclasses)
                        {
                            push(@newclasses, $onefile->{'SolarisClass'});
                        }

                        last;
                    }
                }
            }
        }

        $newclassesstring = installer::converter::convert_array_to_space_separated_string(\@newclasses);
    }

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

    return $newclassesstring;
}

############################################################
# Including additional classes into the pkginfo file
############################################################

sub include_classes_into_pkginfo
{
    my ($changefile, $classesstring) = @_;

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        if ( ${$changefile}[$i] =~ /^\s*CLASSES\=none/ )
        {
            ${$changefile}[$i] =~ s/\s*$//;
            my $oldline = ${$changefile}[$i];
            ${$changefile}[$i] = ${$changefile}[$i] . " " . $classesstring . "\n";
            my $newline = ${$changefile}[$i];
            $newline =~ s/\s*$//;

            my $infoline = "pkginfo file: Changing content from \"$oldline\" to \"$newline\" .\n";
            push(@installer::globals::logfileinfo, $infoline);
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

######################################################################
# Checking, if an extension is included into the package (Solaris).
# All extension files have to be installed into directory
# share/extension/install
######################################################################

sub contains_extension_dir
{
    my ($prototypefile) = @_;

    my $contains_extension_dir = 0;

    # d none opt/openoffice.org3/share/extensions/

    for ( my $i = 0; $i <= $#{$prototypefile}; $i++ )
    {
        my $line = ${$prototypefile}[$i];
        if ( $line =~ /^\s*d\s+none\s.*\/share\/extensions\// )
        {
            $contains_extension_dir = 1;
            last;
        }
    }

    return $contains_extension_dir;
}

############################################################
# Setting the correct Solaris locales
############################################################

sub get_solaris_language_for_langpack
{
    my ( $onelanguage ) = @_;

    my $sollanguage = $onelanguage;
    $sollanguage =~ s/\-/\_/;

    if ( $sollanguage eq "de" ) { $sollanguage = "de"; }
    elsif ( $sollanguage eq "en_US" ) { $sollanguage = "en_AU,en_CA,en_GB,en_IE,en_MT,en_NZ,en_US,en_US.UTF-8"; }
    elsif ( $sollanguage eq "es" ) { $sollanguage = "es"; }
    elsif ( $sollanguage eq "fr" ) { $sollanguage = "fr"; }
    elsif ( $sollanguage eq "hu" ) { $sollanguage = "hu_HU"; }
    elsif ( $sollanguage eq "it" ) { $sollanguage = "it"; }
    elsif ( $sollanguage eq "nl" ) { $sollanguage = "nl_BE,nl_NL"; }
    elsif ( $sollanguage eq "pl" ) { $sollanguage = "pl_PL"; }
    elsif ( $sollanguage eq "sv" ) { $sollanguage = "sv"; }
    elsif ( $sollanguage eq "pt" ) { $sollanguage = "pt_PT"; }
    elsif ( $sollanguage eq "pt_BR" ) { $sollanguage = "pt_BR"; }
    elsif ( $sollanguage eq "ru" ) { $sollanguage = "ru_RU"; }
    elsif ( $sollanguage eq "ja" ) { $sollanguage = "ja,ja_JP,ja_JP.PCK,ja_JP.UTF-8"; }
    elsif ( $sollanguage eq "ko" ) { $sollanguage = "ko,ko.UTF-8"; }
    elsif ( $sollanguage eq "zh_CN" ) { $sollanguage = "zh,zh.GBK,zh_CN.GB18030,zh.UTF-8"; }
    elsif ( $sollanguage eq "zh_TW" ) { $sollanguage = "zh_TW,zh_TW.BIG5,zh_TW.UTF-8,zh_HK.BIG5HK,zh_HK.UTF-8"; }

    return $sollanguage;
}

############################################################
# Adding language infos in pkginfo file
############################################################

sub include_languageinfos_into_pkginfo
{
    my ( $changefile, $filename, $languagestringref, $onepackage, $variableshashref ) = @_;

    # SUNWPKG_LIST=core01
    # SUNW_LOC=de

    my $locallang = $onepackage->{'language'};
    my $solarislanguage = get_solaris_language_for_langpack($locallang);

    my $newline = "SUNW_LOC=" . $solarislanguage . "\n";
    add_one_line_into_file($changefile, $newline, $filename);

    # SUNW_PKGLIST is required, if SUNW_LOC is defined.
    if ( $onepackage->{'pkg_list_entry'} )
    {
        my $packagelistentry = $onepackage->{'pkg_list_entry'};
        installer::packagelist::resolve_packagevariables(\$packagelistentry, $variableshashref, 1);
        $newline = "SUNW_PKGLIST=" . $packagelistentry . "\n";
        add_one_line_into_file($changefile, $newline, $filename);
    }
    else
    {
        # Using default package ooobasis30-core01.
        my $packagelistentry = "%BASISPACKAGEPREFIX%WITHOUTDOTPRODUCTVERSION-core01";
        installer::packagelist::resolve_packagevariables(\$packagelistentry, $variableshashref, 1);
        $newline = "SUNW_PKGLIST=" . $packagelistentry . "\n";
        add_one_line_into_file($changefile, $newline, $filename);
    }
}

############################################################
# Including package names into the depend files.
# The package names have to be included into
# packagelist. They are already saved in
# %installer::globals::dependfilenames.
############################################################

sub put_packagenames_into_dependfile
{
    my ( $file ) = @_;

    for ( my $i = 0; $i <= $#{$file}; $i++ )
    {
        my $line = ${$file}[$i];
        if ( $line =~ /^\s*\w\s+(.*?)\s*$/ )
        {
            my $abbreviation = $1;

            if ( $abbreviation =~ /\%/ ) { installer::exiter::exit_program("ERROR: Could not resolve all properties in Solaris package abbreviation \"$abbreviation\"!", "read_packagemap"); }

            if ( exists($installer::globals::dependfilenames{$abbreviation}) )
            {
                my $packagename = $installer::globals::dependfilenames{$abbreviation};
                if ( $packagename =~ /\%/ ) { installer::exiter::exit_program("ERROR: Could not resolve all properties in Solaris package name \"$packagename\"!", "read_packagemap"); }

                $line =~ s/\s*$//;
                ${$file}[$i] = $line . "\t" . $packagename . "\n";
            }
            else
            {
                installer::exiter::exit_program("ERROR: Missing packagename for Solaris package \"$abbreviation\"!", "put_packagenames_into_dependfile");
            }
        }
    }
}

############################################################
# Including the relocatable directory into
# spec file and pkginfo file
# Linux: set topdir in specfile
# Solaris: remove $relocatablepath (/opt/)
# for all objects in prototype file
# and changing "topdir" for Linux
############################################################

sub prepare_packages
{
    my ($loggingdir, $packagename, $staticpath, $relocatablepath, $onepackage, $variableshashref, $filesref, $languagestringref) = @_;

    my $filename = "";
    my $newline = "";
    my $newepmdir = $installer::globals::epmoutpath . $installer::globals::separator;

    my $localrelocatablepath = $relocatablepath;
    if ( $localrelocatablepath ne "/" ) { $localrelocatablepath =~ s/\/\s*$//; }

    if ( $installer::globals::issolarispkgbuild )
    {
        $filename = $packagename . ".pkginfo";
        $newline = "BASEDIR\=" . $localrelocatablepath . "\n";
    }

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

    # removing the relocatable path in prototype file

    if ( $installer::globals::issolarispkgbuild )
    {
        set_revision_in_pkginfo($changefile, $filename, $variableshashref, $packagename);
        set_maxinst_in_pkginfo($changefile, $filename);
        set_solaris_parameter_in_pkginfo($changefile, $filename, $variableshashref);
        if ( $installer::globals::issolarisx86build ) { fix_architecture_setting($changefile); }
        if (( $onepackage->{'language'} ) && ( $onepackage->{'language'} ne "" ) && ( $onepackage->{'language'} ne "en-US" )) { include_languageinfos_into_pkginfo($changefile, $filename, $languagestringref, $onepackage, $variableshashref); }
        installer::files::save_file($completefilename, $changefile);

        my $prototypefilename = $packagename . ".prototype";
        $prototypefilename = $newepmdir . $prototypefilename;
        if (! -f $prototypefilename) { installer::exiter::exit_program("ERROR: Did not find prototype file: $prototypefilename", "prepare_packages"); }

        my $prototypefile = installer::files::read_file($prototypefilename);
        make_prototypefile_relocatable($prototypefile, $relocatablepath);
        my $classesstring = set_tab_into_datafile($prototypefile, $filesref);
        if ($classesstring)
        {
            include_classes_into_pkginfo($changefile, $classesstring);
            installer::files::save_file($completefilename, $changefile);
        }

        installer::files::save_file($prototypefilename, $prototypefile);

        # Adding package names into depend files for Solaris (not supported by epm)
        my $dependfilename = $packagename . ".depend";
        $dependfilename = $newepmdir . $dependfilename;
        if ( -f $dependfilename)
        {
            my $dependfile = installer::files::read_file($dependfilename);
            put_packagenames_into_dependfile($dependfile);
            installer::files::save_file($dependfilename, $dependfile);
        }
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

    # Solaris: pkgmk -o -f solaris-2.8-sparc/SUNWso8m34.prototype -d solaris-2.8-sparc
    # Solaris: pkgtrans solaris-2.8-sparc SUNWso8m34.pkg SUNWso8m34
    # Solaris: tar -cf - SUNWso8m34 | gzip > SUNWso8m34.tar.gz

    if ( $installer::globals::issolarispkgbuild )
    {
        my $prototypefile = $epmdir . $packagename . ".prototype";
        if (! -f $prototypefile) { installer::exiter::exit_program("ERROR: Did not find file: $prototypefile", "create_packages_without_epm"); }

        my $destinationdir = $prototypefile;
        installer::pathanalyzer::get_path_from_fullqualifiedname(\$destinationdir);
        $destinationdir =~ s/\/\s*$//;  # removing ending slashes

        my $systemcall = "pkgmk -l 1073741824 -o -f $prototypefile -d $destinationdir 2\>\&1 |";
        installer::logger::print_message( "... $systemcall ...\n" );

        my $maxpkgmkcalls = 3;

        for ( my $i = 1; $i <= $maxpkgmkcalls; $i++ )
        {
            my @pkgmkoutput = ();

            open (PKGMK, "$systemcall");
            while (<PKGMK>) {push(@pkgmkoutput, $_); }
            close (PKGMK);

            my $returnvalue = $?;   # $? contains the return value of the systemcall

            my $infoline = "Systemcall (Try $i): $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);

            for ( my $j = 0; $j <= $#pkgmkoutput; $j++ )
            {
                if ( $i < $maxpkgmkcalls ) { $pkgmkoutput[$j] =~ s/\bERROR\b/PROBLEM/ig; }
                push( @installer::globals::logfileinfo, "$pkgmkoutput[$j]");
            }

            if ($returnvalue)
            {
                $infoline = "Try $i : Could not execute \"$systemcall\"!\n";
                push( @installer::globals::logfileinfo, $infoline);
                if ( $i == $maxpkgmkcalls ) { installer::exiter::exit_program("ERROR: \"$systemcall\"!", "create_packages_without_epm"); }
            }
            else
            {
                installer::logger::print_message( "Success (Try $i): \"$systemcall\"\n" );
                $infoline = "Success: Executed \"$systemcall\" successfully!\n";
                push( @installer::globals::logfileinfo, $infoline);
                last;
            }
        }

        # It might be necessary to save uncompressed Solaris packages

        # compressing packages

        if ( ! $installer::globals::solarisdontcompress )
        {
            my $faspac = "faspac-so.sh";

            my $compressorref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$faspac, $includepatharrayref, 0);
            if ($$compressorref ne "")
            {
                # Saving original pkginfo, to set time stamp later
                my $pkginfoorig = "$destinationdir/$packagename/pkginfo";
                my $pkginfotmp = "$destinationdir/$packagename" . ".pkginfo.tmp";
                $systemcall = "cp -p $pkginfoorig $pkginfotmp";
                installer::systemactions::make_systemcall($systemcall);

                $faspac = $$compressorref;
                $infoline = "Found compressor: $faspac\n";
                push( @installer::globals::logfileinfo, $infoline);

                installer::logger::print_message( "... $faspac ...\n" );
                installer::logger::include_timestamp_into_logfile("Starting $faspac");

                $systemcall = "/bin/sh $faspac -a -q -d $destinationdir $packagename";  # $faspac has to be the absolute path!
                installer::systemactions::make_systemcall($systemcall);

                # Setting time stamp for pkginfo, because faspac-so.sh
                # changed the pkginfo file, updated the size and
                # checksum, but not the time stamp.
                $systemcall = "touch -r $pkginfotmp $pkginfoorig";
                installer::systemactions::make_systemcall($systemcall);
                if ( -f $pkginfotmp ) { unlink($pkginfotmp); }

                installer::logger::include_timestamp_into_logfile("End of $faspac");
            }
            else
            {
                $infoline = "Not found: $faspac\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }

        # Setting unix rights to "775" for all created directories inside the package

        $systemcall = "cd $destinationdir; find $packagename -type d | xargs -i chmod 775 \{\} \;";
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


        ######################
        # making pkg files
        ######################

        # my $streamname = $packagename . ".pkg";
        # $systemcall = "pkgtrans $destinationdir $streamname $packagename";
        # print "... $systemcall ...\n";

        # $returnvalue = system($systemcall);

        # $infoline = "Systemcall: $systemcall\n";
        # push( @installer::globals::logfileinfo, $infoline);

        # if ($returnvalue)
        # {
        # $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        # push( @installer::globals::logfileinfo, $infoline);
        # }
        # else
        # {
        # $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        # push( @installer::globals::logfileinfo, $infoline);
        # }

        #########################
        # making tar.gz files
        #########################

        # my $targzname = $packagename . ".tar.gz";
        # $systemcall = "cd $destinationdir; tar -cf - $packagename | gzip > $targzname";
        # print "... $systemcall ...\n";

        # $returnvalue = system($systemcall);

        # $infoline = "Systemcall: $systemcall\n";
        # push( @installer::globals::logfileinfo, $infoline);

        # if ($returnvalue)
        # {
        # $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        # push( @installer::globals::logfileinfo, $infoline);
        # }
        # else
        # {
        # $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        # push( @installer::globals::logfileinfo, $infoline);
        # }

    }

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
        elsif ( $installer::globals::platformid eq 'aix_powerpc')
        {
            $target = "ppc";
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

        my $systemcall = "$rpmcommand -bb --define \"_unpackaged_files_terminate_build  0\" $specfilename --target $target $buildrootstring 2\>\&1 |";

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

                $infoline = "\nLast try: Using $rpmprog directly (problem with LD_LIBRARY_PATH)\n";
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

    if ( $installer::globals::issolarispkgbuild )
    {
        my @extensions = ();
        push(@extensions, ".pkginfo");
        push(@extensions, ".prototype");
        push(@extensions, ".postinstall");
        push(@extensions, ".postremove");
        push(@extensions, ".preinstall");
        push(@extensions, ".preremove");
        push(@extensions, ".depend");

        for ( my $i = 0; $i <= $#extensions; $i++ )
        {
            my $removefile = $epmdir . $packagename . $extensions[$i];
            my $destfile = $loggingdir . $packagename . $extensions[$i] . ".log";

            if (! -f $removefile) { next; }

            my $systemcall = "mv -f $removefile $destfile";
            system($systemcall);     # ignoring the return value
            $infoline = "Systemcall: $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    if ( $installer::globals::isrpmbuild )
    {
        my $removefile = $epmdir . $packagename . ".spec";
        my $destfile = $loggingdir . $packagename . ".spec.log";

        my $systemcall = "mv -f $removefile $destfile";
        system($systemcall);     # ignoring the return value
        $infoline = "Systemcall: $systemcall\n";
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
        elsif ( $installer::globals::platformid eq 'aix_powerpc')
        {
            $machine = "ppc";
            $rpmdir = "$installer::globals::epmoutpath/RPMS/$machine";
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
# Collect modules with product specific styles.
######################################################

sub collect_modules_with_style
{
    my ($style, $modulesarrayref) = @_;

    my @allmodules = ();

    for ( my $i = 0; $i <= $#{$modulesarrayref}; $i++ )
    {
        my $onemodule = ${$modulesarrayref}[$i];
        my $styles = "";
        if ( $onemodule->{'Styles'} ) { $styles = $onemodule->{'Styles'}; }
        if ( $styles =~ /\b\Q$style\E\b/ )
        {
            push(@allmodules, $onemodule);
        }
    }

    return \@allmodules;
}

######################################################
# Remove modules without packagecontent.
######################################################

sub remove_modules_without_package
{
    my ($allmodules) = @_;

    my @allmodules = ();

    for ( my $i = 0; $i <= $#{$allmodules}; $i++ )
    {
        my $onemodule = ${$allmodules}[$i];
        my $packagename = "";
        if ( $onemodule->{'PackageName'} ) { $packagename = $onemodule->{'PackageName'}; }
        if ( $packagename ne "" )
        {
            push(@allmodules, $onemodule);
        }
    }

    return \@allmodules;
}

######################################################
# Copying files for system integration.
######################################################

sub copy_and_unpack_tar_gz_files
{
    my ($sourcefile, $destdir) = @_;

    my $systemcall = "cd $destdir; cat $sourcefile | gunzip | tar -xf -";
    installer::systemactions::make_systemcall($systemcall);
}

######################################################
# Checking whether the new content is a directory and
# not a package. If it is a directory, the complete
# content of the directory has to be added to the
# array newcontent.
######################################################

sub control_subdirectories
{
    my ($content, $subdir) = @_;

    my @newcontent = ();

    for ( my $i = 0; $i <= $#{$content}; $i++ )
    {
        if ( -d ${$content}[$i] )
        {
            $subdir = ${$content}[$i];
            installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$subdir);
            my $allpackages = installer::systemactions::read_directory(${$content}[$i]);
            for ( my $j = 0; $j <= $#{$allpackages}; $j++ )
            {
                # Currently only Linux rpm is supported, debian packages cannot be installed via xpd installer
                if (( $installer::globals::islinuxbuild ) && ( ! ( ${$allpackages}[$j] =~ /\.rpm\s*$/ ))) { next; }
                push(@newcontent, ${$allpackages}[$j]);
            }
        }
        else
        {
            push(@newcontent, ${$content}[$i]);
        }
    }

    return (\@newcontent, $subdir);
}

######################################################
# Including the system integration files into the
# installation sets.
######################################################

sub put_systemintegration_into_installset
{
    my ($newdir, $includepatharrayref, $allvariables, $modulesarrayref) = @_;

    my $destdir = $newdir;

    # adding System integration files

    my $sourcefile = "";

    # Finding the modules defined in scp (with flag SYSTEMMODULE)
    # Getting name of package from scp-Module
    # Search package in list off all include files
    # Copy file into installation set and unpack it (always tar.gz)
    # tar.gz can contain a different number of packages -> automatically create hidden sub modules

    # Collect all modules with flag "SYSTEMMODULE"
    my $allmodules = collect_modules_with_style("SYSTEMMODULE", $modulesarrayref);
    $allmodules = remove_modules_without_package($allmodules);

    for ( my $i = 0; $i <= $#{$allmodules}; $i++ )
    {
        my $onemodule = ${$allmodules}[$i];
        my $packagetarfilename = $onemodule->{'PackageName'};

        my $infoline = "Including into installation set: $packagetarfilename\n";
        push( @installer::globals::logfileinfo, $infoline);

        my $sourcepathref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$packagetarfilename, $includepatharrayref, 1);
        if ( $$sourcepathref eq "" ) { installer::exiter::exit_program("ERROR: Source path not found for $packagetarfilename!", "copy_systemintegration_files"); }

        # Collecting all packages in directory "packages" or "RPMS"
        my $oldcontent = installer::systemactions::read_directory($destdir);

        copy_and_unpack_tar_gz_files($$sourcepathref, $destdir);

        # Finding new content -> that is the package name
        my ($newcontent, $allcontent ) = installer::systemactions::find_new_content_in_directory($destdir, $oldcontent);

        # special handling, if new content is a directory
        my $subdir = "";
        if ( ! $installer::globals::issolarispkgbuild ) { ($newcontent, $subdir) = control_subdirectories($newcontent); }

        # Adding license content into Solaris packages
        if (( $installer::globals::issolarispkgbuild ) && ( $installer::globals::englishlicenseset ) && ( ! $variableshashref->{'NO_LICENSE_INTO_COPYRIGHT'} )) { _add_license_into_systemintegrationpackages($destdir, $newcontent); }
    }
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
        # $$staticpathref is already "/opt/openoffice.org3", no additional $rootpath required.
    }

}

################################################
# Defining the English license text to add
# it into Solaris packages.
################################################

sub _set_english_license
{
    my $additional_license_name = $installer::globals::englishsolarislicensename;   # always the English file
    my $licensefileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$additional_license_name, "" , 0);
    if ( $$licensefileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find license file $additional_license_name!", "set_english_license"); }
    $installer::globals::englishlicenseset = 1;
    $installer::globals::englishlicense = installer::files::read_file($$licensefileref);
    installer::scpzipfiles::replace_all_ziplistvariables_in_file($installer::globals::englishlicense, $variableshashref);
}

################################################
# Adding the content of the English license
# file into the system integration packages.
################################################

sub _add_license_into_systemintegrationpackages
{
    my ($destdir, $packages) = @_;

    for ( my $i = 0; $i <= $#{$packages}; $i++ )
    {
        my $copyrightfilename = ${$packages}[$i] . $installer::globals::separator . "install" . $installer::globals::separator . "copyright";
        if ( ! -f $copyrightfilename ) { installer::exiter::exit_program("ERROR: Could not find license file in system integration package: $copyrightfilename!", "add_license_into_systemintegrationpackages"); }
        my $copyrightfile = installer::files::read_file($copyrightfilename);

        # Saving time stamp of old copyrightfile
        my $savcopyrightfilename = $copyrightfilename . ".sav";
        installer::systemactions::copy_one_file($copyrightfilename, $savcopyrightfilename);
        _set_time_stamp_for_file($copyrightfilename, $savcopyrightfilename); # now $savcopyrightfile has the time stamp of $copyrightfile

        # Adding license content to copyright file
        push(@{$copyrightfile}, "\n");
        for ( my $i = 0; $i <= $#{$installer::globals::englishlicense}; $i++ ) { push(@{$copyrightfile}, ${$installer::globals::englishlicense}[$i]); }
        installer::files::save_file($copyrightfilename, $copyrightfile);

        # Setting the old time stamp saved with $savcopyrightfilename
        _set_time_stamp_for_file($savcopyrightfilename, $copyrightfilename); # now $copyrightfile has the time stamp of $savcopyrightfile
        unlink($savcopyrightfilename);

        # Changing content of copyright file in pkgmap
        my $pkgmapfilename = ${$packages}[$i] . $installer::globals::separator . "pkgmap";
        if ( ! -f $pkgmapfilename ) { installer::exiter::exit_program("ERROR: Could not find pkgmap in system integration package: $pkgmapfilename!", "add_license_into_systemintegrationpackages"); }
        my $pkgmap = installer::files::read_file($pkgmapfilename);
        _change_onefile_in_pkgmap($pkgmap, $copyrightfilename, "copyright");
        installer::files::save_file($pkgmapfilename, $pkgmap);
    }
}

##############################################
# Setting time stamp of copied files to avoid
# errors from pkgchk.
##############################################

sub _set_time_stamp_for_file
{
    my ($sourcefile, $destfile) = @_;

    my $systemcall = "touch -r $sourcefile $destfile";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: \"$systemcall\" failed!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: \"$systemcall\" !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

##############################################
# Setting checksum and wordcount for changed
# pkginfo file into pkgmap.
##############################################

sub _change_onefile_in_pkgmap
{
    my ($pkgmapfile, $fullfilename, $shortfilename) = @_;

    # 1 i pkginfo 442 34577 1166716297
    # ->
    # 1 i pkginfo 443 34737 1166716297
    #
    # wc -c pkginfo | cut -f6 -d' '  -> 442  (variable)
    # sum pkginfo | cut -f1 -d' '  -> 34577  (variable)
    # grep 'pkginfo' pkgmap | cut -f6 -d' '  -> 1166716297  (fix)

    my $checksum = _call_sum($fullfilename);
    if ( $checksum =~ /^\s*(\d+)\s+.*$/ ) { $checksum = $1; }

    my $wordcount = _call_wc($fullfilename);
    if ( $wordcount =~ /^\s*(\d+)\s+.*$/ ) { $wordcount = $1; }

    for ( my $i = 0; $i <= $#{$pkgmapfile}; $i++ )
    {
        if ( ${$pkgmapfile}[$i] =~ /(^.*\b\Q$shortfilename\E\b\s+)(\d+)(\s+)(\d+)(\s+)(\d+)(\s*$)/ )
        {
            my $newline = $1 . $wordcount . $3 . $checksum . $5 . $6 . $7;
            ${$pkgmapfile}[$i] = $newline;
            last;
        }
    }
}

#########################################################
# Calling sum
#########################################################

sub _call_sum
{
    my ($filename) = @_;

    $sumfile = "/usr/bin/sum";

    if ( ! -f $sumfile ) { installer::exiter::exit_program("ERROR: No file /usr/bin/sum", "call_sum"); }

    my $systemcall = "$sumfile $filename |";

    my $sumoutput = "";

    open (SUM, "$systemcall");
    $sumoutput = <SUM>;
    close (SUM);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

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

    return $sumoutput;
}

#########################################################
# Calling wc
# wc -c pkginfo | cut -f6 -d' '
#########################################################

sub _call_wc
{
    my ($filename) = @_;

    $wcfile = "/usr/bin/wc";

    if ( ! -f $wcfile ) { installer::exiter::exit_program("ERROR: No file /usr/bin/wc", "call_wc"); }

    my $systemcall = "$wcfile -c $filename |";

    my $wcoutput = "";

    open (WC, "$systemcall");
    $wcoutput = <WC>;
    close (WC);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

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

    return $wcoutput;
}

1;
