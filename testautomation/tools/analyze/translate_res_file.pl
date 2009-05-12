#! /usr/bin/perl

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: translate_res_file.pl,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: andreschnabel $ $Date: 2007/02/18 12:01:54 $
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
#
#   short description :
#     This Perl sciprt translates *.res files of qatesttool into human
#     readable format
#*************************************************************************

sub usage {
    print << "EOT";

  $0 - translates *.res files of qatesttool into readable format

  Synopsis:
     $0 [options] file...

  Options:
     --help                  prints this message.
     --testtoolrc=filename   specifies .testtoolrc (default: \$HOME/.testtoolrc)
     --with-filename         inserts filename:linenumber:, which helps users go there with emacs
     --lang=language         specifies language defined in app.srs (default: \$LANG)
     --app_srs=filename      specifies resource file app.srs (default: ./app.srs)
     --with-color            outputs colored translated messages using ANSI color sequence code

  Description:
    This Perl sciprt translates *.res files produced by qatesttool
    into human readable text file format.

  Examples:
    perl translate_res_file.pl topten.res

EOT
}

use Getopt::Long;
$Opt_With_Filename = 0;
$With_Color = 0;

use Env qw( LANG HOME );
use Term::ANSIColor;

$BaseDir = undef;
%Resource = ();

%LogType = qw( 0 RUN
           1 TEST_CASE
           2 ERROR
           3 CALL_STACK
           4 MESSAGE
           5 WARNING
           6 ASSERTION
           7 QA_ERROR
           8 ASSERTION_STACK
           );

%LogTypeColor = qw( 0 none
            1 none
            2 on_red
            3 none
            4 none
            5 on_yellow
            6 none
            7 on_cyan
            8 none
            );

sub substitution {
    my ($file, $lineno, $line) = @_;
    my ($id, $arg1, $arg2, $arg3) =
    ($line =~ m/\%ResId=([0-9]+)\%
     (?:\%Arg1=(.+?)\%)?
     (?:\%Arg2=(.+?)\%)?
     (?:\%Arg3=(.+?)\%)?/x);
    my $text = $Resource{$id};
    if (defined $text) {
    $text =~ s/\(\$Arg1\)/$arg1/ if ($text =~ m/\(\$Arg1\)/);
    $text =~ s/\(\$Arg2\)/$arg2/ if ($text =~ m/\(\$Arg2\)/);
    $text =~ s/\(\$Arg3\)/$arg3/ if ($text =~ m/\(\$Arg3\)/);
    return $text;
    }
    else {
    print STDERR "$file:$lineno: Unknown String: $line\n";
    }
}

sub work {
    my $file = shift;
    my ($line, $lineno, $log_type);
    open FH, "$file" or die "$!: $file, stopped";
    while (<FH>) {
    unless (m/\A[0-9]+;/) {
        print;
        next;
    }
    chomp;
    s/(\r|\n)+\Z//;
    $lineno ++;
    $line = $_;
    my ($type, $_filename, $_lineno, $start, $end, $text) = split(m/;/, $_, 6);
    $text = "" unless defined $text;
    $text =~ s/(\%ResId=.+%)/ substitution($file, $lineno, $1) /e;
    #print "\n$line\n";
    $text =~ s/\A\"//;
    $text =~ s/\"\Z//;
    $log_type = $LogType{$type} || "Unknown LogType";
    if (defined $_filename and $_filename ne "") {
        $_filename =~ s/\A\~-//g;
        $_filename =~ s/\A\~/$BaseDir\//g if defined $BaseDir;
        $_filename =~ s{\\}{/}g;
        print "$_filename:$_lineno: " if $Opt_With_Filename;
    }
    if ($With_Color and $LogTypeColor{$type} ne "none") {
        print colored ("$log_type: $text", $LogTypeColor{$type});
        print "\n";
    }
    else {
        print "$log_type: $text\n";
    }
    }
    close FH;
}

sub getBaseDir {
    my $file = shift;
    my $dir;
    open FH, "<", $file or do {
    warn "Warning: $!: $file; BaseDir substitution will be suppressed;";
    return undef;
    };
    while (<FH>) {
    if (m/\ABaseDir=(.*)/) {
        $dir = $1;
        chomp $dir;
        $dir =~ s/(\r|\n)+\Z//;
    }
    }
    close FH;
    return $dir;
}

sub load_app_srs {
    my $file = shift;
    my $lang = shift;
    my $flag = undef;
    my ($id, $text, $fallback);
    my ($x);
    unless (defined $lang) {
    $lang = $LANG || "C";
    }
    $lang = substr($lang, 0, 2);
    $lang = "en-US" if $lang eq "en" or $lang eq "C";
    print $lang;

    open FH, $file or die "Error: $!: $file, stopped";
    while (<FH>) {
    next if m/\A#/;
    if (m/\AString\s+([0-9]+)/) {
        $id = $1;
        $flag = 1;
        undef $text;
        next;
    }
    next unless $flag;
    if (m/\};/) {
        $text = $fallback unless defined $text;
        chomp $text;
        $text =~ s/\A\"//;
        $text =~ s/\"\Z//;
        $Resource{$id} = $text;
        #print "$id\t$text\n";
        undef $flag;
        next;
    }
    if (m/Text = "(.+?)";\Z/) {
        $fallback = $1;
        next;
    }
    if (m/Text\[ (.+?) \] = "(.+?)";\Z/) {
        $x = $1;
        $fallback = $text if ($x eq "en-US");
        $text = $2 if ($x eq $lang);
    }
    }
    close FH;
}

sub main {
    my $opt_help;
    my $testtoolrc = "$HOME/.testtoolrc";
    my $language;
    my $app_srs = "app.srs";
    my $result = GetOptions ( "help"           => \$opt_help,
                  "testtoolrc=s"   => \$testtoolrc,
                  "with-filename"  => \$Opt_With_Filename,
                  "language=s"     => \$language,
                  "app_srs=s"      => \$app_srs,
                  "with-color"     => \$With_Color,
                  );
    if (scalar(@ARGV) <= 0 or !$result or $opt_help) {
    usage();
    exit;
    }

    $BaseDir = getBaseDir($testtoolrc);
    load_app_srs($app_srs, $language);
    foreach $file (@ARGV) {
    print "$file\n";
    work($file);
    print "\n";
    }
}

main();

__END__
