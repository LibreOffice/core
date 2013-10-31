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



package installer::windows::font;

use installer::files;
use installer::globals;
use installer::windows::idtglobal;


#################################################################################
# Creating the file Font.idt dynamically
# Content:
# File_ FontTitle
#################################################################################

sub create_font_table
{
    my ($filesref, $basedir) = @_;

    my @fonttable = ();

    installer::windows::idtglobal::write_idt_header(\@fonttable, "font");

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $styles = "";

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( $styles =~ /\bFONT\b/ )
        {
            my %font = ();

            $font{'File_'} = $onefile->{'uniquename'};
            # $font{'FontTitle'} = $onefile->{'FontName'};  # results in a warning during validation
            $font{'FontTitle'} = "";

            my $oneline = $font{'File_'} . "\t" . $font{'FontTitle'} . "\n";

            push(@fonttable, $oneline);
        }
    }

    # Saving the file

    my $fonttablename = $basedir . $installer::globals::separator . "Font.idt";
    installer::files::save_file($fonttablename ,\@fonttable);
    my $infoline = "Created idt file: $fonttablename\n";
    $installer::logger::Lang->print($infoline);

}

#################################################################################
# Reading the Font version from the ttf file, to avoid installation
# of older files over newer files.
#################################################################################

sub get_font_version
{
    my ( $fontfile ) = @_;

    if ( ! -f $fontfile ) { installer::exiter::exit_program("ERROR: Font file does not exist: \"$fontfile\"", "get_font_version"); }

    my $fontversion = 0;
    my $infoline = "";

    my $onefile = installer::files::read_binary_file($fontfile);

    if ( $onefile =~ /Version\s+(\d+\.\d+\.*\d*)/ )
    {
        $fontversion = $1;
        $infoline = "FONT: Font \"$fontfile\" version: $fontversion\n";
        $installer::logger::Lang->print($infoline);
    }
    else
    {
        $infoline = "FONT: Could not determine font version: \"$fontfile\"\n";
        $installer::logger::Lang->print($infoline);
    }

    return $fontversion;
}

1;
