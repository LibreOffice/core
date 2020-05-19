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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_SPRMIDS_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_SPRMIDS_HXX

#include <sal/types.h>

namespace NS_sprm
{
const sal_uInt16 LN_PFSideBySide = 0x2404;
const sal_uInt16 LN_PBrcl = 0x2408;
const sal_uInt16 LN_PBrcp = 0x2409;
const sal_uInt16 LN_PBrcTop10 = 0x461c;
const sal_uInt16 LN_PBrcLeft10 = 0x461d;
const sal_uInt16 LN_PBrcBottom10 = 0x461e;
const sal_uInt16 LN_PBrcRight10 = 0x461f;
const sal_uInt16 LN_PBrcBetween10 = 0x4620;
const sal_uInt16 LN_PBrcBar10 = 0x4621;
const sal_uInt16 LN_PDxaFromText10 = 0x4622;
const sal_uInt16 LN_PRuler = 0xc632;
const sal_uInt16 LN_PISnapBaseLine = 0x243b;
const sal_uInt16 LN_PAnld = 0xc63e;
const sal_uInt16 LN_PPropRMark = 0xc63f;
const sal_uInt16 LN_PCrLf = 0x2444;
const sal_uInt16 LN_PHugePapx = 0x6645;
const sal_uInt16 LN_CChs = 0xea08;
const sal_uInt16 LN_CIdCharType = 0x480b;
const sal_uInt16 LN_CObjLocation = 0x680e;
const sal_uInt16 LN_CFFtcAsciSymb = 0x2a10;
const sal_uInt16 LN_CDefault = 0x2a32;
const sal_uInt16 LN_CFtcDefault = 0x4a3d;
const sal_uInt16 LN_CSizePos = 0xea3f;
const sal_uInt16 LN_CLid = 0x4a41;
const sal_uInt16 LN_CHpsInc = 0x2a44;
const sal_uInt16 LN_CHpsPosAdj = 0x2a46;
const sal_uInt16 LN_CHpsNew50 = 0xca49;
const sal_uInt16 LN_CHpsInc1 = 0xca4a;
const sal_uInt16 LN_CMajority50 = 0xca4c;
const sal_uInt16 LN_CHpsMul = 0x4a4d;
const sal_uInt16 LN_CFDiacColor = 0x085b;
const sal_uInt16 LN_CCpg = 0x486b;
const sal_uInt16 LN_PicBrcl = 0x2e00;
const sal_uInt16 LN_PicScale = 0xce01;
const sal_uInt16 LN_SOlstAnm = 0xd202;
const sal_uInt16 LN_SFAutoPgn = 0x300d;
const sal_uInt16 LN_SDyaPgn = 0xb00f;
const sal_uInt16 LN_SDxaPgn = 0xb010;
const sal_uInt16 LN_SGprfIhdt = 0x3014;
const sal_uInt16 LN_SBCustomize = 0x301e;
const sal_uInt16 LN_SPropRMark = 0xd227;
const sal_uInt16 LN_SFFacingCol = 0x3229;
const sal_uInt16 LN_TDefTable10 = 0xd606;
const sal_uInt16 LN_THTMLProps = 0x740c;
const sal_uInt16 LN_TSetBrc10 = 0xd626;
const sal_uInt16 LN_TSetShd80 = 0x7627;
const sal_uInt16 LN_TSetShdOdd80 = 0x7628;
const sal_uInt16 LN_TDiagLine = 0xd62a;

namespace v6
{
// Microsoft Word for Windows 6.0 Binary File Format
//                                              Parameter size
const sal_uInt16 sprmPIstd = 2; // short
const sal_uInt16 sprmPIstdPermute = 3; // variable
const sal_uInt16 sprmPIncLv1 = 4; // byte
const sal_uInt16 sprmPJc = 5; // byte
const sal_uInt16 sprmPFSideBySide = 6; // byte
const sal_uInt16 sprmPFKeep = 7; // byte
const sal_uInt16 sprmPFKeepFollow = 8; // byte
const sal_uInt16 sprmPPageBreakBefore = 9; // byte
const sal_uInt16 sprmPBrcl = 10; // byte
const sal_uInt16 sprmPBrcp = 11; // byte
const sal_uInt16 sprmPAnld = 12; // variable
const sal_uInt16 sprmPNLvlAnm = 13; // byte
const sal_uInt16 sprmPFNoLineNumb = 14; // byte
const sal_uInt16 sprmPChgTabsPapx = 15; // variable
const sal_uInt16 sprmPDxaRight = 16; // word
const sal_uInt16 sprmPDxaLeft = 17; // word
const sal_uInt16 sprmPNest = 18; // word
const sal_uInt16 sprmPDxaLeft1 = 19; // word
const sal_uInt16 sprmPDyaLine = 20; // long
const sal_uInt16 sprmPDyaBefore = 21; // word
const sal_uInt16 sprmPDyaAfter = 22; // word
const sal_uInt16 sprmPChgTabs = 23; // variable
const sal_uInt16 sprmPFInTable = 24; // byte
const sal_uInt16 sprmPTtp = 25; // byte
const sal_uInt16 sprmPDxaAbs = 26; // word
const sal_uInt16 sprmPDyaAbs = 27; // word
const sal_uInt16 sprmPDxaWidth = 28; // word
const sal_uInt16 sprmPPc = 29; // byte
const sal_uInt16 sprmPBrcTop10 = 30; // word
const sal_uInt16 sprmPBrcLeft10 = 31; // word
const sal_uInt16 sprmPBrcBottom10 = 32; // word
const sal_uInt16 sprmPBrcRight10 = 33; // word
const sal_uInt16 sprmPBrcBetween10 = 34; // word
const sal_uInt16 sprmPBrcBar10 = 35; // word
const sal_uInt16 sprmPFromText10 = 36; // word
const sal_uInt16 sprmPWr = 37; // byte
const sal_uInt16 sprmPBrcTop = 38; // word
const sal_uInt16 sprmPBrcLeft = 39; // word
const sal_uInt16 sprmPBrcBottom = 40; // word
const sal_uInt16 sprmPBrcRight = 41; // word
const sal_uInt16 sprmPBrcBetween = 42; // word
const sal_uInt16 sprmPBrcBar = 43; // word
const sal_uInt16 sprmPFNoAutoHyph = 44; // byte
const sal_uInt16 sprmPWHeightAbs = 45; // word
const sal_uInt16 sprmPDcs = 46; // short
const sal_uInt16 sprmPShd = 47; // word
const sal_uInt16 sprmPDyaFromText = 48; // word
const sal_uInt16 sprmPDxaFromText = 49; // word
const sal_uInt16 sprmPFLocked = 50; // byte
const sal_uInt16 sprmPFWidowControl = 51; // byte
const sal_uInt16 sprmPRuler = 52;
const sal_uInt16 sprmCFStrikeRM = 65; // bit
const sal_uInt16 sprmCFRMark = 66; // bit
const sal_uInt16 sprmCFFldVanish = 67; // bit
const sal_uInt16 sprmCPicLocation = 68; // variable
const sal_uInt16 sprmCIbstRMark = 69; // short
const sal_uInt16 sprmCDttmRMark = 70; // long
const sal_uInt16 sprmCFData = 71; // bit
const sal_uInt16 sprmCRMReason = 72; // short
const sal_uInt16 sprmCChse = 73; // 3 bytes
const sal_uInt16 sprmCSymbol = 74; // variable
const sal_uInt16 sprmCFOle2 = 75; // bit
const sal_uInt16 sprmCIstd = 80; // short
const sal_uInt16 sprmCIstdPermute = 81; // variable
const sal_uInt16 sprmCDefault = 82; // variable
const sal_uInt16 sprmCPlain = 83; // 0
const sal_uInt16 sprmCFBold = 85; // byte
const sal_uInt16 sprmCFItalic = 86; // byte
const sal_uInt16 sprmCFStrike = 87; // byte
const sal_uInt16 sprmCFOutline = 88; // byte
const sal_uInt16 sprmCFShadow = 89; // byte
const sal_uInt16 sprmCFSmallCaps = 90; // byte
const sal_uInt16 sprmCFCaps = 91; // byte
const sal_uInt16 sprmCFVanish = 92; // byte
const sal_uInt16 sprmCFtc = 93; // word
const sal_uInt16 sprmCKul = 94; // byte
const sal_uInt16 sprmCSizePos = 95; // 3 bytes
const sal_uInt16 sprmCDxaSpace = 96; // word
const sal_uInt16 sprmCLid = 97; // word
const sal_uInt16 sprmCIco = 98; // byte
const sal_uInt16 sprmCHps = 99; // byte
const sal_uInt16 sprmCHpsInc = 100; // byte
const sal_uInt16 sprmCHpsPos = 101; // byte
const sal_uInt16 sprmCHpsPosAdj = 102; // byte
const sal_uInt16 sprmCMajority = 103; // variable
const sal_uInt16 sprmCIss = 104; // byte
const sal_uInt16 sprmCHpsNew50 = 105; // variable
const sal_uInt16 sprmCHpsInc1 = 106; // variable
const sal_uInt16 sprmCHpsKern = 107; // short
const sal_uInt16 sprmCMajority50 = 108; // variable
const sal_uInt16 sprmCHpsMul = 109; // short
const sal_uInt16 sprmCCondHyhen = 110; // short
const sal_uInt16 sprmCFSpec = 117; // bit
const sal_uInt16 sprmCFObj = 118; // bit
const sal_uInt16 sprmPicBrcl = 119; // byte
const sal_uInt16 sprmPicScale = 120; // length
const sal_uInt16 sprmPicBrcTop = 121; // word
const sal_uInt16 sprmPicBrcLeft = 122; // word
const sal_uInt16 sprmPicBrcBottom = 123; // word
const sal_uInt16 sprmPicBrcRight = 124; // word
const sal_uInt16 sprmSScnsPgn = 131; // byte
const sal_uInt16 sprmSiHeadingPgn = 132; // byte
const sal_uInt16 sprmSOlstAnm = 133; // variable
const sal_uInt16 sprmSDxaColWidth = 136; // 3 bytes
const sal_uInt16 sprmSDxaColSpacing = 137; // 3 bytes
const sal_uInt16 sprmSFEvenlySpaced = 138; // byte
const sal_uInt16 sprmSFProtected = 139; // byte
const sal_uInt16 sprmSDmBinFirst = 140; // word
const sal_uInt16 sprmSDmBinOther = 141; // word
const sal_uInt16 sprmSBkc = 142; // byte
const sal_uInt16 sprmSFTitlePage = 143; // byte
const sal_uInt16 sprmSCcolumns = 144; // word
const sal_uInt16 sprmSDxaColumns = 145; // word
const sal_uInt16 sprmSFAutoPgn = 146; // byte
const sal_uInt16 sprmSNfcPgn = 147; // byte
const sal_uInt16 sprmSDyaPgn = 148; // short
const sal_uInt16 sprmSDxaPgn = 149; // short
const sal_uInt16 sprmSFPgnRestart = 150; // byte
const sal_uInt16 sprmSFEndnote = 151; // byte
const sal_uInt16 sprmSLnc = 152; // byte
const sal_uInt16 sprmSGprfIhdt = 153; // byte
const sal_uInt16 sprmSNLnnMod = 154; // word
const sal_uInt16 sprmSDxaLnn = 155; // word
const sal_uInt16 sprmSDyaHdrTop = 156; // word
const sal_uInt16 sprmSDyaHdrBottom = 157; // word
const sal_uInt16 sprmSLBetween = 158; // byte
const sal_uInt16 sprmSVjc = 159; // byte
const sal_uInt16 sprmSLnnMin = 160; // word
const sal_uInt16 sprmSPgnStart = 161; // word
const sal_uInt16 sprmSBOrientation = 162; // byte
const sal_uInt16 sprmSBCustomize = 163;
const sal_uInt16 sprmSXaPage = 164; // word
const sal_uInt16 sprmSYaPage = 165; // word
const sal_uInt16 sprmSDxaLeft = 166; // word
const sal_uInt16 sprmSDxaRight = 167; // word
const sal_uInt16 sprmSDyaTop = 168; // word
const sal_uInt16 sprmSDyaBottom = 169; // word
const sal_uInt16 sprmSDzaGutter = 170; // word
const sal_uInt16 sprmSDMPaperReq = 171; // word
const sal_uInt16 sprmTJc = 182; // word (low
const sal_uInt16 sprmTDxaLeft = 183; // word
const sal_uInt16 sprmTDxaGapHalf = 184; // word
const sal_uInt16 sprmTFCantSplit = 185; // byte
const sal_uInt16 sprmTTableHeader = 186; // byte
const sal_uInt16 sprmTTableBorders = 187; // 12 bytes
const sal_uInt16 sprmTDefTable10 = 188; // variable
const sal_uInt16 sprmTDyaRowHeight = 189; // word
const sal_uInt16 sprmTDefTable = 190;
const sal_uInt16 sprmTDefTableShd = 191;
const sal_uInt16 sprmTTlp = 192; // 4 bytes
const sal_uInt16 sprmTSetBrc = 193; // 5 bytes
const sal_uInt16 sprmTInsert = 194; // 4 bytes
const sal_uInt16 sprmTDelete = 195; // word
const sal_uInt16 sprmTDxaCol = 196; // 4 bytes
const sal_uInt16 sprmTMerge = 197; // word
const sal_uInt16 sprmTSplit = 198; // word
const sal_uInt16 sprmTSetBrc10 = 199; // 5 bytes
const sal_uInt16 sprmTSetShd = 200; // 4 bytes
const sal_uInt16 sprmMax = 208;
}

// [MS-DOC] - v20170112 Section 2.2.5.1
enum class sgc
{
    paragraph = 1,
    character = 2,
    picture = 3,
    section = 4,
    table = 5
};
enum class spra
{
    operand_toggle_1b_0 = 0,
    operand_1b_1 = 1,
    operand_2b_2 = 2,
    operand_4b_3 = 3,
    operand_2b_4 = 4,
    operand_2b_5 = 5,
    operand_varlen_6 = 6,
    operand_3b_7 = 7
};
#define SPRM_PART(num, mask, shift) ((static_cast<sal_uInt16>(num) & mask) << shift)
#define SPRM(ispmd, fSpec, sgc, spra)                                                              \
    SPRM_PART(ispmd, 0x01FF, 0) + SPRM_PART(fSpec, 0x0001, 9) + SPRM_PART(sgc, 0x0007, 10)         \
        + SPRM_PART(spra, 0x0007, 13)
#define SPRM_PAR(ispmd, fSpec, spra) SPRM(ispmd, fSpec, sgc::paragraph, spra)
#define SPRM_CHR(ispmd, fSpec, spra) SPRM(ispmd, fSpec, sgc::character, spra)
#define SPRM_PIC(ispmd, fSpec, spra) SPRM(ispmd, fSpec, sgc::picture, spra)
#define SPRM_SEC(ispmd, fSpec, spra) SPRM(ispmd, fSpec, sgc::section, spra)
#define SPRM_TBL(ispmd, fSpec, spra) SPRM(ispmd, fSpec, sgc::table, spra)

// [MS-DOC] - v20170112 Section 2.6.1
const sal_uInt16 sprmCFRMarkDel = SPRM_CHR(0x00, 0, spra::operand_toggle_1b_0); // 0x0800
const sal_uInt16 sprmCFRMarkIns = SPRM_CHR(0x01, 0, spra::operand_toggle_1b_0); // 0x0801
const sal_uInt16 sprmCFFldVanish = SPRM_CHR(0x02, 0, spra::operand_toggle_1b_0); // 0x0802
const sal_uInt16 sprmCPicLocation = SPRM_CHR(0x03, 1, spra::operand_4b_3); // 0x6A03
const sal_uInt16 sprmCIbstRMark = SPRM_CHR(0x04, 0, spra::operand_2b_2); // 0x4804
const sal_uInt16 sprmCDttmRMark = SPRM_CHR(0x05, 0, spra::operand_4b_3); // 0x6805
const sal_uInt16 sprmCFData = SPRM_CHR(0x06, 0, spra::operand_toggle_1b_0); // 0x0806
const sal_uInt16 sprmCIdslRMark = SPRM_CHR(0x07, 0, spra::operand_2b_2); // 0x4807
const sal_uInt16 sprmCSymbol = SPRM_CHR(0x09, 1, spra::operand_4b_3); // 0x6A09
const sal_uInt16 sprmCFOle2 = SPRM_CHR(0x0A, 0, spra::operand_toggle_1b_0); // 0x080A
const sal_uInt16 sprmCHighlight = SPRM_CHR(0x0C, 1, spra::operand_1b_1); // 0x2A0C
const sal_uInt16 sprmCFWebHidden = SPRM_CHR(0x11, 0, spra::operand_toggle_1b_0); // 0x0811
const sal_uInt16 sprmCRsidProp = SPRM_CHR(0x15, 0, spra::operand_4b_3); // 0x6815
const sal_uInt16 sprmCRsidText = SPRM_CHR(0x16, 0, spra::operand_4b_3); // 0x6816
const sal_uInt16 sprmCRsidRMDel = SPRM_CHR(0x17, 0, spra::operand_4b_3); // 0x6817
const sal_uInt16 sprmCFSpecVanish = SPRM_CHR(0x18, 0, spra::operand_toggle_1b_0); // 0x0818
const sal_uInt16 sprmCFMathPr = SPRM_CHR(0x1A, 0, spra::operand_varlen_6); // 0xC81A
const sal_uInt16 sprmCIstd = SPRM_CHR(0x30, 1, spra::operand_2b_2); // 0x4A30
const sal_uInt16 sprmCIstdPermute = SPRM_CHR(0x31, 1, spra::operand_varlen_6); // 0xCA31
const sal_uInt16 sprmCPlain = SPRM_CHR(0x33, 1, spra::operand_1b_1); // 0x2A33
const sal_uInt16 sprmCKcd = SPRM_CHR(0x34, 1, spra::operand_1b_1); // 0x2A34
const sal_uInt16 sprmCFBold = SPRM_CHR(0x35, 0, spra::operand_toggle_1b_0); // 0x0835
const sal_uInt16 sprmCFItalic = SPRM_CHR(0x36, 0, spra::operand_toggle_1b_0); // 0x0836
const sal_uInt16 sprmCFStrike = SPRM_CHR(0x37, 0, spra::operand_toggle_1b_0); // 0x0837
const sal_uInt16 sprmCFOutline = SPRM_CHR(0x38, 0, spra::operand_toggle_1b_0); // 0x0838
const sal_uInt16 sprmCFShadow = SPRM_CHR(0x39, 0, spra::operand_toggle_1b_0); // 0x0839
const sal_uInt16 sprmCFSmallCaps = SPRM_CHR(0x3A, 0, spra::operand_toggle_1b_0); // 0x083A
const sal_uInt16 sprmCFCaps = SPRM_CHR(0x3B, 0, spra::operand_toggle_1b_0); // 0x083B
const sal_uInt16 sprmCFVanish = SPRM_CHR(0x3C, 0, spra::operand_toggle_1b_0); // 0x083C
const sal_uInt16 sprmCKul = SPRM_CHR(0x3E, 1, spra::operand_1b_1); // 0x2A3E
const sal_uInt16 sprmCDxaSpace = SPRM_CHR(0x40, 0, spra::operand_2b_4); // 0x8840
const sal_uInt16 sprmCIco = SPRM_CHR(0x42, 1, spra::operand_1b_1); // 0x2A42
const sal_uInt16 sprmCHps = SPRM_CHR(0x43, 1, spra::operand_2b_2); // 0x4A43
const sal_uInt16 sprmCHpsPos = SPRM_CHR(0x45, 0, spra::operand_2b_2); // 0x4845
const sal_uInt16 sprmCMajority = SPRM_CHR(0x47, 1, spra::operand_varlen_6); // 0xCA47
const sal_uInt16 sprmCIss = SPRM_CHR(0x48, 1, spra::operand_1b_1); // 0x2A48
const sal_uInt16 sprmCHpsKern = SPRM_CHR(0x4B, 0, spra::operand_2b_2); // 0x484B
const sal_uInt16 sprmCHresi = SPRM_CHR(0x4E, 0, spra::operand_2b_2); // 0x484E
const sal_uInt16 sprmCRgFtc0 = SPRM_CHR(0x4F, 1, spra::operand_2b_2); // 0x4A4F
const sal_uInt16 sprmCRgFtc1 = SPRM_CHR(0x50, 1, spra::operand_2b_2); // 0x4A50
const sal_uInt16 sprmCRgFtc2 = SPRM_CHR(0x51, 1, spra::operand_2b_2); // 0x4A51
const sal_uInt16 sprmCCharScale = SPRM_CHR(0x52, 0, spra::operand_2b_2); // 0x4852
const sal_uInt16 sprmCFDStrike = SPRM_CHR(0x53, 1, spra::operand_1b_1); // 0x2A53
const sal_uInt16 sprmCFImprint = SPRM_CHR(0x54, 0, spra::operand_toggle_1b_0); // 0x0854
const sal_uInt16 sprmCFSpec = SPRM_CHR(0x55, 0, spra::operand_toggle_1b_0); // 0x0855
const sal_uInt16 sprmCFObj = SPRM_CHR(0x56, 0, spra::operand_toggle_1b_0); // 0x0856
const sal_uInt16 sprmCPropRMark90 = SPRM_CHR(0x57, 1, spra::operand_varlen_6); // 0xCA57
const sal_uInt16 sprmCFEmboss = SPRM_CHR(0x58, 0, spra::operand_toggle_1b_0); // 0x0858
const sal_uInt16 sprmCSfxText = SPRM_CHR(0x59, 0, spra::operand_1b_1); // 0x2859
const sal_uInt16 sprmCFBiDi = SPRM_CHR(0x5A, 0, spra::operand_toggle_1b_0); // 0x085A
const sal_uInt16 sprmCFBoldBi = SPRM_CHR(0x5C, 0, spra::operand_toggle_1b_0); // 0x085C
const sal_uInt16 sprmCFItalicBi = SPRM_CHR(0x5D, 0, spra::operand_toggle_1b_0); // 0x085D
const sal_uInt16 sprmCFtcBi = SPRM_CHR(0x5E, 1, spra::operand_2b_2); // 0x4A5E
const sal_uInt16 sprmCLidBi = SPRM_CHR(0x5F, 0, spra::operand_2b_2); // 0x485F
const sal_uInt16 sprmCIcoBi = SPRM_CHR(0x60, 1, spra::operand_2b_2); // 0x4A60
const sal_uInt16 sprmCHpsBi = SPRM_CHR(0x61, 1, spra::operand_2b_2); // 0x4A61
const sal_uInt16 sprmCDispFldRMark = SPRM_CHR(0x62, 1, spra::operand_varlen_6); // 0xCA62
const sal_uInt16 sprmCIbstRMarkDel = SPRM_CHR(0x63, 0, spra::operand_2b_2); // 0x4863
const sal_uInt16 sprmCDttmRMarkDel = SPRM_CHR(0x64, 0, spra::operand_4b_3); // 0x6864
const sal_uInt16 sprmCBrc80 = SPRM_CHR(0x65, 0, spra::operand_4b_3); // 0x6865
const sal_uInt16 sprmCShd80 = SPRM_CHR(0x66, 0, spra::operand_2b_2); // 0x4866
const sal_uInt16 sprmCIdslRMarkDel = SPRM_CHR(0x67, 0, spra::operand_2b_2); // 0x4867
const sal_uInt16 sprmCFUsePgsuSettings = SPRM_CHR(0x68, 0, spra::operand_toggle_1b_0); // 0x0868
const sal_uInt16 sprmCRgLid0_80 = SPRM_CHR(0x6D, 0, spra::operand_2b_2); // 0x486D
const sal_uInt16 sprmCRgLid1_80 = SPRM_CHR(0x6E, 0, spra::operand_2b_2); // 0x486E
const sal_uInt16 sprmCIdctHint = SPRM_CHR(0x6F, 0, spra::operand_1b_1); // 0x286F
const sal_uInt16 sprmCCv = SPRM_CHR(0x70, 0, spra::operand_4b_3); // 0x6870
const sal_uInt16 sprmCShd = SPRM_CHR(0x71, 1, spra::operand_varlen_6); // 0xCA71
const sal_uInt16 sprmCBrc = SPRM_CHR(0x72, 1, spra::operand_varlen_6); // 0xCA72
const sal_uInt16 sprmCRgLid0 = SPRM_CHR(0x73, 0, spra::operand_2b_2); // 0x4873
const sal_uInt16 sprmCRgLid1 = SPRM_CHR(0x74, 0, spra::operand_2b_2); // 0x4874
const sal_uInt16 sprmCFNoProof = SPRM_CHR(0x75, 0, spra::operand_toggle_1b_0); // 0x0875
const sal_uInt16 sprmCFitText = SPRM_CHR(0x76, 1, spra::operand_varlen_6); // 0xCA76
const sal_uInt16 sprmCCvUl = SPRM_CHR(0x77, 0, spra::operand_4b_3); // 0x6877
const sal_uInt16 sprmCFELayout = SPRM_CHR(0x78, 1, spra::operand_varlen_6); // 0xCA78
const sal_uInt16 sprmCLbcCRJ = SPRM_CHR(0x79, 0, spra::operand_1b_1); // 0x2879
const sal_uInt16 sprmCFComplexScripts = SPRM_CHR(0x82, 0, spra::operand_toggle_1b_0); // 0x0882
const sal_uInt16 sprmCWall = SPRM_CHR(0x83, 1, spra::operand_1b_1); // 0x2A83
const sal_uInt16 sprmCCnf = SPRM_CHR(0x85, 1, spra::operand_varlen_6); // 0xCA85
const sal_uInt16 sprmCNeedFontFixup = SPRM_CHR(0x86, 1, spra::operand_1b_1); // 0x2A86
const sal_uInt16 sprmCPbiIBullet = SPRM_CHR(0x87, 0, spra::operand_4b_3); // 0x6887
const sal_uInt16 sprmCPbiGrf = SPRM_CHR(0x88, 0, spra::operand_2b_2); // 0x4888
const sal_uInt16 sprmCPropRMark = SPRM_CHR(0x89, 1, spra::operand_varlen_6); // 0xCA89
const sal_uInt16 sprmCFSdtVanish = SPRM_CHR(0x90, 1, spra::operand_1b_1); // 0x2A90

// [MS-DOC] - v20170112 Section 2.6.2
const sal_uInt16 sprmPIstd = SPRM_PAR(0x00, 1, spra::operand_2b_2); // 0x4600
const sal_uInt16 sprmPIstdPermute = SPRM_PAR(0x01, 1, spra::operand_varlen_6); // 0xC601
const sal_uInt16 sprmPIncLvl = SPRM_PAR(0x02, 1, spra::operand_1b_1); // 0x2602
const sal_uInt16 sprmPJc80 = SPRM_PAR(0x03, 0, spra::operand_1b_1); // 0x2403
const sal_uInt16 sprmPFKeep = SPRM_PAR(0x05, 0, spra::operand_1b_1); // 0x2405
const sal_uInt16 sprmPFKeepFollow = SPRM_PAR(0x06, 0, spra::operand_1b_1); // 0x2406
const sal_uInt16 sprmPFPageBreakBefore = SPRM_PAR(0x07, 0, spra::operand_1b_1); // 0x2407
const sal_uInt16 sprmPIlvl = SPRM_PAR(0x0A, 1, spra::operand_1b_1); // 0x260A
const sal_uInt16 sprmPIlfo = SPRM_PAR(0x0B, 1, spra::operand_2b_2); // 0x460B
const sal_uInt16 sprmPFNoLineNumb = SPRM_PAR(0x0C, 0, spra::operand_1b_1); // 0x240C
const sal_uInt16 sprmPChgTabsPapx = SPRM_PAR(0x0D, 1, spra::operand_varlen_6); // 0xC60D
const sal_uInt16 sprmPDxaRight80 = SPRM_PAR(0x0E, 0, spra::operand_2b_4); // 0x840E
const sal_uInt16 sprmPDxaLeft80 = SPRM_PAR(0x0F, 0, spra::operand_2b_4); // 0x840F
const sal_uInt16 sprmPNest80 = SPRM_PAR(0x10, 1, spra::operand_2b_2); // 0x4610
const sal_uInt16 sprmPDxaLeft180 = SPRM_PAR(0x11, 0, spra::operand_2b_4); // 0x8411
const sal_uInt16 sprmPDyaLine = SPRM_PAR(0x12, 0, spra::operand_4b_3); // 0x6412
const sal_uInt16 sprmPDyaBefore = SPRM_PAR(0x13, 0, spra::operand_2b_5); // 0xA413
const sal_uInt16 sprmPDyaAfter = SPRM_PAR(0x14, 0, spra::operand_2b_5); // 0xA414
const sal_uInt16 sprmPChgTabs = SPRM_PAR(0x15, 1, spra::operand_varlen_6); // 0xC615
const sal_uInt16 sprmPFInTable = SPRM_PAR(0x16, 0, spra::operand_1b_1); // 0x2416
const sal_uInt16 sprmPFTtp = SPRM_PAR(0x17, 0, spra::operand_1b_1); // 0x2417
const sal_uInt16 sprmPDxaAbs = SPRM_PAR(0x18, 0, spra::operand_2b_4); // 0x8418
const sal_uInt16 sprmPDyaAbs = SPRM_PAR(0x19, 0, spra::operand_2b_4); // 0x8419
const sal_uInt16 sprmPDxaWidth = SPRM_PAR(0x1A, 0, spra::operand_2b_4); // 0x841A
const sal_uInt16 sprmPPc = SPRM_PAR(0x1B, 1, spra::operand_1b_1); // 0x261B
const sal_uInt16 sprmPWr = SPRM_PAR(0x23, 0, spra::operand_1b_1); // 0x2423
const sal_uInt16 sprmPBrcTop80 = SPRM_PAR(0x24, 0, spra::operand_4b_3); // 0x6424
const sal_uInt16 sprmPBrcLeft80 = SPRM_PAR(0x25, 0, spra::operand_4b_3); // 0x6425
const sal_uInt16 sprmPBrcBottom80 = SPRM_PAR(0x26, 0, spra::operand_4b_3); // 0x6426
const sal_uInt16 sprmPBrcRight80 = SPRM_PAR(0x27, 0, spra::operand_4b_3); // 0x6427
const sal_uInt16 sprmPBrcBetween80 = SPRM_PAR(0x28, 0, spra::operand_4b_3); // 0x6428
const sal_uInt16 sprmPBrcBar80 = SPRM_PAR(0x29, 1, spra::operand_4b_3); // 0x6629
const sal_uInt16 sprmPFNoAutoHyph = SPRM_PAR(0x2A, 0, spra::operand_1b_1); // 0x242A
const sal_uInt16 sprmPWHeightAbs = SPRM_PAR(0x2B, 0, spra::operand_2b_2); // 0x442B
const sal_uInt16 sprmPDcs = SPRM_PAR(0x2C, 0, spra::operand_2b_2); // 0x442C
const sal_uInt16 sprmPShd80 = SPRM_PAR(0x2D, 0, spra::operand_2b_2); // 0x442D
const sal_uInt16 sprmPDyaFromText = SPRM_PAR(0x2E, 0, spra::operand_2b_4); // 0x842E
const sal_uInt16 sprmPDxaFromText = SPRM_PAR(0x2F, 0, spra::operand_2b_4); // 0x842F
const sal_uInt16 sprmPFLocked = SPRM_PAR(0x30, 0, spra::operand_1b_1); // 0x2430
const sal_uInt16 sprmPFWidowControl = SPRM_PAR(0x31, 0, spra::operand_1b_1); // 0x2431
const sal_uInt16 sprmPFKinsoku = SPRM_PAR(0x33, 0, spra::operand_1b_1); // 0x2433
const sal_uInt16 sprmPFWordWrap = SPRM_PAR(0x34, 0, spra::operand_1b_1); // 0x2434
const sal_uInt16 sprmPFOverflowPunct = SPRM_PAR(0x35, 0, spra::operand_1b_1); // 0x2435
const sal_uInt16 sprmPFTopLinePunct = SPRM_PAR(0x36, 0, spra::operand_1b_1); // 0x2436
const sal_uInt16 sprmPFAutoSpaceDE = SPRM_PAR(0x37, 0, spra::operand_1b_1); // 0x2437
const sal_uInt16 sprmPFAutoSpaceDN = SPRM_PAR(0x38, 0, spra::operand_1b_1); // 0x2438
const sal_uInt16 sprmPWAlignFont = SPRM_PAR(0x39, 0, spra::operand_2b_2); // 0x4439
const sal_uInt16 sprmPFrameTextFlow = SPRM_PAR(0x3A, 0, spra::operand_2b_2); // 0x443A
const sal_uInt16 sprmPOutLvl = SPRM_PAR(0x40, 1, spra::operand_1b_1); // 0x2640
const sal_uInt16 sprmPFBiDi = SPRM_PAR(0x41, 0, spra::operand_1b_1); // 0x2441
const sal_uInt16 sprmPFNumRMIns = SPRM_PAR(0x43, 0, spra::operand_1b_1); // 0x2443
const sal_uInt16 sprmPNumRM = SPRM_PAR(0x45, 1, spra::operand_varlen_6); // 0xC645
const sal_uInt16 sprmPHugePapx = SPRM_PAR(0x46, 1, spra::operand_4b_3); // 0x6646
const sal_uInt16 sprmPFUsePgsuSettings = SPRM_PAR(0x47, 0, spra::operand_1b_1); // 0x2447
const sal_uInt16 sprmPFAdjustRight = SPRM_PAR(0x48, 0, spra::operand_1b_1); // 0x2448
const sal_uInt16 sprmPItap = SPRM_PAR(0x49, 1, spra::operand_4b_3); // 0x6649
const sal_uInt16 sprmPDtap = SPRM_PAR(0x4A, 1, spra::operand_4b_3); // 0x664A
const sal_uInt16 sprmPFInnerTableCell = SPRM_PAR(0x4B, 0, spra::operand_1b_1); // 0x244B
const sal_uInt16 sprmPFInnerTtp = SPRM_PAR(0x4C, 0, spra::operand_1b_1); // 0x244C
const sal_uInt16 sprmPShd = SPRM_PAR(0x4D, 1, spra::operand_varlen_6); // 0xC64D
const sal_uInt16 sprmPBrcTop = SPRM_PAR(0x4E, 1, spra::operand_varlen_6); // 0xC64E
const sal_uInt16 sprmPBrcLeft = SPRM_PAR(0x4F, 1, spra::operand_varlen_6); // 0xC64F
const sal_uInt16 sprmPBrcBottom = SPRM_PAR(0x50, 1, spra::operand_varlen_6); // 0xC650
const sal_uInt16 sprmPBrcRight = SPRM_PAR(0x51, 1, spra::operand_varlen_6); // 0xC651
const sal_uInt16 sprmPBrcBetween = SPRM_PAR(0x52, 1, spra::operand_varlen_6); // 0xC652
const sal_uInt16 sprmPBrcBar = SPRM_PAR(0x53, 1, spra::operand_varlen_6); // 0xC653
const sal_uInt16 sprmPDxcRight = SPRM_PAR(0x55, 0, spra::operand_2b_2); // 0x4455
const sal_uInt16 sprmPDxcLeft = SPRM_PAR(0x56, 0, spra::operand_2b_2); // 0x4456
const sal_uInt16 sprmPDxcLeft1 = SPRM_PAR(0x57, 0, spra::operand_2b_2); // 0x4457
const sal_uInt16 sprmPDylBefore = SPRM_PAR(0x58, 0, spra::operand_2b_2); // 0x4458
const sal_uInt16 sprmPDylAfter = SPRM_PAR(0x59, 0, spra::operand_2b_2); // 0x4459
const sal_uInt16 sprmPFOpenTch = SPRM_PAR(0x5A, 0, spra::operand_1b_1); // 0x245A
const sal_uInt16 sprmPFDyaBeforeAuto = SPRM_PAR(0x5B, 0, spra::operand_1b_1); // 0x245B
const sal_uInt16 sprmPFDyaAfterAuto = SPRM_PAR(0x5C, 0, spra::operand_1b_1); // 0x245C
const sal_uInt16 sprmPDxaRight = SPRM_PAR(0x5D, 0, spra::operand_2b_4); // 0x845D
const sal_uInt16 sprmPDxaLeft = SPRM_PAR(0x5E, 0, spra::operand_2b_4); // 0x845E
const sal_uInt16 sprmPNest = SPRM_PAR(0x5F, 1, spra::operand_2b_2); // 0x465F
const sal_uInt16 sprmPDxaLeft1 = SPRM_PAR(0x60, 0, spra::operand_2b_4); // 0x8460
const sal_uInt16 sprmPJc = SPRM_PAR(0x61, 0, spra::operand_1b_1); // 0x2461
const sal_uInt16 sprmPFNoAllowOverlap = SPRM_PAR(0x62, 0, spra::operand_1b_1); // 0x2462
const sal_uInt16 sprmPWall = SPRM_PAR(0x64, 1, spra::operand_1b_1); // 0x2664
const sal_uInt16 sprmPIpgp = SPRM_PAR(0x65, 0, spra::operand_4b_3); // 0x6465
const sal_uInt16 sprmPCnf = SPRM_PAR(0x66, 1, spra::operand_varlen_6); // 0xC666
const sal_uInt16 sprmPRsid = SPRM_PAR(0x67, 0, spra::operand_4b_3); // 0x6467
const sal_uInt16 sprmPIstdListPermute = SPRM_PAR(0x69, 1, spra::operand_varlen_6); // 0xC669
const sal_uInt16 sprmPTableProps = SPRM_PAR(0x6B, 0, spra::operand_4b_3); // 0x646B
const sal_uInt16 sprmPTIstdInfo = SPRM_PAR(0x6C, 1, spra::operand_varlen_6); // 0xC66C
const sal_uInt16 sprmPFContextualSpacing = SPRM_PAR(0x6D, 0, spra::operand_1b_1); // 0x246D
const sal_uInt16 sprmPPropRMark = SPRM_PAR(0x6F, 1, spra::operand_varlen_6); // 0xC66F
const sal_uInt16 sprmPFMirrorIndents = SPRM_PAR(0x70, 0, spra::operand_1b_1); // 0x2470
const sal_uInt16 sprmPTtwo = SPRM_PAR(0x71, 0, spra::operand_1b_1); // 0x2471

// [MS-DOC] - v20170112 Section 2.6.3
const sal_uInt16 sprmTJc90 = SPRM_TBL(0x00, 0, spra::operand_2b_2); // 0x5400
const sal_uInt16 sprmTDxaLeft = SPRM_TBL(0x01, 1, spra::operand_2b_4); // 0x9601
const sal_uInt16 sprmTDxaGapHalf = SPRM_TBL(0x02, 1, spra::operand_2b_4); // 0x9602
const sal_uInt16 sprmTFCantSplit90 = SPRM_TBL(0x03, 0, spra::operand_1b_1); // 0x3403
const sal_uInt16 sprmTTableHeader = SPRM_TBL(0x04, 0, spra::operand_1b_1); // 0x3404
const sal_uInt16 sprmTTableBorders80 = SPRM_TBL(0x05, 1, spra::operand_varlen_6); // 0xD605
const sal_uInt16 sprmTDyaRowHeight = SPRM_TBL(0x07, 0, spra::operand_2b_4); // 0x9407
const sal_uInt16 sprmTDefTable = SPRM_TBL(0x08, 1, spra::operand_varlen_6); // 0xD608
const sal_uInt16 sprmTDefTableShd80 = SPRM_TBL(0x09, 1, spra::operand_varlen_6); // 0xD609
const sal_uInt16 sprmTTlp = SPRM_TBL(0x0A, 0, spra::operand_4b_3); // 0x740A
const sal_uInt16 sprmTFBiDi = SPRM_TBL(0x0B, 1, spra::operand_2b_2); // 0x560B
const sal_uInt16 sprmTDefTableShd3rd = SPRM_TBL(0x0C, 1, spra::operand_varlen_6); // 0xD60C
const sal_uInt16 sprmTPc = SPRM_TBL(0x0D, 1, spra::operand_1b_1); // 0x360D
const sal_uInt16 sprmTDxaAbs = SPRM_TBL(0x0E, 0, spra::operand_2b_4); // 0x940E
const sal_uInt16 sprmTDyaAbs = SPRM_TBL(0x0F, 0, spra::operand_2b_4); // 0x940F
const sal_uInt16 sprmTDxaFromText = SPRM_TBL(0x10, 0, spra::operand_2b_4); // 0x9410
const sal_uInt16 sprmTDyaFromText = SPRM_TBL(0x11, 0, spra::operand_2b_4); // 0x9411
const sal_uInt16 sprmTDefTableShd = SPRM_TBL(0x12, 1, spra::operand_varlen_6); // 0xD612
const sal_uInt16 sprmTTableBorders = SPRM_TBL(0x13, 1, spra::operand_varlen_6); // 0xD613
const sal_uInt16 sprmTTableWidth = SPRM_TBL(0x14, 1, spra::operand_3b_7); // 0xF614
const sal_uInt16 sprmTFAutofit = SPRM_TBL(0x15, 1, spra::operand_1b_1); // 0x3615
const sal_uInt16 sprmTDefTableShd2nd = SPRM_TBL(0x16, 1, spra::operand_varlen_6); // 0xD616
const sal_uInt16 sprmTWidthBefore = SPRM_TBL(0x17, 1, spra::operand_3b_7); // 0xF617
const sal_uInt16 sprmTWidthAfter = SPRM_TBL(0x18, 1, spra::operand_3b_7); // 0xF618
const sal_uInt16 sprmTFKeepFollow = SPRM_TBL(0x19, 1, spra::operand_1b_1); // 0x3619
const sal_uInt16 sprmTBrcTopCv = SPRM_TBL(0x1A, 1, spra::operand_varlen_6); // 0xD61A
const sal_uInt16 sprmTBrcLeftCv = SPRM_TBL(0x1B, 1, spra::operand_varlen_6); // 0xD61B
const sal_uInt16 sprmTBrcBottomCv = SPRM_TBL(0x1C, 1, spra::operand_varlen_6); // 0xD61C
const sal_uInt16 sprmTBrcRightCv = SPRM_TBL(0x1D, 1, spra::operand_varlen_6); // 0xD61D
const sal_uInt16 sprmTDxaFromTextRight = SPRM_TBL(0x1E, 0, spra::operand_2b_4); // 0x941E
const sal_uInt16 sprmTDyaFromTextBottom = SPRM_TBL(0x1F, 0, spra::operand_2b_4); // 0x941F
const sal_uInt16 sprmTSetBrc80 = SPRM_TBL(0x20, 1, spra::operand_varlen_6); // 0xD620
const sal_uInt16 sprmTInsert = SPRM_TBL(0x21, 1, spra::operand_4b_3); // 0x7621
const sal_uInt16 sprmTDelete = SPRM_TBL(0x22, 1, spra::operand_2b_2); // 0x5622
const sal_uInt16 sprmTDxaCol = SPRM_TBL(0x23, 1, spra::operand_4b_3); // 0x7623
const sal_uInt16 sprmTMerge = SPRM_TBL(0x24, 1, spra::operand_2b_2); // 0x5624
const sal_uInt16 sprmTSplit = SPRM_TBL(0x25, 1, spra::operand_2b_2); // 0x5625
const sal_uInt16 sprmTTextFlow = SPRM_TBL(0x29, 1, spra::operand_4b_3); // 0x7629
const sal_uInt16 sprmTVertMerge = SPRM_TBL(0x2B, 1, spra::operand_varlen_6); // 0xD62B
const sal_uInt16 sprmTVertAlign = SPRM_TBL(0x2C, 1, spra::operand_varlen_6); // 0xD62C
const sal_uInt16 sprmTSetShd = SPRM_TBL(0x2D, 1, spra::operand_varlen_6); // 0xD62D
const sal_uInt16 sprmTSetShdOdd = SPRM_TBL(0x2E, 1, spra::operand_varlen_6); // 0xD62E
const sal_uInt16 sprmTSetBrc = SPRM_TBL(0x2F, 1, spra::operand_varlen_6); // 0xD62F
const sal_uInt16 sprmTCellPadding = SPRM_TBL(0x32, 1, spra::operand_varlen_6); // 0xD632
const sal_uInt16 sprmTCellSpacingDefault = SPRM_TBL(0x33, 1, spra::operand_varlen_6); // 0xD633
const sal_uInt16 sprmTCellPaddingDefault = SPRM_TBL(0x34, 1, spra::operand_varlen_6); // 0xD634
const sal_uInt16 sprmTCellWidth = SPRM_TBL(0x35, 1, spra::operand_varlen_6); // 0xD635
const sal_uInt16 sprmTFitText = SPRM_TBL(0x36, 1, spra::operand_3b_7); // 0xF636
const sal_uInt16 sprmTFCellNoWrap = SPRM_TBL(0x39, 1, spra::operand_varlen_6); // 0xD639
const sal_uInt16 sprmTIstd = SPRM_TBL(0x3A, 1, spra::operand_2b_2); // 0x563A
const sal_uInt16 sprmTCellPaddingStyle = SPRM_TBL(0x3E, 1, spra::operand_varlen_6); // 0xD63E
const sal_uInt16 sprmTCellFHideMark = SPRM_TBL(0x42, 1, spra::operand_varlen_6); // 0xD642
const sal_uInt16 sprmTSetShdTable = SPRM_TBL(0x60, 1, spra::operand_varlen_6); // 0xD660
const sal_uInt16 sprmTWidthIndent = SPRM_TBL(0x61, 1, spra::operand_3b_7); // 0xF661
const sal_uInt16 sprmTCellBrcType = SPRM_TBL(0x62, 1, spra::operand_varlen_6); // 0xD662
const sal_uInt16 sprmTFBiDi90 = SPRM_TBL(0x64, 1, spra::operand_2b_2); // 0x5664
const sal_uInt16 sprmTFNoAllowOverlap = SPRM_TBL(0x65, 0, spra::operand_1b_1); // 0x3465
const sal_uInt16 sprmTFCantSplit = SPRM_TBL(0x66, 0, spra::operand_1b_1); // 0x3466
const sal_uInt16 sprmTPropRMark = SPRM_TBL(0x67, 1, spra::operand_varlen_6); // 0xD667
const sal_uInt16 sprmTWall = SPRM_TBL(0x68, 1, spra::operand_1b_1); // 0x3668
const sal_uInt16 sprmTIpgp = SPRM_TBL(0x69, 0, spra::operand_4b_3); // 0x7469
const sal_uInt16 sprmTCnf = SPRM_TBL(0x6A, 1, spra::operand_varlen_6); // 0xD66A
const sal_uInt16 sprmTDefTableShdRaw = SPRM_TBL(0x70, 1, spra::operand_varlen_6); // 0xD670
const sal_uInt16 sprmTDefTableShdRaw2nd = SPRM_TBL(0x71, 1, spra::operand_varlen_6); // 0xD671
const sal_uInt16 sprmTDefTableShdRaw3rd = SPRM_TBL(0x72, 1, spra::operand_varlen_6); // 0xD672
const sal_uInt16 sprmTRsid = SPRM_TBL(0x79, 0, spra::operand_4b_3); // 0x7479
const sal_uInt16 sprmTCellVertAlignStyle = SPRM_TBL(0x7C, 0, spra::operand_1b_1); // 0x347C
const sal_uInt16 sprmTCellNoWrapStyle = SPRM_TBL(0x7D, 0, spra::operand_1b_1); // 0x347D
const sal_uInt16 sprmTCellBrcTopStyle = SPRM_TBL(0x7F, 0, spra::operand_varlen_6); // 0xD47F
const sal_uInt16 sprmTCellBrcBottomStyle = SPRM_TBL(0x80, 1, spra::operand_varlen_6); // 0xD680
const sal_uInt16 sprmTCellBrcLeftStyle = SPRM_TBL(0x81, 1, spra::operand_varlen_6); // 0xD681
const sal_uInt16 sprmTCellBrcRightStyle = SPRM_TBL(0x82, 1, spra::operand_varlen_6); // 0xD682
const sal_uInt16 sprmTCellBrcInsideHStyle = SPRM_TBL(0x83, 1, spra::operand_varlen_6); // 0xD683
const sal_uInt16 sprmTCellBrcInsideVStyle = SPRM_TBL(0x84, 1, spra::operand_varlen_6); // 0xD684
const sal_uInt16 sprmTCellBrcTL2BRStyle = SPRM_TBL(0x85, 1, spra::operand_varlen_6); // 0xD685
const sal_uInt16 sprmTCellBrcTR2BLStyle = SPRM_TBL(0x86, 1, spra::operand_varlen_6); // 0xD686
const sal_uInt16 sprmTCellShdStyle = SPRM_TBL(0x87, 1, spra::operand_varlen_6); // 0xD687
const sal_uInt16 sprmTCHorzBands = SPRM_TBL(0x88, 0, spra::operand_1b_1); // 0x3488
const sal_uInt16 sprmTCVertBands = SPRM_TBL(0x89, 0, spra::operand_1b_1); // 0x3489
const sal_uInt16 sprmTJc = SPRM_TBL(0x8A, 0, spra::operand_2b_2); // 0x548A

// [MS-DOC] - v20170112 Section 2.6.4
const sal_uInt16 sprmScnsPgn = SPRM_SEC(0x00, 0, spra::operand_1b_1); // 0x3000
const sal_uInt16 sprmSiHeadingPgn = SPRM_SEC(0x01, 0, spra::operand_1b_1); // 0x3001
const sal_uInt16 sprmSDxaColWidth = SPRM_SEC(0x03, 1, spra::operand_3b_7); // 0xF203
const sal_uInt16 sprmSDxaColSpacing = SPRM_SEC(0x04, 1, spra::operand_3b_7); // 0xF204
const sal_uInt16 sprmSFEvenlySpaced = SPRM_SEC(0x05, 0, spra::operand_1b_1); // 0x3005
const sal_uInt16 sprmSFProtected = SPRM_SEC(0x06, 0, spra::operand_1b_1); // 0x3006
const sal_uInt16 sprmSDmBinFirst = SPRM_SEC(0x07, 0, spra::operand_2b_2); // 0x5007
const sal_uInt16 sprmSDmBinOther = SPRM_SEC(0x08, 0, spra::operand_2b_2); // 0x5008
const sal_uInt16 sprmSBkc = SPRM_SEC(0x09, 0, spra::operand_1b_1); // 0x3009
const sal_uInt16 sprmSFTitlePage = SPRM_SEC(0x0A, 0, spra::operand_1b_1); // 0x300A
const sal_uInt16 sprmSCcolumns = SPRM_SEC(0x0B, 0, spra::operand_2b_2); // 0x500B
const sal_uInt16 sprmSDxaColumns = SPRM_SEC(0x0C, 0, spra::operand_2b_4); // 0x900C
const sal_uInt16 sprmSNfcPgn = SPRM_SEC(0x0E, 0, spra::operand_1b_1); // 0x300E
const sal_uInt16 sprmSFPgnRestart = SPRM_SEC(0x11, 0, spra::operand_1b_1); // 0x3011
const sal_uInt16 sprmSFEndnote = SPRM_SEC(0x12, 0, spra::operand_1b_1); // 0x3012
const sal_uInt16 sprmSLnc = SPRM_SEC(0x13, 0, spra::operand_1b_1); // 0x3013
const sal_uInt16 sprmSNLnnMod = SPRM_SEC(0x15, 0, spra::operand_2b_2); // 0x5015
const sal_uInt16 sprmSDxaLnn = SPRM_SEC(0x16, 0, spra::operand_2b_4); // 0x9016
const sal_uInt16 sprmSDyaHdrTop = SPRM_SEC(0x17, 0, spra::operand_2b_5); // 0xB017
const sal_uInt16 sprmSDyaHdrBottom = SPRM_SEC(0x18, 0, spra::operand_2b_5); // 0xB018
const sal_uInt16 sprmSLBetween = SPRM_SEC(0x19, 0, spra::operand_1b_1); // 0x3019
const sal_uInt16 sprmSVjc = SPRM_SEC(0x1A, 0, spra::operand_1b_1); // 0x301A
const sal_uInt16 sprmSLnnMin = SPRM_SEC(0x1B, 0, spra::operand_2b_2); // 0x501B
const sal_uInt16 sprmSPgnStart97 = SPRM_SEC(0x1C, 0, spra::operand_2b_2); // 0x501C
const sal_uInt16 sprmSBOrientation = SPRM_SEC(0x1D, 0, spra::operand_1b_1); // 0x301D
const sal_uInt16 sprmSXaPage = SPRM_SEC(0x1F, 0, spra::operand_2b_5); // 0xB01F
const sal_uInt16 sprmSYaPage = SPRM_SEC(0x20, 0, spra::operand_2b_5); // 0xB020
const sal_uInt16 sprmSDxaLeft = SPRM_SEC(0x21, 0, spra::operand_2b_5); // 0xB021
const sal_uInt16 sprmSDxaRight = SPRM_SEC(0x22, 0, spra::operand_2b_5); // 0xB022
const sal_uInt16 sprmSDyaTop = SPRM_SEC(0x23, 0, spra::operand_2b_4); // 0x9023
const sal_uInt16 sprmSDyaBottom = SPRM_SEC(0x24, 0, spra::operand_2b_4); // 0x9024
const sal_uInt16 sprmSDzaGutter = SPRM_SEC(0x25, 0, spra::operand_2b_5); // 0xB025
const sal_uInt16 sprmSDmPaperReq = SPRM_SEC(0x26, 0, spra::operand_2b_2); // 0x5026
const sal_uInt16 sprmSFBiDi = SPRM_SEC(0x28, 1, spra::operand_1b_1); // 0x3228
const sal_uInt16 sprmSFRTLGutter = SPRM_SEC(0x2A, 1, spra::operand_1b_1); // 0x322A
const sal_uInt16 sprmSBrcTop80 = SPRM_SEC(0x2B, 0, spra::operand_4b_3); // 0x702B
const sal_uInt16 sprmSBrcLeft80 = SPRM_SEC(0x2C, 0, spra::operand_4b_3); // 0x702C
const sal_uInt16 sprmSBrcBottom80 = SPRM_SEC(0x2D, 0, spra::operand_4b_3); // 0x702D
const sal_uInt16 sprmSBrcRight80 = SPRM_SEC(0x2E, 0, spra::operand_4b_3); // 0x702E
const sal_uInt16 sprmSPgbProp = SPRM_SEC(0x2F, 1, spra::operand_2b_2); // 0x522F
const sal_uInt16 sprmSDxtCharSpace = SPRM_SEC(0x30, 0, spra::operand_4b_3); // 0x7030
const sal_uInt16 sprmSDyaLinePitch = SPRM_SEC(0x31, 0, spra::operand_2b_4); // 0x9031
const sal_uInt16 sprmSClm = SPRM_SEC(0x32, 0, spra::operand_2b_2); // 0x5032
const sal_uInt16 sprmSTextFlow = SPRM_SEC(0x33, 0, spra::operand_2b_2); // 0x5033
const sal_uInt16 sprmSBrcTop = SPRM_SEC(0x34, 1, spra::operand_varlen_6); // 0xD234
const sal_uInt16 sprmSBrcLeft = SPRM_SEC(0x35, 1, spra::operand_varlen_6); // 0xD235
const sal_uInt16 sprmSBrcBottom = SPRM_SEC(0x36, 1, spra::operand_varlen_6); // 0xD236
const sal_uInt16 sprmSBrcRight = SPRM_SEC(0x37, 1, spra::operand_varlen_6); // 0xD237
const sal_uInt16 sprmSWall = SPRM_SEC(0x39, 1, spra::operand_1b_1); // 0x3239
const sal_uInt16 sprmSRsid = SPRM_SEC(0x3A, 0, spra::operand_4b_3); // 0x703A
const sal_uInt16 sprmSFpc = SPRM_SEC(0x3B, 0, spra::operand_1b_1); // 0x303B
const sal_uInt16 sprmSRncFtn = SPRM_SEC(0x3C, 0, spra::operand_1b_1); // 0x303C
const sal_uInt16 sprmSRncEdn = SPRM_SEC(0x3E, 0, spra::operand_1b_1); // 0x303E
const sal_uInt16 sprmSNFtn = SPRM_SEC(0x3F, 0, spra::operand_2b_2); // 0x503F
const sal_uInt16 sprmSNfcFtnRef = SPRM_SEC(0x40, 0, spra::operand_2b_2); // 0x5040
const sal_uInt16 sprmSNEdn = SPRM_SEC(0x41, 0, spra::operand_2b_2); // 0x5041
const sal_uInt16 sprmSNfcEdnRef = SPRM_SEC(0x42, 0, spra::operand_2b_2); // 0x5042
const sal_uInt16 sprmSPropRMark = SPRM_SEC(0x43, 1, spra::operand_varlen_6); // 0xD243
const sal_uInt16 sprmSPgnStart = SPRM_SEC(0x44, 0, spra::operand_4b_3); // 0x7044

// [MS-DOC] - v20170112 Section 2.6.5
const sal_uInt16 sprmPicBrcTop80 = SPRM_PIC(0x02, 0, spra::operand_4b_3); // 0x6C02
const sal_uInt16 sprmPicBrcLeft80 = SPRM_PIC(0x03, 0, spra::operand_4b_3); // 0x6C03
const sal_uInt16 sprmPicBrcBottom80 = SPRM_PIC(0x04, 0, spra::operand_4b_3); // 0x6C04
const sal_uInt16 sprmPicBrcRight80 = SPRM_PIC(0x05, 0, spra::operand_4b_3); // 0x6C05
const sal_uInt16 sprmPicBrcTop = SPRM_PIC(0x08, 1, spra::operand_varlen_6); // 0xCE08
const sal_uInt16 sprmPicBrcLeft = SPRM_PIC(0x09, 1, spra::operand_varlen_6); // 0xCE09
const sal_uInt16 sprmPicBrcBottom = SPRM_PIC(0x0A, 1, spra::operand_varlen_6); // 0xCE0A
const sal_uInt16 sprmPicBrcRight = SPRM_PIC(0x0B, 1, spra::operand_varlen_6); // 0xCE0B
}

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_SPRMIDS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
