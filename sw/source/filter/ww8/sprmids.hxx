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
enum class SGC
{
    paragraph = 1,
    character = 2,
    picture = 3,
    section = 4,
    table = 5
};
enum class SPRA
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

template <int ispmd, int fSpec, SGC sgc, SPRA spra> struct sprm
{
    static constexpr sal_uInt16 val = SPRM(ispmd, fSpec, sgc, spra);
    static constexpr int len()
    {
        switch (spra)
        {
            case SPRA::operand_toggle_1b_0:
            case SPRA::operand_1b_1:
                return 1;
            case SPRA::operand_2b_2:
            case SPRA::operand_2b_4:
            case SPRA::operand_2b_5:
                return 2;
            case SPRA::operand_4b_3:
                return 4;
            case SPRA::operand_varlen_6:
                return 0; // variable
            case SPRA::operand_3b_7:
                return 3;
        }
    }
    static constexpr bool varlen() { return (spra == SPRA::operand_varlen_6); }
};

template <int ispmd, int fSpec, SPRA spra> using sprmPar = sprm<ispmd, fSpec, SGC::paragraph, spra>;
template <int ispmd, int fSpec, SPRA spra> using sprmChr = sprm<ispmd, fSpec, SGC::character, spra>;
template <int ispmd, int fSpec, SPRA spra> using sprmPic = sprm<ispmd, fSpec, SGC::picture, spra>;
template <int ispmd, int fSpec, SPRA spra> using sprmSec = sprm<ispmd, fSpec, SGC::section, spra>;
template <int ispmd, int fSpec, SPRA spra> using sprmTbl = sprm<ispmd, fSpec, SGC::table, spra>;

// Each of the following NS_sprm::Foo corresponds to a sprmFoo in [MS-DOC].
// E.g., NS_sprm::CFRMarkDel is for sprmCFRMarkDel.

