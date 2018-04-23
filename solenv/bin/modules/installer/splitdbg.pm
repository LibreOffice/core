#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

package installer::splitdbg;

use strict;
use warnings;

use base 'Exporter';

use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::systemactions;

our @EXPORT_OK = qw(splitdbg_libraries);

sub _need_to_splitdbg
{
    my ( $filename ) = @_;

    my $splitdbg = 0;

    # Check using the "file" command

    $filename =~ s/'/'\\''/g;
    open (FILE, "file '$filename' |");
    my $fileoutput = <FILE>;
    close (FILE);

    if (( $fileoutput =~ /not stripped/i ) && ( $fileoutput =~ /\bELF\b/ )) { $splitdbg = 1; }

    return $splitdbg;
}

sub _do_splitdbg
{
    my ( $filename ) = @_;

    my $systemcall = "objcopy --only-keep-debug $filename $filename.dbg && ";
    $systemcall .= "objcopy --strip-debug $filename && ";
    $systemcall .= "objcopy --add-gnu-debuglink=$filename.dbg $filename";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not split debug info of $filename!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "SUCCESS: Split debug info of $filename!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

sub splitdbg_libraries
{
    my ( $filelist, $languagestringref ) = @_;

    my @debugfilelist = ();

    installer::logger::include_header_into_logfile("Splitting debuginfo from files:");

    my $splitdbgdirbase = installer::systemactions::create_directories("splitdbg", $languagestringref);

    if (! grep {$_ eq $splitdbgdirbase} @installer::globals::removedirs)
    {
        push(@installer::globals::removedirs, $splitdbgdirbase);
    }

    for ( my $i = 0; $i <= $#{$filelist}; $i++ )
    {
        my $sourcefilename = ${$filelist}[$i]->{'sourcepath'};

        if ( _need_to_splitdbg($sourcefilename) )
        {
            my $shortfilename = $sourcefilename;
            installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$shortfilename);

            my $infoline = "Split debuginfo: $shortfilename\n";
            push(@installer::globals::logfileinfo, $infoline);

            # copy file into directory for split debuginfo libraries

            my $onelanguage = ${$filelist}[$i]->{'specificlanguage'};

            # files without language into directory "00"

            if ($onelanguage eq "") { $onelanguage = "00"; }

            my $splitdbgdir = $splitdbgdirbase . $installer::globals::separator . $onelanguage;
            installer::systemactions::create_directory($splitdbgdir); # creating language specific subdirectories

            my $destfilename = $splitdbgdir . $installer::globals::separator . $shortfilename;
            installer::systemactions::copy_one_file($sourcefilename, $destfilename);

            # change sourcepath in files collector

            ${$filelist}[$i]->{'sourcepath'} = $destfilename;

            # split debuginfo from file

            _do_splitdbg($destfilename);

            push(@debugfilelist, ${$filelist}[$i]);
        }
    }
    return \@debugfilelist;
}

1;
