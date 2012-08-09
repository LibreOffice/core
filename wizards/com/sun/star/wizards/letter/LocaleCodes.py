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
from wizards.common.Resource import Resource

class LocaleCodes(Resource):
    UNIT_NAME = "svtres",
    MODULE_NAME = "svt",

    def __init__(self, xmsf):
        super(LocaleCodes, self).__init__(xmsf, LocaleCodes.MODULE_NAME)
        #self.allLanguageStrings = self.getStringList(16633)

    def getLanguageString(self, MSID):
        LS = "unknown Language",
        for i in self.allLanguageStrings:
            if str(i.Value).lower() == MSID.lower():
                LS = i.Name
        return LS

    def getIDs(self):
        Ids = ("Afrikaans;af;1078",
        "Albanian;sq;1052",
        "Arabic - United Arab Emirates;ar-ae;14337",
        "Arabic - Bahrain;ar-bh;15361",
        "Arabic - Algeria;ar-dz;5121",
        "Arabic - Egypt;ar-eg;3073",
        "Arabic - Iraq;ar-iq;2049",
        "Arabic - Jordan;ar-jo;11265",
        "Arabic - Kuwait;ar-kw;13313",
        "Arabic - Lebanon;ar-lb;12289",
        "Arabic - Libya;ar-ly;4097",
        "Arabic - Morocco;ar-ma;6145",
        "Arabic - Oman;ar-om;8193",
        "Arabic - Qatar;ar-qa;16385",
        "Arabic - Saudi Arabia;ar-sa;1025",
        "Arabic - Syria;ar-sy;10241",
        "Arabic - Tunisia;ar-tn;7169",
        "Arabic - Yemen;ar-ye;9217",
        "Armenian;hy;1067",
        "Azeri - Latin;az-az;1068",
        "Azeri - Cyrillic;az-az;2092",
        "Basque;eu;1069",
        "Belarusian;be;1059",
        "Bulgarian;bg;1026",
        "Catalan;ca;1027",
        "Chinese - China;zh-cn;2052",
        "Chinese - Hong Kong SAR;zh-hk;3076",
        "Chinese - Macau SAR;zh-mo;5124",
        "Chinese - Singapore;zh-sg;4100",
        "Chinese - Taiwan;zh-tw;1028",
        "Croatian;hr;1050",
        "Czech;cs;1029",
        "Danish;da;1030",
        "Dutch - The Netherlands;nl-nl;1043",
        "Dutch - Belgium;nl-be;2067",
        "English - Australia;en-au;3081",
        "English - Belize;en-bz;10249",
        "English - Canada;en-ca;4105",
        "English - Caribbean;en-cb;9225",
        "English - Ireland;en-ie;6153",
        "English - Jamaica;en-jm;8201",
        "English - New Zealand;en-nz;5129",
        "English - Phillippines;en-ph;13321",
        "English - South Africa;en-za;7177",
        "English - Trinidad;en-tt;11273",
        "English - United Kingdom;en-gb;2057",
        "English - United States;en-us;1033",
        "Estonian;et;1061",
        "Farsi;fa;1065",
        "Finnish;fi;1035",
        "Faroese;fo;1080",
        "French - France;fr-fr;1036",
        "French - Belgium;fr-be;2060",
        "French - Canada;fr-ca;3084",
        "French - Luxembourg;fr-lu;5132",
        "French - Switzerland;fr-ch;4108",
        "Gaelic - Ireland;gd-ie;2108",
        "Gaelic - Scotland;gd;1084",
        "German - Germany;de-de;1031",
        "German - Austria;de-at;3079",
        "German - Liechtenstein;de-li;5127",
        "German - Luxembourg;de-lu;4103",
        "German - Switzerland;de-ch;2055",
        "Greek;el;1032",
        "Hebrew;he;1037",
        "Hindi;hi;1081",
        "Hungarian;hu;1038",
        "Icelandic;is;1039",
        "Indonesian;id;1057",
        "Italian - Italy;it-it;1040",
        "Italian - Switzerland;it-ch;2064",
        "Japanese;ja;1041",
        "Korean;ko;1042",
        "Latvian;lv;1062",
        "Lithuanian;lt;1063",
        "FYRO Macedonian;mk;1071",
        "Malay - Malaysia;ms-my;1086",
        "Malay - Brunei;ms-bn;2110",
        "Maltese;mt;1082",
        "Marathi;mr;1102",
        "Norwegian - Bokm?l;no-no;1044",
        "Norwegian - Nynorsk;no-no;2068",
        "Polish;pl;1045",
        "Portuguese - Portugal;pt-pt;2070",
        "Portuguese - Brazil;pt-br;1046",
        "Raeto-Romance;rm;1047",
        "Romanian - Romania;ro;1048",
        "Romanian - Moldova;ro-mo;2072",
        "Russian;ru;1049",
        "Russian - Moldova;ru-mo;2073",
        "Sanskrit;sa;1103",
        "Serbian - Cyrillic;sr-sp;3098",
        "Serbian - Latin;sr-sp;2074",
        "Setsuana;tn;1074",
        "Slovenian;sl;1060",
        "Slovak;sk;1051",
        "Sorbian;sb;1070",
        "Spanish - Spain;es-es;3082",
        "Spanish - Argentina;es-ar;11274",
        "Spanish - Bolivia;es-bo;16394",
        "Spanish - Chile;es-cl;13322",
        "Spanish - Colombia;es-co;9226",
        "Spanish - Costa Rica;es-cr;5130",
        "Spanish - Dominican Republic;es-do;7178",
        "Spanish - Ecuador;es-ec;12298",
        "Spanish - Guatemala;es-gt;4106",
        "Spanish - Honduras;es-hn;18442",
        "Spanish - Mexico;es-mx;2058",
        "Spanish - Nicaragua;es-ni;19466",
        "Spanish - Panama;es-pa;6154",
        "Spanish - Peru;es-pe;10250",
        "Spanish - Puerto Rico;es-pr;20490",
        "Spanish - Paraguay;es-py;15370",
        "Spanish - El Salvador;es-sv;17418",
        "Spanish - Uruguay;es-uy;14346",
        "Spanish - Venezuela;es-ve;8202",
        "Sutu;sx;1072",
        "Swahili;sw;1089",
        "Swedish - Sweden;sv-se;1053",
        "Swedish - Finland;sv-fi;2077",
        "Tamil;ta;1097",
        "Tatar;tt;1092",
        "Thai;th;1054",
        "Turkish;tr;1055",
        "Tsonga;ts;1073",
        "Ukrainian;uk;1058",
        "Urdu;ur;1056",
        "Uzbek - Cyrillic;uz-uz;2115",
        "Uzbek - Latin;uz-uz;1091",
        "Vietnamese;vi;1066",
        "Xhosa;xh;1076",
        "Yiddish;yi;1085",
        "Zulu;zu;1077",
        "Khmer;km-kh;1107",
        "Burmese;my-mm;1109")
        return Ids