// [MS-DOC] - v20170112 Section 2.6.1
using CFRMarkDel = sprmChr<0x00, 0, SPRA::operand_toggle_1b_0>; // 0x0800
using CFRMarkIns = sprmChr<0x01, 0, SPRA::operand_toggle_1b_0>; // 0x0801
using CFFldVanish = sprmChr<0x02, 0, SPRA::operand_toggle_1b_0>; // 0x0802
using CPicLocation = sprmChr<0x03, 1, SPRA::operand_4b_3>; // 0x6A03
using CIbstRMark = sprmChr<0x04, 0, SPRA::operand_2b_2>; // 0x4804
using CDttmRMark = sprmChr<0x05, 0, SPRA::operand_4b_3>; // 0x6805
using CFData = sprmChr<0x06, 0, SPRA::operand_toggle_1b_0>; // 0x0806
using CIdslRMark = sprmChr<0x07, 0, SPRA::operand_2b_2>; // 0x4807
using CSymbol = sprmChr<0x09, 1, SPRA::operand_4b_3>; // 0x6A09
using CFOle2 = sprmChr<0x0A, 0, SPRA::operand_toggle_1b_0>; // 0x080A
using CHighlight = sprmChr<0x0C, 1, SPRA::operand_1b_1>; // 0x2A0C
using CFWebHidden = sprmChr<0x11, 0, SPRA::operand_toggle_1b_0>; // 0x0811
using CRsidProp = sprmChr<0x15, 0, SPRA::operand_4b_3>; // 0x6815
using CRsidText = sprmChr<0x16, 0, SPRA::operand_4b_3>; // 0x6816
using CRsidRMDel = sprmChr<0x17, 0, SPRA::operand_4b_3>; // 0x6817
using CFSpecVanish = sprmChr<0x18, 0, SPRA::operand_toggle_1b_0>; // 0x0818
using CFMathPr = sprmChr<0x1A, 0, SPRA::operand_varlen_6>; // 0xC81A
using CIstd = sprmChr<0x30, 1, SPRA::operand_2b_2>; // 0x4A30
using CIstdPermute = sprmChr<0x31, 1, SPRA::operand_varlen_6>; // 0xCA31
using CPlain = sprmChr<0x33, 1, SPRA::operand_1b_1>; // 0x2A33
using CKcd = sprmChr<0x34, 1, SPRA::operand_1b_1>; // 0x2A34
using CFBold = sprmChr<0x35, 0, SPRA::operand_toggle_1b_0>; // 0x0835
using CFItalic = sprmChr<0x36, 0, SPRA::operand_toggle_1b_0>; // 0x0836
using CFStrike = sprmChr<0x37, 0, SPRA::operand_toggle_1b_0>; // 0x0837
using CFOutline = sprmChr<0x38, 0, SPRA::operand_toggle_1b_0>; // 0x0838
using CFShadow = sprmChr<0x39, 0, SPRA::operand_toggle_1b_0>; // 0x0839
using CFSmallCaps = sprmChr<0x3A, 0, SPRA::operand_toggle_1b_0>; // 0x083A
using CFCaps = sprmChr<0x3B, 0, SPRA::operand_toggle_1b_0>; // 0x083B
using CFVanish = sprmChr<0x3C, 0, SPRA::operand_toggle_1b_0>; // 0x083C
using CKul = sprmChr<0x3E, 1, SPRA::operand_1b_1>; // 0x2A3E
using CDxaSpace = sprmChr<0x40, 0, SPRA::operand_2b_4>; // 0x8840
using CIco = sprmChr<0x42, 1, SPRA::operand_1b_1>; // 0x2A42
using CHps = sprmChr<0x43, 1, SPRA::operand_2b_2>; // 0x4A43
using CHpsPos = sprmChr<0x45, 0, SPRA::operand_2b_2>; // 0x4845
using CMajority = sprmChr<0x47, 1, SPRA::operand_varlen_6>; // 0xCA47
using CIss = sprmChr<0x48, 1, SPRA::operand_1b_1>; // 0x2A48
using CHpsKern = sprmChr<0x4B, 0, SPRA::operand_2b_2>; // 0x484B
using CHresi = sprmChr<0x4E, 0, SPRA::operand_2b_2>; // 0x484E
using CRgFtc0 = sprmChr<0x4F, 1, SPRA::operand_2b_2>; // 0x4A4F
using CRgFtc1 = sprmChr<0x50, 1, SPRA::operand_2b_2>; // 0x4A50
using CRgFtc2 = sprmChr<0x51, 1, SPRA::operand_2b_2>; // 0x4A51
using CCharScale = sprmChr<0x52, 0, SPRA::operand_2b_2>; // 0x4852
using CFDStrike = sprmChr<0x53, 1, SPRA::operand_1b_1>; // 0x2A53
using CFImprint = sprmChr<0x54, 0, SPRA::operand_toggle_1b_0>; // 0x0854
using CFSpec = sprmChr<0x55, 0, SPRA::operand_toggle_1b_0>; // 0x0855
using CFObj = sprmChr<0x56, 0, SPRA::operand_toggle_1b_0>; // 0x0856
using CPropRMark90 = sprmChr<0x57, 1, SPRA::operand_varlen_6>; // 0xCA57
using CFEmboss = sprmChr<0x58, 0, SPRA::operand_toggle_1b_0>; // 0x0858
using CSfxText = sprmChr<0x59, 0, SPRA::operand_1b_1>; // 0x2859
using CFBiDi = sprmChr<0x5A, 0, SPRA::operand_toggle_1b_0>; // 0x085A
using CFBoldBi = sprmChr<0x5C, 0, SPRA::operand_toggle_1b_0>; // 0x085C
using CFItalicBi = sprmChr<0x5D, 0, SPRA::operand_toggle_1b_0>; // 0x085D
using CFtcBi = sprmChr<0x5E, 1, SPRA::operand_2b_2>; // 0x4A5E
using CLidBi = sprmChr<0x5F, 0, SPRA::operand_2b_2>; // 0x485F
using CIcoBi = sprmChr<0x60, 1, SPRA::operand_2b_2>; // 0x4A60
using CHpsBi = sprmChr<0x61, 1, SPRA::operand_2b_2>; // 0x4A61
using CDispFldRMark = sprmChr<0x62, 1, SPRA::operand_varlen_6>; // 0xCA62
using CIbstRMarkDel = sprmChr<0x63, 0, SPRA::operand_2b_2>; // 0x4863
using CDttmRMarkDel = sprmChr<0x64, 0, SPRA::operand_4b_3>; // 0x6864
using CBrc80 = sprmChr<0x65, 0, SPRA::operand_4b_3>; // 0x6865
using CShd80 = sprmChr<0x66, 0, SPRA::operand_2b_2>; // 0x4866
using CIdslRMarkDel = sprmChr<0x67, 0, SPRA::operand_2b_2>; // 0x4867
using CFUsePgsuSettings = sprmChr<0x68, 0, SPRA::operand_toggle_1b_0>; // 0x0868
using CRgLid0_80 = sprmChr<0x6D, 0, SPRA::operand_2b_2>; // 0x486D
using CRgLid1_80 = sprmChr<0x6E, 0, SPRA::operand_2b_2>; // 0x486E
using CIdctHint = sprmChr<0x6F, 0, SPRA::operand_1b_1>; // 0x286F
using CCv = sprmChr<0x70, 0, SPRA::operand_4b_3>; // 0x6870
using CShd = sprmChr<0x71, 1, SPRA::operand_varlen_6>; // 0xCA71
using CBrc = sprmChr<0x72, 1, SPRA::operand_varlen_6>; // 0xCA72
using CRgLid0 = sprmChr<0x73, 0, SPRA::operand_2b_2>; // 0x4873
using CRgLid1 = sprmChr<0x74, 0, SPRA::operand_2b_2>; // 0x4874
using CFNoProof = sprmChr<0x75, 0, SPRA::operand_toggle_1b_0>; // 0x0875
using CFitText = sprmChr<0x76, 1, SPRA::operand_varlen_6>; // 0xCA76
using CCvUl = sprmChr<0x77, 0, SPRA::operand_4b_3>; // 0x6877
using CFELayout = sprmChr<0x78, 1, SPRA::operand_varlen_6>; // 0xCA78
using CLbcCRJ = sprmChr<0x79, 0, SPRA::operand_1b_1>; // 0x2879
using CFComplexScripts = sprmChr<0x82, 0, SPRA::operand_toggle_1b_0>; // 0x0882
using CWall = sprmChr<0x83, 1, SPRA::operand_1b_1>; // 0x2A83
using CCnf = sprmChr<0x85, 1, SPRA::operand_varlen_6>; // 0xCA85
using CNeedFontFixup = sprmChr<0x86, 1, SPRA::operand_1b_1>; // 0x2A86
using CPbiIBullet = sprmChr<0x87, 0, SPRA::operand_4b_3>; // 0x6887
using CPbiGrf = sprmChr<0x88, 0, SPRA::operand_2b_2>; // 0x4888
using CPropRMark = sprmChr<0x89, 1, SPRA::operand_varlen_6>; // 0xCA89
using CFSdtVanish = sprmChr<0x90, 1, SPRA::operand_1b_1>; // 0x2A90

