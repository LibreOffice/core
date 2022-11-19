/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cstdio>
#include <cstring>
#include <vector>
#include <assert.h>

#include <fontsubset.hxx>

#include <comphelper/flagguard.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/sprintf.hxx>
#include <rtl/math.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <strhelper.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>

typedef sal_uInt8 U8;
typedef sal_uInt16 U16;
typedef sal_Int64 S64;

typedef double RealType;
typedef RealType ValType;

constexpr OStringLiteral tok_notdef = ".notdef";
constexpr OStringLiteral tok_space = "space";
constexpr OStringLiteral tok_exclam = "exclam";
constexpr OStringLiteral tok_quotedbl = "quotedbl";
constexpr OStringLiteral tok_numbersign = "numbersign";
constexpr OStringLiteral tok_dollar = "dollar";
constexpr OStringLiteral tok_percent = "percent";
constexpr OStringLiteral tok_ampersand = "ampersand";
constexpr OStringLiteral tok_quoteright = "quoteright";
constexpr OStringLiteral tok_parenleft = "parenleft";
constexpr OStringLiteral tok_parenright = "parenright";
constexpr OStringLiteral tok_asterisk = "asterisk";
constexpr OStringLiteral tok_plus = "plus";
constexpr OStringLiteral tok_comma = "comma";
constexpr OStringLiteral tok_hyphen = "hyphen";
constexpr OStringLiteral tok_period = "period";
constexpr OStringLiteral tok_slash = "slash";
constexpr OStringLiteral tok_zero = "zero";
constexpr OStringLiteral tok_one = "one";
constexpr OStringLiteral tok_two = "two";
constexpr OStringLiteral tok_three = "three";
constexpr OStringLiteral tok_four = "four";
constexpr OStringLiteral tok_five = "five";
constexpr OStringLiteral tok_six = "six";
constexpr OStringLiteral tok_seven = "seven";
constexpr OStringLiteral tok_eight = "eight";
constexpr OStringLiteral tok_nine = "nine";
constexpr OStringLiteral tok_colon = "colon";
constexpr OStringLiteral tok_semicolon = "semicolon";
constexpr OStringLiteral tok_less = "less";
constexpr OStringLiteral tok_equal = "equal";
constexpr OStringLiteral tok_greater = "greater";
constexpr OStringLiteral tok_question = "question";
constexpr OStringLiteral tok_at = "at";
constexpr OStringLiteral tok_A = "A";
constexpr OStringLiteral tok_B = "B";
constexpr OStringLiteral tok_C = "C";
constexpr OStringLiteral tok_D = "D";
constexpr OStringLiteral tok_E = "E";
constexpr OStringLiteral tok_F = "F";
constexpr OStringLiteral tok_G = "G";
constexpr OStringLiteral tok_H = "H";
constexpr OStringLiteral tok_I = "I";
constexpr OStringLiteral tok_J = "J";
constexpr OStringLiteral tok_K = "K";
constexpr OStringLiteral tok_L = "L";
constexpr OStringLiteral tok_M = "M";
constexpr OStringLiteral tok_N = "N";
constexpr OStringLiteral tok_O = "O";
constexpr OStringLiteral tok_P = "P";
constexpr OStringLiteral tok_Q = "Q";
constexpr OStringLiteral tok_R = "R";
constexpr OStringLiteral tok_S = "S";
constexpr OStringLiteral tok_T = "T";
constexpr OStringLiteral tok_U = "U";
constexpr OStringLiteral tok_V = "V";
constexpr OStringLiteral tok_W = "W";
constexpr OStringLiteral tok_X = "X";
constexpr OStringLiteral tok_Y = "Y";
constexpr OStringLiteral tok_Z = "Z";
constexpr OStringLiteral tok_bracketleft = "bracketleft";
constexpr OStringLiteral tok_backslash = "backslash";
constexpr OStringLiteral tok_bracketright = "bracketright";
constexpr OStringLiteral tok_asciicircum = "asciicircum";
constexpr OStringLiteral tok_underscore = "underscore";
constexpr OStringLiteral tok_quoteleft = "quoteleft";
constexpr OStringLiteral tok_a = "a";
constexpr OStringLiteral tok_b = "b";
constexpr OStringLiteral tok_c = "c";
constexpr OStringLiteral tok_d = "d";
constexpr OStringLiteral tok_e = "e";
constexpr OStringLiteral tok_f = "f";
constexpr OStringLiteral tok_g = "g";
constexpr OStringLiteral tok_h = "h";
constexpr OStringLiteral tok_i = "i";
constexpr OStringLiteral tok_j = "j";
constexpr OStringLiteral tok_k = "k";
constexpr OStringLiteral tok_l = "l";
constexpr OStringLiteral tok_m = "m";
constexpr OStringLiteral tok_n = "n";
constexpr OStringLiteral tok_o = "o";
constexpr OStringLiteral tok_p = "p";
constexpr OStringLiteral tok_q = "q";
constexpr OStringLiteral tok_r = "r";
constexpr OStringLiteral tok_s = "s";
constexpr OStringLiteral tok_t = "t";
constexpr OStringLiteral tok_u = "u";
constexpr OStringLiteral tok_v = "v";
constexpr OStringLiteral tok_w = "w";
constexpr OStringLiteral tok_x = "x";
constexpr OStringLiteral tok_y = "y";
constexpr OStringLiteral tok_z = "z";
constexpr OStringLiteral tok_braceleft = "braceleft";
constexpr OStringLiteral tok_bar = "bar";
constexpr OStringLiteral tok_braceright = "braceright";
constexpr OStringLiteral tok_asciitilde = "asciitilde";
constexpr OStringLiteral tok_exclamdown = "exclamdown";
constexpr OStringLiteral tok_cent = "cent";
constexpr OStringLiteral tok_sterlin = "sterlin";
constexpr OStringLiteral tok_fraction = "fraction";
constexpr OStringLiteral tok_yen = "yen";
constexpr OStringLiteral tok_florin = "florin";
constexpr OStringLiteral tok_section = "section";
constexpr OStringLiteral tok_currency = "currency";
constexpr OStringLiteral tok_quotesingle = "quotesingle";
constexpr OStringLiteral tok_quotedblleft = "quotedblleft";
constexpr OStringLiteral tok_guillemotleft = "guillemotleft";
constexpr OStringLiteral tok_guilsinglleft = "guilsinglleft";
constexpr OStringLiteral tok_guilsinglright = "guilsinglright";
constexpr OStringLiteral tok_fi = "fi";
constexpr OStringLiteral tok_fl = "fl";
constexpr OStringLiteral tok_endash = "endash";
constexpr OStringLiteral tok_dagger = "dagger";
constexpr OStringLiteral tok_daggerdbl = "daggerdbl";
constexpr OStringLiteral tok_periodcentered = "periodcentered";
constexpr OStringLiteral tok_paragraph = "paragraph";
constexpr OStringLiteral tok_bullet = "bullet";
constexpr OStringLiteral tok_quotesinglbase = "quotesinglbase";
constexpr OStringLiteral tok_quotedblbase = "quotedblbase";
constexpr OStringLiteral tok_quotedblright = "quotedblright";
constexpr OStringLiteral tok_guillemotright = "guillemotright";
constexpr OStringLiteral tok_ellipsis = "ellipsis";
constexpr OStringLiteral tok_perthousand = "perthousand";
constexpr OStringLiteral tok_questiondown = "questiondown";
constexpr OStringLiteral tok_grave = "grave";
constexpr OStringLiteral tok_acute = "acute";
constexpr OStringLiteral tok_circumflex = "circumflex";
constexpr OStringLiteral tok_tilde = "tilde";
constexpr OStringLiteral tok_macron = "macron";
constexpr OStringLiteral tok_breve = "breve";
constexpr OStringLiteral tok_dotaccent = "dotaccent";
constexpr OStringLiteral tok_dieresis = "dieresis";
constexpr OStringLiteral tok_ring = "ring";
constexpr OStringLiteral tok_cedilla = "cedilla";
constexpr OStringLiteral tok_hungarumlaut = "hungarumlaut";
constexpr OStringLiteral tok_ogonek = "ogonek";
constexpr OStringLiteral tok_caron = "caron";
constexpr OStringLiteral tok_emdash = "emdash";
constexpr OStringLiteral tok_AE = "AE";
constexpr OStringLiteral tok_ordfeminine = "ordfeminine";
constexpr OStringLiteral tok_Lslash = "Lslash";
constexpr OStringLiteral tok_Oslash = "Oslash";
constexpr OStringLiteral tok_OE = "OE";
constexpr OStringLiteral tok_ordmasculine = "ordmasculine";
constexpr OStringLiteral tok_ae = "ae";
constexpr OStringLiteral tok_dotlessi = "dotlessi";
constexpr OStringLiteral tok_lslash = "lslash";
constexpr OStringLiteral tok_oslash = "oslash";
constexpr OStringLiteral tok_oe = "oe";
constexpr OStringLiteral tok_germandbls = "germandbls";
constexpr OStringLiteral tok_onesuperior = "onesuperior";
constexpr OStringLiteral tok_logicalnot = "logicalnot";
constexpr OStringLiteral tok_mu = "mu";
constexpr OStringLiteral tok_trademark = "trademark";
constexpr OStringLiteral tok_Eth = "Eth";
constexpr OStringLiteral tok_onehalf = "onehalf";
constexpr OStringLiteral tok_plusminus = "plusminus";
constexpr OStringLiteral tok_Thorn = "Thorn";
constexpr OStringLiteral tok_onequarter = "onequarter";
constexpr OStringLiteral tok_divide = "divide";
constexpr OStringLiteral tok_brokenbar = "brokenbar";
constexpr OStringLiteral tok_degree = "degree";
constexpr OStringLiteral tok_thorn = "thorn";
constexpr OStringLiteral tok_threequarters = "threequarters";
constexpr OStringLiteral tok_twosuperior = "twosuperior";
constexpr OStringLiteral tok_registered = "registered";
constexpr OStringLiteral tok_minus = "minus";
constexpr OStringLiteral tok_eth = "eth";
constexpr OStringLiteral tok_multiply = "multiply";
constexpr OStringLiteral tok_threesuperior = "threesuperior";
constexpr OStringLiteral tok_copyright = "copyright";
constexpr OStringLiteral tok_Aacute = "Aacute";
constexpr OStringLiteral tok_Acircumflex = "Acircumflex";
constexpr OStringLiteral tok_Adieresis = "Adieresis";
constexpr OStringLiteral tok_Agrave = "Agrave";
constexpr OStringLiteral tok_Aring = "Aring";
constexpr OStringLiteral tok_Atilde = "Atilde";
constexpr OStringLiteral tok_Ccedilla = "Ccedilla";
constexpr OStringLiteral tok_Eacute = "Eacute";
constexpr OStringLiteral tok_Ecircumflex = "Ecircumflex";
constexpr OStringLiteral tok_Edieresis = "Edieresis";
constexpr OStringLiteral tok_Egrave = "Egrave";
constexpr OStringLiteral tok_Iacute = "Iacute";
constexpr OStringLiteral tok_Icircumflex = "Icircumflex";
constexpr OStringLiteral tok_Idieresis = "Idieresis";
constexpr OStringLiteral tok_Igrave = "Igrave";
constexpr OStringLiteral tok_Ntilde = "Ntilde";
constexpr OStringLiteral tok_Oacute = "Oacute";
constexpr OStringLiteral tok_Ocircumflex = "Ocircumflex";
constexpr OStringLiteral tok_Odieresis = "Odieresis";
constexpr OStringLiteral tok_Ograve = "Ograve";
constexpr OStringLiteral tok_Otilde = "Otilde";
constexpr OStringLiteral tok_Scaron = "Scaron";
constexpr OStringLiteral tok_Uacute = "Uacute";
constexpr OStringLiteral tok_Ucircumflex = "Ucircumflex";
constexpr OStringLiteral tok_Udieresis = "Udieresis";
constexpr OStringLiteral tok_Ugrave = "Ugrave";
constexpr OStringLiteral tok_Yacute = "Yacute";
constexpr OStringLiteral tok_Ydieresis = "Ydieresis";
constexpr OStringLiteral tok_Zcaron = "Zcaron";
constexpr OStringLiteral tok_aacute = "aacute";
constexpr OStringLiteral tok_acircumflex = "acircumflex";
constexpr OStringLiteral tok_adieresis = "adieresis";
constexpr OStringLiteral tok_agrave = "agrave";
constexpr OStringLiteral tok_aring = "aring";
constexpr OStringLiteral tok_atilde = "atilde";
constexpr OStringLiteral tok_ccedilla = "ccedilla";
constexpr OStringLiteral tok_eacute = "eacute";
constexpr OStringLiteral tok_ecircumflex = "ecircumflex";
constexpr OStringLiteral tok_edieresis = "edieresis";
constexpr OStringLiteral tok_egrave = "egrave";
constexpr OStringLiteral tok_iacute = "iacute";
constexpr OStringLiteral tok_icircumflex = "icircumflex";
constexpr OStringLiteral tok_idieresis = "idieresis";
constexpr OStringLiteral tok_igrave = "igrave";
constexpr OStringLiteral tok_ntilde = "ntilde";
constexpr OStringLiteral tok_oacute = "oacute";
constexpr OStringLiteral tok_ocircumflex = "ocircumflex";
constexpr OStringLiteral tok_odieresis = "odieresis";
constexpr OStringLiteral tok_ograve = "ograve";
constexpr OStringLiteral tok_otilde = "otilde";
constexpr OStringLiteral tok_scaron = "scaron";
constexpr OStringLiteral tok_uacute = "uacute";
constexpr OStringLiteral tok_ucircumflex = "ucircumflex";
constexpr OStringLiteral tok_udieresis = "udieresis";
constexpr OStringLiteral tok_ugrave = "ugrave";
constexpr OStringLiteral tok_yacute = "yacute";
constexpr OStringLiteral tok_ydieresis = "ydieresis";
constexpr OStringLiteral tok_zcaron = "zcaron";
constexpr OStringLiteral tok_exclamsmall = "exclamsmall";
constexpr OStringLiteral tok_Hungarumlautsmall = "Hungarumlautsmall";
constexpr OStringLiteral tok_dollaroldstyle = "dollaroldstyle";
constexpr OStringLiteral tok_dollarsuperior = "dollarsuperior";
constexpr OStringLiteral tok_ampersandsmall = "ampersandsmall";
constexpr OStringLiteral tok_Acutesmall = "Acutesmall";
constexpr OStringLiteral tok_parenleftsuperior = "parenleftsuperior";
constexpr OStringLiteral tok_parenrightsuperior = "parenrightsuperior";
constexpr OStringLiteral tok_twodotenleader = "twodotenleader";
constexpr OStringLiteral tok_onedotenleader = "onedotenleader";
constexpr OStringLiteral tok_zerooldstyle = "zerooldstyle";
constexpr OStringLiteral tok_oneoldstyle = "oneoldstyle";
constexpr OStringLiteral tok_twooldstyle = "twooldstyle";
constexpr OStringLiteral tok_threeoldstyle = "threeoldstyle";
constexpr OStringLiteral tok_fouroldstyle = "fouroldstyle";
constexpr OStringLiteral tok_fiveoldstyle = "fiveoldstyle";
constexpr OStringLiteral tok_sixoldstyle = "sixoldstyle";
constexpr OStringLiteral tok_sevenoldstyle = "sevenoldstyle";
constexpr OStringLiteral tok_eightoldstyle = "eightoldstyle";
constexpr OStringLiteral tok_nineoldstile = "nineoldstile";
constexpr OStringLiteral tok_commasuperior = "commasuperior";
constexpr OStringLiteral tok_threequartersemdash = "threequartersemdash";
constexpr OStringLiteral tok_periodsuperior = "periodsuperior";
constexpr OStringLiteral tok_questionsmall = "questionsmall";
constexpr OStringLiteral tok_asuperior = "asuperior";
constexpr OStringLiteral tok_bsuperior = "bsuperior";
constexpr OStringLiteral tok_centsuperior = "centsuperior";
constexpr OStringLiteral tok_dsuperior = "dsuperior";
constexpr OStringLiteral tok_esuperior = "esuperior";
constexpr OStringLiteral tok_isuperior = "isuperior";
constexpr OStringLiteral tok_lsuperior = "lsuperior";
constexpr OStringLiteral tok_msuperior = "msuperior";
constexpr OStringLiteral tok_nsuperior = "nsuperior";
constexpr OStringLiteral tok_osuperior = "osuperior";
constexpr OStringLiteral tok_rsuperior = "rsuperior";
constexpr OStringLiteral tok_ssuperior = "ssuperior";
constexpr OStringLiteral tok_tsuperior = "tsuperior";
constexpr OStringLiteral tok_ff = "ff";
constexpr OStringLiteral tok_ffi = "ffi";
constexpr OStringLiteral tok_ffl = "ffl";
constexpr OStringLiteral tok_parenleftinferior = "parenleftinferior";
constexpr OStringLiteral tok_parenrightinferior = "parenrightinferior";
constexpr OStringLiteral tok_Circumflexsmall = "Circumflexsmall";
constexpr OStringLiteral tok_hyphensuperior = "hyphensuperior";
constexpr OStringLiteral tok_Gravesmall = "Gravesmall";
constexpr OStringLiteral tok_Asmall = "Asmall";
constexpr OStringLiteral tok_Bsmall = "Bsmall";
constexpr OStringLiteral tok_Csmall = "Csmall";
constexpr OStringLiteral tok_Dsmall = "Dsmall";
constexpr OStringLiteral tok_Esmall = "Esmall";
constexpr OStringLiteral tok_Fsmall = "Fsmall";
constexpr OStringLiteral tok_Gsmall = "Gsmall";
constexpr OStringLiteral tok_Hsmall = "Hsmall";
constexpr OStringLiteral tok_Ismall = "Ismall";
constexpr OStringLiteral tok_Jsmall = "Jsmall";
constexpr OStringLiteral tok_Ksmall = "Ksmall";
constexpr OStringLiteral tok_Lsmall = "Lsmall";
constexpr OStringLiteral tok_Msmall = "Msmall";
constexpr OStringLiteral tok_Nsmall = "Nsmall";
constexpr OStringLiteral tok_Osmall = "Osmall";
constexpr OStringLiteral tok_Psmall = "Psmall";
constexpr OStringLiteral tok_Qsmall = "Qsmall";
constexpr OStringLiteral tok_Rsmall = "Rsmall";
constexpr OStringLiteral tok_Ssmall = "Ssmall";
constexpr OStringLiteral tok_Tsmall = "Tsmall";
constexpr OStringLiteral tok_Usmall = "Usmall";
constexpr OStringLiteral tok_Vsmall = "Vsmall";
constexpr OStringLiteral tok_Wsmall = "Wsmall";
constexpr OStringLiteral tok_Xsmall = "Xsmall";
constexpr OStringLiteral tok_Ysmall = "Ysmall";
constexpr OStringLiteral tok_Zsmall = "Zsmall";
constexpr OStringLiteral tok_colonmonetary = "colonmonetary";
constexpr OStringLiteral tok_onefitted = "onefitted";
constexpr OStringLiteral tok_rupia = "rupia";
constexpr OStringLiteral tok_Tildesmall = "Tildesmall";
constexpr OStringLiteral tok_exclamdownsmall = "exclamdownsmall";
constexpr OStringLiteral tok_centoldstyle = "centoldstyle";
constexpr OStringLiteral tok_Lslashsmall = "Lslashsmall";
constexpr OStringLiteral tok_Scaronsmall = "Scaronsmall";
constexpr OStringLiteral tok_Zcaronsmall = "Zcaronsmall";
constexpr OStringLiteral tok_Dieresissmall = "Dieresissmall";
constexpr OStringLiteral tok_Brevesmall = "Brevesmall";
constexpr OStringLiteral tok_Caronsmall = "Caronsmall";
constexpr OStringLiteral tok_Dotaccentsmall = "Dotaccentsmall";
constexpr OStringLiteral tok_Macronsmall = "Macronsmall";
constexpr OStringLiteral tok_figuredash = "figuredash";
constexpr OStringLiteral tok_hypheninferior = "hypheninferior";
constexpr OStringLiteral tok_Ogoneksmall = "Ogoneksmall";
constexpr OStringLiteral tok_Ringsmall = "Ringsmall";
constexpr OStringLiteral tok_Cedillasmall = "Cedillasmall";
constexpr OStringLiteral tok_questiondownsmall = "questiondownsmall";
constexpr OStringLiteral tok_oneeight = "oneeight";
constexpr OStringLiteral tok_threeeights = "threeeights";
constexpr OStringLiteral tok_fiveeights = "fiveeights";
constexpr OStringLiteral tok_seveneights = "seveneights";
constexpr OStringLiteral tok_onethird = "onethird";
constexpr OStringLiteral tok_twothirds = "twothirds";
constexpr OStringLiteral tok_zerosuperior = "zerosuperior";
constexpr OStringLiteral tok_foursuperior = "foursuperior";
constexpr OStringLiteral tok_fivesuperior = "fivesuperior";
constexpr OStringLiteral tok_sixsuperior = "sixsuperior";
constexpr OStringLiteral tok_sevensuperior = "sevensuperior";
constexpr OStringLiteral tok_eightsuperior = "eightsuperior";
constexpr OStringLiteral tok_ninesuperior = "ninesuperior";
constexpr OStringLiteral tok_zeroinferior = "zeroinferior";
constexpr OStringLiteral tok_oneinferior = "oneinferior";
constexpr OStringLiteral tok_twoinferior = "twoinferior";
constexpr OStringLiteral tok_threeinferior = "threeinferior";
constexpr OStringLiteral tok_fourinferior = "fourinferior";
constexpr OStringLiteral tok_fiveinferior = "fiveinferior";
constexpr OStringLiteral tok_sixinferior = "sixinferior";
constexpr OStringLiteral tok_seveninferior = "seveninferior";
constexpr OStringLiteral tok_eightinferior = "eightinferior";
constexpr OStringLiteral tok_nineinferior = "nineinferior";
constexpr OStringLiteral tok_centinferior = "centinferior";
constexpr OStringLiteral tok_dollarinferior = "dollarinferior";
constexpr OStringLiteral tok_periodinferior = "periodinferior";
constexpr OStringLiteral tok_commainferior = "commainferior";
constexpr OStringLiteral tok_Agravesmall = "Agravesmall";
constexpr OStringLiteral tok_Aacutesmall = "Aacutesmall";
constexpr OStringLiteral tok_Acircumflexsmall = "Acircumflexsmall";
constexpr OStringLiteral tok_Atildesmall = "Atildesmall";
constexpr OStringLiteral tok_Adieresissmall = "Adieresissmall";
constexpr OStringLiteral tok_Aringsmall = "Aringsmall";
constexpr OStringLiteral tok_AEsmall = "AEsmall";
constexpr OStringLiteral tok_Ccedillasmall = "Ccedillasmall";
constexpr OStringLiteral tok_Egravesmall = "Egravesmall";
constexpr OStringLiteral tok_Eacutesmall = "Eacutesmall";
constexpr OStringLiteral tok_Ecircumflexsmall = "Ecircumflexsmall";
constexpr OStringLiteral tok_Edieresissmall = "Edieresissmall";
constexpr OStringLiteral tok_Igravesmall = "Igravesmall";
constexpr OStringLiteral tok_Iacutesmall = "Iacutesmall";
constexpr OStringLiteral tok_Icircumflexsmall = "Icircumflexsmall";
constexpr OStringLiteral tok_Idieresissmall = "Idieresissmall";
constexpr OStringLiteral tok_Ethsmall = "Ethsmall";
constexpr OStringLiteral tok_Ntildesmall = "Ntildesmall";
constexpr OStringLiteral tok_Ogravesmall = "Ogravesmall";
constexpr OStringLiteral tok_Oacutesmall = "Oacutesmall";
constexpr OStringLiteral tok_Ocircumflexsmall = "Ocircumflexsmall";
constexpr OStringLiteral tok_Otildesmall = "Otildesmall";
constexpr OStringLiteral tok_Odieressissmall = "Odieressissmall";
constexpr OStringLiteral tok_OEsmall = "OEsmall";
constexpr OStringLiteral tok_Oslashsmall = "Oslashsmall";
constexpr OStringLiteral tok_Ugravesmall = "Ugravesmall";
constexpr OStringLiteral tok_Uacutesmall = "Uacutesmall";
constexpr OStringLiteral tok_Ucircumflexsmall = "Ucircumflexsmall";
constexpr OStringLiteral tok_Udieresissmall = "Udieresissmall";
constexpr OStringLiteral tok_Yacutesmall = "Yacutesmall";
constexpr OStringLiteral tok_Thornsmall = "Thornsmall";
constexpr OStringLiteral tok_Ydieresissmall = "Ydieresissmall";
constexpr OStringLiteral tok_001_000 = "001.000";
constexpr OStringLiteral tok_001_001 = "001.001";
constexpr OStringLiteral tok_001_002 = "001.002";
constexpr OStringLiteral tok_001_003 = "001.003";
constexpr OStringLiteral tok_Black = "Black";
constexpr OStringLiteral tok_Bold = "Bold";
constexpr OStringLiteral tok_Book = "Book";
constexpr OStringLiteral tok_Light = "Light";
constexpr OStringLiteral tok_Medium = "Medium";
constexpr OStringLiteral tok_Regular = "Regular";
constexpr OStringLiteral tok_Roman = "Roman";
constexpr OStringLiteral tok_Semibold = "Semibold";

