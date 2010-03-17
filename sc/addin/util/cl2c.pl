#!/usr/solar/bin/perl

##########################################################################
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
##########################################################################

if ( $#ARGV != 3 ) {
    print STDERR "usage: cl2c.pl <file.cl> <file.c> <file.src> <resname>\n";
    exit -1;
}

$CL=$ARGV[0];
$C=$ARGV[1];
$SRC=$ARGV[2];
$RNAME=$ARGV[3];

sub sconv
{
    local($s)=@_[0];
    local($o,$c);
    $_="";
    foreach $o ( unpack("C*",$s) ) {
        $c=chr($o);
        if ( $o >= 32 && $o < 127 ) {
            $_ .= $c;
        } else {
            $_ .= sprintf("\\%o", $o);
        }
    }
    return $_;
}


sub makeneutral {

    print COUT "\n\n/**\n";
    print COUT " * Get neutral language for specific language.\n";
    print COUT " * This simplifies the getText switch cases and allows to handle\n";
    print COUT " * previously unknown language derivates due to foreign installations.\n";
    print COUT " * If you want to distinguish between some dialects change this function\n";
    print COUT " * to return the desired nLang before doing the bit masking stuff.\n";
    print COUT " * See xlang.h for defined LANGUAGE_*\n";
    print COUT " */\n";

    # taken from tools/source/intntl/intn.cxx International::GetNeutralLanguage
    print COUT "static USHORT GetNeutralLanguage( USHORT nLang )\n";
    print COUT "{\n";
    print COUT "\tUSHORT nPrimLang;\n";
    print COUT "\n";
    print COUT "\t/* ignore LANGUAGE_USER* */\n";
    print COUT "\tif ( (nLang & 0x03FF) >= 0x0200 )\n";
    print COUT "\t  return nLang;\n";
    print COUT "\n";
    print COUT "\tnLang &= 0x03FF;\n";
    print COUT "\n";
    print COUT "\tnPrimLang = nLang | 0x0400;\n";
    print COUT "\n";
    print COUT "\tswitch ( nPrimLang )\n";
    print COUT "\t{\n";
    print COUT "\t\tcase LANGUAGE_CHINESE_TRADITIONAL:\n";
    print COUT "\t\t\tnLang = LANGUAGE_CHINESE;\n";
    print COUT "\t\t\tbreak;\n";
    print COUT "\t\tcase LANGUAGE_ENGLISH_US:\n";
    print COUT "\t\t\tnLang = LANGUAGE_ENGLISH;\n";
    print COUT "\t\t\tbreak;\n";
    print COUT "\t\tcase LANGUAGE_NORWEGIAN_BOKMAL:\n";
    print COUT "\t\t\tnLang = LANGUAGE_NORWEGIAN;\n";
    print COUT "\t\t\tbreak;\n";
    print COUT "\t\tcase LANGUAGE_PORTUGUESE_BRAZILIAN:\n";
    print COUT "\t\t\tnLang = LANGUAGE_PORTUGUESE;\n";
    print COUT "\t\t\tbreak;\n";
    print COUT "\n";
    print COUT "\t\tdefault:\n";
    print COUT "\t\t\tnLang = nPrimLang;\n";
    print COUT "\t\t\tbreak;\n";
    print COUT "\t}\n";
    print COUT "\n";
    print COUT "\treturn nLang;\n";
    print COUT "}\n";
    print COUT "\n";

}


sub maketext {

    print COUT "\n\n/**\n";
    print COUT " * Get text resource for current language.\n";
    print COUT " * Remember that 8-bit characters are shown in\n";
    print COUT " * system dependend code pages!\n";
    print COUT " * To get correct results you will have to distuinguish\n";
    print COUT " * for example between UNIX and WIN and OS2 target systems.\n";
    print COUT " */\n";

    print COUT "static char* getText( int nResource )\n{\n";
    print COUT "\tswitch( nResource ) {\n";

    $resflag=0;
    $strname="";
    $cnt=0;
    $text_english="";

    while (<SRCIN>) {
        $resflag=1 if ( /Resource\s$RNAME/ );

        if ( /\{/ ) {
            if ( ++$cnt == 2 ) {
                # start language
                $text_english="";
                print COUT "\t\t\tswitch( _nLanguage ) {\n";
                next;
            }
        }

        if ( /\}/ ) {
            if ( --$cnt == 1 ) {
                # end language

                if ( $text_english ne "" ) {
                    print COUT "\t\t\t\tcase LANGUAGE_ENGLISH:\n\t\t\t\tdefault:\n";
                    print COUT "\t\t\t\treturn(" . $text_english . ")\;\n";
                }

                print COUT "\t\t\t}\n\t\t\tbreak;\n";
                next;
            } elsif ( $cnt == 0 ) {
                # end of resource
                $resflag=0;
                print COUT "\t\tdefault:\n\t\t\tbreak;\n";
                print COUT "\t}\n\treturn(\"\");\n}\n";
                next;
            }

        }

        if ( $resflag && $cnt == 1) {
            if ( /\sString\s(([A-Z]|\_|[0-9]|[a-z])*)/ ) {
                $strname=$1;
                print COUT "\t\tcase " . $strname . ":\n";
            }
        }

        if ( $cnt == 2 && /^\s*Text/ ) {
            $langname="german";
            ($textdef,@textx)=split(/=/);
            $text=join("=",@textx);
            if ( $textdef =~ /\[\s+(.*)\s+\]/ ) {
                $langname=$1;
            }
            else {
                $langname="ENGLISH_US";     # no [...] => not to be translated
            }

            $langname="LANGUAGE_" . uc($langname);

            chop($text) while ( $text=~/(\r|\n|\;)$/ );
            $text=sconv($text);
            # english_us, not english because it's developer's pigeon
            if ( $langname eq "LANGUAGE_ENGLISH_US" ) {
                $text_english=$text;
            }
            # ISO coded, obtain at least the default
            elsif ( $langname =~ /^LANGUAGE_EN-US$/ ) {
                $text_english=$text;
            }
            # we don't know about USER languages, ENGLISH will be appended later
            elsif ( ! ( $langname =~ /LANGUAGE_USER/ || $langname =~ /^LANGUAGE_ENGLISH$/ ) ) {
                # ER 28.04.99: for the moment only German and English are
                # exported, because we have a problem with non-existing
                # character code tables for systems other than Windoze.
                # => Chinese would be definitely mixed up and we don't
                # want to insult anybody.. others like Spanish would look
                # very ugly, but we'll have to live with bad German Umlauts.
                if ( $langname =~ /LANGUAGE_(GERMAN|ENGLISH)/ ) {
                    print COUT "\t\t\t\tcase " . $langname . ":\n";
                    print COUT "\t\t\t\treturn(" . $text . ")\;\n";
                }
            }

        }
    }

    makeneutral();

}

open(CLIN,"<$CL") || die "can not open $CL\n";
open(SRCIN,"<$SRC") || die "can not open $CL\n";
open(COUT,">$C") || die "can not open $CL\n";

$ccnt=0;
$incomment=0;
while(<CLIN>) {
    if ( /^\/\*--(-*)/ ) {
        $incomment=1;
        $ccnt++;
    }

    print COUT $_ if ( $incomment==0 || $ccnt==1 );

    &maketext() if ( /^static USHORT _nLanguage=/ );

    if ( /(-*)--\*\/$/ ) {
        $incomment=0;
    }

}

close(CLIN);
close(SRCIN);
close(COUT);

exit 0;


