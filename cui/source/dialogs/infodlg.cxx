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

//#include <sal/log.hxx>

#include <infodlg.hxx>

#include <config_folders.h>
#include <dialmgr.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <strings.hrc>
#include <vcl/graphicfilter.hxx>
#include <vcl/virdev.hxx>

InfoDialog::InfoDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/infodialog.ui", "InfoDialog")
    , m_pImage(m_xBuilder->weld_image("imImage"))
    , m_pText(m_xBuilder->weld_label("lbText"))
    , m_pShowTip(m_xBuilder->weld_check_button("cbShowTip"))
    , m_pNext(m_xBuilder->weld_button("btnNext"))
    , m_pLink(m_xBuilder->weld_link_button("btnLink"))
{
    m_pShowTip->connect_toggled(LINK(this, InfoDialog, OnShowTipToggled));
    m_pNext->connect_clicked(LINK(this, InfoDialog, OnNextClick));

    nNumberOfTips = RID_SVX_MAX_TIPOFTHEDAY;
    nCurrentTip = officecfg::Office::Common::Misc::CurrentTipOfTheDay::get() + 1;
    if (nCurrentTip >= nNumberOfTips)
        nCurrentTip = 0;
    UpdateTip();
}

InfoDialog::~InfoDialog()
{
    // save current tip
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::CurrentTipOfTheDay::set(nCurrentTip, xChanges);
    xChanges->commit();
}

inline bool file_exists(const OUString& fileName)
{
    if (FILE* f = fopen(OUStringToOString(fileName, RTL_TEXTENCODING_ASCII_US).getStr(), "r"))
    {
        fclose(f);
        return true;
    }
    else
    {
        return false;
    }
}