// [MS-DOC] - v20170112 Section 2.6.2
using PIstd = sprmPar<0x00, 1, SPRA::operand_2b_2>; // 0x4600
using PIstdPermute = sprmPar<0x01, 1, SPRA::operand_varlen_6>; // 0xC601
using PIncLvl = sprmPar<0x02, 1, SPRA::operand_1b_1>; // 0x2602
using PJc80 = sprmPar<0x03, 0, SPRA::operand_1b_1>; // 0x2403
using PFKeep = sprmPar<0x05, 0, SPRA::operand_1b_1>; // 0x2405
using PFKeepFollow = sprmPar<0x06, 0, SPRA::operand_1b_1>; // 0x2406
using PFPageBreakBefore = sprmPar<0x07, 0, SPRA::operand_1b_1>; // 0x2407
using PIlvl = sprmPar<0x0A, 1, SPRA::operand_1b_1>; // 0x260A
using PIlfo = sprmPar<0x0B, 1, SPRA::operand_2b_2>; // 0x460B
using PFNoLineNumb = sprmPar<0x0C, 0, SPRA::operand_1b_1>; // 0x240C
using PChgTabsPapx = sprmPar<0x0D, 1, SPRA::operand_varlen_6>; // 0xC60D
using PDxaRight80 = sprmPar<0x0E, 0, SPRA::operand_2b_4>; // 0x840E
using PDxaLeft80 = sprmPar<0x0F, 0, SPRA::operand_2b_4>; // 0x840F
using PNest80 = sprmPar<0x10, 1, SPRA::operand_2b_2>; // 0x4610
using PDxaLeft180 = sprmPar<0x11, 0, SPRA::operand_2b_4>; // 0x8411
using PDyaLine = sprmPar<0x12, 0, SPRA::operand_4b_3>; // 0x6412
using PDyaBefore = sprmPar<0x13, 0, SPRA::operand_2b_5>; // 0xA413
using PDyaAfter = sprmPar<0x14, 0, SPRA::operand_2b_5>; // 0xA414
using PChgTabs = sprmPar<0x15, 1, SPRA::operand_varlen_6>; // 0xC615
using PFInTable = sprmPar<0x16, 0, SPRA::operand_1b_1>; // 0x2416
using PFTtp = sprmPar<0x17, 0, SPRA::operand_1b_1>; // 0x2417
using PDxaAbs = sprmPar<0x18, 0, SPRA::operand_2b_4>; // 0x8418
using PDyaAbs = sprmPar<0x19, 0, SPRA::operand_2b_4>; // 0x8419
using PDxaWidth = sprmPar<0x1A, 0, SPRA::operand_2b_4>; // 0x841A
using PPc = sprmPar<0x1B, 1, SPRA::operand_1b_1>; // 0x261B
using PWr = sprmPar<0x23, 0, SPRA::operand_1b_1>; // 0x2423
using PBrcTop80 = sprmPar<0x24, 0, SPRA::operand_4b_3>; // 0x6424
using PBrcLeft80 = sprmPar<0x25, 0, SPRA::operand_4b_3>; // 0x6425
using PBrcBottom80 = sprmPar<0x26, 0, SPRA::operand_4b_3>; // 0x6426
using PBrcRight80 = sprmPar<0x27, 0, SPRA::operand_4b_3>; // 0x6427
using PBrcBetween80 = sprmPar<0x28, 0, SPRA::operand_4b_3>; // 0x6428
using PBrcBar80 = sprmPar<0x29, 1, SPRA::operand_4b_3>; // 0x6629
using PFNoAutoHyph = sprmPar<0x2A, 0, SPRA::operand_1b_1>; // 0x242A
using PWHeightAbs = sprmPar<0x2B, 0, SPRA::operand_2b_2>; // 0x442B
using PDcs = sprmPar<0x2C, 0, SPRA::operand_2b_2>; // 0x442C
using PShd80 = sprmPar<0x2D, 0, SPRA::operand_2b_2>; // 0x442D
using PDyaFromText = sprmPar<0x2E, 0, SPRA::operand_2b_4>; // 0x842E
using PDxaFromText = sprmPar<0x2F, 0, SPRA::operand_2b_4>; // 0x842F
using PFLocked = sprmPar<0x30, 0, SPRA::operand_1b_1>; // 0x2430
using PFWidowControl = sprmPar<0x31, 0, SPRA::operand_1b_1>; // 0x2431
using PFKinsoku = sprmPar<0x33, 0, SPRA::operand_1b_1>; // 0x2433
using PFWordWrap = sprmPar<0x34, 0, SPRA::operand_1b_1>; // 0x2434
using PFOverflowPunct = sprmPar<0x35, 0, SPRA::operand_1b_1>; // 0x2435
using PFTopLinePunct = sprmPar<0x36, 0, SPRA::operand_1b_1>; // 0x2436
using PFAutoSpaceDE = sprmPar<0x37, 0, SPRA::operand_1b_1>; // 0x2437
using PFAutoSpaceDN = sprmPar<0x38, 0, SPRA::operand_1b_1>; // 0x2438
using PWAlignFont = sprmPar<0x39, 0, SPRA::operand_2b_2>; // 0x4439
using PFrameTextFlow = sprmPar<0x3A, 0, SPRA::operand_2b_2>; // 0x443A
using POutLvl = sprmPar<0x40, 1, SPRA::operand_1b_1>; // 0x2640
using PFBiDi = sprmPar<0x41, 0, SPRA::operand_1b_1>; // 0x2441
using PFNumRMIns = sprmPar<0x43, 0, SPRA::operand_1b_1>; // 0x2443
using PNumRM = sprmPar<0x45, 1, SPRA::operand_varlen_6>; // 0xC645
using PHugePapx = sprmPar<0x46, 1, SPRA::operand_4b_3>; // 0x6646
using PFUsePgsuSettings = sprmPar<0x47, 0, SPRA::operand_1b_1>; // 0x2447
using PFAdjustRight = sprmPar<0x48, 0, SPRA::operand_1b_1>; // 0x2448
using PItap = sprmPar<0x49, 1, SPRA::operand_4b_3>; // 0x6649
using PDtap = sprmPar<0x4A, 1, SPRA::operand_4b_3>; // 0x664A
using PFInnerTableCell = sprmPar<0x4B, 0, SPRA::operand_1b_1>; // 0x244B
using PFInnerTtp = sprmPar<0x4C, 0, SPRA::operand_1b_1>; // 0x244C
using PShd = sprmPar<0x4D, 1, SPRA::operand_varlen_6>; // 0xC64D
using PBrcTop = sprmPar<0x4E, 1, SPRA::operand_varlen_6>; // 0xC64E
using PBrcLeft = sprmPar<0x4F, 1, SPRA::operand_varlen_6>; // 0xC64F
using PBrcBottom = sprmPar<0x50, 1, SPRA::operand_varlen_6>; // 0xC650
using PBrcRight = sprmPar<0x51, 1, SPRA::operand_varlen_6>; // 0xC651
using PBrcBetween = sprmPar<0x52, 1, SPRA::operand_varlen_6>; // 0xC652
using PBrcBar = sprmPar<0x53, 1, SPRA::operand_varlen_6>; // 0xC653
using PDxcRight = sprmPar<0x55, 0, SPRA::operand_2b_2>; // 0x4455
using PDxcLeft = sprmPar<0x56, 0, SPRA::operand_2b_2>; // 0x4456
using PDxcLeft1 = sprmPar<0x57, 0, SPRA::operand_2b_2>; // 0x4457
using PDylBefore = sprmPar<0x58, 0, SPRA::operand_2b_2>; // 0x4458
using PDylAfter = sprmPar<0x59, 0, SPRA::operand_2b_2>; // 0x4459
using PFOpenTch = sprmPar<0x5A, 0, SPRA::operand_1b_1>; // 0x245A
using PFDyaBeforeAuto = sprmPar<0x5B, 0, SPRA::operand_1b_1>; // 0x245B
using PFDyaAfterAuto = sprmPar<0x5C, 0, SPRA::operand_1b_1>; // 0x245C
using PDxaRight = sprmPar<0x5D, 0, SPRA::operand_2b_4>; // 0x845D
using PDxaLeft = sprmPar<0x5E, 0, SPRA::operand_2b_4>; // 0x845E
using PNest = sprmPar<0x5F, 1, SPRA::operand_2b_2>; // 0x465F
using PDxaLeft1 = sprmPar<0x60, 0, SPRA::operand_2b_4>; // 0x8460
using PJc = sprmPar<0x61, 0, SPRA::operand_1b_1>; // 0x2461
using PFNoAllowOverlap = sprmPar<0x62, 0, SPRA::operand_1b_1>; // 0x2462
using PWall = sprmPar<0x64, 1, SPRA::operand_1b_1>; // 0x2664
using PIpgp = sprmPar<0x65, 0, SPRA::operand_4b_3>; // 0x6465
using PCnf = sprmPar<0x66, 1, SPRA::operand_varlen_6>; // 0xC666
using PRsid = sprmPar<0x67, 0, SPRA::operand_4b_3>; // 0x6467
using PIstdListPermute = sprmPar<0x69, 1, SPRA::operand_varlen_6>; // 0xC669
using PTableProps = sprmPar<0x6B, 0, SPRA::operand_4b_3>; // 0x646B
using PTIstdInfo = sprmPar<0x6C, 1, SPRA::operand_varlen_6>; // 0xC66C
using PFContextualSpacing = sprmPar<0x6D, 0, SPRA::operand_1b_1>; // 0x246D
using PPropRMark = sprmPar<0x6F, 1, SPRA::operand_varlen_6>; // 0xC66F
using PFMirrorIndents = sprmPar<0x70, 0, SPRA::operand_1b_1>; // 0x2470
using PTtwo = sprmPar<0x71, 0, SPRA::operand_1b_1>; // 0x2471

