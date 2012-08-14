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


package pre2par::work;

use pre2par::exiter;
use pre2par::remover;
use pre2par::pathanalyzer;

############################################
# pre2par working module
############################################

############################################
# procedure to split a line, that contains
# more than one par file lines
############################################

sub split_line
{
    my ($line, $parfile) = @_;

    while ( $line =~ /^((?:[^"]|\"(?:[^"\\]|\\.)*\")*?\;\s+)\s*(.*)$/ )
    {
        my $oneline = $1;
        $line = $2;
        pre2par::remover::remove_leading_and_ending_whitespaces(\$oneline);
        $oneline = $oneline . "\n";
        push(@{$parfile}, $oneline);

        if ( $line =~ /^\s*End\s+(\w+.*$)/i )
        {
            $line = $1;
            push(@{$parfile}, "End\n\n");
        }
    }

    # the last line

    pre2par::remover::remove_leading_and_ending_whitespaces(\$line);
    $line = $line . "\n";
    push(@{$parfile}, $line);

    if ( $line =~ /^\s*End\s*$/i ) { push(@{$parfile}, "\n"); }
}

###################################################################
# Preprocessing the pre file to split all lines with semicolon
###################################################################

sub preprocess_macros
{
    my ($prefile) = @_;

    my @newprefile = ();

    for ( my $i = 0; $i <= $#{$prefile}; $i++ )
    {
        my $oneline = ${$prefile}[$i];
        if ( $oneline =~ /\;\s*\w+/ )
        {
            split_line($oneline, \@newprefile);
        }
        else
        {
            push(@newprefile, $oneline);
        }
    }

    return \@newprefile;
}

############################################
# main working procedure
############################################

sub convert
{
    my ($prefile) = @_;

    my @parfile = ();

    my $iscodesection = 0;
    my $ismultiliner = 0;
    my $globalline = "";

    # Preprocessing the pre file to split all lines with semicolon
    $prefile = preprocess_macros($prefile);

    for ( my $i = 0; $i <= $#{$prefile}; $i++ )
    {
        my $oneline = ${$prefile}[$i];

        if ($iscodesection)
        {
            if ( $oneline =~ /^\s*\}\;\s*$/ )
            {
                $iscodesection = 0;
            }
            else    # nothing to do for code inside a code section
            {
                push(@parfile, $oneline);
                next;
             }
        }

        if ( $oneline =~ /^\s*$/ ) { next; }

        if ( $oneline =~ /^\s*Code\s+\=\s+\{/ )
        {
            $iscodesection = 1;
        }

        pre2par::remover::remove_leading_and_ending_whitespaces(\$oneline);

        my $insertemptyline = 0;

        if ( $oneline =~ /^\s*End\s*$/i ) { $insertemptyline = 1; }

        # Sometimes the complete file is in one line, then the gid line has to be separated

        if ( $oneline =~ /^\s*(\w+\s+\w+)\s+(\w+\s+\=.*$)/ )    # three words before the equal sign
        {
            my $gidline = $1;
            $oneline = $2;
            $gidline = $gidline . "\n";

            push(@parfile, $gidline);
        }

        if ( $oneline =~ /\;\s*\w+/ )
        {
            split_line($oneline, \@parfile);
            next;
        }

        # searching for lines with brackets, like Customs = { ..., which can be parted above several lines

        if ( $oneline =~ /^\s*\w+\s+\=\s*\(.*\)\s*\;\s*$/ )     # only one line
        {
            if (( ! ( $oneline =~ /^\s*Assignment\d+\s*\=/ )) && ( ! ( $oneline =~ /^\s*PatchAssignment\d+\s*\=/ )))
            {
                $oneline =~ s/\s//g;        # removing whitespaces in lists
                $oneline =~ s/\=/\ \=\ /;   # adding whitespace around equals sign
            }
        }

        if ( $oneline =~ /^\s*\w+\s+\=\s*$/ )
        {
            $oneline =~ s/\s*$//;
            pre2par::exiter::exit_program("Error: Illegal syntax, no line break after eqals sign allowed. Line: \"$oneline\"", "convert");
        }

        if (( $oneline =~ /^\s*\w+\s+\=\s*\(/ ) && (!( $oneline =~ /\)\s*\;\s*$/ )))     # several lines
        {
            $ismultiliner = 1;
            $oneline =~ s/\s//g;
            $globalline .= $oneline;
            next;                       # not including yet
        }

        if ( $ismultiliner )
        {
            $oneline =~ s/\s//g;
            $globalline .= $oneline;

            if ( $oneline =~ /\)\s*\;\s*$/ ) {  $ismultiliner = 0; }

            if (! ( $ismultiliner ))
            {
                $globalline =~ s/\=/\ \=\ /;    # adding whitespace around equals sign
                $globalline .= "\n";
                push(@parfile, $globalline);
                $globalline = "";
            }

            next;
        }

        $oneline = $oneline . "\n";

        $oneline =~ s/\s*\=\s*/ \= /;   # nice, to have only one whitespace around equal signs

        # Concatenate adjacent string literals:
        while ($oneline =~
               s/^((?:[^"]*
                      \"(?:[^\\"]|\\.)*\"
                      (?:[^"]*[^[:blank:]"][^"]*\"(?:[^\\"]|\\.)*\")*)*
                   [^"]*
                   \"(?:[^\\"]|\\.)*)
                 \"[[:blank:]]*\"
                 ((?:[^\\"]|\\.)*\")
                /\1\2/x)
        {}

        push(@parfile, $oneline);

        if ($insertemptyline) { push(@parfile, "\n"); }

    }

    return \@parfile;
}

############################################
# formatting the par file
############################################

sub formatter
{
    my ($parfile) = @_;

    my $iscodesection = 0;

    my $tabcounter = 0;
    my $isinsideitem = 0;
    my $currentitem;

    for ( my $i = 0; $i <= $#{$parfile}; $i++ )
    {
        my $oneline = ${$parfile}[$i];
        my $isitemline = 0;

        if (! $isinsideitem )
        {
            for ( my $j = 0; $j <= $#pre2par::globals::allitems; $j++ )
            {
                if ( $oneline =~ /^\s*$pre2par::globals::allitems[$j]\s+\w+\s*$/ )
                {
                    $currentitem = $pre2par::globals::allitems[$j];
                    $isitemline = 1;
                    $isinsideitem = 1;
                    $tabcounter = 0;
                    last;
                }
            }
        }

        if ( $isitemline )
        {
            next;   # nothing to do
        }

        if ( $oneline =~ /^\s*end\s*$/i )
        {
            $isinsideitem = 0;
            $tabcounter--;
        }

        if ( $isinsideitem )
        {
            $oneline = "\t" . $oneline;
            ${$parfile}[$i] = $oneline;
        }
    }
}

###################################################
# Returning the language file name
###################################################

sub getlangfilename
{
    return $pre2par::globals::langfilename;
}

############################################
# Checking if a file exists
############################################

sub fileexists
{
    my ($langfilename) = @_;

    my $fileexists = 0;

    if( -f $langfilename ) { $fileexists = 1; }

    return $fileexists;
}

############################################
# Checking the existence of ulf file
############################################

sub check_existence_of_langfiles
{
    my ($langfilename) = @_;

    my $do_localize = 0;

    if ( fileexists($langfilename) ) { $do_localize = 1; }

    return $do_localize;
}

############################################
# Checking that the pre file has content
############################################

sub check_content
{
    my ($filecontent, $filename) = @_;

    if ( $#{$filecontent} < 0 ) { pre2par::exiter::exit_program("Error: $filename has no content!", "check_content"); }
}

############################################
# Checking content of par files.
# Currently only size.
############################################

sub diff_content
{
    my ($content1, $content2, $filename) = @_;

    if ( $#{$content1} != $#{$content2} ) { pre2par::exiter::exit_program("Error: $filename was not saved correctly!", "diff_content"); }
}

1;
