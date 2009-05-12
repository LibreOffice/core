: # -*- perl -*-  vim: ft=perl
eval 'exec perl -w -S $0 ${1+"$@"}'
if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: langid.pl,v $
#
# $Revision: 1.4 $
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

# See Usage() below or invoke without arguments for short instructions.
# For long instructions use the source, Luke ;-)

use strict;

sub Usage()
{
    print STDERR
        "\n",
        "langid - a hackish utility to lookup lang.h language defines and LangIDs,\n",
        "isolang.cxx ISO639/ISO3166 mapping, locale data files, langtab.src language\n",
        "listbox entries, Langpack.ulf installer language and file_ooo.scp registry name.\n\n",

        "Usage: $0 [--single] {language string} | {LangID} | {primarylanguage sublanguage} | {language-country}\n\n",

        "A language string will be used as a generic string match in all searched files.\n",
        "You may enclose the language string in word delimiters,\n",
        "e.g. \\blanguage_german\\b for a specific match.\n",
        "If the language string expression matches more than one define,\n",
        "e.g. as in 'german', all matching defines will be processed.\n",
        "If the language string does not match a define or an identifier in\n",
        "langtab.src, a generic string match of the listbox entries will be tried.\n\n",

        "Numeric values of LangID,primarylanguage,sublanguage can be given\n",
        "decimal, hexagesimal (leading 0x), octal (leading 0) or binary (leading 0b).\n",
        "The exact language_define of an exact match will be used in remaining lookups.\n\n",

        "A language-country pair will lookup a xx-YY mapping from isolang.cxx,\n",
        "for example: 'en-US' or 'de-' or '-CH',\n",
        "xx and YY can be given case insensitive, will be lowered-uppered internally,\n",
        "and xx and YY themselfs may be regular expressions.\n",
        "Also here a list of matches will be processed.\n\n",

        "If option --single is given, only the first match will be processed.\n\n";
}

my $SOLARVERSION = $ENV{"SOLARVERSION"};
my $INPATH       = $ENV{"INPATH"};
my $SRC_ROOT     = $ENV{"SRC_ROOT"};
my $UPDMINOREXT  = $ENV{"UPDMINOREXT"};
if (!defined($SOLARVERSION) || !defined($INPATH) || !defined($SRC_ROOT))
{
    print "\nNeed \$SOLARVERSION, \$INPATH and \$SRC_ROOT, please set your OOo environment!\n";
    Usage();
    exit 1;
}
if (!defined($UPDMINOREXT)) {
    $UPDMINOREXT  = '';
}
my $SOLENVINC = "$SOLARVERSION/$INPATH/inc$UPDMINOREXT";

my $LANGUAGE_MASK_PRIMARY = 0x03ff;

sub getPrimaryLanguage($)
{
    my($lcid) = @_;
    return $lcid & $LANGUAGE_MASK_PRIMARY;
}

sub getSubLanguage($)
{
    my($lcid) = @_;
    return $lcid >> 10;
}

sub makeLangID($$)
{
    my( $sub, $pri) = @_;
    return ($sub << 10) | $pri;
}


sub grepFile($$$$$)
{
    my( $regex, $modifier, $path, $module, $name) = @_;
    my @result;
    my $found = 0;
    my $file;
    # Try module under current working directory first to catch local
    # modifications. A Not yet delivered lang.h is a special case.
    if ("$path/$module/$name" eq "$SOLENVINC/i18npool/lang.h") {
        $file = "./$module/inc/i18npool/lang.h"; }
    else {
        $file = "./$module/$name"; }
    if (!($found = open( IN, $file)))
    {
        # Then with the given path.
        $file = "$path/$module/$name";
        if (!($found = open( IN, $file)))
        {
            print "No $file\n";
            $file = "$path/$module.lnk/$name";
            if (!($found = open( IN, $file))) {
                print "No $file either.\n"; }
        }
    }
    if ($found)
    {
        $found = 0;
        if ($modifier eq "i")
        {
            while (my $line = <IN>)
            {
                if ($line =~ /$regex/i)
                {
                    if (!$found)
                    {
                        $found = 1;
                        print "$file:\n";
                    }
                    chomp( $line);
                    print "$line\n";
                    push( @result, $line);
                }
            }
        }
        else
        {
            while (my $line = <IN>)
            {
                if ($line =~ /$regex/)
                {
                    if (!$found)
                    {
                        $found = 1;
                        print "$file:\n";
                    }
                    chomp( $line);
                    print "$line\n";
                    push( @result, $line);
                }
            }
        }
        close( IN);
    }
    if (!$found) {
        print "Not found in $file\n"; }
    return @result;
}


