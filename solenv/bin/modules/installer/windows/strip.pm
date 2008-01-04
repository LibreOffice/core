#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: strip.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: obo $ $Date: 2008-01-04 16:15:41 $
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

package installer::windows::strip;

use installer::converter;
use installer::existence;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::systemactions;

#####################################################################
# Checking whether a file has to be stripped
#####################################################################

sub need_to_strip
{
    my ( $filename ) = @_;

    my $strip = 0;

    # Check using the "nm" command

    $filename = qx{cygpath -m "$filename"};
    $filename =~ s/[\r\n]//g;

    open (FILE, "nm $filename 2>&1 |");
    my $nmoutput = <FILE>;
    close (FILE);

    if ( $nmoutput && !( $nmoutput =~ /no symbols/i || $nmoutput =~ /not recognized/i )) { $strip = 1; }

    return $strip
}

#####################################################################
# Checking whether a file has to be stripped
#####################################################################

sub do_strip
{
    my ( $filename ) = @_;

    my $systemcall = "strip" . " " . $filename;

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not strip $filename!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "SUCCESS: Stripped library $filename!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

#####################################################################
# Resolving all variables in the packagename.
#####################################################################

sub strip_binaries
{
    my ( $filelist, $languagestringref ) = @_;

    installer::logger::include_header_into_logfile("Stripping files:");

    my $strippeddirbase = installer::systemactions::create_directories("stripped", $languagestringref);

    if (! installer::existence::exists_in_array($strippeddirbase, \@installer::globals::removedirs))
    {
        push(@installer::globals::removedirs, $strippeddirbase);
    }

    for ( my $i = 0; $i <= $#{$filelist}; $i++ )
    {
        my $sourcefilename = ${$filelist}[$i]->{'sourcepath'};

        if ( need_to_strip($sourcefilename) )
        {
            my $shortfilename = $sourcefilename;
            installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$shortfilename);

            $infoline = "Strip: $shortfilename\n";
            push( @installer::globals::logfileinfo, $infoline);

            # copy file into directory for stripped libraries

            my $onelanguage = ${$filelist}[$i]->{'specificlanguage'};

            # files without language into directory "00"

            if ($onelanguage eq "") { $onelanguage = "00"; }

            my $strippeddir = $strippeddirbase . $installer::globals::separator . $onelanguage;
            installer::systemactions::create_directory($strippeddir);   # creating language specific subdirectories

            my $destfilename = $strippeddir . $installer::globals::separator . $shortfilename;
            installer::systemactions::copy_one_file($sourcefilename, $destfilename);

            # change sourcepath in files collector

            ${$filelist}[$i]->{'sourcepath'} = $destfilename;

            # strip file

            do_strip($destfilename);
        }
    }
}

1;
