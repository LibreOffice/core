#*************************************************************************
#
#   $RCSfile: msiglobal.pm,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2004-08-02 13:46:23 $
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

package installer::windows::msiglobal;

use Cwd;
use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::remover;
use installer::scriptitems;
use installer::systemactions;
use installer::windows::idtglobal;
use installer::windows::language;

###########################################################################
# Generating the header of the ddf file.
# The usage of ddf files is needed, because makecab.exe can only include
# one sourcefile into a cab file
###########################################################################

sub write_ddf_file_header
{
    my ($ddffileref, $cabinetfile, $installdir) = @_;

    my $oneline;

    $oneline = ".Set CabinetName1=" . $cabinetfile . "\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set ReservePerCabinetSize=128\n";  # This reserves space for a digital signature.
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set MaxDiskSize=CDROM\n";          # This allows the .cab file to be as large as needed.
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set CompressionType=MSZIP\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set Compress=ON\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set CompressionLevel=$installer::globals::cabfilecompressionlevel\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set Cabinet=ON\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set DiskDirectoryTemplate=" . $installdir . "\n";
    push(@{$ddffileref} ,$oneline);
}

##########################################################################
# Generation the list, in which the source of the files is connected
# with the cabinet destination file. Because more than one file needs
# to be included into a cab file, this has to be done via ddf files.
##########################################################################

sub generate_cab_file_list
{
    my ($filesref, $installdir, $ddfdir) = @_;

    my @cabfilelist = ();

    installer::logger::include_header_into_logfile("Generating ddf files");

    if (( $installer::globals::cab_file_per_component ) || ( $installer::globals::fix_number_of_cab_files ))
    {
        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            my $onefile = ${$filesref}[$i];
            my $cabinetfile = $onefile->{'cabinet'};
            my $sourcepath =  $onefile->{'sourcepath'};
            my $uniquename =  $onefile->{'uniquename'};

            # all files with the same cabinetfile are directly behind each other in the files collector

            my @ddffile = ();

            write_ddf_file_header(\@ddffile, $cabinetfile, $installdir);

            my $ddfline = "\"" . $sourcepath . "\"" . " " . $uniquename . "\n";
            push(@ddffile, $ddfline);

            my $nextfile = ${$filesref}[$i+1];
            my $nextcabinetfile = "";

            if ( $nextfile->{'cabinet'} ) { $nextcabinetfile = $nextfile->{'cabinet'}; }

            while ( $nextcabinetfile eq $cabinetfile )
            {
                $sourcepath =  $nextfile->{'sourcepath'};
                $uniquename =  $nextfile->{'uniquename'};
                $ddfline = "\"" . $sourcepath . "\"" . " " . $uniquename . "\n";
                push(@ddffile, $ddfline);
                $i++;                                           # increasing the counter!
                $nextfile = ${$filesref}[$i+1];
                if ( $nextfile ) { $nextcabinetfile = $nextfile->{'cabinet'}; }
                else { $nextcabinetfile = "_lastfile_"; }
            }

            # creating the DDF file

            my $ddffilename = $cabinetfile;
            $ddffilename =~ s/.cab/.ddf/;
            $ddfdir =~ s/\Q$installer::globals::separator\E\s*$//;
            $ddffilename = $ddfdir . $installer::globals::separator . $ddffilename;

            installer::files::save_file($ddffilename ,\@ddffile);
            my $infoline = "Created ddf file: $ddffilename\n";
            push(@installer::globals::logfileinfo, $infoline);

            # Writing the makecab system call

            my $oneline = "makecab.exe /F " . $ddffilename . "\n";

            push(@cabfilelist, $oneline);
        }
    }
    elsif ( $installer::globals::one_cab_file )
    {
        my @ddffile = ();

        my $cabinetfile = "";

        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            my $onefile = ${$filesref}[$i];
            $cabinetfile = $onefile->{'cabinet'};
            my $sourcepath =  $onefile->{'sourcepath'};
            my $uniquename =  $onefile->{'uniquename'};

            if ( $i == 0 ) { write_ddf_file_header(\@ddffile, $cabinetfile, $installdir); }

            my $ddfline = "\"" . $sourcepath . "\"" . " " . $uniquename . "\n";
            push(@ddffile, $ddfline);
        }

        # creating the DDF file

        my $ddffilename = $cabinetfile;
        $ddffilename =~ s/.cab/.ddf/;
        $ddfdir =~ s/\Q$installer::globals::separator\E\s*$//;
        $ddffilename = $ddfdir . $installer::globals::separator . $ddffilename;

        installer::files::save_file($ddffilename ,\@ddffile);
        my $infoline = "Created ddf file: $ddffilename\n";
        push(@installer::globals::logfileinfo, $infoline);

        # Writing the makecab system call

        my $oneline = "makecab.exe /F " . $ddffilename . "\n";

        push(@cabfilelist, $oneline);
    }
    else
    {
        installer::exiter::exit_program("ERROR: No cab file specification in globals.pm !", "create_media_table");
    }


    return \@cabfilelist;   # contains all system calls for packaging process
}