// [MS-DOC] - v20170112 Section 2.6.3
using TJc90 = sprmTbl<0x00, 0, SPRA::operand_2b_2>; // 0x5400
using TDxaLeft = sprmTbl<0x01, 1, SPRA::operand_2b_4>; // 0x9601
using TDxaGapHalf = sprmTbl<0x02, 1, SPRA::operand_2b_4>; // 0x9602
using TFCantSplit90 = sprmTbl<0x03, 0, SPRA::operand_1b_1>; // 0x3403
using TTableHeader = sprmTbl<0x04, 0, SPRA::operand_1b_1>; // 0x3404
using TTableBorders80 = sprmTbl<0x05, 1, SPRA::operand_varlen_6>; // 0xD605
using TDyaRowHeight = sprmTbl<0x07, 0, SPRA::operand_2b_4>; // 0x9407
using TDefTable = sprmTbl<0x08, 1, SPRA::operand_varlen_6>; // 0xD608
using TDefTableShd80 = sprmTbl<0x09, 1, SPRA::operand_varlen_6>; // 0xD609
using TTlp = sprmTbl<0x0A, 0, SPRA::operand_4b_3>; // 0x740A
using TFBiDi = sprmTbl<0x0B, 1, SPRA::operand_2b_2>; // 0x560B
using TDefTableShd3rd = sprmTbl<0x0C, 1, SPRA::operand_varlen_6>; // 0xD60C
using TPc = sprmTbl<0x0D, 1, SPRA::operand_1b_1>; // 0x360D
using TDxaAbs = sprmTbl<0x0E, 0, SPRA::operand_2b_4>; // 0x940E
using TDyaAbs = sprmTbl<0x0F, 0, SPRA::operand_2b_4>; // 0x940F
using TDxaFromText = sprmTbl<0x10, 0, SPRA::operand_2b_4>; // 0x9410
using TDyaFromText = sprmTbl<0x11, 0, SPRA::operand_2b_4>; // 0x9411
using TDefTableShd = sprmTbl<0x12, 1, SPRA::operand_varlen_6>; // 0xD612
using TTableBorders = sprmTbl<0x13, 1, SPRA::operand_varlen_6>; // 0xD613
using TTableWidth = sprmTbl<0x14, 1, SPRA::operand_3b_7>; // 0xF614
using TFAutofit = sprmTbl<0x15, 1, SPRA::operand_1b_1>; // 0x3615
using TDefTableShd2nd = sprmTbl<0x16, 1, SPRA::operand_varlen_6>; // 0xD616
using TWidthBefore = sprmTbl<0x17, 1, SPRA::operand_3b_7>; // 0xF617
using TWidthAfter = sprmTbl<0x18, 1, SPRA::operand_3b_7>; // 0xF618
using TFKeepFollow = sprmTbl<0x19, 1, SPRA::operand_1b_1>; // 0x3619
using TBrcTopCv = sprmTbl<0x1A, 1, SPRA::operand_varlen_6>; // 0xD61A
using TBrcLeftCv = sprmTbl<0x1B, 1, SPRA::operand_varlen_6>; // 0xD61B
using TBrcBottomCv = sprmTbl<0x1C, 1, SPRA::operand_varlen_6>; // 0xD61C
using TBrcRightCv = sprmTbl<0x1D, 1, SPRA::operand_varlen_6>; // 0xD61D
using TDxaFromTextRight = sprmTbl<0x1E, 0, SPRA::operand_2b_4>; // 0x941E
using TDyaFromTextBottom = sprmTbl<0x1F, 0, SPRA::operand_2b_4>; // 0x941F
using TSetBrc80 = sprmTbl<0x20, 1, SPRA::operand_varlen_6>; // 0xD620
using TInsert = sprmTbl<0x21, 1, SPRA::operand_4b_3>; // 0x7621
using TDelete = sprmTbl<0x22, 1, SPRA::operand_2b_2>; // 0x5622
using TDxaCol = sprmTbl<0x23, 1, SPRA::operand_4b_3>; // 0x7623
using TMerge = sprmTbl<0x24, 1, SPRA::operand_2b_2>; // 0x5624
using TSplit = sprmTbl<0x25, 1, SPRA::operand_2b_2>; // 0x5625
using TTextFlow = sprmTbl<0x29, 1, SPRA::operand_4b_3>; // 0x7629
using TVertMerge = sprmTbl<0x2B, 1, SPRA::operand_varlen_6>; // 0xD62B
using TVertAlign = sprmTbl<0x2C, 1, SPRA::operand_varlen_6>; // 0xD62C
using TSetShd = sprmTbl<0x2D, 1, SPRA::operand_varlen_6>; // 0xD62D
using TSetShdOdd = sprmTbl<0x2E, 1, SPRA::operand_varlen_6>; // 0xD62E
using TSetBrc = sprmTbl<0x2F, 1, SPRA::operand_varlen_6>; // 0xD62F
using TCellPadding = sprmTbl<0x32, 1, SPRA::operand_varlen_6>; // 0xD632
using TCellSpacingDefault = sprmTbl<0x33, 1, SPRA::operand_varlen_6>; // 0xD633
using TCellPaddingDefault = sprmTbl<0x34, 1, SPRA::operand_varlen_6>; // 0xD634
using TCellWidth = sprmTbl<0x35, 1, SPRA::operand_varlen_6>; // 0xD635
using TFitText = sprmTbl<0x36, 1, SPRA::operand_3b_7>; // 0xF636
using TFCellNoWrap = sprmTbl<0x39, 1, SPRA::operand_varlen_6>; // 0xD639
using TIstd = sprmTbl<0x3A, 1, SPRA::operand_2b_2>; // 0x563A
using TCellPaddingStyle = sprmTbl<0x3E, 1, SPRA::operand_varlen_6>; // 0xD63E
using TCellFHideMark = sprmTbl<0x42, 1, SPRA::operand_varlen_6>; // 0xD642
using TSetShdTable = sprmTbl<0x60, 1, SPRA::operand_varlen_6>; // 0xD660
using TWidthIndent = sprmTbl<0x61, 1, SPRA::operand_3b_7>; // 0xF661
using TCellBrcType = sprmTbl<0x62, 1, SPRA::operand_varlen_6>; // 0xD662
using TFBiDi90 = sprmTbl<0x64, 1, SPRA::operand_2b_2>; // 0x5664
using TFNoAllowOverlap = sprmTbl<0x65, 0, SPRA::operand_1b_1>; // 0x3465
using TFCantSplit = sprmTbl<0x66, 0, SPRA::operand_1b_1>; // 0x3466
using TPropRMark = sprmTbl<0x67, 1, SPRA::operand_varlen_6>; // 0xD667
using TWall = sprmTbl<0x68, 1, SPRA::operand_1b_1>; // 0x3668
using TIpgp = sprmTbl<0x69, 0, SPRA::operand_4b_3>; // 0x7469
using TCnf = sprmTbl<0x6A, 1, SPRA::operand_varlen_6>; // 0xD66A
using TDefTableShdRaw = sprmTbl<0x70, 1, SPRA::operand_varlen_6>; // 0xD670
using TDefTableShdRaw2nd = sprmTbl<0x71, 1, SPRA::operand_varlen_6>; // 0xD671
using TDefTableShdRaw3rd = sprmTbl<0x72, 1, SPRA::operand_varlen_6>; // 0xD672
using TRsid = sprmTbl<0x79, 0, SPRA::operand_4b_3>; // 0x7479
using TCellVertAlignStyle = sprmTbl<0x7C, 0, SPRA::operand_1b_1>; // 0x347C
using TCellNoWrapStyle = sprmTbl<0x7D, 0, SPRA::operand_1b_1>; // 0x347D
using TCellBrcTopStyle = sprmTbl<0x7F, 0, SPRA::operand_varlen_6>; // 0xD47F
using TCellBrcBottomStyle = sprmTbl<0x80, 1, SPRA::operand_varlen_6>; // 0xD680
using TCellBrcLeftStyle = sprmTbl<0x81, 1, SPRA::operand_varlen_6>; // 0xD681
using TCellBrcRightStyle = sprmTbl<0x82, 1, SPRA::operand_varlen_6>; // 0xD682
using TCellBrcInsideHStyle = sprmTbl<0x83, 1, SPRA::operand_varlen_6>; // 0xD683
using TCellBrcInsideVStyle = sprmTbl<0x84, 1, SPRA::operand_varlen_6>; // 0xD684
using TCellBrcTL2BRStyle = sprmTbl<0x85, 1, SPRA::operand_varlen_6>; // 0xD685
using TCellBrcTR2BLStyle = sprmTbl<0x86, 1, SPRA::operand_varlen_6>; // 0xD686
using TCellShdStyle = sprmTbl<0x87, 1, SPRA::operand_varlen_6>; // 0xD687
using TCHorzBands = sprmTbl<0x88, 0, SPRA::operand_1b_1>; // 0x3488
using TCVertBands = sprmTbl<0x89, 0, SPRA::operand_1b_1>; // 0x3489
using TJc = sprmTbl<0x8A, 0, SPRA::operand_2b_2>; // 0x548A