static rtl::OStringConstExpr pStringIds[] = {
/*0*/   tok_notdef,       tok_space,            tok_exclam,           tok_quotedbl,
    tok_numbersign,       tok_dollar,           tok_percent,          tok_ampersand,
    tok_quoteright,       tok_parenleft,        tok_parenright,       tok_asterisk,
    tok_plus,             tok_comma,            tok_hyphen,           tok_period,
/*16*/  tok_slash,        tok_zero,             tok_one,              tok_two,
    tok_three,            tok_four,             tok_five,             tok_six,
    tok_seven,            tok_eight,            tok_nine,             tok_colon,
    tok_semicolon,        tok_less,             tok_equal,            tok_greater,
/*32*/  tok_question,     tok_at,               tok_A,                tok_B,
    tok_C,                tok_D,                tok_E,                tok_F,
    tok_G,                tok_H,                tok_I,                tok_J,
    tok_K,                tok_L,                tok_M,                tok_N,
/*48*/  tok_O,            tok_P,                tok_Q,                tok_R,
    tok_S,                tok_T,                tok_U,                tok_V,
    tok_W,                tok_X,                tok_Y,                tok_Z,
    tok_bracketleft,      tok_backslash,        tok_bracketright,     tok_asciicircum,
/*64*/  tok_underscore,   tok_quoteleft,        tok_a,                tok_b,
    tok_c,                tok_d,                tok_e,                tok_f,
    tok_g,                tok_h,                tok_i,                tok_j,
    tok_k,                tok_l,                tok_m,                tok_n,
/*80*/  tok_o,            tok_p,                tok_q,                tok_r,
    tok_s,                tok_t,                tok_u,                tok_v,
    tok_w,                tok_x,                tok_y,                tok_z,
    tok_braceleft,        tok_bar,              tok_braceright,       tok_asciitilde,
/*96*/  tok_exclamdown,   tok_cent,             tok_sterlin,          tok_fraction,
    tok_yen,              tok_florin,           tok_section,          tok_currency,
    tok_quotesingle,      tok_quotedblleft,     tok_guillemotleft,    tok_guilsinglleft,
    tok_guilsinglright,   tok_fi,               tok_fl,               tok_endash,
/*112*/ tok_dagger,       tok_daggerdbl,        tok_periodcentered,   tok_paragraph,
    tok_bullet,           tok_quotesinglbase,   tok_quotedblbase,     tok_quotedblright,
    tok_guillemotright,   tok_ellipsis,         tok_perthousand,      tok_questiondown,
    tok_grave,            tok_acute,            tok_circumflex,       tok_tilde,
/*128*/ tok_macron,       tok_breve,            tok_dotaccent,        tok_dieresis,
    tok_ring,             tok_cedilla,          tok_hungarumlaut,     tok_ogonek,
    tok_caron,            tok_emdash,           tok_AE,               tok_ordfeminine,
    tok_Lslash,           tok_Oslash,           tok_OE,               tok_ordmasculine,
/*144*/ tok_ae,           tok_dotlessi,         tok_lslash,           tok_oslash,
    tok_oe,               tok_germandbls,       tok_onesuperior,      tok_logicalnot,
    tok_mu,               tok_trademark,        tok_Eth,              tok_onehalf,
    tok_plusminus,        tok_Thorn,            tok_onequarter,       tok_divide,
/*160*/ tok_brokenbar,    tok_degree,           tok_thorn,            tok_threequarters,
    tok_twosuperior,      tok_registered,       tok_minus,            tok_eth,
    tok_multiply,         tok_threesuperior,    tok_copyright,        tok_Aacute,
    tok_Acircumflex,      tok_Adieresis,        tok_Agrave,           tok_Aring,
/*176*/ tok_Atilde,       tok_Ccedilla,         tok_Eacute,           tok_Ecircumflex,
    tok_Edieresis,        tok_Egrave,           tok_Iacute,           tok_Icircumflex,
    tok_Idieresis,        tok_Igrave,           tok_Ntilde,           tok_Oacute,
    tok_Ocircumflex,      tok_Odieresis,        tok_Ograve,           tok_Otilde,
/*192*/ tok_Scaron,       tok_Uacute,           tok_Ucircumflex,      tok_Udieresis,
    tok_Ugrave,           tok_Yacute,           tok_Ydieresis,        tok_Zcaron,
    tok_aacute,           tok_acircumflex,      tok_adieresis,        tok_agrave,
    tok_aring,            tok_atilde,           tok_ccedilla,         tok_eacute,
/*208*/ tok_ecircumflex,  tok_edieresis,        tok_egrave,           tok_iacute,
    tok_icircumflex,      tok_idieresis,        tok_igrave,           tok_ntilde,
    tok_oacute,           tok_ocircumflex,      tok_odieresis,        tok_ograve,
    tok_otilde,           tok_scaron,           tok_uacute,           tok_ucircumflex,
/*224*/ tok_udieresis,    tok_ugrave,           tok_yacute,           tok_ydieresis,
    tok_zcaron,           tok_exclamsmall,      tok_Hungarumlautsmall,tok_dollaroldstyle,
    tok_dollarsuperior,   tok_ampersandsmall,   tok_Acutesmall,       tok_parenleftsuperior,
    tok_parenrightsuperior,tok_twodotenleader,  tok_onedotenleader,   tok_zerooldstyle,
/*240*/ tok_oneoldstyle,  tok_twooldstyle,      tok_threeoldstyle,    tok_fouroldstyle,
    tok_fiveoldstyle,     tok_sixoldstyle,      tok_sevenoldstyle,    tok_eightoldstyle,
    tok_nineoldstile,     tok_commasuperior,    tok_threequartersemdash,tok_periodsuperior,
    tok_questionsmall,    tok_asuperior,        tok_bsuperior,        tok_centsuperior,
/*256*/ tok_dsuperior,    tok_esuperior,        tok_isuperior,        tok_lsuperior,
    tok_msuperior,        tok_nsuperior,        tok_osuperior,        tok_rsuperior,
    tok_ssuperior,        tok_tsuperior,        tok_ff,               tok_ffi,
    tok_ffl,              tok_parenleftinferior,tok_parenrightinferior,tok_Circumflexsmall,
/*272*/ tok_hyphensuperior,tok_Gravesmall,      tok_Asmall,           tok_Bsmall,
    tok_Csmall,           tok_Dsmall,           tok_Esmall,           tok_Fsmall,
    tok_Gsmall,           tok_Hsmall,           tok_Ismall,           tok_Jsmall,
    tok_Ksmall,           tok_Lsmall,           tok_Msmall,           tok_Nsmall,
/*288*/ tok_Osmall,       tok_Psmall,           tok_Qsmall,           tok_Rsmall,
    tok_Ssmall,           tok_Tsmall,           tok_Usmall,           tok_Vsmall,
    tok_Wsmall,           tok_Xsmall,           tok_Ysmall,           tok_Zsmall,
    tok_colonmonetary,    tok_onefitted,        tok_rupia,            tok_Tildesmall,
/*304*/ tok_exclamdownsmall,tok_centoldstyle,   tok_Lslashsmall,      tok_Scaronsmall,
    tok_Zcaronsmall,      tok_Dieresissmall,    tok_Brevesmall,       tok_Caronsmall,
    tok_Dotaccentsmall,   tok_Macronsmall,      tok_figuredash,       tok_hypheninferior,
    tok_Ogoneksmall,      tok_Ringsmall,        tok_Cedillasmall,     tok_questiondownsmall,
/*320*/ tok_oneeight,     tok_threeeights,      tok_fiveeights,       tok_seveneights,
    tok_onethird,         tok_twothirds,        tok_zerosuperior,     tok_foursuperior,
    tok_fivesuperior,     tok_sixsuperior,      tok_sevensuperior,    tok_eightsuperior,
    tok_ninesuperior,     tok_zeroinferior,     tok_oneinferior,      tok_twoinferior,
/*336*/ tok_threeinferior,tok_fourinferior,     tok_fiveinferior,     tok_sixinferior,
    tok_seveninferior,    tok_eightinferior,    tok_nineinferior,     tok_centinferior,
    tok_dollarinferior,   tok_periodinferior,   tok_commainferior,    tok_Agravesmall,
    tok_Aacutesmall,      tok_Acircumflexsmall, tok_Atildesmall,      tok_Adieresissmall,
/*352*/ tok_Aringsmall,   tok_AEsmall,          tok_Ccedillasmall,    tok_Egravesmall,
    tok_Eacutesmall,      tok_Ecircumflexsmall, tok_Edieresissmall,   tok_Igravesmall,
    tok_Iacutesmall,      tok_Icircumflexsmall, tok_Idieresissmall,   tok_Ethsmall,
    tok_Ntildesmall,      tok_Ogravesmall,      tok_Oacutesmall,      tok_Ocircumflexsmall,
/*368*/ tok_Otildesmall,  tok_Odieressissmall,  tok_OEsmall,          tok_Oslashsmall,
    tok_Ugravesmall,      tok_Uacutesmall,      tok_Ucircumflexsmall, tok_Udieresissmall,
    tok_Yacutesmall,      tok_Thornsmall,       tok_Ydieresissmall,   tok_001_000,
    tok_001_001,          tok_001_002,          tok_001_003,          tok_Black,
/*384*/ tok_Bold,         tok_Book,             tok_Light,            tok_Medium,
    tok_Regular,          tok_Roman,            tok_Semibold
};