sub main()
{
    my( $lcid, @parts, $grepdef, $options, $single);
    $grepdef = 0;
    $single = 0;
    for ($options = 0; $options < @ARGV && $ARGV[$options] =~ /^--/; ++$options)
    {
        if ($ARGV[$options] eq '--single') { $single = 1; }
        else { print "Unknown option: $ARGV[$options]\n"; }
    }
    if (@ARGV == 1 + $options)
    {
        # 0x hex, 0b bin, 0 oct
        if ($ARGV[$options] =~ /^0/) {
            $lcid = oct( $ARGV[0]); }
        elsif ($ARGV[$options] =~ /^[0-9]/) {
            $lcid = $ARGV[$options]; }
        else
        {
            $grepdef = $ARGV[$options];
            $lcid = 0;
        }
        $parts[0] = getPrimaryLanguage( $lcid);
        $parts[1] = getSubLanguage( $lcid);
    }
    elsif (@ARGV == 2 + $options)
    {
        for (my $i = $options; $i < 2 + $options; ++$i)
        {
            if ($ARGV[$i] =~ /^0/) {
                $parts[$i] = oct( $ARGV[$i]); }
            else {
                $parts[$i] = $ARGV[$i]; }
        }
        $lcid = makeLangID( $parts[1], $parts[0]);
    }
    else
    {
        Usage();
        return 1;
    }
    my $modifier = "i";
    my (@resultlist, @greplist, @lcidlist, $result);
    # If no string was given on the command line, but value(s) were, lookup the
    # LangID value to obtain the define identifier.
    if ($grepdef)
    {
        # #define LANGUAGE_AFRIKAANS                  0x0436
        @resultlist = grepFile(
            '^\s*#\s*define\s+[A-Z_]*' . $grepdef, $modifier,
            $SOLENVINC, "i18npool", "lang.h");
    }
    else
    {
        printf( "LangID: 0x%04X (dec %d), primary: 0x%03x, sub 0x%02x\n", $lcid,
                $lcid, $parts[0], $parts[1]);
        my $buf = sprintf( "0x%04X", $lcid);
        @resultlist = grepFile(
            '^\s*#\s*define\s+\w+\s+' . $buf, "",
            $SOLENVINC, "i18npool", "lang.h");
    }
    for $result (@resultlist)
    {
        # #define LANGUAGE_AFRIKAANS                  0x0436
        if ($result =~ /^\s*#\s*define\s+(\w+)\s+(0x[0-9a-fA-F]+)/)
        {
            push( @greplist, '\b' . $1 . '\b');
            $modifier = "";     # complete identifier now case sensitive
            if ($single) {
                last; }
        }
    }
    # If the string given is of the form xx-yy lookup a language,country pair
    # to obtain the define identifier. xx and yy themselfs may be regexps.
    # xx- is a short form for 'xx-.*' and -yy a short form for '.*-yy'
    if ($grepdef =~ /^(.*)-$/) {
        $grepdef = $1 . "-.*"; }
    if ($grepdef =~ /^-(.*)$/) {
        $grepdef = ".*-" . $1; }
    if ($grepdef =~ /^(.*)-(.*)$/)
    {
        my $lang = $1;
        my $coun = $2;
        $lang = lc($lang);
        $coun = uc($coun);
        #     { LANGUAGE_AFRIKAANS,                   "af", "ZA" },
        @resultlist = grepFile(
            '^\s*\{\s*\w+\s*,\s*\"' . $lang . '\"\s*,\s*\"'  . $coun . '\"\s*\}\s*,', "",
            "$SRC_ROOT", "i18npool", "source/isolang/isolang.cxx");
        for $result (@resultlist)
        {
            if ($result =~ /^\s*\{\s*(\w+)\s*,\s*\"\w+\"\s*,\s*\"(\w+)?\"\s*\}\s*,/)
            {
                push( @greplist, '\b' . $1 . '\b');
                $modifier = "";     # complete identifier now case sensitive
                if ($single) {
                    last; }
            }
        }
        $grepdef = 0;
    }
    if (!@greplist && $grepdef) {
        push( @greplist, $grepdef); }
    for $grepdef (@greplist)
    {
        print "\nUsing: " . $grepdef . "\n";
        # #define LANGUAGE_AFRIKAANS                  0x0436
        @resultlist = grepFile(
            '^\s*#\s*define\s+[A-Z_]*' . $grepdef, $modifier,
            $SOLENVINC, "i18npool", "lang.h");
        my @lcidlist;
        for $result (@resultlist)
        {
            # #define LANGUAGE_AFRIKAANS                  0x0436
            if ($result =~ /^\s*#\s*define\s+(\w+)\s+(0x[0-9a-fA-F]+)/)
            {
                push( @lcidlist, oct( $2));
            }
        }

        #     { LANGUAGE_AFRIKAANS,                   "af", "ZA" },
        @resultlist = grepFile(
            '^\s*\{\s*.*' . $grepdef . '.*\s*,\s*\".*\"\s*,\s*\".*\"\s*\}\s*,', $modifier,
            "$SRC_ROOT", "i18npool", "source/isolang/isolang.cxx");

        my @langcoungreplist;
        for $result (@resultlist)
        {
            if ($result =~ /^\s*\{\s*\w+\s*,\s*\"(\w+)\"\s*,\s*\"(\w+)?\"\s*\}\s*,/)
            {
                my $lang = $1;
                my $coun = $2;
                my $loca;
                if ($coun)
                {
                    $loca = $lang . "_" . $coun;
                    push( @langcoungreplist, $lang . '(-' . $coun . ')?');
                }
                else
                {
                    $loca = $lang;
                    $coun = "";
                    push( @langcoungreplist, $lang);
                }
                my $file = "$SRC_ROOT/i18npool/source/localedata/data/$loca.xml";
                my $found;
                if (!($found = open( LD, $file)))
                {
                    $file = "$SRC_ROOT/i18npool.lnk/source/localedata/data/$loca.xml";
                    $found = open( LD, $file);
                }
                if ($found)
                {
                    print "Found $file:\n";
                    my $on = 0;
                    while (my $line = <LD>)
                    {
                        if ($line =~ /<(Language|Country)>/) {
                            $on = 1; }
                        if ($on) {
                            print $line; }
                        if ($line =~ /<\/(Language|Country)>/) {
                            $on = 0; }
                    }
                    close( LD);
                }
                else {
                    print "No $SRC_ROOT/i18npool/source/localedata/data/$loca.xml\n"; }
            }
        }

        #         case LANGUAGE_ARABIC:
        grepFile(
            '^\s*case\s*.*' . $grepdef . '.*\s*:', $modifier,
            "$SRC_ROOT", "i18npool", "source/isolang/mslangid.cxx");

        # With CWS 'langstatusbar' the language listbox resource file gets a new location.
        my $module = "svx";
        my $name = "source/dialog/langtab.src";
        if (!(-e "$SRC_ROOT/$module/$name")) {
            $module = "svtools";
            $name = "source/misc/langtab.src";
        }
        #         < "Afrikaans" ; LANGUAGE_AFRIKAANS ; > ;
        # lookup define
        @resultlist = grepFile(
            '^\s*<\s*\".*\"\s*;\s*.*' . $grepdef . '.*\s*;\s*>\s*;', $modifier,
            "$SRC_ROOT", $module, $name);
        # lookup string
        if (!@resultlist) {
            grepFile(
                '^\s*<\s*\".*' . $grepdef . '.*\"\s*;\s*.*\s*;\s*>\s*;', $modifier,
                "$SRC_ROOT", $module, $name); }

        for $lcid (@lcidlist)
        {
            # [OOO_LANGPACK_NAME_1033]
            grepFile(
                '^\s*\[OOO_LANGPACK_NAME_' . $lcid . '\]', "",
                "$SRC_ROOT", "instsetoo_native", "inc_openoffice/windows/msi_languages/Langpack.ulf");
        }

        for my $langcoun (@langcoungreplist)
        {
            # Name (xxx) = "/registry/spool/org/openoffice/Office/Common-ctl.xcu";
            grepFile(
                '^\s*Name\s*\(' . $langcoun . '\)\s*=', "",
                "$SRC_ROOT", "scp2", "source/ooo/file_ooo.scp");
        }
    }
    return 0;
}

main();
