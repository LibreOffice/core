#*************************************************************************
#
#   $RCSfile: control.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: svesik $ $Date: 2004-04-20 12:25:37 $
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

package installer::control;

use Cwd;
use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::scriptitems;

#########################################################
# Checking the local system
# Checking existence of needed files in include path
#########################################################

sub check_system_path
{
    # The following files have to be found in the environment variable PATH
    # All platforms: zip, unzip
    # Windows only: msvcp70.dll, msvcr70.dll for regcomp.exe
    # Windows only: "msiinfo.exe", "msidb.exe", "uuidgen.exe", "makecab.exe", "msitran.exe" for msi database and packaging

    my $onefile;
    my $error = 0;
    my $pathvariable = $ENV{'PATH'};

    my $patharrayref = installer::converter::convert_stringlist_into_array(\$pathvariable, $installer::globals::pathseparator);

    my @needed_files_in_path = ();

    if (($installer::globals::iswin) && ($installer::globals::iswindowsbuild))
    {
        @needed_files_in_path = ("zip.exe", "unzip.exe", "msvcp70.dll", "msvcr70.dll", "msiinfo.exe", "msidb.exe", "uuidgen.exe", "makecab.exe", "msitran.exe");
    }
    elsif ($installer::globals::iswin)
    {
        @needed_files_in_path = ("zip.exe", "unzip.exe");
    }
    else
    {
        @needed_files_in_path = ("zip", "unzip");
    }

    foreach $onefile ( @needed_files_in_path )
    {
        print "...... seaching $onefile ...";

        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$onefile, $patharrayref , 0);

        if ( $$fileref eq "" )
        {
            $error = 1;
            print "ERROR: Not found\n";
        }
        else
        {
            print "\tFound: $$fileref\n";
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

        print "...... $key ...";

        if ( $value eq "" )
        {
            print "\tERROR: Not set\n";
            $error = 1;
        }
        else
        {
            print "\tSET\n";
        }
    }

    if ( $error )
    {
        installer::exiter::exit_program("ERROR: Environment variable not set!", "check_system_environment");
    }

    return \%variables;
}

#############################################################
# Reading the packagelist
#############################################################

sub read_packagelist
{
    my ($packagelistname) = @_;

    my @packages = ();

    my $packagelist = installer::files::read_file($packagelistname);

    for ( my $i = 0; $i <= $#{$packagelist}; $i++ )
    {
        my $line = ${$packagelist}[$i];

        if ( $line =~ /^\s*\#/ ) { next; }  # this is a comment line

        if ( $line =~ /^\s*Start\s*$/i )    # a new package definition
        {
            my %onepackage = ();

            my $counter = $i + 1;

            while (!( ${$packagelist}[$counter] =~ /^\s*End\s*$/i ))
            {
                if ( ${$packagelist}[$counter] =~ /^\s*(\S+)\s*\=\s*\"(.*)\"/ )
                {
                    my $key = $1;
                    my $value = $2;
                    $onepackage{$key} = $value;
                }

                $counter++;
            }

            push(@packages, \%onepackage);
        }
    }

    return \@packages;
}

##################################################################
# Controlling the content of the packagelist
# 1. Items in @installer::globals::packagelistitems must exist
# 2. If a shellscript file is defined, it must exist
##################################################################

sub check_packagelist
{
    my ($packages) = @_;

    for ( my $i = 0; $i <= $#{$packages}; $i++ )
    {
        my $onepackage = ${$packages}[$i];

        my $element;

        # checking all items that must be defined

        foreach $element (@installer::globals::packagelistitems)
        {
            my $value = "";

            if ( $onepackage->{$element} )
            {
                $value = $onepackage->{$element};
            }

            # Checking the value. All values must not be empty.

            if ( $value eq "" )
            {
                print "\n*********************************************************************\n";
                print "ERROR in package list: No value for $element !";
                print "\n*********************************************************************\n";
                usage();
                exit(-1);
            }
        }

        # checking the existence of the script file, if defined

        if ( $onepackage->{'script'} )
        {
            installer::files::check_file($onepackage->{'script'});
        }
    }
}

#############################################################
# Controlling the log file at the end of the
# packaging process
#############################################################

sub check_logfile
{
    my ($logfile) = @_;

    my @errors = ();
    my @output = ();
    my $contains_error = 0;

    for ( my $i = 0; $i <= $#{$logfile}; $i++ )
    {
        my $line = ${$logfile}[$i];

        # Errors are all errors, but not the Windows installer table "Error.idt"

        my $compareline = $line;
        $compareline =~ s/Error\.idt//g;    # removing all occurences of "Error.idt"

        if ( $compareline =~ /\bError\b/i )
        {
            $contains_error = 1;
            push(@errors, $line);
        }
    }

    if ($contains_error)
    {
        my $line = "\n*********************************************************************\n";
        push(@output, $line);
        $line = "ERROR: The following errors occured in packaging process:\n\n";
        push(@output, $line);

        for ( my $i = 0; $i <= $#errors; $i++ )
        {
            $line = "$errors[$i]";
            push(@output, $line);
        }

        $line = "*********************************************************************\n";
        push(@output, $line);
#       exit(-1);
    }
    else
    {
        my $line = "\n***********************************************************\n";
        push(@output, $line);
        $line = "SUCCESS: No errors in packaging process!\n";
        push(@output, $line);
        $line = "***********************************************************\n";
        push(@output, $line);
    }

    # printing the output file and adding it to the logfile

    installer::logger::include_header_into_logfile("Summary:");

    for ( my $i = 0; $i <= $#output; $i++ )
    {
        my $line = "$output[$i]";
        print "$line";
        push( @installer::globals::logfileinfo, $line);
    }


}

1;