#################################################################
# Returning the name of the msi database
#################################################################

sub get_msidatabasename
{
    my ($allvariableshashref, $language) = @_;

    my $databasename = $allvariableshashref->{'PRODUCTNAME'} . $allvariableshashref->{'PRODUCTVERSION'};
    $databasename = lc($databasename);
    $databasename =~ s/\.//g;
    $databasename =~ s/\-//g;
    $databasename =~ s/\s//g;

    if ( $language )
    {
        if (!($language eq ""))
        {
            $databasename .= "_$language";
        }
    }

    $databasename .= ".msi";

    return $databasename;
}

#################################################################
# Creating the msi database
# This works only on Windows
#################################################################

sub create_msi_database
{
    my ($idtdirbase ,$msifilename) = @_;

    # -f : path containing the idt files
    # -d : msi database, including path
    # -c : create database
    # -i : include the following tables ("*" includes all available tables)

    my $msidb = "msidb.exe";    # Has to be in the path

    installer::logger::include_header_into_logfile("Creating msi database");

    $idtdirbase = installer::converter::make_path_conform($idtdirbase);

    $msifilename = installer::converter::make_path_conform($msifilename);

    my $systemcall = $msidb . " -f " . $idtdirbase . " -d " . $msifilename . " -c " . "-i \*";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $msidb!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed $msidb successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

#####################################################################
# Returning the value from sis.ulf for Summary Information Stream
#####################################################################

sub get_value_from_sis_lng
{
    my ($language, $languagefile, $searchstring) = @_;

    my $language_block = installer::windows::idtglobal::get_language_block_from_language_file($searchstring, $languagefile);
    my $newstring = installer::windows::idtglobal::get_language_string_from_language_block($language_block, $language, $searchstring);
    $newstring = "\"" . $newstring . "\"";

    return $newstring;
}

#################################################################
# Returning the msi version for the Summary Information Stream
#################################################################

sub get_msiversion_for_sis
{
    my $msiversion = "200";
    return $msiversion;
}

#################################################################
# Returning the word count for the Summary Information Stream
#################################################################

sub get_wordcount_for_sis
{
    my $wordcount = "0";
    return $wordcount;
}

#################################################################
# Returning the codepage for the Summary Information Stream
#################################################################

sub get_codepage_for_sis
{
    my ( $language ) = @_;

    my $codepage = "1252";  # determine dynamically in a function
    # my $codepage = "65001";       # UTF-8
    return $codepage;
}

#################################################################
# Returning the template for the Summary Information Stream
#################################################################

sub get_template_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $windowslanguage = installer::windows::language::get_windows_language($language);

    my $value = get_value_from_sis_lng($language, $languagefile, $searchstring );

    $value =~ s/\"\s*$//;                       # removing ending '"'

    $value = $value . ";" . $windowslanguage;   # adding the Windows language

    $value = $value . "\"";                     # adding ending '"'

    return $value ;
}

#################################################################
# Returning the PackageCode for the Summary Information Stream
#################################################################

sub get_packagecode_for_sis
{
    # always generating a new package code for each package

    my $guidref = get_guid_list(1); # only one GUID shall be generated

    ${$guidref}[0] =~ s/\s*$//;     # removing ending spaces

    my $guid = "\{" . ${$guidref}[0] . "\}";

    my $infoline = "PackageCode: $guid\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $guid;
}

#################################################################
# Returning the title for the Summary Information Stream
#################################################################

