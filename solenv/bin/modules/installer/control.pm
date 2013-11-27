#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



package installer::control;

use Cwd;
use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::pathanalyzer;
use installer::scriptitems;
use installer::systemactions;

our @ErrorMessages = undef;

#########################################################
# Function that can be used for additional controls.
# Search happens in $installer::globals::patharray.
#########################################################

sub check_needed_files_in_path
{
    my ( $filesref ) = @_;

    my $error = 0;
    foreach my $onefile ( @{$filesref} )
    {
        $installer::logger::Info->printf("...... searching %s ...\n", $onefile);

        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$onefile, $installer::globals::patharray , 0);

        if ( $$fileref eq "" )
        {
            $error = 1;
            installer::logger::print_error( "$onefile not found\n" );
        }
        else
        {
            $installer::logger::Info->print( "\tFound: $$fileref\n" );
        }
    }

    if ( $error )
    {
        installer::exiter::exit_program("ERROR: Could not find all needed files in path!", "check_needed_files_in_path");
    }
}

#########################################################
# Checking the local system
# Checking existence of needed files in include path
#########################################################

sub check_system_path
{
    # The following files have to be found in the environment variable PATH
    # All platforms: zip
    # Windows only: "msiinfo.exe", "msidb.exe", "uuidgen.exe", "makecab.exe", "msitran.exe", "expand.exe" for msi database and packaging

    my $onefile;
    my $error = 0;
    my $pathvariable = $ENV{'PATH'};
    my $local_pathseparator = $installer::globals::pathseparator;

    if( $^O =~ /cygwin/i )
    {   # When using cygwin's perl the PATH variable is POSIX style and ...
        $pathvariable = qx{cygpath -mp "$pathvariable"} ;
        # has to be converted to DOS style for further use.
        $local_pathseparator = ';';
    }
    if( $^O =~ /os2/i )
    {
        # has to be converted to DOS style for further use.
        $local_pathseparator = ';';
    }
    my $patharrayref = installer::converter::convert_stringlist_into_array(\$pathvariable, $local_pathseparator);

    $installer::globals::patharray = $patharrayref;

    my @needed_files_in_path = ();

    if (($installer::globals::iswin) && ($installer::globals::iswindowsbuild))
    {
        @needed_files_in_path = ("zip.exe", "msiinfo.exe", "msidb.exe", "uuidgen.exe", "makecab.exe", "msitran.exe", "expand.exe");
    }
    elsif ($installer::globals::iswin || $installer::globals::isos2)
    {
        @needed_files_in_path = ("zip.exe");
    }
    else
    {
        @needed_files_in_path = ("zip");
    }

    foreach $onefile ( @needed_files_in_path )
    {
        $installer::logger::Info->printf("...... searching %s ...\n", $onefile);

        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$onefile, $patharrayref , 0);

        if ( $$fileref eq "" )
        {
            $error = 1;
            installer::logger::print_error( "$onefile not found\n" );
        }
        else
        {
            $installer::logger::Info->print( "\tFound: $$fileref\n" );
            # Saving the absolut path for msitran.exe. This is required for the determination of the checksum.
            if ( $onefile eq "msitran.exe" ) { $installer::globals::msitranpath = $$fileref; }
        }
    }

    if ( $error )
    {
        installer::exiter::exit_program("ERROR: Could not find all needed files in path!", "check_system_path");
    }

    # checking for epm, which has to be in the path or in the solver

    if (( $installer::globals::call_epm ) && (!($installer::globals::iswindowsbuild)))
    {
        my $onefile = "epm";
        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$onefile, $patharrayref , 0);
        if (!( $$fileref eq "" ))
        {
            $installer::globals::epm_in_path = 1;

            if ( $$fileref =~ /^\s*\.\/epm\s*$/ )
            {
                my $currentdir = cwd();
                $$fileref =~ s/\./$currentdir/;
            }

            $installer::globals::epm_path = $$fileref;
        }
    }

    # checking, if upx can be found in path

    if ( $installer::globals::iswindowsbuild ) { $installer::globals::upxfile = "upx.exe"; }
    else { $installer::globals::upxfile = "upx"; }

    my $upxfilename = $installer::globals::upxfile;
    my $upxfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$upxfilename, $patharrayref , 0);

    if (!( $$upxfileref eq "" ))
    {
        $installer::globals::upx_in_path = 1;
        $installer::globals::upxfile = $$upxfileref;
        $installer::logger::Info->print( "\tFound: $$upxfileref\n" );
    }

}

######################################################################
# Determining the version of file makecab.exe
######################################################################

