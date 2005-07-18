#*************************************************************************
#
#   $RCSfile: epmfile.pm,v $
#
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
use installer::converter;
use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
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

        # if (!($dir =~ /\bPREDEFINED_/ ))
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
    my $group = "bin";

    if ( $installer::globals::islinuxbuild )
    {
        $group = "root";
    }

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
        my $line = "$filetype $unixrights root $group $destination $sourcepath\n";

        push(@{$epmfileref}, $line)
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

    my $line = "";

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

    # using the description for the %product line in the epm list file

    my $productnamestring = $onepackage->{'description'};
    installer::packagelist::resolve_packagevariables(\$productnamestring, $variableshashref, 0);
    if ( $variableshashref->{'PRODUCTEXTENSION'} ) { $productnamestring = $productnamestring . " " . $variableshashref->{'PRODUCTEXTENSION'}; }

    $line = "%product" . " " . $productnamestring . "\n";
    push(@epmheader, $line);

    # Determining the release version
    # This release version has to be listed in the line %version : %version versionnumber releasenumber

    if ( $variableshashref->{'PACKAGEVERSION'} ) { $installer::globals::packageversion = $variableshashref->{'PACKAGEVERSION'}; }
    if ( $variableshashref->{'PACKAGEREVISION'} ) { $installer::globals::packagerevision = $variableshashref->{'PACKAGEREVISION'}; }

    $line = "%version" . " " . $installer::globals::packageversion . " " . $installer::globals::packagerevision . "\n";
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
    }
    else
    {
        $licensefilename = "LICENSE";
        $readmefilename = "README";
    }

    if ( $installer::globals::languagepack )    # in language packs the files LICENSE and README are removed, because they are not language specific
    {
        if ( $installer::globals::iswindowsbuild )
        {
            $licensefilename = "license_$searchlanguage.txt";
            $readmefilename = "readme_$searchlanguage.txt";
        }
        else
        {
            $licensefilename = "LICENSE_$searchlanguage";
            $readmefilename = "README_$searchlanguage";
        }
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

    if ( $installer::globals::issolarispkgbuild )
    {
        $provides = "solarisprovides";   # the name in the packagelist
        $requires = "solarisrequires";   # the name in the packagelist
    }
    else
    {
        $provides = "provides";         # the name in the packagelist
        $requires = "requires";         # the name in the packagelist
    }

    if ( $installer::globals::patch )
    {
        $onepackage->{$provides} = "";
        $onepackage->{$requires} = "";
    }

    if ( $onepackage->{$provides} )
    {
        my $providesstring = $onepackage->{$provides};

        my $allprovides = installer::converter::convert_stringlist_into_array(\$providesstring, ",");

        for ( my $i = 0; $i <= $#{$allprovides}; $i++ )
        {
            my $oneprovides = ${$allprovides}[$i];
            installer::packagelist::resolve_packagevariables(\$oneprovides, $variableshashref, 1);
            $line = "%provides" . " " . $oneprovides . "\n";
            push(@epmheader, $line);
        }
    }

    if ( $onepackage->{$requires} )
    {
        my $requiresstring = $onepackage->{$requires};

        if ( $installer::globals::add_required_package ) { $requiresstring = $requiresstring . "," . $installer::globals::add_required_package; }

        my $allrequires = installer::converter::convert_stringlist_into_array(\$requiresstring, ",");

        for ( my $i = 0; $i <= $#{$allrequires}; $i++ )
        {
            my $onerequires = ${$allrequires}[$i];
            installer::packagelist::resolve_packagevariables(\$onerequires, $variableshashref, 1);
            $line = "%requires" . " " . $onerequires . "\n";
            push(@epmheader, $line);
        }
    }
    else
    {
        if ( $installer::globals::add_required_package )
        {
            my $requiresstring = $installer::globals::add_required_package;

            my $allrequires = installer::converter::convert_stringlist_into_array(\$requiresstring, ",");

            for ( my $i = 0; $i <= $#{$allrequires}; $i++ )
            {
                my $onerequires = ${$allrequires}[$i];
                installer::packagelist::resolve_packagevariables(\$onerequires, $variableshashref, 1);
                $line = "%requires" . " " . $onerequires . "\n";
                push(@epmheader, $line);
            }
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
        $value =~ s/\.org/org/g;    # openofficeorg instead of openoffice.org
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

}

#################################################
# Calling epm to create the installation sets
#################################################

sub call_epm
{
    my ($epmname, $epmlistfilename, $packagename) = @_;

    installer::logger::include_header_into_logfile("epm call for $packagename");

    my $packageformat = $installer::globals::packageformat;

    my $outdirstring = "";
    if ( $installer::globals::epmoutpath ne "" ) { $outdirstring = " --output-dir $installer::globals::epmoutpath"; }

    my $systemcall = $epmname . " -f " . $packageformat . " " . $packagename . " " . $epmlistfilename . $outdirstring . " 2\>\&1 |";

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

    if ( $installer::globals::islinuxrpmbuild )
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
    my ($file, $filename, $variables) = @_;

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

    if ( $variables->{$pkgversion} )
    {
        if ( $variables->{$pkgversion} ne "FINALVERSION" )
        {
            my $versionstring = $variables->{$pkgversion};

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
# Setting Patch information for Respin versions
# into pkginfo file. This prevents Respin versions
# from patching.
########################################################

sub set_patchlist_in_pkginfo_for_respin
{
    my ($changefile, $filename, $allvariables) = @_;

    my $patchlistname = "SOLSPARCPATCHLISTFORRESPIN";
    if ( $installer::globals::issolarisx86build ) { $patchlistname = "SOLIAPATCHLISTFORRESPIN"; }

    if ( $allvariables->{$patchlistname} )
    {
        my $newline = "PATCHLIST=" . $allvariables->{$patchlistname} . "\n";
        add_one_line_into_file($changefile, $newline, $filename);

        # Adding patch info for each  patch in the patchlist
        # patchlist separator is a blank

        my $allpatchesstring = $allvariables->{$patchlistname};
        my $allpatches = installer::converter::convert_whitespace_stringlist_into_array(\$allpatchesstring);

        for ( my $i = 0; $i <= $#{$allpatches}; $i++ )
        {
            my $patchid = ${$allpatches}[$i];
            my $key = "PATCH_INFO_" . $patchid;
            $key =~ s/\s*$//;

            if ( ! $allvariables->{$key} ) { installer::exiter::exit_program("ERROR: No Patch info available in zip list file for $key", "set_patchlist_in_pkginfo"); }
            $newline = $key . "=" . $allvariables->{$key} . "\n";
            add_one_line_into_file($changefile, $newline, $filename);
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

    $newline = "HOTLINE=Please contact your local service provider\n";
    add_one_line_into_file($changefile, $newline, $filename);

    $newline = "EMAIL=\n";
    add_one_line_into_file($changefile, $newline, $filename);
}

#####################################################################
# Adding a new line for topdir into specfile, removing old
# topdir if set.
#####################################################################

sub set_topdir_in_specfile
{
    my ($changefile, $filename, $newepmdir) = @_;

    # $newepmdir =~ s/^\s*\.//; # removing leading "."
    $newepmdir = cwd() . $installer::globals::separator . $newepmdir; # only absolute path allowed

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

    # Adding "topdir" behind the line beginning with: Prefix:

    my $inserted_line = 0;

    my $topdirline = "\%define _topdir $newepmdir\n";

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        if ( ${$changefile}[$i] =~ /^\s*Prefix\:\s*/ )
        {
            splice(@{$changefile},$i+1,0,$topdirline);
            $inserted_line = 1;
            $topdirline =~ s/\s*$//;
            my $infoline = "Success: Added line $topdirline into file $filename!\n";
            push( @installer::globals::logfileinfo, $infoline);
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

    my $releasenumber = $installer::globals::packagerevision;
#   if ( $variableshashref->{'RELEASENUMBER'} ) { $releasenumber = $variableshashref->{'RELEASENUMBER'}; }

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
# Setting the Auto[Req]Prov line and __find_requires
#####################################################################

sub set_autoprovreq_in_specfile
{
    my ($changefile, $findrequires, $bindir) = @_;

    my $autoreqprovline;

    if ( $findrequires )
    {
        $autoreqprovline = "AutoProv\: no\n%define __find_requires $bindir/$findrequires\n";
    }
    else
    {
        $autoreqprovline = "AutoReqProv\: no\n";
    }

    for ( my $i = 0; $i <= $#{$changefile}; $i++ )
    {
        # Adding "autoreqprov" behind the line beginning with: Prefix:
        if ( ${$changefile}[$i] =~ /^\s*Prefix\:\s*/ )
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
# Syntax: License: LGPL, SISSL
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

    $relocatablepath =~ s/\/\s*$//;     # removing the ending slash

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
                        if (! installer::existence::exists_in_array($onefile->{'SolarisClass'}, \@newclasses))
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

    if ( $installer::globals::islinuxrpmbuild )
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

############################################################
# A Solaris patch contains 7 specific scripts
############################################################

sub add_scripts_into_prototypefile
{
    my ($prototypefile) = @_;

    # The files are stored in the directory $installer::globals::patchincludepath
    # The file names are available via @installer::globals::solarispatchscripts

    my $path = $installer::globals::patchincludepath;
    $path =~ s/\/\s*$//;
    $path = $path . $installer::globals::separator;

    my @newlines = ();

    for ( my $i = 0; $i <= $#installer::globals::solarispatchscripts; $i++ )
    {
        my $line = "i $installer::globals::solarispatchscripts[$i]=" . $path . $installer::globals::solarispatchscripts[$i] . "\n";
        push(@newlines, $line);
    }

    # Including the new lines after the last line starting with "i"

    for ( my $i = 0; $i <= $#{$prototypefile}; $i++ )
    {
        if ( ${$prototypefile}[$i] =~ /^\s*i\s+copyright/ )
        {
            splice(@{$prototypefile}, $i, 1);   # ignoring old copyright text, using patch standard
            next;
        }
        if ( ${$prototypefile}[$i] =~ /^\s*i\s+/ ) { next; }
        splice(@{$prototypefile}, $i, 0, @newlines);
        last;
    }
}

############################################################
# Adding patch infos in pkginfo file
############################################################

sub include_patchinfos_into_pkginfo
{
    my ( $changefile, $filename, $variableshashref ) = @_;

    # SUNW_PATCHID=101998-10
    # SUNW_OBSOLETES=114999-01 113999-01
    # SUNW_PKGTYPE=usr
    # SUNW_PKGVERS=1.0

    my $patchidname = "SOLSPARCPATCHID";
    if ( $installer::globals::issolarisx86build ) { $patchidname = "SOLIAPATCHID"; }

    if ( ! $variableshashref->{$patchidname} ) { installer::exiter::exit_program("ERROR: Variable $patchidname not defined in zip list file!", "include_patchinfos_into_pkginfo"); }

    my $newline = "SUNW_PATCHID=" . $variableshashref->{$patchidname} . "\n";
    add_one_line_into_file($changefile, $newline, $filename);

    my $patchobsoletesname = "SOLSPARCPATCHOBSOLETES";
    if ( $installer::globals::issolarisx86build ) { $patchobsoletesname = "SOLIAPATCHOBSOLETES"; }

    my $obsoletes = "";
    if ( $variableshashref->{$patchobsoletesname} ) { $obsoletes = $variableshashref->{$patchobsoletesname}; }
    $newline = "SUNW_OBSOLETES=" . $obsoletes . "\n";
    add_one_line_into_file($changefile, $newline, $filename);

    # $newline = "SUNW_PKGTYPE=usr\n";
    # add_one_line_into_file($changefile, $newline, $filename);

    # $newline = "SUNW_PKGVERS=1.0\n";
    # add_one_line_into_file($changefile, $newline, $filename);
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
    elsif ( $sollanguage eq "es" ) { $sollanguage = "es"; }
    elsif ( $sollanguage eq "fr" ) { $sollanguage = "fr"; }
    elsif ( $sollanguage eq "it" ) { $sollanguage = "it"; }
    elsif ( $sollanguage eq "sv" ) { $sollanguage = "sv"; }
    elsif ( $sollanguage eq "pt_BR" ) { $sollanguage = "pt_BR"; }
    elsif ( $sollanguage eq "ja" ) { $sollanguage = "ja,ja_JP.PCK,ja_JP.UTF-8"; }
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

    my $solarislanguage = get_solaris_language_for_langpack($$languagestringref);

    my $newline = "SUNW_LOC=" . $solarislanguage . "\n";
    add_one_line_into_file($changefile, $newline, $filename);

    if ( $onepackage->{'pkg_list_entry'} )
    {
        my $packagelistentry = $onepackage->{'pkg_list_entry'};
        installer::packagelist::resolve_packagevariables(\$packagelistentry, $variableshashref, 1);
        $newline = "SUNW_PKGLIST=" . $packagelistentry . "\n";
        add_one_line_into_file($changefile, $newline, $filename);
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
    $localrelocatablepath =~ s/\/\s*$//;

    if ( $installer::globals::issolarispkgbuild )
    {
        $filename = $packagename . ".pkginfo";
        $newline = "BASEDIR\=" . $localrelocatablepath . "\n";
    }

    if ( $installer::globals::islinuxrpmbuild )
    {
        $filename =  $packagename . ".spec";
        $newline = "Prefix\:\ " . $localrelocatablepath . "\n";
    }

    my $completefilename = $newepmdir . $filename;

    if ( ! -f $completefilename) { installer::exiter::exit_program("ERROR: Did not find file: $completefilename", "prepare_packages"); }
    my $changefile = installer::files::read_file($completefilename);
    add_one_line_into_file($changefile, $newline, $filename);
    installer::files::save_file($completefilename, $changefile);

    # my $newepmdir = $completefilename;
    # installer::pathanalyzer::get_path_from_fullqualifiedname(\$newepmdir);

    # adding new "topdir" and removing old "topdir" in specfile

    if ( $installer::globals::islinuxrpmbuild )
    {
        set_topdir_in_specfile($changefile, $filename, $newepmdir);
        set_releaseversion_in_specfile($changefile, $variableshashref);
        set_autoprovreq_in_specfile($changefile, $onepackage->{'findrequires'}, "$installer::globals::unpackpath" . "/bin");
        set_packager_in_specfile($changefile);
        set_license_in_specfile($changefile, $variableshashref);
        set_tab_into_datafile($changefile, $filesref);
        # check_requirements_in_specfile($changefile);
        installer::files::save_file($completefilename, $changefile);
    }

    # removing the relocatable path in prototype file

    if ( $installer::globals::issolarispkgbuild )
    {
        set_revision_in_pkginfo($changefile, $filename, $variableshashref);
        set_maxinst_in_pkginfo($changefile, $filename);
        set_solaris_parameter_in_pkginfo($changefile, $filename, $variableshashref);
        if ( ! $installer::globals::patch ) { set_patchlist_in_pkginfo_for_respin($changefile, $filename, $variableshashref); }
        if ( $installer::globals::patch ) { include_patchinfos_into_pkginfo($changefile, $filename, $variableshashref); }
        if ( $installer::globals::languagepack ) { include_languageinfos_into_pkginfo($changefile, $filename, $languagestringref, $onepackage, $variableshashref); }
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

        if ( $installer::globals::patch ) { add_scripts_into_prototypefile($prototypefile); }

        installer::files::save_file($prototypefilename, $prototypefile);
    }

    return $newepmdir;
}

############################################################
# Linux requirement for perl is changed by epm from
# /usr/bin/perl to perl .
# Requires: perl
############################################################

sub check_requirements_in_specfile
{
    my ( $specfile ) = @_;

    for ( my $i = 0; $i <= $#{$specfile}; $i++ )
    {
        if (( ${$specfile}[$i] =~ /^\s*Requires/ ) && ( ${$specfile}[$i] =~ /\bperl\b/ ) && ( ! (  ${$specfile}[$i] =~ /\/usr\/bin\/perl\b/ )))
        {
            my $oldline = ${$specfile}[$i];
            ${$specfile}[$i] =~ s/perl/\/usr\/bin\/perl/;
            my $newline = ${$specfile}[$i];

            $oldline =~ s/\s*$//;
            $newline =~ s/\s*$//;
            my $infoline = "Spec File: Changing content from \"$oldline\" to \"$newline\".\n";
            push(@installer::globals::logfileinfo, $infoline);
        }
    }
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

    if ( $rpmout eq "" ) { $infoline = "ERROR: Could not find file \"rpm\" !\n"; }
    else { $infoline = "Success: rpm version: $rpmout\n"; }

    push( @installer::globals::logfileinfo, $infoline);

    if ( $rpmout =~ /(\d+)\.(\d+)\.(\d+)/ ) { $rpmversion = $1; }
    elsif ( $rpmout =~ /(\d+)\.(\d+)/ ) { $rpmversion = $1; }
    elsif ( $rpmout =~ /(\d+)/ ) { $rpmversion = $1; }
    else { installer::exiter::exit_program("ERROR: Unknown format: $rpmout ! Expected: \"a.b.c\", or \"a.b\", or \"a\"", "determine_rpm_version"); }

    return $rpmversion;
}

#################################################
# Systemcall to start the packaging process
#################################################

sub create_packages_without_epm
{
    my ($epmdir, $packagename, $includepatharrayref) = @_;

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

        # my $systemcall = "pkgmk -o -f $prototypefile -d $destinationdir \> /dev/null 2\>\&1";
        my $systemcall = "pkgmk -o -f $prototypefile -d $destinationdir 2\>\&1 |";
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

        # compressing packages

        my $faspac = "faspac-so.sh";

        my $compressorref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$faspac, $includepatharrayref, 0);
        if ($$compressorref ne "")
        {
            $faspac = $$compressorref;
            $infoline = "Found compressor: $faspac\n";
            push( @installer::globals::logfileinfo, $infoline);

            installer::logger::print_message( "... $faspac ...\n" );
            installer::logger::include_timestamp_into_logfile("Starting $faspac");

             $systemcall = "/bin/sh $faspac -a -q -d $destinationdir $packagename";  # $faspac has to be the absolute path!
             make_systemcall($systemcall);

            installer::logger::include_timestamp_into_logfile("End of $faspac");
        }
        else
        {
            $infoline = "Not found: $faspac\n";
            push( @installer::globals::logfileinfo, $infoline);
        }

        # Setting unix rights to "775" for all created directories inside the package

        $systemcall = "cd $destinationdir; find $packagename -type d -exec chmod 775 \{\} \\\;";
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
        #   $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        #   push( @installer::globals::logfileinfo, $infoline);
        # }
        # else
        # {
        #   $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        #   push( @installer::globals::logfileinfo, $infoline);
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
        #   $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        #   push( @installer::globals::logfileinfo, $infoline);
        # }
        # else
        # {
        #   $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        #   push( @installer::globals::logfileinfo, $infoline);
        # }
    }

    # Linux: rpm -bb so8m35.spec    ( -> dependency check abklemmen? )

    if ( $installer::globals::islinuxrpmbuild )
    {
        my $specfilename = $epmdir . $packagename . ".spec";
        if (! -f $specfilename) { installer::exiter::exit_program("ERROR: Did not find file: $specfilename", "create_packages_without_epm"); }

        my $rpmcommand = "rpm";
        my $rpmversion = determine_rpm_version();

        if ( $rpmversion >= 4 ) { $rpmcommand = "rpmbuild"; }

        # saving globally for later usage
        $installer::globals::rpmcommand = $rpmcommand;
        $installer::globals::rpmquerycommand = "rpm"; # For queries "rpm" is used, not "rpmbuild"

        my $target = "";
        if ( $installer::globals::compiler =~ /unxlngi/) { $target = "i586"; }
        if ( $installer::globals::compiler =~ /unxlngx/) { $target = "x86_64"; }

        my $systemcall = "$rpmcommand -bb $specfilename --target $target 2\>\&1 |";

        installer::logger::print_message( "... $systemcall ...\n" );

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

            for ( my $j = 0; $j <= $#rpmoutput; $j++ )
            {
                if ( $i < $maxrpmcalls ) { $rpmoutput[$j] =~ s/\bERROR\b/PROBLEM/ig; }
                push( @installer::globals::logfileinfo, "$rpmoutput[$j]");
            }

            if ($returnvalue)
            {
                $infoline = "Try $i : Could not execute \"$systemcall\"!\n";
                push( @installer::globals::logfileinfo, $infoline);
                if ( $i == $maxrpmcalls ) { installer::exiter::exit_program("ERROR: \"$systemcall\"!", "create_packages_without_epm"); }
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

    if ( $installer::globals::islinuxrpmbuild )
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
    }
}

######################################################
# Making the systemcall
######################################################

sub make_systemcall
{
    my ($systemcall) = @_;

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

###########################################################
# Creating a better directory structure in the solver.
###########################################################

sub create_new_directory_structure
{
    my ($newepmdir) = @_;

    my $newdir = $installer::globals::epmoutpath;

    if ( $installer::globals::islinuxrpmbuild )
    {
        my $rpmdir;
        if ( $installer::globals::compiler =~ /unxlngi/) { $rpmdir = "$installer::globals::epmoutpath/RPMS/i586"; }
        if ( $installer::globals::compiler =~ /unxlngx/) { $rpmdir = "$installer::globals::epmoutpath/RPMS/x86_64"; }

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

        installer::systemactions::remove_empty_directory("$installer::globals::epmoutpath/RPMS/x86_64");
        installer::systemactions::remove_empty_directory("$installer::globals::epmoutpath/RPMS/i586");
        installer::systemactions::remove_empty_directory("$installer::globals::epmoutpath/RPMS/i386");
        installer::systemactions::remove_empty_directory("$installer::globals::epmoutpath/RPMS");

    }

    return $newdir;
}

######################################################
# Including child packages into the
# installation set.
######################################################

sub put_childprojects_into_installset
{
    my ($newdir, $allvariables) = @_;

    my $infoline = "";

    my $sopackpath = "";
    if ( $ENV{'SO_PACK'} ) { $sopackpath  = $ENV{'SO_PACK'}; }
    else { installer::exiter::exit_program("ERROR: Environment variable SO_PACK not set!", "add_childprojects"); }

    my $destdir = "$newdir";

    # adding Java

    my $sourcefile = "";

    if ( $allvariables->{'JAVAPRODUCT'} )
    {
        if ( $installer::globals::javafilename ne "" )
        {
            $sourcefile = $sopackpath . $installer::globals::separator . $installer::globals::compiler . $installer::globals::separator . "jre" . $installer::globals::separator . $installer::globals::javafilename;
            if ( ! -f $sourcefile ) { installer::exiter::exit_program("ERROR: Java file not found: $sourcefile !", "put_childprojects_into_installset"); }
            installer::systemactions::copy_one_file($sourcefile, $destdir);
        }

        if ( $installer::globals::javafilename2 ne "" )
        {
            $sourcefile = $sopackpath . $installer::globals::separator . $installer::globals::compiler . $installer::globals::separator . "jre" . $installer::globals::separator . $installer::globals::javafilename2;
            if ( ! -f $sourcefile ) { installer::exiter::exit_program("ERROR: Java file not found: $sourcefile !", "put_childprojects_into_installset"); }
            installer::systemactions::copy_one_file($sourcefile, $destdir);
        }

        if ( $installer::globals::javafilename3 ne "" )
        {
            $sourcefile = $sopackpath . $installer::globals::separator . $installer::globals::compiler . $installer::globals::separator . "jre" . $installer::globals::separator . $installer::globals::javafilename3;
            if ( ! -f $sourcefile ) { installer::exiter::exit_program("ERROR: Java file not found: $sourcefile !", "put_childprojects_into_installset"); }
            installer::systemactions::copy_one_file($sourcefile, $destdir);
        }
    }

    # adding Ada

    if ( $allvariables->{'ADAPRODUCT'} )
    {
        if ( $installer::globals::adafilename ne "" )
        {
            $sourcefile = $sopackpath . $installer::globals::separator . $installer::globals::compiler . $installer::globals::separator . "adabas" . $installer::globals::separator . $installer::globals::adafilename;
            if ( ! -f $sourcefile ) { installer::exiter::exit_program("ERROR: Ada file not found: $sourcefile !", "put_childprojects_into_installset"); }
            installer::systemactions::copy_one_file($sourcefile, $destdir);
        }
    }

    # Adding additional required packages (freetype).
    # This package names are stored in global array @installer::globals::requiredpackages

    if ( $allvariables->{'ADDREQUIREDPACKAGES'} )
    {
        for ( my $i = 0; $i <= $#installer::globals::requiredpackages; $i++ )
        {
            $sourcefile = $sopackpath . $installer::globals::separator . $installer::globals::compiler . $installer::globals::separator . "requiredpackages" . $installer::globals::separator . $installer::globals::requiredpackages[$i];
            if ( ! -f $sourcefile ) { installer::exiter::exit_program("ERROR: Required package file not found: $sourcefile !", "put_childprojects_into_installset"); }
            installer::systemactions::copy_one_file($sourcefile, $destdir);
        }
    }

    # unpacking and removing the ada tar.gz file

    if ( $installer::globals::issolarispkgbuild )
    {
        # determining the tar.gz files in directory $destdir
        my $fileextension = "gz";
        my $targzfiles = installer::systemactions::find_file_with_file_extension($fileextension, $destdir);

        for ( my $i = 0; $i <= $#{$targzfiles}; $i++ )
        {
            # unpacking
            my $systemcall = "cd $destdir; cat ${$targzfiles}[$i] | gunzip | tar -xf -";
            make_systemcall($systemcall);

            # deleting the tar.gz files
            $systemcall = "cd $destdir; rm -f ${$targzfiles}[$i]";
            make_systemcall($systemcall);
        }
    }

}

######################################################
# Including the system integration files into the
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
    my $infoline = "";

    # Attention: OOO has other names !

    if ( $installer::globals::issolarispkgbuild )
    {
        if ($installer::globals::product =~ /OpenOffice/i )
        {
            push(@systemfiles, "openofficeorg-desktop-integratn.tar.gz");
        }
        else
        {
            my $productname = $variables->{'UNIXPRODUCTNAME'};
            push(@systemfiles, "SUNW$productname-desktop-integratn.tar.gz");
            push(@systemfiles, "SUNW$productname-desktop-int-root.tar.gz");
            push(@systemfiles, "SUNW$productname-shared-mime-info.tar.gz");
        }
    }

    if ( $installer::globals::islinuxrpmbuild )
    {
        my $productversion = $variables->{'PRODUCTVERSION'};

        if ($installer::globals::product =~ /OpenOffice/i && $ENV{'PKGFORMAT'} eq "rpm")
        {
            push(@systemfiles, "openofficeorg-redhat-menus-$productversion-1.noarch.rpm");
            push(@systemfiles, "openofficeorg-suse-menus-$productversion-1.noarch.rpm");
            push(@systemfiles, "openofficeorg-mandriva-menus-$productversion-1.noarch.rpm");
            push(@systemfiles, "openofficeorg-freedesktop-menus-$productversion-1.noarch.rpm");

            # i46530 create desktop-integration subdirectory
            $destdir = "$destdir/desktop-integration";
            mkdir $destdir,0777;
        }
        else
        {
            my $productname = $variables->{'UNIXPRODUCTNAME'};
            push(@systemfiles, "$productname-desktop-integration-$installer::globals::packageversion-$installer::globals::packagerevision.noarch.rpm");
            push(@systemfiles, "$productname-suse-menus-$installer::globals::packageversion-$installer::globals::packagerevision.noarch.rpm");
        }
    }

    my $deb = 'openofficeorg-debian-menus_' . $variables->{'PRODUCTVERSION'} . '-1_all.deb';
    my $debref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$deb, $includepatharrayref, 0);

    if ( $$debref ne "" )
    {
         push(@systemfiles, $deb);
    }

    for ( my $i = 0; $i <= $#systemfiles; $i++ )
    {
        my $onefilename = $systemfiles[$i];
        my $sourcepathref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$onefilename, $includepatharrayref, 1);

        if ( $$sourcepathref eq "" )
        {
            $infoline = "ERROR: Did not find file for system integration: $onefilename\n";
            push( @installer::globals::logfileinfo, $infoline);
            next;
        }

        my $destfile = $destdir . $installer::globals::separator . $systemfiles[$i];
        installer::systemactions::copy_one_file($$sourcepathref, $destfile);

        # unpacking and deleting the tar.gz files for Solaris

        if ( $installer::globals::issolarispkgbuild )
        {
            # unpacking

            my $systemcall = "cd $destdir; cat $systemfiles[$i] | gunzip | tar -xf -";

            make_systemcall($systemcall);

            # compressing packages

            my $faspac = "faspac-so.sh";

            my $compressorref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$faspac, $includepatharrayref, 0);
            if ($$compressorref ne "")
            {
                $faspac = $$compressorref;
                $infoline = "Found compressor: $faspac\n";
                push( @installer::globals::logfileinfo, $infoline);

                installer::logger::print_message( "... $faspac ...\n" );
                installer::logger::include_timestamp_into_logfile("Starting $faspac");

                my $package = $systemfiles[$i];
                $package =~ s/\.tar\.gz$//;

                 $systemcall = "/bin/sh $faspac -a -q -d $destdir $package";     # $faspac has to be the absolute path!
                 make_systemcall($systemcall);

                installer::logger::include_timestamp_into_logfile("End of $faspac");
            }
            else
            {
                $infoline = "Not found: $faspac\n";
                push( @installer::globals::logfileinfo, $infoline);
            }

            # deleting the tar.gz files

            $systemcall = "cd $destdir; rm -f $systemfiles[$i]";

            make_systemcall($systemcall);
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

    ##############################################################
    # Version 1: "/opt" is variable and "openofficeorg20" fixed
    ##############################################################

    # my $staticpath = $rootpath;
    # installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$staticpath);
    # $$staticpathref = $staticpath;                # will be "openofficeorg20"

    # my $relocatablepath = $rootpath;
    # installer::pathanalyzer::get_path_from_fullqualifiedname(\$relocatablepath);
    # $$relocatablepathref = $relocatablepath;      # will be "/opt/"

    ##############################################################
    # Version 2: "/opt/openofficeorg20" is variable and "" fixed
    ##############################################################

    my $staticpath = "";
    $$staticpathref = $staticpath;              # will be ""

    my $relocatablepath = $rootpath . "\/";
    $$relocatablepathref = $relocatablepath;    # will be "/opt/openofficeorg20/"
}

######################################################
# Including license and readme into
# Unix installation sets.
######################################################

sub put_installsetfiles_into_installset
{
    my ($destdir) = @_;

    # All files for the installation set are saved in the global
    # array @installer::globals::installsetfiles

    for ( my $i = 0; $i <= $#installer::globals::installsetfiles; $i++ )
    {
        my $onefile = $installer::globals::installsetfiles[$i];
        my $sourcefile = $onefile->{'sourcepath'};
        my $destfile = "";
        if ( $installer::globals::addjavainstaller ) { $destfile = $onefile->{'Name'}; }
        else { $destfile = $destdir . $installer::globals::separator . $onefile->{'Name'}; }
        installer::systemactions::copy_one_file($sourcefile, $destfile);

        my $infoline = "Adding to installation set \"$destfile\" from source \"$sourcefile\".\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

######################################################
# Replacing one variable in patchinfo file
######################################################

sub replace_one_variable_in_file
{
    my ( $file, $placeholder, $value ) = @_;

    for ( my $i = 0; $i <= $#{$file}; $i++ )
    {
        ${$file}[$i] =~ s/$placeholder/$value/g;
    }
}

######################################################
# Setting variables in the patchinfo file
######################################################

sub set_patchinfo
{
    my ( $patchinfofile, $patchid ) = @_;

    # Setting: PATCHIDPLACEHOLDER and ARCHITECTUREPLACEHOLDER

    replace_one_variable_in_file($patchinfofile, "PATCHIDPLACEHOLDER", $patchid);

    my $architecture = "";
    if ( $installer::globals::issolarissparcbuild ) { $architecture = "sparc"; }
    if ( $installer::globals::issolarisx86build ) { $architecture = "x86"; }

    replace_one_variable_in_file($patchinfofile, "ARCHITECTUREPLACEHOLDER", $architecture);
}

######################################################
# Finalizing patch: Renaming directory and
# including additional patch files.
######################################################

sub finalize_patch
{
    my ( $newepmdir, $allvariables ) = @_;

    my $patchidname = "SOLSPARCPATCHID";
    if ( $installer::globals::issolarisx86build ) { $patchidname = "SOLIAPATCHID"; }

    if ( ! $allvariables->{$patchidname} ) { installer::exiter::exit_program("ERROR: Variable $patchidname not defined in zip list file!", "finalize_patch"); }
    my $patchid = $allvariables->{$patchidname};
    installer::systemactions::rename_directory($newepmdir, $patchid);

    # Copying all typical patch files into the patch directory
    # All patch file names are stored in @installer::globals::solarispatchfiles
    # Location of the file is $installer::globals::patchincludepath

    my $sourcepath = $installer::globals::patchincludepath;
    $sourcepath =~ s/\/\s*$//;

    for ( my $i = 0; $i <= $#installer::globals::solarispatchfiles; $i++ )
    {
        my $sourcefile = $sourcepath . $installer::globals::separator . $installer::globals::solarispatchfiles[$i];
        my $destfile = $patchid . $installer::globals::separator . $installer::globals::solarispatchfiles[$i];
        installer::systemactions::copy_one_file($sourcefile, $destfile);
    }

    # And editing the patchinfo file

    my $patchinfofilename = $patchid . $installer::globals::separator . "patchinfo";
    my $patchinfofile = installer::files::read_file($patchinfofilename);
    set_patchinfo($patchinfofile, $patchid);
    installer::files::save_file($patchinfofilename, $patchinfofile);
}

######################################################
# Finalizing Linux patch: Renaming directory and
# including additional patch files.
######################################################

sub finalize_linux_patch
{
    my ( $newepmdir, $allvariables, $includepatharrayref ) = @_;

    # Copying the setup into the patch directory
    # and including the list of RPMs into it

    print "... creating patch setup ...\n";

    installer::logger::include_header_into_logfile("Creating Linux patch setup:");

    # find and read setup script template

    my $scriptfilename = "linuxpatchscript.sh";
    my $scriptref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$scriptfilename, $includepatharrayref, 0);
    if ($$scriptref eq "") { installer::exiter::exit_program("ERROR: Could not find patch script template $scriptfilename!", "finalize_linux_patch"); }
    my $scriptfile = installer::files::read_file($$scriptref);

    my $infoline = "Found  script file $scriptfilename: $$scriptref \n";
    push( @installer::globals::logfileinfo, $infoline);

#   # Collecting all RPMs in the patch directory
#
#   my $fileextension = "rpm";
#   my $rpmfiles = installer::systemactions::find_file_with_file_extension($fileextension, $newepmdir);
#   if ( ! ( $#{$rpmfiles} > -1 )) { installer::exiter::exit_program("ERROR: Could not find rpm in directory $newepmdir!", "finalize_linux_patch"); }
#   for ( my $i = 0; $i <= $#{$rpmfiles}; $i++ ) { installer::pathanalyzer::make_absolute_filename_to_relative_filename(\${$rpmfiles}[$i]); }
#
#   my $installline = "";
#
#   for ( my $i = 0; $i <= $#{$rpmfiles}; $i++ )
#   {
#       $installline = $installline . "  rpm --prefix \$PRODUCTINSTALLLOCATION -U $newepmdir/${$rpmfiles}[$i]\n";
#   }
#
#   $installline =~ s/\s*$//;
#
#   for ( my $j = 0; $j <= $#{$scriptfile}; $j++ )
#   {
#       ${$scriptfile}[$j] =~ s/INSTALLLINES/$installline/;
#   }

    # Replacing the productname

    my $productname = $allvariables->{'PRODUCTNAME'};
    $productname = lc($productname);
    $productname =~ s/\.//g;    # openoffice.org -> openofficeorg

    $infoline = "Adding productname $productname into Linux patch script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $j = 0; $j <= $#{$scriptfile}; $j++ ) { ${$scriptfile}[$j] =~ s/PRODUCTNAMEPLACEHOLDER/$productname/; }

    # Saving the file

    my $newscriptfilename = "setup"; # $newepmdir . $installer::globals::separator . "setup";
    installer::files::save_file($newscriptfilename, $scriptfile);

    $infoline = "Saved Linux patch setup $newscriptfilename \n";
    push( @installer::globals::logfileinfo, $infoline);

    # Setting unix rights 755
    my $localcall = "chmod 775 $newscriptfilename \>\/dev\/null 2\>\&1";
    system($localcall);
}

1;
