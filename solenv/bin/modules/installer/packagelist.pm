#*************************************************************************
#
#   $RCSfile: packagelist.pm,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2005-06-17 09:48:54 $
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

package installer::packagelist;

use installer::converter;
use installer::remover;

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

###################################################
# Setting the modules for language packs
###################################################

sub analyze_list_languagepack
{
    my ($packagelist) = @_;

    @allpackages = ();

    for ( my $i = 0; $i <= $#{$packagelist}; $i++ )
    {
        my $onepackage = ${$packagelist}[$i];

        my $onegid = $onepackage->{'module'};

        installer::remover::remove_leading_and_ending_whitespaces(\$onegid);

        my @allmodules = ();

        push(@allmodules, $onegid);

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
# into the root module ($packagename eq gid_Module_Root)
#####################################################################

sub find_dirs_for_package
{
    my ($dirlist, $filelist, $linklist, $packagename) = @_;

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

        # also investigating the flag CREATE (only added to gid_Module_Root)

        if (( ! $includedir ) && ( $packagename eq "gid_Module_Root" ))
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
        $$packagenameref =~ s/\%$key/$value/;
    }
}

1;