// TOP DICT keywords (also covers PRIV DICT keywords)
static const char* pDictOps[] = {
    "sVersion",         "sNotice",              "sFullName",        "sFamilyName",
    "sWeight",          "aFontBBox",            "dBlueValues",      "dOtherBlues",
    "dFamilyBlues",     "dFamilyOtherBlues",    "nStdHW",           "nStdVW",
    "xESC",             "nUniqueID",            "aXUID",            "nCharset",
    "nEncoding",        "nCharStrings",         "PPrivate",         "nSubrs",
    "nDefaultWidthX",   "nNominalWidthX",       nullptr,               nullptr,
    nullptr,               nullptr,                   nullptr,               nullptr,
    "shortint",         "longint",              "BCD",              nullptr
};

// TOP DICT escapes (also covers PRIV DICT escapes)
static const char* pDictEscs[] = {
    "sCopyright",           "bIsFixedPitch",    "nItalicAngle",     "nUnderlinePosition",
    "nUnderlineThickness",  "nPaintType",       "tCharstringType",  "aFontMatrix",
    "nStrokeWidth",         "nBlueScale",       "nBlueShift",       "nBlueFuzz",
    "dStemSnapH",           "dStemSnapV",       "bForceBold",       nullptr,
    nullptr,                   "nLanguageGroup",   "nExpansionFactor", "nInitialRandomSeed",
    "nSyntheticBase",       "sPostScript",      "sBaseFontName",    "dBaseFontBlend",
    nullptr,                   nullptr,               nullptr,               nullptr,
    nullptr,                   nullptr,               "rROS",             "nCIDFontVersion",
    "nCIDFontRevision",     "nCIDFontType",     "nCIDCount",        "nUIDBase",
    "nFDArray",             "nFDSelect",        "sFontName"
};

static const char* pStandardEncoding[] = {
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", "space", "exclam", "quotedbl",
    "numbersign", "dollar", "percent", "ampersand",
    "quoteright", "parenleft", "parenright", "asterisk", "plus",
    "comma", "hyphen", "period", "slash", "zero", "one", "two",
    "three", "four", "five", "six", "seven", "eight", "nine",
    "colon", "semicolon", "less", "equal", "greater",
    "question", "at", "A", "B", "C", "D", "E", "F", "G", "H",
    "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
    "U", "V", "W", "X", "Y", "Z", "bracketleft", "backslash",
    "bracketright", "asciicircum", "underscore", "quoteleft",
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l",
    "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
    "y", "z", "braceleft", "bar", "braceright", "asciitilde",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", "exclamdown",
    "cent", "sterling", "fraction", "yen", "florin", "section",
    "currency", "quotesingle", "quotedblleft", "guillemotleft",
    "guilsinglleft", "guilsinglright", "fi", "fl", ".notdef",
    "endash", "dagger", "daggerdbl", "periodcentered",
    ".notdef", "paragraph", "bullet", "quotesinglbase",
    "quotedblbase", "quotedblright", "guillemotright",
    "ellipsis", "perthousand", ".notdef", "questiondown",
    ".notdef", "grave", "acute", "circumflex", "tilde",
    "macron", "breve", "dotaccent", "dieresis", ".notdef",
    "ring", "cedilla", ".notdef", "hungarumlaut", "ogonek",
    "caron", "emdash", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", ".notdef",
    ".notdef", ".notdef", ".notdef", "AE", ".notdef",
    "ordfeminine", ".notdef", ".notdef", ".notdef", ".notdef",
    "Lslash", "Oslash", "OE", "ordmasculine", ".notdef",
    ".notdef", ".notdef", ".notdef", ".notdef", "ae", ".notdef",
    ".notdef", ".notdef", "dotlessi", ".notdef", ".notdef",
    "lslash", "oslash", "oe", "germandbls", ".notdef",
    ".notdef", ".notdef", ".notdef"
};

namespace {

namespace TYPE1OP
{
    enum OPS
    {
        HSTEM=1,        VSTEM=3,        VMOVETO=4,      RLINETO=5,
        HLINETO=6,      VLINETO=7,      RCURVETO=8,     CLOSEPATH=9,
        CALLSUBR=10,    RETURN=11,      T1ESC=12,       HSBW=13,
        ENDCHAR=14,     RMOVETO=21,     HMOVETO=22,     VHCURVETO=30,
        HVCURVETO=31
    };

    enum ESCS
    {
        DOTSECTION=0,   VSTEM3=1,           HSTEM3=2,   SEAC=6,
        SBW=7,          ABS=9,              ADD=10,     SUB=11,
        DIV=12,         CALLOTHERSUBR=16,   POP=17,     SETCURRENTPOINT=33
    };
}

namespace TYPE2OP
{
    enum OPS
    {
        HSTEM=1,        VSTEM=3,        VMOVETO=4,      RLINETO=5,
        HLINETO=6,      VLINETO=7,      RCURVETO=8,     CALLSUBR=10,
        RETURN=11,      T2ESC=12,       ENDCHAR=14,     HSTEMHM=18,
        HINTMASK=19,    CNTRMASK=20,    RMOVETO=21,     HMOVETO=22,
        VSTEMHM=23,     RCURVELINE=24,  RLINECURVE=25,  VVCURVETO=26,
        HHCURVETO=27,   SHORTINT=28,    CALLGSUBR=29,   VHCURVETO=30,
        HVCURVETO=31
    };

    enum ESCS
    {
        AND=3,      OR=4,       NOT=5,      ABS=9,
        ADD=10,     SUB=11,     DIV=12,     NEG=14,
        EQ=15,      DROP=18,    PUT=20,     GET=21,
        IFELSE=22,  RANDOM=23,  MUL=24,     SQRT=26,
        DUP=27,     EXCH=28,    INDEX=29,   ROLL=30,
        HFLEX=34,   FLEX=35,    HFLEX1=36,  FLEX1=37
    };
}

struct CffGlobal
{
    explicit CffGlobal();

    int     mnNameIdxBase;
    int     mnStringIdxBase;
    bool    mbCIDFont;
    int     mnCharStrBase;
    int     mnCharStrCount;
    int     mnCharsetBase;
    int     mnGlobalSubrBase;
    int     mnGlobalSubrCount;
    int     mnGlobalSubrBias;
    int     mnFDSelectBase;
    int     mnFontDictBase;
    int     mnFDAryCount;

    std::vector<ValType>   maFontBBox;
    std::vector<ValType>   maFontMatrix;

    int     mnFontNameSID;
    int     mnFullNameSID;
};

struct CffLocal
{
    explicit CffLocal();

    int     mnPrivDictBase;
    int     mnPrivDictSize;
    int     mnLocalSubrOffs;
    int     mnLocalSubrBase;
    int     mnLocalSubrBias;

    ValType maNominalWidth;
    ValType maDefaultWidth;

    // ATM hinting related values
    ValType     maStemStdHW;
    ValType     maStemStdVW;
    std::vector<ValType>   maStemSnapH;
    std::vector<ValType>   maStemSnapV;
    std::vector<ValType>   maBlueValues;
    std::vector<ValType>   maOtherBlues;
    std::vector<ValType>   maFamilyBlues;
    std::vector<ValType>   maFamilyOtherBlues;
    RealType    mfBlueScale;
    RealType    mfBlueShift;
    RealType    mfBlueFuzz;
    RealType    mfExpFactor;
    int         mnLangGroup;
    bool        mbForceBold;
};

const int MAX_T1OPS_SIZE = 81920; // TODO: use dynamic value

struct CharString
{
    int nLen;
    U8 aOps[MAX_T1OPS_SIZE];
    int nCffGlyphId;
};


class CffSubsetterContext
:   private CffGlobal
{
public:
    static const int NMAXSTACK = 48;    // see CFF.appendixB
    static const int NMAXHINTS = 2*96;  // see CFF.appendixB
    static const int NMAXTRANS = 32;    // see CFF.appendixB

    explicit CffSubsetterContext( const U8* pBasePtr, int nBaseLen);

    bool    initialCffRead();
    void    emitAsType1( class Type1Emitter&,
                const sal_GlyphId* pGlyphIds, const U8* pEncoding,
                int nGlyphCount, FontSubsetInfo& );

private:
    void    convertCharStrings(const sal_GlyphId* pGlyphIds, int nGlyphCount,
                std::vector<CharString>& rCharStrings);
    int     convert2Type1Ops( CffLocal*, const U8* pType2Ops, int nType2Len, U8* pType1Ops);
    void    convertOneTypeOp();
    void    convertOneTypeEsc();
    void    callType2Subr( bool bGlobal, int nSubrNumber);
    sal_Int32 getReadOfs() const { return static_cast<sal_Int32>(mpReadPtr - mpBasePtr);}

    const U8* mpBasePtr;
    const U8* mpBaseEnd;

    const U8* mpReadPtr;
    const U8* mpReadEnd;

    U8*     mpWritePtr;
    bool    mbNeedClose;
    bool    mbIgnoreHints;
    sal_Int32 mnCntrMask;

    int     seekIndexData( int nIndexBase, int nDataIndex);
    void    seekIndexEnd( int nIndexBase);

    CffLocal    maCffLocal[256];
    CffLocal*   mpCffLocal;

    void        readDictOp();
    RealType    readRealVal();
    OString     getString( int nStringID);
    int         getFDSelect( int nGlyphIndex) const;
    int         getGlyphSID( int nGlyphIndex) const;
    OString     getGlyphName( int nGlyphIndex);
    bool        getBaseAccent(ValType aBase, ValType aAccent, int* nBase, int* nAccent);

    void    read2push();
    void    writeType1Val( ValType);
    void    writeTypeOp( int nTypeOp);
    void    writeTypeEsc( int nTypeOp);
    void    writeCurveTo( int nStackPos, int nIX1, int nIY1, int nIX2, int nIY2, int nIX3, int nIY3);
    void    pop2MultiWrite( int nArgsPerTypo, int nTypeOp, int nTypeXor=0);
    void    popAll2Write( int nTypeOp);

public: // TODO: is public really needed?
    // accessing the value stack
    // TODO: add more checks
    void    push( ValType nVal) { mnValStack[ mnStackIdx++] = nVal;}
    ValType popVal() { return ((mnStackIdx>0) ? mnValStack[ --mnStackIdx] : 0);}
    ValType getVal( int nIndex) const { return mnValStack[ nIndex];}
    int     popInt();
    int     size() const { return mnStackIdx;}
    void    clear() { mnStackIdx = 0;}

    // accessing the charstring hints
    void    addHints( bool bVerticalHints);

    // accessing other charstring specifics
    void    updateWidth( bool bUseFirstVal);

private:
    // typeop execution context
    int mnStackIdx;
    ValType mnValStack[ NMAXSTACK+4];
    ValType mnTransVals[ NMAXTRANS];

    int mnHintSize;
    int mnHorzHintSize;
    ValType mnHintStack[ NMAXHINTS];

    ValType maCharWidth;

    bool    mbDoSeac;
    std::vector<sal_GlyphId> maExtraGlyphIds;
};

}

