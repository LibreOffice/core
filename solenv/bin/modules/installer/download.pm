#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: download.pm,v $
#
#   $Revision: 1.37 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 18:39:05 $
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

package installer::download;

use File::Spec;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::remover;
use installer::systemactions;

BEGIN { # This is needed so that cygwin's perl evaluates ACLs
    # (needed for correctly evaluating the -x test.)
    if( $^O =~ /cygwin/i ) {
        require filetest; import filetest "access";
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
    $productname =~ s/\s*//g;

    my $infoline = "Adding productname $productname into download shell script\n";
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

    my $infoline = "Adding linenumber $linenumber into download shell script\n";
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

    $installer::globals::downloadfileextension = ".sh";
    $filename = $filename . $installer::globals::downloadfileextension;
    $installer::globals::downloadfilename = $filename;

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
# Including checksum and size into script file
#########################################################

sub put_checksum_and_size_into_script
{
    my ($scriptfile, $sumout) = @_;

    my $checksum = "";
    my $size = "";

    if  ( $sumout =~ /^\s*(\d+)\s+(\d+)\s*$/ )
    {
        $checksum = $1;
        $size = $2;
    }
    else
    {
        installer::exiter::exit_program("ERROR: Incorrect return value from /usr/bin/sum: $sumout", "put_checksum_and_size_into_script");
    }

    my $infoline = "Adding checksum $checksum and size $size into download shell script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/CHECKSUMPLACEHOLDER/$checksum/;
        ${$scriptfile}[$i] =~ s/DISCSPACEPLACEHOLDER/$size/;
    }

}

#########################################################
# Calling md5sum
#########################################################

sub call_md5sum
{
    my ($filename) = @_;

    $md5sumfile = "/usr/bin/md5sum";

    if ( ! -f $md5sumfile ) { installer::exiter::exit_program("ERROR: No file /usr/bin/md5sum", "call_md5sum"); }

    my $systemcall = "$md5sumfile $filename |";

    my $md5sumoutput = "";

    open (SUM, "$systemcall");
    $md5sumoutput = <SUM>;
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

    return $md5sumoutput;
}

#########################################################
# Calling md5sum
#########################################################

sub get_md5sum
{
    ($md5sumoutput) = @_;

    my $md5sum;

    if  ( $md5sumoutput =~ /^\s*(\w+?)\s+/ )
    {
        $md5sum = $1;
    }
    else
    {
        installer::exiter::exit_program("ERROR: Incorrect return value from /usr/bin/md5sum: $md5sumoutput", "get_md5sum");
    }

    my $infoline = "Setting md5sum: $md5sum\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $md5sum;
}

#########################################################
# Determining checksum and size of tar file
#########################################################

sub call_sum
{
    my ($installdir, $getuidlibrary) = @_;

#   my $ownerstring = "";
#   if ( $installer::globals::islinuxrpmbuild ) { $ownerstring = "--owner=0"; }
#   my $systemcall = "cd $installdir; tar $ownerstring -cf - * | /usr/bin/sum |";

    my $ldpreloadstring = "";
    if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }

    my $systemcall = "cd $installdir; $ldpreloadstring tar -cf - * | /usr/bin/sum |";

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
# Searching for the getuid.so in the solver
#########################################################

sub get_path_for_library
{
    my ($includepatharrayref) = @_;

    my $getuidlibraryname = "getuid.so";
    my $getuidlibraryref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$getuidlibraryname, $includepatharrayref, 0);
    if ($$getuidlibraryref eq "") { installer::exiter::exit_program("ERROR: Could not find $getuidlibraryname!", "get_path_for_library"); }

    return $$getuidlibraryref;
}

#########################################################
# Including the binary package into the script
#########################################################

sub include_package_into_script
{
    my ( $installdir, $scriptfilename, $getuidlibrary) = @_;

#   my $ownerstring = "";
#   if ( $installer::globals::islinuxrpmbuild ) { $ownerstring = "--owner=0"; }
#   my $systemcall = "cd $installdir; tar $ownerstring -cf - * >> $scriptfilename";

    my $ldpreloadstring = "";
    if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }

    my $systemcall = "cd $installdir; $ldpreloadstring tar -cf - * >> $scriptfilename";

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
# Creating a tar.gz file
#########################################################

