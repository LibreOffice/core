#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: packagelist.pm,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: obo $ $Date: 2008-01-04 16:57:52 $
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

package installer::packagelist;

use installer::converter;
use installer::exiter;
use installer::globals;
use installer::remover;
use installer::scriptitems;

########################################
# Check existence of module
########################################

sub check_module_existence
{
    my ($onegid, $moduleslist) = @_;

    my $foundgid = 0;

    for ( my $i = 0; $i <= $#{$moduleslist}; $i++ )
    {
        my $gid = ${$moduleslist}[$i]->{'gid'};

        if ( $gid eq $onegid )
        {
            $foundgid = 1;
            last;
        }
    }

    return $foundgid;
}

###################################################
# Analyzing the gids, defined in the packagelist
###################################################

sub analyze_list
{
    my ($packagelist, $moduleslist) = @_;

    @allpackages = ();

    for ( my $i = 0; $i <= $#{$packagelist}; $i++ )
    {
        my $onepackage = ${$packagelist}[$i];

        my $onegid = $onepackage->{'module'};

        installer::remover::remove_leading_and_ending_whitespaces(\$onegid);

        my $moduleexists = check_module_existence($onegid, $moduleslist);

        if ( ! $moduleexists ) { next; }

        my @allmodules = ();

        push(@allmodules, $onegid);

        get_children($moduleslist, $onegid, \@allmodules);

        $onepackage->{'allmodules'} = \@allmodules;

        push(@allpackages, $onepackage);
    }

    return \@allpackages;
}

########################################################
# Recursively defined procedure to order
# modules and directories
########################################################

sub get_children
{
    my ($allitems, $startparent, $newitemorder) = @_;

    for ( my $i = 0; $i <= $#{$allitems}; $i++ )
    {
        my $gid = ${$allitems}[$i]->{'gid'};
        my $parent = "";
        if ( ${$allitems}[$i]->{'ParentID'} ) { $parent = ${$allitems}[$i]->{'ParentID'}; }

        if ( $parent eq $startparent )
        {
            push(@{$newitemorder}, $gid);
            my $parent = $gid;
            get_children($allitems, $parent, $newitemorder);    # recursive!
        }
    }
}

#####################################################################
# All modules below a defined gid_Module_A are collected now for
# each modules defined in the packagelist. Now the modules have
# to be removed, that are part of more than one package.
#####################################################################

sub remove_multiple_modules_packages
{
    my ($allpackagemodules) = @_;

    # iterating over all packages

    for ( my $i = 0; $i <= $#{$allpackagemodules}; $i++ )
    {
        my $onepackage = ${$allpackagemodules}[$i];
        my $allmodules = $onepackage->{'allmodules'};

        # print "Modules below $onepackage->{'module'}: $#{$allmodules}\n";

        # Comparing each package, with all following packages. If a
        # gid for the module is part of more than one package, it is
        # removed if the number of modules in the package is greater
        # in the current package than in the compare package.

        # Taking all modules from package $i

        my $packagecount = $#{$allmodules};

        my @optimizedpackage = ();

        # iterating over all modules of this package

        for ( my $j = 0; $j <= $#{$allmodules}; $j++ )
        {
            my $onemodule = ${$allmodules}[$j]; # this is the module, that shall be removed or not

            my $put_module_into_new_package = 1;

            # iterating over all other packages

            for ( my $k = 0; $k <= $#{$allpackagemodules}; $k++ )
            {
                if ( $k == $i ) { next; }   # not comparing equal module

                if (! $put_module_into_new_package) { next; } # do not compare, if already found

                my $comparepackage = ${$allpackagemodules}[$k];
                my $allcomparemodules = $comparepackage->{'allmodules'};

                my $comparepackagecount = $#{$allcomparemodules};

                # modules will only be removed from packages, that have more modules
                # than the compare package

                if ( $packagecount <= $comparepackagecount ) { next; }  # nothing to do, take next package

                # iterating over all modules of this package

                for ( my $m = 0; $m <= $#{$allcomparemodules}; $m++ )
                {
                    my $onecomparemodule = ${$allcomparemodules}[$m];

                    if ( $onemodule eq $onecomparemodule )  # this $onemodule has to be removed
                    {
                        $put_module_into_new_package = 0;
                    }
                }
            }

            if ( $put_module_into_new_package )
            {
                push(@optimizedpackage, $onemodule)
            }
        }

        $onepackage->{'allmodules'} = \@optimizedpackage;
    }

    # for ( my $i = 0; $i <= $#{$allpackagemodules}; $i++ )
    # {
    #   my $onepackage = ${$allpackagemodules}[$i];
    #   my $allmodules = $onepackage->{'allmodules'};
    #   print "New: Modules below $onepackage->{'module'}: $#{$allmodules}\n";
    # }

}

#####################################################################
# Analyzing all files if they belong to a special package.
# A package is described by a list of modules.
#####################################################################

sub find_files_for_package
{
    my ($filelist, $onepackage) = @_;

    my @newfilelist = ();

    for ( my $i = 0; $i <= $#{$filelist}; $i++ )
    {
        my $onefile = ${$filelist}[$i];
        my $modulesstring = $onefile->{'modules'};   # comma separated modules list
        my $moduleslist = installer::converter::convert_stringlist_into_array(\$modulesstring, ",");

        my $includefile = 0;

        # iterating over all modules of this file

        for ( my $j = 0; $j <= $#{$moduleslist}; $j++ )
        {
            if ( $includefile ) { next; }
            my $filemodule = ${$moduleslist}[$j];
            installer::remover::remove_leading_and_ending_whitespaces(\$filemodule);

            # iterating over all modules of the package

            my $packagemodules = $onepackage->{'allmodules'};

            for ( my $k = 0; $k <= $#{$packagemodules}; $k++ )
            {
                if ( $includefile ) { next; }
                my $packagemodule = ${$packagemodules}[$k];

                if ( $filemodule eq $packagemodule )
                {
                    $includefile = 1;
                    last;
                }
            }
        }

        if ( $includefile )
        {
            push(@newfilelist, $onefile);
        }
    }

    return \@newfilelist;
}

#####################################################################
# Analyzing all links if they belong to a special package.
# A package is described by a list of modules.
# A link is inserted into the package, if the corresponding
# file is also inserted.
#####################################################################

sub find_links_for_package
{
    my ($linklist, $filelist) = @_;

    # First looking for all links with a FileID.
    # Then looking for all links with a ShortcutID.

    my @newlinklist = ();

    for ( my $i = 0; $i <= $#{$linklist}; $i++ )
    {
        my $includelink = 0;

        my $onelink = ${$linklist}[$i];

        my $fileid = "";
        if ( $onelink->{'FileID'} ) { $fileid = $onelink->{'FileID'}; }

        if ( $fileid eq "" ) { next; }   # A link with a ShortcutID

        for ( my $j = 0; $j <= $#{$filelist}; $j++ )     # iterating over file list
        {
            my $onefile = ${$filelist}[$j];
            my $gid = $onefile->{'gid'};

            if ( $gid eq $fileid )
            {
                $includelink = 1;
                last;
            }
        }

        if ( $includelink )
        {
            push(@newlinklist, $onelink);
        }
    }

    # iterating over the new list, because of all links with a ShortcutID

    for ( my $i = 0; $i <= $#{$linklist}; $i++ )
    {
        my $includelink = 0;

        my $onelink = ${$linklist}[$i];

        my $shortcutid = "";
        if ( $onelink->{'ShortcutID'} ) { $shortcutid = $onelink->{'ShortcutID'}; }

        if ( $shortcutid eq "" ) { next; }   # A link with a ShortcutID

        for ( my $j = 0; $j <= $#newlinklist; $j++ )     # iterating over newly created link list
        {
            my $onefilelink = $newlinklist[$j];
            my $gid = $onefilelink->{'gid'};

            if ( $gid eq $shortcutid )
            {
                $includelink = 1;
                last;
            }
        }

        if ( $includelink )
        {
            push(@newlinklist, $onelink);
        }
    }

    return \@newlinklist;
}

#####################################################################
# Analyzing all directories if they belong to a special package.
# A package is described by a list of modules.
# Directories are included into the package, if they are needed
# by a file or a link included into the package.
# Attention: A directory with the flag CREATE, is only included
# into the root module:
# ($packagename eq $installer::globals::rootmodulegid)
#####################################################################

sub find_dirs_for_package
{
    my ($dirlist, $filelist, $linklist, $unixlinklist, $packagename) = @_;

    my @newdirlist = ();

    # iterating over all directories

    for ( my $i = 0; $i <= $#{$dirlist}; $i++ )
    {
        my $onedir = ${$dirlist}[$i];

        my $includedir = 0;

        my $hostname = "";

        if ( $onedir->{'HostName'} ) { $hostname = $onedir->{'HostName'}; }

        if ( $hostname eq "" ) { next; }

        # iterating over all files, looking for this hostname

        for ( my $j = 0; $j <= $#{$filelist}; $j++ )
        {
            my $onefile = ${$filelist}[$j];
            my $destination = $onefile->{'destination'};

            if ( $destination =~ /^\s*\Q$hostname\E/ )  # the directory path is part of the file path!
            {
                $includedir = 1;
                last;
            }
        }

        # also searching for links

        if ( ! $includedir )    # also looking for links
        {
            for ( my $j = 0; $j <= $#{$linklist}; $j++ )
            {
                my $onelink = ${$linklist}[$j];
                my $destination = $onelink->{'destination'};

                if ( $destination =~ /^\s*\Q$hostname\E/ )  # the directory path is part of the file path!
                {
                    $includedir = 1;
                    last;
                }
            }
        }

        # also searching for unix links

        if ( ! $includedir )    # also looking for links
        {
            for ( my $j = 0; $j <= $#{$unixlinklist}; $j++ )
            {
                my $onelink = ${$unixlinklist}[$j];
                my $destination = $onelink->{'destination'};

                if ( $destination =~ /^\s*\Q$hostname\E/ )  # the directory path is part of the file path!
                {
                    $includedir = 1;
                    last;
                }
            }
        }

        # also investigating the flag CREATE (only added to $installer::globals::rootmodulegid)

        if (( ! $includedir ) && ( $packagename eq $installer::globals::rootmodulegid ))
        {
            my $styles = "";

            if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }

            if ( $styles =~ /\bCREATE\b/ )
            {
                $includedir = 1;
            }
        }

        if ( $includedir )
        {
            push(@newdirlist, $onedir);
        }
    }

    return \@newdirlist;
}

#####################################################################
# Resolving all variables in the packagename.
#####################################################################

sub resolve_packagevariables
{
    my ($packagenameref, $variableshashref, $make_lowercase) = @_;

    my $key;

    foreach $key (keys %{$variableshashref})
    {
        my $value = $variableshashref->{$key};
        if ( $make_lowercase ) { $value = lc($value); }
        $$packagenameref =~ s/\%$key/$value/g;
    }
}

#####################################################################
# New packages system.
#####################################################################

##################################################################
# Controlling the content of the packagelist
# 1. Items in @installer::globals::packagelistitems must exist
# 2. If a shellscript file is defined, it must exist
##################################################################

sub check_packagelist
{
    my ($packages) = @_;

    if ( ! ( $#{$packages} > -1 )) { installer::exiter::exit_program("ERROR: No packages defined!", "check_packagelist"); }

    for ( my $i = 0; $i <= $#{$packages}; $i++ )
    {
        my $onepackage = ${$packages}[$i];

        my $element;

        # checking all items that must be defined

        foreach $element (@installer::globals::packagelistitems)
        {
            if ( ! exists($onepackage->{$element}) )
            {
                installer::exiter::exit_program("ERROR in package list: No value for $element !", "check_packagelist");
            }
        }

        # checking the existence of the script file, if defined

        if ( $onepackage->{'script'} )
        {
            my $scriptfile = $onepackage->{'script'};
            my $gid =  $onepackage->{'module'};
            my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$scriptfile, "" , 0);

            if ( $$fileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find script file $scriptfile for module $gid!", "check_packagelist"); }

            my $infoline = "$gid: Using script file: \"$$fileref\"!\n";
            push( @installer::globals::logfileinfo, $infoline);

            $onepackage->{'script'} = $$fileref;
        }
    }
}

#####################################################################
# Reading pack info for one module from packinfo file.
#####################################################################

sub get_packinfo
{
    my ($gid, $filename, $packages, $onelanguage) = @_;

    my $packagelist = installer::files::read_file($filename);

    my @allpackages = ();

    for ( my $i = 0; $i <= $#{$packagelist}; $i++ )
    {
        my $line = ${$packagelist}[$i];

        if ( $line =~ /^\s*\#/ ) { next; }  # this is a comment line

        if ( $line =~ /^\s*Start\s*$/i )    # a new package definition
        {
            my %onepackage = ();

            my $counter = $i + 1;

            while (!( ${$packagelist}[$counter] =~ /^\s*End\s*$/i ))
            {
                if ( ${$packagelist}[$counter] =~ /^\s*(\S+)\s*\=\s*\"(.*)\"/ )
                {
                    my $key = $1;
                    my $value = $2;
                    $onepackage{$key} = $value;
                }

                $counter++;
            }

            push(@allpackages, \%onepackage);
        }
    }

    # looking for the packinfo with the correct gid

    my $foundgid = 0;
    my $onepackage;
    foreach $onepackage (@allpackages)
    {
        # Adding the language to the module gid for LanguagePacks !
        # Making the module gid language specific: gid_Module_Root -> gir_Module_Root_pt_BR (as defined in scp2)
        if ( $onelanguage ne "" ) { $onepackage->{'module'} = $onepackage->{'module'} . "_$onelanguage"; }

        if ( $onepackage->{'module'} eq $gid )
        {
            # Resolving the language identifier
            my $onekey;
            foreach $onekey ( keys %{$onepackage} )
            {
                # Some keys require "-" instead of "_" for example in "en-US". All package names do not use underlines.
                my $locallang = $onelanguage;
                if (( $onekey eq "solarispackagename" ) ||
                   ( $onekey eq "solarisrequires" ) ||
                   ( $onekey eq "packagename" ) ||
                   ( $onekey eq "requires" )) { $locallang =~ s/_/-/g; } # avoiding illegal package abbreviation
                $onepackage->{$onekey} =~ s/\%LANGUAGESTRING/$locallang/;
            }
            push(@{$packages}, $onepackage);
            $foundgid = 1;
            last;
        }
    }

    if ( ! $foundgid )
    {
        installer::exiter::exit_program("ERROR: Could not find package info for module $gid in file \"$filename\"!", "get_packinfo");
    }
}

#####################################################################
# Collecting all packages from scp project.
#####################################################################

sub collectpackages
{
    my ( $allmodules, $languagesarrayref ) = @_;

    installer::logger::include_header_into_logfile("Collecting packages:");

    my @packages = ();
    my %gid_analyzed = ();

    my $onemodule;
    foreach $onemodule ( @{$allmodules} )
    {
        my $packageinfo = "PackageInfo";
        if (( $installer::globals::tab ) && ( $onemodule->{"TabPackageInfo"} )) { $packageinfo = "TabPackageInfo" }

        if ( $onemodule->{$packageinfo} )   # this is a package module!
        {
            my $modulegid = $onemodule->{'gid'};

            # Only collecting modules with correct language for language packs
#           if ( $installer::globals::languagepack ) { if ( ! ( $modulegid =~ /_$onelanguage\s*$/ )) { next; } }
            # Resetting language, if this is no language pack
#           if ( ! $installer::globals::languagepack ) { $onelanguage = ""; }

            # Ignoring packages, if they are marked to be ignored. Otherwise OOo 2.x update will break.
            # Style OOOIGNORE has to be removed for OOo 3.x
            my $styles = "";
            if ( $onemodule->{'Styles'} ) { $styles = $onemodule->{'Styles'}; }
            if (( $styles =~ /\bOOOIGNORE\b/ ) && ( $installer::globals::isopensourceproduct )) { next; }

            # checking modules with style LANGUAGEMODULE
            my $islanguagemodule = 0;
            my $onelanguage = "";
            if ( $styles =~ /\bLANGUAGEMODULE\b/ )
            {
                $islanguagemodule = 1;
                $onelanguage = $onemodule->{'Language'}; # already checked, that it is set.
                $onelanguage =~ s/-/_/g; # pt-BR -> pt_BR in scp
            }

            # Modules in different languages are listed more than once in multilingual installation sets
            if ( exists($gid_analyzed{$modulegid}) ) { next; }
            $gid_analyzed{$modulegid} = 1;

            my $packinfofile = $onemodule->{$packageinfo};

            # The file with package information has to be found in path list
            my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$packinfofile, "" , 0);

            if ( $$fileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $packinfofile for module $modulegid!", "collectpackages"); }

            my $infoline = "$modulegid: Using packinfo: \"$$fileref\"!\n";
            push( @installer::globals::logfileinfo, $infoline);

            get_packinfo($modulegid, $$fileref, \@packages, $onelanguage);
        }
    }

    return \@packages;
}

#####################################################################
# Printing packages content for debugging purposes
#####################################################################

sub log_packages_content
{
    my ($packages) = @_;

    if ( ! ( $#{$packages} > -1 )) { installer::exiter::exit_program("ERROR: No packages defined!", "print_content"); }

    installer::logger::include_header_into_logfile("Logging packages content:");

    my $infoline = "";

    for ( my $i = 0; $i <= $#{$packages}; $i++ )
    {
        my $onepackage = ${$packages}[$i];

        # checking all items that must be defined

        $infoline = "Package $onepackage->{'module'}\n";
        push(@installer::globals::logfileinfo, $infoline);

        my $key;
        foreach $key (sort keys %{$onepackage})
        {
            if ( $key =~ /^\s*\;/ ) { next; }

            if ( $key eq "allmodules" )
            {
                $infoline = "\t$key:\n";
                push(@installer::globals::logfileinfo, $infoline);
                my $onemodule;
                foreach $onemodule ( @{$onepackage->{$key}} )
                {
                    $infoline = "\t\t$onemodule\n";
                    push(@installer::globals::logfileinfo, $infoline);
                }
            }
            else
            {
                $infoline = "\t$key: $onepackage->{$key}\n";
                push(@installer::globals::logfileinfo, $infoline);
            }
        }

        $infoline = "\n";
        push(@installer::globals::logfileinfo, $infoline);

    }
}

#####################################################################
# Creating list of cabinet files from packages
#####################################################################

sub prepare_cabinet_files
{
    my ($packages, $allvariables) = @_;

    if ( ! ( $#{$packages} > -1 )) { installer::exiter::exit_program("ERROR: No packages defined!", "print_content"); }

    installer::logger::include_header_into_logfile("Preparing cabinet files:");

    my $infoline = "";

    for ( my $i = 0; $i <= $#{$packages}; $i++ )
    {
        my $onepackage = ${$packages}[$i];

        my $cabinetfile = "$onepackage->{'packagename'}\.cab";

        resolve_packagevariables(\$cabinetfile, $allvariables, 0);

        $installer::globals::allcabinets{$cabinetfile} = 1;

        # checking all items that must be defined

        $infoline = "Package $onepackage->{'module'}\n";
        push(@installer::globals::logfileinfo, $infoline);

        # Assigning the cab file to the module and also to all corresponding sub modules

        my $onemodule;
        foreach $onemodule ( @{$onepackage->{'allmodules'}} )
        {
            if ( ! exists($installer::globals::allcabinetassigns{$onemodule}) )
            {
                $installer::globals::allcabinetassigns{$onemodule} = $cabinetfile;
            }
            else
            {
                my $infoline = "Warning: Already existing assignment: $onemodule : $installer::globals::allcabinetassigns{$onemodule}\n";
                push(@installer::globals::logfileinfo, $infoline);
                $infoline = "Ignoring further assignment: $onemodule : $cabinetfile\n";
                push(@installer::globals::logfileinfo, $infoline);
            }
        }
    }
}

#####################################################################
# Logging assignments of cabinet files
#####################################################################

sub log_cabinet_assignments
{
    installer::logger::include_header_into_logfile("Logging cabinet files:");

    my $infoline = "List of cabinet files:\n";
    push(@installer::globals::logfileinfo, $infoline);

    my $key;
    foreach $key ( sort keys %installer::globals::allcabinets ) { push(@installer::globals::logfileinfo, "\t$key\n"); }

    $infoline = "\nList of assignments from modules to cabinet files:\n";
    push(@installer::globals::logfileinfo, $infoline);

    foreach $key ( sort keys %installer::globals::allcabinetassigns ) { push(@installer::globals::logfileinfo, "\t$key : $installer::globals::allcabinetassigns{$key}\n"); }
}

1;
