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



package installer::upx;

use installer::converter;
use installer::existence;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::scriptitems;
use installer::systemactions;

#####################################################################
# Checking whether a file has to be stripped
#####################################################################

sub is_upx_candidate
{
    my ( $filename, $onefile ) = @_;

    my $useupx = 0;

    if (( $filename =~ /\.so\s*$/ ) ||
        ( $filename =~ /\.dll\s*$/ ) ||
        ( $filename =~ /\.exe\s*$/ ) ||
        ( $filename =~ /\.bin\s*$/ ))
    {
        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
        if ( ! ( $styles =~ /\bDONT_UPX\b/ )) { $useupx = 1; }
    }

    return $useupx;
}

#####################################################################
# Checking whether a file has to be stripped
#####################################################################

sub do_upx
{
    my ( $filename ) = @_;

    my $compression = "9";
    my $systemcall = $installer::globals::upxfile . " -" . $compression . " " . $filename;

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    $installer::logger::Lang->print($infoline);

    if ($returnvalue)
    {
        $infoline = "WARNING: Could not successfully upx $filename! Using original file.\n";
        $installer::logger::Lang->print($infoline);
    }
    else
    {
        $infoline = "SUCCESS: upx $filename!\n";
        $installer::logger::Lang->print($infoline);
    }

    return $returnvalue;
}

#####################################################################
# Using upx to decrease file size
#####################################################################

sub upx_on_libraries
{
    my ( $filelist, $languagestringref) = @_;

    installer::logger::include_header_into_logfile("UPX'ing files:");
    my $infoline = "";

    if ( ! $installer::globals::upx_in_path )
    {
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->print("Warning: This is an UPX product, but upx was not found in PATH!\n");
        $installer::logger::Lang->print("\n");
    }
    else
    {
        $infoline = "Using upx: $installer::globals::upxfile\n";
        $installer::logger::Lang->print($infoline);

        my $upxdirbase = installer::systemactions::create_directories("upx", $languagestringref);

        if (! installer::existence::exists_in_array($upxdirbase, \@installer::globals::removedirs))
        {
            push(@installer::globals::removedirs, $upxdirbase);
        }

        for ( my $i = 0; $i <= $#{$filelist}; $i++ )
        {
            my $sourcefilename = ${$filelist}[$i]->{'sourcepath'};

            if ( is_upx_candidate($sourcefilename, ${$filelist}[$i]) )
            {
                my $shortfilename = $sourcefilename;
                installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$shortfilename);

                $installer::logger::Lang->print("\n");
                $installer::logger::Lang->printf("Upx: %s", $shortfilename);

                # copy file into directory for stripped libraries
                my $onelanguage = ${$filelist}[$i]->{'specificlanguage'};

                # files without language into directory "00"
                if ($onelanguage eq "") { $onelanguage = "00"; }

                my $upxdir = $upxdirbase . $installer::globals::separator . $onelanguage;
                installer::systemactions::create_directory($upxdir);    # creating language specific subdirectories

                my $destfilename = $upxdir . $installer::globals::separator . $shortfilename;
                installer::systemactions::copy_one_file($sourcefilename, $destfilename);

                # change sourcepath in files collector
                ${$filelist}[$i]->{'sourcepath'} = $destfilename;

                # do upx on file
                my $return = do_upx($destfilename);

                # Using original file, if upx was not successful (no reason for error)
                if ( $return ) { ${$filelist}[$i]->{'sourcepath'} = $sourcefilename; }
            }
        }
    }
}

1;
