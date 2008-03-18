#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: work.pm,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:06:11 $
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

    while ( $line =~ /^(.*?\;\s+)\s*(.*)$/ )
    {
        my $oneline = $1;
        $line = $2;
        pre2par::remover::remove_leading_and_ending_whitespaces(\$oneline);
        $oneline = $oneline . "\n";
        push(@{$parfile}, $oneline);
    }

    # the last line

    pre2par::remover::remove_leading_and_ending_whitespaces(\$line);
    $line = $line . "\n";
    push(@{$parfile}, $line);

    if ( $line =~ /^\s*End\s*$/i ) { push(@{$parfile}, "\n"); }
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

###################################################
# Creating the ulf file name from the
# corresponding pre file name
###################################################

sub getulffilename
{
    my ($prefilename) = @_;

    my $ulffilename = $prefilename;
    $ulffilename =~ s/\.pre\s*$/\.ulf/;
    pre2par::pathanalyzer::make_absolute_filename_to_relative_filename(\$ulffilename);

    return $ulffilename;
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
# Checking the existence of ulf and
# jlf/mlf files
############################################

sub check_existence_of_langfiles
{
    my ($langfilename, $ulffilename) = @_;

    my $do_localize = 0;

    if (( fileexists($ulffilename) ) && ( ! fileexists($langfilename) )) { pre2par::exiter::exit_program("Error: Did not find language file $langfilename", "check_existence_of_langfiles"); }
    if (( fileexists($ulffilename) ) && ( fileexists($langfilename) )) { $do_localize = 1; }

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
