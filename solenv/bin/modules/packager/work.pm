#*************************************************************************
#
#   $RCSfile: work.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-07-29 15:15:26 $
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

        generate_makefile($tempdir, $makefilepath, $prjroot, $target, \@targetlines);

        sleep 5;
        do_broadcast($tempdir, $prjname, $prj, $platform, $prjdep);
        push @targetdirs, $tempdir;
    }
    @targetlines = ();
    if ( defined $ENV{USE_SHELL} && $ENV{USE_SHELL} eq "4nt" ) {
        push( @targetlines, "RMRECURSIVEFLAGS=/sxyz\n" );
    } else {
        push( @targetlines, "RMRECURSIVEFLAGS=-rf\n" );
    }
    push( @targetlines, "\ngenerated_target :\n");  # to be included into the makefile.mk
    my @depdirs = @targetdirs;
    foreach my $i (@targetdirs) {
        $i =~ s/.*?([\w-]+)$/\.\.\$\/$1/;
        push( @targetlines, "\t+\$(RM) \$(RMRECURSIVEFLAGS) $i\n"); # to be included into the makefile.mk
    }
    push( @targetlines, "\n" );
    $target = "cleanup";
    $tempdir = $miscpath . $packager::globals::separator . $target;
    $tempdir = packager::files::create_unique_directory ($tempdir);
    generate_makefile($tempdir, $makefilepath, $prjroot, $target, \@targetlines);
    $prjdep = join ";", @depdirs;
    $prjdep =~ s/\$\//\\/g;
    $prjdep =~ s/\//\\/g;
    $prjdep =~ s/\.\.[\.\\]+/$prjname\\/g;
    do_broadcast($tempdir, $prjname, $prj, $platform, $prjdep);
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
    # Example: cmd_bcst -s 18 "SRC680;wntmsci10.pro;instsetoo_native;instsetoo_native\bla1;instsetoo_native\util"

    if ( ! $ENV{'WORK_STAMP'} ) { packager::exiter::exit_program("ERROR: Environment variable WORK_STAMP not set!", "do_broadcast"); }
    my $workstamp = $ENV{WORK_STAMP};

    my $prjdir = $tempdir;
    $prjdir =~ s/$prj/$prjname/;
    $prjdir =~ s/\//\\/g;                   # convert to windows path syntax

    my $tempfiledir = $ENV{TMP};
    $tempfiledir = $tempdir if ( ! defined $tempfiledir );
    my ( $tmpfile_handle, $tmpfile_name ) = mkstemp( $tempfiledir . $packager::globals::separator . "packagerXXXXX");
    if ( ! $tmpfile_handle ) {
        packager::exiter::exit_program("ERROR: Couldn't open temporary file \"$tmpfile_name\"!", "do_broadcast");
    }
    print $tmpfile_handle "\"$workstamp;$platform;$prjname;$prjdir;;$prjdep\"";
    print "to tmpfile: \"$workstamp;$platform;$prjname;$prjdir;;$prjdep\"\n";
    close $tmpfile_handle;
    my $returnvalue = system("cmd_bcst -s 18 \@$tmpfile_name");
    print "cmd_bcst -s 18 \@$tmpfile_name\n";
    unlink "$tmpfile_name";

    if ( $returnvalue ) # an error occured
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