CffSubsetterContext::CffSubsetterContext( const U8* pBasePtr, int nBaseLen)
    : mpBasePtr( pBasePtr)
    , mpBaseEnd( pBasePtr+nBaseLen)
    , mpReadPtr(nullptr)
    , mpReadEnd(nullptr)
    , mpWritePtr(nullptr)
    , mbNeedClose(false)
    , mbIgnoreHints(false)
    , mnCntrMask(0)
    , mnStackIdx(0)
    , mnValStack{}
    , mnTransVals{}
    , mnHintSize(0)
    , mnHorzHintSize(0)
    , mnHintStack{}
    , maCharWidth(-1)
    , mbDoSeac(true)
{
//  setCharStringType( 1);
    // TODO: new CffLocal[ mnFDAryCount];
    mpCffLocal = &maCffLocal[0];
}

inline int CffSubsetterContext::popInt()
{
    const ValType aVal = popVal();
    const int nInt = static_cast<int>(aVal);
    assert( nInt == aVal);
    return nInt;
}

inline void CffSubsetterContext::updateWidth( bool bUseFirstVal)
{
    // the first value is not a hint but the charwidth
    if( maCharWidth>0 )
        return;

    if( bUseFirstVal) {
        maCharWidth = mpCffLocal->maNominalWidth + mnValStack[0];
        // remove bottom stack entry
        --mnStackIdx;
        for( int i = 0; i < mnStackIdx; ++i)
            mnValStack[ i] = mnValStack[ i+1];
    } else {
        maCharWidth = mpCffLocal->maDefaultWidth;
    }
}

void CffSubsetterContext::addHints( bool bVerticalHints)
{
    // the first charstring value may a charwidth instead of a charwidth
    updateWidth( (mnStackIdx & 1) != 0);
    // return early (e.g. no implicit hints for hintmask)
    if( !mnStackIdx)
        return;

    // copy the remaining values to the hint arrays
    // assert( (mnStackIdx & 1) == 0); // depends on called subrs
    if( mnStackIdx & 1) --mnStackIdx;//#######
    // TODO: if( !bSubr) assert( mnStackIdx >= 2);

    assert( (mnHintSize + mnStackIdx) <= 2*NMAXHINTS);

    ValType nHintOfs = 0;
    for( int i = 0; i < mnStackIdx; ++i) {
        nHintOfs += mnValStack[ i ];
        mnHintStack[ mnHintSize++] = nHintOfs;
    }

    if( !bVerticalHints)
        mnHorzHintSize = mnHintSize;

    // clear all values from the stack
    mnStackIdx = 0;
}

void CffSubsetterContext::readDictOp()
{
    const U8 c = *mpReadPtr;
    if( c <= 21 ) {
        int nOpId = *(mpReadPtr++);
        const char* pCmdName = nullptr;
        if( nOpId != 12)
            pCmdName = pDictOps[nOpId];
        else {
            const U8 nExtId = *(mpReadPtr++);
            if (nExtId < 39)
               pCmdName = pDictEscs[nExtId];
            nOpId = 900 + nExtId;
        }

        if (!pCmdName)  // skip reserved operators
            return;

        //TODO: if( nStackIdx > 0)
        int nInt = 0;
        switch( *pCmdName) {
        default: SAL_WARN("vcl.fonts", "unsupported DictOp.type='" << *pCmdName << "'."); break;
        case 'b':   // bool
            nInt = popInt();
            switch( nOpId) {
            case 915: mpCffLocal->mbForceBold = nInt; break;    // "ForceBold"
            default: break; // TODO: handle more boolean dictops?
            }
            break;
        case 'n': { // dict-op number
            ValType nVal = popVal();
            nInt = static_cast<int>(nVal);
            switch( nOpId) {
            case  10: mpCffLocal->maStemStdHW = nVal; break;    // "StdHW"
            case  11: mpCffLocal->maStemStdVW = nVal; break;    // "StdVW"
            case  15: mnCharsetBase = nInt; break;              // "charset"
            case  16: break;                                    // "nEncoding"
            case  17: mnCharStrBase = nInt; break;              // "nCharStrings"
            case  19: mpCffLocal->mnLocalSubrOffs = nInt; break;// "nSubrs"
            case  20: mpCffLocal->maDefaultWidth = nVal; break; // "defaultWidthX"
            case  21: mpCffLocal->maNominalWidth = nVal; break; // "nominalWidthX"
            case 909: mpCffLocal->mfBlueScale = nVal; break;    // "BlueScale"
            case 910: mpCffLocal->mfBlueShift = nVal; break;    // "BlueShift"
            case 911: mpCffLocal->mfBlueFuzz = nVal; break;     // "BlueFuzz"
            case 912: mpCffLocal->mfExpFactor = nVal; break;    // "ExpansionFactor"
            case 917: mpCffLocal->mnLangGroup = nInt; break;    // "LanguageGroup"
            case 936: mnFontDictBase = nInt; break;             // "nFDArray"
            case 937: mnFDSelectBase = nInt; break;             // "nFDSelect"
            default: break; // TODO: handle more numeric dictops?
            }
            } break;
        case 'a': { // array
            switch( nOpId) {
            case   5: maFontBBox.clear(); break;     // "FontBBox"
            case 907: maFontMatrix.clear(); break; // "FontMatrix"
            default: break; // TODO: reset other arrays?
            }
            for( int i = 0; i < size(); ++i ) {
                ValType nVal = getVal(i);
                switch( nOpId) {
                case   5: maFontBBox.push_back( nVal); break;     // "FontBBox"
                case 907: maFontMatrix.push_back( nVal); break; // "FontMatrix"
                default: break; // TODO: handle more array dictops?
                }
            }
            clear();
            } break;
        case 'd': { // delta array
            ValType nVal = 0;
            for( int i = 0; i < size(); ++i ) {
                nVal += getVal(i);
                switch( nOpId) {
                case   6: mpCffLocal->maBlueValues.push_back( nVal); break;     // "BlueValues"
                case   7: mpCffLocal->maOtherBlues.push_back( nVal); break;     // "OtherBlues"
                case   8: mpCffLocal->maFamilyBlues.push_back( nVal); break;    // "FamilyBlues"
                case   9: mpCffLocal->maFamilyOtherBlues.push_back( nVal); break;// "FamilyOtherBlues"
                case 912: mpCffLocal->maStemSnapH.push_back( nVal); break;      // "StemSnapH"
                case 913: mpCffLocal->maStemSnapV.push_back( nVal); break;      // "StemSnapV"
                default: break; // TODO: handle more delta-array dictops?
                }
            }
            clear();
            } break;
        case 's':   // stringid (SID)
            nInt = popInt();
            switch( nOpId ) {
            case   2: mnFullNameSID = nInt; break;      // "FullName"
            case   3: break;    // "FamilyName"
            case 938: mnFontNameSID = nInt; break;      // "FontName"
            default: break; // TODO: handle more string dictops?
            }
            break;
        case 'P':   // private dict
            mpCffLocal->mnPrivDictBase = popInt();
            mpCffLocal->mnPrivDictSize = popInt();
            break;
        case 'r': { // ROS operands
            popInt(); // TODO: use sid1
            popInt(); // TODO: use sid2
            popVal();
            mbCIDFont = true;
            } break;
        case 't':   // CharstringType
            popInt();
            break;
        }
    } else if( (c >= 32) || (c == 28) ) {
//      --mpReadPtr;
        read2push();
    } else if( c == 29 ) {      // longint
        ++mpReadPtr;            // skip 29
        sal_Int32 nS32 = mpReadPtr[0] << 24;
        nS32 += mpReadPtr[1] << 16;
        nS32 += mpReadPtr[2] << 8;
        nS32 += mpReadPtr[3] << 0;
        mpReadPtr += 4;
        ValType nVal = static_cast<ValType>(nS32);
        push( nVal );
    } else if( c == 30) {       // real number
        ++mpReadPtr; // skip 30
        const RealType fReal = readRealVal();
        // push value onto stack
        ValType nVal = fReal;
        push( nVal);
    }
}

void CffSubsetterContext::read2push()
{
    ValType aVal = 0;

    const U8*& p = mpReadPtr;
    const U8 c = *p;
    if( c == 28 ) {
        sal_Int16 nS16 = (p[1] << 8) + p[2];
        aVal = nS16;
        p += 3;
    } else if( c <= 246 ) {     // -107..+107
        aVal = static_cast<ValType>(p[0] - 139);
        p += 1;
    } else if( c <= 250 ) {     // +108..+1131
        aVal = static_cast<ValType>(((p[0] << 8) + p[1]) - 63124);
        p += 2;
    } else if( c <= 254 ) {     // -108..-1131
        aVal = static_cast<ValType>(64148 - ((p[0] << 8) + p[1]));
        p += 2;
    } else /*if( c == 255)*/ {  // Fixed16.16
        int nS32 = (p[1] << 24) + (p[2] << 16) + (p[3] << 8) + p[4];
        if( (sizeof(nS32) != 2) && (nS32 & (1U<<31)))
            nS32 |= (~0U) << 31;    // assuming 2s complement
        aVal = static_cast<ValType>(nS32 * (1.0 / 0x10000));
        p += 5;
    }

    push( aVal);
}

void CffSubsetterContext::writeType1Val( ValType aVal)
{
    U8* pOut = mpWritePtr;

    // tdf#126242
    // Type2 has 16.16 fixed numbers, but Type1 does not. To represent values
    // with fractions we multiply it by a factor then use “div” operator to
    // divide it back and keep the fractions.
    // Code Adapted from:
    // https://github.com/fontforge/fontforge/blob/f152f12e567ea5bd737a2907c318ae26cfaabd08/fontforge/splinesave.c#L378
    int nDiv = 0;
    aVal = rint(aVal * 1024) / 1024;
    if (aVal != floor(aVal))
    {
        if (aVal == rint(aVal * 64) / 64)
            nDiv = 64;
        else
            nDiv = 1024;
        aVal *= nDiv;
    }

    int nInt = static_cast<int>(rint(aVal));
    if (nDiv && floor(nInt) / nDiv == floor(nInt / nDiv))
    {
        nInt = rint(nInt / nDiv);
        nDiv = 0;
    }

    if( (nInt >= -107) && (nInt <= +107)) {
        *(pOut++) = static_cast<U8>(nInt + 139);    // -107..+107
    } else if( (nInt >= -1131) && (nInt <= +1131)) {
        if( nInt >= 0)
            nInt += 63124;                          // +108..+1131
        else
            nInt = 64148 - nInt;                    // -108..-1131
        *(pOut++) = static_cast<U8>(nInt >> 8);
        *(pOut++) = static_cast<U8>(nInt);
    } else {
        // numtype==255 means int32 for Type1, but 16.16 for Type2 charstrings!!!
        *(pOut++) = 255;
        *(pOut++) = static_cast<U8>(nInt >> 24);
        *(pOut++) = static_cast<U8>(nInt >> 16);
        *(pOut++) = static_cast<U8>(nInt >> 8);
        *(pOut++) = static_cast<U8>(nInt);
    }

    mpWritePtr = pOut;

    if (nDiv)
    {
        writeType1Val(nDiv);
        writeTypeEsc(TYPE1OP::DIV);
    }
}

inline void CffSubsetterContext::writeTypeOp( int nTypeOp)
{
    *(mpWritePtr++) = static_cast<U8>(nTypeOp);
}

inline void CffSubsetterContext::writeTypeEsc( int nTypeEsc)
{
    *(mpWritePtr++) = TYPE1OP::T1ESC;
    *(mpWritePtr++) = static_cast<U8>(nTypeEsc);
}

void CffSubsetterContext::pop2MultiWrite( int nArgsPerTypo, int nTypeOp, int nTypeXor)
{
    for( int i = 0; i < mnStackIdx;) {
        for( int j = 0; j < nArgsPerTypo; ++j) {
            const ValType aVal = mnValStack[i+j];
            writeType1Val( aVal);
        }
        i += nArgsPerTypo;
        writeTypeOp( nTypeOp);
        nTypeOp ^= nTypeXor;    // for toggling vlineto/hlineto
    }
    clear();
}

void CffSubsetterContext::popAll2Write( int nTypeOp)
{
    // pop in reverse order, then write
    for( int i = 0; i < mnStackIdx; ++i) {
        const ValType aVal = mnValStack[i];
        writeType1Val( aVal);
    }
    clear();
    writeTypeOp( nTypeOp);
}

void CffSubsetterContext::writeCurveTo( int nStackPos,
    int nIX1, int nIY1, int nIX2, int nIY2, int nIX3, int nIY3)
{
    // get the values from the stack
    const ValType nDX1 = nIX1 ? mnValStack[ nStackPos+nIX1 ] : 0;
    const ValType nDY1 = nIY1 ? mnValStack[ nStackPos+nIY1 ] : 0;
    const ValType nDX2 = nIX2 ? mnValStack[ nStackPos+nIX2 ] : 0;
    const ValType nDY2 = nIY2 ? mnValStack[ nStackPos+nIY2 ] : 0;
    const ValType nDX3 = nIX3 ? mnValStack[ nStackPos+nIX3 ] : 0;
    const ValType nDY3 = nIY3 ? mnValStack[ nStackPos+nIY3 ] : 0;

    // emit the curveto operator and operands
    // TODO: determine the most efficient curveto operator
    // TODO: depending on type1op or type2op target
    writeType1Val( nDX1 );
    writeType1Val( nDY1 );
    writeType1Val( nDX2 );
    writeType1Val( nDY2 );
    writeType1Val( nDX3 );
    writeType1Val( nDY3 );
    writeTypeOp( TYPE1OP::RCURVETO );
}

