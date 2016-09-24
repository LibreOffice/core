#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

package installer::windows::strip;

use File::Temp qw(tmpnam);
use installer::converter;
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

    $filename =~ s/\\/\\\\/g;

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

    if (! grep {$_ eq $strippeddirbase} @installer::globals::removedirs)
    {
        push(@installer::globals::removedirs, $strippeddirbase);
    }

    my ($tmpfilehandle, $tmpfilename) = tmpnam();
    open SOURCEPATHLIST, ">$tmpfilename" or die "oops...\n";
    for ( my $i = 0; $i <= $#{$filelist}; $i++ )
    {
        print SOURCEPATHLIST "${$filelist}[$i]->{'sourcepath'}\n";
    }
    close SOURCEPATHLIST;
    my @filetypelist = qx{file -f "$tmpfilename"};
    chomp @filetypelist;
    unlink "$tmpfilename" or die "oops\n";
    for ( my $i = 0; $i <= $#{$filelist}; $i++ )
    {
        ${$filelist}[$i]->{'is_executable'} = ( $filetypelist[$i] =~ /:.*PE executable/ );
    }

    if ( $^O =~ /cygwin/i ) { installer::worker::generate_cygwin_paths($filelist); }

    for ( my $i = 0; $i <= $#{$filelist}; $i++ )
    {
        my $sourcefilename = ${$filelist}[$i]->{'cyg_sourcepath'};

        if ( ${$filelist}[$i]->{'is_executable'} && need_to_strip($sourcefilename) )
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
