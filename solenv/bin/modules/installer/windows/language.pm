#*************************************************************************
#
#   $RCSfile: language.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: svesik $ $Date: 2004-04-20 12:33:30 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

package installer::windows::language;

use installer::exiter;

#############################################
# Determining the Windows language of a file
# This list has to be removed from this
# script as soon as possible.
# FAKE (?)
#############################################

sub get_windows_language
{
    my ($language) = @_;

    my $windowslangugage = "";

    if (( $language eq "01" ) || ( $language eq "en" ) || ( $language eq "en-US" )) { $windowslanguage = "1033"; }
    elsif (( $language eq "03" ) || ( $language eq "pt" ) || ( $language eq "pt-PT" )) { $windowslanguage = "2070"; }
    elsif (( $language eq "07" ) || ( $language eq "ru" )) { $windowslanguage = "1049"; }
    elsif (( $language eq "30" ) || ( $language eq "el" )) { $windowslanguage = "1032"; }
    elsif (( $language eq "31" ) || ( $language eq "nl" )) { $windowslanguage = "1043"; }
    elsif (( $language eq "33" ) || ( $language eq "fr" )) { $windowslanguage = "1036"; }
    elsif (( $language eq "34" ) || ( $language eq "es" )) { $windowslanguage = "1034"; }
    elsif (( $language eq "35" ) || ( $language eq "fi" )) { $windowslanguage = "1035"; }
    elsif (( $language eq "36" ) || ( $language eq "hu" )) { $windowslanguage = "1038"; }
    elsif (( $language eq "37" ) || ( $language eq "ca" )) { $windowslanguage = "1027"; }   # Catalan
    elsif (( $language eq "39" ) || ( $language eq "it" )) { $windowslanguage = "1040"; }
    elsif (( $language eq "42" ) || ( $language eq "cs" )) { $windowslanguage = "1029"; }
    elsif (( $language eq "43" ) || ( $language eq "sk" )) { $windowslanguage = "1051"; }
    elsif (( $language eq "44" ) || ( $language eq "en-GB" )) { $windowslanguage = "2057"; }
    elsif (( $language eq "45" ) || ( $language eq "da" )) { $windowslanguage = "1030"; }
    elsif (( $language eq "46" ) || ( $language eq "sv" )) { $windowslanguage = "1053"; }
    elsif (( $language eq "47" ) || ( $language eq "no" )) { $windowslanguage = "1044"; }
    elsif (( $language eq "48" ) || ( $language eq "pl" )) { $windowslanguage = "1045"; }
    elsif (( $language eq "49" ) || ( $language eq "de" )) { $windowslanguage = "1031"; }
    elsif (( $language eq "55" ) || ( $language eq "pt-BR" )) { $windowslanguage = "1046"; }
    elsif (( $language eq "66" ) || ( $language eq "th" )) { $windowslanguage = "1054"; }
    elsif (( $language eq "77" ) || ( $language eq "et" )) { $windowslanguage = "1061"; }
    elsif (( $language eq "81" ) || ( $language eq "ja" )) { $windowslanguage = "1041"; }
    elsif (( $language eq "82" ) || ( $language eq "ko" )) { $windowslanguage = "1042"; }
    elsif (( $language eq "86" ) || ( $language eq "zh-CN" )) { $windowslanguage = "2052"; }
    elsif (( $language eq "88" ) || ( $language eq "zh-TW" )) { $windowslanguage = "1028"; }
    elsif (( $language eq "90" ) || ( $language eq "tr" )) { $windowslanguage = "1055"; }
    elsif (( $language eq "91" ) || ( $language eq "hi" ) || ( $language eq "hi-IN" )) { $windowslanguage = "1081"; }
    elsif (( $language eq "96" ) || ( $language eq "ar" ) || ( $language eq "ar-SA" )) { $windowslanguage = "1025"; }
    elsif (( $language eq "97" ) || ( $language eq "he" )) { $windowslanguage = "1037"; }
    elsif ( $language eq "af" ) { $windowslanguage = "1078"; }   # Afrikaans
    elsif ( $language eq "sq" ) { $windowslanguage = "1052"; }   # Albanian
    elsif ( $language eq "hy" ) { $windowslanguage = "1067"; }   # Armenian
    elsif ( $language eq "eu" ) { $windowslanguage = "1069"; }   # Basque
    elsif ( $language eq "be" ) { $windowslanguage = "1059"; }   # Belarusian
    elsif ( $language eq "bg" ) { $windowslanguage = "1026"; }   # Bulgarian
    elsif ( $language eq "is" ) { $windowslanguage = "1039"; }   # Icelandic
    elsif ( $language eq "id" ) { $windowslanguage = "1057"; }   # Indonesian
    elsif ( $language eq "lv" ) { $windowslanguage = "1062"; }   # Latvian
    elsif ( $language eq "lt" ) { $windowslanguage = "1063"; }   # Lithuanian
    elsif ( $language eq "mt" ) { $windowslanguage = "1082"; }   # Maltese
    elsif ( $language eq "mr" ) { $windowslanguage = "1102"; }   # Marathi
    elsif ( $language eq "rm" ) { $windowslanguage = "1047"; }   # Raeto-Romance
    elsif ( $language eq "ro" ) { $windowslanguage = "1048"; }   # Romanian
    elsif ( $language eq "sa" ) { $windowslanguage = "1103"; }   # Sanskrit
    elsif ( $language eq "tn" ) { $windowslanguage = "1074"; }   # Setsuana
    elsif ( $language eq "hr" ) { $windowslanguage = "1050"; }   # Croatian
    elsif ( $language eq "fa" ) { $windowslanguage = "1065"; }   # Farsi
    elsif ( $language eq "fo" ) { $windowslanguage = "1080"; }   # Faroese
    elsif ( $language eq "sl" ) { $windowslanguage = "1060"; }   # Slovenian
    elsif ( $language eq "sb" ) { $windowslanguage = "1070"; }   # Sorbian
    elsif ( $language eq "sx" ) { $windowslanguage = "1072"; }   # Sutu
    elsif ( $language eq "sw" ) { $windowslanguage = "1089"; }   # Swahili
    elsif ( $language eq "ta" ) { $windowslanguage = "1097"; }   # Tamil
    elsif ( $language eq "tt" ) { $windowslanguage = "1092"; }   # Tatar
    elsif ( $language eq "ts" ) { $windowslanguage = "1073"; }   # Tsonga
    elsif ( $language eq "uk" ) { $windowslanguage = "1058"; }   # Ukrainian
    elsif ( $language eq "ur" ) { $windowslanguage = "1056"; }   # Urdu
    elsif ( $language eq "vi" ) { $windowslanguage = "1066"; }   # Vietnamese
    elsif ( $language eq "xh" ) { $windowslanguage = "1076"; }   # Xhosa
    elsif ( $language eq "yi" ) { $windowslanguage = "1085"; }   # Yiddish
    elsif ( $language eq "zu" ) { $windowslanguage = "1077"; }   # Zulu
    else
    {
        installer::exiter::exit_program("ERROR: Unknown language $language in function get_windows_language", "get_windows_language");
    }

    return $windowslanguage
}

1;