void CffSubsetterContext::convertOneTypeOp()
{
    const int nType2Op = *(mpReadPtr++);

    int i, nInt; // prevent WAE for declarations inside switch cases
    // convert each T2op
    switch( nType2Op) {
    case TYPE2OP::T2ESC:
        convertOneTypeEsc();
        break;
    case TYPE2OP::HSTEM:
    case TYPE2OP::VSTEM:
        addHints( nType2Op == TYPE2OP::VSTEM );
        for( i = 0; i < mnHintSize; i+=2 ) {
            writeType1Val( mnHintStack[i]);
            writeType1Val( mnHintStack[i+1] - mnHintStack[i]);
            writeTypeOp( nType2Op );
        }
        break;
    case TYPE2OP::HSTEMHM:
    case TYPE2OP::VSTEMHM:
        addHints( nType2Op == TYPE2OP::VSTEMHM);
        break;
    case TYPE2OP::CNTRMASK:
        // TODO: replace cntrmask with vstem3/hstem3
        addHints( true);
        {
        U8 nMaskBit = 0;
        U8 nMaskByte = 0;
        for( i = 0; i < mnHintSize; i+=2, nMaskBit>>=1) {
            if( !nMaskBit) {
                nMaskByte = *(mpReadPtr++);
                nMaskBit = 0x80;
            }
            if( !(nMaskByte & nMaskBit))
                continue;
            if( i >= 8*int(sizeof(mnCntrMask)))
                mbIgnoreHints = true;
            if( mbIgnoreHints)
                continue;
            mnCntrMask |= (1U << i);
        }
        }
        break;
    case TYPE2OP::HINTMASK:
        addHints( true);
        {
        sal_Int32 nHintMask = 0;
        int nCntrBits[2] = {0,0};
        U8 nMaskBit = 0;
        U8 nMaskByte = 0;
        int const MASK_BITS = 8*sizeof(nHintMask);
        for( i = 0; i < mnHintSize; i+=2, nMaskBit>>=1) {
            if( !nMaskBit) {
                nMaskByte = *(mpReadPtr++);
                nMaskBit = 0x80;
            }
            if( !(nMaskByte & nMaskBit))
                continue;
            if( i >= MASK_BITS)
                mbIgnoreHints = true;
            if( mbIgnoreHints)
                continue;
            nHintMask |= (1U << i);
            nCntrBits[ i < mnHorzHintSize] += (mnCntrMask >> i) & 1;
        }

        mbIgnoreHints |= (nCntrBits[0] && (nCntrBits[0] != 3));
        mbIgnoreHints |= (nCntrBits[1] && (nCntrBits[1] != 3));
        if( mbIgnoreHints)
            break;

        for( i = 0; i < mnHintSize; i+=2) {
            if(i >= MASK_BITS || !(nHintMask & (1U << i)))
                continue;
            writeType1Val( mnHintStack[i]);
            writeType1Val( mnHintStack[i+1] - mnHintStack[i]);
            const bool bHorz = (i < mnHorzHintSize);
            if( !nCntrBits[ bHorz])
                writeTypeOp( bHorz ? TYPE1OP::HSTEM : TYPE1OP::VSTEM);
            else if( !--nCntrBits[ bHorz])
                writeTypeEsc( bHorz ? TYPE1OP::HSTEM3 : TYPE1OP::VSTEM3);
        }
        }
        break;
    case TYPE2OP::CALLSUBR:
    case TYPE2OP::CALLGSUBR:
        {
        nInt = popInt();
        const bool bGlobal = (nType2Op == TYPE2OP::CALLGSUBR);
        callType2Subr( bGlobal, nInt);
        }
        break;
    case TYPE2OP::RETURN:
        // TODO: check that we are in a subroutine
        return;
    case TYPE2OP::VMOVETO:
    case TYPE2OP::HMOVETO:
        if( mbNeedClose)
            writeTypeOp( TYPE1OP::CLOSEPATH);
        else
            updateWidth( size() > 1);
        mbNeedClose = true;
        pop2MultiWrite( 1, nType2Op);
        break;
    case TYPE2OP::VLINETO:
    case TYPE2OP::HLINETO:
        pop2MultiWrite( 1, nType2Op,
            TYPE1OP::VLINETO ^ TYPE1OP::HLINETO);
        break;
    case TYPE2OP::RMOVETO:
        // TODO: convert rmoveto to vlineto/hlineto if possible
        if( mbNeedClose)
            writeTypeOp( TYPE1OP::CLOSEPATH);
        else
            updateWidth( size() > 2);
        mbNeedClose = true;
        pop2MultiWrite( 2, nType2Op);
        break;
    case TYPE2OP::RLINETO:
        // TODO: convert rlineto to vlineto/hlineto if possible
        pop2MultiWrite( 2, nType2Op);
        break;
    case TYPE2OP::RCURVETO:
        // TODO: convert rcurveto to vh/hv/hh/vv-curveto if possible
        pop2MultiWrite( 6, nType2Op);
        break;
    case TYPE2OP::RCURVELINE:
        i = 0;
        while( (i += 6) <= mnStackIdx)
            writeCurveTo( i, -6, -5, -4, -3, -2, -1 );
        i -= 6;
        while( (i += 2) <= mnStackIdx) {
            writeType1Val( mnValStack[i-2]);
            writeType1Val( mnValStack[i-1]);
            writeTypeOp( TYPE2OP::RLINETO);
        }
        clear();
        break;
    case TYPE2OP::RLINECURVE:
        i = 0;
        while( (i += 2) <= mnStackIdx-6) {
            writeType1Val( mnValStack[i-2]);
            writeType1Val( mnValStack[i-1]);
            writeTypeOp( TYPE2OP::RLINETO);
        }
        i -= 2;
        while( (i += 6) <= mnStackIdx)
            writeCurveTo( i, -6, -5, -4, -3, -2, -1 );
        clear();
        break;
    case TYPE2OP::VHCURVETO:
    case TYPE2OP::HVCURVETO:
        {
        bool bVert = (nType2Op == TYPE2OP::VHCURVETO);
        i = 0;
        nInt = 0;
        if( mnStackIdx & 1 )
            nInt = static_cast<int>(mnValStack[ --mnStackIdx ]);
        while( (i += 4) <= mnStackIdx) {
            // TODO: use writeCurveTo()
            if( bVert ) writeType1Val( 0 );
            writeType1Val( mnValStack[i-4] );
            if( !bVert ) writeType1Val( 0);
            writeType1Val( mnValStack[i-3] );
            writeType1Val( mnValStack[i-2] );
            if( !bVert ) writeType1Val( static_cast<ValType>((i==mnStackIdx) ? nInt : 0) );
            writeType1Val( mnValStack[i-1] );
            if( bVert ) writeType1Val( static_cast<ValType>((i==mnStackIdx) ? nInt : 0) );
            bVert = !bVert;
            writeTypeOp( TYPE2OP::RCURVETO);
        }
        }
        clear();
        break;
    case TYPE2OP::HHCURVETO:
        i = (mnStackIdx & 1);
        while( (i += 4) <= mnStackIdx) {
            if( i != 5)
                writeCurveTo( i, -4,  0, -3, -2, -1, 0);
            else
                writeCurveTo( i, -4, -5, -3, -2, -1, 0);
        }
        clear();
        break;
    case TYPE2OP::VVCURVETO:
        i = (mnStackIdx & 1);
        while( (i += 4) <= mnStackIdx) {
            if( i != 5)
                writeCurveTo( i,  0, -4, -3, -2, 0, -1);
            else
                writeCurveTo( i, -5, -4, -3, -2, 0, -1);
        }
        clear();
        break;
    case TYPE2OP::ENDCHAR:
        if (size() >= 4 && mbDoSeac)
        {
            // Deprecated seac-like use of endchar (Adobe Technical Note #5177,
            // Appendix C).
            auto achar = popVal();
            auto bchar = popVal();
            auto ady = popVal();
            auto adx = popVal();
            int nBase = {}, nAccent = {};
            if (getBaseAccent(bchar, achar, &nBase, &nAccent))
            {
                maExtraGlyphIds.push_back(nBase);
                maExtraGlyphIds.push_back(nAccent);
                writeType1Val(0); // TODO accent sb
                writeType1Val(adx);
                writeType1Val(ady);
                writeType1Val(bchar);
                writeType1Val(achar);
                writeTypeEsc(TYPE1OP::SEAC);
            }
        }
        if( mbNeedClose)
            writeTypeOp( TYPE1OP::CLOSEPATH);
        else
            updateWidth( size() >= 1);
        // mbNeedClose = true;
        writeTypeOp( TYPE1OP::ENDCHAR);
        break;
    default:
        if( ((nType2Op >= 32) && (nType2Op <= 255)) || (nType2Op == 28)) {
            --mpReadPtr;
            read2push();
        } else {
            popAll2Write( nType2Op);
            assert(false && "TODO?");
        }
        break;
    }
}

void CffSubsetterContext::convertOneTypeEsc()
{
    const int nType2Esc = *(mpReadPtr++);
    ValType* pTop = &mnValStack[ mnStackIdx-1];
    // convert each T2op
    switch( nType2Esc) {
    case TYPE2OP::AND:
        assert( mnStackIdx >= 2 );
        pTop[0] = static_cast<ValType>(static_cast<int>(pTop[0]) & static_cast<int>(pTop[-1]));
        --mnStackIdx;
        break;
    case TYPE2OP::OR:
        assert( mnStackIdx >= 2 );
        pTop[0] = static_cast<ValType>(static_cast<int>(pTop[0]) | static_cast<int>(pTop[-1]));
        --mnStackIdx;
        break;
    case TYPE2OP::NOT:
        assert( mnStackIdx >= 1 );
        pTop[0] = ValType(pTop[0] == 0);
        break;
    case TYPE2OP::ABS:
        assert( mnStackIdx >= 1 );
        if( pTop[0] >= 0)
            break;
        [[fallthrough]];
    case TYPE2OP::NEG:
        assert( mnStackIdx >= 1 );
        pTop[0] = -pTop[0];
        break;
    case TYPE2OP::ADD:
        assert( mnStackIdx >= 2 );
        pTop[0] += pTop[-1];
        --mnStackIdx;
        break;
    case TYPE2OP::SUB:
        assert( mnStackIdx >= 2 );
        pTop[0] -= pTop[-1];
        --mnStackIdx;
        break;
    case TYPE2OP::MUL:
        assert( mnStackIdx >= 2 );
        if( pTop[-1])
            pTop[0] *= pTop[-1];
        --mnStackIdx;
        break;
    case TYPE2OP::DIV:
        assert( mnStackIdx >= 2 );
        if( pTop[-1])
            pTop[0] /= pTop[-1];
        --mnStackIdx;
        break;
    case TYPE2OP::EQ:
        assert( mnStackIdx >= 2 );
        pTop[0] = ValType(pTop[0] == pTop[-1]);
        --mnStackIdx;
        break;
    case TYPE2OP::DROP:
        assert( mnStackIdx >= 1 );
        --mnStackIdx;
        break;
    case TYPE2OP::PUT: {
        assert( mnStackIdx >= 2 );
        const int nIdx = static_cast<int>(pTop[0]);
        assert( nIdx >= 0 );
        assert( nIdx < NMAXTRANS );
        mnTransVals[ nIdx] = pTop[-1];
        mnStackIdx -= 2;
        break;
        }
    case TYPE2OP::GET: {
        assert( mnStackIdx >= 1 );
        const int nIdx = static_cast<int>(pTop[0]);
        assert( nIdx >= 0 );
        assert( nIdx < NMAXTRANS );
        pTop[0] = mnTransVals[ nIdx ];
        break;
        }
    case TYPE2OP::IFELSE: {
        assert( mnStackIdx >= 4 );
        if( pTop[-1] > pTop[0] )
            pTop[-3] = pTop[-2];
        mnStackIdx -= 3;
        break;
        }
    case TYPE2OP::RANDOM:
        pTop[+1] = 1234; // TODO
        ++mnStackIdx;
        break;
    case TYPE2OP::SQRT:
        // TODO: implement
        break;
    case TYPE2OP::DUP:
        assert( mnStackIdx >= 1 );
        pTop[+1] = pTop[0];
        ++mnStackIdx;
        break;
    case TYPE2OP::EXCH: {
        assert( mnStackIdx >= 2 );
        const ValType nVal = pTop[0];
        pTop[0] = pTop[-1];
        pTop[-1] = nVal;
        break;
        }
    case TYPE2OP::INDEX: {
        assert( mnStackIdx >= 1 );
        const int nVal = static_cast<int>(pTop[0]);
        assert( nVal >= 0 );
        assert( nVal < mnStackIdx-1 );
        pTop[0] = pTop[-1-nVal];
        break;
        }
    case TYPE2OP::ROLL: {
        assert( mnStackIdx >= 1 );
        const int nNum = static_cast<int>(pTop[0]);
        assert( nNum >= 0);
        assert( nNum < mnStackIdx-2 );
        (void)nNum; // TODO: implement
        // TODO: implement: const int nOfs = static_cast<int>(pTop[-1]);
        mnStackIdx -= 2;
        break;
        }
    case TYPE2OP::HFLEX1: {
            assert( mnStackIdx == 9);

            writeCurveTo( mnStackIdx, -9, -8, -7, -6, -5,  0);
            writeCurveTo( mnStackIdx, -4,  0, -3, -2, -1,  0);
        // TODO: emulate hflex1 using othersubr call

            mnStackIdx -= 9;
        }
        break;
    case TYPE2OP::HFLEX: {
            assert( mnStackIdx == 7);
            ValType* pX = &mnValStack[ mnStackIdx];

            pX[+1] = -pX[-5]; // temp: +dy5==-dy2
            writeCurveTo( mnStackIdx, -7,  0, -6, -5, -4,  0);
            writeCurveTo( mnStackIdx, -3,  0, -2, +1, -1,  0);
        // TODO: emulate hflex using othersubr call

            mnStackIdx -= 7;
        }
        break;
    case TYPE2OP::FLEX: {
            assert( mnStackIdx == 13 );
            writeCurveTo( mnStackIdx, -13, -12, -11, -10, -9, -8 );
            writeCurveTo( mnStackIdx,  -7,  -6,  -5,  -4, -3, -2 );
            // ignoring ValType nFlexDepth = mnValStack[ mnStackIdx-1 ];
            mnStackIdx -= 13;
        }
        break;
    case TYPE2OP::FLEX1: {
            assert( mnStackIdx == 11 );
            // write the first part of the flex1-hinted curve
            writeCurveTo( mnStackIdx, -11, -10, -9, -8, -7, -6 );

            // determine if nD6 is horizontal or vertical
            const int i = mnStackIdx;
            ValType nDeltaX = mnValStack[i-11] + mnValStack[i-9] + mnValStack[i-7] + mnValStack[i-5] + mnValStack[i-3];
            if( nDeltaX < 0 ) nDeltaX = -nDeltaX;
            ValType nDeltaY = mnValStack[i-10] + mnValStack[i-8] + mnValStack[i-6] + mnValStack[i-4] + mnValStack[i-2];
            if( nDeltaY < 0 ) nDeltaY = -nDeltaY;
            const bool bVertD6 = (nDeltaY > nDeltaX);

            // write the second part of the flex1-hinted curve
            if( !bVertD6 )
                writeCurveTo( mnStackIdx, -5, -4, -3, -2, -1, 0);
            else
                writeCurveTo( mnStackIdx, -5, -4, -3, -2, 0, -1);
            mnStackIdx -= 11;
        }
        break;
    default:
        SAL_WARN("vcl.fonts", "unhandled type2esc " << nType2Esc);
        assert( false);
        break;
    }
}