sub get_title_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $title = get_value_from_sis_lng($language, $languagefile, $searchstring );

    return $title;
}

#################################################################
# Returning the author for the Summary Information Stream
#################################################################

sub get_author_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $author = get_value_from_sis_lng($language, $languagefile, $searchstring );

    return $author;
}

#################################################################
# Returning the subject for the Summary Information Stream
#################################################################

sub get_subject_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $subject = get_value_from_sis_lng($language, $languagefile, $searchstring );

    return $subject;
}

#################################################################
# Returning the comment for the Summary Information Stream
#################################################################

sub get_comment_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $comment = get_value_from_sis_lng($language, $languagefile, $searchstring );

    return $comment;
}

#################################################################
# Returning the keywords for the Summary Information Stream
#################################################################

sub get_keywords_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $keywords = get_value_from_sis_lng($language, $languagefile, $searchstring );

    return $keywords;
}

######################################################################
# Returning the application name for the Summary Information Stream
######################################################################

sub get_appname_for_sis
{
    my ( $language, $languagefile, $searchstring ) = @_;

    my $appname = get_value_from_sis_lng($language, $languagefile, $searchstring );

    return $appname;
}

######################################################################
# Returning the security for the Summary Information Stream
######################################################################

sub get_security_for_sis
{
    my $security = "0";
    return $security;
}

#################################################################
# Writing the Summary information stream into the msi database
# This works only on Windows
#################################################################