sub create_tar_gz_file_from_package
{
    my ($installdir, $getuidlibrary) = @_;

    my $infoline = "";
    my $alldirs = installer::systemactions::get_all_directories($installdir);
    my $onedir = ${$alldirs}[0];
    $installdir = $onedir;

    my $allfiles = installer::systemactions::get_all_files_from_one_directory($installdir);

    for ( my $i = 0; $i <= $#{$allfiles}; $i++ )
    {
        my $onefile = ${$allfiles}[$i];
        my $systemcall = "cd $installdir; rm $onefile";
        my $returnvalue = system($systemcall);

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

    $alldirs = installer::systemactions::get_all_directories($installdir);
    $packagename = ${$alldirs}[0]; # only taking the first Solaris package
    if ( $packagename eq "" ) { installer::exiter::exit_program("ERROR: Could not find package in directory $installdir!", "determine_packagename"); }

    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$packagename);

    $installer::globals::downloadfileextension = ".tar.gz";
    my $targzname = $packagename . $installer::globals::downloadfileextension;
    $installer::globals::downloadfilename = $targzname;
    my $ldpreloadstring = "";
    if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }

    $systemcall = "cd $installdir; $ldpreloadstring tar -cf - $packagename | gzip > $targzname";
    print "... $systemcall ...\n";

    my $returnvalue = system($systemcall);

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

#########################################################
# Setting type of installation
#########################################################

sub get_installation_type
{
    my $type = "";

    if ( $installer::globals::languagepack ) { $type = "langpack"; }
    else { $type = "install"; }

    return $type;
}

#########################################################
# Setting installation languages
#########################################################

sub get_downloadname_language
{
    my ($languagestringref) = @_;

    my $languages = $$languagestringref;

    if ( $installer::globals::added_english )
    {
        $languages =~ s/en-US_//;
        $languages =~ s/_en-US//;
    }

    # en-US is default language and can be removed therefore
    # for one-language installation sets

    if ( $languages =~ /^\s*en-US\s*$/ )
    {
        $languages = "";
    }


    if ( length ($languages) > $installer::globals::max_lang_length )
    {
        $languages = 'multi';
    }

    return $languages;
}

#########################################################
# Setting download name, first part
#########################################################

sub get_downloadname_start
{
    my ($allvariables) = @_;

    my $start = "OOo";
    if ( $allvariables->{'PRODUCTNAME'} eq "BrOffice.org" ) { $start = "BrOo"; }

    return $start;
}

#########################################################
# Setting installation addons
#########################################################

sub get_downloadname_addon
{
    my $addon = "";

    if ( $installer::globals::product =~ /_wJRE\s*$/ ) { $addon = "_wJRE"; }

    return $addon;
}

#########################################################
# Looking for versionstring in version.info
# This has to be the only content of this file.
#########################################################

sub get_versionstring
{
    my ( $versionfile ) = @_;

    my $versionstring = "";

    for ( my $i = 0; $i <= $#{$versionfile}; $i++ )
    {
        my $oneline = ${$versionfile}[$i];

        if ( $oneline =~ /^\s*\#/ ) { next; } # comment line
        if ( $oneline =~ /^\s*\"\s*(.*?)\s*\"\s*$/ )
        {
            $versionstring = $1;
            last;
        }
    }

    return $versionstring;
}

#########################################################
# Returning the current product version
# This has to be defined in file "version.info"
# in directory $installer::globals::ooouploaddir
#########################################################

sub get_current_version
{
    my $infoline = "";
    my $versionstring = "";
    my $filename = "version.info";
    # $filename = $installer::globals::ooouploaddir . $installer::globals::separator . $filename;

    if ( -f $filename )
    {
        $infoline = "File $filename exists. Trying to find current version.\n";
        push( @installer::globals::logfileinfo, $infoline);
        my $versionfile = installer::files::read_file($filename);
        $versionstring = get_versionstring($versionfile);
        $infoline = "Setting version string: $versionstring\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "File $filename does not exist. No version setting in download file name.\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    $installer::globals::oooversionstring = $versionstring;

    return $versionstring;
}

#########################################################
# Determining the download file name
# Samples:
# OOo_2.0.2rc1_060213_Solarisx86_install_de
# OOo_2.0.2rc1_060213_LinuxIntel_langpack_zh-TW
# OOo_2.0.2rc1_060213_SolarisSparc_install_zh-TW_wJRE
# OOo_2.0.2rc1_060213_Win32Intel_install_zh-TW_wJRE
# OOo_2.0.157_LinuxIntel_install_de
#
#########################################################

sub set_download_filename
{
    my ($languagestringref, $allvariables) = @_;

    my $start = get_downloadname_start($allvariables);
    # my $versionstring = get_current_version();
    my $versionstring = "";
    my $date = installer::logger::set_installation_date();
    if ( $installer::globals::product =~ /_Dev\s*$/ ) { $date = ""; }
    my $platform = installer::worker::get_platform_name();
    my $type = get_installation_type();
    my $language = get_downloadname_language($languagestringref);
    my $addon = get_downloadname_addon();
    if ( $installer::globals::product =~ /_Dev\s*$/ )
    {
        my $localminor = "";
        if ( $installer::globals::minor ne "" ) { $localminor = $installer::globals::minor; }
        else { $localminor = $installer::globals::lastminor; }
        if ( $localminor =~ /^\s*\w(\d+)\w*\s*$/ ) { $localminor = $1; }
        $versionstring = $allvariables->{'PRODUCTVERSION'} . "." . $localminor;
    }
    else
    {
        if ( $allvariables->{'PACKAGEVERSION'} )
        {
            $versionstring = $allvariables->{'PACKAGEVERSION'};
        }
    }

    my $filename = $start . "_" . $versionstring . "_" . $date . "_" . $platform . "_" . $type . "_" . $language . $addon;

    $filename =~ s/\_\_/\_/g;   # necessary, if $versionstring or $platform or $language are empty
    $filename =~ s/\_\s*$//;    # necessary, if $language and $addon are empty

    $installer::globals::ooodownloadfilename = $filename;

    return $filename;
}

#########################################################
# Creating a tar.gz file
#########################################################

sub create_tar_gz_file_from_directory
{
    my ($installdir, $getuidlibrary, $downloaddir, $downloadfilename) = @_;

    my $infoline = "";

    my $packdir = $installdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$packdir);
    my $changedir = $installdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$changedir);

    my $ldpreloadstring = "";
    if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }

    $installer::globals::downloadfileextension = ".tar.gz";
    $installer::globals::downloadfilename = $downloadfilename . $installer::globals::downloadfileextension;
    my $targzname = $downloaddir . $installer::globals::separator . $installer::globals::downloadfilename;

    $systemcall = "cd $changedir; $ldpreloadstring tar -cf - $packdir | gzip > $targzname";

    my $returnvalue = system($systemcall);

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

    return $targzname;
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

##################################################################
# Windows: Replacing one placeholder with the specified value
##################################################################

sub replace_one_variable
{
    my ($templatefile, $placeholder, $value) = @_;

    my $infoline = "Replacing $placeholder by $value in nsi file\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$templatefile}; $i++ )
    {
        ${$templatefile}[$i] =~ s/$placeholder/$value/g;
    }

}

##################################################################
# Windows: Setting nsis version is necessary because of small
# changes in nsis from version 2.0.4 to 2.3.1
##################################################################

sub set_nsis_version
{
    my ($nshfile) = @_;

    my $searchstring = "\$\{LangFileString\}"; # occurs only in nsis 2.3.1 or similar

    for ( my $i = 0; $i <= $#{$nshfile}; $i++ )
    {
        if ( ${$nshfile}[$i] =~ /\Q$searchstring\E/ )
        {
            # this is nsis 2.3.1 or similar
            $installer::globals::nsis231 = 1;
            last;
        }
    }

    if ( ! $installer::globals::nsis231 ) { $installer::globals::nsis204 = 1; }
}

##################################################################
# Windows: Including the product name into nsi template
##################################################################

sub put_windows_productname_into_template
{
    my ($templatefile, $variableshashref) = @_;

    my $productname = $variableshashref->{'PRODUCTNAME'};
    $productname =~ s/\.//g;    # OpenOffice.org -> OpenOfficeorg

    replace_one_variable($templatefile, "PRODUCTNAMEPLACEHOLDER", $productname);
}

##################################################################
# Windows: Including the path to the banner.bmp into nsi template
##################################################################

sub put_banner_bmp_into_template
{
    my ($templatefile, $includepatharrayref, $allvariables) = @_;

    # my $filename = "downloadbanner.bmp";
    if ( ! $allvariables->{'DOWNLOADBANNER'} ) { installer::exiter::exit_program("ERROR: DOWNLOADBANNER not defined in product definition!", "put_banner_bmp_into_template"); }
    my $filename = $allvariables->{'DOWNLOADBANNER'};

    my $completefilenameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 0);

    if ( $^O =~ /cygwin/i ) { $$completefilenameref =~ s/\//\\/g; }

    if ($$completefilenameref eq "") { installer::exiter::exit_program("ERROR: Could not find download file $filename!", "put_banner_bmp_into_template"); }

    replace_one_variable($templatefile, "BANNERBMPPLACEHOLDER", $$completefilenameref);
}

##################################################################
# Windows: Including the path to the welcome.bmp into nsi template
##################################################################

sub put_welcome_bmp_into_template
{
    my ($templatefile, $includepatharrayref, $allvariables) = @_;

    # my $filename = "downloadbitmap.bmp";
    if ( ! $allvariables->{'DOWNLOADBITMAP'} ) { installer::exiter::exit_program("ERROR: DOWNLOADBITMAP not defined in product definition!", "put_welcome_bmp_into_template"); }
    my $filename = $allvariables->{'DOWNLOADBITMAP'};

    my $completefilenameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 0);

    if ( $^O =~ /cygwin/i ) { $$completefilenameref =~ s/\//\\/g; }

    if ($$completefilenameref eq "") { installer::exiter::exit_program("ERROR: Could not find download file $filename!", "put_welcome_bmp_into_template"); }

    replace_one_variable($templatefile, "WELCOMEBMPPLACEHOLDER", $$completefilenameref);
}

##################################################################
# Windows: Including the path to the setup.ico into nsi template
##################################################################

sub put_setup_ico_into_template
{
    my ($templatefile, $includepatharrayref, $allvariables) = @_;

    # my $filename = "downloadsetup.ico";
    if ( ! $allvariables->{'DOWNLOADSETUPICO'} ) { installer::exiter::exit_program("ERROR: DOWNLOADSETUPICO not defined in product definition!", "put_setup_ico_into_template"); }
    my $filename = $allvariables->{'DOWNLOADSETUPICO'};

    my $completefilenameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 0);

    if ( $^O =~ /cygwin/i ) { $$completefilenameref =~ s/\//\\/g; }

    if ($$completefilenameref eq "") { installer::exiter::exit_program("ERROR: Could not find download file $filename!", "put_setup_ico_into_template"); }

    replace_one_variable($templatefile, "SETUPICOPLACEHOLDER", $$completefilenameref);
}

##################################################################
# Windows: Including the publisher into nsi template
##################################################################

sub put_publisher_into_template
{
    my ($templatefile) = @_;

    my $publisher = "Sun Microsystems, Inc.";

    replace_one_variable($templatefile, "PUBLISHERPLACEHOLDER", $publisher);
}

##################################################################
# Windows: Including the web site into nsi template
##################################################################

sub put_website_into_template
{
    my ($templatefile) = @_;

    my $website = "http\:\/\/www\.sun\.com\/staroffice";

    replace_one_variable($templatefile, "WEBSITEPLACEHOLDER", $website);
}

##################################################################
# Windows: Including the Java file name into nsi template
##################################################################

sub put_javafilename_into_template
{
    my ($templatefile, $variableshashref) = @_;

    my $javaversion = "";

    if ( $variableshashref->{'WINDOWSJAVAFILENAME'} ) { $javaversion = $variableshashref->{'WINDOWSJAVAFILENAME'}; }

    replace_one_variable($templatefile, "WINDOWSJAVAFILENAMEPLACEHOLDER", $javaversion);
}

##################################################################
# Windows: Including the product version into nsi template
##################################################################

sub put_windows_productversion_into_template
{
    my ($templatefile, $variableshashref) = @_;

    my $productversion = $variableshashref->{'PRODUCTVERSION'};

    replace_one_variable($templatefile, "PRODUCTVERSIONPLACEHOLDER", $productversion);
}

##################################################################
# Windows: Including the product version into nsi template
##################################################################

sub put_windows_productpath_into_template
{
    my ($templatefile, $variableshashref, $languagestringref) = @_;

    my $productpath = $variableshashref->{'PROPERTYTABLEPRODUCTNAME'};

    my $locallangs = $$languagestringref;
    $locallangs =~ s/_/ /g;
    if ( ! $installer::globals::languagepack ) { $productpath = $productpath . " (" . $locallangs . ")"; }

    replace_one_variable($templatefile, "PRODUCTPATHPLACEHOLDER", $productpath);
}

##################################################################
# Windows: Including download file name into nsi template
##################################################################

sub put_outputfilename_into_template
{
    my ($templatefile, $downloadname) = @_;

    $installer::globals::downloadfileextension = ".exe";
    $downloadname = $downloadname . $installer::globals::downloadfileextension;
    $installer::globals::downloadfilename = $downloadname;

    replace_one_variable($templatefile, "DOWNLOADNAMEPLACEHOLDER", $downloadname);
}

##################################################################
# Windows: Generating the file list in nsi file format
##################################################################

sub get_file_list
{
    my ( $basedir ) = @_;

    my @filelist = ();

    my $alldirs = installer::systemactions::get_all_directories($basedir);
    unshift(@{$alldirs}, $basedir); # $basedir is the first directory in $alldirs

    for ( my $i = 0; $i <= $#{$alldirs}; $i++ )
    {
        my $onedir = ${$alldirs}[$i];

        # Syntax:
        # SetOutPath "$INSTDIR"

        my $relativedir = $onedir;
        $relativedir =~ s/\Q$basedir\E//;

        my $oneline = " " . "SetOutPath" . " " . "\"\$INSTDIR" . $relativedir . "\"" . "\n";

        if ( $^O =~ /cygwin/i ) {
            $oneline =~ s/\//\\/g;
        }
        push(@filelist, $oneline);

        # Collecting all files in the specific directory

        my $files = installer::systemactions::get_all_files_from_one_directory($onedir);

        for ( my $j = 0; $j <= $#{$files}; $j++ )
        {
            my $onefile = ${$files}[$j];

            my $fileline = "  " . "File" . " " . "\"" . $onefile . "\"" . "\n";
            if ( $^O =~ /cygwin/i ) {
                $fileline =~ s/\//\\/g;
            }
            push(@filelist, $fileline);
        }
    }

    return \@filelist;
}

##################################################################
# Windows: Including list of all files into nsi template
##################################################################

sub put_filelist_into_template
{
    my ($templatefile, $installationdir) = @_;

    my $filelist = get_file_list($installationdir);

    my $filestring = "";

    for ( my $i = 0; $i <= $#{$filelist}; $i++ )
    {
        $filestring = $filestring . ${$filelist}[$i];
    }

    $filestring =~ s/\s*$//;

    replace_one_variable($templatefile, "ALLFILESPLACEHOLDER", $filestring);
}

##################################################################
# Windows: NSIS uses specific language names
##################################################################

sub nsis_language_converter
{
    my ($language) = @_;

    my $nsislanguage = "";

    if ( $language eq "en-US" ) { $nsislanguage = "English"; }
    elsif ( $language eq "sq" ) { $nsislanguage = "Albanian"; }
    elsif ( $language eq "ar" ) { $nsislanguage = "Arabic"; }
    elsif ( $language eq "bg" ) { $nsislanguage = "Bulgarian"; }
    elsif ( $language eq "ca" ) { $nsislanguage = "Catalan"; }
    elsif ( $language eq "hr" ) { $nsislanguage = "Croatian"; }
    elsif ( $language eq "cs" ) { $nsislanguage = "Czech"; }
    elsif ( $language eq "da" ) { $nsislanguage = "Danish"; }
    elsif ( $language eq "nl" ) { $nsislanguage = "Dutch"; }
    elsif ( $language eq "de" ) { $nsislanguage = "German"; }
    elsif ( $language eq "de-LU" ) { $nsislanguage = "Luxembourgish"; }
    elsif ( $language eq "et" ) { $nsislanguage = "Estonian"; }
    elsif ( $language eq "fa" ) { $nsislanguage = "Farsi"; }
    elsif ( $language eq "el" ) { $nsislanguage = "Greek"; }
    elsif ( $language eq "fi" ) { $nsislanguage = "Finnish"; }
    elsif ( $language eq "fr" ) { $nsislanguage = "French"; }
    elsif ( $language eq "hu" ) { $nsislanguage = "Hungarian"; }
    elsif ( $language eq "he" ) { $nsislanguage = "Hebrew"; }
    elsif ( $language eq "id" ) { $nsislanguage = "Indonesian"; }
    elsif ( $language eq "it" ) { $nsislanguage = "Italian"; }
    elsif ( $language eq "lv" ) { $nsislanguage = "Latvian"; }
    elsif ( $language eq "lt" ) { $nsislanguage = "Lithuanian"; }
    elsif ( $language eq "mk" ) { $nsislanguage = "Macedonian"; }
    elsif ( $language eq "no" ) { $nsislanguage = "Norwegian"; }
    elsif ( $language eq "no-NO" ) { $nsislanguage = "Norwegian"; }
    elsif ( $language eq "es" ) { $nsislanguage = "Spanish"; }
    elsif ( $language eq "sl" ) { $nsislanguage = "Slovenian"; }
    elsif ( $language eq "sv" ) { $nsislanguage = "Swedish"; }
    elsif ( $language eq "sk" ) { $nsislanguage = "Slovak"; }
    elsif ( $language eq "pl" ) { $nsislanguage = "Polish"; }
    elsif ( $language eq "pt-BR" ) { $nsislanguage = "PortugueseBR"; }
    elsif ( $language eq "pt" ) { $nsislanguage = "Portuguese"; }
    elsif ( $language eq "ro" ) { $nsislanguage = "Romanian"; }
    elsif ( $language eq "ru" ) { $nsislanguage = "Russian"; }
    elsif ( $language eq "sr" ) { $nsislanguage = "Serbian"; }
    elsif ( $language eq "sr-SP" ) { $nsislanguage = "Serbian"; }
    elsif ( $language eq "uk" ) { $nsislanguage = "Ukrainian"; }
    elsif ( $language eq "tr" ) { $nsislanguage = "Turkish"; }
    elsif ( $language eq "ja" ) { $nsislanguage = "Japanese"; }
    elsif ( $language eq "ko" ) { $nsislanguage = "Korean"; }
    elsif ( $language eq "th" ) { $nsislanguage = "Thai"; }
    elsif ( $language eq "zh-CN" ) { $nsislanguage = "SimpChinese"; }
    elsif ( $language eq "zh-TW" ) { $nsislanguage = "TradChinese"; }
    else {

        my $infoline = "NSIS language_converter : Could not find nsis language for $language!\n";
        push( @installer::globals::logfileinfo, $infoline);
        $nsislanguage = "English";
        # installer::exiter::exit_program("ERROR: Could not find nsis language for $language!", "nsis_language_converter");
    }

    return $nsislanguage;
}

##################################################################
# Windows: Including list of all languages into nsi template
##################################################################

sub put_language_list_into_template
{
    my ($templatefile, $languagesarrayref) = @_;

    my $alllangstring = "";
    my %nsislangs;

    for ( my $i = 0; $i <= $#{$languagesarrayref}; $i++ )
    {
        my $onelanguage = ${$languagesarrayref}[$i];
        my $nsislanguage = nsis_language_converter($onelanguage);
        $nsislangs{$nsislanguage}++;
    }

    foreach my $nsislanguage ( keys(%nsislangs) )
    {
        # Syntax: !insertmacro MUI_LANGUAGE "English"
        my $langstring = "\!insertmacro MUI_LANGUAGE_PACK " . $nsislanguage . "\n";
        $alllangstring = $alllangstring . $langstring;
    }

    $alllangstring =~ s/\s*$//;

    replace_one_variable($templatefile, "ALLLANGUAGESPLACEHOLDER", $alllangstring);
}

##################################################################
# Windows: Collecting all identifier from mlf file
##################################################################

sub get_identifier
{
    my ( $mlffile ) = @_;

    my @identifier = ();

    for ( my $i = 0; $i <= $#{$mlffile}; $i++ )
    {
        my $oneline = ${$mlffile}[$i];

        if ( $oneline =~ /^\s*\[(.+)\]\s*$/ )
        {
            my $identifier = $1;
            push(@identifier, $identifier);
        }
    }

    return \@identifier;
}

##############################################################
# Returning the complete block in all languages
# for a specified string
##############################################################

sub get_language_block_from_language_file
{
    my ($searchstring, $languagefile) = @_;

    my @language_block = ();

    for ( my $i = 0; $i <= $#{$languagefile}; $i++ )
    {
        if ( ${$languagefile}[$i] =~ /^\s*\[\s*$searchstring\s*\]\s*$/ )
        {
            my $counter = $i;

            push(@language_block, ${$languagefile}[$counter]);
            $counter++;

            while (( $counter <= $#{$languagefile} ) && (!( ${$languagefile}[$counter] =~ /^\s*\[/ )))
            {
                push(@language_block, ${$languagefile}[$counter]);
                $counter++;
            }

            last;
        }
    }

    return \@language_block;
}

##############################################################
# Returning a specific language string from the block
# of all translations
##############################################################

sub get_language_string_from_language_block
{
    my ($language_block, $language) = @_;

    my $newstring = "";

    for ( my $i = 0; $i <= $#{$language_block}; $i++ )
    {
        if ( ${$language_block}[$i] =~ /^\s*$language\s*\=\s*\"(.*)\"\s*$/ )
        {
            $newstring = $1;
            last;
        }
    }

    if ( $newstring eq "" )
    {
        $language = "en-US";    # defaulting to english

        for ( my $i = 0; $i <= $#{$language_block}; $i++ )
        {
            if ( ${$language_block}[$i] =~ /^\s*$language\s*\=\s*\"(.*)\"\s*$/ )
            {
                $newstring = $1;
                last;
            }
        }
    }

    return $newstring;
}

##################################################################
# Windows: Replacing strings in NSIS nsh file
# nsh file syntax:
# !define MUI_TEXT_DIRECTORY_TITLE "Zielverzeichnis auswählen"
##################################################################

sub replace_identifier_in_nshfile
{
    my ( $nshfile, $identifier, $newstring, $nshfilename, $onelanguage ) = @_;

    if ( $installer::globals::nsis231 )
    {
        $newstring =~ s/\\r/\$\\r/g;    # \r -> $\r  in modern nsis versions
        $newstring =~ s/\\n/\$\\n/g;    # \n -> $\n  in modern nsis versions
    }

    for ( my $i = 0; $i <= $#{$nshfile}; $i++ )
    {
        if ( ${$nshfile}[$i] =~ /\s+\Q$identifier\E\s+\"(.+)\"\s*$/ )
        {
            my $oldstring = $1;
            ${$nshfile}[$i] =~ s/\Q$oldstring\E/$newstring/;
            my $infoline = "NSIS replacement in $nshfilename ($onelanguage): $oldstring \-\> $newstring\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

##################################################################
# Windows: Replacing strings in NSIS nlf file
# nlf file syntax (2 lines):
# # ^DirSubText
# Zielverzeichnis
##################################################################

sub replace_identifier_in_nlffile
{
    my ( $nlffile, $identifier, $newstring, $nlffilename, $onelanguage ) = @_;

    for ( my $i = 0; $i <= $#{$nlffile}; $i++ )
    {
        if ( ${$nlffile}[$i] =~ /^\s*\#\s+\^\s*\Q$identifier\E\s*$/ )
        {
            my $next = $i+1;
            my $oldstring = ${$nlffile}[$next];
            ${$nlffile}[$next] = $newstring . "\n";
            $oldstring =~ s/\s*$//;
            my $infoline = "NSIS replacement in $nlffilename ($onelanguage): $oldstring \-\> $newstring\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

##################################################################
# Windows: Translating the NSIS nsh and nlf file
##################################################################

sub translate_nsh_nlf_file
{
    my ($nshfile, $nlffile, $mlffile, $onelanguage, $nshfilename, $nlffilename) = @_;

    # Analyzing the mlf file, collecting all Identifier
    my $allidentifier = get_identifier($mlffile);

    for ( my $i = 0; $i <= $#{$allidentifier}; $i++ )
    {
        my $identifier = ${$allidentifier}[$i];
        my $language_block = get_language_block_from_language_file($identifier, $mlffile);
        my $newstring = get_language_string_from_language_block($language_block, $onelanguage);

        # removing mask
        $newstring =~ s/\\\'/\'/g;

        replace_identifier_in_nshfile($nshfile, $identifier, $newstring, $nshfilename, $onelanguage);
        replace_identifier_in_nlffile($nlffile, $identifier, $newstring, $nlffilename, $onelanguage);
    }
}

##################################################################
# Windows: Copying NSIS language files to local nsis directory
##################################################################

sub copy_and_translate_nsis_language_files
{
    my ($nsispath, $localnsisdir, $languagesarrayref, $mlffile) = @_;

    my $nlffilepath = $nsispath . $installer::globals::separator . "Contrib" . $installer::globals::separator . "Language\ files" . $installer::globals::separator;
    my $nshfilepath = $nsispath . $installer::globals::separator . "Contrib" . $installer::globals::separator . "Modern\ UI" . $installer::globals::separator . "Language files" . $installer::globals::separator;

    for ( my $i = 0; $i <= $#{$languagesarrayref}; $i++ )
    {
        my $onelanguage = ${$languagesarrayref}[$i];
        my $nsislanguage = nsis_language_converter($onelanguage);

        # Copying the nlf file
        my $sourcepath = $nlffilepath . $nsislanguage . "\.nlf";
        if ( ! -f $sourcepath ) { installer::exiter::exit_program("ERROR: Could not find nsis file: $sourcepath!", "copy_and_translate_nsis_language_files"); }
        my $nlffilename = $localnsisdir . $installer::globals::separator . $nsislanguage . "_pack.nlf";
        if ( $^O =~ /cygwin/i ) { $nlffilename =~ s/\//\\/g; }
        installer::systemactions::copy_one_file($sourcepath, $nlffilename);

        # Copying the nsh file
        # In newer nsis versions, the nsh file is located next to the nlf file
        $sourcepath = $nshfilepath . $nsislanguage . "\.nsh";
        if ( ! -f $sourcepath )
        {
            # trying to find the nsh file next to the nlf file
            $sourcepath = $nlffilepath . $nsislanguage . "\.nsh";
            if ( ! -f $sourcepath )
            {
                installer::exiter::exit_program("ERROR: Could not find nsis file: $sourcepath!", "copy_and_translate_nsis_language_files");
            }
        }
        my $nshfilename = $localnsisdir . $installer::globals::separator . $nsislanguage . "_pack.nsh";
        if ( $^O =~ /cygwin/i ) { $nshfilename =~ s/\//\\/g; }
        installer::systemactions::copy_one_file($sourcepath, $nshfilename);

        # Changing the macro name in nsh file: MUI_LANGUAGEFILE_BEGIN -> MUI_LANGUAGEFILE_PACK_BEGIN
        my $nshfile = installer::files::read_file($nshfilename);
        replace_one_variable($nshfile, "MUI_LANGUAGEFILE_BEGIN", "MUI_LANGUAGEFILE_PACK_BEGIN");
        set_nsis_version($nshfile);

        # Translate the files
        my $nlffile = installer::files::read_file($nlffilename);
        translate_nsh_nlf_file($nshfile, $nlffile, $mlffile, $onelanguage, $nshfilename, $nlffilename);

        installer::files::save_file($nshfilename, $nshfile);
        installer::files::save_file($nlffilename, $nlffile);
    }

}

##################################################################
# Windows: Including the nsis path into the nsi template
##################################################################

sub put_nsis_path_into_template
{
    my ($templatefile, $nsisdir) = @_;

    replace_one_variable($templatefile, "NSISPATHPLACEHOLDER", $nsisdir);
}

##################################################################
# Windows: Including the output path into the nsi template
##################################################################

sub put_output_path_into_template
{
    my ($templatefile, $downloaddir) = @_;

    if ( $^O =~ /cygwin/i ) { $downloaddir =~ s/\//\\/g; }

    replace_one_variable($templatefile, "OUTPUTDIRPLACEHOLDER", $downloaddir);
}

##################################################################
# Windows: Only allow specific code for nsis 2.0.4 or nsis 2.3.1
##################################################################

sub put_version_specific_code_into_template
{
    my ($templatefile) = @_;

    my $subst204 = "";
    my $subst231 = "";

    if ( $installer::globals::nsis204 )
    {
        $subst231 = ";";
    }
    else
    {
        $subst204 = ";";
    }

    replace_one_variable($templatefile, "\#204\#", $subst204);
    replace_one_variable($templatefile, "\#231\#", $subst231);
}

##################################################################
# Windows: Finding the path to the nsis SDK
##################################################################

sub get_path_to_nsis_sdk
{
    my $vol;
    my $dir;
    my $file;
    my $nsispath = "";

    if ( $ENV{'NSIS_PATH'} ) {
        $nsispath = $ENV{'NSIS_PATH'};
    } elsif ( $ENV{'SOLARROOT'} ) {
        $nsispath = $ENV{'SOLARROOT'} . $installer::globals::separator . "NSIS";
    } else {
        # do we have nsis already in path ?
        @paths = split(/:/, $ENV{'PATH'});
        foreach $paths (@paths) {
            $path =~ s/[\/\\]+$//; # remove trailing slashes;
            $nsispath = $paths . "/nsis";

            if ( -x $nsispath ) {
                $nsispath = $paths;
                last;
            }
            else {
                $nsispath = "";
            }
        }
    }
    if ( $ENV{'NSISSDK_SOURCE'} ) {
        installer::logger::print_warning( "NSISSDK_SOURCE is deprecated. use NSIS_PATH instead.\n" );
        $nsispath = $ENV{'NSISSDK_SOURCE'}; # overriding the NSIS SDK with NSISSDK_SOURCE
    }

    if( ($^O =~ /cygwin/i) and $nsispath =~ /\\/ ) {
        # We need a POSIX path for W32-4nt-cygwin-perl
        $nsispath =~ s/\\/\\\\/g;
        chomp( $nsispath = qx{cygpath -u "$nsispath"} );
    }

    if ( $nsispath eq "" )
    {
        installer::logger::print_message( "... no Environment variable \"SOLARROOT\", \"NSIS_PATH\" or \"NSISSDK_SOURCE\" found and NSIS not found in path!", "get_path_to_nsis_sdk");
    } elsif ( ! -d $nsispath )
    {
        installer::exiter::exit_program("ERROR: NSIS path $nsispath does not exist!", "get_path_to_nsis_sdk");
    }

    return $nsispath;
}

##################################################################
# Windows: Executing NSIS to create the installation set
##################################################################

sub call_nsis
{
    my ( $nsispath, $nsifile ) = @_;

    my $makensisexe = $nsispath . $installer::globals::separator . "makensis.exe";

    installer::logger::print_message( "... starting $makensisexe ... \n" );

    my $systemcall = "$makensisexe $nsifile |";

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    my @nsisoutput = ();

    open (NSI, "$systemcall");
    while (<NSI>) {push(@nsisoutput, $_); }
    close (NSI);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    if ($returnvalue)
    {
        $infoline = "ERROR: $systemcall !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    for ( my $i = 0; $i <= $#nsisoutput; $i++ ) { push( @installer::globals::logfileinfo, "$nsisoutput[$i]"); }

}

#################################################################################
# Replacing one variable in one files
#################################################################################

sub replace_one_variable_in_translationfile
{
    my ($translationfile, $variable, $searchstring) = @_;

    for ( my $i = 0; $i <= $#{$translationfile}; $i++ )
    {
        ${$translationfile}[$i] =~ s/\%$searchstring/$variable/g;
    }
}

#################################################################################
# Replacing the variables in the translation file
#################################################################################

sub replace_variables
{
    my ($translationfile, $variableshashref) = @_;

    foreach $key (keys %{$variableshashref})
    {
        my $value = $variableshashref->{$key};

        # special handling for PRODUCTVERSION, if $allvariables->{'POSTVERSIONEXTENSION'}
        if (( $key eq "PRODUCTVERSION" ) && ( $variableshashref->{'POSTVERSIONEXTENSION'} )) { $value = $value . " " . $variableshashref->{'POSTVERSIONEXTENSION'}; }

        replace_one_variable_in_translationfile($translationfile, $value, $key);
    }
}

#########################################################
# Getting the translation file for the nsis installer
#########################################################

sub get_translation_file
{
    my ($allvariableshashref) = @_;
    my $translationfilename = $installer::globals::idtlanguagepath . $installer::globals::separator . $installer::globals::nsisfilename;
    if ( ! -f $translationfilename ) { installer::exiter::exit_program("ERROR: Could not find language file $translationfilename!", "get_translation_file"); }
    my $translationfile = installer::files::read_file($translationfilename);
    replace_variables($translationfile, $allvariableshashref);

    return $translationfile;
}

####################################################
# Removing english, if it was added before
####################################################

sub remove_english_for_nsis_installer
{
    my ($languagestringref, $languagesarrayref) = @_;

    # $$languagestringref =~ s/en-US_//;
    # shift(@{$languagesarrayref});

    @{$languagesarrayref} = ("en-US");  # only english for NSIS installer!
}

####################################################
# Creating link tree for upload
####################################################

sub create_link_tree
{
    my ($sourcedownloadfile, $destfilename, $versionstring) = @_;

    if ( ! $installer::globals::ooouploaddir ) { installer::exiter::exit_program("ERROR: Directory for OOo upload not defined!", "create_link_tree"); }
    my $versiondir = $installer::globals::ooouploaddir . $installer::globals::separator . $versionstring;
    my $infoline = "Directory for the link: $versiondir\n";
    push(@installer::globals::logfileinfo, $infoline);

    if ( ! -d $versiondir ) { installer::systemactions::create_directory_structure($versiondir); }

    # inside directory $versiondir all links have to be created
    my $linkdestination = $versiondir . $installer::globals::separator . $destfilename;

    # If there is an older version of this file (link), it has to be removed
    if ( -f $linkdestination ) { unlink($linkdestination); }

    $infoline = "Creating hard link from $sourcedownloadfile to $linkdestination\n";
    push(@installer::globals::logfileinfo, $infoline);
    installer::systemactions::hardlink_one_file($sourcedownloadfile, $linkdestination);
}

#######################################################
# Setting supported platform for Sun OpenOffice.org
# builds
#######################################################

sub is_supported_platform
{
    my $is_supported = 0;

    if (( $installer::globals::islinuxrpmbuild ) ||
        ( $installer::globals::issolarissparcbuild ) ||
        ( $installer::globals::issolarisx86build ) ||
        ( $installer::globals::iswindowsbuild ))
    {
        $is_supported = 1;
    }

    return $is_supported;
}

####################################################
# Creating download installation sets
####################################################

sub create_download_sets
{
    my ($installationdir, $includepatharrayref, $allvariableshashref, $downloadname, $languagestringref, $languagesarrayref) = @_;

    my $infoline = "";

    installer::logger::print_message( "\n******************************************\n" );
    installer::logger::print_message( "... creating download installation set ...\n" );
    installer::logger::print_message( "******************************************\n" );

    installer::logger::include_header_into_logfile("Creating download installation sets:");

    # special handling for installation sets, to which english was added automatically
    if ( $installer::globals::added_english ) { remove_english_for_nsis_installer($languagestringref, $languagesarrayref); }

    my $firstdir = $installationdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$firstdir);

    my $lastdir = $installationdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$lastdir);

    if ( $lastdir =~ /\./ ) { $lastdir =~ s/\./_download_inprogress\./ }
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

    $installer::globals::saveinstalldir = $downloaddir;

    # evaluating the name of the download file

    if ( $allvariableshashref->{'OOODOWNLOADNAME'} ) { $downloadname = set_download_filename($languagestringref, $allvariableshashref); }
    else { $downloadname = resolve_variables_in_downloadname($allvariableshashref, $downloadname, $languagestringref); }

    if ( ! $installer::globals::iswindowsbuild )    # Unix specific part
    {

        # getting the path of the getuid.so (only required for Solaris and Linux)
        my $getuidlibrary = "";
        if (( $installer::globals::issolarisbuild ) || ( $installer::globals::islinuxbuild )) { $getuidlibrary = get_path_for_library($includepatharrayref); }

        if ( $allvariableshashref->{'OOODOWNLOADNAME'} )
        {
            my $downloadfile = create_tar_gz_file_from_directory($installationdir, $getuidlibrary, $downloaddir, $downloadname);
        }
        else
        {
            # find and read setup script template
            my $scriptfilename = "downloadscript.sh";
            my $scriptref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$scriptfilename, $includepatharrayref, 0);
            if ($$scriptref eq "") { installer::exiter::exit_program("ERROR: Could not find script file $scriptfilename!", "create_download_sets"); }
            my $scriptfile = installer::files::read_file($$scriptref);

            $infoline = "Found  script file $scriptfilename: $$scriptref \n";
            push( @installer::globals::logfileinfo, $infoline);

            # add product name into script template
            put_productname_into_script($scriptfile, $allvariableshashref);

            # replace linenumber in script template
            put_linenumber_into_script($scriptfile);

            # calling sum to determine checksum and size of the tar file
            my $sumout = call_sum($installationdir, $getuidlibrary);

            # writing checksum and size into scriptfile
            put_checksum_and_size_into_script($scriptfile, $sumout);

            # saving the script file
            my $newscriptfilename = determine_scriptfile_name($downloadname);

            installer::logger::print_message( "... including installation set into $newscriptfilename ... \n" );

            $newscriptfilename = save_script_file($downloaddir, $newscriptfilename, $scriptfile);

            if (( $installer::globals::issolarisbuild ) && ( $ENV{'SPECIAL_ADA_BUILD'} )) { create_tar_gz_file_from_package($installationdir, $getuidlibrary); }
            else { include_package_into_script($installationdir, $newscriptfilename, $getuidlibrary); }
        }
    }
    else    # Windows specific part
    {
        my $localnsisdir = installer::systemactions::create_directories("nsis", $languagestringref);
        # push(@installer::globals::removedirs, $localnsisdir);

        # find nsis in the system
        my $nsispath = get_path_to_nsis_sdk();

        if ( $nsispath eq "" ) {
            # If nsis is not found just skip the rest of this function
            # and do not create the NSIS file.
            $infoline = "\nNo NSIS SDK found. Skipping the generation of NSIS file.\n";
            push(@installer::globals::logfileinfo, $infoline);
            installer::logger::print_message( "... no NSIS SDK found. Skipping the generation of NSIS file ... \n" );
            return $downloaddir;
        }

        # find the ulf file for translation
        my $mlffile = get_translation_file($allvariableshashref);

        # copy language files into nsis directory and translate them
        copy_and_translate_nsis_language_files($nsispath, $localnsisdir, $languagesarrayref, $mlffile);

        # find and read the nsi file template
        my $templatefilename = "downloadtemplate.nsi";
        my $templateref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$templatefilename, $includepatharrayref, 0);
        if ($$templateref eq "") { installer::exiter::exit_program("ERROR: Could not find nsi template file $templatefilename!", "create_download_sets"); }
        my $templatefile = installer::files::read_file($$templateref);

        # add product name into script template
        put_windows_productname_into_template($templatefile, $allvariableshashref);
        put_banner_bmp_into_template($templatefile, $includepatharrayref, $allvariableshashref);
        put_welcome_bmp_into_template($templatefile, $includepatharrayref, $allvariableshashref);
        put_setup_ico_into_template($templatefile, $includepatharrayref, $allvariableshashref);
        put_publisher_into_template($templatefile);
        put_website_into_template($templatefile);
        put_javafilename_into_template($templatefile, $allvariableshashref);
        put_windows_productversion_into_template($templatefile, $allvariableshashref);
        put_windows_productpath_into_template($templatefile, $allvariableshashref, $languagestringref);
        put_outputfilename_into_template($templatefile, $downloadname);
        put_filelist_into_template($templatefile, $installationdir);
        put_language_list_into_template($templatefile, $languagesarrayref);
        put_nsis_path_into_template($templatefile, $localnsisdir);
        put_output_path_into_template($templatefile, $downloaddir);
        put_version_specific_code_into_template($templatefile);

        my $nsifilename = save_script_file($localnsisdir, $templatefilename, $templatefile);

        installer::logger::print_message( "... created NSIS file $nsifilename ... \n" );

        # starting the NSIS SDK to create the download file
        call_nsis($nsispath, $nsifilename);
    }

    return $downloaddir;
}

####################################################
# Creating OOo upload tree
####################################################

sub create_download_link_tree
{
    my ($downloaddir, $languagestringref, $allvariableshashref) = @_;

    my $infoline;

    installer::logger::print_message( "\n******************************************\n" );
    installer::logger::print_message( "... creating download hard link ...\n" );
    installer::logger::print_message( "******************************************\n" );

    installer::logger::include_header_into_logfile("Creating download hard link:");
    installer::logger::include_timestamp_into_logfile("\nPerformance Info: Creating hard link, start");

    if ( is_supported_platform() )
    {
        my $versionstring = "";
        # Already defined $installer::globals::oooversionstring and $installer::globals::ooodownloadfilename ?

        if ( ! $installer::globals::oooversionstring ) { $versionstring = get_current_version(); }
        else { $versionstring = $installer::globals::oooversionstring; }

        # Is $versionstring empty? If yes, there is nothing to do now.

        $infoline = "Version string is set to: $versionstring\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ( $versionstring )
        {
            # Now the downloadfilename has to be set (if not already done)
            my $destdownloadfilename = "";
            if ( ! $installer::globals::ooodownloadfilename ) { $destdownloadfilename = set_download_filename($languagestringref, $versionstring, $allvariableshashref); }
            else { $destdownloadfilename = $installer::globals::ooodownloadfilename; }

            if ( $destdownloadfilename )
            {
                $destdownloadfilename = $destdownloadfilename . $installer::globals::downloadfileextension;

                $infoline = "Setting destination download file name: $destdownloadfilename\n";
                push( @installer::globals::logfileinfo, $infoline);

                my $sourcedownloadfile = $downloaddir . $installer::globals::separator . $installer::globals::downloadfilename;

                $infoline = "Setting source download file name: $sourcedownloadfile\n";
                push( @installer::globals::logfileinfo, $infoline);

                create_link_tree($sourcedownloadfile, $destdownloadfilename, $versionstring);
                # my $md5sumoutput = call_md5sum($downloadfile);
                # my $md5sum = get_md5sum($md5sumoutput);

            }
        }
        else
        {
            $infoline = "Version string is empty. Nothing to do!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
    else
    {
        $infoline = "Platform not used for hard linking. Nothing to do!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    installer::logger::include_timestamp_into_logfile("Performance Info: Creating hard link, stop");
}

1;