void CffSubsetterContext::callType2Subr( bool bGlobal, int nSubrNumber)
{
    const U8* const pOldReadPtr = mpReadPtr;
    const U8* const pOldReadEnd = mpReadEnd;

    if( bGlobal ) {
        nSubrNumber += mnGlobalSubrBias;
        seekIndexData( mnGlobalSubrBase, nSubrNumber);
    } else {
        nSubrNumber += mpCffLocal->mnLocalSubrBias;
        seekIndexData( mpCffLocal->mnLocalSubrBase, nSubrNumber);
    }

    while( mpReadPtr < mpReadEnd)
        convertOneTypeOp();

    mpReadPtr = pOldReadPtr;
    mpReadEnd = pOldReadEnd;
}

int CffSubsetterContext::convert2Type1Ops( CffLocal* pCffLocal, const U8* const pT2Ops, int nT2Len, U8* const pT1Ops)
{
    mpCffLocal = pCffLocal;

    // prepare the charstring conversion
    mpWritePtr = pT1Ops;
    U8 aType1Ops[ MAX_T1OPS_SIZE];
    if( !pT1Ops)
        mpWritePtr = aType1Ops;
    *const_cast<U8**>(&pT1Ops) = mpWritePtr;

    // prepend random seed for T1crypt
    *(mpWritePtr++) = 0x48;
    *(mpWritePtr++) = 0x44;
    *(mpWritePtr++) = 0x55;
    *(mpWritePtr++) = ' ';

    // convert the Type2 charstring to Type1
    mpReadPtr = pT2Ops;
    mpReadEnd = pT2Ops + nT2Len;
    // prepend "hsbw" or "sbw"
    // TODO: only emit hsbw when charwidth is known
    writeType1Val(0); // TODO: aSubsetterContext.getLeftSideBearing();
    U8* pCharWidthPtr=mpWritePtr; // need to overwrite that later
    // pad out 5 bytes for the char width with default val 1000 (to be
    // filled with the actual value below)
    *(mpWritePtr++) = 255;
    *(mpWritePtr++) = static_cast<U8>(0);
    *(mpWritePtr++) = static_cast<U8>(0);
    *(mpWritePtr++) = static_cast<U8>(250);
    *(mpWritePtr++) = static_cast<U8>(124);
    writeTypeOp(TYPE1OP::HSBW);
    mbNeedClose = false;
    mbIgnoreHints = false;
    mnHintSize=mnHorzHintSize=mnStackIdx=0; maCharWidth=-1;//#######
    mnCntrMask = 0;
    while( mpReadPtr < mpReadEnd)
        convertOneTypeOp();
    if( maCharWidth != -1 )
    {
        // overwrite earlier charWidth value, which we only now have
        // parsed out of mpReadPtr buffer (by way of
        // convertOneTypeOp()s above)
        const int nInt = static_cast<int>(maCharWidth);
        *(pCharWidthPtr++) = 255;
        *(pCharWidthPtr++) = static_cast<U8>(nInt >> 24);
        *(pCharWidthPtr++) = static_cast<U8>(nInt >> 16);
        *(pCharWidthPtr++) = static_cast<U8>(nInt >> 8);
        *(pCharWidthPtr++) = static_cast<U8>(nInt);
    }

    const int nType1Len = mpWritePtr - pT1Ops;

    // encrypt the Type1 charstring
    unsigned nRDCryptR = 4330; // TODO: mnRDCryptSeed;
    for( U8* p = pT1Ops; p < mpWritePtr; ++p) {
        *p ^= (nRDCryptR >> 8);
        nRDCryptR = (*p + nRDCryptR) * 52845 + 22719;
    }

    return nType1Len;
}

RealType CffSubsetterContext::readRealVal()
{
    // TODO: more thorough number validity test
    bool bComma = false;
    int nExpVal = 0;
    int nExpSign = 0;
    S64 nNumber = 0;
    RealType fReal = +1.0;
    for(;;){
        const U8 c = *(mpReadPtr++); // read nibbles
        // parse high nibble
        const U8 nH = c >> 4U;
        if( nH <= 9) {
            nNumber = nNumber * 10 + nH;
            --nExpVal;
        } else if( nH == 10) {  // comma
            nExpVal = 0;
            bComma = true;
        } else if( nH == 11) {  // +exp
            fReal *= nNumber;
            nExpSign = +1;
            nNumber = 0;
        } else if( nH == 12) {  // -exp
            fReal *= nNumber;
            nExpSign = -1;
            nNumber = 0;
        } else if( nH == 13) {  // reserved
            // TODO: ignore or error?
        } else if( nH == 14)    // minus
            fReal = -fReal;
        else if( nH == 15)  // end
            break;
        // parse low nibble
        const U8 nL = c & 0x0F;
        if( nL <= 9) {
            nNumber = nNumber * 10 + nL;
            --nExpVal;
        } else if( nL == 10) {  // comma
            nExpVal = 0;
            bComma = true;
        } else if( nL == 11) {  // +exp
            fReal *= nNumber;
            nNumber = 0;
            nExpSign = +1;
        } else if( nL == 12) {  // -exp
            fReal *= nNumber;
            nNumber = 0;
            nExpSign = -1;
        } else if( nL == 13) {  // reserved
            // TODO: ignore or error?
        } else if( nL == 14)    // minus
            fReal = -fReal;
        else if( nL == 15)  // end
            break;
    }

    // merge exponents
    if( !bComma)
        nExpVal = 0;
    if( !nExpSign) { fReal *= nNumber;}
    else if( nExpSign > 0) { nExpVal += static_cast<int>(nNumber);}
    else if( nExpSign < 0) { nExpVal -= static_cast<int>(nNumber);}

    // apply exponents
    if( !nExpVal) { /*nothing to apply*/}
    else if( nExpVal > 0) { while( --nExpVal >= 0) fReal *= 10.0;}
    else if( nExpVal < 0) { while( ++nExpVal <= 0) fReal /= 10.0;}
    return fReal;
}

// prepare to access an element inside a CFF/CID index table
int CffSubsetterContext::seekIndexData( int nIndexBase, int nDataIndex)
{
    assert( (nIndexBase > 0) && (mpBasePtr + nIndexBase + 3 <= mpBaseEnd));
    if( nDataIndex < 0)
        return -1;
    mpReadPtr = mpBasePtr + nIndexBase;
    const int nDataCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
    if( nDataIndex >= nDataCount)
        return -1;
    const int nDataOfsSz = mpReadPtr[2];
    mpReadPtr += 3 + (nDataOfsSz * nDataIndex);
    int nOfs1 = 0;
    switch( nDataOfsSz) {
        default: SAL_WARN("vcl.fonts", "\tINVALID nDataOfsSz=" << nDataOfsSz); return -1;
        case 1: nOfs1 = mpReadPtr[0]; break;
        case 2: nOfs1 = (mpReadPtr[0]<<8) + mpReadPtr[1]; break;
        case 3: nOfs1 = (mpReadPtr[0]<<16) + (mpReadPtr[1]<<8) + mpReadPtr[2]; break;
        case 4: nOfs1 = (mpReadPtr[0]<<24) + (mpReadPtr[1]<<16) + (mpReadPtr[2]<<8) + mpReadPtr[3]; break;
    }
    mpReadPtr += nDataOfsSz;

    int nOfs2 = 0;
    switch( nDataOfsSz) {
        case 1: nOfs2 = mpReadPtr[0]; break;
        case 2: nOfs2 = (mpReadPtr[0]<<8) + mpReadPtr[1]; break;
        case 3: nOfs2 = (mpReadPtr[0]<<16) + (mpReadPtr[1]<<8) + mpReadPtr[2]; break;
        case 4: nOfs2 = (mpReadPtr[0]<<24) + (mpReadPtr[1]<<16) + (mpReadPtr[2]<<8) + mpReadPtr[3]; break;
    }

    mpReadPtr = mpBasePtr + (nIndexBase + 2) + nDataOfsSz * (nDataCount + 1) + nOfs1;
    mpReadEnd = mpReadPtr + (nOfs2 - nOfs1);
    assert( nOfs1 >= 0);
    assert( nOfs2 >= nOfs1);
    assert( mpReadPtr <= mpBaseEnd);
    assert( mpReadEnd <= mpBaseEnd);
    return (nOfs2 - nOfs1);
}

// skip over a CFF/CID index table
void CffSubsetterContext::seekIndexEnd( int nIndexBase)
{
    assert( (nIndexBase > 0) && (mpBasePtr + nIndexBase + 3 <= mpBaseEnd));
    mpReadPtr = mpBasePtr + nIndexBase;
    const int nDataCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
    const int nDataOfsSz = mpReadPtr[2];
    mpReadPtr += 3 + nDataOfsSz * nDataCount;
    assert( mpReadPtr <= mpBaseEnd);
    int nEndOfs = 0;
    switch( nDataOfsSz) {
        default: SAL_WARN("vcl.fonts", "\tINVALID nDataOfsSz=" << nDataOfsSz); return;
        case 1: nEndOfs = mpReadPtr[0]; break;
        case 2: nEndOfs = (mpReadPtr[0]<<8) + mpReadPtr[1]; break;
        case 3: nEndOfs = (mpReadPtr[0]<<16) + (mpReadPtr[1]<<8) + mpReadPtr[2];break;
        case 4: nEndOfs = (mpReadPtr[0]<<24) + (mpReadPtr[1]<<16) + (mpReadPtr[2]<<8) + mpReadPtr[3]; break;
    }
    mpReadPtr += nDataOfsSz;
    mpReadPtr += nEndOfs - 1;
    mpReadEnd = mpBaseEnd;
    assert( nEndOfs >= 0);
    assert( mpReadEnd <= mpBaseEnd);
}

// initialize FONTDICT specific values
CffLocal::CffLocal()
:   mnPrivDictBase( 0)
,   mnPrivDictSize( 0)
,   mnLocalSubrOffs( 0)
,   mnLocalSubrBase( 0)
,   mnLocalSubrBias( 0)
,   maNominalWidth( 0)
,   maDefaultWidth( 0)
,   maStemStdHW( 0)
,   maStemStdVW( 0)
,   mfBlueScale( 0.0)
,   mfBlueShift( 0.0)
,   mfBlueFuzz( 0.0)
,   mfExpFactor( 0.0)
,   mnLangGroup( 0)
,   mbForceBold( false)
{
}

CffGlobal::CffGlobal()
:   mnNameIdxBase( 0)
,   mnStringIdxBase( 0)
,   mbCIDFont( false)
,   mnCharStrBase( 0)
,   mnCharStrCount( 0)
,   mnCharsetBase( 0)
,   mnGlobalSubrBase( 0)
,   mnGlobalSubrCount( 0)
,   mnGlobalSubrBias( 0)
,   mnFDSelectBase( 0)
,   mnFontDictBase( 0)
,   mnFDAryCount( 1)
,   mnFontNameSID( 0)
,   mnFullNameSID( 0)
{
}

bool CffSubsetterContext::initialCffRead()
{
    // get the CFFHeader
    mpReadPtr = mpBasePtr;
    const U8 nVerMajor = *(mpReadPtr++);
    const U8 nVerMinor = *(mpReadPtr++);
    const U8 nHeaderSize = *(mpReadPtr++);
    const U8 nOffsetSize = *(mpReadPtr++);
    // TODO: is the version number useful for anything else?
    assert( (nVerMajor == 1) && (nVerMinor == 0));
    (void)(nVerMajor + nVerMinor + nOffsetSize); // avoid compiler warnings

    // prepare access to the NameIndex
    mnNameIdxBase = nHeaderSize;
    mpReadPtr = mpBasePtr + nHeaderSize;
    seekIndexEnd( mnNameIdxBase);

    // get the TopDict index
    const sal_Int32 nTopDictBase = getReadOfs();
    const int nTopDictCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
    if( nTopDictCount) {
        for( int i = 0; i < nTopDictCount; ++i) {
            seekIndexData( nTopDictBase, i);
            while( mpReadPtr < mpReadEnd)
                readDictOp();
            assert( mpReadPtr == mpReadEnd);
        }
    }

    // prepare access to the String index
    mnStringIdxBase =  getReadOfs();
    seekIndexEnd( mnStringIdxBase);

    // prepare access to the GlobalSubr index
    mnGlobalSubrBase =  getReadOfs();
    mnGlobalSubrCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
    mnGlobalSubrBias = (mnGlobalSubrCount<1240)?107:(mnGlobalSubrCount<33900)?1131:32768;
    // skip past the last GlobalSubr entry
//  seekIndexEnd( mnGlobalSubrBase);

    // get/skip the Encodings (we got mnEncodingBase from TOPDICT)
//  seekEncodingsEnd( mnEncodingBase);
    // get/skip the Charsets (we got mnCharsetBase from TOPDICT)
//  seekCharsetsEnd( mnCharStrBase);
    // get/skip FDSelect (CID only) data

    // prepare access to the CharStrings index (we got the base from TOPDICT)
    mpReadPtr = mpBasePtr + mnCharStrBase;
    mnCharStrCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
//  seekIndexEnd( mnCharStrBase);

    // read the FDArray index (CID only)
    if( mbCIDFont) {
//      assert( mnFontDictBase == tellRel());
        mpReadPtr = mpBasePtr + mnFontDictBase;
        mnFDAryCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
        if (o3tl::make_unsigned(mnFDAryCount) >= SAL_N_ELEMENTS(maCffLocal))
        {
            SAL_INFO("vcl.fonts", "CffSubsetterContext: too many CFF in font");
            return false;
        }

        // read FDArray details to get access to the PRIVDICTs
        for( int i = 0; i < mnFDAryCount; ++i) {
            mpCffLocal = &maCffLocal[i];
            seekIndexData( mnFontDictBase, i);
            while( mpReadPtr < mpReadEnd)
                readDictOp();
            assert( mpReadPtr == mpReadEnd);
        }
    }

    for( int i = 0; i < mnFDAryCount; ++i) {
        mpCffLocal = &maCffLocal[i];

        // get the PrivateDict index
        // (we got mnPrivDictSize and mnPrivDictBase from TOPDICT or FDArray)
        if( mpCffLocal->mnPrivDictSize != 0) {
            assert( mpCffLocal->mnPrivDictSize > 0);
            // get the PrivDict data
            mpReadPtr = mpBasePtr + mpCffLocal->mnPrivDictBase;
            mpReadEnd = mpReadPtr + mpCffLocal->mnPrivDictSize;
            assert( mpReadEnd <= mpBaseEnd);
            // read PrivDict details
            while( mpReadPtr < mpReadEnd)
                readDictOp();
        }

        // prepare access to the LocalSubrs (we got mnLocalSubrOffs from PRIVDICT)
        if( mpCffLocal->mnLocalSubrOffs) {
            // read LocalSubrs summary
            mpCffLocal->mnLocalSubrBase = mpCffLocal->mnPrivDictBase + mpCffLocal->mnLocalSubrOffs;
            mpReadPtr = mpBasePtr + mpCffLocal->mnLocalSubrBase;
            const int nSubrCount = (mpReadPtr[0] << 8) + mpReadPtr[1];
            mpCffLocal->mnLocalSubrBias = (nSubrCount<1240)?107:(nSubrCount<33900)?1131:32768;
//          seekIndexEnd( mpCffLocal->mnLocalSubrBase);
        }
    }

    // ignore the Notices info

    return true;
}

