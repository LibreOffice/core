#*************************************************************************
#
#   $RCSfile: epmfile.pm,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: is $ $Date: 2004-07-29 11:09:48 $
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

package installer::epmfile;

use Cwd;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::packagelist;
use installer::pathanalyzer;
use installer::remover;
use installer::scriptitems;

############################################################################
# The header file contains the strings for the epm header in all languages
############################################################################

sub get_string_from_headerfile
{
    my ($searchstring, $language, $fileref) = @_;

    my $returnstring  = "";
    my $onestring  = "";
    my $englishstring  = "";
    my $foundblock = 0;
    my $foundstring = 0;
    my $foundenglishstring = 0;
    my $englishidentifier = "01";

    $searchstring = "[" . $searchstring . "]";

    for ( my $i = 0; $i <= $#{$fileref}; $i++ )
    {
        my $line = ${$fileref}[$i];

        if ( $line =~ /^\s*\Q$searchstring\E\s*$/ )
        {
            $foundblock = 1;
            my $counter = $i + 1;

            $line = ${$fileref}[$counter];

            # Beginning of the next block oder Dateiende

            while ((!($line =~ /^\s*\[\s*\w+\s*\]\s*$/ )) && ( $counter <= $#{$fileref} ))
            {
                if ( $line =~ /^\s*\Q$language\E\s+\=\s*\"(.*)\"\s*$/ )
                {
                    $onestring = $1;
                    $foundstring = 1;
                    last;
                }

                if ( $line =~ /^\s*\Q$englishidentifier\E\s+\=\s*\"(.*)\"\s*$/ )
                {
                    $englishstring = $1;
                    $foundenglishstring = 1;
                }

                $counter++;
                $line = ${$fileref}[$counter];
            }
        }
    }

    if ( $foundstring )
    {
        $returnstring = $onestring;
    }
    else
    {
        if ( $foundenglishstring )
        {
            $returnstring = $englishstring;
        }
        else
        {
            installer::exiter::exit_program("ERROR: No string found for $searchstring in epm header file (-h)", "get_string_from_headerfile");
        }
    }

    return \$returnstring;
}

##########################################################
# Filling the epm file with directories, files and links
##########################################################

sub put_directories_into_epmfile
{
    my ($directoriesarrayref, $epmfileref) = @_;

    for ( my $i = 0; $i <= $#{$directoriesarrayref}; $i++ )
    {
        my $onedir = ${$directoriesarrayref}[$i];
        my $dir = "";

        if ( $onedir->{'Dir'} ) { $dir = $onedir->{'Dir'}; }

        # if (!($dir =~ /\bPREDEFINED_/ ))
        if ((!($dir =~ /\bPREDEFINED_/ )) || ( $dir =~ /\bPREDEFINED_PROGDIR\b/ ))
        {
            my $hostname = $onedir->{'HostName'};

            my $line = "d 755 root sys $hostname -\n";

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

        my $line = "f $unixrights root sys $destination $sourcepath\n";

        push(@{$epmfileref}, $line)
    }
}

sub put_links_into_epmfile
{
    my ($linksinproductarrayref, $epmfileref) = @_;

    for ( my $i = 0; $i <= $#{$linksinproductarrayref}; $i++ )
    {
        my $onelink = ${$linksinproductarrayref}[$i];
        my $destination = $onelink->{'destination'};
        my $destinationfile = $onelink->{'destinationfile'};

        my $line = "l 000 root sys $destination $destinationfile\n";

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

    my ($licensefilename, $readmefilename);

    my $foundlicensefile = 0;
    my $foundreadmefile = 0;

    # %product OpenOffice.org Software
    # %version 2.0
    # %description A really great software
    # %copyright 1999-2003 by OOo
    # %vendor OpenOffice.org
    # %license /test/replace/01/LICENSE01
    # %readme /test/replace/01/README01
    # %requires foo
    # %provides bar

    # The first language in the languages array determines the language of license and readme file

    my $searchlanguage = ${$languagesref}[0];

    # Setting the global variable $installer:globals::listfileproductname

    if ( $variableshashref->{'PRODUCTEXTENSION'} )
    {
        $installer::globals::listfileproductname = $variableshashref->{'PRODUCTNAME'} . " " . $variableshashref->{'PRODUCTEXTENSION'};
    }
    else
    {
        $installer::globals::listfileproductname = $variableshashref->{'PRODUCTNAME'};
    }

    # Productame and Productversion are stored in ziplistfile
    my $line = "";
    $line = "%product" . " " . $installer::globals::listfileproductname . "\n";
    push(@epmheader, $line);

    # Determining the release version
    # This release version has to be listed in the line %version : %version versionnumber releasenumber

    my $releasenumber = 1;  # default
    if ( $variableshashref->{'RELEASENUMBER'} ) { $releasenumber = $variableshashref->{'RELEASENUMBER'}; }

    $line = "%version" . " " . $variableshashref->{'PRODUCTVERSION'} . " " . $releasenumber . "\n";
    push(@epmheader, $line);

    # Description, Copyright and Vendor are multilingual and are defined in
    # the string file for the header file ($headerfileref)

#   my $onestringref = get_string_from_headerfile("description", $searchlanguage, $headerfileref);
#   $line = "%description" . " " . $$onestringref . "\n";

    my $descriptionstring = $onepackage->{'description'};
    installer::packagelist::resolve_packagevariables(\$descriptionstring, $variableshashref, 0);
    $line = "%description" . " " . $descriptionstring . "\n";
    push(@epmheader, $line);

#   $onestringref = get_string_from_headerfile("copyright", $searchlanguage, $headerfileref);
#   $line = "%copyright" . " " . $$onestringref . "\n";
    my $copyrightstring = $onepackage->{'copyright'};
    installer::packagelist::resolve_packagevariables(\$copyrightstring, $variableshashref, 0);
    $line = "%copyright" . " " . $copyrightstring . "\n";
    push(@epmheader, $line);

#   $onestringref = get_string_from_headerfile("vendor", $searchlanguage, $headerfileref);
#   $line = "%vendor" . " " . $$onestringref . "\n";
    my $vendorstring = $onepackage->{'vendor'};
    installer::packagelist::resolve_packagevariables(\$vendorstring, $variableshashref, 0);
    $line = "%vendor" . " " . $vendorstring . "\n";
    push(@epmheader, $line);

    # License and Readme file can be included automatically from the file list

    if ( $installer::globals::iswindowsbuild )
    {
        $licensefilename = "license.txt";
        $readmefilename = "readme.txt";
    }
    else
    {
        $licensefilename = "LICENSE";
        $readmefilename = "README";
    }

    for ( my $i = 0; $i <= $#{$filesinproduct}; $i++ )
    {
        my $onefile = ${$filesinproduct}[$i];
        my $filename = $onefile->{'Name'};

        if ( $filename eq $licensefilename )
        {
            $foundlicensefile = 1;
            $line = "%license" . " " . $onefile->{'sourcepath'} . "\n";
            push(@epmheader, $line);
        }

        if ( $filename eq $readmefilename )
        {
            $foundreadmefile = 1;
            $line = "%readme" . " " . $onefile->{'sourcepath'} . "\n";
            push(@epmheader, $line);
        }

        if ( $foundlicensefile && $foundreadmefile )
        {
            last;
        }
    }

    if (!($foundlicensefile))
    {
        installer::exiter::exit_program("ERROR: Could not find license file $licensefilename", "create_epm_header");
    }

    if (!($foundreadmefile))
    {
        installer::exiter::exit_program("ERROR: Could not find readme file $readmefilename", "create_epm_header");
    }

    # including the directives for %requires and %provides

    my $provides = "";
    my $requires = "";

    if ( $installer::globals::issolarisbuild )
    {
        $provides = "solarisprovides";   # the name in the packagelist
        $requires = "solarisrequires";   # the name in the packagelist
    }
    else
    {
        $provides = "provides";         # the name in the packagelist
        $requires = "requires";         # the name in the packagelist
    }

    if ( $onepackage->{$provides} )
    {
        my $providesstring = $onepackage->{$provides};
        installer::packagelist::resolve_packagevariables(\$providesstring, $variableshashref, 1);
        $line = "%provides" . " " . $providesstring . "\n";
        push(@epmheader, $line);
    }

    if ( $onepackage->{$requires} )
    {
        my $requiresstring = $onepackage->{$requires};
        installer::packagelist::resolve_packagevariables(\$requiresstring, $variableshashref, 1);
        $line = "%requires" . " " . $requiresstring . "\n";
        push(@epmheader, $line);
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

###################################################
# Replace one in shell scripts ( %VARIABLENAME )
###################################################

sub replace_percent_variable_in_shellscripts
{
    my ($scriptref, $variable, $searchstring) = @_;

    for ( my $i = 0; $i <= $#{$scriptref}; $i++ )
    {
        ${$scriptref}[$i] =~ s/\%$searchstring/$variable/g;
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
        $value = lc($value);    # lowercase !
        $value =~ s/\.org//g;   # openoffice instead of openoffice.org
        replace_variable_in_shellscripts($scriptref, $value, $key);
    }
}

#######################################
# Adding shell scripts to epm file
#######################################

sub adding_shellscripts_to_epm_file
{
    my ($epmfileref, $shellscriptsfilename, $localrootpath, $allvariableshashref) = @_;

    # $installer::globals::shellscriptsfilename

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

    # searching for epm. It has to be found in the solver or it has to be in the path (saved in $installer::globals::epm_in_path)

    my $epmname = "epm";
    my $epmfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$epmname, $includepatharrayref, 0);

    if (($$epmfileref eq "") && (!($installer::globals::epm_in_path))) { installer::exiter::exit_program("ERROR: Could not find program $epmname!", "find_epm_on_system"); }
    if (($$epmfileref eq "") && ($installer::globals::epm_in_path)) { $epmname = $installer::globals::epm_path; }
    if (!($$epmfileref eq "")) { $epmname = $$epmfileref; }

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

}

#################################################
# Calling epm to create the installation sets
#################################################

sub call_epm
{
    my ($epmname, $epmlistfilename, $allvariables, $packagename, $includepatharrayref) = @_;

    installer::logger::include_header_into_logfile("epm call for $packagename");

    my $packageformat = $installer::globals::packageformat;

    # my $systemcall = $epmname . " -f " . $packageformat . " " . $packagename . " " . $epmlistfilename;
    my $systemcall = $epmname . " -f " . $packageformat . " " . $packagename . " " . $epmlistfilename . " 2\>\&1 |";

    print "... $systemcall ...\n";

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

        for ( my $j = 0; $j <= $#epmoutput; $j++ ) { push( @installer::globals::logfileinfo, "$epmoutput[$j]"); }

        if ($returnvalue)
        {
            $infoline = "Try $i : Could not execute \"$systemcall\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
            if ( $i == $maxepmcalls ) { installer::exiter::exit_program("ERROR: \"$systemcall\"!", "call_epm"); }
            }
        else
        {
            print "Success (Try $i): \"$systemcall\"\n";
            $infoline = "Success: Executed \"$systemcall\" successfully!\n";
            push( @installer::globals::logfileinfo, $infoline);
            last;
        }
    }
}

###############################################################
# Finding the complete file name for the pkginfo (Solaris)
# or spec file (Linux) created by epm
###############################################################

sub get_completefilename
{
    my ($searchfile) = @_;

    my $completefile = "";

    my $systemcall = "find . -name $searchfile |";
    open (FIND, "$systemcall");
    $completefile = <FIND>;
    close (FIND);

    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( $completefile eq "" )
    {
        $infoline = "ERROR: Could not find file $searchfile !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $completefile;
}

#####################################################################
# Adding the new line for relocatables into pkginfo file (Solaris)
# or spec file (Linux) created by epm
#####################################################################

sub add_one_line_into_file
{
    my ($file, $insertline, $filename) = @_;

    if ( $installer::globals::issolarisbuild )
    {
        push(@{$file}, $insertline);        # simply adding at the end of pkginfo file
    }

    if ( $installer::globals::islinuxbuild )
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
# Also adding the new line: "AutoReqProv: no"
#####################################################################

sub set_topdir_in_specfile
{
    my ($changefile, $filename, $newepmdir) = @_;

    $newepmdir =~ s/^\s*.//;    # removing leading "."
    $newepmdir = cwd() . $newepmdir; # only absolute path allowed

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

    # Adding "topdir" and "autoreqprov" behind the line beginning with: Prefix:

    my $inserted_line = 0;

    my $topdirline = "\%define _topdir $newepmdir\n";
    my $autoreqprovline = "AutoReqProv\: no\n";

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        if ( ${$changefile}[$i] =~ /^\s*Prefix\:\s*/ )
        {
            splice(@{$changefile},$i+1,0,$topdirline);
            $inserted_line = 1;
            $topdirline =~ s/\s*$//;
            my $infoline = "Success: Added line $topdirline into file $filename!\n";
            push( @installer::globals::logfileinfo, $infoline);

            # also including the line "AutoReqProv\: no\n"

            splice(@{$changefile},$i+1,0,$autoreqprovline);
            $autoreqprovline =~ s/\s*$//;
            $infoline = "Success: Added line $autoreqprovline into file $filename!\n";
            push( @installer::globals::logfileinfo, $infoline);

            last;
        }
    }

    if (! $inserted_line) { installer::exiter::exit_program("ERROR: Did not find string \"Prefix:\" in file: $filename", "set_topdir_in_specfile"); }

}

#####################################################################
# Setting the release number in the spec file
#####################################################################

sub set_releaseversion_in_specfile
{
    my ($changefile, $variableshashref) = @_;

    my $releasenumber = 1;  # default value
    if ( $variableshashref->{'RELEASENUMBER'} ) { $releasenumber = $variableshashref->{'RELEASENUMBER'}; }

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        if ( ${$changefile}[$i] =~ /^\s*Release\s*:\s*(\d+)\s*$/ )
        {
            my $number = $1;

            if ( $number != $releasenumber )
            {
                ${$changefile}[$i] =~ s/$number/$releasenumber/;
                my $infoline = "Info: Changed release number in spec file from $number to $releasenumber!\n";
                push( @installer::globals::logfileinfo, $infoline);
                last;
            }
        }
    }
}

#########################################################
# Building relocatable Solaris packages means:
# 1. Add "BASEDIR=/opt" into pkginfo
# 2. Remove "/opt/" from all objects in prototype file
# For step2 this function exists
# Sample: d none /opt/openofficeorg20/help 0755 root sys
# -> d none openofficeorg20/help 0755 root sys
#########################################################

sub make_prototypefile_relocatable
{
    my ($prototypefile, $relocatablepath) = @_;

    for ( my $i = 0; $i <= $#{$prototypefile}; $i++ )
    {
        if ( ${$prototypefile}[$i] =~ /^\s*\w\s+\w+\s+\/\w+/ )  # this is an object line
        {
            ${$prototypefile}[$i] =~ s/$relocatablepath//;
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

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        if ( ${$scriptfile}[$i] =~ /\Q$oldstring\E/ )
        {
            ${$scriptfile}[$i] =~ s/\Q$oldstring\E/$newstring/;
            my $infoline = "Info: Substituting in $scriptfilename $oldstring by $newstring\n";
            push(@installer::globals::logfileinfo, $infoline);
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
# Including the relocatable directory into
# spec file and pkginfo file
# Linux: set topdir in specfile
# Solaris: remove $relocatablepath (/opt/)
# for all objects in prototype file
# and changing "topdir" for Linux
############################################################

sub prepare_packages
{
    my ($loggingdir, $packagename, $staticpath, $relocatablepath, $onepackage, $variableshashref) = @_;

    my $filename;
    my $newline;

    my $localrelocatablepath = $relocatablepath;
    $localrelocatablepath =~ s/\/\s*$//;

    if ( $installer::globals::issolarisbuild )
    {
        $filename = $packagename . ".pkginfo";
        $newline = "BASEDIR\=" . $localrelocatablepath . "\n";
    }

    if ( $installer::globals::islinuxbuild )
    {
        $filename =  $packagename . ".spec";
        $newline = "Prefix\:\ " . $localrelocatablepath . "\n";
    }

    my $completefilename = get_completefilename($filename);
    if ($completefilename eq "") { installer::exiter::exit_program("ERROR: Did not find file: $filename", "prepare_packages"); }
    my $changefile = installer::files::read_file($completefilename);
    add_one_line_into_file($changefile, $newline, $filename);
    installer::files::save_file($completefilename, $changefile);

    my $newepmdir = $completefilename;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$newepmdir);

    # adding new "topdir" and removing old "topdir" in specfile

    if ( $installer::globals::islinuxbuild )
    {
        set_topdir_in_specfile($changefile, $filename, $newepmdir);
        set_releaseversion_in_specfile($changefile, $variableshashref);
        installer::files::save_file($completefilename, $changefile);
    }

    # removing the relocatable path in prototype file

    if ( $installer::globals::issolarisbuild )
    {
        my $prototypefilename = $packagename . ".prototype";
        $prototypefilename = $newepmdir . $prototypefilename;
        if (! -f $prototypefilename) { installer::exiter::exit_program("ERROR: Did not find prototype file: $prototypefilename", "prepare_packages"); }

        my $prototypefile = installer::files::read_file($prototypefilename);
        make_prototypefile_relocatable($prototypefile, $relocatablepath);
        installer::files::save_file($prototypefilename, $prototypefile);

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

    my $systemcall = "rpm --version |";
    open (RPM, "$systemcall");
    $rpmout = <RPM>;
    close (RPM);

    $rpmout =~ s/\s*$//g;

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( $rpmout eq "" ) { $infoline = "ERROR: Could not find file $searchfile !\n"; }
    else { $infoline = "Success: rpm version: $rpmout\n"; }

    push( @installer::globals::logfileinfo, $infoline);

    if ( $rpmout =~ /(\d+)\.(\d+)\.(\d+)/ ) { $rpmversion = $1; }
    else { installer::exiter::exit_program("ERROR: Unknown format: $rpmout ! Expected: a.b.c", "determine_rpm_version"); }

    return $rpmversion;
}

#################################################
# Systemcall to start the packaging process
#################################################

sub create_packages_without_epm
{
    my ($epmdir, $packagename) = @_;

    # Solaris: pkgmk -o -f solaris-2.8-sparc/SUNWso8m34.prototype -d solaris-2.8-sparc
    # Solaris: pkgtrans solaris-2.8-sparc SUNWso8m34.pkg SUNWso8m34
    # Solaris: tar -cf - SUNWso8m34 | gzip > SUNWso8m34.tar.gz

    if ( $installer::globals::issolarisbuild )
    {
        my $prototypefile = $epmdir . $packagename . ".prototype";
        if (! -f $prototypefile) { installer::exiter::exit_program("ERROR: Did not find file: $prototypefile", "create_packages_without_epm"); }

        my $destinationdir = $prototypefile;
        installer::pathanalyzer::get_path_from_fullqualifiedname(\$destinationdir);
        $destinationdir =~ s/\/\s*$//;  # removing ending slashes

        # my $systemcall = "pkgmk -o -f $prototypefile -d $destinationdir \> /dev/null 2\>\&1";
        my $systemcall = "pkgmk -o -f $prototypefile -d $destinationdir 2\>\&1 |";
        print "... $systemcall ...\n";

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

            for ( my $j = 0; $j <= $#pkgmkoutput; $j++ ) { push( @installer::globals::logfileinfo, "$pkgmkoutput[$j]"); }

            if ($returnvalue)
            {
                $infoline = "Try $i : Could not execute \"$systemcall\"!\n";
                push( @installer::globals::logfileinfo, $infoline);
                if ( $i == $maxpkgmkcalls ) { installer::exiter::exit_program("ERROR: \"$systemcall\"!", "create_packages_without_epm"); }
            }
            else
            {
                print "Success (Try $i): \"$systemcall\"\n";
                $infoline = "Success: Executed \"$systemcall\" successfully!\n";
                push( @installer::globals::logfileinfo, $infoline);
                last;
            }
        }

        # Setting unix rights to "775" for all created directories inside the package

        $systemcall = "cd $destinationdir; find $packagename -type d -exec chmod 775 \{\} \\\;";
        print "... $systemcall ...\n";

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

        # making pkg files

        # my $streamname = $packagename . ".pkg";
        # $systemcall = "pkgtrans $destinationdir $streamname $packagename";
        # print "... $systemcall ...\n";

        # $returnvalue = system($systemcall);

        # $infoline = "Systemcall: $systemcall\n";
        # push( @installer::globals::logfileinfo, $infoline);

        # if ($returnvalue)
        # {
        #   $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        #   push( @installer::globals::logfileinfo, $infoline);
        # }
        # else
        # {
        #   $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        #   push( @installer::globals::logfileinfo, $infoline);
        # }

        # making tar.gz files

        my $targzname = $packagename . ".tar.gz";
        $systemcall = "cd $destinationdir; tar -cf - $packagename | gzip > $targzname";
        print "... $systemcall ...\n";

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

    # Linux: rpm -bb so8m35.spec    ( -> dependency check abklemmen? )

    if ( $installer::globals::islinuxbuild )
    {
        my $specfilename = $epmdir . $packagename . ".spec";
        if (! -f $specfilename) { installer::exiter::exit_program("ERROR: Did not find file: $specfilename", "create_packages_without_epm"); }

        my $rpmcommand = "rpm";
        my $rpmversion = determine_rpm_version();

        if ( $rpmversion >= 4 ) { $rpmcommand = "rpmbuild"; }

        # my $systemcall = "$rpmcommand -bb $specfilename --target i586 \> /dev/null";
        my $systemcall = "$rpmcommand -bb $specfilename --target i586 2\>\&1 |";
        print "... $systemcall ...\n";

        my $maxrpmcalls = 3;

        for ( my $i = 1; $i <= $maxrpmcalls; $i++ )
        {
            my @rpmoutput = ();

            open (RPM, "$systemcall");
            while (<RPM>) {push(@rpmoutput, $_); }
            close (RPM);

            my $returnvalue = $?;   # $? contains the return value of the systemcall

            my $infoline = "Systemcall (Try $i): $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);

            for ( my $j = 0; $j <= $#rpmoutput; $j++ ) { push( @installer::globals::logfileinfo, "$rpmoutput[$j]"); }

            if ($returnvalue)
            {
                $infoline = "Try $i : Could not execute \"$systemcall\"!\n";
                push( @installer::globals::logfileinfo, $infoline);
                if ( $i == $maxrpmcalls ) { installer::exiter::exit_program("ERROR: \"$systemcall\"!", "create_packages_without_epm"); }
            }
            else
            {
                print "Success (Try $i): \"$systemcall\"\n";
                $infoline = "Success: Executed \"$systemcall\" successfully!\n";
                push( @installer::globals::logfileinfo, $infoline);
                last;
            }
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

    if ( $installer::globals::issolarisbuild )
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

        # removing the package

#       my $removedir = $epmdir . $packagename;
#
#       my $systemcall = "rm -rf $removedir";
#
#       print "... $systemcall ...\n";
#
#       my $returnvalue = system($systemcall);
#
#       my $infoline = "Systemcall: $systemcall\n";
#       push( @installer::globals::logfileinfo, $infoline);
#
#       if ($returnvalue)
#       {
#           $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
#           push( @installer::globals::logfileinfo, $infoline);
#       }
#       else
#       {
#           $infoline = "Success: Executed \"$systemcall\" successfully!\n";
#           push( @installer::globals::logfileinfo, $infoline);
#       }
    }

    if ( $installer::globals::islinuxbuild )
    {
        my $removefile = $epmdir . $packagename . ".spec";
        my $destfile = $loggingdir . $packagename . ".spec.log";

         # if (! -f $removefile) { next; }

        my $systemcall = "mv -f $removefile $destfile";
        system($systemcall);     # ignoring the return value
        $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        # removing the directory "buildroot"

        my $removedir = $epmdir . "buildroot";

        $systemcall = "rm -rf $removedir";

        print "... $systemcall ...\n";

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
    }
}

######################################################
# Creating a better directory structure in the solver.
# This is also preparation for the Java installer.
# Linux: Removing the directory "linux-..."
# Solaris: Renaming "solaris-..." to "packages"
######################################################

sub create_new_directory_structure
{
    my ($newepmdir) = @_;

    my $localdir = $newepmdir;
    installer::remover::remove_ending_pathseparator(\$localdir);
    my $newdir = "";

    if ( $installer::globals::issolarisbuild )
    {
        # my $directoryname = installer::systemactions::get_directoryname($newepmdir, "solaris");

        $newdir = "packages";

        my $systemcall = "mv $localdir $newdir";

        my $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not move \"$localdir\" to \"packages\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Moved \"$localdir\" to \"packages\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }

        my $localcall = "chmod 775 $newdir \>\/dev\/null 2\>\&1";
        system($localcall);

    }

    if ( $installer::globals::islinuxbuild )
    {
        # my $directoryname = installer::systemactions::get_directoryname($localdir, "linux");

        # creating a directory "RPMS" directly in the current directory

        $newdir = "RPMS";
        my $rpmdir = "$localdir/RPMS/i586";

        installer::systemactions::create_directory($newdir);

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

        installer::systemactions::remove_empty_directory("$localdir/RPMS/i586");
        installer::systemactions::remove_empty_directory("$localdir/RPMS/i386");
        installer::systemactions::remove_empty_directory("$localdir/RPMS");
        installer::systemactions::remove_empty_directory("$localdir");

    }

    return $newdir;
}

######################################################
# Including child packages into the
# installation set.
######################################################

sub put_childprojects_into_installset
{
    my ($newdir) = @_;

    my $infoline = "";

    # the source directory is defined with the parameter "-javafilespath"
    # in the variable $installer::globals::javafilespath (no extra path shall be used now (scp todo!)

    if (! ($installer::globals::javafilespath))
    {
        $infoline = "Warning: Cannot copy child project, \"-javafilespath\" not set!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        my $sourcedir = "$installer::globals::javafilespath";
        installer::remover::remove_ending_pathseparator(\$sourcedir);

        my $destdir = "$newdir";

        if ( $installer::globals::issolarissparcbuild ) { $sourcedirjava = $sourcedir . $installer::globals::separator . "java" . $installer::globals::separator . "solaris_sparc"; }
        if ( $installer::globals::issolarisx86build ) { $sourcedirjava = $sourcedir . $installer::globals::separator . "java" . $installer::globals::separator . "solaris_x86"; }
        if ( $installer::globals::islinuxbuild ) { $sourcedirjava = $sourcedir . $installer::globals::separator . "java" . $installer::globals::separator . "linux"; }

        installer::systemactions::copy_directory($sourcedirjava, $destdir);

        if ( $installer::globals::issolarissparcbuild ) { $sourcedirada = $sourcedir . $installer::globals::separator . "adabas" . $installer::globals::separator . "solaris_sparc"; }
        if ( $installer::globals::islinuxbuild ) { $sourcedirada = $sourcedir . $installer::globals::separator . "adabas" . $installer::globals::separator . "linux"; }

        if (( $installer::globals::issolarissparcbuild ) || ( $installer::globals::islinuxbuild ))
        {
            installer::systemactions::copy_directory($sourcedirada, $destdir);
        }

        # unpacking tar.gz files
    }
}

######################################################
# Including the Java installer into the
# installation sets.
######################################################

sub put_java_installer_into_installset
{
    my ($newdir) = @_;

    # the source directory is defined with the parameter "-javafilespath"
    # in the variable $installer::globals::javafilespath

    my $sourcedir = $installer::globals::javafilespath;
    installer::remover::remove_ending_pathseparator(\$sourcedir);

    my $destdir = ".";

    if ( $installer::globals::issolarissparcbuild ) { $sourcedir = $sourcedir . $installer::globals::separator . "solaris_sparc"; }
    if ( $installer::globals::issolarisx86build ) { $sourcedir = $sourcedir . $installer::globals::separator . "solaris_x86"; }
    if ( $installer::globals::islinuxbuild ) { $sourcedir = $sourcedir . $installer::globals::separator . "linux"; }

    installer::systemactions::copy_directory_except_fileextension($sourcedir, $destdir, "xml");

    # Setting Unix rights for Java starter (file "setup")

    my $localcall = "chmod 775 $destdir/setup \>\/dev\/null 2\>\&1";
    system($localcall);

}

######################################################
# Including the Java installer into the
# installation sets.
######################################################

sub put_systemintegration_into_installset
{
    my ($newdir, $includepatharrayref, $variables) = @_;

    # The system integration packages have to be found in the solver in the bin directory.
    # Linux: "staroffice-redhat-menus-8-1.noarch.rpm", "staroffice-suse-menus-8-1.noarch.rpm"
    # Solaris: "SUNWsogm.tar.gz"
    # scp Todo: This files have to be included into scp after removal of old setup.

    my @systemfiles = ();
    my $destdir = $newdir;

    # Attention: OOO has other names !

    if ( $installer::globals::issolarisbuild )
    {
        if ($installer::globals::product =~ /OpenOffice/i )
        {
            push(@systemfiles, "OOOopenoffice-gnome.tar.gz");
            push(@systemfiles, "OOOopenoffice-cde.tar.gz");
        }
        else
        {
            my $productname = lc($variables->{'PRODUCTNAME'});
            push(@systemfiles, "SUNW$productname-gnome-ea.tar.gz");
            push(@systemfiles, "SUNW$productname-cde-ea.tar.gz");
        }
    }

    if ( $installer::globals::islinuxbuild )
    {
        my $productversion = $variables->{'PRODUCTVERSION'};

        if ($installer::globals::product =~ /OpenOffice/i )
        {
            push(@systemfiles, "openoffice-redhat-menus-$productversion-1.noarch.rpm");
            push(@systemfiles, "openoffice-suse-menus-$productversion-1.noarch.rpm");
        }
        else
        {
            my $productname = lc($variables->{'PRODUCTNAME'});
            push(@systemfiles, "$productname-redhat-menus-ea-$productversion-1.noarch.rpm");
            push(@systemfiles, "$productname-suse-menus-ea-$productversion-1.noarch.rpm");
        }
    }

    for ( my $i = 0; $i <= $#systemfiles; $i++ )
    {
        my $onefilename = $systemfiles[$i];
        my $sourcepathref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$onefilename, $includepatharrayref, 1);

        my $destfile = $destdir . $installer::globals::separator . $systemfiles[$i];
        installer::systemactions::copy_one_file($$sourcepathref, $destfile);

        # unpacking the tar.gz file for Solaris

        if ( $installer::globals::issolarisbuild )
        {
            my $systemcall = "cd $destdir; cat $systemfiles[$i] | gunzip | tar -xf -";

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
}

######################################################
# Analyzing the Unix installation path.
# From the installation path /opt/openofficeorg20
# is the part /opt relocatable and the part
# openofficeorg20 static.
######################################################

sub analyze_rootpath
{
    my ($rootpath, $staticpathref, $relocatablepathref) = @_;

    $rootpath =~ s/\/\s*$//;    # removing ending slash

    my $staticpath = $rootpath;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$staticpath);
    $$staticpathref = $staticpath;              # will be "openofficeorg20"

    my $relocatablepath = $rootpath;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$relocatablepath);
    $$relocatablepathref = $relocatablepath;        # will be "/opt/"
}

1;
