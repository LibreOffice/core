#*************************************************************************
#
#   $RCSfile: work.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:21:41 $
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

use packager::existence;
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

    my @calls = ();

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

                # now all information is available to create the systemcalls

                for ( my $j = 0; $j <= $#{$languagesets}; $j++ )
                {
                    my $languagestring = ${$languagesets}[$j];
                    $languagestring =~ s/\,/\_/g;   # comma in pack.lst becomes "_" in dmake command

                    my $systemcall = "dmake " . $target . "_" . $languagestring;
                    push(@calls, $systemcall);

                    my $insertline = $systemcall . "\n";
                    push( @packager::globals::logfileinfo, $insertline);
                }
            }
        }
    }

    return \@calls;
}

###########################################
# Executing the generated system calls
###########################################

sub execute_system_calls
{
    my ( $calls ) = @_;

    for ( my $i = 0; $i <= $#{$calls}; $i++ )
    {
        my $systemcall = ${$calls}[$i];

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
        }
    }
}

1;