// get a cstring from a StringID
OString CffSubsetterContext::getString( int nStringID)
{
    // get a standard string if possible
    const static int nStdStrings = SAL_N_ELEMENTS(pStringIds);
    if( (nStringID >= 0) && (nStringID < nStdStrings))
        return pStringIds[ nStringID];

    // else get the string from the StringIndex table
    comphelper::ValueRestorationGuard pReadPtr(mpReadPtr);
    comphelper::ValueRestorationGuard pReadEnd(mpReadEnd);
    nStringID -= nStdStrings;
    int nLen = seekIndexData( mnStringIdxBase, nStringID);
    // assert( nLen >= 0);
    // TODO: just return the undecorated name
    if( nLen < 0) {
        return "name[" + OString::number(nStringID) + "].notfound!";
    } else {
        const int nMaxLen = 2560 - 1;
        if( nLen >= nMaxLen)
            nLen = nMaxLen; // TODO: still needed?
        return OString(reinterpret_cast<char const *>(mpReadPtr), nLen);
    }
}

// access a CID's FDSelect table
int CffSubsetterContext::getFDSelect( int nGlyphIndex) const
{
    assert( nGlyphIndex >= 0);
    assert( nGlyphIndex < mnCharStrCount);
    if( !mbCIDFont)
        return 0;

    const U8* pReadPtr = mpBasePtr + mnFDSelectBase;
    const U8 nFDSelFormat = *(pReadPtr++);
    switch( nFDSelFormat) {
        case 0: { // FDSELECT format 0
                pReadPtr += nGlyphIndex;
                const U8 nFDIdx = *(pReadPtr++);
                return nFDIdx;
            } //break;
        case 3: { // FDSELECT format 3
                const U16 nRangeCount = (pReadPtr[0]<<8) + pReadPtr[1];
                assert( nRangeCount > 0);
                assert( nRangeCount <= mnCharStrCount);
                U16 nPrev = (pReadPtr[2]<<8) + pReadPtr[3];
                assert( nPrev == 0);
                (void)nPrev;
                pReadPtr += 4;
                // TODO? binary search
                for( int i = 0; i < nRangeCount; ++i) {
                    const U8 nFDIdx = pReadPtr[0];
                    const U16 nNext = (pReadPtr[1]<<8) + pReadPtr[2];
                    assert( nPrev < nNext);
                    if( nGlyphIndex < nNext)
                        return nFDIdx;
                    pReadPtr += 3;
                    nPrev = nNext;
                }
            } break;
        default:    // invalid FDselect format
            SAL_WARN("vcl.fonts", "invalid CFF.FdselType=" << nFDSelFormat);
            break;
    }

    assert( false);
    return -1;
}

int CffSubsetterContext::getGlyphSID( int nGlyphIndex) const
{
    if( nGlyphIndex == 0)
        return 0;       // ".notdef"
    assert( nGlyphIndex >= 0);
    assert( nGlyphIndex < mnCharStrCount);
    if( (nGlyphIndex < 0) || (nGlyphIndex >= mnCharStrCount))
        return -1;

    // get the SID/CID from the Charset table
    const U8* pReadPtr = mpBasePtr + mnCharsetBase;
    const U8 nCSetFormat = *(pReadPtr++);
    int nGlyphsToSkip = nGlyphIndex - 1;
    switch( nCSetFormat) {
        case 0: // charset format 0
            pReadPtr += 2 * nGlyphsToSkip;
            nGlyphsToSkip = 0;
            break;
        case 1: // charset format 1
            while( nGlyphsToSkip >= 0) {
                const int nLeft = pReadPtr[2];
                if( nGlyphsToSkip <= nLeft)
                    break;
                nGlyphsToSkip -= nLeft + 1;
                pReadPtr += 3;
            }
            break;
        case 2: // charset format 2
            while( nGlyphsToSkip >= 0) {
                const int nLeft = (pReadPtr[2]<<8) + pReadPtr[3];
                if( nGlyphsToSkip <= nLeft)
                    break;
                nGlyphsToSkip -= nLeft + 1;
                pReadPtr += 4;
            }
            break;
        default:
            SAL_WARN("vcl.fonts", "ILLEGAL CFF-Charset format " << nCSetFormat);
            return -2;
    }

    int nSID = (pReadPtr[0]<<8) + pReadPtr[1];
    nSID += nGlyphsToSkip;
    // NOTE: for CID-fonts the resulting SID is interpreted as CID
    return nSID;
}

// NOTE: the result becomes invalid with the next call to this method
OString CffSubsetterContext::getGlyphName( int nGlyphIndex)
{
    // the first glyph is always the .notdef glyph
    if( nGlyphIndex == 0)
        return tok_notdef;

    // get the glyph specific name
    const int nSID = getGlyphSID( nGlyphIndex);
    if( nSID < 0)           // default glyph name
    {
        char aDefaultGlyphName[64];
        o3tl::sprintf( aDefaultGlyphName, "gly%03d", nGlyphIndex);
        return aDefaultGlyphName;
    }
    else if( mbCIDFont)     // default glyph name in CIDs
    {
         char aDefaultGlyphName[64];
         o3tl::sprintf( aDefaultGlyphName, "cid%03d", nSID);
         return aDefaultGlyphName;
    }
    else {                  // glyph name from string table
        auto const pSidName = getString( nSID);
        // check validity of glyph name
        const char* p = pSidName.getStr();
        while( (*p >= '0') && (*p <= 'z')) ++p;
        if( (p >= pSidName.getStr()+1) && (*p == '\0'))
            return pSidName;
        // if needed invent a fallback name
        char aDefaultGlyphName[64];
        o3tl::sprintf( aDefaultGlyphName, "bad%03d", nSID);
        return aDefaultGlyphName;
    }
}

bool CffSubsetterContext::getBaseAccent(ValType aBase, ValType aAccent, int* nBase, int* nAccent)
{
    bool bBase = false, bAccent = false;
    for (int i = 0; i < mnCharStrCount; i++)
    {
        OString pGlyphName = getGlyphName(i);
        if (pGlyphName == pStandardEncoding[int(aBase)])
        {
            *nBase = i;
            bBase = true;
        }
        if (pGlyphName == pStandardEncoding[int(aAccent)])
        {
            *nAccent = i;
            bAccent = true;
        }
        if (bBase && bAccent)
            return true;
    }
    return false;
}

namespace {

class Type1Emitter
{
public:
    explicit    Type1Emitter( SvStream* pOutFile, bool bPfbSubset);
    ~Type1Emitter();
    void        setSubsetName( const char* );

    size_t      emitRawData( const char* pData, size_t nLength) const;
    void        emitAllRaw();
    void        emitAllHex();
    void        emitAllCrypted();
    int         tellPos() const;
    void        updateLen( int nTellPos, size_t nLength);
    void        emitValVector( const char* pLineHead, const char* pLineTail, const std::vector<ValType>&);
private:
    SvStream*   mpFileOut;
    unsigned    mnEECryptR;
public:
    OStringBuffer maBuffer;

    char        maSubsetName[256];
    bool        mbPfbSubset;
    int         mnHexLineCol;
};

}

Type1Emitter::Type1Emitter( SvStream* pOutFile, bool bPfbSubset)
:   mpFileOut( pOutFile)
,   mnEECryptR( 55665)  // default eexec seed, TODO: mnEECryptSeed
,   mbPfbSubset( bPfbSubset)
,   mnHexLineCol( 0)
{
    maSubsetName[0] = '\0';
}

Type1Emitter::~Type1Emitter()
{
    if( !mpFileOut)
        return;
    mpFileOut = nullptr;
}

void Type1Emitter::setSubsetName( const char* pSubsetName)
{
    maSubsetName[0] = '\0';
    if( pSubsetName)
        strncpy( maSubsetName, pSubsetName, sizeof(maSubsetName) - 1);
    maSubsetName[sizeof(maSubsetName)-1] = '\0';
}

int Type1Emitter::tellPos() const
{
    int nTellPos = mpFileOut->Tell();
    return nTellPos;
}

void Type1Emitter::updateLen( int nTellPos, size_t nLength)
{
    // update PFB segment header length
    U8 cData[4];
    cData[0] = static_cast<U8>(nLength >>  0);
    cData[1] = static_cast<U8>(nLength >>  8);
    cData[2] = static_cast<U8>(nLength >> 16);
    cData[3] = static_cast<U8>(nLength >> 24);
    const tools::Long nCurrPos = mpFileOut->Tell();
    if (nCurrPos < 0)
        return;
    if (mpFileOut->Seek(nTellPos) != static_cast<sal_uInt64>(nTellPos))
        return;
    mpFileOut->WriteBytes(cData, sizeof(cData));
    mpFileOut->Seek(nCurrPos);
}

inline size_t Type1Emitter::emitRawData(const char* pData, size_t nLength) const
{
    return mpFileOut->WriteBytes( pData, nLength );
}

inline void Type1Emitter::emitAllRaw()
{
    // writeout raw data
    emitRawData( maBuffer.getStr(), maBuffer.getLength());
    // reset the raw buffer
    maBuffer.setLength(0);
}

inline void Type1Emitter::emitAllHex()
{
    auto const end = maBuffer.getStr() + maBuffer.getLength();
    for( const char* p = maBuffer.getStr(); p < end;) {
        // convert binary chunk to hex
        char aHexBuf[0x4000];
        char* pOut = aHexBuf;
        while( (p < end) && (pOut < aHexBuf+sizeof(aHexBuf)-4)) {
            // convert each byte to hex
            char cNibble = (static_cast<unsigned char>(*p) >> 4) & 0x0F;
            cNibble += (cNibble < 10) ? '0' : 'A'-10;
            *(pOut++) = cNibble;
            cNibble = *(p++) & 0x0F;
            cNibble += (cNibble < 10) ? '0' : 'A'-10;
            *(pOut++) = cNibble;
            // limit the line length
            if( (++mnHexLineCol & 0x3F) == 0)
                *(pOut++) = '\n';
        }
        // writeout hex-converted chunk
        emitRawData( aHexBuf, pOut-aHexBuf);
    }
    // reset the raw buffer
    maBuffer.setLength(0);
}

void Type1Emitter::emitAllCrypted()
{
    // apply t1crypt
    for( sal_Int32 i = 0; i < maBuffer.getLength(); ++i) {
        maBuffer[i] ^= (mnEECryptR >> 8);
        mnEECryptR = (static_cast<U8>(maBuffer[i]) + mnEECryptR) * 52845 + 22719;
    }

    // emit the t1crypt result
    if( mbPfbSubset)
        emitAllRaw();
    else
        emitAllHex();
}

// #i110387# quick-and-dirty double->ascii conversion
// also strip off trailing zeros in fraction while we are at it
static OString dbl2str( double fVal)
{
    return rtl::math::doubleToString(fVal, rtl_math_StringFormat_G, 6, '.', true);
}

void Type1Emitter::emitValVector( const char* pLineHead, const char* pLineTail,
    const std::vector<ValType>& rVector)
{
    // ignore empty vectors
    if( rVector.empty())
        return;

    // emit the line head
    maBuffer.append( pLineHead);
    // emit the vector values
    std::vector<ValType>::value_type aVal = 0;
    for( std::vector<ValType>::const_iterator it = rVector.begin();;) {
        aVal = *it;
        if( ++it == rVector.end() )
            break;
        maBuffer.append(dbl2str( aVal));
        maBuffer.append(' ');
    }
    // emit the last value
    maBuffer.append(dbl2str( aVal));
    // emit the line tail
    maBuffer.append( pLineTail);
}

void CffSubsetterContext::convertCharStrings(const sal_GlyphId* pGlyphIds, int nGlyphCount,
                                             std::vector<CharString>& rCharStrings)
{
    // If we are doing extra glyphs used for seac operator, check for already
    // converted glyphs.
    bool bCheckDuplicates = !rCharStrings.empty();
    rCharStrings.reserve(rCharStrings.size() + nGlyphCount);
    for (int i = 0; i < nGlyphCount; ++i)
    {
        const int nCffGlyphId = pGlyphIds[i];
        assert((nCffGlyphId >= 0) && (nCffGlyphId < mnCharStrCount));

        if (!bCheckDuplicates)
        {
            const auto& it
                = std::find_if(rCharStrings.begin(), rCharStrings.end(),
                               [&](const CharString& c) { return c.nCffGlyphId == nCffGlyphId; });
            if (it != rCharStrings.end())
                continue;
        }

        // get privdict context matching to the glyph
        const int nFDSelect = getFDSelect(nCffGlyphId);
        if (nFDSelect < 0)
            continue;
        mpCffLocal = &maCffLocal[nFDSelect];

        // convert the Type2op charstring to its Type1op counterpart
        const int nT2Len = seekIndexData(mnCharStrBase, nCffGlyphId);
        assert(nT2Len > 0);

        CharString aCharString;
        const int nT1Len = convert2Type1Ops(mpCffLocal, mpReadPtr, nT2Len, aCharString.aOps);
        aCharString.nLen = nT1Len;
        aCharString.nCffGlyphId = nCffGlyphId;

        rCharStrings.push_back(aCharString);
    }
}

