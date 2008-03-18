#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: servicesfile.pm,v $
#
#   $Revision: 1.32 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:01:29 $
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

sub add_services_sourcepath_into_filearray
{
    my ( $filesarrayref, $servicesfile, $servicesname ) = @_;

    my $found = 0;
    my $onefile;

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        $onefile = ${$filesarrayref}[$i];
        my $name = $onefile->{'Name'};

        if ( $servicesname eq $name )
        {
            $found = 1;
            $onefile->{'sourcepath'} = $servicesfile;   # setting the sourcepath!
            last;
        }
    }

    if ( ! $found ) { installer::exiter::exit_program("ERROR: Did not find $servicesname in files collector!", "add_services_sourcepath_into_filearray"); }

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
    my ($allvariableshashref, $unocomponents, $regcompfileref, $servicesfile, $nativeservicesurlprefix) = @_;

    installer::logger::include_header_into_logfile("Registering UNO components:");

    my $error_occured = 0;
    my $filestring = "";
    for ( my $i = 0; $i <= $#{$unocomponents}; )
    {
        my $sourcepath = ${$unocomponents}[$i++]->{'sourcepath'};

        $filestring = $filestring . make_file_url($sourcepath);

        if ( $i % $installer::globals::unomaxservices == 0 || $i > $#{$unocomponents} )    # limiting to $installer::globals::maxservices files
        {
            my @regcompoutput = ();

            my $systemcall = "$installer::globals::wrapcmd $$regcompfileref -register -r ".fix_cygwin_path($servicesfile)." -c "  . $installer::globals::quote . $filestring . $installer::globals::quote . " -wop=" . $installer::globals::quote . $nativeservicesurlprefix . $installer::globals::quote . " 2\>\&1 |";

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

            $filestring = "";
        }
        else
        {
            $filestring = $filestring . ";";
        }
    }

    return $error_occured;
}

################################################################
# Registering all java component files in the services.rdb
################################################################

sub register_javacomponents
{
    my ($allvariableshashref, $javacomponents, $regcompfileref, $servicesfile, $regcomprdb, $javaservicesurlprefix) = @_;

    installer::logger::include_header_into_logfile("Registering Java components:");

    my $ridljar_ref = "ridl.jar";
    my $ure_internal_java_dir_ref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$ridljar_ref, "", 1);
    installer::pathanalyzer::get_path_from_fullqualifiedname($ure_internal_java_dir_ref);
    if ( $$ure_internal_java_dir_ref eq "" ) { installer::exiter::exit_program("Could not determine URE_INTERNAL_JAVA_DIR when registering Java components!", "register_javacomponents"); }

    my $error_occured = 0;

    my $do_register = 1;
    if (!( $installer::globals::solarjava )) { $do_register = 0; }

    if ( $do_register )
    {
        my $filestring = "";

        for ( my $i = 0; $i <= $#{$javacomponents}; )
        {
            my $sourcepath = ${$javacomponents}[$i++]->{'sourcepath'};

            $filestring = $filestring . make_file_url($sourcepath);

            if ( $i % $installer::globals::javamaxservices == 0 || $i > $#{$javacomponents} )   # limiting to $installer::globals::maxservices files
            {
                my @regcompoutput = ();

                my $systemcall = "$installer::globals::wrapcmd $$regcompfileref -register -br ".fix_cygwin_path($regcomprdb)." -r ".fix_cygwin_path($servicesfile)." -c " . $installer::globals::quote . $filestring . $installer::globals::quote . " -l com.sun.star.loader.Java2 -wop=" . $installer::globals::quote . $javaservicesurlprefix . $installer::globals::quote ." -env:URE_INTERNAL_JAVA_DIR=" . $installer::globals::quote . make_file_url($$ure_internal_java_dir_ref) . $installer::globals::quote . " 2\>\&1 |";

                open (REG, "$systemcall");
                while (<REG>) {push(@regcompoutput, $_); }
                close (REG);

                my $returnvalue = $?;   # $? contains the return value of the systemcall

                my $infoline = "Systemcall: $systemcall\n";
                push( @installer::globals::logfileinfo, $infoline);

                for ( my $k = 0; $k <= $#regcompoutput; $k++ ) { push( @installer::globals::logfileinfo, "$regcompoutput[$k]"); }

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

                $filestring = "";
            }
            else
            {
                $filestring = $filestring . ";";
            }
        }
    }

    return $error_occured;
}



################################################################
# Helper routine to change cygwin (POSIX) path to DOS notation
# if needed
################################################################
sub fix_cygwin_path
{
    my ( $path ) = @_;

    if ( $installer::globals::iswin eq 1 && $ENV{'USE_SHELL'} ne "4nt" && $installer::globals::wrapcmd eq "" )
    {
    $path = qx{cygpath -m "$path"};
    chomp($path);
    }

    return $path;
}



################################################################
# Registering all uno component files in the services.rdb
################################################################
sub get_source_path_cygwin_safe
{
    my ( $name, $array, $int ) = @_;

    my $ret = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$name, $array, $int);
    if ( $installer::globals::iswin eq 1 && $ENV{'USE_SHELL'} ne "4nt" )
    {
    if( substr( $$ret, 1,1 ) eq ":" )
    {
        chomp($$ret = qx{cygpath -u "$$ret"});
    }
    }
    return $ret;
}

sub register_pythoncomponents
{
    my ($pythoncomponents, $regcompfileref, $servicesfile,$includepatharrayref) = @_;

    installer::logger::include_header_into_logfile("Registering python UNO components:");

    my $error_occured = 0;
    my $counter = 0;

    my $systemcall = "";

    my $allsourcepathes = get_all_sourcepathes($pythoncomponents);

    for ( my $j = 0; $j <= $#{$allsourcepathes}; $j++ )
    {
        my $filestring = "";
        my $onesourcepath = ${$allsourcepathes}[$j];
        my $to = "";
        my $from = cwd();
        if ( $installer::globals::iswin ) { $from =~ s/\//\\/g; }

        my $typesrdbname = "types.rdb";

        # FIXME: Remove the unneeded
        # get_source_path_cygwin_safe() -> fix_cygwin_path()
        # when WRAPCMD is gone
        my $typesrdbref = get_source_path_cygwin_safe($typesrdbname, $includepatharrayref, 1);

        if ( $$typesrdbref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $typesrdbname !", "register_pythoncomponents"); }

        my $pyunoservicesrdbname = "pyuno_services.rdb";
        my $pyunoservicesrdbref = get_source_path_cygwin_safe($pyunoservicesrdbname, $includepatharrayref, 1);

        if ( $$pyunoservicesrdbref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $pyunoservicesrname !", "register_pythoncomponents"); }

        for ( my $i = 0; $i <= $#{$pythoncomponents}; $i++ )
        {
            my $doinclude = 1;
            my $sourcepath = ${$pythoncomponents}[$i]->{'sourcepath'};

            $to = $sourcepath;
            installer::pathanalyzer::get_path_from_fullqualifiedname(\$to);

            if (!($to eq $onesourcepath)) { $doinclude = 0; }

            if ( $doinclude )
            {
                my $filename = ${$pythoncomponents}[$i]->{'Name'};
                $filestring = $filestring . $filename . "\;";
                $counter++;
            }

#           if ((( $counter > 0 ) && ( $counter%$installer::globals::unomaxservices == 0 )) || (( $counter > 0 ) && ( $i == $#{$pythoncomponents} )))   # limiting to $installer::globals::maxservices files
            if ( $counter > 0 )
            {
                $filestring =~ s/\;\s*$//;
                $filestring = substr( $filestring, 0, length($filestring)-3);
                chdir($onesourcepath);

                my @regcompoutput = ();

                $systemcall = "$installer::globals::wrapcmd $$regcompfileref -register"
                . " -br " . fix_cygwin_path($$typesrdbref)
                . " -br " . fix_cygwin_path($$pyunoservicesrdbref)
                . " -r " . fix_cygwin_path($servicesfile)
                . " -c vnd.openoffice.pymodule:" . $filestring . " -l com.sun.star.loader.Python 2\>\&1 |";

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
# Iterating over all files, to find all files with the
# style UNO_COMPONENT. This can be libraries and jar files.
################################################################

sub register_all_components
{
    my ( $allvariableshashref, $servicesgid, $filesarrayref, $regcompfileref, $servicesfile, $regcomprdb, $includepatharrayref, $nativeservicesurlprefix, $javaservicesurlprefix ) = @_;

    my $registererrorflag = 0;

    my @unocomponents = ();
    my @javacomponents = ();
    my @pythoncomponents = ();

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $styles = "";
        my $regmergefile = "";
        my $registryid = "";

        if ( $onefile->{'RegistryID'} ) { $registryid = $onefile->{'RegistryID'}; }

        if ( $servicesgid ne $registryid ) { next; } # only registration for the current $servicesgid

        if ( $onefile->{'Regmergefile'} ) { $regmergefile = $onefile->{'Regmergefile'}; }

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if (( $styles =~ /\bUNO_COMPONENT\b/ ) && ( $regmergefile eq "" ))  # regmergefiles will not be registered with regcomp
        {
            my $filename = $onefile->{'Name'};

            if ( $filename =~ /\.jar\s*$/ ) # java component
            {
                push(@javacomponents, $onefile);
            }
            elsif( $filename =~ /\.py\s*$/ )    # python_component
            {
                push(@pythoncomponents, $onefile);
            }
            else                            # uno component
            {
                push(@unocomponents, $onefile);
            }
        }
    }

    $uno_error_occured = 0;
    $java_error_occured = 0;
    $python_error_occured = 0;

    if ( $#unocomponents > -1 ) { $uno_error_occured = register_unocomponents($allvariableshashref, \@unocomponents, $regcompfileref, $servicesfile, $nativeservicesurlprefix); }
    if ( $#javacomponents > -1 ) { $java_error_occured = register_javacomponents($allvariableshashref, \@javacomponents, $regcompfileref, $servicesfile, $regcomprdb, $javaservicesurlprefix); }
    if ( $#pythoncomponents > -1 ) { $python_error_occured = register_pythoncomponents(\@pythoncomponents, $regcompfileref, $servicesfile, $includepatharrayref); }

    if ( $uno_error_occured || $java_error_occured || $python_error_occured ) { $registererrorflag = 1; }

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
    my $new_ld_library_path = $ld_library_path;
    if ( $oldldlibrarypathstring ne "" ) {
        $new_ld_library_path = $new_ld_library_path . $installer::globals::pathseparator . $oldldlibrarypathstring;
    }
    if ( $ENV{'SYSTEM_MOZILLA'} && $ENV{'SYSTEM_MOZILLA'} eq "YES" &&
      (!$ENV{'WITH_OPENLDAP'} || $ENV{'WITH_OPENLDAP'} ne "YES")) {
        $new_ld_library_path = $new_ld_library_path . $installer::globals::pathseparator . $ENV{'MOZ_LIB'};
    }
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
    my $local_pathseparator = $installer::globals::pathseparator;

    if( $^O =~ /cygwin/i )
    {   # $CLASSPATH must use DOS separator even when using cygwin's perl
        $local_pathseparator = ';';
    }

    for ( my $i = 0; $i <= $#installer::globals::regcompjars; $i++ )
    {
        my $filename = $installer::globals::regcompjars[$i];

        my $jarfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 1);

        if ( $$jarfileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $filename for registering java components!", "prepare_classpath_for_java_registration"); }

        my $oldclasspathstring = "";
        if ( $ENV{'CLASSPATH'} ) { $oldclasspathstring = $ENV{'CLASSPATH'}; }
        else { $oldclasspathstring = "\."; }
        my $classpathstring = $$jarfileref . $local_pathseparator . $oldclasspathstring;
        if (( $^O =~ /cygwin/i ) && ( $ENV{'USE_SHELL'} ne "4nt" )) {
            $classpathstring =~ s/\//\\/g;      # guw.pl likes '\' in $PATH.
        }
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

sub add_path_to_pathvariable_directory
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
# Adding the path of a specified library to the path variable
# (for example msvcr70.dll). This is needed to register all
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

        if ( $sourcepath =~ /\Q$searchstring\E\s*$/ )
        {
            installer::pathanalyzer::get_path_from_fullqualifiedname(\$sourcepath);
            installer::remover::remove_ending_pathseparator(\$sourcepath);
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

    if ( $installer::globals::jrepath ne "" )
    {
        my $newpath = $installer::globals::jrepath . $installer::globals::pathseparator . $oldpath;
        $ENV{'PATH'} = $newpath;

        my $infoline = "Setting PATH to $ENV{'PATH'}\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
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

        my $systemcall = "$installer::globals::wrapcmd $regcompfile -register -s -r " . fix_cygwin_path($regcomprdb) . " -c $libfilename";

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
# Collecting all gids of the databases, that are part of
# the file definition
################################################################

sub collect_all_services_gids
{
    my ($filesarrayref) = @_;

    my @databasegids = ();
    my $error_occured = 0;
    my @error_files = ();

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];

        if ( $onefile->{'RegistryID'} )
        {
            my $databasegid = $onefile->{'RegistryID'};
            if (! installer::existence::exists_in_array($databasegid, \@databasegids)) { push(@databasegids, $databasegid); }
        }
        else
        {
            push(@error_files, $onefile->{'gid'});
            $error_occured = 1;
        }
    }

    if ( $error_occured )
    {
        my $infoline = "ERROR: Style UNO_COMPONENT is set, but no RegistryID is assigned!\n";
        push( @installer::globals::logfileinfo, $infoline);
        print $infoline;

        for ( my $j = 0; $j <= $#error_files; $j++ )
        {
            $infoline = "$error_files[$j]\n";
            push( @installer::globals::logfileinfo, $infoline);
            print $infoline;
        }

        installer::exiter::exit_program("ERROR: File defintion error.", "collect_all_services_gids");
    }

    return \@databasegids;
}

######################################################################
# All gids in $databasegids are as RegistryID assigned to files.
# For all this Registry Files a definition has to exist.
######################################################################

sub check_defintion_of_databasegids
{
    my ($databasegids, $registryfiles) = @_;

    my @registryfiles = ();

    # First check: For all assigned Registry files, a definition of
    # a file with flag STARREGISTRY has to exist.

    for ( my $i = 0; $i <= $#{$databasegids}; $i++ )
    {
        my $onegid = ${$databasegids}[$i];
        my $gid_defined = 0;

        for ( my $j = 0; $j <= $#{$registryfiles}; $j++ )
        {
            my $registrygid = ${$registryfiles}[$j]->{'gid'};

            if ( $onegid eq $registrygid )
            {
                $gid_defined = 1;
                last;
            }
        }

        if ( ! $gid_defined )
        {
            installer::exiter::exit_program("ERROR: Gid $onegid is assigned to file(s), but not defined!", "check_defintion_of_databasegids");
        }
    }

    # Second check: If there is a file defined as StarRegistry, is a file with flag UNO_COMPONENT assigned?

    for ( my $j = 0; $j <= $#{$registryfiles}; $j++ )
    {
        my $onefile = ${$registryfiles}[$j];
        my $registrygid = $onefile->{'gid'};

        my $gid_assigned = 0;

        for ( my $i = 0; $i <= $#{$databasegids}; $i++ )
        {
            my $onegid = ${$databasegids}[$i];

            if ( $onegid eq $registrygid )
            {
                $gid_assigned = 1;
                last;
            }
        }

        if ( ! $gid_assigned )
        {
            my $infoline = "Warning: $registrygid is defined with flag STARREGISTRY, but no file is assigned to the registry.\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            push(@registryfiles, $onefile);
        }
    }

    return \@registryfiles;
}

################################################################
# Some files have flag UNO_COMPONENT, but are not registered
# with regcomp. This files use the regmerge mechanism, that
# is not used in this perl-file. Therefore this files
# have to be filtered out here.
################################################################

sub filter_regmergefiles
{
    my ($unocomponentfiles) = @_;

    my @regcompfiles = ();

    for ( my $i = 0; $i <= $#{$unocomponentfiles}; $i++ )
    {
        my $onefile = ${$unocomponentfiles}[$i];
        my $regmergefile = "";

        if ( $onefile->{'Regmergefile'} ) { $regmergefile = $onefile->{'Regmergefile'}; }
        if ( $regmergefile ne "" ) { next; }

        push(@regcompfiles, $onefile);
    }

    return \@regcompfiles;
}

################################################################
# Setting defaults for Creating services.rdb file by registering all uno components
################################################################

sub set_url_prefixes
{
    my ( $registryfile ) = @_;

    my $nativeservicesurlprefix = "";
    my $javaservicesurlprefix = "";

    if ( $registryfile->{'NativeServicesURLPrefix'} ) { $nativeservicesurlprefix = $registryfile->{'NativeServicesURLPrefix'}; }
    else { $nativeservicesurlprefix = "vnd.sun.star.expand:\$ORIGIN/"; }

    if ( $registryfile->{'JavaServicesURLPrefix'} ) { $javaservicesurlprefix = $registryfile->{'JavaServicesURLPrefix'}; }
    else { $javaservicesurlprefix = "vnd.sun.star.expand:\$UNO_JAVA_COMPONENT_PATH/"; }

    return ($nativeservicesurlprefix, $javaservicesurlprefix);
}

################################################################
# Creating services.rdb file by registering all uno components
################################################################

sub create_services_rdb
{
    my ($allvariableshashref, $filesarrayref, $includepatharrayref, $languagestringref) = @_;

    # collecting all services files
    my $unocomponentfiles = installer::worker::collect_all_items_with_special_flag($filesarrayref, "UNO_COMPONENT");
    $unocomponentfiles = filter_regmergefiles($unocomponentfiles);

    if ( $#{$unocomponentfiles} > -1 )  # not empty -> at least one file with flag UNO_COMPONENT
    {
        my $databasegids = collect_all_services_gids($unocomponentfiles);

        my $registryfiles = installer::worker::collect_all_items_with_special_flag($filesarrayref, "STARREGISTRY");

        $registryfiles = check_defintion_of_databasegids($databasegids, $registryfiles);

        # Now the creation of all files with flag STARREGISTRY can begin

        for ( my $i = 0; $i <= $#{$registryfiles}; $i++ )
        {
            my $registryfile = ${$registryfiles}[$i];

            # my $servicesname = "services.rdb";
            my $servicesname = $registryfile->{'Name'};  # not unique!
            my $servicesgid = $registryfile->{'gid'};  # unique
            my $uniquedirname = $servicesgid . "_servicesrdb";
            # my $uniquedirname = $servicesgid;

            my ($nativeservicesurlprefix, $javaservicesurlprefix) = set_url_prefixes($registryfile);

            installer::logger::include_header_into_logfile("Creating $servicesname ($servicesgid):");

            # my $servicesdir = installer::systemactions::create_directories($servicesname, $languagestringref);
            my $servicesdir = installer::systemactions::create_directories($uniquedirname, $languagestringref);

            if ( $^O =~ /cygwin/i && $ENV{'USE_SHELL'} eq "4nt" )
            {      # $servicesdir is used as a parameter for regcomp and has to be DOS style
                $servicesdir = qx{cygpath -d "$servicesdir"};
                chomp($servicesdir);
                $servicesdir =~ s/\\/\//g;
            }

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

                $regcompfileref = get_source_path_cygwin_safe($searchname, $includepatharrayref, 1);
                if ( $$regcompfileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $searchname for registering uno components!", "create_services_rdb"); }

                # For Windows the libraries included into the mozruntime.zip have to be added to the path
                if ($installer::globals::iswin) { add_path_to_pathvariable_directory($filesarrayref, "mozruntime_zip"); }
                if ($installer::globals::iswin) { add_path_to_pathvariable($filesarrayref, "msvcr70.dll"); }

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

                # my $error_during_registration = register_all_components($filesarrayref, $regcompfileref, $servicesfile, $regcomprdb, $includepatharrayref);
                my $error_during_registration = register_all_components($allvariableshashref, $servicesgid, $unocomponentfiles, $regcompfileref, $servicesfile, $regcomprdb, $includepatharrayref, $nativeservicesurlprefix, $javaservicesurlprefix);

                # Dependent from the success, the registration directory can be renamed.

                if ( $error_during_registration )
                {
                    $servicesdir = installer::systemactions::rename_string_in_directory($servicesdir, "inprogress", "witherror");
                    push(@installer::globals::removedirs, $servicesdir);
                    # and exiting the packaging process
                    installer::exiter::exit_program("ERROR: Could not register all components for file $servicesname ($servicesgid)!", "create_services_rdb");
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

            # Adding the new services file source path to the filearray
            $registryfile->{'sourcepath'} = $servicesfile;  # setting the sourcepath!
            # add_services_sourcepath_into_filearray( $filesarrayref, $servicesfile, $servicesname );
        }
    }

    # Setting the global variable $installer::globals::services_rdb_created

    $installer::globals::services_rdb_created = 1;
}

1;
