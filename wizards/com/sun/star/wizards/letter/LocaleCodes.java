/*
 ************************************************************************
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
package com.sun.star.wizards.letter;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Resource;
import com.sun.star.beans.PropertyValue;

public class LocaleCodes extends Resource
{

    final static String UNIT_NAME = "svtres";
    final static String MODULE_NAME = "svt";
    PropertyValue[] allLanguageStrings;

    public LocaleCodes(XMultiServiceFactory xmsf)
    {
        super(xmsf, UNIT_NAME, MODULE_NAME);
        allLanguageStrings = getStringList(16633);  // STR_ARR_SVT_LANGUAGE_TABLE from svtools/source/misc/langtab.src
    }

    public String getLanguageString(String MSID)
    {
        String LS = "unknown Language";
        for (int i = 0; i < allLanguageStrings.length; i++)
        {
            if (allLanguageStrings[i].Value.toString().equalsIgnoreCase(MSID))
            {
                LS = allLanguageStrings[i].Name;
            }
        }
        return LS;
    }

    public String[] getIDs()
    {
        String[] Ids = new String[135];

        Ids[0] = "Afrikaans;af;1078";
        Ids[1] = "Albanian;sq;1052";
        Ids[2] = "Arabic - United Arab Emirates;ar-ae;14337";
        Ids[3] = "Arabic - Bahrain;ar-bh;15361";
        Ids[4] = "Arabic - Algeria;ar-dz;5121";
        Ids[5] = "Arabic - Egypt;ar-eg;3073";
        Ids[6] = "Arabic - Iraq;ar-iq;2049";
        Ids[7] = "Arabic - Jordan;ar-jo;11265";
        Ids[8] = "Arabic - Kuwait;ar-kw;13313";
        Ids[9] = "Arabic - Lebanon;ar-lb;12289";
        Ids[10] = "Arabic - Libya;ar-ly;4097";
        Ids[11] = "Arabic - Morocco;ar-ma;6145";
        Ids[12] = "Arabic - Oman;ar-om;8193";
        Ids[13] = "Arabic - Qatar;ar-qa;16385";
        Ids[14] = "Arabic - Saudi Arabia;ar-sa;1025";
        Ids[15] = "Arabic - Syria;ar-sy;10241";
        Ids[16] = "Arabic - Tunisia;ar-tn;7169";
        Ids[17] = "Arabic - Yemen;ar-ye;9217";
        Ids[18] = "Armenian;hy;1067";
        Ids[19] = "Azeri - Latin;az-az;1068";
        Ids[20] = "Azeri - Cyrillic;az-az;2092";
        Ids[21] = "Basque;eu;1069";
        Ids[22] = "Belarusian;be;1059";
        Ids[23] = "Bulgarian;bg;1026";
        Ids[24] = "Catalan;ca;1027";
        Ids[25] = "Chinese - China;zh-cn;2052";
        Ids[26] = "Chinese - Hong Kong SAR;zh-hk;3076";
        Ids[27] = "Chinese - Macau SAR;zh-mo;5124";
        Ids[28] = "Chinese - Singapore;zh-sg;4100";
        Ids[29] = "Chinese - Taiwan;zh-tw;1028";
        Ids[30] = "Croatian;hr;1050";
        Ids[31] = "Czech;cs;1029";
        Ids[32] = "Danish;da;1030";
        Ids[33] = "Dutch - The Netherlands;nl-nl;1043";
        Ids[34] = "Dutch - Belgium;nl-be;2067";
        Ids[35] = "English - Australia;en-au;3081";
        Ids[36] = "English - Belize;en-bz;10249";
        Ids[37] = "English - Canada;en-ca;4105";
        Ids[38] = "English - Caribbean;en-cb;9225";
        Ids[39] = "English - Ireland;en-ie;6153";
        Ids[40] = "English - Jamaica;en-jm;8201";
        Ids[41] = "English - New Zealand;en-nz;5129";
        Ids[42] = "English - Phillippines;en-ph;13321";
        Ids[43] = "English - South Africa;en-za;7177";
        Ids[44] = "English - Trinidad;en-tt;11273";
        Ids[45] = "English - United Kingdom;en-gb;2057";
        Ids[46] = "English - United States;en-us;1033";
        Ids[47] = "Estonian;et;1061";
        Ids[48] = "Farsi;fa;1065";
        Ids[49] = "Finnish;fi;1035";
        Ids[50] = "Faroese;fo;1080";
        Ids[51] = "French - France;fr-fr;1036";
        Ids[52] = "French - Belgium;fr-be;2060";
        Ids[53] = "French - Canada;fr-ca;3084";
        Ids[54] = "French - Luxembourg;fr-lu;5132";
        Ids[55] = "French - Switzerland;fr-ch;4108";
        Ids[56] = "Gaelic - Ireland;gd-ie;2108";
        Ids[57] = "Gaelic - Scotland;gd;1084";
        Ids[58] = "German - Germany;de-de;1031";
        Ids[59] = "German - Austria;de-at;3079";
        Ids[60] = "German - Liechtenstein;de-li;5127";
        Ids[61] = "German - Luxembourg;de-lu;4103";
        Ids[62] = "German - Switzerland;de-ch;2055";
        Ids[63] = "Greek;el;1032";
        Ids[64] = "Hebrew;he;1037";
        Ids[65] = "Hindi;hi;1081";
        Ids[66] = "Hungarian;hu;1038";
        Ids[67] = "Icelandic;is;1039";
        Ids[68] = "Indonesian;id;1057";
        Ids[69] = "Italian - Italy;it-it;1040";
        Ids[70] = "Italian - Switzerland;it-ch;2064";
        Ids[71] = "Japanese;ja;1041";
        Ids[72] = "Korean;ko;1042";
        Ids[73] = "Latvian;lv;1062";
        Ids[74] = "Lithuanian;lt;1063";
        Ids[75] = "FYRO Macedonian;mk;1071";
        Ids[76] = "Malay - Malaysia;ms-my;1086";
        Ids[77] = "Malay - Brunei;ms-bn;2110";
        Ids[78] = "Maltese;mt;1082";
        Ids[79] = "Marathi;mr;1102";
        Ids[80] = "Norwegian - Bokm?l;no-no;1044";
        Ids[81] = "Norwegian - Nynorsk;no-no;2068";
        Ids[82] = "Polish;pl;1045";
        Ids[83] = "Portuguese - Portugal;pt-pt;2070";
        Ids[84] = "Portuguese - Brazil;pt-br;1046";
        Ids[85] = "Raeto-Romance;rm;1047";
        Ids[86] = "Romanian - Romania;ro;1048";
        Ids[87] = "Romanian - Moldova;ro-mo;2072";
        Ids[88] = "Russian;ru;1049";
        Ids[89] = "Russian - Moldova;ru-mo;2073";
        Ids[90] = "Sanskrit;sa;1103";
        Ids[91] = "Serbian - Cyrillic;sr-sp;3098";
        Ids[92] = "Serbian - Latin;sr-sp;2074";
        Ids[93] = "Setsuana;tn;1074";
        Ids[94] = "Slovenian;sl;1060";
        Ids[95] = "Slovak;sk;1051";
        Ids[96] = "Sorbian;sb;1070";
        Ids[97] = "Spanish - Spain;es-es;3082";
        Ids[98] = "Spanish - Argentina;es-ar;11274";
        Ids[99] = "Spanish - Bolivia;es-bo;16394";
        Ids[100] = "Spanish - Chile;es-cl;13322";
        Ids[101] = "Spanish - Colombia;es-co;9226";
        Ids[102] = "Spanish - Costa Rica;es-cr;5130";
        Ids[103] = "Spanish - Dominican Republic;es-do;7178";
        Ids[104] = "Spanish - Ecuador;es-ec;12298";
        Ids[105] = "Spanish - Guatemala;es-gt;4106";
        Ids[106] = "Spanish - Honduras;es-hn;18442";
        Ids[107] = "Spanish - Mexico;es-mx;2058";
        Ids[108] = "Spanish - Nicaragua;es-ni;19466";
        Ids[109] = "Spanish - Panama;es-pa;6154";
        Ids[110] = "Spanish - Peru;es-pe;10250";
        Ids[111] = "Spanish - Puerto Rico;es-pr;20490";
        Ids[112] = "Spanish - Paraguay;es-py;15370";
        Ids[113] = "Spanish - El Salvador;es-sv;17418";
        Ids[114] = "Spanish - Uruguay;es-uy;14346";
        Ids[115] = "Spanish - Venezuela;es-ve;8202";
        Ids[116] = "Sutu;sx;1072";
        Ids[117] = "Swahili;sw;1089";
        Ids[118] = "Swedish - Sweden;sv-se;1053";
        Ids[119] = "Swedish - Finland;sv-fi;2077";
        Ids[120] = "Tamil;ta;1097";
        Ids[121] = "Tatar;tt;1092";
        Ids[122] = "Thai;th;1054";
        Ids[123] = "Turkish;tr;1055";
        Ids[124] = "Tsonga;ts;1073";
        Ids[125] = "Ukrainian;uk;1058";
        Ids[126] = "Urdu;ur;1056";
        Ids[127] = "Uzbek - Cyrillic;uz-uz;2115";
        Ids[128] = "Uzbek - Latin;uz-uz;1091";
        Ids[129] = "Vietnamese;vi;1066";
        Ids[130] = "Xhosa;xh;1076";
        Ids[131] = "Yiddish;yi;1085";
        Ids[132] = "Zulu;zu;1077";
        Ids[133] = "Khmer;km-kh;1107";
        Ids[134] = "Burmese;my-mm;1109";

        return Ids;
    }
}
