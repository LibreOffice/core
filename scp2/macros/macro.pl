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

my $outfile;
my $destfile;
my $config_stamp;
my $lastcompletelangiso_var;
my $completelangiso_var = $ENV{COMPLETELANGISO_VAR};
my $help_langs_var = $ENV{HELP_LANGS};

if ( !defined $completelangiso_var) {
    print STDERR "ERROR: No language defined!\n";
    exit 1;
}

if (!args_require_build()) {
    print STDERR "No new languages, or config. Keeping old file\n";
    exit 0;
}

my @completelangiso = split " +", $completelangiso_var;
my @helplangs = split " +", $help_langs_var;

open OUTFILE, ">$outfile" or die "$0 ERROR: cannot open $outfile for writing!\n";

print OUTFILE "// generated file, do not edit\n\n";
print OUTFILE "// languages used for last time generation\n";
print OUTFILE "// completelangiso: $completelangiso_var\n\n";
print OUTFILE "// help_langs: $help_langs_var\n\n";
write_ITERATE_ALL_LANG();
write_ALL_LANG();
write_OTHER_LANGS();
write_DIR_ISOLANGUAGE_ALL_LANG_2();
write_DIR_ISOLANGUAGE_ALL_LANG();
write_DIR_ISOLANGUAGE_ALL_LANG_LPROJ();
write_EXTRA_ALL_LANG();
write_EXTRA_ALL_LANG_BUT_EN_US();
write_EXTRA_ALL_GOOD_HELP_LOCALIZATIONS_LANG();
write_RESFILE_ALL_LANG();
write_README_ALL_LANG();
write_README_TXT_ALL_LANG();
write_FILE_ALL_LANG_LETTER();

close OUTFILE;

rename $outfile, $destfile;

sub write_ITERATE_ALL_LANG
{
    print OUTFILE "#define ITERATE_ALL_LANG_FILE(gid,dir,macro,name,ext) ";
    foreach $lang (@helplangs) {
        my $shortlang = $lang;
        $shortlang = "en" if $shortlang eq "en-US";
        my $speciallang = $lang;
        $speciallang =~ s/-/_/;
        print OUTFILE "\\\nFile CONCAT3(gid_File_,gid,_$speciallang)";
        print OUTFILE "\\\n\tDir = CONCAT3(gid_Dir_,dir,_$speciallang);";
        print OUTFILE "\\\n\tmacro;";
        print OUTFILE "\\\n\tName = STRING(CONCAT3(name,_$lang,ext)); ";
        print OUTFILE "\\\nEnd ";
        print OUTFILE "\\\n";
    }
    print OUTFILE "\n\n";

    print OUTFILE "#define ITERATE_ALL_LANG_DIR_LPROJ(gid,parent) ";
    foreach $lang (@helplangs) {
        my $shortlang = $lang;
        $shortlang = "en" if $shortlang eq "en-US";
        my $speciallang = $lang;
        $speciallang =~ s/-/_/;
        print OUTFILE "\\\nDirectory CONCAT3(gid_Dir_,gid,_$speciallang)";
        print OUTFILE "\\\n\tParentID = CONCAT2(gid_Dir_,parent);";
        print OUTFILE "\\\n\tDosName = \"$shortlang.lproj\"; ";
        print OUTFILE "\\\nEnd ";
        print OUTFILE "\\\n";
    }
    print OUTFILE "\n\n";
}

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

sub write_DIR_ISOLANGUAGE_ALL_LANG_LPROJ
{
    print OUTFILE "#define DIR_ISOLANGUAGE_ALL_LANG_LPROJ ";
    foreach $lang (@completelangiso) {
        my $speciallang = $lang;
        if ( $speciallang eq "en-US" ) { $speciallang = "en"; }
        print OUTFILE "\\\n\tDosName ($lang) = \"$speciallang.lproj\"";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_EXTRA_ALL_LANG
{
    print OUTFILE "#define EXTRA_ALL_LANG(name,ext) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tName ($lang) = CONFIGLANGFILENAME(name,_$lang,ext)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub write_EXTRA_ALL_LANG_BUT_EN_US
{
    print OUTFILE "#define EXTRA_ALL_LANG_BUT_EN_US(name,ext) ";
    my $first = 1;
    foreach $lang (@completelangiso) {
        if ($lang ne "en-US") {
            print OUTFILE "; " unless $first;
            $first = 0;
            print OUTFILE
                "\\\n\tName ($lang) = CONFIGLANGFILENAME(name,_$lang,ext)";
        }
    }
    print OUTFILE "\n\n";
}

sub write_EXTRA_ALL_GOOD_HELP_LOCALIZATIONS_LANG
{
    my $first = 1;
    print OUTFILE "#define EXTRA_ALL_GOOD_HELP_LOCALIZATIONS_LANG(name) ";
    foreach $lang (@helplangs) {
        print OUTFILE ";" unless $first;
        $first = 0;
        print OUTFILE "\\\n\tName ($lang) = EXTRAFILELISTNAME(HelpTarget/,name,/$lang)";
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

sub write_FILE_ALL_LANG_LETTER
{
    print OUTFILE "#define FILE_ALL_LANG_LETTER(name, ext) ";
    foreach $lang (@completelangiso) {
        print OUTFILE "\\\n\tName ($lang) = CONFIGLANGFILENAME(name,$lang,ext)";
        print OUTFILE "; " if ( $lang ne $completelangiso[$#completelangiso]);
    }
    print OUTFILE "\n\n";
}

sub args_require_build
{
    while (@ARGV) {
        $opt = shift @ARGV;
        $destfile = shift @ARGV if ($opt eq '-o');
        $config_stamp = shift @ARGV if ($opt eq '-c');
    }
    usage() if (!defined ($destfile) || !defined ($config_stamp));
    $outfile = "$destfile.tmp";

    if ( -f "$destfile" ) {
        # changed script - run always
        return 1 if (stat($0))[9] > (stat("$destfile"))[9] ;

        # changed config timestamp - run always
        if (-f "$config_stamp") {
            return 1 if (stat($config_stamp))[9] > (stat($destfile))[9];
        }

        open OLDFILE, "$destfile" or die "$0 - ERROR: $outfile exists but isn't readable.\n";
        while ( $line = <OLDFILE> ) {
            if ( $line =~ /^\/\/.*completelangiso:/ ) {
                $lastcompletelangiso_var = $line;
                chomp $lastcompletelangiso_var;
                $lastcompletelangiso_var =~ s/^\/\/.*completelangiso:\s*//;
                last;
            }

        }
        close OLDFILE;

        return 0 if ( "$completelangiso_var" eq "$lastcompletelangiso_var" );
    }
    return 1;
}

sub usage
{
    print STDERR "Generate language dependend macros use in *.scp files\n";
    print STDERR "perl $0 -o <outputfile> -c <config_stamp_file>\n";
    exit  1;
}