// [MS-DOC] - v20170112 Section 2.6.4
using ScnsPgn = sprmSec<0x00, 0, SPRA::operand_1b_1>; // 0x3000
using SiHeadingPgn = sprmSec<0x01, 0, SPRA::operand_1b_1>; // 0x3001
using SDxaColWidth = sprmSec<0x03, 1, SPRA::operand_3b_7>; // 0xF203
using SDxaColSpacing = sprmSec<0x04, 1, SPRA::operand_3b_7>; // 0xF204
using SFEvenlySpaced = sprmSec<0x05, 0, SPRA::operand_1b_1>; // 0x3005
using SFProtected = sprmSec<0x06, 0, SPRA::operand_1b_1>; // 0x3006
using SDmBinFirst = sprmSec<0x07, 0, SPRA::operand_2b_2>; // 0x5007
using SDmBinOther = sprmSec<0x08, 0, SPRA::operand_2b_2>; // 0x5008
using SBkc = sprmSec<0x09, 0, SPRA::operand_1b_1>; // 0x3009
using SFTitlePage = sprmSec<0x0A, 0, SPRA::operand_1b_1>; // 0x300A
using SCcolumns = sprmSec<0x0B, 0, SPRA::operand_2b_2>; // 0x500B
using SDxaColumns = sprmSec<0x0C, 0, SPRA::operand_2b_4>; // 0x900C
using SNfcPgn = sprmSec<0x0E, 0, SPRA::operand_1b_1>; // 0x300E
using SFPgnRestart = sprmSec<0x11, 0, SPRA::operand_1b_1>; // 0x3011
using SFEndnote = sprmSec<0x12, 0, SPRA::operand_1b_1>; // 0x3012
using SLnc = sprmSec<0x13, 0, SPRA::operand_1b_1>; // 0x3013
using SNLnnMod = sprmSec<0x15, 0, SPRA::operand_2b_2>; // 0x5015
using SDxaLnn = sprmSec<0x16, 0, SPRA::operand_2b_4>; // 0x9016
using SDyaHdrTop = sprmSec<0x17, 0, SPRA::operand_2b_5>; // 0xB017
using SDyaHdrBottom = sprmSec<0x18, 0, SPRA::operand_2b_5>; // 0xB018
using SLBetween = sprmSec<0x19, 0, SPRA::operand_1b_1>; // 0x3019
using SVjc = sprmSec<0x1A, 0, SPRA::operand_1b_1>; // 0x301A
using SLnnMin = sprmSec<0x1B, 0, SPRA::operand_2b_2>; // 0x501B
using SPgnStart97 = sprmSec<0x1C, 0, SPRA::operand_2b_2>; // 0x501C
using SBOrientation = sprmSec<0x1D, 0, SPRA::operand_1b_1>; // 0x301D
using SXaPage = sprmSec<0x1F, 0, SPRA::operand_2b_5>; // 0xB01F
using SYaPage = sprmSec<0x20, 0, SPRA::operand_2b_5>; // 0xB020
using SDxaLeft = sprmSec<0x21, 0, SPRA::operand_2b_5>; // 0xB021
using SDxaRight = sprmSec<0x22, 0, SPRA::operand_2b_5>; // 0xB022
using SDyaTop = sprmSec<0x23, 0, SPRA::operand_2b_4>; // 0x9023
using SDyaBottom = sprmSec<0x24, 0, SPRA::operand_2b_4>; // 0x9024
using SDzaGutter = sprmSec<0x25, 0, SPRA::operand_2b_5>; // 0xB025
using SDmPaperReq = sprmSec<0x26, 0, SPRA::operand_2b_2>; // 0x5026
using SFBiDi = sprmSec<0x28, 1, SPRA::operand_1b_1>; // 0x3228
using SFRTLGutter = sprmSec<0x2A, 1, SPRA::operand_1b_1>; // 0x322A
using SBrcTop80 = sprmSec<0x2B, 0, SPRA::operand_4b_3>; // 0x702B
using SBrcLeft80 = sprmSec<0x2C, 0, SPRA::operand_4b_3>; // 0x702C
using SBrcBottom80 = sprmSec<0x2D, 0, SPRA::operand_4b_3>; // 0x702D
using SBrcRight80 = sprmSec<0x2E, 0, SPRA::operand_4b_3>; // 0x702E
using SPgbProp = sprmSec<0x2F, 1, SPRA::operand_2b_2>; // 0x522F
using SDxtCharSpace = sprmSec<0x30, 0, SPRA::operand_4b_3>; // 0x7030
using SDyaLinePitch = sprmSec<0x31, 0, SPRA::operand_2b_4>; // 0x9031
using SClm = sprmSec<0x32, 0, SPRA::operand_2b_2>; // 0x5032
using STextFlow = sprmSec<0x33, 0, SPRA::operand_2b_2>; // 0x5033
using SBrcTop = sprmSec<0x34, 1, SPRA::operand_varlen_6>; // 0xD234
using SBrcLeft = sprmSec<0x35, 1, SPRA::operand_varlen_6>; // 0xD235
using SBrcBottom = sprmSec<0x36, 1, SPRA::operand_varlen_6>; // 0xD236
using SBrcRight = sprmSec<0x37, 1, SPRA::operand_varlen_6>; // 0xD237
using SWall = sprmSec<0x39, 1, SPRA::operand_1b_1>; // 0x3239
using SRsid = sprmSec<0x3A, 0, SPRA::operand_4b_3>; // 0x703A
using SFpc = sprmSec<0x3B, 0, SPRA::operand_1b_1>; // 0x303B
using SRncFtn = sprmSec<0x3C, 0, SPRA::operand_1b_1>; // 0x303C
using SRncEdn = sprmSec<0x3E, 0, SPRA::operand_1b_1>; // 0x303E
using SNFtn = sprmSec<0x3F, 0, SPRA::operand_2b_2>; // 0x503F
using SNfcFtnRef = sprmSec<0x40, 0, SPRA::operand_2b_2>; // 0x5040
using SNEdn = sprmSec<0x41, 0, SPRA::operand_2b_2>; // 0x5041
using SNfcEdnRef = sprmSec<0x42, 0, SPRA::operand_2b_2>; // 0x5042
using SPropRMark = sprmSec<0x43, 1, SPRA::operand_varlen_6>; // 0xD243
using SPgnStart = sprmSec<0x44, 0, SPRA::operand_4b_3>; // 0x7044

// [MS-DOC] - v20170112 Section 2.6.5
using PicBrcTop80 = sprmPic<0x02, 0, SPRA::operand_4b_3>; // 0x6C02
using PicBrcLeft80 = sprmPic<0x03, 0, SPRA::operand_4b_3>; // 0x6C03
using PicBrcBottom80 = sprmPic<0x04, 0, SPRA::operand_4b_3>; // 0x6C04
using PicBrcRight80 = sprmPic<0x05, 0, SPRA::operand_4b_3>; // 0x6C05
using PicBrcTop = sprmPic<0x08, 1, SPRA::operand_varlen_6>; // 0xCE08
using PicBrcLeft = sprmPic<0x09, 1, SPRA::operand_varlen_6>; // 0xCE09
using PicBrcBottom = sprmPic<0x0A, 1, SPRA::operand_varlen_6>; // 0xCE0A
using PicBrcRight = sprmPic<0x0B, 1, SPRA::operand_varlen_6>; // 0xCE0B
}

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_SPRMIDS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
