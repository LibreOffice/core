#*************************************************************************
#
#   $RCSfile: servicesfile.pm,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: rt $ $Date: 2004-07-13 09:10:00 $
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

package installer::servicesfile;

use Cwd;
use installer::converter;
use installer::existence;
use installer::exiter;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::remover;
use installer::scriptitems;
use installer::systemactions;

################################################################
# Adding the newly created file into the files collector
################################################################

sub add_services_file_into_filearray
{
    my ( $filesarrayref, $servicesfile, $servicesname ) = @_;

    # Some data are set now, others are taken from the file "gid_File_Lib_Vcl"

    my %servicesfile = ();  # This has to be done always, not only once

    # Taking the data from the "gid_File_Lib_Vcl"

    my $vclgid = "gid_File_Lib_Vcl";
    my $vclfile = installer::existence::get_specified_file($filesarrayref, $vclgid);

    # copying all base data
    installer::converter::copy_item_object($vclfile, \%servicesfile);

    # and overriding all new values

    $servicesfile{'ismultilingual'} = 0;
    $servicesfile{'sourcepath'} = $servicesfile;
    $servicesfile{'Name'} = $servicesname;
    $servicesfile{'UnixRights'} = "644";
    $servicesfile{'gid'} = "gid_File_Rdb_Services";

    my $destinationpath = $vclfile->{'destination'};
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$destinationpath);
    $servicesfile{'destination'} = $destinationpath . $servicesname;

    push(@{$filesarrayref}, \%servicesfile);
}

############################################################################
# Adding the newly created legacy_binfilters.rdb into the files collector
############################################################################

sub add_legacy_binfilters_rdb_file_into_filearray
{
    my ( $filesarrayref, $includepatharrayref ) = @_;

    # Some data are set now, others are taken from the file "gid_File_Lib_Vcl"

    my %legacyfile = ();    # This has to be done always, not only once

    my $legacyfilename = "legacy_binfilters.rdb";

    my $legacyfilesourceref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$legacyfilename, $includepatharrayref, 1);
    if ( $$legacyfilesourceref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $legacyfilename!", "add_legacy_binfilters_rdb_file_into_filearray"); }

    # Taking the data from the "gid_File_Lib_Vcl"

    my $vclgid = "gid_File_Lib_Vcl";
    my $vclfile = installer::existence::get_specified_file($filesarrayref, $vclgid);

    # copying all base data
    installer::converter::copy_item_object($vclfile, \%legacyfile);

    # and overriding all new values

    $legacyfile{'ismultilingual'} = 0;
    $legacyfile{'sourcepath'} = $$legacyfilesourceref;
    $legacyfile{'Name'} = $legacyfilename;
    $legacyfile{'UnixRights'} = "644";
    $legacyfile{'gid'} = "gid_File_Rdb_Legacy_Binfilters";

    my $destinationpath = $vclfile->{'destination'};
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$destinationpath);
    $legacyfile{'destination'} = $destinationpath . $legacyfilename;

    push(@{$filesarrayref}, \%legacyfile);
}

################################################################
# Generating a file url from a path
################################################################

sub make_file_url
{
    my ( $path ) = @_;

    my $fileurl = "";

    # removing ending slash/backslash

    installer::remover::remove_ending_pathseparator(\$path);

    if ($installer::globals::iswin)
    {
        $path =~ s/\\/\//g;
        $fileurl = "file\:\/\/\/" . $path;
    }
    else
    {
        $fileurl = "file\:\/\/" . $path;
    }

    return $fileurl;
}

################################################################
# Determining all sourcepath from the uno components
################################################################

sub get_all_sourcepathes
{
    my ( $filesref ) = @_;

    my @pathes = ();

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $path = $onefile->{'sourcepath'};

        installer::pathanalyzer::get_path_from_fullqualifiedname(\$path);

        if (! installer::existence::exists_in_array($path, \@pathes))
        {
            push(@pathes, $path);
        }
    }

    return \@pathes;
}

################################################################
# Registering all uno component files in the services.rdb
################################################################