inline const char* getResID(const int nId)
{
    /* char pKey[] = "RID_SVXSTR_TIPOFTHEDAY_";
    strcat( pKey, std::to_string(nCurrentTip).c_str() );
    aText = Translate::get(reinterpret_cast<const char*>(pKey), Translate::Create("cui"));
*/
    switch (nId)
    {
        case 0:
            return RID_SVXSTR_TIPOFTHEDAY_0;
        case 1:
            return RID_SVXSTR_TIPOFTHEDAY_1;
        case 2:
            return RID_SVXSTR_TIPOFTHEDAY_2;
        case 3:
            return RID_SVXSTR_TIPOFTHEDAY_3;
        case 4:
            return RID_SVXSTR_TIPOFTHEDAY_4;
        case 5:
            return RID_SVXSTR_TIPOFTHEDAY_5;
        case 6:
            return RID_SVXSTR_TIPOFTHEDAY_6;
        case 7:
            return RID_SVXSTR_TIPOFTHEDAY_7;
        case 8:
            return RID_SVXSTR_TIPOFTHEDAY_8;
        case 9:
            return RID_SVXSTR_TIPOFTHEDAY_9;
        case 10:
            return RID_SVXSTR_TIPOFTHEDAY_10;
        case 11:
            return RID_SVXSTR_TIPOFTHEDAY_11;
        case 12:
            return RID_SVXSTR_TIPOFTHEDAY_12;
        case 13:
            return RID_SVXSTR_TIPOFTHEDAY_13;
        case 14:
            return RID_SVXSTR_TIPOFTHEDAY_14;
        case 15:
            return RID_SVXSTR_TIPOFTHEDAY_15;
        case 16:
            return RID_SVXSTR_TIPOFTHEDAY_16;
        case 17:
            return RID_SVXSTR_TIPOFTHEDAY_17;
        case 18:
            return RID_SVXSTR_TIPOFTHEDAY_18;
        case 19:
            return RID_SVXSTR_TIPOFTHEDAY_19;
        case 20:
            return RID_SVXSTR_TIPOFTHEDAY_20;
        case 21:
            return RID_SVXSTR_TIPOFTHEDAY_21;
        case 22:
            return RID_SVXSTR_TIPOFTHEDAY_22;
        case 23:
            return RID_SVXSTR_TIPOFTHEDAY_23;
        case 24:
            return RID_SVXSTR_TIPOFTHEDAY_24;
        case 25:
            return RID_SVXSTR_TIPOFTHEDAY_25;
        case 26:
            return RID_SVXSTR_TIPOFTHEDAY_26;
        case 27:
            return RID_SVXSTR_TIPOFTHEDAY_27;
        case 28:
            return RID_SVXSTR_TIPOFTHEDAY_28;
        case 29:
            return RID_SVXSTR_TIPOFTHEDAY_29;
        case 30:
            return RID_SVXSTR_TIPOFTHEDAY_30;
        case 31:
            return RID_SVXSTR_TIPOFTHEDAY_31;
        case 32:
            return RID_SVXSTR_TIPOFTHEDAY_32;
        case 33:
            return RID_SVXSTR_TIPOFTHEDAY_33;
        case 34:
            return RID_SVXSTR_TIPOFTHEDAY_34;
        case 35:
            return RID_SVXSTR_TIPOFTHEDAY_35;
        case 36:
            return RID_SVXSTR_TIPOFTHEDAY_36;
        case 37:
            return RID_SVXSTR_TIPOFTHEDAY_37;
        case 38:
            return RID_SVXSTR_TIPOFTHEDAY_38;
        case 39:
            return RID_SVXSTR_TIPOFTHEDAY_39;
        case 40:
            return RID_SVXSTR_TIPOFTHEDAY_40;
        case 41:
            return RID_SVXSTR_TIPOFTHEDAY_41;
        case 42:
            return RID_SVXSTR_TIPOFTHEDAY_42;
        case 43:
            return RID_SVXSTR_TIPOFTHEDAY_43;
        case 44:
            return RID_SVXSTR_TIPOFTHEDAY_44;
        case 45:
            return RID_SVXSTR_TIPOFTHEDAY_45;
        case 46:
            return RID_SVXSTR_TIPOFTHEDAY_46;
        case 47:
            return RID_SVXSTR_TIPOFTHEDAY_47;
        case 48:
            return RID_SVXSTR_TIPOFTHEDAY_48;
        case 49:
            return RID_SVXSTR_TIPOFTHEDAY_49;
        case 50:
            return RID_SVXSTR_TIPOFTHEDAY_50;
        case 51:
            return RID_SVXSTR_TIPOFTHEDAY_51;
        case 52:
            return RID_SVXSTR_TIPOFTHEDAY_52;
        case 53:
            return RID_SVXSTR_TIPOFTHEDAY_53;
        case 54:
            return RID_SVXSTR_TIPOFTHEDAY_54;
        case 55:
            return RID_SVXSTR_TIPOFTHEDAY_55;
        case 56:
            return RID_SVXSTR_TIPOFTHEDAY_56;
        case 57:
            return RID_SVXSTR_TIPOFTHEDAY_57;
        case 58:
            return RID_SVXSTR_TIPOFTHEDAY_58;
        case 59:
            return RID_SVXSTR_TIPOFTHEDAY_59;
        case 60:
            return RID_SVXSTR_TIPOFTHEDAY_60;
        case 61:
            return RID_SVXSTR_TIPOFTHEDAY_61;
        case 62:
            return RID_SVXSTR_TIPOFTHEDAY_62;
        case 63:
            return RID_SVXSTR_TIPOFTHEDAY_63;
        case 64:
            return RID_SVXSTR_TIPOFTHEDAY_64;
        case 65:
            return RID_SVXSTR_TIPOFTHEDAY_65;
        case 66:
            return RID_SVXSTR_TIPOFTHEDAY_66;
        case 67:
            return RID_SVXSTR_TIPOFTHEDAY_67;
        case 68:
            return RID_SVXSTR_TIPOFTHEDAY_68;
        case 69:
            return RID_SVXSTR_TIPOFTHEDAY_69;
        case 70:
            return RID_SVXSTR_TIPOFTHEDAY_70;
        case 71:
            return RID_SVXSTR_TIPOFTHEDAY_71;
        case 72:
            return RID_SVXSTR_TIPOFTHEDAY_72;
        case 73:
            return RID_SVXSTR_TIPOFTHEDAY_73;
        case 74:
            return RID_SVXSTR_TIPOFTHEDAY_74;
        case 75:
            return RID_SVXSTR_TIPOFTHEDAY_75;
        case 76:
            return RID_SVXSTR_TIPOFTHEDAY_76;
        case 77:
            return RID_SVXSTR_TIPOFTHEDAY_77;
        case 78:
            return RID_SVXSTR_TIPOFTHEDAY_78;
        case 79:
            return RID_SVXSTR_TIPOFTHEDAY_79;
        case 80:
            return RID_SVXSTR_TIPOFTHEDAY_80;
        case 81:
            return RID_SVXSTR_TIPOFTHEDAY_81;
        case 82:
            return RID_SVXSTR_TIPOFTHEDAY_82;
        case 83:
            return RID_SVXSTR_TIPOFTHEDAY_83;
        case 84:
            return RID_SVXSTR_TIPOFTHEDAY_84;
        case 85:
            return RID_SVXSTR_TIPOFTHEDAY_85;
        case 86:
            return RID_SVXSTR_TIPOFTHEDAY_86;
        case 87:
            return RID_SVXSTR_TIPOFTHEDAY_87;
        case 88:
            return RID_SVXSTR_TIPOFTHEDAY_88;
        case 89:
            return RID_SVXSTR_TIPOFTHEDAY_89;
        case 90:
            return RID_SVXSTR_TIPOFTHEDAY_90;
        case 91:
            return RID_SVXSTR_TIPOFTHEDAY_91;
        case 92:
            return RID_SVXSTR_TIPOFTHEDAY_92;
        case 93:
            return RID_SVXSTR_TIPOFTHEDAY_93;
        case 94:
            return RID_SVXSTR_TIPOFTHEDAY_94;
        case 95:
            return RID_SVXSTR_TIPOFTHEDAY_95;
        case 96:
            return RID_SVXSTR_TIPOFTHEDAY_96;
        case 97:
            return RID_SVXSTR_TIPOFTHEDAY_97;
        case 98:
            return RID_SVXSTR_TIPOFTHEDAY_98;
        case 99:
            return RID_SVXSTR_TIPOFTHEDAY_99;
        case 100:
            return RID_SVXSTR_TIPOFTHEDAY_100;
        case 101:
            return RID_SVXSTR_TIPOFTHEDAY_101;
        case 102:
            return RID_SVXSTR_TIPOFTHEDAY_102;
        case 103:
            return RID_SVXSTR_TIPOFTHEDAY_103;
        case 104:
            return RID_SVXSTR_TIPOFTHEDAY_104;
        case 105:
            return RID_SVXSTR_TIPOFTHEDAY_105;
        case 106:
            return RID_SVXSTR_TIPOFTHEDAY_106;
        case 107:
            return RID_SVXSTR_TIPOFTHEDAY_107;
        case 108:
            return RID_SVXSTR_TIPOFTHEDAY_108;
        case 109:
            return RID_SVXSTR_TIPOFTHEDAY_109;
        case 110:
            return RID_SVXSTR_TIPOFTHEDAY_110;
        case 111:
            return RID_SVXSTR_TIPOFTHEDAY_111;
        case 112:
            return RID_SVXSTR_TIPOFTHEDAY_112;
        case 113:
            return RID_SVXSTR_TIPOFTHEDAY_113;
        case 114:
            return RID_SVXSTR_TIPOFTHEDAY_114;
        case 115:
            return RID_SVXSTR_TIPOFTHEDAY_115;
        case 116:
            return RID_SVXSTR_TIPOFTHEDAY_116;
        case 117:
            return RID_SVXSTR_TIPOFTHEDAY_117;
        case 118:
            return RID_SVXSTR_TIPOFTHEDAY_118;
        case 119:
            return RID_SVXSTR_TIPOFTHEDAY_119;
        case 120:
            return RID_SVXSTR_TIPOFTHEDAY_120;
        case 121:
            return RID_SVXSTR_TIPOFTHEDAY_121;
        case 122:
            return RID_SVXSTR_TIPOFTHEDAY_122;
        case 123:
            return RID_SVXSTR_TIPOFTHEDAY_123;
        case 124:
            return RID_SVXSTR_TIPOFTHEDAY_124;
        case 125:
            return RID_SVXSTR_TIPOFTHEDAY_125;
        case 126:
            return RID_SVXSTR_TIPOFTHEDAY_126;
        case 127:
            return RID_SVXSTR_TIPOFTHEDAY_127;
        case 128:
            return RID_SVXSTR_TIPOFTHEDAY_128;
        case 129:
            return RID_SVXSTR_TIPOFTHEDAY_129;
        case 130:
            return RID_SVXSTR_TIPOFTHEDAY_130;
        case 131:
            return RID_SVXSTR_TIPOFTHEDAY_131;
        case 132:
            return RID_SVXSTR_TIPOFTHEDAY_132;
        case 133:
            return RID_SVXSTR_TIPOFTHEDAY_133;
        case 134:
            return RID_SVXSTR_TIPOFTHEDAY_134;
        case 135:
            return RID_SVXSTR_TIPOFTHEDAY_135;
        case 136:
            return RID_SVXSTR_TIPOFTHEDAY_136;
        case 137:
            return RID_SVXSTR_TIPOFTHEDAY_137;
        case 138:
            return RID_SVXSTR_TIPOFTHEDAY_138;
        case 139:
            return RID_SVXSTR_TIPOFTHEDAY_139;
        case 140:
            return RID_SVXSTR_TIPOFTHEDAY_140;
        case 141:
            return RID_SVXSTR_TIPOFTHEDAY_141;
        case 142:
            return RID_SVXSTR_TIPOFTHEDAY_142;
        case 143:
            return RID_SVXSTR_TIPOFTHEDAY_143;
        case 144:
            return RID_SVXSTR_TIPOFTHEDAY_144;
        case 145:
            return RID_SVXSTR_TIPOFTHEDAY_145;
        case 146:
            return RID_SVXSTR_TIPOFTHEDAY_146;
        case 147:
            return RID_SVXSTR_TIPOFTHEDAY_147;
        case 148:
            return RID_SVXSTR_TIPOFTHEDAY_148;
        case 149:
            return RID_SVXSTR_TIPOFTHEDAY_149;
        case 150:
            return RID_SVXSTR_TIPOFTHEDAY_150;
        case 151:
            return RID_SVXSTR_TIPOFTHEDAY_151;
        case 152:
            return RID_SVXSTR_TIPOFTHEDAY_152;
        case 153:
            return RID_SVXSTR_TIPOFTHEDAY_153;
        case 154:
            return RID_SVXSTR_TIPOFTHEDAY_154;
        case 155:
            return RID_SVXSTR_TIPOFTHEDAY_155;
        case 156:
            return RID_SVXSTR_TIPOFTHEDAY_156;
        case 157:
            return RID_SVXSTR_TIPOFTHEDAY_157;
        case 158:
            return RID_SVXSTR_TIPOFTHEDAY_158;
        case 159:
            return RID_SVXSTR_TIPOFTHEDAY_159;
        case 160:
            return RID_SVXSTR_TIPOFTHEDAY_160;
        case 161:
            return RID_SVXSTR_TIPOFTHEDAY_161;
        case 162:
            return RID_SVXSTR_TIPOFTHEDAY_162;
        case 163:
            return RID_SVXSTR_TIPOFTHEDAY_163;
        case 164:
            return RID_SVXSTR_TIPOFTHEDAY_164;
        case 165:
            return RID_SVXSTR_TIPOFTHEDAY_165;
        case 166:
            return RID_SVXSTR_TIPOFTHEDAY_166;
        case 167:
            return RID_SVXSTR_TIPOFTHEDAY_167;
        case 168:
            return RID_SVXSTR_TIPOFTHEDAY_168;
        case 169:
            return RID_SVXSTR_TIPOFTHEDAY_169;
        case 170:
            return RID_SVXSTR_TIPOFTHEDAY_170;
        case 171:
            return RID_SVXSTR_TIPOFTHEDAY_171;
        case 172:
            return RID_SVXSTR_TIPOFTHEDAY_172;
        case 173:
            return RID_SVXSTR_TIPOFTHEDAY_173;
        case 174:
            return RID_SVXSTR_TIPOFTHEDAY_174;
        case 175:
            return RID_SVXSTR_TIPOFTHEDAY_175;
        case 176:
            return RID_SVXSTR_TIPOFTHEDAY_176;
        case 177:
            return RID_SVXSTR_TIPOFTHEDAY_177;
        case 178:
            return RID_SVXSTR_TIPOFTHEDAY_178;
        case 179:
            return RID_SVXSTR_TIPOFTHEDAY_179;
        case 180:
            return RID_SVXSTR_TIPOFTHEDAY_180;
        case 181:
            return RID_SVXSTR_TIPOFTHEDAY_181;
        case 182:
            return RID_SVXSTR_TIPOFTHEDAY_182;
        case 183:
            return RID_SVXSTR_TIPOFTHEDAY_183;
        case 184:
            return RID_SVXSTR_TIPOFTHEDAY_184;
        case 185:
            return RID_SVXSTR_TIPOFTHEDAY_185;
        case 186:
            return RID_SVXSTR_TIPOFTHEDAY_186;
        case 187:
            return RID_SVXSTR_TIPOFTHEDAY_187;
        case 188:
            return RID_SVXSTR_TIPOFTHEDAY_188;
        case 189:
            return RID_SVXSTR_TIPOFTHEDAY_189;
        case 190:
            return RID_SVXSTR_TIPOFTHEDAY_190;
        case 191:
            return RID_SVXSTR_TIPOFTHEDAY_191;
        case 192:
            return RID_SVXSTR_TIPOFTHEDAY_192;
        case 193:
            return RID_SVXSTR_TIPOFTHEDAY_193;
        case 194:
            return RID_SVXSTR_TIPOFTHEDAY_194;
        case 195:
            return RID_SVXSTR_TIPOFTHEDAY_195;
        case 196:
            return RID_SVXSTR_TIPOFTHEDAY_196;
        case 197:
            return RID_SVXSTR_TIPOFTHEDAY_197;
        case 198:
            return RID_SVXSTR_TIPOFTHEDAY_198;
        case 199:
            return RID_SVXSTR_TIPOFTHEDAY_199;
        case 200:
            return RID_SVXSTR_TIPOFTHEDAY_200;
        case 201:
            return RID_SVXSTR_TIPOFTHEDAY_201;
        case 202:
            return RID_SVXSTR_TIPOFTHEDAY_202;
        case 203:
            return RID_SVXSTR_TIPOFTHEDAY_203;
        case 204:
            return RID_SVXSTR_TIPOFTHEDAY_204;
        case 205:
            return RID_SVXSTR_TIPOFTHEDAY_205;
        case 206:
            return RID_SVXSTR_TIPOFTHEDAY_206;
        case 207:
            return RID_SVXSTR_TIPOFTHEDAY_207;
        case 208:
            return RID_SVXSTR_TIPOFTHEDAY_208;
        case 209:
            return RID_SVXSTR_TIPOFTHEDAY_209;
        case 210:
            return RID_SVXSTR_TIPOFTHEDAY_210;
        case 211:
            return RID_SVXSTR_TIPOFTHEDAY_211;
        case 212:
            return RID_SVXSTR_TIPOFTHEDAY_212;
        case 213:
            return RID_SVXSTR_TIPOFTHEDAY_213;
        case 214:
            return RID_SVXSTR_TIPOFTHEDAY_214;
        case 215:
            return RID_SVXSTR_TIPOFTHEDAY_215;
        case 216:
            return RID_SVXSTR_TIPOFTHEDAY_216;
        case 217:
            return RID_SVXSTR_TIPOFTHEDAY_217;
        case 218:
            return RID_SVXSTR_TIPOFTHEDAY_218;
        case 219:
            return RID_SVXSTR_TIPOFTHEDAY_219;
        case 220:
            return RID_SVXSTR_TIPOFTHEDAY_220;
        case 221:
            return RID_SVXSTR_TIPOFTHEDAY_221;
        case 222:
            return RID_SVXSTR_TIPOFTHEDAY_222;
        case 223:
            return RID_SVXSTR_TIPOFTHEDAY_223;
        case 224:
            return RID_SVXSTR_TIPOFTHEDAY_224;
        case 225:
            return RID_SVXSTR_TIPOFTHEDAY_225;
        case 226:
            return RID_SVXSTR_TIPOFTHEDAY_226;
        case 227:
            return RID_SVXSTR_TIPOFTHEDAY_227;
        case 228:
            return RID_SVXSTR_TIPOFTHEDAY_228;
        case 229:
            return RID_SVXSTR_TIPOFTHEDAY_229;
        case 230:
            return RID_SVXSTR_TIPOFTHEDAY_230;
        case 231:
            return RID_SVXSTR_TIPOFTHEDAY_231;
        case 232:
            return RID_SVXSTR_TIPOFTHEDAY_232;
        case 233:
            return RID_SVXSTR_TIPOFTHEDAY_233;
        case 234:
            return RID_SVXSTR_TIPOFTHEDAY_234;
        case 235:
            return RID_SVXSTR_TIPOFTHEDAY_235;
        case 236:
            return RID_SVXSTR_TIPOFTHEDAY_236;
        case 237:
            return RID_SVXSTR_TIPOFTHEDAY_237;
        case 238:
            return RID_SVXSTR_TIPOFTHEDAY_238;
        case 239:
            return RID_SVXSTR_TIPOFTHEDAY_239;
        case 240:
            return RID_SVXSTR_TIPOFTHEDAY_240;
        case 241:
            return RID_SVXSTR_TIPOFTHEDAY_241;
        case 242:
            return RID_SVXSTR_TIPOFTHEDAY_242;
        case 243:
            return RID_SVXSTR_TIPOFTHEDAY_243;
        case 244:
            return RID_SVXSTR_TIPOFTHEDAY_244;
        case 245:
            return RID_SVXSTR_TIPOFTHEDAY_245;
        case 246:
            return RID_SVXSTR_TIPOFTHEDAY_246;
        case 247:
            return RID_SVXSTR_TIPOFTHEDAY_247;
        case 248:
            return RID_SVXSTR_TIPOFTHEDAY_248;
        case 249:
            return RID_SVXSTR_TIPOFTHEDAY_249;
        case 250:
            return RID_SVXSTR_TIPOFTHEDAY_250;
        case 251:
            return RID_SVXSTR_TIPOFTHEDAY_251;
        case 252:
            return RID_SVXSTR_TIPOFTHEDAY_252;
        case 253:
            return RID_SVXSTR_TIPOFTHEDAY_253;
        case 254:
            return RID_SVXSTR_TIPOFTHEDAY_254;
        case 255:
            return RID_SVXSTR_TIPOFTHEDAY_255;
        case 256:
            return RID_SVXSTR_TIPOFTHEDAY_256;
        case 257:
            return RID_SVXSTR_TIPOFTHEDAY_257;
        case 258:
            return RID_SVXSTR_TIPOFTHEDAY_258;
        case 259:
            return RID_SVXSTR_TIPOFTHEDAY_259;
        case 260:
            return RID_SVXSTR_TIPOFTHEDAY_260;
        case 261:
            return RID_SVXSTR_TIPOFTHEDAY_261;
        case 262:
            return RID_SVXSTR_TIPOFTHEDAY_262;
        case 263:
            return RID_SVXSTR_TIPOFTHEDAY_263;
        case 264:
            return RID_SVXSTR_TIPOFTHEDAY_264;
        case 265:
            return RID_SVXSTR_TIPOFTHEDAY_265;
        case 266:
            return RID_SVXSTR_TIPOFTHEDAY_266;
        case 267:
            return RID_SVXSTR_TIPOFTHEDAY_267;
        case 268:
            return RID_SVXSTR_TIPOFTHEDAY_268;
        case 269:
            return RID_SVXSTR_TIPOFTHEDAY_269;
        case 270:
            return RID_SVXSTR_TIPOFTHEDAY_270;
        case 271:
            return RID_SVXSTR_TIPOFTHEDAY_271;
        case 272:
            return RID_SVXSTR_TIPOFTHEDAY_272;
        case 273:
            return RID_SVXSTR_TIPOFTHEDAY_273;
        case 274:
            return RID_SVXSTR_TIPOFTHEDAY_274;
        case 275:
            return RID_SVXSTR_TIPOFTHEDAY_275;
        case 276:
            return RID_SVXSTR_TIPOFTHEDAY_276;
        case 277:
            return RID_SVXSTR_TIPOFTHEDAY_277;
        case 278:
            return RID_SVXSTR_TIPOFTHEDAY_278;
        case 279:
            return RID_SVXSTR_TIPOFTHEDAY_279;
        case 280:
            return RID_SVXSTR_TIPOFTHEDAY_280;
        case 281:
            return RID_SVXSTR_TIPOFTHEDAY_281;
        case 282:
            return RID_SVXSTR_TIPOFTHEDAY_282;
        case 283:
            return RID_SVXSTR_TIPOFTHEDAY_283;
        case 284:
            return RID_SVXSTR_TIPOFTHEDAY_284;
        case 285:
            return RID_SVXSTR_TIPOFTHEDAY_285;
        case 286:
            return RID_SVXSTR_TIPOFTHEDAY_286;
        case 287:
            return RID_SVXSTR_TIPOFTHEDAY_287;
        case 288:
            return RID_SVXSTR_TIPOFTHEDAY_288;
        case 289:
            return RID_SVXSTR_TIPOFTHEDAY_289;
        case 290:
            return RID_SVXSTR_TIPOFTHEDAY_290;
        case 291:
            return RID_SVXSTR_TIPOFTHEDAY_291;
        case 292:
            return RID_SVXSTR_TIPOFTHEDAY_292;
        case 293:
            return RID_SVXSTR_TIPOFTHEDAY_293;
        case 294:
            return RID_SVXSTR_TIPOFTHEDAY_294;
        case 295:
            return RID_SVXSTR_TIPOFTHEDAY_295;
        case 296:
            return RID_SVXSTR_TIPOFTHEDAY_296;
        case 297:
            return RID_SVXSTR_TIPOFTHEDAY_297;
        case 298:
            return RID_SVXSTR_TIPOFTHEDAY_298;
        case 299:
            return RID_SVXSTR_TIPOFTHEDAY_299;
        case 300:
            return RID_SVXSTR_TIPOFTHEDAY_300;
        case 301:
            return RID_SVXSTR_TIPOFTHEDAY_301;
        case 302:
            return RID_SVXSTR_TIPOFTHEDAY_302;
        case 303:
            return RID_SVXSTR_TIPOFTHEDAY_303;
    }
}