sub get_makecab_version
{
    my $makecabversion = -1;

    my $systemcall = "makecab.exe |";
    my @makecaboutput = ();

    open (CAB, $systemcall);
    while (<CAB>) { push(@makecaboutput, $_); }
    close (CAB);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    if ($returnvalue)
    {
        $installer::logger::Global->printf("ERROR: Could not execute \"%s\"!\n", $systemcall);
    }
    else
    {
        $installer::logger::Global->printf("Success: Executed \"%s\" successfully!\n", $systemcall);

        my $versionline = "";

        for ( my $i = 0; $i <= $#makecaboutput; $i++ )
        {
            if ( $makecaboutput[$i] =~ /\bVersion\b/i )
            {
                $versionline = $makecaboutput[$i];
                last;
            }
        }

        $installer::logger::Global->printf("%s\n", $versionline);

        if ( $versionline =~ /\bVersion\b\s+(\d+[\d\.]+\d+)\s+/ )
        {
            $makecabversion = $1;
        }

        # Only using the first number

        if ( $makecabversion =~ /^\s*(\d+?)\D*/ )
        {
            $makecabversion = $1;
        }

        $installer::logger::Global->printf("Using version: %s\n", $makecabversion);
    }

    return $makecabversion;
}

######################################################################
# Checking the version of file makecab.exe
######################################################################

sub check_makecab_version
{
    # checking version of makecab.exe
    # Now it is guaranteed, that makecab.exe is in the path

    my $do_check = 1;

    my $makecabversion = get_makecab_version();

    $installer::logger::Global->printf("Tested version: %s\n", $installer::globals::controlledmakecabversion);

    if ( $makecabversion < 0 ) { $do_check = 0; } # version could not be determined

    if ( $do_check )
    {
        if ( $makecabversion < $installer::globals::controlledmakecabversion )
        {
            # warning for OOo, error for inhouse products
            if ( $installer::globals::isopensourceproduct )
            {
                installer::logger::print_warning("Old version of makecab.exe. Found version: \"$makecabversion\", tested version: \"$installer::globals::controlledmakecabversion\"!\n");
            }
            else
            {
                installer::exiter::exit_program("makecab.exe too old. Found version: \"$makecabversion\", required version: \"$installer::globals::controlledmakecabversion\"!", "check_makecab_version");
            }
        }
    }
    else
    {
        $installer::logger::Global->print("Warning: No version check of makecab.exe\n");
    }
}

######################################################################
# Reading the environment variables for the pathes in ziplist.
# solarpath, solarenvpath, solarcommonpath, os, osdef, pmiscpath
######################################################################

sub check_system_environment
{
    my %variables = ();
    my $key;
    my $error = 0;

    foreach $key ( @installer::globals::environmentvariables )
    {
        my $value = "";
        if ( $ENV{$key} ) { $value = $ENV{$key}; }
        $variables{$key} = $value;

        if ( $value eq "" )
        {
            installer::logger::print_error( "$key not set in environment\n" );
            $error = 1;
        }
    }

    if ( $error )
    {
        installer::exiter::exit_program("ERROR: Environment variable not set!", "check_system_environment");
    }

    return \%variables;
}


sub prepare_error_processing ()
{
    @ErrorMessages = ();
}

=item filter_log_error ($relative_time, $log_id, $process_id, $message)

    Process the given log message.  Returns $message unaltered.

=cut
sub filter_log_error ($$$$)
{
    my ($relative_time, $log_id, $process_id, $message) = @_;

    if ($message =~ /\berror\b/i)
    {
        # Message contains the word "error".  Now we have to find out if it is relevant.

        # Remove all filenames that contain the word "Error".
        my $work_string = $message;
        $work_string =~ s/Error\.(idt|mlf|ulf|idl|html|hpp|ipp)//g;

        if ($work_string =~ /\bError\b/i)
        {
            # This really is an error message.
            push @ErrorMessages, {'relative_time' => $relative_time,
                                  'message' => $message};
        }
    }

    return $message;
}




sub printocessed_error_lines ()
{
    my $lines = [];

    foreach my $line (@ErrorMessages)
    {
        push @$lines, sprintf("    %12.6f : %s", $line->{'relative_time'}, $line->{'message'});
    }

    return $lines;
}




=item check_logfile()

    Print all error messages (typically at the end) on the console.

=cut
sub check_logfile ()
{
    my ($logfile) = @_;

    my @errors = ();
    my @output = ();

    my $ignore_errors = ( ! $installer::globals::pro ) && ( $installer::globals::ignore_error_in_logfile );
    my $contains_errors = scalar @ErrorMessages > 0;

    # Format errors
    if ($contains_errors)
    {
        push(@output, "\n");
        push(@output, "*********************************************************************\n");
        if ($ignore_errors)
        {
            push(@output, "The following errors in the log file were ignored:\n");
        }
        else
        {
            push(@output, "ERROR: The following errors occured in packaging process:\n");
        }
        push(@output, "\n");

        foreach my $line (@ErrorMessages)
        {
            push @output, sprintf("    %12.6f : %s", $line->{'relative_time'}, $line->{'message'});
        }

        push(@output, "*********************************************************************\n");
    }

    # Claim success if there where no errors or if errors are treated as warnings.
    if ( ! $contains_errors || $ignore_errors)
    {
        push(@output, "\n");
        push(@output, "***********************************************************\n");
        push(@output, "Successful packaging process!\n");
        push(@output, "***********************************************************\n");
    }

    # Print the summary.
    installer::logger::include_header_into_logfile("Summary:");
    my $force = 1; # print this message even in 'quiet' mode
    foreach my $line (@output)
    {
        $installer::logger::Info->print($line, $force);
    }

    # Delete the accumulated error messages.  The @ErrorMessages will now contain
    # lines caused by printing those error messages.
    @ErrorMessages = ();

    @installer::globals::errorlogfileinfo = @output;

    return $contains_error && ! $ignore_error;
}

#############################################################
# Determining the ship installation directory
#############################################################

sub determine_ship_directory
{
    my ($languagesref) = @_;

    if (!( $ENV{'SHIPDRIVE'} )) { installer::exiter::exit_program("ERROR: SHIPDRIVE must be set for updater!", "determine_ship_directory"); }

    my $shipdrive = $ENV{'SHIPDRIVE'};

    my $languagestring = $$languagesref;

    if (length($languagestring) > $installer::globals::max_lang_length )
    {
        my $number_of_languages = installer::systemactions::get_number_of_langs($languagestring);
        chomp(my $shorter = `echo $languagestring | md5sum | sed -e "s/ .*//g"`);
        # $languagestring = $shorter;
        my $id = substr($shorter, 0, 8); # taking only the first 8 digits
        $languagestring = "lang_" . $number_of_languages . "_id_" . $id;
    }

    my $productstring = $installer::globals::product;
    my $productsubdir = "";

    if ( $productstring =~ /^\s*(.+?)\_\_(.+?)\s*$/ )
    {
        $productstring = $1;
        $productsubdir = $2;
    }

    if ( $installer::globals::languagepack ) { $productstring = $productstring . "_languagepack"; }
    if ( $installer::globals::patch ) { $productstring = $productstring . "_patch"; }

    my $destdir = $shipdrive . $installer::globals::separator . $installer::globals::compiler .
                $installer::globals::productextension . $installer::globals::separator .
                $productstring . $installer::globals::separator;

    if ( $productsubdir ) { $destdir = $destdir . $productsubdir . $installer::globals::separator; }

    $destdir = $destdir . $installer::globals::installertypedir . $installer::globals::separator .
                $installer::globals::build . "_" . $installer::globals::lastminor . "_" .
                "native_inprogress-number_" . $languagestring . "\." . $installer::globals::buildid;

    $installer::logger::Global->print("\n");
    $installer::logger::Global->printf("Setting ship directory: %s\n", $destdir);

    return $destdir;
}

#############################################################
# Controlling if this is an official RE pack process
#############################################################

sub check_updatepack
{
    my $shipdrive = "";
    my $filename = "";
    my $infoline = "";

    if ( $ENV{'UPDATER'} )  # the environment variable UPDATER has to be set
    {
        $installer::logger::Global->print("\n");
        $installer::logger::Global->print("Environment variable UPDATER set\n");

        if ( ! $ENV{'CWS_WORK_STAMP'} ) # the environment variable CWS_WORK_STAMP must not be set (set only in CWS)
        {
            $installer::logger::Global->print("Environment variable CWS_WORK_STAMP not set\n");

            if ( $ENV{'SHIPDRIVE'} )    # the environment variable SHIPDRIVE must be set
            {
                $shipdrive = $ENV{'SHIPDRIVE'};
                $installer::logger::Global->printf("Ship drive defined: %s\n", $shipdrive);

                if ( -d $shipdrive )    # SHIPDRIVE must be a directory
                {
                    $installer::logger::Global->print("Ship drive exists\n");

                    # try to write into $shipdrive

                    my $directory = $installer::globals::product . "_" . $installer::globals::compiler . "_" . $installer::globals::buildid . "_" . $installer::globals::languageproduct . "_test_$$";
                    $directory =~ s/\,/\_/g;    # for the list of languages
                    $directory =~ s/\-/\_/g;    # for en-US, pt-BR, ...
                    $directory = $shipdrive . $installer::globals::separator . $directory;

                    $installer::logger::Global->printf("Try to create directory: %s\n", $directory);

                    # saving this directory for later removal
                    $installer::globals::shiptestdirectory = $directory;

                    if ( installer::systemactions::try_to_create_directory($directory))
                    {
                        $installer::logger::Global->print("Write access on Ship drive\n");
                        $installer::logger::Global->print(
                            "Ship test directory %s was successfully created\n",
                            $installer::globals::shiptestdirectory);
                        my $systemcall = "rmdir $directory";
                        my $returnvalue = system($systemcall);

                        # 5th condition: No local build environment.
                        # In this case the content of SOLARENV starts with the content of SOL_TMP

                        my $solarenv = "";
                        my $sol_tmp;
                        if ( $ENV{'SOLARENV'} ) { $solarenv = $ENV{'SOLARENV'}; }

                        $installer::logger::Global->printf("Environment variable SOLARENV: %s\n", $solarenv);

                        if ( $ENV{'SOL_TMP'} )
                        {
                            $sol_tmp = $ENV{'SOL_TMP'};
                            $infoline = "Environment variable SOL_TMP: $sol_tmp\n";
                        } else {
                            $infoline = "Environment variable SOL_TMP not set\n";
                        }
                        $installer::logger::Global->print($infoline);

                        if ( defined $sol_tmp && ( $solarenv =~ /^\s*\Q$sol_tmp\E/ ))
                        {
                            $installer::logger::Global->print("Content of SOLARENV starts with the content of SOL_TMP\: Local environment -\> No Updatepack\n");
                        }
                        else
                        {
                            $installer::logger::Global->print("Content of SOLARENV does not start with the content of SOL_TMP: No local environment\n");

                            $installer::globals::updatepack = 1;    # That's it
                        }

                        # Additional logging information for the temporary ship directory

                        if ( -d $installer::globals::shiptestdirectory )
                        {
                            $installer::logger::Global->printf(
                                "Ship test directory %s still exists. Trying removal later again.\n",
                                $installer::globals::shiptestdirectory);
                        }
                        else
                        {
                            $installer::logger::Global->printf(
                                "Ship test directory %s was successfully removed.\n",
                                $installer::globals::shiptestdirectory);
                        }
                    }
                    else
                    {
                        $installer::logger::Global->print("No write access on Ship drive\n");
                        $installer::logger::Global->printf("Failed to create directory \n", $directory);
                        if ( defined $ENV{'BSCLIENT'} && ( uc $ENV{'BSCLIENT'} eq 'TRUE' ) )
                        {
                            installer::exiter::exit_program("ERROR: No write access to SHIPDRIVE allthough BSCLIENT is set.", "check_updatepack");
                        }
                    }
                }
                else
                {
                    $installer::logger::Global->print("Ship drive not found: No updatepack\n");
                }
            }
            else
            {
                $installer::logger::Global->print("Environment variable SHIPDRIVE not set: No updatepack\n");
            }
        }
        else
        {
            $installer::logger::Global->print("Environment variable CWS_WORK_STAMP defined: No updatepack\n");
        }
    }

    if ( $installer::globals::updatepack )
    {
        $installer::logger::Global->print("Setting updatepack true\n");
        $installer::logger::Global->print("\n");
    }
    else
    {
        $installer::logger::Global->print("\n");
        $installer::logger::Global->print("No updatepack\n");
    }

}

#############################################################
# Reading the Windows list file for language encodings
#############################################################

sub read_encodinglist
{
    my ($patharrayref) = @_;

    my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$installer::globals::encodinglistname, $patharrayref , 0);

    if ( $$fileref eq "" ) { installer::exiter::exit_program("ERROR: Did not find Windows encoding list $installer::globals::encodinglistname!", "read_encodinglist"); }

    $installer::logger::Global->printf("Found encoding file: %s\n", $$fileref);

    my $encodinglist = installer::files::read_file($$fileref);

    my %msiencoding = ();
    my %msilanguage = ();

    # Controlling the encoding list

    for ( my $i = 0; $i <= $#{$encodinglist}; $i++ )
    {
        my $line = ${$encodinglist}[$i];

        if ( $line =~ /^\s*\#/ ) { next; }  # this is a comment line

        if ( $line =~ /^(.*?)(\#.*)$/ ) { $line = $1; } # removing comments after "#"

        if ( $line =~ /^\s*([\w-]+)\s*(\d+)\s*(\d+)\s*$/ )
        {
            my $onelanguage = $1;
            my $codepage = $2;
            my $windowslanguage = $3;

            $msiencoding{$onelanguage} = $codepage;
            $msilanguage{$onelanguage} = $windowslanguage;
        }
        else
        {
            installer::exiter::exit_program("ERROR: Wrong syntax in Windows encoding list $installer::globals::encodinglistname : en-US 1252 1033 !", "read_encodinglist");
        }
    }

    $installer::globals::msiencoding = \%msiencoding;
    $installer::globals::msilanguage = \%msilanguage;

    # my $key;
    # foreach $key (keys %{$installer::globals::msiencoding}) { print "A Key: $key : Value: $installer::globals::msiencoding->{$key}\n"; }
    # foreach $key (keys %{$installer::globals::msilanguage}) { print "B Key: $key : Value: $installer::globals::msilanguage->{$key}\n"; }

}

#############################################################
# Only for Windows and Linux (RPM)there is currently
# a reliable mechanism to register extensions during
# installation process. Therefore it is for all other
# platforms forbidden to install oxt files into that
# directory, in which they are searched for registration.
#############################################################

sub check_oxtfiles
{
    my ( $filesarray ) = @_;

    for ( my $i = 0; $i <= $#{$filesarray}; $i++ )
    {
        my $onefile = ${$filesarray}[$i];

        if (( $onefile->{'Name'} ) && ( $onefile->{'Dir'} ))
        {
            if (( $onefile->{'Name'} =~ /\.oxt\s*$/ ) && ( $onefile->{'Dir'} eq $installer::globals::extensioninstalldir ))
            {
                installer::exiter::exit_program("There is currently only for Linux (RPM) and Windows a reliable mechanism to register extensions during installation.\nPlease remove file \"$onefile->{'gid'}\" from your installation set!\nYou can use \"\#ifdef WNT\" and \"\#ifdef LINUX\" in scp.", "check_oxtfiles");
            }
        }
    }
}

#############################################################
# Check if Java is available to create xpd installer
#############################################################

sub check_java_for_xpd
{
    my ( $allvariables ) = @_;

    if ( ! $installer::globals::solarjavaset ) { $allvariables->{'XPDINSTALLER'} = 0; }
}

####################################################################
# Setting global variable "$installer::globals::addchildprojects"
####################################################################

sub set_addchildprojects
{
    my ($allvariables) = @_;

    if (( $allvariables->{'JAVAPRODUCT'} ) ||
        ( $allvariables->{'ADAPRODUCT'} ) ||
        ( $allvariables->{'UREPRODUCT'} ) ||
        ( $allvariables->{'ADDREQUIREDPACKAGES'} )) { $installer::globals::addchildprojects = 1; }

    if ( $installer::globals::patch )
    {
        $installer::globals::addchildprojects = 0;  # no child projects for patches
    }

    $installer::logger::Global->printf(
        "Value of \$installer::globals::addchildprojects: %s\n",
        $installer::globals::addchildprojects);
}

####################################################################
# Setting global variable "$installer::globals::addjavainstaller"
####################################################################

sub set_addjavainstaller
{
    my ($allvariables) = @_;

    if ( $allvariables->{'JAVAINSTALLER'} ) { $installer::globals::addjavainstaller = 1; }

    if ( $installer::globals::patch ) { $installer::globals::addjavainstaller = 0; }
    if ( $installer::globals::languagepack ) { $installer::globals::addjavainstaller = 0; }
    if ( $allvariableshashref->{'XPDINSTALLER'} ) { $installer::globals::addjavainstaller = 0; }

    $installer::logger::Global->printf(
        "Value of \$installer::globals::addjavainstaller: %s\n",
        $installer::globals::addjavainstaller);
}

#######################################################################
# Setting global variable "$installer::globals::addsystemintegration"
#######################################################################

sub set_addsystemintegration
{
    my ($allvariables) = @_;

    if ( $allvariables->{'ADDSYSTEMINTEGRATION'} ) { $installer::globals::addsystemintegration = 1; }

    if ( $installer::globals::patch ) { $installer::globals::addsystemintegration = 0; }
    if ( $installer::globals::languagepack ) { $installer::globals::addsystemintegration = 0; }
    if (( $installer::globals::packageformat eq "native" ) || ( $installer::globals::packageformat eq "portable" )) { $installer::globals::addsystemintegration = 0; }

    $installer::logger::Global->printf(
        "Value of \$installer::globals::addsystemintegration: %s\n",
        $installer::globals::addsystemintegration);
}

1;
