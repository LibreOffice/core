#!/bin/sh -- # This comment tells perl not to loop!
#
#***************************************************************************
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
#***************************************************************************


eval 'exec perl -S $0 "$@"'
if 0;
#
# @(#)jstyle 1.2 98/01/08
#
# jstyle - check for some common stylistic errors.
#
#	jstyle is a sort of "lint" for Java coding style.
#
#	There's a lot this can't check for, like proper
#	indentation of continuation lines.  There's also
#	a lot more this could check for.
#
#	A note to the non perl literate:
#
#		perl regular expressions are pretty much like egrep
#		regular expressions, with the following special symbols
#
#		\s	any space character
#		\S	any non-space character
#		\w	any "word" character [a-zA-Z0-9_]
#		\W	any non-word character
#		\d	a digit [0-9]
#		\D	a non-digit
#		\b	word boundary (between \w and \W)
#		\B	non-word boundary
#
#require "getopts.pl";
# XXX - because some versions of perl can not find the lib directory,
# we just include this here.
;# getopts.pl - a better getopt.pl

;# Usage:
;#      do Getopts("a:bc");  # -a takes arg. -b & -c not. Sets opt_* as a
;#                           #  side effect.

sub Getopts {
    local($argumentative) = @_;
    local(@args,$_,$first,$rest);
    local($[) = 0;
    local($errs) = 0;

    @args = split( / */, $argumentative );
    while(($_ = $ARGV[0]) =~ /^-(.)(.*)/) {
    ($first,$rest) = ($1,$2);
    $pos = index($argumentative,$first);
    if($pos >= $[) {
        if($args[$pos+1] eq ":") {
        shift(@ARGV);
        if($rest eq "") {
            $rest = shift(@ARGV);
        }
        eval "\$opt_$first = \$rest;";
        }
        else {
        eval "\$opt_$first = 1";
        if($rest eq "") {
            shift(@ARGV);
        }
        else {
            $ARGV[0] = "-$rest";
        }
        }
    }
    else {
        print STDERR "Unknown option: $first\n";
        ++$errs;
        if($rest ne "") {
        $ARGV[0] = "-$rest";
        }
        else {
        shift(@ARGV);
        }
    }
    }
    $errs == 0;
}

1;
# end of getopts.pl

$usage =
"usage: jstyle [-c] [-h] [-p] [-s] [-t] [-v] [-C] file ...
    -c	check continuation line indenting
    -h	perform heuristic checks that are sometimes wrong
    -p	perform some of the more picky checks
    -s	check for spaces vs. tabs
    -t	insist on indenting by tabs
    -v	verbose
    -C	don't check anything in header block comments
    -S	print out overall statistics
";

if (!&Getopts("chpstvCS")) {
    print $usage;
    exit 1;
}

$check_continuation = $opt_c;
$heuristic = $opt_h;
$picky = $opt_p;
$spaces = $opt_s;
$tabs = $opt_t;
$verbose = $opt_v;
$ignore_hdr_comment = $opt_C;
$statistics = $opt_S;

if ($verbose) {
    $fmt = "%s: %d: %s\n%s\n";
} else {
    $fmt = "%s: %d: %s\n";
}

# Note, following must be in single quotes so that \s and \w work right.
$typename = '(int|char|boolean|byte|short|long|float|double)';

if ($#ARGV >= 0) {
    foreach $arg (@ARGV) {
        if (!open(STDIN, $arg)) {
            printf "%s: can not open\n", $arg;
        } else {
            &jstyle($arg);
            close STDIN;
        }
    }
} else {
    &jstyle("<stdin>");
}

if ($statistics != 0) {
    foreach $key (sort(keys %errcount)) {
        printf "%6d %s\n", $errcount{$key}, $key;
    }
    printf " -----\n";
    printf "%6d Total warnings\n", $tot_errcount;
    printf "%6d Lines of code\n", $totlines;
}

sub err {
    if ($statistics == 0) {
        printf $fmt, $filename, $., $_[0], $line;
    } else {
        $msg = $_[0];
        $msg =~ s/ \([0-9][0-9]*\)$//;
        $errcount{$msg} += 1;
        $tot_errcount += 1;
    }
}

sub jstyle {

$in_comment = 0;
$in_header_comment = 0;
$in_continuation = 0;
$in_class = 0;
$in_declaration = 0;
$note_level = 0;
$nextok = 0;
$nocheck = 0;
$expect_continuation = 0;
$prev = '';

$filename = $_[0];

line: while (<STDIN>) {
    ++$totlines;
    s/\r?\n$//;	# strip return and newline

    # save the original line, then remove all text from within
    # double or single quotes, we do not want to check such text.

    $line = $_;
    s/"[^"]*"/\"\"/g;
    s/'.'/''/g;

    # an /* END JSTYLED */ comment ends a no-check block.
    if ($nocheck) {
        if (/\/\* *END *JSTYLED *\*\//) {
            $nocheck = 0;
        } else {
            next line;
        }
    }

    # a /*JSTYLED*/ comment indicates that the next line is ok.
    if ($nextok) {
        if ($okmsg) {
            do err($okmsg);
        }
        $nextok = 0;
        $okmsg = 0;
        if (/\/\* *JSTYLED.*\*\//) {
            /^.*\/\* *JSTYLED *(.*) *\*\/.*$/;
            $okmsg = $1;
            $nextok = 1;
        }
        $prev = $line;
        next line;
    }

    # check length of line.
    # first, a quick check to see if there is any chance of being too long.
    if ($line =~ tr/\t/\t/ * 7 + length($line) > 100) {
        # yes, there is a chance.
        # replace tabs with spaces and check again.
        $eline = $line;
        1 while $eline =~
            s/\t+/' ' x (length($&) * 8 - length($`) % 8)/e;
        $l = length($eline);
        if (length($eline) > 100) {
            do err("line > 100 characters ($l)");
        }
    }
#	this is the fastest way to check line length,
#	but it doesnt work with perl 3.0.
#	if ($line =~ tr/\t/\t/ * 7 + length($line) > 80) {
#		$pos = $oldp = $p = 0;
#		while (($p = index($line, "\t", $p)) >= 0) {
#			$pos = ($pos + $p - $oldp + 8) & ~7;
#			$oldp = ++$p;
#		}
#		$pos += length($line) - $oldp;
#		if ($pos > 80) {
#			do err("line > 80 characters");
#		}
#	}

    # remember whether we expect to be inside a continuation line.
    $in_continuation = $expect_continuation;

    # check for proper continuation line.  blank lines
    # in the middle of the
    # continuation do not count.
    # XXX - only check within functions.
    if ($check_continuation && $expect_continuation && $in_class &&
        !/^\s*$/) {
        # continuation line must start with whitespace of
        # previous line, plus either 4 spaces or a tab, but
        # do not check lines that start with a string constant
        # since they are often shifted to the left to make them
        # fit on the line.
        if (!/^$continuation_indent    \S/ &&
            !/^$continuation_indent\t\S/ && !/^\s*"/) {
            do err("continuation line improperly indented");
        }
        $expect_continuation = 0;
    }

    # a /* BEGIN JSTYLED */ comment starts a no-check block.
    if (/\/\* *BEGIN *JSTYLED *\*\//) {
        $nocheck = 1;
    }

    # a /*JSTYLED*/ comment indicates that the next line is ok.
    if (/\/\* *JSTYLED.*\*\//) {
        /^.*\/\* *JSTYLED *(.*) *\*\/.*$/;
        $okmsg = $1;
        $nextok = 1;
    }
    if (/\/\/ *JSTYLED/) {
        /^.*\/\/ *JSTYLED *(.*)$/;
        $okmsg = $1;
        $nextok = 1;
    }

    # is this the beginning or ending of a class?
    if (/^(public\s+)*\w(class|interface)\s/) {
        $in_class = 1;
        $in_declaration = 1;
        $prev = $line;
        next line;
    }
    if (/^}\s*(\/\*.*\*\/\s*)*$/) {
        $in_class = 0;
        $prev = $line;
        next line;
    }

    if (!$spaces) {
        # strip trailing spaces
        s/\s*$//;
    }

    # does this looks like the start of a block comment?
    if (/^\s*\/\*(\*|)$/) {
        if (!/^(\t|    )*\/\*(\*|)$/) {
            do err("block comment not indented properly");
        }
        $in_comment = 1;
        s/\/\*(\*|)/ /;
        $comment_prefix = $_;
        if ($comment_prefix eq " ") {
            $in_header_comment = 1;
        }
        $prev = $line;
        next line;
    }
    if (/^\s*\/\*./ && !/^\s*\/\*\*$/ && !/^\s*\/\*.*\*\//) {
        do err("improper first line of block comment");
        # it's a bad one, but it still is one.
        # avoid ripple effect of not recognizing this.
        if (!/^(\t|    )*\/\*(\*|)/) {
            do err("block comment not indented properly");
        }
        $in_comment = 1;
        s/\/\*.*/ /;
        $comment_prefix = $_;
        if ($comment_prefix eq " ") {
            $in_header_comment = 1;
        }
        $prev = $line;
        next line;
    }
    # are we still in the block comment?
    if ($in_comment && !/^$comment_prefix\*/) {
        # assume out of comment
        $in_comment = 0;
        $in_header_comment = 0;
    }

    if ($in_header_comment && $ignore_hdr_comment) {
        $prev = $line;
        next line;
    }

    # check for errors that might occur in comments and in code.

    # allow spaces to be used to draw pictures in header comments.
    if ($spaces && /[^ ]     / && !/".*     .*"/ && !$in_header_comment) {
        do err("spaces instead of tabs");
    }
    if ($tabs && /^ / && !/^ \*[ \t\/]/ && !/^ \*$/ &&
        (!/^    \w/ || $in_class != 0)) {
        do err("indent by spaces instead of tabs");
    }
    if (!$in_comment && (/^(\t    )* {1,3}\S/ || /^(\t    )* {5,7}\S/) &&
        !(/^\s*[-+|&\/?:=]/ || ($prev =~ /,\s*$/))) {
        do err("indent not a multiple of 4");
    }
    if ($spaces && /\s$/) {
        do err("space or tab at end of line");
    }
if (0) {
    if (/^[\t]+ [^ \t\*]/ || /^[\t]+  \S/ || /^[\t]+   \S/) {
        do err("continuation line not indented by 4 spaces");
    }
}
    if (/[^ \t(]\/\*/ && !/\w\(\/\*.*\*\/\);/) {
        do err("comment preceded by non-blank");
    }
    if ($spaces && /\t[ ]+\t/) {
        do err("spaces between tabs");
    }
    if ($spaces && / [\t]+ /) {
        do err("tabs between spaces");
    }

    if ($in_comment) {	# still in comment
        $prev = $line;
        next line;
    }

    if ((/\/\*\S/ && !/\/\*\*/) || /\/\*\*\S/) {
        do err("missing blank after open comment");
    }
    if (/\S\*\//) {
        do err("missing blank before close comment");
    }
    # allow // at beginnging of line, often used to comment out code
    if (/.\/\/\S/) {		# C++ comments
        do err("missing blank after start comment");
    }
    # check for unterminated single line comments.
    if (/\S.*\/\*/ && !/\S.*\/\*.*\*\//) {
        do err("unterminated single line comment");
    }

    # delete any comments and check everything else.
    s/\/\*.*\*\///g;
    s/\/\/.*$//;		# C++ comments

    # delete any trailing whitespace; we have already checked for that.
    s/\s*$//;

    # following checks do not apply to text in comments.

    # if it looks like an operator at the end of the line, and it is
    # not really the end of a comment (...*/), and it is not really
    # a label (done:), and it is not a case label (case FOO:),
    # or we are not in a function definition (ANSI C style) and the
    # operator is a "," (to avoid hitting "int\nfoo(\n\tint i,\n\tint j)"),
    # or we are in a function and the operator is a
    # "*" (to avoid hitting on "char*\nfunc()").
    if ((/[-+|&\/?:=]$/ && !/\*\/$/ && !/^\s*\w*:$/ &&
        !/^\s\s*case\s\s*\w*:$/) ||
        /,$/ ||
        ($in_class && /\*$/)) {
        $expect_continuation = 1;
        if (!$in_continuation) {
            /^(\s*)\S/;
            $continuation_indent = $1;
        }
    }
    if (/[^<>\s][!<>=]=/ || /[^<>][!<>=]=\S/ ||
        (/[^->]>[^=>\s]/ && !/[^->]>$/) || (/[^<]<[^=<\s]/ && !/[^<]<$/) ||
        /[^<\s]<[^<]/ || /[^->\s]>[^>]/) {
        do err("missing space around relational operator");
    }
    if (/\S>>=/ || /\S<<=/ || />>=\S/ || /<<=\S/ || /\S[-+*\/&|^%]=/ ||
        (/[^-+*\/&|^%!<>=\s]=[^=]/ && !/[^-+*\/&|^%!<>=\s]=$/) ||
        (/[^!<>=]=[^=\s]/ && !/[^!<>=]=$/)) {
        do err("missing space around assignment operator");
    }
    if (/[,;]\S/ && !/\bfor \(;;\)/) {
        do err("comma or semicolon followed by non-blank");
    }
    # allow "for" statements to have empty "while" clauses
    if (/\s[,;]/ && !/^[\t]+;$/ && !/^\s*for \([^;]*; ;[^;]*\)/) {
        do err("comma or semicolon preceded by blank");
    }
if (0) {
    if (/^\s*(&&|\|\|)/) {
        do err("improper boolean continuation");
    }
}
    if ($picky && /\S   *(&&|\|\|)/ || /(&&|\|\|)   *\S/) {
        do err("more than one space around boolean operator");
    }
    if (/\b(for|if|while|switch|return|case|catch|synchronized)\(/) {
        do err("missing space between keyword and paren");
    }
    if (/(\b(for|if|while|switch|return|catch|synchronized)\b.*){2,}/) {
        # multiple "case" allowed
        do err("more than one keyword on line");
    }
    if (/\b(for|if|while|switch|return|case|catch|synchronized)\s\s+\(/ &&
        !/^#if\s+\(/) {
        do err("extra space between keyword and paren");
    }
    # try to detect "func (x)" but not "if (x)" or
    # "int (*func)();"
    if (/\w\s\(/) {
        $s = $_;
        # strip off all keywords on the line
        s/\b(for|if|while|switch|return|case|catch|synchronized)\s\(/XXX(/g;
        #s/\b($typename|void)\s+\(+/XXX(/og;
        if (/\w\s\(/) {
            do err("extra space between function name and left paren");
        }
        $_ = $s;
    }
    if (/\(\s/) {
        do err("whitespace after left paren");
    }
    # allow "for" statements to have empty "continue" clauses
    if (/\s\)/ && !/^\s*for \([^;]*;[^;]*; \)/) {
        do err("whitespace before right paren");
    }
    if (/^\s*\(void\)[^ ]/) {
        do err("missing space after (void) cast");
    }
    if (/\S{/ && !/{{/) {
        do err("missing space before left brace");
    }
    if ($in_class && /^\s+{/ && ($prev =~ /\)\s*$/)) {
        do err("left brace starting a line");
    }
    if (/}(else|while)/) {
        do err("missing space after right brace");
    }
    if (/}\s\s+(else|while)/) {
        do err("extra space after right brace");
    }
    if (/\b$typename\*/o) {
        do err("missing space between type name and *");
    }
    if ($heuristic) {
        # cannot check this everywhere due to "struct {\n...\n} foo;"
        if ($in_class && !$in_declaration &&
            /}./ && !/}\s+=/ && !/{.*}[;,]$/ && !/}(\s|)*$/ &&
            !/} (else|while)/ && !/}}/) {
            do err("possible bad text following right brace");
        }
        # cannot check this because sub-blocks in
        # the middle of code are ok
        if ($in_class && /^\s+{/) {
            do err("possible left brace starting a line");
        }
    }
    if (/^\s*else\W/) {
        if ($prev =~ /^\s*}$/) {
            $str = "else and right brace should be on same line";
            if ($statistics == 0) {
                printf $fmt, $filename, $., $str, $prev;
                if ($verbose) {
                    printf "%s\n", $line;
                }
            } else {
                $errcount{$str} += 1;
                $tot_errcount += 1;
            }
        }
    }
    $prev = $line;
}

if ($picky && $prev eq "") {
    do err("last line in file is blank");
}

}
