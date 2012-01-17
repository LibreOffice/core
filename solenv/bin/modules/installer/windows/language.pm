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



package installer::windows::language;

use installer::exiter;

####################################################
# Determining the Windows language (LCID)
# English: 1033
####################################################

sub get_windows_language
{
    my ($language) = @_;

    my $windowslanguage = "";

    if ( $installer::globals::msilanguage->{$language} ) { $windowslanguage = $installer::globals::msilanguage->{$language}; }

    if ( $windowslanguage eq "" ) { installer::exiter::exit_program("ERROR: Unknown language $language in function get_windows_language", "get_windows_language"); }

    return $windowslanguage;
}

####################################################
# Determining the Windows language ANSI-Codepage
# English: 1252
####################################################

sub get_windows_encoding
{
    my ($language) = @_;

    my $windowsencoding = "";

    if ( $installer::globals::msiencoding->{$language} ) { $windowsencoding = $installer::globals::msiencoding->{$language}; }

    # if ( $windowsencoding eq "" ) { installer::exiter::exit_program("ERROR: Unknown language $language in function get_windows_encoding", "get_windows_encoding"); }
    if ( $windowsencoding eq "" ) { $windowsencoding = "0"; }   # setting value, if the language is not listed in the encodinglist

    if ( $windowsencoding eq "0" ) { $windowsencoding = "65001"; }  # languages with "0" have to be available in UTF-8 (65001)

    # Asian multilingual installation sets need a code neutral Windows Installer database -> $windowsencoding = 0
    if (( $language eq "en-US" ) && (( $installer::globals::product =~ /suitemulti/i ) || ( $installer::globals::product =~ /officemulti/i ) || ( $installer::globals::product =~ /c05office/i ) || ( $installer::globals::added_english ))) { $windowsencoding = "0"; }

    return $windowsencoding;
}

1;