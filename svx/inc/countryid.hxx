/*************************************************************************
 *
 *  $RCSfile: countryid.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-23 17:25:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ============================================================================

#ifndef INCLUDED_SVX_COUNTRYID_HXX
#define INCLUDED_SVX_COUNTRYID_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

// ----------------------------------------------------------------------------

namespace svx {

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
const CountryId COUNTRY_CARRIBEAN           =   1;  // no own ID
const CountryId COUNTRY_DOMINICAN_REPUBLIC  =   1;  // no own ID
const CountryId COUNTRY_JAMAICA             =   1;  // no own ID
const CountryId COUNTRY_PUERTO_RICO         =   1;  // no own ID
const CountryId COUNTRY_TRINIDAD_Y_TOBAGO   =   1;  // no own ID
const CountryId COUNTRY_CANADA              =   2;
const CountryId COUNTRY_RUSSIA              =   7;
const CountryId COUNTRY_KAZACHSTAN          =   7;  // no own ID
const CountryId COUNTRY_TAJIKISTAN          =   7;  // no own ID
const CountryId COUNTRY_TATARSTAN           =   7;  // no own ID
const CountryId COUNTRY_UZBEKISTAN          =   7;  // no own ID
const CountryId COUNTRY_EGYPT               =  20;
const CountryId COUNTRY_SOUTH_AFRICA        =  27;
const CountryId COUNTRY_BOTSWANA            =  27;  // no own ID
const CountryId COUNTRY_LESOTHO             =  27;  // no own ID
const CountryId COUNTRY_GREECE              =  30;
const CountryId COUNTRY_NETHERLANDS         =  31;
const CountryId COUNTRY_BELGIUM             =  32;
const CountryId COUNTRY_FRANCE              =  33;
const CountryId COUNTRY_CAMEROON            =  33;  // no own ID
const CountryId COUNTRY_COTE_D_IVOIRE       =  33;  // no own ID
const CountryId COUNTRY_MALI                =  33;  // no own ID
const CountryId COUNTRY_REUNION             =  33;  // no own ID
const CountryId COUNTRY_SENEGAL             =  33;  // no own ID
const CountryId COUNTRY_WEST_INDIES         =  33;  // no own ID
const CountryId COUNTRY_ZAIRE               =  33;  // no own ID
const CountryId COUNTRY_SPAIN               =  34;
const CountryId COUNTRY_HUNGARY             =  36;
const CountryId COUNTRY_ITALY               =  39;
const CountryId COUNTRY_ROMANIA             =  40;
const CountryId COUNTRY_MOLDOVA             =  40;  // no own ID
const CountryId COUNTRY_SWITZERLAND         =  41;
const CountryId COUNTRY_LIECHTENSTEIN       =  41;  // no own ID
const CountryId COUNTRY_AUSTRIA             =  43;
const CountryId COUNTRY_UNITED_KINGDOM      =  44;
const CountryId COUNTRY_DENMARK             =  45;
const CountryId COUNTRY_SWEDEN              =  46;
const CountryId COUNTRY_NORWAY              =  47;
const CountryId COUNTRY_POLAND              =  48;
const CountryId COUNTRY_GERMANY             =  49;
const CountryId COUNTRY_PERU                =  51;
const CountryId COUNTRY_MEXICO              =  52;
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
const CountryId COUNTRY_CAMBODIA            =  66;  // no own ID
const CountryId COUNTRY_JAPAN               =  81;
const CountryId COUNTRY_SOUTH_KOREA         =  82;
const CountryId COUNTRY_VIET_NAM            =  84;
const CountryId COUNTRY_PR_CHINA            =  86;
const CountryId COUNTRY_TIBET               =  86;  // no own ID
const CountryId COUNTRY_TURKEY              =  90;
const CountryId COUNTRY_INDIA               =  91;
const CountryId COUNTRY_BANGLADESH          =  91;  // no own ID
const CountryId COUNTRY_NEPAL               =  91;  // no own ID
const CountryId COUNTRY_PAKISTAN            =  92;
const CountryId COUNTRY_MOROCCO             = 212;
const CountryId COUNTRY_ALGERIA             = 213;
const CountryId COUNTRY_TUNISIA             = 216;
const CountryId COUNTRY_LIBYA               = 218;
const CountryId COUNTRY_KENYA               = 254;
const CountryId COUNTRY_ZIMBABWE            = 263;
const CountryId COUNTRY_FAEROE_ISLANDS      = 298;
const CountryId COUNTRY_PORTUGAL            = 351;
const CountryId COUNTRY_LUXEMBOURG          = 352;
const CountryId COUNTRY_IRELAND             = 353;
const CountryId COUNTRY_ICELAND             = 354;
const CountryId COUNTRY_ALBANIA             = 355;
const CountryId COUNTRY_FINLAND             = 358;
const CountryId COUNTRY_BULGARIA            = 359;
const CountryId COUNTRY_LITHUANIA           = 370;
const CountryId COUNTRY_LATVIA              = 371;
const CountryId COUNTRY_ESTONIA             = 372;
const CountryId COUNTRY_ARMENIA             = 374;
const CountryId COUNTRY_BELARUS             = 375;
const CountryId COUNTRY_MONACO              = 377;
const CountryId COUNTRY_UKRAINE             = 380;
const CountryId COUNTRY_SERBIA              = 381;
const CountryId COUNTRY_CROATIA             = 385;
const CountryId COUNTRY_SLOVENIA            = 386;
const CountryId COUNTRY_MACEDONIA           = 389;
const CountryId COUNTRY_CZECH               = 420;
const CountryId COUNTRY_SLOVAK              = 421;
const CountryId COUNTRY_BELIZE              = 501;
const CountryId COUNTRY_GUATEMALA           = 502;
const CountryId COUNTRY_EL_SALVADOR         = 503;
const CountryId COUNTRY_HONDURAS            = 504;
const CountryId COUNTRY_NICARAGUA           = 505;
const CountryId COUNTRY_COSTA_RICA          = 506;
const CountryId COUNTRY_PANAMA              = 507;
const CountryId COUNTRY_BOLIVIA             = 591;
const CountryId COUNTRY_ECUADOR             = 593;
const CountryId COUNTRY_PARAGUAY            = 595;
const CountryId COUNTRY_URUGUAY             = 598;
const CountryId COUNTRY_BRUNEI_DARUSSALAM   = 673;
const CountryId COUNTRY_HONG_KONG           = 852;
const CountryId COUNTRY_MACAU               = 853;
const CountryId COUNTRY_TAIWAN              = 886;
const CountryId COUNTRY_MALEDIVES           = 960;
const CountryId COUNTRY_LEBANON             = 961;
const CountryId COUNTRY_JORDAN              = 962;
const CountryId COUNTRY_SYRIA               = 963;
const CountryId COUNTRY_IRAQ                = 964;
const CountryId COUNTRY_KUWAIT              = 965;
const CountryId COUNTRY_SAUDI_ARABIA        = 966;
const CountryId COUNTRY_YEMEN               = 967;
const CountryId COUNTRY_OMAN                = 968;
const CountryId COUNTRY_UAE                 = 971;
const CountryId COUNTRY_ISRAEL              = 972;
const CountryId COUNTRY_BAHRAIN             = 973;
const CountryId COUNTRY_QATAR               = 974;
const CountryId COUNTRY_MONGOLIA            = 976;
const CountryId COUNTRY_IRAN                = 981;
const CountryId COUNTRY_TURKMENISTAN        = 981;  // no own ID
const CountryId COUNTRY_AZERBAIJAN          = 994;
const CountryId COUNTRY_GEORGIA             = 995;
const CountryId COUNTRY_KYRGYZSTAN          = 996;

// Country ID <-> Language type conversion ====================================

/** Converts a language type to a Windows country ID.

    The function regards the sub type of the passed language, and tries to
    return the appropriate country, i.e. COUNTRY_IRELAND for
    LANGUAGE_ENGLISH_EIRE.

    A few countries do not have an own associated country ID. Most of these
    countries are mapped to another related country, i.e. Liechtenstein is
    mapped to Switzerland.

    If no country can be found, the value COUNTRY_DONTKNOW will be returned.

    @param eLanguage
        A language type, defined in tools/lang.hxx.

    @return
        The best Windows country ID for the passed language type, or
        COUNTRY_DONTKNOW on error.
 */
CountryId ConvertLanguageToCountry( LanguageType eLanguage );

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
LanguageType ConvertCountryToLanguage( CountryId eCountry );

// ============================================================================

} // namespace svx

// ============================================================================

#endif

