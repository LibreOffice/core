#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


package packager::work;

use packager::exiter;
use packager::existence;
use packager::files;
use packager::globals;

###########################################
# Setting global variables
###########################################

sub set_global_variable
{
    my $compiler = $ENV{'OUTPATH'};

    if ( $ENV{'PROEXT'} ) { $compiler = $compiler . $ENV{'PROEXT'}; }

    $packager::globals::compiler = $compiler;
}

#############################################################################
# Converting a string list with separator $listseparator
# into an array
#############################################################################

sub convert_stringlist_into_array
{
    my ( $includestringref, $listseparator ) = @_;

    my @newarray = ();
    my $first;
    my $last = ${$includestringref};

    while ( $last =~ /^\s*(.+?)\Q$listseparator\E(.+)\s*$/) # "$" for minimal matching
    {
        $first = $1;
        $last = $2;
        push(@newarray, "$first");
    }

    push(@newarray, "$last");

    return \@newarray;
}

###########################################
# Generating a list of package calls
# corresponding to the package list
###########################################

sub create_package_todos
{
    my ( $packagelist ) = @_;

    my @targets = ();   # only used, if the build server is not used

    for ( my $i = 0; $i <= $#{$packagelist}; $i++ )
    {
        my $line = ${$packagelist}[$i];

        if ( $line =~ /^\s*\#/ ) { next; }  # comment line

        if ( $line =~ /^\s*(\w+?)\s+(\S+?)\s+(\S+?)\s+(\w+?)\s*$/ )
        {
            my $product = $1;
            my $compilerlist = $2;
            my $languagelist = $3;
            my $target = $4;

            $product =~ s/\s//g;
            $compilerlist =~ s/\s//g;
            $languagelist =~ s/\s//g;
            $target =~ s/\s//g;

            my $compilers = convert_stringlist_into_array(\$compilerlist, ",");

            # is the compiler of this "build" part of the compiler list in pack.lst ?

            if ( packager::existence::exists_in_array($packager::globals::compiler, $compilers) )
            {
                # products are separated in pack.lst by "|"

                my $languagesets = convert_stringlist_into_array(\$languagelist, "\|");

                # now all information is available to create the targets for the systemcalls

                for ( my $j = 0; $j <= $#{$languagesets}; $j++ )
                {
                    my $languagestring = ${$languagesets}[$j];
                    $languagestring =~ s/\,/\_/g;   # comma in pack.lst becomes "_" in dmake command

                    my $target = $target . "_" . $languagestring;
                    push(@targets, $target);

                    my $insertline = $target . "\n";
                    push( @packager::globals::logfileinfo, $insertline);
                }
            }
        }
    }

    return \@targets;
}

###########################################
# Executing the generated system calls
###########################################

sub execute_system_calls
{
    my ( $targets ) = @_;

    for ( my $i = 0; $i <= $#{$targets}; $i++ )
    {
        my $systemcall = "dmake " . ${$targets}[$i];

        my $infoline = "Packager: $systemcall\n";
        print $infoline;
        push( @packager::globals::logfileinfo, $infoline);

        my $returnvalue = system($systemcall);

        $infoline = "Packager finished: $systemcall\n";
        print $infoline;
        push( @packager::globals::logfileinfo, $infoline);

        if ( $returnvalue )
        {
            $infoline = "\nERROR: Packager $systemcall\n";
            print $infoline;
            push( @packager::globals::logfileinfo, $infoline);
            if (!($packager::globals::ignoreerrors)) { packager::exiter::exit_program("ERROR: Packing not successful : $systemcall", "execute_system_calls"); }
        }
    }
}

##############################################################
# Starting the build server with the generated system calls
##############################################################

sub start_build_server
{
    my ( $targets ) = @_;

    # preparing the directory structure

    my $prj = $ENV{PRJ};                # for example "..";
    my $platform = $ENV{INPATH};        # wntmsci10.pro, unxsols4.pro
    my $platformpath = $prj . $packager::globals::separator . $platform;
    if ( ! -d $platformpath ) { packager::files::create_directory($miscpath); }
    my $miscpath = $platformpath  . $packager::globals::separator . "misc";
    if ( ! -d $miscpath ) { packager::files::create_directory($miscpath); }
    $miscpath = $miscpath  . $packager::globals::separator . "temp";
    if ( -d $miscpath ) { packager::files::remove_complete_directory($miscpath); }  # removing old files !
    if ( ! -d $miscpath ) { packager::files::create_directory($miscpath); }

    my $prjroot = ".." . $packager::globals::separator . ".." . $packager::globals::separator . ".." . $packager::globals::separator . ".."; # platform/misc/temp/uniquetempdir

    my $makefilepath = $prj . $packager::globals::separator . "util" . $packager::globals::separator . "makefile.mk";

    if ( ! $ENV{'PRJNAME'} ) { packager::exiter::exit_program("ERROR: Environment variable PRJNAME not set!", "do_broadcast"); }
    my $prjname = $ENV{PRJNAME};

    my $pkgformat = $ENV{PKGFORMAT};

    my $prjdep = $prjname . "\\" . "util";  # always windows like path
    my @targetdirs;
    my @targetlines = ();
    # iterating over all targets
    for ( my $i = 0; $i <= $#{$targets}; $i++ )
    {
        my $target = ${$targets}[$i];
        my $tempdir = $miscpath . $packager::globals::separator . $target;
        $tempdir = packager::files::create_unique_directory ($tempdir);
        @targetlines=();
        push( @targetlines, "\ngenerated_target : $target\n\n");    # to be included into the makefile.mk

        if ( defined $pkgformat ) {
            push( @targetlines, "\n$target : ".'$$@{$(PKGFORMAT:^".")}'."\n\n");    # to be included into the makefile.mk
        }

        generate_makefile($tempdir, $makefilepath, $prjroot, $target, \@targetlines);

        do_broadcast($tempdir, $prjname, $prj, $platform, $prjdep);
        push @targetdirs, $tempdir;
    }
}

##############################################################
# Generating the makefile in the temporary directory
##############################################################

sub generate_makefile
{
    my ( $tempdir, $makefilepath, $prjroot, $target, $targetlines_ref ) = @_;

    my $makefile = packager::files::read_file($makefilepath);

    my @targetlines = ();
    push( @targetlines, @{$targetlines_ref});   # to be included into the makefile.mk

    $prjroot = $prjroot . "\n";

    my $uniquename = $tempdir;
    get_filename_from_path(\$uniquename);
    $uniquename = $uniquename . "\n";

    my $counter = 0;
    my $increase = 1;

    for ( my $i = 0; $i <= $#{$makefile}; $i++ )
    {
        if ( ${$makefile}[$i] =~ /^\s*TARGET\s*=.*/ ) { ${$makefile}[$i] = "TARGET=" . $uniquename; }   # setting the new project root

        if ( ${$makefile}[$i] =~ /^\s*PRJ\s*=.*/ ) { ${$makefile}[$i] = "PRJ=" . $prjroot; }    # setting the new project root

        if ( ${$makefile}[$i] =~ /^\s*\.INCLUDE[\t ]*:[\t ]*target.mk[\t ]*$/ ) { $increase = 0; }  # no more increase of the counter

        if ( $increase ) { $counter++; }
    }

    splice(@{$makefile}, $counter, 0, @targetlines);    # including the new target lines at position $counter

    my $newmakefilepath = $tempdir . $packager::globals::separator . "makefile.mk";
    packager::files::save_file($newmakefilepath, $makefile);
}

##############################################################
# Generating the broadcasts for the build server
##############################################################

sub do_broadcast
{
    use  File::Temp;

    my ( $tempdir, $prjname, $prj, $platform, $prjdep ) = @_;

    # Syntax:  cmd_bcst -s 18 "Version;Environment;Project;Verzeichnis;Restriction[;Abhaengigkeit1][;Abhaengigkeit n]..."
    # Example: cmd_bcst -s 18 "SRC680;wntmsci10.pro;instsetoo_native;;instsetoo_native\bla1;instsetoo_native\util"

    if ( ! $ENV{'WORK_STAMP'} ) { packager::exiter::exit_program("ERROR: Environment variable WORK_STAMP not set!", "do_broadcast"); }
    my $workstamp = $ENV{WORK_STAMP};
    my $cwsworkstamp = $ENV{CWS_WORK_STAMP};

    my $prjdir = $tempdir;
    $prjdir =~ s/$prj/$prjname/;
    $prjdir =~ s/\//\\/g;                   # convert to windows path syntax

    my $tempfiledir = $ENV{TMP};
    $tempfiledir = $tempdir if ( ! defined $tempfiledir );
    my ( $tmpfile_handle, $tmpfile_name ) = mkstemp( $tempfiledir . $packager::globals::separator . "packagerXXXXX");
    if ( ! $tmpfile_handle ) {
        packager::exiter::exit_program("ERROR: Couldn't open temporary file \"$tmpfile_name\"!", "do_broadcast");
    }
    if (defined($cwsworkstamp)) {
        print $tmpfile_handle "\"$cwsworkstamp;$platform;$prjname;$prjdir;nobase;$prjdep\"";
        print "to tmpfile: \"$cwsworkstamp;$platform;$prjname;$prjdir;nobase;$prjdep\"\n";
    }
    else {
        print $tmpfile_handle "\"$workstamp;$platform;$prjname;$prjdir;nobase;$prjdep\"";
        print "to tmpfile: \"$workstamp;$platform;$prjname;$prjdir;nobase;$prjdep\"\n";
    }
    close $tmpfile_handle;
    my $returnvalue = system("cmd_bcst -s 18 \@$tmpfile_name");
    print "cmd_bcst -s 18 \@$tmpfile_name\n";
    unlink "$tmpfile_name";

    if ( $returnvalue ) # an error occurred
    {
        if (!($packager::globals::ignoreerrors)) { packager::exiter::exit_program("ERROR: Packing not successful!", "do_broadcast"); }
    }
}

##############################################################
# Returning the name of file or directory from complete path
##############################################################

sub get_filename_from_path
{
    my ($longfilenameref) = @_;

    if ( $packager::globals::isunix )
    {
        if ( $$longfilenameref =~ /^.*\/(\S.+\S?)/ )
        {
            $$longfilenameref = $1;
        }
    }

    if ( $packager::globals::iswin )
    {
        if ( $$longfilenameref =~ /^.*\\(\S.+\S?)/ )
        {
            $$longfilenameref = $1;
        }
    }
}

1;