sub write_summary_into_msi_database
{
    my ($msifilename, $language, $languagefile) = @_;

    # -g : requrired msi version
    # -c : codepage
    # -p : template

    installer::logger::include_header_into_logfile("Writing summary information stream");

    my $msiinfo = "msiinfo.exe";    # Has to be in the path

    my $msiversion = get_msiversion_for_sis();
    my $codepage = get_codepage_for_sis($language);
    my $template = get_template_for_sis($language,$languagefile, "OOO_SIS_TEMPLATE");
    my $guid = get_packagecode_for_sis();
    my $title = get_title_for_sis($language,$languagefile, "OOO_SIS_TITLE");
    my $author = get_author_for_sis($language,$languagefile, "OOO_SIS_AUTHOR");
    my $subject = get_subject_for_sis($language,$languagefile, "OOO_SIS_SUBJECT");
    my $comment = get_comment_for_sis($language,$languagefile, "OOO_SIS_COMMENT");
    my $keywords = get_keywords_for_sis($language,$languagefile, "OOO_SIS_KEYWORDS");
    my $appname = get_appname_for_sis($language,$languagefile, "OOO_SIS_APPNAME");
    my $security = get_security_for_sis();
    my $wordcount = get_wordcount_for_sis();

    $msifilename = installer::converter::make_path_conform($msifilename);

    my $systemcall = $msiinfo . " " . $msifilename . " -g " . $msiversion . " -c " . $codepage
                    . " -p " . $template . " -v " . $guid . " -t " . $title . " -a " . $author
                    . " -j " . $subject . " -o " . $comment . " -k " . $keywords . " -n " . $appname
                    . " -u " . $security . " -w " . $wordcount;

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $msiinfo!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed $msiinfo successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

#########################################################################
# For more than one language in the installation set:
# Use one database and create Transformations for all other languages
#########################################################################

sub create_transforms
{
    my ($languagesarray, $defaultlanguage, $installdir, $allvariableshashref) = @_;

    installer::logger::include_header_into_logfile("Creating Transforms");

    my $msitran = "msitran.exe";    # Has to be in the path

    $installdir = installer::converter::make_path_conform($installdir);

    # Syntax for creating a transformation
    # msitran.exe -g <baseDB> <referenceDB> <transformfile> [<errorhandling>}

    my $basedbname = get_msidatabasename($allvariableshashref, $defaultlanguage);
    $basedbname = $installdir . $installer::globals::separator . $basedbname;

    # Iterating over all files

    for ( my $i = 0; $i <= $#{$languagesarray}; $i++ )
    {
        my $onelanguage = ${$languagesarray}[$i];

        if ( $onelanguage eq $defaultlanguage ) { next; }

        my $referencedbname = get_msidatabasename($allvariableshashref, $onelanguage);
        $referencedbname = $installdir . $installer::globals::separator . $referencedbname;

        my $transformfile = $installdir . $installer::globals::separator . "trans_" . $onelanguage . ".mst";

        my $systemcall = $msitran . " " . " -g " . $basedbname . " " . $referencedbname . " " . $transformfile;

        my $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not execute $msitran!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Executed $msitran successfully!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }

        # The reference database can be deleted

        unlink($referencedbname);
    }
}

#########################################################################
# The default language msi database does not need to contain
# the langugage in the database name. Therefore the file
# is renamed. Example: "openofficeorg20_01.msi" to "openofficeorg20.msi"
#########################################################################

sub rename_msi_database_in_installset
{
    my ($defaultlanguage, $installdir, $allvariableshashref) = @_;

    installer::logger::include_header_into_logfile("Renaming msi database");

    my $olddatabasename = get_msidatabasename($allvariableshashref, $defaultlanguage);
    $olddatabasename = $installdir . $installer::globals::separator . $olddatabasename;

    my $newdatabasename = get_msidatabasename($allvariableshashref);
    $newdatabasename = $installdir . $installer::globals::separator . $newdatabasename;

    installer::systemactions::rename_one_file($olddatabasename, $newdatabasename);
}

##########################################################################
# Writing the databasename into the setup.ini.
##########################################################################

sub put_databasename_into_setupini
{
    my ($setupinifile, $allvariableshashref) = @_;

    my $databasename = get_msidatabasename($allvariableshashref);
    my $line = "database=" . $databasename . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the path to the instmsiw.exe into setup.ini
##########################################################################

sub put_instmsiwpath_into_setupini
{
    my ($setupinifile) = @_;

    my $instmsiwexepath = "instmsiw.exe";
    my $line = "instmsiw=" . $instmsiwexepath . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the path to the instmsia.exe into setup.ini
##########################################################################

sub put_instmsiapath_into_setupini
{
    my ($setupinifile) = @_;

    my $instmsiaexepath = "instmsia.exe";
    my $line = "instmsia=" . $instmsiaexepath . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the required msi version into setup.ini
##########################################################################

sub put_msiversion_into_setupini
{
    my ($setupinifile) = @_;

    my $msiversion = "2.0";
    my $line = "msiversion=" . $msiversion . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the productname into setup.ini
##########################################################################

sub put_productname_into_setupini
{
    my ($setupinifile, $allvariableshashref) = @_;

    my $productname = $allvariableshashref->{'PRODUCTNAME'};
    my $line = "productname=" . $productname . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the number of languages into setup.ini
##########################################################################

sub put_languagecount_into_setupini
{
    my ($setupinifile, $languagesarray) = @_;

    my $languagecount = $#{$languagesarray} + 1;
    my $line = "count=" . $languagecount . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the defaultlanguage into setup.ini
##########################################################################

sub put_defaultlanguage_into_setupini
{
    my ($setupinifile, $defaultlanguage) = @_;

    my $windowslanguage = installer::windows::language::get_windows_language($defaultlanguage);
    my $line = "default=" . $windowslanguage . "\n";
    push(@{$setupinifile}, $line);
}

##########################################################################
# Writing the information about transformations into setup.ini
##########################################################################

sub put_transforms_into_setupini
{
    my ($setupinifile, $onelanguage, $counter) = @_;

    my $windowslanguage = installer::windows::language::get_windows_language($onelanguage);
    my $transformfilename = "trans_" . $onelanguage . ".mst";

    my $line = "lang" . $counter . "=" . $windowslanguage . "," . $transformfilename . "\n";

    push(@{$setupinifile}, $line);
}

##########################################################################
# Generation the file setup.ini, that is used by the loader setup.exe.
##########################################################################

sub create_setup_ini
{
    my ($languagesarray, $defaultlanguage, $installdir, $allvariableshashref) = @_;

    installer::logger::include_header_into_logfile("Creating setup.ini");

    my $setupinifilename = $installdir . $installer::globals::separator . "setup.ini";

    my @setupinifile = ();
    my $setupinifile = \@setupinifile;

    my $line = "\[setup\]\n";
    push(@setupinifile, $line);

    put_databasename_into_setupini($setupinifile, $allvariableshashref);
    put_instmsiwpath_into_setupini($setupinifile);
    put_instmsiapath_into_setupini($setupinifile);
    put_msiversion_into_setupini($setupinifile);
    put_productname_into_setupini($setupinifile, $allvariableshashref);

    $line = "\[languages\]\n";
    push(@setupinifile, $line);

    put_languagecount_into_setupini($setupinifile, $languagesarray);
    put_defaultlanguage_into_setupini($setupinifile, $defaultlanguage);

    if ( $#{$languagesarray} > 0 )  # writing the transforms information
    {
        my $counter = 1;

        for ( my $i = 0; $i <= $#{$languagesarray}; $i++ )
        {
            if ( ${$languagesarray}[$i] eq $defaultlanguage ) { next; }

            put_transforms_into_setupini($setupinifile, ${$languagesarray}[$i], $counter);
            $counter++;
        }
    }

    installer::files::save_file($setupinifilename, $setupinifile);

    $infoline = "Generated file $setupinifilename !\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#################################################################
# Copying the files defined as ScpActions into the
# installation set.
#################################################################

sub copy_scpactions_into_installset
{
    my ($defaultlanguage, $installdir, $allscpactions) = @_;

    installer::logger::include_header_into_logfile("Copying files into installation set");

    for ( my $i = 0; $i <= $#{$allscpactions}; $i++ )
    {
        my $onescpaction = ${$allscpactions}[$i];

        if ( $onescpaction->{'Name'} eq "loader.exe" ) { next; }    # do not copy this ScpAction loader

        # only copying language independent files or files with the correct language (the defaultlanguage)

        my $filelanguage = $onescpaction->{'specificlanguage'};

        if ( ($filelanguage eq $defaultlanguage) || ($filelanguage eq "") )
        {
            my $sourcefile = $onescpaction->{'sourcepath'};
            my $destfile = $installdir . $installer::globals::separator . $onescpaction->{'DestinationName'};

            installer::systemactions::copy_one_file($sourcefile, $destfile);
        }
    }
}

#################################################################
# Copying the files for the Windows installer into the
# installation set (setup.exe, instmsia.exe, instmsiw.exe).
#################################################################

sub copy_windows_installer_files_into_installset
{
    my ($installdir, $includepatharrayref) = @_;

    # the source directory is defined with the parameter "-msifiles"
    # in the variable $installer::globals::msifilespath

    @copyfile = ();
    push(@copyfile, "instmsia.exe");
    push(@copyfile, "instmsiw.exe");
    push(@copyfile, "loader2.exe");

    for ( my $i = 0; $i <= $#copyfile; $i++ )
    {
        # my $sourcefile = $installer::globals::msifilespath . $installer::globals::separator . $copyfile[$i];

        my $filename = $copyfile[$i];
        my $sourcefileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 1);

        if ( ! -f $$sourcefileref ) { installer::exiter::exit_program("ERROR: msi file not found: $$sourcefileref !", "copy_windows_installer_files_into_installset"); }

        my $destfile;
        if ( $copyfile[$i] eq "loader2.exe" ) { $destfile = "setup.exe"; }  # renaming the loader
        else { $destfile = $copyfile[$i]; }

        $destfile = $installdir . $installer::globals::separator . $destfile;

        installer::systemactions::copy_one_file($$sourcefileref, $destfile);
    }
}

#################################################################
# Copying the child projects into the
# installation set
#################################################################

sub copy_child_projects_into_installset
{
    my ($installdir) = @_;

    # the source directory is defined with the parameter "-msifiles"
    # in the variable $installer::globals::msifilespath

    @copydirs = ();
    push(@copydirs, "java");
    push(@copydirs, "adabas");

    for ( my $i = 0; $i <= $#copydirs; $i++ )
    {
        my $sourcedir = $installer::globals::msifilespath . $installer::globals::separator . $copydirs[$i];

        if ( ! -d $sourcedir ) { installer::exiter::exit_program("ERROR: Child project directory not found: $sourcedir !", "copy_child_projects_into_installset"); }

        my $destdir = $installdir . $installer::globals::separator . $copydirs[$i];

        if ( ! -d $destdir) { installer::systemactions::create_directory($destdir); }

        installer::systemactions::copy_directory($sourcedir, $destdir);
    }
}

#################################################################
# Getting a list of GUID using uuidgen.exe.
# This works only on Windows
#################################################################

sub get_guid_list
{
    my ($number) = @_;

    installer::logger::include_header_into_logfile("Generating $number GUID");

    my $uuidgen = "uuidgen.exe";        # Has to be in the path

    # "-c" for uppercase output

    my $systemcall = "$uuidgen -n$number -c |";

    open (UUIDGEN, "$systemcall");
    my @uuidlist = <UUIDGEN>;
    close (UUIDGEN);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $comparenumber = $#uuidlist + 1;

    if ( $comparenumber == $number )
    {
        $infoline = "Success: Executed $uuidgen successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "ERROR: Could not execute $uuidgen successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return \@uuidlist;
}

#################################################################
# Filling real component GUID into the component table.
# This works only on Windows
#################################################################

sub set_uuid_into_component_table
{
    my ($idtdirbase) = @_;

    my $componentfilename  = $idtdirbase . $installer::globals::separator . "Componen.idt";

    my $componentfile = installer::files::read_file($componentfilename);

    my $number = $#{$componentfile} + 1;

    my $guidref = get_guid_list($number);

    for ( my $i = 0; $i <= $#{$componentfile}; $i++ )
    {
        my $uuid = ${$guidref}[$i];
        installer::remover::remove_leading_and_ending_whitespaces(\$uuid);
        ${$componentfile}[$i] =~ s/COMPONENTGUID/$uuid/;
    }

    installer::files::save_file($componentfilename, $componentfile)
}


#################################################################
# Executing the created batch file to pack all files.
# This works only on Windows
#################################################################

sub execute_packaging
{
    my ($localpackjobref, $loggingdir) = @_;

    installer::logger::include_header_into_logfile("Packaging process");

    my $from = cwd();
    my $to = $loggingdir;

    chdir($to);

    for ( my $i = 0; $i <= $#{$localpackjobref}; $i++ )
    {
        my $systemcall = ${$localpackjobref}[$i];

        my $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: $systemcall !";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: $systemcall";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    chdir($from);
}

###############################################################
# Setting the global variables ProductCode and the UpgradeCode
###############################################################

sub set_global_code_variables
{
    my ( $languagesref ) = @_;

    my $codefile = installer::files::read_file($installer::globals::codefilename);

    my $onelanguage = "";

    if ( $#{$languagesref} > 0 )    # more than one language
    {
        if (( ${$languagesref}[1] =~ /81/ ) ||
            ( ${$languagesref}[1] =~ /82/ ) ||
            ( ${$languagesref}[1] =~ /86/ ) ||
            ( ${$languagesref}[1] =~ /88/ ) ||
            ( ${$languagesref}[1] =~ /jp/ ) ||
            ( ${$languagesref}[1] =~ /ko/ ) ||
            ( ${$languagesref}[1] =~ /zh/ ))
        {
            $onelanguage = "multiasia";
        }
        else
        {
            $onelanguage = "multiwestern";
        }
    }
    else    # only one language
    {
        $onelanguage = ${$languagesref}[0];
    }

    # ProductCode has to be specified in each language

    my $searchstring = "PRODUCTCODE";
    my $codeblock = installer::windows::idtglobal::get_language_block_from_language_file($searchstring, $codefile);
    $installer::globals::productcode = installer::windows::idtglobal::get_code_from_code_block($codeblock, $onelanguage);

    # UpgradeCode can take english as default, if not defined in specified language

    $searchstring = "UPGRADECODE";
    $codeblock = installer::windows::idtglobal::get_language_block_from_language_file($searchstring, $codefile);
    $installer::globals::upgradecode = installer::windows::idtglobal::get_language_string_from_language_block($codeblock, $onelanguage, "");

    if ( $installer::globals::productcode eq "" ) { installer::exiter::exit_program("ERROR: ProductCode for language $onelanguage not defined in $installer::globals::codefilename !", "set_global_code_variables"); }
    if ( $installer::globals::upgradecode eq "" ) { installer::exiter::exit_program("ERROR: UpgradeCode not defined in $installer::globals::codefilename !", "set_global_code_variables"); }

    $infoline = "Setting ProductCode to: $installer::globals::productcode \n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "Setting UpgradeCode to: $installer::globals::upgradecode \n";
    push( @installer::globals::logfileinfo, $infoline);
}

1;