sub register_unocomponents
{
    my ($unocomponents, $regcompfileref, $servicesfile) = @_;

    installer::logger::include_header_into_logfile("Registering UNO components:");

    my $error_occured = 0;
    my $counter = 0;

    my $systemcall = "";

    my $allsourcepathes = get_all_sourcepathes($unocomponents);

    for ( my $j = 0; $j <= $#{$allsourcepathes}; $j++ )
    {
        my $filestring = "";
        my $onesourcepath = ${$allsourcepathes}[$j];
        my $to = "";
        my $from = cwd();
        if ( $installer::globals::iswin ) { $from =~ s/\//\\/g; }

        for ( my $i = 0; $i <= $#{$unocomponents}; $i++ )
        {
            my $doinclude = 1;
            my $sourcepath = ${$unocomponents}[$i]->{'sourcepath'};

            $to = $sourcepath;
            installer::pathanalyzer::get_path_from_fullqualifiedname(\$to);

            if (!($to eq $onesourcepath)) { $doinclude = 0; }

            if ( $doinclude )
            {
                my $filename = ${$unocomponents}[$i]->{'Name'};
                $filestring = $filestring . $filename . "\;";
                $counter++;
            }

            if ((( $counter > 0 ) && ( $counter%$installer::globals::unomaxservices == 0 )) || (( $counter > 0 ) && ( $i == $#{$unocomponents} )))  # limiting to $installer::globals::maxservices files
            {
                $filestring =~ s/\;\s*$//;
                chdir($onesourcepath);

                my @regcompoutput = ();

                $systemcall = "$$regcompfileref -register -s -r $servicesfile -c "  . $installer::globals::quote . $filestring . $installer::globals::quote . " |";

                open (REG, "$systemcall");
                while (<REG>) {push(@regcompoutput, $_); }
                close (REG);

                my $returnvalue = $?;   # $? contains the return value of the systemcall

                my $infoline = "Systemcall: $systemcall\n";
                push( @installer::globals::logfileinfo, $infoline);

                for ( my $j = 0; $j <= $#regcompoutput; $j++ ) { push( @installer::globals::logfileinfo, "$regcompoutput[$j]"); }

                if ($returnvalue)
                {
                    $infoline = "ERROR: $systemcall\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    $error_occured = 1;
                }
                else
                {
                    $infoline = "SUCCESS: $systemcall\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }

                chdir($from);

                $counter = 0;
                $filestring = "";
            }
        }
    }

    return $error_occured;
}

################################################################
# Registering all java component files in the services.rdb
################################################################

sub register_javacomponents
{
    my ($javacomponents, $regcompfileref, $servicesfile, $regcomprdb) = @_;

    installer::logger::include_header_into_logfile("Registering Java components:");

    my $error_occured = 0;
    my $systemcall;

    my $do_register = 1;
    if (!( $installer::globals::solarjava )) { $do_register = 0; }

    if ( $do_register )
    {
        my $allsourcepathes = get_all_sourcepathes($javacomponents);

        for ( my $j = 0; $j <= $#{$allsourcepathes}; $j++ )
        {
            my $filestring = "";
            my $onesourcepath = ${$allsourcepathes}[$j];
            my $to = "";
            my $from = cwd();
            if ( $installer::globals::iswin ) { $from =~ s/\//\\/g; }

            for ( my $i = 0; $i <= $#{$javacomponents}; $i++ )
            {
                my $doinclude = 1;
                my $sourcepath = ${$javacomponents}[$i]->{'sourcepath'};

                $to = $sourcepath;
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$to);

                if (!($to eq $onesourcepath)) { $doinclude = 0; }

                if ( $doinclude )
                {
                    my $filename = ${$javacomponents}[$i]->{'Name'};
                    $filename = "vnd.sun.star.expand\:\$UNO_JAVA_COMPONENT_PATH\/$filename";
                    $filestring = $filestring . $filename . "\;";
                    $counter++;
                }

                if ((( $counter > 0 ) && ( $counter%$installer::globals::javamaxservices == 0 )) || (( $counter > 0 ) && ( $i == $#{$javacomponents} )))    # limiting to $installer::globals::maxservices files
                {
                    $filestring =~ s/\;\s*$//;
                    chdir($onesourcepath);

                    my $fileurl = make_file_url($onesourcepath);

                    my @regcompoutput = ();

                    $systemcall = "$$regcompfileref -register -s -br $regcomprdb -r $servicesfile -c " . $installer::globals::quote . $filestring . $installer::globals::quote . " -l com.sun.star.loader.Java2 -env:UNO_JAVA_COMPONENT_PATH=" . $installer::globals::quote . $fileurl . $installer::globals::quote . " |";

                    open (REG, "$systemcall");
                    while (<REG>) {push(@regcompoutput, $_); }
                    close (REG);

                    my $returnvalue = $?;   # $? contains the return value of the systemcall

                    my $infoline = "Systemcall: $systemcall\n";
                    push( @installer::globals::logfileinfo, $infoline);

                    for ( my $j = 0; $j <= $#regcompoutput; $j++ ) { push( @installer::globals::logfileinfo, "$regcompoutput[$j]"); }

                    if ($returnvalue)
                    {
                        $infoline = "ERROR: $systemcall\n";
                        $error_occured = 1;
                    }
                    else
                    {
                        $infoline = "SUCCESS: $systemcall\n";
                    }

                    push( @installer::globals::logfileinfo, $infoline);

                    chdir($from);

                    $counter = 0;
                    $filestring = "";
                }
            }
        }
    }

    return $error_occured;
}

################################################################
# Iterating over all files, to find all files with the
# style UNO_COMPONENT. This can be libraries and jar files.
################################################################

sub register_all_components
{
    my ( $filesarrayref, $regcompfileref, $servicesfile, $regcomprdb ) = @_;

    my $registererrorflag = 0;

    my @unocomponents = ();
    my @javacomponents = ();

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $styles = "";

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( $styles =~ /\bUNO_COMPONENT\b/ )
        {
            my $filename = $onefile->{'Name'};

            if ( $filename =~ /\.jar\s*$/ ) # java component
            {
                push(@javacomponents, $onefile);
            }
            else    # uno_component
            {
                push(@unocomponents, $onefile);
            }
        }
    }

    if ( $#unocomponents > -1 ) { $error_occured = register_unocomponents(\@unocomponents, $regcompfileref, $servicesfile); }
    if ( $#javacomponents > -1 ) { $error_occured = register_javacomponents(\@javacomponents, $regcompfileref, $servicesfile, $regcomprdb); }

    if ( $error_occured ) { $registererrorflag = 1; }

    return $registererrorflag;
}

###################################################
# Include the solver lib directory into
# the LD_LIBRARY_PATH for Unix platforms
###################################################

sub include_regcomp_into_ld_library_path
{
    my ( $regcompfileref ) = @_;

    my $ld_library_path = $$regcompfileref;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$ld_library_path);
    $ld_library_path =~ s/\/\s*$//;     # removing ending slashes
    $ld_library_path =~ s/\/bin\./\/lib\./;
    $ld_library_path =~ s/\/bin\s*$/\/lib/; # when packing from flat

    my $oldldlibrarypathstring = "";
    if ( $ENV{'LD_LIBRARY_PATH'} ) { $oldldlibrarypathstring = $ENV{'LD_LIBRARY_PATH'}; }
    else { $oldldlibrarypathstring = "\."; }
    my $new_ld_library_path = $ld_library_path . $installer::globals::pathseparator . $oldldlibrarypathstring;
    $ENV{'LD_LIBRARY_PATH'} = $new_ld_library_path;

    my $infoline = "Setting LD_LIBRARY_PATH to $ENV{'LD_LIBRARY_PATH'}\n";
    push( @installer::globals::logfileinfo, $infoline);
}

##################################################################
# Setting the needed jar files into the CLASSPATH
# They are needed from regcomp.
# The jar files are defined in @installer::globals::regcompjars
##################################################################

sub prepare_classpath_for_java_registration
{
    my ( $includepatharrayref ) = @_;

    for ( my $i = 0; $i <= $#installer::globals::regcompjars; $i++ )
    {
        my $filename = $installer::globals::regcompjars[$i];

        my $jarfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 1);

        if ( $$jarfileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $filename for registering java components!", "prepare_classpath_for_java_registration"); }

        my $oldclasspathstring = "";
        if ( $ENV{'CLASSPATH'} ) { $oldclasspathstring = $ENV{'CLASSPATH'}; }
        else { $oldclasspathstring = "\."; }
        my $classpathstring = $$jarfileref . $installer::globals::pathseparator . $oldclasspathstring;

        $ENV{'CLASSPATH'} = $classpathstring;

        my $infoline = "Setting CLASSPATH to $ENV{'CLASSPATH'}\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

##################################################################
# Setting the jdk lib into the LD_LIBRARY_PATH (Unix)
# This is used by regcomp to register Java components.
# The jdk lib is defined in $installer::globals::jdklib
##################################################################

sub add_jdklib_into_ld_library_path
{
    my $oldldlibrarypathstring = "";
    if ( $ENV{'LD_LIBRARY_PATH'} ) { $oldldlibrarypathstring = $ENV{'LD_LIBRARY_PATH'}; }
    else { $oldldlibrarypathstring = "\."; }
    my $new_ld_library_path = $installer::globals::jdklib . $installer::globals::pathseparator . $oldldlibrarypathstring;
    $ENV{'LD_LIBRARY_PATH'} = $new_ld_library_path;

    my $infoline = "Setting LD_LIBRARY_PATH to $ENV{'LD_LIBRARY_PATH'}\n";
    push( @installer::globals::logfileinfo, $infoline);
}

##################################################################
# Adding the libraries included in zip files into path variable
# (for example mozruntime.zip). This is needed to register all
# libraries successfully.
##################################################################

sub add_path_to_pathvariable
{
    my ( $filesarrayref, $searchstring ) = @_;

    # determining the path

    my $path = "";

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $sourcepath = $onefile->{'sourcepath'};

        installer::pathanalyzer::get_path_from_fullqualifiedname(\$sourcepath);
        installer::remover::remove_ending_pathseparator(\$sourcepath);

        if ( $sourcepath =~ /\Q$searchstring\E\s*$/ )
        {
            $path = $sourcepath;
            last;
        }
    }

    # adding the path to the PATH variable

    if ( $path ne "" )
    {
        my $oldpath = "";
        if ( $ENV{'PATH'} ) { $oldpath = $ENV{'PATH'}; }
        else { $oldpath = "\."; }
        my $newpath = $path . $installer::globals::pathseparator . $oldpath;
        $ENV{'PATH'} = $newpath;

        my $infoline = "Setting PATH to $ENV{'PATH'}\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

##################################################################
# Setting the jre path into the PATH (Windows only)
# This is used by regcomp.exe to register Java components.
# The jre path is saved in $installer::globals::jrepath
##################################################################

sub add_jrepath_into_path
{
    my $oldpath = "";
    if ( $ENV{'PATH'} ) { $oldpath = $ENV{'PATH'}; }
    else { $oldpath = "\."; }
    my $newpath = $installer::globals::jrepath . $installer::globals::pathseparator . $oldpath;
    $ENV{'PATH'} = $newpath;

    my $infoline = "Setting PATH to $ENV{'PATH'}\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#######################################################################################
# Preparing a registry "regcomp.rdb" which regcomp can work on (types+java services).
# Copying the "udkapi.rdb", renaming it to "regcomp.rdb" and registering the
# libraries @installer::globals::regcompregisterlibs, which are javavm.uno.so
# and javaloader.uno.so or javavm.uno.dll and javaloader.uno.dll
#######################################################################################

sub prepare_regcomp_rdb
{
    my ( $regcompfile, $servicesdir, $includepatharrayref) = @_;

    # udkapi.rdb has to be found in the sourcepath

    my $filename = "udkapi.rdb";
    my $udkapirdbref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 1);
    if ( $$udkapirdbref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $filename for creating regcomp.rdb!", "prepare_regcomp_rdb"); }

    my $regcompfilename = "regcomp.rdb";
    my $regcomprdb = $servicesdir . $installer::globals::separator . $regcompfilename;

    # If there is an older version of this file, it has to be removed
    if ( -f $regcomprdb ) { unlink($regcomprdb); }

    installer::systemactions::copy_one_file($$udkapirdbref, $regcomprdb);

    # now the libraries in @installer::globals::regcompregisterlibs can be registered in the "regcomp.rdb"

    for ( my $i = 0; $i <= $#installer::globals::regcompregisterlibs; $i++ )
    {
        my $libfilename = $installer::globals::regcompregisterlibs[$i] . $installer::globals::libextension;
        my $libfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$libfilename, $includepatharrayref, 1);
        if ( $$libfileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $libfilename for creating regcomp.rdb!", "prepare_regcomp_rdb"); }

        my $from = cwd();
        if ( $installer::globals::iswin ) { $from =~ s/\//\\/g; }

        my $to = $$libfileref;
        installer::pathanalyzer::get_path_from_fullqualifiedname(\$to);

        chdir($to);

        my $systemcall = "$regcompfile -register -s -r $regcomprdb -c $libfilename";

        my $returnvalue = system($systemcall);

        chdir($from);

        my $infoline;
        if ($returnvalue) { $infoline = "ERROR: $systemcall\n"; }
        else { $infoline = "SUCCESS: $systemcall\n"; }

        push( @installer::globals::logfileinfo, $infoline);
    }

    return $regcomprdb;
}

################################################################
# Creating services.rdb file by registering all uno components
################################################################

sub create_services_rdb
{
    my ($filesarrayref, $includepatharrayref, $languagestringref) = @_;

    my $servicesname = "services.rdb";

    installer::logger::include_header_into_logfile("Creating $servicesname:");

    my $servicesdir = installer::systemactions::create_directories($servicesname, $languagestringref);
    push(@installer::globals::removedirs, $servicesdir);

    my $servicesfile = $servicesdir . $installer::globals::separator . $servicesname;

    # If there is an older version of this file, it has to be removed
    if ( -f $servicesfile ) { unlink($servicesfile); }

    # if ((-f $servicesfile) && (!($installer::globals::services_rdb_created))) { $installer::globals::services_rdb_created = 1; }

    # if ((!($installer::globals::services_rdb_created)) && $installer::globals::servicesrdb_can_be_created )   # This has to be done once
    if ( $installer::globals::servicesrdb_can_be_created )  # This has to be done always
    {
        # Creating the services.rdb in directory "inprogress"
        my $origservicesdir = $servicesdir;
        $servicesdir = installer::systemactions::make_numbered_dir("inprogress", $servicesdir);
        $servicesfile = $servicesdir . $installer::globals::separator . $servicesname;

        # determining the location of the file regcomp
        # Because the program regcomp.exe (regcomp) is used now, it has to be taken the version
        # from the platform, this script is running. It is not important, for which platform the
        # product is built.

        my $searchname;

        if ($installer::globals::isunix) { $searchname = "regcomp"; }
        else { $searchname = "regcomp.exe"; }

        $regcompfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$searchname, $includepatharrayref, 1);
        if ( $$regcompfileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $searchname for registering uno components!", "create_services_rdb"); }

        # For Windows the libraries included into the mozruntime.zip have to be added to the path
        if ($installer::globals::iswin) { add_path_to_pathvariable($filesarrayref, "mozruntime_zip"); }

        # setting the LD_LIBRARY_PATH, needed by regcomp
        # Linux: Take care of the lock daemon. He has to be started!
        # For windows it is necessary that "msvcp7x.dll" and "msvcr7x.dll" are included into the path !

        if ( $installer::globals::isunix ) { include_regcomp_into_ld_library_path($regcompfileref); }

        my $regcomprdb = "";

        if ( $installer::globals::solarjava )    # this is needed to register Java components
        {
            prepare_classpath_for_java_registration($includepatharrayref);

            if ( $installer::globals::isunix ) { add_jdklib_into_ld_library_path(); }
            else { add_jrepath_into_path(); }

            # Preparing a registry which regcomp can work on (types+java services).
            # Copying the "udkapi.rdb", renaming it to "regcomp.rdb" and registering the
            # libraries $(REGISTERLIBS_JAVA), which are javavm.uno.so and javaloader.uno.so
            # or javavm.uno.dll and javaloader.uno.dll

            $regcomprdb = prepare_regcomp_rdb($$regcompfileref, $servicesdir, $includepatharrayref);
        }

        # and now iteration over all files

        my $error_during_registration = register_all_components($filesarrayref, $regcompfileref, $servicesfile, $regcomprdb);

        # Dependent from the success, the registration directory can be renamed.

        if ( $error_during_registration )
        {
            $servicesdir = installer::systemactions::rename_string_in_directory($servicesdir, "inprogress", "with_error");
            push(@installer::globals::removedirs, $servicesdir);
        }
        else
        {
            $servicesdir = installer::systemactions::rename_directory($servicesdir, $origservicesdir);
        }

        $servicesfile = $servicesdir . $installer::globals::separator . $servicesname;
    }
    else
    {
        my $infoline;

        if (!($installer::globals::servicesrdb_can_be_created))
        {
            $infoline = "Warning: $servicesname was not created. Build platform and compiler do not match. Build platform: $installer::globals::plat, compiler : $installer::globals::compiler\n";
            push( @installer::globals::logfileinfo, $infoline);
        }

        if ( $installer::globals::services_rdb_created )
        {
            $infoline = "Info: $servicesname was not created. $servicesfile already exists.\n";
            push( @installer::globals::logfileinfo, $infoline);
        }

        if ((!($installer::globals::servicesrdb_can_be_created)) && (!($installer::globals::services_rdb_created)))
        {
            $infoline = "ERROR: $servicesname was not created and does not exist!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    # Adding the services.rdb to the filearray

    add_services_file_into_filearray( $filesarrayref, $servicesfile, $servicesname );

    # Setting the global variable $installer::globals::services_rdb_created

    $installer::globals::services_rdb_created = 1;

    # Adding the created file "legacy_binfilters.rdb" to the filearray

    add_legacy_binfilters_rdb_file_into_filearray($filesarrayref, $includepatharrayref);

}

1;