void InfoDialog::UpdateTip()
{
    //get string
    OUString aText;
    aText = CuiResId(getResID(nCurrentTip));
    //move hyperlink into linkbutton
    sal_Int32 nPos = aText.indexOf("http");
    if (nPos > 0)
    {
        m_pLink->set_visible(true);
        if (aText.getLength() - nPos > 40)
            m_pLink->set_label(aText.copy(nPos, 40) + "...");
        else
            m_pLink->set_label(aText.copy(nPos));
        m_pLink->set_uri(aText.copy(nPos));
        aText = aText.copy(0, nPos - 1);
    }
    else
        m_pLink->set_visible(false);
    m_pText->set_label(aText);

    // import the image
    OUString aURL("$BRAND_BASE_DIR/" LIBO_SHARE_RESOURCE_FOLDER "/");
    rtl::Bootstrap::expandMacros(aURL);
    ::osl::FileBase::getSystemPathFromFileURL(aURL, aURL);
    OUString aName = "tipoftheday_" + OUString::number(nCurrentTip) + ".png";
    // use default image if none is available with the number
    if (!file_exists(aURL + aName))
        aName = "tipoftheday.png";

    // draw image
    Graphic aGraphic;
    if (GraphicFilter::LoadGraphic(aURL + aName, OUString(), aGraphic) == ERRCODE_NONE)
    {
        ScopedVclPtr<VirtualDevice> m_pVirDev;
        m_pVirDev = m_pImage->create_virtual_device();
        m_pVirDev->SetOutputSizePixel(aGraphic.GetSizePixel());
        m_pVirDev->DrawBitmapEx(Point(0, 0), aGraphic.GetBitmapEx());
        m_pImage->set_image(m_pVirDev.get());
        m_pVirDev.disposeAndClear();
    }
}

IMPL_LINK(InfoDialog, OnShowTipToggled, weld::ToggleButton&, rButton, void)
{
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ShowTipOfTheDay::set(rButton.get_active(), xChanges);
    xChanges->commit();
}

IMPL_LINK(InfoDialog, OnNextClick, weld::Button&, SAL_UNUSED_PARAMETER rButton, void)
{
    if (nCurrentTip < nNumberOfTips)
        nCurrentTip++;
    else
        nCurrentTip = 0;
    UpdateTip();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
