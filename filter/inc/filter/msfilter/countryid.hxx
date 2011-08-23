/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// ============================================================================

#ifndef INCLUDED_SVX_COUNTRYID_HXX
#define INCLUDED_SVX_COUNTRYID_HXX

#include <sal/types.h>
#include <i18npool/lang.h>
#include "filter/msfilter/msfilterdllapi.h"

// ----------------------------------------------------------------------------

namespace msfilter {

// Windows Country IDs ========================================================

/** Represents a Windows country ID.

    The country IDs used in Windows are equal to the international calling
    code of each country, i.e. the code 353 represents Ireland (+353).

    ATTENTION: These country codes are not supposed to be used anywhere in core
    structures! They shall ONLY be used in the import/export filters for
    Microsoft Office documents!
 */
typedef sal_uInt16 CountryId;

const CountryId COUNTRY_DONTKNOW            =   0;
const CountryId COUNTRY_USA                 =   1;
const CountryId COUNTRY_DOMINICAN_REPUBLIC  =   1;  // NANP
const CountryId COUNTRY_GUAM                =   1;  // NANP
const CountryId COUNTRY_JAMAICA             =   1;  // NANP
const CountryId COUNTRY_PUERTO_RICO         =   1;  // NANP
const CountryId COUNTRY_TRINIDAD_Y_TOBAGO   =   1;  // NANP
const CountryId COUNTRY_CANADA              =   2;
const CountryId COUNTRY_RUSSIA              =   7;
const CountryId COUNTRY_KAZAKHSTAN          =   7;  // no own ID
const CountryId COUNTRY_TATARSTAN           =   7;  // no own ID
const CountryId COUNTRY_EGYPT               =  20;
const CountryId COUNTRY_SOUTH_AFRICA        =  27;
const CountryId COUNTRY_GREECE              =  30;
const CountryId COUNTRY_NETHERLANDS         =  31;
const CountryId COUNTRY_BELGIUM             =  32;
const CountryId COUNTRY_FRANCE              =  33;
const CountryId COUNTRY_SPAIN               =  34;
const CountryId COUNTRY_HUNGARY             =  36;
const CountryId COUNTRY_ITALY               =  39;
const CountryId COUNTRY_ROMANIA             =  40;
const CountryId COUNTRY_SWITZERLAND         =  41;
const CountryId COUNTRY_AUSTRIA             =  43;
const CountryId COUNTRY_UNITED_KINGDOM      =  44;
const CountryId COUNTRY_DENMARK             =  45;
const CountryId COUNTRY_SWEDEN              =  46;
const CountryId COUNTRY_NORWAY              =  47;
const CountryId COUNTRY_POLAND              =  48;
const CountryId COUNTRY_GERMANY             =  49;
const CountryId COUNTRY_PERU                =  51;
const CountryId COUNTRY_MEXICO              =  52;
const CountryId COUNTRY_CUBA                =  53;
const CountryId COUNTRY_ARGENTINIA          =  54;
const CountryId COUNTRY_BRAZIL              =  55;
const CountryId COUNTRY_CHILE               =  56;
const CountryId COUNTRY_COLOMBIA            =  57;
const CountryId COUNTRY_VENEZUELA           =  58;
const CountryId COUNTRY_MALAYSIA            =  60;
const CountryId COUNTRY_AUSTRALIA           =  61;
const CountryId COUNTRY_INDONESIA           =  62;
const CountryId COUNTRY_PHILIPPINES         =  63;
const CountryId COUNTRY_NEW_ZEALAND         =  64;
const CountryId COUNTRY_SINGAPORE           =  65;
const CountryId COUNTRY_THAILAND            =  66;
const CountryId COUNTRY_JAPAN               =  81;
const CountryId COUNTRY_SOUTH_KOREA         =  82;
const CountryId COUNTRY_VIET_NAM            =  84;
const CountryId COUNTRY_PR_CHINA            =  86;
const CountryId COUNTRY_TIBET               =  86;  // no own ID
const CountryId COUNTRY_TURKEY              =  90;
const CountryId COUNTRY_INDIA               =  91;
const CountryId COUNTRY_PAKISTAN            =  92;
const CountryId COUNTRY_AFGHANISTAN         =  93;
const CountryId COUNTRY_SRI_LANKA           =  94;
const CountryId COUNTRY_MYANMAR             =  95;
const CountryId COUNTRY_MOROCCO             = 212;
const CountryId COUNTRY_ALGERIA             = 213;
const CountryId COUNTRY_TUNISIA             = 216;
const CountryId COUNTRY_LIBYA               = 218;
const CountryId COUNTRY_GAMBIA              = 220;
const CountryId COUNTRY_SENEGAL             = 221;
const CountryId COUNTRY_MAURITANIA          = 222;
const CountryId COUNTRY_MALI                = 223;
const CountryId COUNTRY_GUINEA              = 224;
const CountryId COUNTRY_COTE_D_IVOIRE       = 225;
const CountryId COUNTRY_BURKINA_FARSO       = 226;
const CountryId COUNTRY_NIGER               = 227;
const CountryId COUNTRY_TOGO                = 228;
const CountryId COUNTRY_BENIN               = 229;
const CountryId COUNTRY_MAURITIUS           = 230;
const CountryId COUNTRY_LIBERIA             = 231;
const CountryId COUNTRY_SIERRA_LEONE        = 232;
const CountryId COUNTRY_GHANA               = 233;
const CountryId COUNTRY_NIGERIA             = 234;
const CountryId COUNTRY_CHAD                = 235;
const CountryId COUNTRY_CENTRAL_AFR_REP     = 236;
const CountryId COUNTRY_CAMEROON            = 237;
const CountryId COUNTRY_CAPE_VERDE          = 238;
const CountryId COUNTRY_SAO_TOME            = 239;
const CountryId COUNTRY_EQUATORIAL_GUINEA   = 240;
const CountryId COUNTRY_GABON               = 241;
const CountryId COUNTRY_CONGO               = 242;
const CountryId COUNTRY_ZAIRE               = 243;
const CountryId COUNTRY_ANGOLA              = 244;
const CountryId COUNTRY_GUINEA_BISSAU       = 245;
const CountryId COUNTRY_DIEGO_GARCIA        = 246;
const CountryId COUNTRY_ASCENSION_ISLAND    = 247;
const CountryId COUNTRY_SEYCHELLES          = 248;
const CountryId COUNTRY_SUDAN               = 249;
const CountryId COUNTRY_RWANDA              = 250;
const CountryId COUNTRY_ETHIOPIA            = 251;
const CountryId COUNTRY_SOMALIA             = 252;
const CountryId COUNTRY_DJIBOUTI            = 253;
const CountryId COUNTRY_KENYA               = 254;
const CountryId COUNTRY_TANZANIA            = 255;
const CountryId COUNTRY_UGANDA              = 256;
const CountryId COUNTRY_BURUNDI             = 257;
const CountryId COUNTRY_MOZAMBIQUE          = 258;
const CountryId COUNTRY_ZANZIBAR            = 259;
const CountryId COUNTRY_ZAMBIA              = 260;
const CountryId COUNTRY_MADAGASKAR          = 261;
const CountryId COUNTRY_REUNION             = 262;
const CountryId COUNTRY_ZIMBABWE            = 263;
const CountryId COUNTRY_NAMIBIA             = 264;
const CountryId COUNTRY_MALAWI              = 265;
const CountryId COUNTRY_LESOTHO             = 266;
const CountryId COUNTRY_BOTSWANA            = 267;
const CountryId COUNTRY_SWAZILAND           = 268;
const CountryId COUNTRY_COMOROS_MAYOTTE     = 269;
const CountryId COUNTRY_ST_HELENA           = 290;
const CountryId COUNTRY_ERITREA             = 291;
const CountryId COUNTRY_ARUBA               = 297;
const CountryId COUNTRY_FAEROE_ISLANDS      = 298;
const CountryId COUNTRY_GREEN_ISLAND        = 299;
const CountryId COUNTRY_GIBRALTAR           = 350;
const CountryId COUNTRY_PORTUGAL            = 351;
const CountryId COUNTRY_LUXEMBOURG          = 352;
const CountryId COUNTRY_IRELAND             = 353;
const CountryId COUNTRY_ICELAND             = 354;
const CountryId COUNTRY_ALBANIA             = 355;
const CountryId COUNTRY_MALTA               = 356;
const CountryId COUNTRY_CYPRUS              = 357;
const CountryId COUNTRY_FINLAND             = 358;
const CountryId COUNTRY_BULGARIA            = 359;
const CountryId COUNTRY_LITHUANIA           = 370;
const CountryId COUNTRY_LATVIA              = 371;
const CountryId COUNTRY_ESTONIA             = 372;
const CountryId COUNTRY_MOLDOVA             = 373;
const CountryId COUNTRY_ARMENIA             = 374;
const CountryId COUNTRY_BELARUS             = 375;
const CountryId COUNTRY_ANDORRA             = 376;
const CountryId COUNTRY_MONACO              = 377;
const CountryId COUNTRY_SAN_MARINO          = 378;
const CountryId COUNTRY_VATICAN_CITY        = 379;
const CountryId COUNTRY_UKRAINE             = 380;
const CountryId COUNTRY_SERBIA              = 381;
const CountryId COUNTRY_CROATIA             = 385;
const CountryId COUNTRY_SLOVENIA            = 386;
const CountryId COUNTRY_BOSNIA              = 387;
const CountryId COUNTRY_MACEDONIA           = 389;
const CountryId COUNTRY_CZECH               = 420;
const CountryId COUNTRY_SLOVAK              = 421;
const CountryId COUNTRY_LIECHTENSTEIN       = 423;
const CountryId COUNTRY_FALKLAND_ISLANDS    = 500;
const CountryId COUNTRY_BELIZE              = 501;
const CountryId COUNTRY_GUATEMALA           = 502;
const CountryId COUNTRY_EL_SALVADOR         = 503;
const CountryId COUNTRY_HONDURAS            = 504;
const CountryId COUNTRY_NICARAGUA           = 505;
const CountryId COUNTRY_COSTA_RICA          = 506;
const CountryId COUNTRY_PANAMA              = 507;
const CountryId COUNTRY_ST_PIERRE           = 508;
const CountryId COUNTRY_HAITI               = 509;
const CountryId COUNTRY_GUADELOUPE          = 590;
const CountryId COUNTRY_BOLIVIA             = 591;
const CountryId COUNTRY_GUYANA              = 592;
const CountryId COUNTRY_ECUADOR             = 593;
const CountryId COUNTRY_FRENCH_GUIANA       = 594;
const CountryId COUNTRY_PARAGUAY            = 595;
const CountryId COUNTRY_MARTINIQUE          = 596;
const CountryId COUNTRY_SURINAME            = 597;
const CountryId COUNTRY_URUGUAY             = 598;
const CountryId COUNTRY_NETHERL_ANTILLES    = 599;
const CountryId COUNTRY_EAST_TIMOR          = 670;
const CountryId COUNTRY_ANTARCTICA          = 672;
const CountryId COUNTRY_BRUNEI_DARUSSALAM   = 673;
const CountryId COUNTRY_NARUPU              = 674;
const CountryId COUNTRY_PAPUA_NEW_GUINEA    = 675;
const CountryId COUNTRY_TONGA               = 676;
const CountryId COUNTRY_SOLOMON_ISLANDS     = 677;
const CountryId COUNTRY_VANUATU             = 678;
const CountryId COUNTRY_FIJI                = 679;
const CountryId COUNTRY_PALAU               = 680;
const CountryId COUNTRY_WALLIS_AND_FUTUNA   = 681;
const CountryId COUNTRY_COOK_ISLANDS        = 682;
const CountryId COUNTRY_NIUE_ISLAND         = 683;
const CountryId COUNTRY_AMERICAN_SAMOA      = 684;
const CountryId COUNTRY_WESTERN_SAMOA       = 685;
const CountryId COUNTRY_KIRIBATI            = 686;
const CountryId COUNTRY_NEW_CALEDONIA       = 687;
const CountryId COUNTRY_TUVALU              = 688;
const CountryId COUNTRY_FRENCH_POLYNESIA    = 689;
const CountryId COUNTRY_TOKELAU             = 690;
const CountryId COUNTRY_MICRONESIA          = 691;
const CountryId COUNTRY_MARSHALL_ISLANDS    = 692;
const CountryId COUNTRY_NORTH_KOREA         = 850;
const CountryId COUNTRY_HONG_KONG           = 852;
const CountryId COUNTRY_MACAU               = 853;
const CountryId COUNTRY_CAMBODIA            = 855;
const CountryId COUNTRY_LAOS                = 856;
const CountryId COUNTRY_BANGLADESH          = 880;
const CountryId COUNTRY_TAIWAN              = 886;
const CountryId COUNTRY_MALDIVES            = 960;
const CountryId COUNTRY_LEBANON             = 961;
const CountryId COUNTRY_JORDAN              = 962;
const CountryId COUNTRY_SYRIA               = 963;
const CountryId COUNTRY_IRAQ                = 964;
const CountryId COUNTRY_KUWAIT              = 965;
const CountryId COUNTRY_SAUDI_ARABIA        = 966;
const CountryId COUNTRY_YEMEN               = 967;
const CountryId COUNTRY_OMAN                = 968;
const CountryId COUNTRY_PALESTINE           = 970;
const CountryId COUNTRY_UAE                 = 971;
const CountryId COUNTRY_ISRAEL              = 972;
const CountryId COUNTRY_BAHRAIN             = 973;
const CountryId COUNTRY_QATAR               = 974;
const CountryId COUNTRY_BHUTAN              = 975;
const CountryId COUNTRY_MONGOLIA            = 976;
const CountryId COUNTRY_NEPAL               = 977;
const CountryId COUNTRY_IRAN                = 981;
const CountryId COUNTRY_TAJIKISTAN          = 992;
const CountryId COUNTRY_TURKMENISTAN        = 993;
const CountryId COUNTRY_AZERBAIJAN          = 994;
const CountryId COUNTRY_GEORGIA             = 995;
const CountryId COUNTRY_KYRGYZSTAN          = 996;
const CountryId COUNTRY_UZBEKISTAN          = 998;

// Country ID <-> Language type conversion ====================================

/** Converts a language type to a Windows country ID.

    The function regards the sub type of the passed language, and tries to
    return the appropriate country, i.e. COUNTRY_IRELAND for
    LANGUAGE_ENGLISH_EIRE.

    A few countries do not have an own associated country ID. Most of these
    countries are mapped to another related country, i.e. Kazakhstan is
    mapped to Russia.

    If no country can be found, the value COUNTRY_DONTKNOW will be returned.

    @param eLanguage
        A language type, defined in tools/lang.hxx.

    @return
        The best Windows country ID for the passed language type, or
        COUNTRY_DONTKNOW on error.
 */
MSFILTER_DLLPUBLIC CountryId ConvertLanguageToCountry( LanguageType eLanguage );

/** Converts a Windows country ID to a language type.

    The function returns the most used language for the passed country
    (hopefully), i.e. LANGUAGE_ENGLISH_EIRE for COUNTRY_IRELAND.

    If the passed country ID is not valid, the value LANGUAGE_DONTKNOW will be
    returned.

    @param eCountry
        A Windows country ID, defined above.

    @return
        The most used language type in the passed country, or LANGUAGE_DONTKNOW
        on error.
 */
MSFILTER_DLLPUBLIC LanguageType ConvertCountryToLanguage( CountryId eCountry );

// ============================================================================

} // namespace svx

// ============================================================================

#endif

