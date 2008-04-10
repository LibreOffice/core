#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: macro.pl,v $
#
# $Revision: 1.9 $
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

my $completelangiso_var = $ENV{COMPLETELANGISO_VAR};
my $lastcompletelangiso_var;;
my $outfile = "";

if ( !defined $completelangiso_var) {
    print STDERR "ERROR: No language defined!\n";
    exit 1;
}

my $poorhelplocalizations_var = $ENV{WITH_POOR_HELP_LOCALIZATIONS};
my %poorhelplocalizations;
foreach $lang (split (/ /, $poorhelplocalizations_var)) {
  $poorhelplocalizations{$lang}++;
}

startup_check();
if ( "$completelangiso_var" eq "$lastcompletelangiso_var" ) {
    print STDERR "No new languages. Keeping old file\n";
    exit 0;
}

my @completelangiso = split " +", $completelangiso_var;

open OUTFILE, ">$outfile" or die "$0 ERROR: cannot open $outfile for writing!\n";

print OUTFILE "// generated file, do not edit\n\n";
print OUTFILE "// languages used for last time generation\n";
print OUTFILE "// completelangiso: $completelangiso_var\n\n";
write_ALL_LANG();
write_OTHER_LANGS();
write_DIR_ISOLANGUAGE_ALL_LANG_2();
write_DIR_ISOLANGUAGE_ALL_LANG();
write_DIR_IDENT_ALL_LANG();
write_EXTRA_ALL_LANG();
write_EXTRA_ALL_GOOD_HELP_LOCALIZATIONS_LANG();
write_EXTRA_IDENT_ALL_LANG();
write_RESFILE_ALL_LANG();
write_SHORT_RESFILE_ALL_LANG();
write_README_ALL_LANG();
write_README_TXT_ALL_LANG();
write_VALUE_ALL_LANG_LETTER_DIR();
write_KEY_ALL_LANG_LETTER();
write_FILE_ALL_LANG_LETTER();

close OUTFILE;


sub write_ALL_LANG
{
    print OUTFILE "#define ALL_LANG(ident, resid) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tident ($lang) = resid";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_OTHER_LANGS
{
    print OUTFILE "#define OTHER_LANGS ";
    foreach $lang (@completelangiso) {
        next if ( $lang eq "en-US");
        print OUTFILE "\\\n\tDosName ($lang) = \"$lang\"";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
        print OUTFILE "\n\n";
}

sub write_DIR_ISOLANGUAGE_ALL_LANG_2
{
    print OUTFILE "#define DIR_ISOLANGUAGE_ALL_LANG_2 ";
    print OUTFILE "\\\n\tDosName (en-US) = \"en-US\"; \\\n\t";
    print OUTFILE "OTHER_LANGS";
        print OUTFILE "\n\n";
}

sub write_DIR_ISOLANGUAGE_ALL_LANG
{
    print OUTFILE "#define DIR_ISOLANGUAGE_ALL_LANG ";
    print OUTFILE "\\\n\tDosName (en-US) = \"en\"; \\\n\t";
    print OUTFILE "OTHER_LANGS";
        print OUTFILE "\n\n";
}

sub write_DIR_IDENT_ALL_LANG
{
    print OUTFILE "#define DIR_IDENT_ALL_LANG(name) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tDosName ($lang) = STRING(name)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_EXTRA_ALL_LANG
{
    print OUTFILE "#define EXTRA_ALL_LANG(name) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tName ($lang) = EXTRAFILENAME(name,_$lang)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_EXTRA_ALL_GOOD_HELP_LOCALIZATIONS_LANG
{
    my $first = 1;
    print OUTFILE "#define EXTRA_ALL_GOOD_HELP_LOCALIZATIONS_LANG(name) ";
    foreach $lang (@completelangiso) {
        next if ( $poorhelplocalizations{$lang} );
        print OUTFILE ";" unless $first;
        $first = 0;
        print OUTFILE "\\\n\tName ($lang) = EXTRAFILENAME(name,_$lang)";
    }
    print OUTFILE "\n\n";
}

sub write_EXTRA_IDENT_ALL_LANG
{
    print OUTFILE "#define EXTRA_IDENT_ALL_LANG(name) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tName ($lang) = STRING(name)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_RESFILE_ALL_LANG
{
    print OUTFILE "#define RESFILE_ALL_LANG(name) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tName ($lang) = RESFILENAME(name,$lang)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_SHORT_RESFILE_ALL_LANG
{
    print OUTFILE "#define SHORT_RESFILE_ALL_LANG(name) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tName ($lang) = SHORTRESFILENAME(name,$lang)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_README_ALL_LANG
{
    print OUTFILE "#define README_ALL_LANG(key, name) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tkey ($lang) = READMEFILENAME(name,_$lang)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_README_TXT_ALL_LANG
{
    print OUTFILE "#define README_TXT_ALL_LANG(key, name, ext) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tkey ($lang) = READMETXTFILENAME(name,_$lang,ext)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

# FIXME: Not used at all in OOo?, #i38597#
sub write_VALUE_ALL_LANG_LETTER_DIR
{
    print OUTFILE "#define VALUE_ALL_LANG_LETTER_DIR ";
    print OUTFILE "\\\n\tValue (en-US) = STRING(en);";
    foreach $lang (@completelangiso) {
        next if ( $lang eq "en-US");
        print OUTFILE "\\\n\tValue ($lang) = STRING($lang)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_KEY_ALL_LANG_LETTER
{
    print OUTFILE "#define KEY_ALL_LANG_LETTER ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tKey ($lang) = STRING($lang)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_FILE_ALL_LANG_LETTER
{
    print OUTFILE "#define FILE_ALL_LANG_LETTER(name, ext) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tName ($lang) = CONFIGLANGFILENAME(name,$lang,ext)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub startup_check
{
    my $i;
    for ( $i=0; $i <= $#ARGV; $i++) {
        if ( "$ARGV[$i]" eq "-o" ) {
            if ( defined $ARGV[ $i + 1] ) {
                $outfile = $ARGV[ $i + 1];
            } else {
                usage();
            }
        }
    }
    usage() if $i<2;
    usage() if "$outfile" eq "";
    if ( -f "$outfile" ) {
        # changed script - run allways
        return if (stat($0))[9] > (stat("$outfile"))[9] ;

        open OLDFILE, "$outfile" or die "$0 - ERROR: $outfile exists but isn't readable.\n";
        while ( $line = <OLDFILE> ) {
            if ( $line =~ /^\/\/.*completelangiso:/ ) {
                $lastcompletelangiso_var = $line;
                chomp $lastcompletelangiso_var;
                $lastcompletelangiso_var =~ s/^\/\/.*completelangiso:\s*//;
                last;
            }

        }
        close OLDFILE;
    }
}

sub usage
{
    print STDERR "Generate language dependend macros use in *.scp files\n";
    print STDERR "perl $0 -o outputfile\n";
    exit  1;
}
