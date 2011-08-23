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

#ifndef _SVDDEF_HXX
#define _SVDDEF_HXX

namespace binfilter {

/*************************************************************************/
// SdrItemPool V2, 09-11-1995:
//      7 neue Items fuer XLineAttrSetItem
//      7 neue Items fuer XFillAttrSetItem
//      8 neue Items fuer SdrShadowSetItem
//      5 neue Items fuer SdrCaptionSetItem
//     25 neue Items fuer SdrMiscSetItem
//      6 neue Items fuer SdrNotPersist
// SdrItemPool V3, 29-02-1996
//     17 neue Items fuer XFillAttrSetItem (Bitmapfuellungen) (von KA)
// SdrItemPool V4, 10-08-1996
//     45 neue Items fuer Edge und Measure (Verbinder und Bemassungsobjekt) (von Joe)
// SdrItemPool V4+, 18-10-1996
//      8 neue Items fuer CircItems
//     18 neue Items fuer NotPersist+Umbau dort
// SdrItemPool V4++, 13-11-1996
//      2 neue Items fuer NotPersist+(MoveX/Y)
// SdrItemPool V4+++, 07-06-1999
//     15 neue Items fuer NotPersist (Reserve)
//	   15 neue Items fuer GrafItems
//
//   WhichMapping:
//     V1 Range = 1000..1065  (66)
//     V2 Range = 1000..1123 (124) (58 neue Whiches)
//     V3 Range = 1000..1140 (141) (17 neue Whiches)
//     V4 Range = 1000..1185 (186) (45 neue Whiches)
//     Map V2:  10           6          20           4          11           6           9
//     V1:  1000..1009, 1010..1015, 1016..1035, 1036..1039, 1040..1050, 1051..1056  1057..1065
//     V2:  1000..1009, 1017..1022, 1030..1049, 1050..1053, 1062..1072, 1078..1083, 1109..1117, und hinten noch 6 neue dran
//          |<------------ XOut ------------>|  |<----------- SvDraw ----------->
//     Map V3:
//     V2:  1000..1029, 1030..1123
//     V3:  1000..1029, 1047..1140
//     Map V4:
//     V3:  1000..1126, 1127..1140
//     V4:  1000..1126, 1172..1185

#define SDRATTR_START               XATTR_START                    /* 1000   */
                                                                   /* Pool V4*/ /* Pool V3*/ /* Pool V2*/
#define SDRATTR_SHADOW_FIRST            (XATTR_END + 1)            /* 1067   */ /* 1067   */ /* 1050   */ /* Pool V1: 1036 */
#define SDRATTR_SHADOW                  (SDRATTR_SHADOW_FIRST+ 0)  /*   1067 */ /*   1067 */ /*   1050 */
#define SDRATTR_SHADOWCOLOR             (SDRATTR_SHADOW_FIRST+ 1)  /*   1068 */ /*   1068 */ /*   1051 */
#define SDRATTR_SHADOWXDIST             (SDRATTR_SHADOW_FIRST+ 2)  /*   1069 */ /*   1069 */ /*   1052 */
#define SDRATTR_SHADOWYDIST             (SDRATTR_SHADOW_FIRST+ 3)  /*   1070 */ /*   1070 */ /*   1053 */
#define SDRATTR_SHADOWTRANSPARENCE      (SDRATTR_SHADOW_FIRST+ 4)  /*   1071 */ /*   1071 */ /*   1054 */ /* Pool V2 */
#define SDRATTR_SHADOW3D                (SDRATTR_SHADOW_FIRST+ 5)  /*   1072 */ /*   1072 */ /*   1055 */ /* Pool V2 */
#define SDRATTR_SHADOWPERSP             (SDRATTR_SHADOW_FIRST+ 6)  /*   1073 */ /*   1073 */ /*   1056 */ /* Pool V2 */
#define SDRATTR_SHADOWRESERVE1          (SDRATTR_SHADOW_FIRST+ 7)  /*   1074 */ /*   1074 */ /*   1057 */ /* Pool V2 */
#define SDRATTR_SHADOWRESERVE2          (SDRATTR_SHADOW_FIRST+ 8)  /*   1075 */ /*   1075 */ /*   1058 */ /* Pool V2 */
#define SDRATTR_SHADOWRESERVE3          (SDRATTR_SHADOW_FIRST+ 9)  /*   1076 */ /*   1076 */ /*   1059 */ /* Pool V2 */
#define SDRATTR_SHADOWRESERVE4          (SDRATTR_SHADOW_FIRST+10)  /*   1077 */ /*   1077 */ /*   1060 */ /* Pool V2 */
#define SDRATTR_SHADOWRESERVE5          (SDRATTR_SHADOW_FIRST+11)  /*   1078 */ /*   1078 */ /*   1061 */ /* Pool V2 */
#define SDRATTR_SHADOW_LAST             (SDRATTR_SHADOWRESERVE5)   /* 1078   */ /* 1078   */ /* 1061   */ /* Pool V1: 1039 */
#define SDRATTRSET_SHADOW               (SDRATTR_SHADOW_LAST + 1)  /*   1079 */ /*   1079 */ /*   1062 */ /* Pool V1: 1040 */

#define SDRATTR_CAPTION_FIRST           (SDRATTRSET_SHADOW + 1)    /* 1080   */ /* 1080   */ /* 1063   */ /* Pool V1: 1041 */
#define SDRATTR_CAPTIONTYPE             (SDRATTR_CAPTION_FIRST+ 0) /*   1080 */ /*   1080 */ /*   1063 */
#define SDRATTR_CAPTIONFIXEDANGLE       (SDRATTR_CAPTION_FIRST+ 1) /*   1081 */ /*   1081 */ /*   1064 */
#define SDRATTR_CAPTIONANGLE            (SDRATTR_CAPTION_FIRST+ 2) /*   1082 */ /*   1082 */ /*   1065 */
#define SDRATTR_CAPTIONGAP              (SDRATTR_CAPTION_FIRST+ 3) /*   1083 */ /*   1083 */ /*   1066 */
#define SDRATTR_CAPTIONESCDIR           (SDRATTR_CAPTION_FIRST+ 4) /*   1084 */ /*   1084 */ /*   1067 */
#define SDRATTR_CAPTIONESCISREL         (SDRATTR_CAPTION_FIRST+ 5) /*   1085 */ /*   1085 */ /*   1068 */
#define SDRATTR_CAPTIONESCREL           (SDRATTR_CAPTION_FIRST+ 6) /*   1086 */ /*   1086 */ /*   1069 */
#define SDRATTR_CAPTIONESCABS           (SDRATTR_CAPTION_FIRST+ 7) /*   1087 */ /*   1087 */ /*   1070 */
#define SDRATTR_CAPTIONLINELEN          (SDRATTR_CAPTION_FIRST+ 8) /*   1088 */ /*   1088 */ /*   1071 */
#define SDRATTR_CAPTIONFITLINELEN       (SDRATTR_CAPTION_FIRST+ 9) /*   1089 */ /*   1089 */ /*   1072 */
#define SDRATTR_CAPTIONRESERVE1         (SDRATTR_CAPTION_FIRST+10) /*   1090 */ /*   1090 */ /*   1073 */ /* Pool V2 */
#define SDRATTR_CAPTIONRESERVE2         (SDRATTR_CAPTION_FIRST+11) /*   1091 */ /*   1091 */ /*   1074 */ /* Pool V2 */
#define SDRATTR_CAPTIONRESERVE3         (SDRATTR_CAPTION_FIRST+12) /*   1092 */ /*   1092 */ /*   1075 */ /* Pool V2 */
#define SDRATTR_CAPTIONRESERVE4         (SDRATTR_CAPTION_FIRST+13) /*   1093 */ /*   1093 */ /*   1076 */ /* Pool V2 */
#define SDRATTR_CAPTIONRESERVE5         (SDRATTR_CAPTION_FIRST+14) /*   1094 */ /*   1094 */ /*   1077 */ /* Pool V2 */
#define SDRATTR_CAPTION_LAST            (SDRATTR_CAPTIONRESERVE5)  /* 1094   */ /* 1094   */ /* 1077   */ /* Pool V1: 1050 */
#define SDRATTRSET_CAPTION              (SDRATTR_CAPTION_LAST + 1) /*   1095 */ /*   1095 */ /*   1078 */ /* Pool V1: 1051 */

#define SDRATTRSET_OUTLINER             (SDRATTRSET_CAPTION + 1)   /*   1096 */ /*   1096 */ /*   1079 */ /* Pool V1: 1052 */

#define SDRATTR_MISC_FIRST              (SDRATTRSET_OUTLINER + 1)  /* 1097   */ /* 1097   */ /* 1080   */ /* Pool V1: 1053 */
#define SDRATTR_ECKENRADIUS             (SDRATTR_MISC_FIRST + 0)   /*   1097 */ /*   1097 */ /*   1080 long, def=0       */
#define SDRATTR_TEXT_MINFRAMEHEIGHT     (SDRATTR_MISC_FIRST + 1)   /*   1098 */ /*   1098 */ /*   1081 long, def=0       */
#define SDRATTR_TEXT_AUTOGROWHEIGHT     (SDRATTR_MISC_FIRST + 2)   /*   1099 */ /*   1099 */ /*   1082 BOOL, def=TRUE    */
#define SDRATTR_TEXT_FITTOSIZE          (SDRATTR_MISC_FIRST + 3)   /*   1100 */ /*   1100 */ /*   1083 enum, def=SDRTEXTFIT_NONE */
#define SDRATTR_TEXT_LEFTDIST           (SDRATTR_MISC_FIRST + 4)   /*   1101 */ /*   1101 */ /*   1084 long, def=0 */ /* Pool V2 */
#define SDRATTR_TEXT_RIGHTDIST          (SDRATTR_MISC_FIRST + 5)   /*   1102 */ /*   1102 */ /*   1085 long, def=0 */ /* Pool V2 */
#define SDRATTR_TEXT_UPPERDIST          (SDRATTR_MISC_FIRST + 6)   /*   1103 */ /*   1103 */ /*   1086 long, def=0 */ /* Pool V2 */
#define SDRATTR_TEXT_LOWERDIST          (SDRATTR_MISC_FIRST + 7)   /*   1104 */ /*   1104 */ /*   1087 long, def=0 */ /* Pool V2 */
#define SDRATTR_TEXT_VERTADJUST         (SDRATTR_MISC_FIRST + 8)   /*   1105 */ /*   1105 */ /*   1088 enum, def=SDRTEXTVERTADJUST_TOP */ /* Pool V2 */
#define SDRATTR_TEXT_MAXFRAMEHEIGHT     (SDRATTR_MISC_FIRST + 9)   /*   1106 */ /*   1106 */ /*   1089 long, def=0     - Pool V2 */
#define SDRATTR_TEXT_MINFRAMEWIDTH      (SDRATTR_MISC_FIRST +10)   /*   1107 */ /*   1107 */ /*   1090 long, def=0     - Pool V2 */
#define SDRATTR_TEXT_MAXFRAMEWIDTH      (SDRATTR_MISC_FIRST +11)   /*   1108 */ /*   1108 */ /*   1091 long, def=0     - Pool V2 */
#define SDRATTR_TEXT_AUTOGROWWIDTH      (SDRATTR_MISC_FIRST +12)   /*   1109 */ /*   1109 */ /*   1092 BOOL, def=FALSE - Pool V2 */
#define SDRATTR_TEXT_HORZADJUST         (SDRATTR_MISC_FIRST +13)   /*   1110 */ /*   1110 */ /*   1093 enum, def=SDRTEXTHORZADJUST_LEFT */ /* Pool V2+ (04-12-1995) */
#define SDRATTR_TEXT_ANIKIND            (SDRATTR_MISC_FIRST +14)   /*   1111 */ /*   1111 */ /*   1094 enum, def=SDRTEXTANI_NONE - Pool V2/V4 */
#define SDRATTR_TEXT_ANIDIRECTION       (SDRATTR_MISC_FIRST +15)   /*   1112 */ /*   1112 */ /*   1095 enum, def=SDRTEXTANI_LEFT - Pool V2/V4 */
#define SDRATTR_TEXT_ANISTARTINSIDE     (SDRATTR_MISC_FIRST +16)   /*   1113 */ /*   1113 */ /*   1096 BOOL, def=FALSE - Pool V2/V4 */
#define SDRATTR_TEXT_ANISTOPINSIDE      (SDRATTR_MISC_FIRST +17)   /*   1114 */ /*   1114 */ /*   1097 BOOL, def=FALSE - Pool V2/V4 */
#define SDRATTR_TEXT_ANICOUNT           (SDRATTR_MISC_FIRST +18)   /*   1115 */ /*   1115 */ /*   1098 USHORT, def=0   - Pool V2/V4 */
#define SDRATTR_TEXT_ANIDELAY           (SDRATTR_MISC_FIRST +19)   /*   1116 */ /*   1116 */ /*   1099 USHORT, def=0   - Pool V2/V4 */
#define SDRATTR_TEXT_ANIAMOUNT          (SDRATTR_MISC_FIRST +20)   /*   1117 */ /*   1117 */ /*   1100 USHORT, def=0   - Pool V2/V4 */
#define SDRATTR_TEXT_CONTOURFRAME       (SDRATTR_MISC_FIRST +21)   /*   1118 */ /*   1118 */ /*   1101 */             /* Pool V2 */
#define SDRATTR_AUTOSHAPE_ADJUSTMENT	(SDRATTR_MISC_FIRST +22)   /*   1119 */ /*   1119 */ /*   1102 */             /* Pool V2 */
#define SDRATTR_XMLATTRIBUTES           (SDRATTR_MISC_FIRST +23)   /*   1120 */ /*   1120 */ /*   1103 */             /* Pool V2 */
#define SDRATTR_RESERVE15               (SDRATTR_MISC_FIRST +24)   /*   1121 */ /*   1121 */ /*   1104 */             /* Pool V2 */
#define SDRATTR_RESERVE16               (SDRATTR_MISC_FIRST +25)   /*   1122 */ /*   1122 */ /*   1105 */             /* Pool V2 */
#define SDRATTR_RESERVE17               (SDRATTR_MISC_FIRST +26)   /*   1123 */ /*   1123 */ /*   1106 */             /* Pool V2 */
#define SDRATTR_RESERVE18               (SDRATTR_MISC_FIRST +27)   /*   1124 */ /*   1124 */ /*   1107 */             /* Pool V2 */
#define SDRATTR_RESERVE19               (SDRATTR_MISC_FIRST +28)   /*   1125 */ /*   1125 */ /*   1108 */             /* Pool V2 */
#define SDRATTR_MISC_LAST               (SDRATTR_RESERVE19)        /* 1125   */ /* 1125   */ /* 1108   */ /* Pool V1: 1056 */
#define SDRATTRSET_MISC                 (SDRATTR_MISC_LAST + 1)    /*   1126 */ /*   1126 */ /*   1109 */ /* Pool V1: 1057 */

#define SDRATTR_EDGE_FIRST              (SDRATTRSET_MISC + 1)      /* 1127   */ /* Pool V4 */
#define SDRATTR_EDGEKIND                (SDRATTR_EDGE_FIRST + 0)   /*   1127 */ /* Pool V4 */
#define SDRATTR_EDGENODE1HORZDIST       (SDRATTR_EDGE_FIRST + 1)   /*   1128 */ /* Pool V4 */
#define SDRATTR_EDGENODE1VERTDIST       (SDRATTR_EDGE_FIRST + 2)   /*   1129 */ /* Pool V4 */
#define SDRATTR_EDGENODE2HORZDIST       (SDRATTR_EDGE_FIRST + 3)   /*   1130 */ /* Pool V4 */
#define SDRATTR_EDGENODE2VERTDIST       (SDRATTR_EDGE_FIRST + 4)   /*   1131 */ /* Pool V4 */
#define SDRATTR_EDGENODE1GLUEDIST       (SDRATTR_EDGE_FIRST + 5)   /*   1132 */ /* Pool V4 */
#define SDRATTR_EDGENODE2GLUEDIST       (SDRATTR_EDGE_FIRST + 6)   /*   1133 */ /* Pool V4 */
#define SDRATTR_EDGELINEDELTAANZ        (SDRATTR_EDGE_FIRST + 7)   /*   1134 */ /* Pool V4 */
#define SDRATTR_EDGELINE1DELTA          (SDRATTR_EDGE_FIRST + 8)   /*   1135 */ /* Pool V4 */
#define SDRATTR_EDGELINE2DELTA          (SDRATTR_EDGE_FIRST + 9)   /*   1136 */ /* Pool V4 */
#define SDRATTR_EDGELINE3DELTA          (SDRATTR_EDGE_FIRST +10)   /*   1137 */ /* Pool V4 */
#define SDRATTR_EDGERESERVE02           (SDRATTR_EDGE_FIRST +11)   /*   1138 */ /* Pool V4 */
#define SDRATTR_EDGERESERVE03           (SDRATTR_EDGE_FIRST +12)   /*   1139 */ /* Pool V4 */
#define SDRATTR_EDGERESERVE04           (SDRATTR_EDGE_FIRST +13)   /*   1140 */ /* Pool V4 */
#define SDRATTR_EDGERESERVE05           (SDRATTR_EDGE_FIRST +14)   /*   1141 */ /* Pool V4 */
#define SDRATTR_EDGERESERVE06           (SDRATTR_EDGE_FIRST +15)   /*   1142 */ /* Pool V4 */
#define SDRATTR_EDGERESERVE07           (SDRATTR_EDGE_FIRST +16)   /*   1143 */ /* Pool V4 */
#define SDRATTR_EDGERESERVE08           (SDRATTR_EDGE_FIRST +17)   /*   1144 */ /* Pool V4 */
#define SDRATTR_EDGERESERVE09           (SDRATTR_EDGE_FIRST +18)   /*   1145 */ /* Pool V4 */
#define SDRATTR_EDGE_LAST               (SDRATTR_EDGERESERVE09)    /* 1145   */ /* Pool V4 */
#define SDRATTRSET_EDGE                 (SDRATTR_EDGE_LAST + 1)    /*   1146 */ /* Pool V4 */

#define SDRATTR_MEASURE_FIRST            (SDRATTRSET_EDGE + 1)      /* 1147   */ /* Pool V4 */
#define SDRATTR_MEASUREKIND              (SDRATTR_MEASURE_FIRST+ 0) /*   1147 */ /* Pool V4 */
#define SDRATTR_MEASURETEXTHPOS          (SDRATTR_MEASURE_FIRST+ 1) /*   1148 */ /* Pool V4 */
#define SDRATTR_MEASURETEXTVPOS          (SDRATTR_MEASURE_FIRST+ 2) /*   1149 */ /* Pool V4 */
#define SDRATTR_MEASURELINEDIST          (SDRATTR_MEASURE_FIRST+ 3) /*   1150 */ /* Pool V4 */
#define SDRATTR_MEASUREHELPLINEOVERHANG  (SDRATTR_MEASURE_FIRST+ 4) /*   1151 */ /* Pool V4 */
#define SDRATTR_MEASUREHELPLINEDIST      (SDRATTR_MEASURE_FIRST+ 5) /*   1152 */ /* Pool V4 */
#define SDRATTR_MEASUREHELPLINE1LEN      (SDRATTR_MEASURE_FIRST+ 6) /*   1153 */ /* Pool V4 */
#define SDRATTR_MEASUREHELPLINE2LEN      (SDRATTR_MEASURE_FIRST+ 7) /*   1154 */ /* Pool V4 */
#define SDRATTR_MEASUREBELOWREFEDGE      (SDRATTR_MEASURE_FIRST+ 8) /*   1155 */ /* Pool V4 */
#define SDRATTR_MEASURETEXTROTA90        (SDRATTR_MEASURE_FIRST+ 9) /*   1156 */ /* Pool V4 */
#define SDRATTR_MEASURETEXTUPSIDEDOWN    (SDRATTR_MEASURE_FIRST+10) /*   1157 */ /* Pool V4 */
#define SDRATTR_MEASUREOVERHANG          (SDRATTR_MEASURE_FIRST+11) /*   1158 */ /* Pool V4 */
#define SDRATTR_MEASUREUNIT              (SDRATTR_MEASURE_FIRST+12) /*   1159 */ /* Pool V4 */
#define SDRATTR_MEASURESCALE             (SDRATTR_MEASURE_FIRST+13) /*   1160 */ /* Pool V4 */
#define SDRATTR_MEASURESHOWUNIT          (SDRATTR_MEASURE_FIRST+14) /*   1161 */ /* Pool V4 */
#define SDRATTR_MEASUREFORMATSTRING      (SDRATTR_MEASURE_FIRST+15) /*   1162 */ /* Pool V4 */
#define SDRATTR_MEASURETEXTAUTOANGLE     (SDRATTR_MEASURE_FIRST+16) /*   1163 */ /* Pool V4, Nachtrag 25-9-1996 */
#define SDRATTR_MEASURETEXTAUTOANGLEVIEW (SDRATTR_MEASURE_FIRST+17) /*   1164 */ /* Pool V4, Nachtrag 25-9-1996  */
#define SDRATTR_MEASURETEXTISFIXEDANGLE  (SDRATTR_MEASURE_FIRST+18) /*   1165 */ /* Pool V4, Nachtrag 25-9-1996  */
#define SDRATTR_MEASURETEXTFIXEDANGLE    (SDRATTR_MEASURE_FIRST+19) /*   1166 */ /* Pool V4, Nachtrag 25-9-1996  */
#define SDRATTR_MEASUREDECIMALPLACES     (SDRATTR_MEASURE_FIRST+20) /*   1167 */ /* Pool V4 */
#define SDRATTR_MEASURERESERVE05         (SDRATTR_MEASURE_FIRST+21) /*   1168 */ /* Pool V4 */
#define SDRATTR_MEASURERESERVE06         (SDRATTR_MEASURE_FIRST+22) /*   1169 */ /* Pool V4 */
#define SDRATTR_MEASURERESERVE07         (SDRATTR_MEASURE_FIRST+23) /*   1170 */ /* Pool V4 */
#define SDRATTR_MEASURE_LAST             (SDRATTR_MEASURERESERVE07) /* 1170   */ /* Pool V4 */
#define SDRATTRSET_MEASURE               (SDRATTR_MEASURE_LAST + 1) /*   1171 */ /* Pool V4 */

#define SDRATTR_CIRC_FIRST              (SDRATTRSET_MEASURE+ 1)    /* 1172   */ /* Pool V4+ */
#define SDRATTR_CIRCKIND                (SDRATTR_CIRC_FIRST+ 0)    /*   1172 */ /* Pool V4+ */
#define SDRATTR_CIRCSTARTANGLE          (SDRATTR_CIRC_FIRST+ 1)    /*   1173 */ /* Pool V4+ */
#define SDRATTR_CIRCENDANGLE            (SDRATTR_CIRC_FIRST+ 2)    /*   1174 */ /* Pool V4+ */
#define SDRATTR_CIRCRESERVE0            (SDRATTR_CIRC_FIRST+ 3)    /*   1175 */ /* Pool V4+ */
#define SDRATTR_CIRCRESERVE1            (SDRATTR_CIRC_FIRST+ 4)    /*   1176 */ /* Pool V4+ */
#define SDRATTR_CIRCRESERVE2            (SDRATTR_CIRC_FIRST+ 5)    /*   1177 */ /* Pool V4+ */
#define SDRATTR_CIRCRESERVE3            (SDRATTR_CIRC_FIRST+ 6)    /*   1178 */ /* Pool V4+ */
#define SDRATTR_CIRC_LAST               (SDRATTR_CIRCRESERVE3)     /*	1178 */ /* Pool V4+ */
#define SDRATTRSET_CIRC                 (SDRATTR_CIRC_LAST + 1)    /*   1179 */ /* Pool V4+ */

// Nur fuer Interfaceing, not implemented yet.                     /*  V4+   */
#define SDRATTR_NOTPERSIST_FIRST    (SDRATTRSET_CIRC + 1)          /* 1180   */ /* 1172   */ /* 1127   */ /* 1110   */ /* Pool V1: 1058 */
#define SDRATTR_OBJMOVEPROTECT      (SDRATTR_NOTPERSIST_FIRST+ 0)  /*   1180  BOOL,   def=FALSE */
#define SDRATTR_OBJSIZEPROTECT      (SDRATTR_NOTPERSIST_FIRST+ 1)  /*   1181  BOOL,   def=FALSE */
#define SDRATTR_OBJPRINTABLE        (SDRATTR_NOTPERSIST_FIRST+ 2)  /*   1182  BOOL,   def=TRUE  */
#define SDRATTR_LAYERID             (SDRATTR_NOTPERSIST_FIRST+ 3)  /*   1183  USHORT, def=0     */
#define SDRATTR_LAYERNAME           (SDRATTR_NOTPERSIST_FIRST+ 4)  /*   1184  String, def=""    */
#define SDRATTR_OBJECTNAME          (SDRATTR_NOTPERSIST_FIRST+ 5)  /*   1185  String, def=""    */
#define SDRATTR_ALLPOSITIONX        (SDRATTR_NOTPERSIST_FIRST+ 6)  /*   1186  long,   def=0     */
#define SDRATTR_ALLPOSITIONY        (SDRATTR_NOTPERSIST_FIRST+ 7)  /*   1187  long,   def=0     */
#define SDRATTR_ALLSIZEWIDTH        (SDRATTR_NOTPERSIST_FIRST+ 8)  /*   1188  long,   def=0     */
#define SDRATTR_ALLSIZEHEIGHT       (SDRATTR_NOTPERSIST_FIRST+ 9)  /*   1189  long,   def=0     */
#define SDRATTR_ONEPOSITIONX        (SDRATTR_NOTPERSIST_FIRST+10)  /*   1190  long,   def=0     */
#define SDRATTR_ONEPOSITIONY        (SDRATTR_NOTPERSIST_FIRST+11)  /*   1191  long,   def=0     */
#define SDRATTR_ONESIZEWIDTH        (SDRATTR_NOTPERSIST_FIRST+12)  /*   1192  long,   def=0     */
#define SDRATTR_ONESIZEHEIGHT       (SDRATTR_NOTPERSIST_FIRST+13)  /*   1193  long,   def=0     */
#define SDRATTR_LOGICSIZEWIDTH      (SDRATTR_NOTPERSIST_FIRST+14)  /*   1194  long,   def=0     */
#define SDRATTR_LOGICSIZEHEIGHT     (SDRATTR_NOTPERSIST_FIRST+15)  /*   1195  long,   def=0     */
#define SDRATTR_ROTATEANGLE         (SDRATTR_NOTPERSIST_FIRST+16)  /*   1196  long,   def=0     */
#define SDRATTR_SHEARANGLE          (SDRATTR_NOTPERSIST_FIRST+17)  /*   1197  long,   def=0     */
#define SDRATTR_MOVEX               (SDRATTR_NOTPERSIST_FIRST+18)  /*   1198  long,   def=0     */
#define SDRATTR_MOVEY               (SDRATTR_NOTPERSIST_FIRST+19)  /*   1199  long,   def=0     */
#define SDRATTR_RESIZEXONE          (SDRATTR_NOTPERSIST_FIRST+20)  /*   1200  long,   def=0     */
#define SDRATTR_RESIZEYONE          (SDRATTR_NOTPERSIST_FIRST+21)  /*   1201  long,   def=0     */
#define SDRATTR_ROTATEONE           (SDRATTR_NOTPERSIST_FIRST+22)  /*   1202  long,   def=0     */
#define SDRATTR_HORZSHEARONE        (SDRATTR_NOTPERSIST_FIRST+23)  /*   1203  long,   def=0     */
#define SDRATTR_VERTSHEARONE        (SDRATTR_NOTPERSIST_FIRST+24)  /*   1204  long,   def=0     */
#define SDRATTR_RESIZEXALL          (SDRATTR_NOTPERSIST_FIRST+25)  /*   1205  long,   def=0     */
#define SDRATTR_RESIZEYALL          (SDRATTR_NOTPERSIST_FIRST+26)  /*   1206  long,   def=0     */
#define SDRATTR_ROTATEALL           (SDRATTR_NOTPERSIST_FIRST+27)  /*   1207  long,   def=0     */
#define SDRATTR_HORZSHEARALL        (SDRATTR_NOTPERSIST_FIRST+28)  /*   1208  long,   def=0     */
#define SDRATTR_VERTSHEARALL        (SDRATTR_NOTPERSIST_FIRST+29)  /*   1209  long,   def=0     */
#define SDRATTR_TRANSFORMREF1X      (SDRATTR_NOTPERSIST_FIRST+30)  /*   1210  long,   def=0     */
#define SDRATTR_TRANSFORMREF1Y      (SDRATTR_NOTPERSIST_FIRST+31)  /*   1211  long,   def=0     */
#define SDRATTR_TRANSFORMREF2X      (SDRATTR_NOTPERSIST_FIRST+32)  /*   1212  long,   def=0     */
#define SDRATTR_TRANSFORMREF2Y      (SDRATTR_NOTPERSIST_FIRST+33)  /*   1213  long,   def=0     */
#define SDRATTR_TEXTDIRECTION		(SDRATTR_NOTPERSIST_FIRST+34)  /*   1214 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE2	(SDRATTR_NOTPERSIST_FIRST+35)  /*   1215 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE3	(SDRATTR_NOTPERSIST_FIRST+36)  /*   1216 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE4	(SDRATTR_NOTPERSIST_FIRST+37)  /*   1217 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE5	(SDRATTR_NOTPERSIST_FIRST+38)  /*   1218 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE6	(SDRATTR_NOTPERSIST_FIRST+39)  /*   1219 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE7	(SDRATTR_NOTPERSIST_FIRST+40)  /*   1220 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE8	(SDRATTR_NOTPERSIST_FIRST+41)  /*   1221 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE9	(SDRATTR_NOTPERSIST_FIRST+42)  /*   1222 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE10	(SDRATTR_NOTPERSIST_FIRST+43)  /*   1223 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE11	(SDRATTR_NOTPERSIST_FIRST+44)  /*   1224 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE12	(SDRATTR_NOTPERSIST_FIRST+45)  /*   1225 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE13	(SDRATTR_NOTPERSIST_FIRST+46)  /*   1226 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE14	(SDRATTR_NOTPERSIST_FIRST+47)  /*   1227 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSISTRESERVE15	(SDRATTR_NOTPERSIST_FIRST+48)  /*   1228 V4+++  long,   def=0     */
#define SDRATTR_NOTPERSIST_LAST     (SDRATTR_NOTPERSISTRESERVE15)  /*	1228 V4+++*/ /*	1213*/	/*	1085 */ /*	1040 */ /*	1123 */ /* Pool V1: 1065 */
// kein SetItem hierfuer

#define SDRATTR_GRAF_FIRST			(SDRATTR_NOTPERSIST_LAST+1)		/*	1229 V4+++*/
#define SDRATTR_GRAFRED             (SDRATTR_GRAF_FIRST+0)			/*	1229 V4+++*/
#define SDRATTR_GRAFGREEN			(SDRATTR_GRAF_FIRST+1)			/*	1230 V4+++*/
#define SDRATTR_GRAFBLUE			(SDRATTR_GRAF_FIRST+2)			/*	1231 V4+++*/
#define SDRATTR_GRAFLUMINANCE       (SDRATTR_GRAF_FIRST+3)			/*	1232 V4+++*/
#define SDRATTR_GRAFCONTRAST		(SDRATTR_GRAF_FIRST+4)			/*	1233 V4+++*/
#define SDRATTR_GRAFGAMMA			(SDRATTR_GRAF_FIRST+5)			/*	1234 V4+++*/
#define SDRATTR_GRAFTRANSPARENCE	(SDRATTR_GRAF_FIRST+6)			/*	1235 V4+++*/
#define SDRATTR_GRAFINVERT			(SDRATTR_GRAF_FIRST+7)			/*	1236 V4+++*/
#define SDRATTR_GRAFMODE			(SDRATTR_GRAF_FIRST+8)			/*	1237 V4+++*/
#define SDRATTR_GRAFCROP			(SDRATTR_GRAF_FIRST+9)			/*	1238 V4+++*/
#define SDRATTR_GRAFRESERVE3		(SDRATTR_GRAF_FIRST+10)			/*	1239 V4+++*/
#define SDRATTR_GRAFRESERVE4		(SDRATTR_GRAF_FIRST+11)			/*	1240 V4+++*/
#define SDRATTR_GRAFRESERVE5		(SDRATTR_GRAF_FIRST+12)			/*	1241 V4+++*/
#define SDRATTR_GRAFRESERVE6		(SDRATTR_GRAF_FIRST+13)			/*	1242 V4+++*/
#define SDRATTR_GRAF_LAST           (SDRATTR_GRAFRESERVE6)			/*	1242 V4+++*/
#define SDRATTRSET_GRAF             (SDRATTR_GRAF_LAST+1)			/*	1243 V4+++*/

#define	SDRATTR_3D_FIRST						(SDRATTRSET_GRAF + 1)		/* 1244 V4+++*/

#define	SDRATTR_3DOBJ_FIRST						(SDRATTR_3D_FIRST)			/* 1244 V4+++*/
#define	SDRATTR_3DOBJ_PERCENT_DIAGONAL			(SDRATTR_3DOBJ_FIRST + 0)	/* 1244 V4+++*/
#define	SDRATTR_3DOBJ_BACKSCALE					(SDRATTR_3DOBJ_FIRST + 1)	/* 1245 V4+++*/
#define	SDRATTR_3DOBJ_DEPTH						(SDRATTR_3DOBJ_FIRST + 2)	/* 1246 V4+++*/
#define	SDRATTR_3DOBJ_HORZ_SEGS					(SDRATTR_3DOBJ_FIRST + 3)	/* 1247 V4+++*/
#define	SDRATTR_3DOBJ_VERT_SEGS					(SDRATTR_3DOBJ_FIRST + 4)	/* 1248 V4+++*/
#define	SDRATTR_3DOBJ_END_ANGLE					(SDRATTR_3DOBJ_FIRST + 5)	/* 1249 V4+++*/
#define	SDRATTR_3DOBJ_DOUBLE_SIDED				(SDRATTR_3DOBJ_FIRST + 6)	/* 1250 V4+++*/
#define	SDRATTR_3DOBJ_NORMALS_KIND				(SDRATTR_3DOBJ_FIRST + 7)	/* 1251 V4+++*/
#define	SDRATTR_3DOBJ_NORMALS_INVERT			(SDRATTR_3DOBJ_FIRST + 8)	/* 1252 V4+++*/
#define	SDRATTR_3DOBJ_TEXTURE_PROJ_X			(SDRATTR_3DOBJ_FIRST + 9)	/* 1253 V4+++*/
#define	SDRATTR_3DOBJ_TEXTURE_PROJ_Y			(SDRATTR_3DOBJ_FIRST + 10)	/* 1254 V4+++*/
#define	SDRATTR_3DOBJ_SHADOW_3D					(SDRATTR_3DOBJ_FIRST + 11)	/* 1255 V4+++*/
#define	SDRATTR_3DOBJ_MAT_COLOR					(SDRATTR_3DOBJ_FIRST + 12)	/* 1256 V4+++*/
#define	SDRATTR_3DOBJ_MAT_EMISSION				(SDRATTR_3DOBJ_FIRST + 13)	/* 1257 V4+++*/
#define	SDRATTR_3DOBJ_MAT_SPECULAR				(SDRATTR_3DOBJ_FIRST + 14)	/* 1258 V4+++*/
#define	SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY	(SDRATTR_3DOBJ_FIRST + 15)	/* 1259 V4+++*/
#define	SDRATTR_3DOBJ_TEXTURE_KIND				(SDRATTR_3DOBJ_FIRST + 16)	/* 1260 V4+++*/
#define	SDRATTR_3DOBJ_TEXTURE_MODE				(SDRATTR_3DOBJ_FIRST + 17)	/* 1261 V4+++*/
#define	SDRATTR_3DOBJ_TEXTURE_FILTER			(SDRATTR_3DOBJ_FIRST + 18)	/* 1262 V4+++*/

// #107245# New items for 3d objects use former range SDRATTR_3DOBJ_RESERVED_01
// up to SDRATTR_3DOBJ_RESERVED_05 
#define	SDRATTR_3DOBJ_SMOOTH_NORMALS			(SDRATTR_3DOBJ_FIRST + 19)	/* 1263 V4+++*/
#define	SDRATTR_3DOBJ_SMOOTH_LIDS				(SDRATTR_3DOBJ_FIRST + 20)	/* 1264 V4+++*/
#define	SDRATTR_3DOBJ_CHARACTER_MODE			(SDRATTR_3DOBJ_FIRST + 21)	/* 1265 V4+++*/
#define	SDRATTR_3DOBJ_CLOSE_FRONT				(SDRATTR_3DOBJ_FIRST + 22)	/* 1266 V4+++*/
#define	SDRATTR_3DOBJ_CLOSE_BACK				(SDRATTR_3DOBJ_FIRST + 23)	/* 1267 V4+++*/

#define	SDRATTR_3DOBJ_RESERVED_06				(SDRATTR_3DOBJ_FIRST + 24)	/* 1268 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_07				(SDRATTR_3DOBJ_FIRST + 25)	/* 1269 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_08				(SDRATTR_3DOBJ_FIRST + 26)	/* 1270 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_09				(SDRATTR_3DOBJ_FIRST + 27)	/* 1271 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_10				(SDRATTR_3DOBJ_FIRST + 28)	/* 1272 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_11				(SDRATTR_3DOBJ_FIRST + 29)	/* 1273 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_12				(SDRATTR_3DOBJ_FIRST + 30)	/* 1274 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_13				(SDRATTR_3DOBJ_FIRST + 31)	/* 1275 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_14				(SDRATTR_3DOBJ_FIRST + 32)	/* 1276 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_15				(SDRATTR_3DOBJ_FIRST + 33)	/* 1277 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_16				(SDRATTR_3DOBJ_FIRST + 34)	/* 1278 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_17				(SDRATTR_3DOBJ_FIRST + 35)	/* 1279 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_18				(SDRATTR_3DOBJ_FIRST + 36)	/* 1280 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_19				(SDRATTR_3DOBJ_FIRST + 37)	/* 1281 V4+++*/
#define	SDRATTR_3DOBJ_RESERVED_20				(SDRATTR_3DOBJ_FIRST + 38)	/* 1282 V4+++*/
#define	SDRATTR_3DOBJ_LAST						(SDRATTR_3DOBJ_RESERVED_20)	/* 1282 V4+++*/

#define	SDRATTR_3DSCENE_FIRST					(SDRATTR_3DOBJ_LAST + 1)		/* 1283 V4+++*/
#define	SDRATTR_3DSCENE_PERSPECTIVE				(SDRATTR_3DSCENE_FIRST + 0)		/* 1283 V4+++*/
#define	SDRATTR_3DSCENE_DISTANCE				(SDRATTR_3DSCENE_FIRST + 1)		/* 1284 V4+++*/
#define	SDRATTR_3DSCENE_FOCAL_LENGTH			(SDRATTR_3DSCENE_FIRST + 2)		/* 1285 V4+++*/
#define	SDRATTR_3DSCENE_TWO_SIDED_LIGHTING		(SDRATTR_3DSCENE_FIRST + 3)		/* 1286 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTCOLOR_1			(SDRATTR_3DSCENE_FIRST + 4)		/* 1287 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTCOLOR_2			(SDRATTR_3DSCENE_FIRST + 5)		/* 1288 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTCOLOR_3			(SDRATTR_3DSCENE_FIRST + 6)		/* 1289 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTCOLOR_4			(SDRATTR_3DSCENE_FIRST + 7)		/* 1290 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTCOLOR_5			(SDRATTR_3DSCENE_FIRST + 8)		/* 1291 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTCOLOR_6			(SDRATTR_3DSCENE_FIRST + 9)		/* 1292 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTCOLOR_7			(SDRATTR_3DSCENE_FIRST + 10)	/* 1293 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTCOLOR_8			(SDRATTR_3DSCENE_FIRST + 11)	/* 1294 V4+++*/
#define	SDRATTR_3DSCENE_AMBIENTCOLOR			(SDRATTR_3DSCENE_FIRST + 12)	/* 1295 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTON_1				(SDRATTR_3DSCENE_FIRST + 13)	/* 1296 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTON_2				(SDRATTR_3DSCENE_FIRST + 14)	/* 1297 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTON_3				(SDRATTR_3DSCENE_FIRST + 15)	/* 1298 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTON_4				(SDRATTR_3DSCENE_FIRST + 16)	/* 1299 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTON_5				(SDRATTR_3DSCENE_FIRST + 17)	/* 1300 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTON_6				(SDRATTR_3DSCENE_FIRST + 18)	/* 1301 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTON_7				(SDRATTR_3DSCENE_FIRST + 19)	/* 1302 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTON_8				(SDRATTR_3DSCENE_FIRST + 20)	/* 1303 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTDIRECTION_1		(SDRATTR_3DSCENE_FIRST + 21)	/* 1304 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTDIRECTION_2		(SDRATTR_3DSCENE_FIRST + 22)	/* 1305 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTDIRECTION_3		(SDRATTR_3DSCENE_FIRST + 23)	/* 1306 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTDIRECTION_4		(SDRATTR_3DSCENE_FIRST + 24)	/* 1307 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTDIRECTION_5		(SDRATTR_3DSCENE_FIRST + 25)	/* 1308 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTDIRECTION_6		(SDRATTR_3DSCENE_FIRST + 26)	/* 1309 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTDIRECTION_7		(SDRATTR_3DSCENE_FIRST + 27)	/* 1310 V4+++*/
#define	SDRATTR_3DSCENE_LIGHTDIRECTION_8		(SDRATTR_3DSCENE_FIRST + 28)	/* 1311 V4+++*/
#define	SDRATTR_3DSCENE_SHADOW_SLANT			(SDRATTR_3DSCENE_FIRST + 29)	/* 1312 V4+++*/
#define	SDRATTR_3DSCENE_SHADE_MODE				(SDRATTR_3DSCENE_FIRST + 30)	/* 1313 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_01				(SDRATTR_3DSCENE_FIRST + 31)	/* 1314 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_02				(SDRATTR_3DSCENE_FIRST + 32)	/* 1315 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_03				(SDRATTR_3DSCENE_FIRST + 33)	/* 1316 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_04				(SDRATTR_3DSCENE_FIRST + 34)	/* 1317 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_05				(SDRATTR_3DSCENE_FIRST + 35)	/* 1318 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_06				(SDRATTR_3DSCENE_FIRST + 36)	/* 1319 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_07				(SDRATTR_3DSCENE_FIRST + 37)	/* 1320 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_08				(SDRATTR_3DSCENE_FIRST + 38)	/* 1321 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_09				(SDRATTR_3DSCENE_FIRST + 39)	/* 1322 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_10				(SDRATTR_3DSCENE_FIRST + 40)	/* 1323 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_11				(SDRATTR_3DSCENE_FIRST + 41)	/* 1324 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_12				(SDRATTR_3DSCENE_FIRST + 42)	/* 1325 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_13				(SDRATTR_3DSCENE_FIRST + 43)	/* 1326 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_14				(SDRATTR_3DSCENE_FIRST + 44)	/* 1327 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_15				(SDRATTR_3DSCENE_FIRST + 45)	/* 1328 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_16				(SDRATTR_3DSCENE_FIRST + 46)	/* 1329 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_17				(SDRATTR_3DSCENE_FIRST + 47)	/* 1330 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_18				(SDRATTR_3DSCENE_FIRST + 48)	/* 1331 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_19				(SDRATTR_3DSCENE_FIRST + 49)	/* 1332 V4+++*/
#define	SDRATTR_3DSCENE_RESERVED_20				(SDRATTR_3DSCENE_FIRST + 50)	/* 1333 V4+++*/
#define	SDRATTR_3DSCENE_LAST					(SDRATTR_3DSCENE_RESERVED_20)	/* 1333 V4+++*/

#define	SDRATTR_3D_LAST							(SDRATTR_3DSCENE_LAST)	/* 1333 V4+++*/

#define SDRATTR_END								SDRATTR_3D_LAST		/* 1333 V4+++*/ /* 1243 V4+++*/	/*1213*/ /*1085*/ /*1040*/ /*Pool V2: 1123,V1: 1065 */

}//end of namespace binfilter
#endif // _SVDDEF_HXX