void CffSubsetterContext::emitAsType1( Type1Emitter& rEmitter,
    const sal_GlyphId* pReqGlyphIds, const U8* pReqEncoding,
    int nGlyphCount, FontSubsetInfo& rFSInfo)
{
    // prepare some fontdirectory details
    static const int nUniqueIdBase = 4100000; // using private-interchange UniqueIds
    static int nUniqueId = nUniqueIdBase;
    ++nUniqueId;

    char* pFontName = rEmitter.maSubsetName;
    if( !*pFontName ) {
        if( mnFontNameSID) {
            // get the fontname directly if available
            strncpy(
                pFontName, getString( mnFontNameSID).getStr(), sizeof(rEmitter.maSubsetName) - 1);
            pFontName[sizeof(rEmitter.maSubsetName) - 1] = 0;
        } else if( mnFullNameSID) {
            // approximate fontname as fullname-whitespace
            auto const str = getString( mnFullNameSID);
            const char* pI = str.getStr();
            char* pO = pFontName;
            const char* pLimit = pFontName + sizeof(rEmitter.maSubsetName) - 1;
            while( pO < pLimit) {
                const char c = *(pI++);
                if( c != ' ')
                    *(pO++) = c;
                if( !c)
                    break;
            }
            *pO = '\0';
        } else {
            // fallback name of last resort
            strncpy( pFontName, "DummyName", sizeof(rEmitter.maSubsetName));
        }
    }
    const char* pFullName = pFontName;
    const char* pFamilyName = pFontName;

    // create a PFB+Type1 header
    if( rEmitter.mbPfbSubset ) {
        static const char aPfbHeader[] = "\x80\x01\x00\x00\x00\x00";
        rEmitter.emitRawData( aPfbHeader, sizeof(aPfbHeader)-1);
    }

    rEmitter.maBuffer.append(
        "%!FontType1-1.0: " + OString::Concat(rEmitter.maSubsetName) + " 001.003\n");
    // emit TOPDICT
    rEmitter.maBuffer.append(
        "11 dict begin\n"   // TODO: dynamic entry count for TOPDICT
        "/FontType 1 def\n"
        "/PaintType 0 def\n");
    rEmitter.maBuffer.append( "/FontName /" + OString::Concat(rEmitter.maSubsetName) + " def\n");
    rEmitter.maBuffer.append( "/UniqueID " + OString::number(nUniqueId) + " def\n");
    // emit FontMatrix
    if( maFontMatrix.size() == 6)
        rEmitter.emitValVector( "/FontMatrix [", "]readonly def\n", maFontMatrix);
    else // emit default FontMatrix if needed
        rEmitter.maBuffer.append( "/FontMatrix [0.001 0 0 0.001 0 0]readonly def\n");
    // emit FontBBox
    auto aFontBBox = maFontBBox;
    if (aFontBBox.size() != 4)
        aFontBBox = { 0, 0, 999, 999 }; // emit default FontBBox if needed
    rEmitter.emitValVector( "/FontBBox {", "}readonly def\n", aFontBBox);
    // emit FONTINFO into TOPDICT
    rEmitter.maBuffer.append(
        "/FontInfo 2 dict dup begin\n"  // TODO: check fontinfo entry count
        " /FullName (" + OString::Concat(pFullName) + ") readonly def\n"
        " /FamilyName (" + pFamilyName + ") readonly def\n"
        "end readonly def\n");

    rEmitter.maBuffer.append(
        "/Encoding 256 array\n"
        "0 1 255 {1 index exch /.notdef put} for\n");
    for( int i = 1; (i < nGlyphCount) && (i < 256); ++i) {
        OString pGlyphName = getGlyphName( pReqGlyphIds[i]);
        rEmitter.maBuffer.append(
            "dup " + OString::number(pReqEncoding[i]) + " /" + pGlyphName + " put\n");
    }
    rEmitter.maBuffer.append( "readonly def\n");
    rEmitter.maBuffer.append(
        // TODO: more topdict entries
        "currentdict end\n"
        "currentfile eexec\n");

    // emit PFB header
    rEmitter.emitAllRaw();
    if( rEmitter.mbPfbSubset) {
        // update PFB header segment
        const int nPfbHeaderLen = rEmitter.tellPos() - 6;
        rEmitter.updateLen( 2, nPfbHeaderLen);

        // prepare start of eexec segment
        rEmitter.emitRawData( "\x80\x02\x00\x00\x00\x00", 6);   // segment start
    }
    const int nEExecSegTell = rEmitter.tellPos();

    // which always starts with a privdict
    // count the privdict entries
    int nPrivEntryCount = 9;
    // emit blue hints only if non-default values
    nPrivEntryCount += int(!mpCffLocal->maOtherBlues.empty());
    nPrivEntryCount += int(!mpCffLocal->maFamilyBlues.empty());
    nPrivEntryCount += int(!mpCffLocal->maFamilyOtherBlues.empty());
    nPrivEntryCount += int(mpCffLocal->mfBlueScale != 0.0);
    nPrivEntryCount += int(mpCffLocal->mfBlueShift != 0.0);
    nPrivEntryCount += int(mpCffLocal->mfBlueFuzz != 0.0);
    // emit stem hints only if non-default values
    nPrivEntryCount += int(mpCffLocal->maStemStdHW != 0);
    nPrivEntryCount += int(mpCffLocal->maStemStdVW != 0);
    nPrivEntryCount += int(!mpCffLocal->maStemSnapH.empty());
    nPrivEntryCount += int(!mpCffLocal->maStemSnapV.empty());
    // emit other hints only if non-default values
    nPrivEntryCount += int(mpCffLocal->mfExpFactor != 0.0);
    nPrivEntryCount += int(mpCffLocal->mnLangGroup != 0);
    nPrivEntryCount += int(mpCffLocal->mnLangGroup == 1);
    nPrivEntryCount += int(mpCffLocal->mbForceBold);
    // emit the privdict header
    rEmitter.maBuffer.append(
        "\110\104\125 "
        "dup\n/Private " + OString::number(nPrivEntryCount) + " dict dup begin\n"
        "/RD{string currentfile exch readstring pop}executeonly def\n"
        "/ND{noaccess def}executeonly def\n"
        "/NP{noaccess put}executeonly def\n"
        "/MinFeature{16 16}ND\n"
        "/password 5839 def\n");    // TODO: mnRDCryptSeed?

    // emit blue hint related privdict entries
    if( !mpCffLocal->maBlueValues.empty())
        rEmitter.emitValVector( "/BlueValues [", "]ND\n", mpCffLocal->maBlueValues);
    else
        rEmitter.maBuffer.append( "/BlueValues []ND\n"); // default to empty BlueValues
    rEmitter.emitValVector( "/OtherBlues [", "]ND\n", mpCffLocal->maOtherBlues);
    rEmitter.emitValVector( "/FamilyBlues [", "]ND\n", mpCffLocal->maFamilyBlues);
    rEmitter.emitValVector( "/FamilyOtherBlues [", "]ND\n", mpCffLocal->maFamilyOtherBlues);

    if( mpCffLocal->mfBlueScale) {
        rEmitter.maBuffer.append( "/BlueScale ");
        rEmitter.maBuffer.append(dbl2str( mpCffLocal->mfBlueScale));
        rEmitter.maBuffer.append( " def\n");
    }
    if( mpCffLocal->mfBlueShift) {  // default BlueShift==7
        rEmitter.maBuffer.append( "/BlueShift ");
        rEmitter.maBuffer.append(dbl2str( mpCffLocal->mfBlueShift));
        rEmitter.maBuffer.append( " def\n");
    }
    if( mpCffLocal->mfBlueFuzz) {       // default BlueFuzz==1
        rEmitter.maBuffer.append( "/BlueFuzz ");
        rEmitter.maBuffer.append(dbl2str( mpCffLocal->mfBlueFuzz));
        rEmitter.maBuffer.append( " def\n");
    }

    // emit stem hint related privdict entries
    if( mpCffLocal->maStemStdHW) {
        rEmitter.maBuffer.append( "/StdHW [");
        rEmitter.maBuffer.append(dbl2str( mpCffLocal->maStemStdHW));
        rEmitter.maBuffer.append( "] def\n");
    }
    if( mpCffLocal->maStemStdVW) {
        rEmitter.maBuffer.append( "/StdVW [");
        rEmitter.maBuffer.append(dbl2str( mpCffLocal->maStemStdVW));
        rEmitter.maBuffer.append( "] def\n");
    }
    rEmitter.emitValVector( "/StemSnapH [", "]ND\n", mpCffLocal->maStemSnapH);
    rEmitter.emitValVector( "/StemSnapV [", "]ND\n", mpCffLocal->maStemSnapV);

    // emit other hints
    if( mpCffLocal->mbForceBold)
        rEmitter.maBuffer.append( "/ForceBold true def\n");
    if( mpCffLocal->mnLangGroup != 0)
        rEmitter.maBuffer.append(
            "/LanguageGroup " + OString::number(mpCffLocal->mnLangGroup) + " def\n");
    if( mpCffLocal->mnLangGroup == 1) // compatibility with ancient printers
        rEmitter.maBuffer.append( "/RndStemUp false def\n");
    if( mpCffLocal->mfExpFactor) {
        rEmitter.maBuffer.append( "/ExpansionFactor ");
        rEmitter.maBuffer.append(dbl2str( mpCffLocal->mfExpFactor));
        rEmitter.maBuffer.append( " def\n");
    }

    // emit remaining privdict entries
    rEmitter.maBuffer.append( "/UniqueID " + OString::number(nUniqueId) + " def\n");
    // TODO?: more privdict entries?

    rEmitter.maBuffer.append(
        "/OtherSubrs\n"
        "% Dummy code for faking flex hints\n"
        "[ {} {} {} {systemdict /internaldict known not {pop 3}\n"
        "{1183615869 systemdict /internaldict get exec\n"
        "dup /startlock known\n"
        "{/startlock get exec}\n"
        "{dup /strtlck known\n"
        "{/strtlck get exec}\n"
        "{pop 3}\nifelse}\nifelse}\nifelse\n} executeonly\n"
        "] ND\n");

    // emit used GlobalSubr charstrings
    // these are the just the default subrs
    // TODO: do we need them as the flex hints are resolved differently?
    rEmitter.maBuffer.append(
        "/Subrs 5 array\n"
        "dup 0 15 RD \x5F\x3D\x6B\xAC\x3C\xBD\x74\x3D\x3E\x17\xA0\x86\x58\x08\x85 NP\n"
        "dup 1 9 RD \x5F\x3D\x6B\xD8\xA6\xB5\x68\xB6\xA2 NP\n"
        "dup 2 9 RD \x5F\x3D\x6B\xAC\x39\x46\xB9\x43\xF9 NP\n"
        "dup 3 5 RD \x5F\x3D\x6B\xAC\xB9 NP\n"
        "dup 4 12 RD \x5F\x3D\x6B\xAC\x3E\x5D\x48\x54\x62\x76\x39\x03 NP\n"
        "ND\n");

    // TODO: emit more GlobalSubr charstrings?
    // TODO: emit used LocalSubr charstrings?

    // emit the CharStrings for the requested glyphs
    std::vector<CharString> aCharStrings;
    mbDoSeac = true;
    convertCharStrings(pReqGlyphIds, nGlyphCount, aCharStrings);

    // The previous convertCharStrings might collect extra glyphs used in seac
    // operator, convert them as well
    if (!maExtraGlyphIds.empty())
    {
        mbDoSeac = false;
        convertCharStrings(maExtraGlyphIds.data(), maExtraGlyphIds.size(), aCharStrings);
    }
    rEmitter.maBuffer.append(
        "2 index /CharStrings " + OString::number(aCharStrings.size()) + " dict dup begin\n");
    rEmitter.emitAllCrypted();
    for (const auto& rCharString : aCharStrings)
    {
        // get the glyph name
        OString pGlyphName = getGlyphName(rCharString.nCffGlyphId);
        // emit the encrypted Type1op charstring
        rEmitter.maBuffer.append(
            "/" + pGlyphName + " " + OString::number(rCharString.nLen) + " RD ");
        rEmitter.maBuffer.append(
            reinterpret_cast<char const *>(rCharString.aOps), rCharString.nLen);
        rEmitter.maBuffer.append( " ND\n");
        rEmitter.emitAllCrypted();
        // provide individual glyphwidths if requested
    }
    rEmitter.maBuffer.append( "end end\nreadonly put\nput\n");
    rEmitter.maBuffer.append( "dup/FontName get exch definefont pop\n");
    rEmitter.maBuffer.append( "mark currentfile closefile\n");
    rEmitter.emitAllCrypted();

    // mark stop of eexec encryption
    if( rEmitter.mbPfbSubset) {
        const int nEExecLen = rEmitter.tellPos() - nEExecSegTell;
        rEmitter.updateLen( nEExecSegTell-4, nEExecLen);
    }

    // create PFB footer
    static const char aPfxFooter[] = "\x80\x01\x14\x02\x00\x00\n" // TODO: check segment len
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "cleartomark\n"
        "\x80\x03";
    if( rEmitter.mbPfbSubset)
        rEmitter.emitRawData( aPfxFooter, sizeof(aPfxFooter)-1);
    else
        rEmitter.emitRawData( aPfxFooter+6, sizeof(aPfxFooter)-9);

    // provide details to the subset requesters, TODO: move into own method?
    // note: Top and Bottom are flipped between Type1 and VCL
    // note: the rest of VCL expects the details below to be scaled like for an emUnits==1000 font
    ValType fXFactor = 1.0;
    ValType fYFactor = 1.0;
    if( maFontMatrix.size() >= 4) {
        fXFactor = 1000.0F * maFontMatrix[0];
        fYFactor = 1000.0F * maFontMatrix[3];
    }
    rFSInfo.m_aFontBBox = { Point(static_cast<sal_Int32>(aFontBBox[0] * fXFactor),
                                  static_cast<sal_Int32>(aFontBBox[1] * fYFactor)),
                            Point(static_cast<sal_Int32>(aFontBBox[2] * fXFactor),
                                  static_cast<sal_Int32>(aFontBBox[3] * fYFactor)) };
    // PDF-Spec says the values below mean the ink bounds!
    // TODO: use better approximations for these ink bounds
    rFSInfo.m_nAscent  = +rFSInfo.m_aFontBBox.Bottom(); // for capital letters
    rFSInfo.m_nDescent = -rFSInfo.m_aFontBBox.Top();    // for all letters
    rFSInfo.m_nCapHeight = rFSInfo.m_nAscent;           // for top-flat capital letters

    rFSInfo.m_nFontType = rEmitter.mbPfbSubset ? FontType::TYPE1_PFB : FontType::TYPE1_PFA;
    rFSInfo.m_aPSName   = OUString( rEmitter.maSubsetName, strlen(rEmitter.maSubsetName), RTL_TEXTENCODING_UTF8 );
}

bool FontSubsetInfo::CreateFontSubsetFromCff()
{
    CffSubsetterContext aCff( mpInFontBytes, mnInByteLength);
    bool bRC = aCff.initialCffRead();
    if (!bRC)
        return bRC;

    // emit Type1 subset from the CFF input
    // TODO: also support CFF->CFF subsetting (when PDF-export and PS-printing need it)
    const bool bPfbSubset(mnReqFontTypeMask & FontType::TYPE1_PFB);
    Type1Emitter aType1Emitter( mpOutFile, bPfbSubset);
    aType1Emitter.setSubsetName( mpReqFontName);
    aCff.emitAsType1( aType1Emitter,
        mpReqGlyphIds, mpReqEncodedIds,
        mnReqGlyphCount, *this);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
