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

// [MS-DOC] - v20170112 Section 2.6.1
const sal_uInt16 sprmCFRMarkDel        = 0x0800;
const sal_uInt16 sprmCFRMarkIns        = 0x0801;
const sal_uInt16 sprmCFFldVanish       = 0x0802;
const sal_uInt16 sprmCPicLocation      = 0x6A03;
const sal_uInt16 sprmCIbstRMark        = 0x4804;
const sal_uInt16 sprmCDttmRMark        = 0x6805;
const sal_uInt16 sprmCFData            = 0x0806;
const sal_uInt16 sprmCIdslRMark        = 0x4807;
const sal_uInt16 sprmCSymbol           = 0x6A09;
const sal_uInt16 sprmCFOle2            = 0x080A;
const sal_uInt16 sprmCHighlight        = 0x2A0C;
const sal_uInt16 sprmCFWebHidden       = 0x0811;
const sal_uInt16 sprmCRsidProp         = 0x6815;
const sal_uInt16 sprmCRsidText         = 0x6816;
const sal_uInt16 sprmCRsidRMDel        = 0x6817;
const sal_uInt16 sprmCFSpecVanish      = 0x0818;
const sal_uInt16 sprmCFMathPr          = 0xC81A;
const sal_uInt16 sprmCIstd             = 0x4A30;
const sal_uInt16 sprmCIstdPermute      = 0xCA31;
const sal_uInt16 sprmCPlain            = 0x2A33;
const sal_uInt16 sprmCKcd              = 0x2A34;
const sal_uInt16 sprmCFBold            = 0x0835;
const sal_uInt16 sprmCFItalic          = 0x0836;
const sal_uInt16 sprmCFStrike          = 0x0837;
const sal_uInt16 sprmCFOutline         = 0x0838;
const sal_uInt16 sprmCFShadow          = 0x0839;
const sal_uInt16 sprmCFSmallCaps       = 0x083A;
const sal_uInt16 sprmCFCaps            = 0x083B;
const sal_uInt16 sprmCFVanish          = 0x083C;
const sal_uInt16 sprmCKul              = 0x2A3E;
const sal_uInt16 sprmCDxaSpace         = 0x8840;
const sal_uInt16 sprmCIco              = 0x2A42;
const sal_uInt16 sprmCHps              = 0x4A43;
const sal_uInt16 sprmCHpsPos           = 0x4845;
const sal_uInt16 sprmCMajority         = 0xCA47;
const sal_uInt16 sprmCIss              = 0x2A48;
const sal_uInt16 sprmCHpsKern          = 0x484B;
const sal_uInt16 sprmCHresi            = 0x484E;
const sal_uInt16 sprmCRgFtc0           = 0x4A4F;
const sal_uInt16 sprmCRgFtc1           = 0x4A50;
const sal_uInt16 sprmCRgFtc2           = 0x4A51;
const sal_uInt16 sprmCCharScale        = 0x4852;
const sal_uInt16 sprmCFDStrike         = 0x2A53;
const sal_uInt16 sprmCFImprint         = 0x0854;
const sal_uInt16 sprmCFSpec            = 0x0855;
const sal_uInt16 sprmCFObj             = 0x0856;
const sal_uInt16 sprmCPropRMark90      = 0xCA57;
const sal_uInt16 sprmCFEmboss          = 0x0858;
const sal_uInt16 sprmCSfxText          = 0x2859;
const sal_uInt16 sprmCFBiDi            = 0x085A;
const sal_uInt16 sprmCFBoldBi          = 0x085C;
const sal_uInt16 sprmCFItalicBi        = 0x085D;
const sal_uInt16 sprmCFtcBi            = 0x4A5E;
const sal_uInt16 sprmCLidBi            = 0x485F;
const sal_uInt16 sprmCIcoBi            = 0x4A60;
const sal_uInt16 sprmCHpsBi            = 0x4A61;
const sal_uInt16 sprmCDispFldRMark     = 0xCA62;
const sal_uInt16 sprmCIbstRMarkDel     = 0x4863;
const sal_uInt16 sprmCDttmRMarkDel     = 0x6864;
const sal_uInt16 sprmCBrc80            = 0x6865;
const sal_uInt16 sprmCShd80            = 0x4866;
const sal_uInt16 sprmCIdslRMarkDel     = 0x4867;
const sal_uInt16 sprmCFUsePgsuSettings = 0x0868;
const sal_uInt16 sprmCRgLid0_80        = 0x486D;
const sal_uInt16 sprmCRgLid1_80        = 0x486E;
const sal_uInt16 sprmCIdctHint         = 0x286F;
const sal_uInt16 sprmCCv               = 0x6870;
const sal_uInt16 sprmCShd              = 0xCA71;
const sal_uInt16 sprmCBrc              = 0xCA72;
const sal_uInt16 sprmCRgLid0           = 0x4873;
const sal_uInt16 sprmCRgLid1           = 0x4874;
const sal_uInt16 sprmCFNoProof         = 0x0875;
const sal_uInt16 sprmCFitText          = 0xCA76;
const sal_uInt16 sprmCCvUl             = 0x6877;
const sal_uInt16 sprmCFELayout         = 0xCA78;
const sal_uInt16 sprmCLbcCRJ           = 0x2879;
const sal_uInt16 sprmCFComplexScripts  = 0x0882;
const sal_uInt16 sprmCWall             = 0x2A83;
const sal_uInt16 sprmCCnf              = 0xCA85;
const sal_uInt16 sprmCNeedFontFixup    = 0x2A86;
const sal_uInt16 sprmCPbiIBullet       = 0x6887;
const sal_uInt16 sprmCPbiGrf           = 0x4888;
const sal_uInt16 sprmCPropRMark        = 0xCA89;
const sal_uInt16 sprmCFSdtVanish       = 0x2A90;

// [MS-DOC] - v20170112 Section 2.6.2
const sal_uInt16 sprmPIstd               = 0x4600;
const sal_uInt16 sprmPIstdPermute        = 0xC601;
const sal_uInt16 sprmPIncLvl             = 0x2602;
const sal_uInt16 sprmPJc80               = 0x2403;
const sal_uInt16 sprmPFKeep              = 0x2405;
const sal_uInt16 sprmPFKeepFollow        = 0x2406;
const sal_uInt16 sprmPFPageBreakBefore   = 0x2407;
const sal_uInt16 sprmPIlvl               = 0x260A;
const sal_uInt16 sprmPIlfo               = 0x460B;
const sal_uInt16 sprmPFNoLineNumb        = 0x240C;
const sal_uInt16 sprmPChgTabsPapx        = 0xC60D;
const sal_uInt16 sprmPDxaRight80         = 0x840E;
const sal_uInt16 sprmPDxaLeft80          = 0x840F;
const sal_uInt16 sprmPNest80             = 0x4610;
const sal_uInt16 sprmPDxaLeft180         = 0x8411;
const sal_uInt16 sprmPDyaLine            = 0x6412;
const sal_uInt16 sprmPDyaBefore          = 0xA413;
const sal_uInt16 sprmPDyaAfter           = 0xA414;
const sal_uInt16 sprmPChgTabs            = 0xC615;
const sal_uInt16 sprmPFInTable           = 0x2416;
const sal_uInt16 sprmPFTtp               = 0x2417;
const sal_uInt16 sprmPDxaAbs             = 0x8418;
const sal_uInt16 sprmPDyaAbs             = 0x8419;
const sal_uInt16 sprmPDxaWidth           = 0x841A;
const sal_uInt16 sprmPPc                 = 0x261B;
const sal_uInt16 sprmPWr                 = 0x2423;
const sal_uInt16 sprmPBrcTop80           = 0x6424;
const sal_uInt16 sprmPBrcLeft80          = 0x6425;
const sal_uInt16 sprmPBrcBottom80        = 0x6426;
const sal_uInt16 sprmPBrcRight80         = 0x6427;
const sal_uInt16 sprmPBrcBetween80       = 0x6428;
const sal_uInt16 sprmPBrcBar80           = 0x6629;
const sal_uInt16 sprmPFNoAutoHyph        = 0x242A;
const sal_uInt16 sprmPWHeightAbs         = 0x442B;
const sal_uInt16 sprmPDcs                = 0x442C;
const sal_uInt16 sprmPShd80              = 0x442D;
const sal_uInt16 sprmPDyaFromText        = 0x842E;
const sal_uInt16 sprmPDxaFromText        = 0x842F;
const sal_uInt16 sprmPFLocked            = 0x2430;
const sal_uInt16 sprmPFWidowControl      = 0x2431;
const sal_uInt16 sprmPFKinsoku           = 0x2433;
const sal_uInt16 sprmPFWordWrap          = 0x2434;
const sal_uInt16 sprmPFOverflowPunct     = 0x2435;
const sal_uInt16 sprmPFTopLinePunct      = 0x2436;
const sal_uInt16 sprmPFAutoSpaceDE       = 0x2437;
const sal_uInt16 sprmPFAutoSpaceDN       = 0x2438;
const sal_uInt16 sprmPWAlignFont         = 0x4439;
const sal_uInt16 sprmPFrameTextFlow      = 0x443A;
const sal_uInt16 sprmPOutLvl             = 0x2640;
const sal_uInt16 sprmPFBiDi              = 0x2441;
const sal_uInt16 sprmPFNumRMIns          = 0x2443;
const sal_uInt16 sprmPNumRM              = 0xC645;
const sal_uInt16 sprmPHugePapx           = 0x6646;
const sal_uInt16 sprmPFUsePgsuSettings   = 0x2447;
const sal_uInt16 sprmPFAdjustRight       = 0x2448;
const sal_uInt16 sprmPItap               = 0x6649;
const sal_uInt16 sprmPDtap               = 0x664A;
const sal_uInt16 sprmPFInnerTableCell    = 0x244B;
const sal_uInt16 sprmPFInnerTtp          = 0x244C;
const sal_uInt16 sprmPShd                = 0xC64D;
const sal_uInt16 sprmPBrcTop             = 0xC64E;
const sal_uInt16 sprmPBrcLeft            = 0xC64F;
const sal_uInt16 sprmPBrcBottom          = 0xC650;
const sal_uInt16 sprmPBrcRight           = 0xC651;
const sal_uInt16 sprmPBrcBetween         = 0xC652;
const sal_uInt16 sprmPBrcBar             = 0xC653;
const sal_uInt16 sprmPDxcRight           = 0x4455;
const sal_uInt16 sprmPDxcLeft            = 0x4456;
const sal_uInt16 sprmPDxcLeft1           = 0x4457;
const sal_uInt16 sprmPDylBefore          = 0x4458;
const sal_uInt16 sprmPDylAfter           = 0x4459;
const sal_uInt16 sprmPFOpenTch           = 0x245A;
const sal_uInt16 sprmPFDyaBeforeAuto     = 0x245B;
const sal_uInt16 sprmPFDyaAfterAuto      = 0x245C;
const sal_uInt16 sprmPDxaRight           = 0x845D;
const sal_uInt16 sprmPDxaLeft            = 0x845E;
const sal_uInt16 sprmPNest               = 0x465F;
const sal_uInt16 sprmPDxaLeft1           = 0x8460;
const sal_uInt16 sprmPJc                 = 0x2461;
const sal_uInt16 sprmPFNoAllowOverlap    = 0x2462;
const sal_uInt16 sprmPWall               = 0x2664;
const sal_uInt16 sprmPIpgp               = 0x6465;
const sal_uInt16 sprmPCnf                = 0xC666;
const sal_uInt16 sprmPRsid               = 0x6467;
const sal_uInt16 sprmPIstdListPermute    = 0xC669;
const sal_uInt16 sprmPTableProps         = 0x646B;
const sal_uInt16 sprmPTIstdInfo          = 0xC66C;
const sal_uInt16 sprmPFContextualSpacing = 0x246D;
const sal_uInt16 sprmPPropRMark          = 0xC66F;
const sal_uInt16 sprmPFMirrorIndents     = 0x2470;
const sal_uInt16 sprmPTtwo               = 0x2471;

// [MS-DOC] - v20170112 Section 2.6.3
const sal_uInt16 sprmTJc90                = 0x5400;
const sal_uInt16 sprmTDxaLeft             = 0x9601;
const sal_uInt16 sprmTDxaGapHalf          = 0x9602;
const sal_uInt16 sprmTFCantSplit90        = 0x3403;
const sal_uInt16 sprmTTableHeader         = 0x3404;
const sal_uInt16 sprmTTableBorders80      = 0xD605;
const sal_uInt16 sprmTDyaRowHeight        = 0x9407;
const sal_uInt16 sprmTDefTable            = 0xD608;
const sal_uInt16 sprmTDefTableShd80       = 0xD609;
const sal_uInt16 sprmTTlp                 = 0x740A;
const sal_uInt16 sprmTFBiDi               = 0x560B;
const sal_uInt16 sprmTDefTableShd3rd      = 0xD60C;
const sal_uInt16 sprmTPc                  = 0x360D;
const sal_uInt16 sprmTDxaAbs              = 0x940E;
const sal_uInt16 sprmTDyaAbs              = 0x940F;
const sal_uInt16 sprmTDxaFromText         = 0x9410;
const sal_uInt16 sprmTDyaFromText         = 0x9411;
const sal_uInt16 sprmTDefTableShd         = 0xD612;
const sal_uInt16 sprmTTableBorders        = 0xD613;
const sal_uInt16 sprmTTableWidth          = 0xF614;
const sal_uInt16 sprmTFAutofit            = 0x3615;
const sal_uInt16 sprmTDefTableShd2nd      = 0xD616;
const sal_uInt16 sprmTWidthBefore         = 0xF617;
const sal_uInt16 sprmTWidthAfter          = 0xF618;
const sal_uInt16 sprmTFKeepFollow         = 0x3619;
const sal_uInt16 sprmTBrcTopCv            = 0xD61A;
const sal_uInt16 sprmTBrcLeftCv           = 0xD61B;
const sal_uInt16 sprmTBrcBottomCv         = 0xD61C;
const sal_uInt16 sprmTBrcRightCv          = 0xD61D;
const sal_uInt16 sprmTDxaFromTextRight    = 0x941E;
const sal_uInt16 sprmTDyaFromTextBottom   = 0x941F;
const sal_uInt16 sprmTSetBrc80            = 0xD620;
const sal_uInt16 sprmTInsert              = 0x7621;
const sal_uInt16 sprmTDelete              = 0x5622;
const sal_uInt16 sprmTDxaCol              = 0x7623;
const sal_uInt16 sprmTMerge               = 0x5624;
const sal_uInt16 sprmTSplit               = 0x5625;
const sal_uInt16 sprmTTextFlow            = 0x7629;
const sal_uInt16 sprmTVertMerge           = 0xD62B;
const sal_uInt16 sprmTVertAlign           = 0xD62C;
const sal_uInt16 sprmTSetShd              = 0xD62D;
const sal_uInt16 sprmTSetShdOdd           = 0xD62E;
const sal_uInt16 sprmTSetBrc              = 0xD62F;
const sal_uInt16 sprmTCellPadding         = 0xD632;
const sal_uInt16 sprmTCellSpacingDefault  = 0xD633;
const sal_uInt16 sprmTCellPaddingDefault  = 0xD634;
const sal_uInt16 sprmTCellWidth           = 0xD635;
const sal_uInt16 sprmTFitText             = 0xF636;
const sal_uInt16 sprmTFCellNoWrap         = 0xD639;
const sal_uInt16 sprmTIstd                = 0x563A;
const sal_uInt16 sprmTCellPaddingStyle    = 0xD63E;
const sal_uInt16 sprmTCellFHideMark       = 0xD642;
const sal_uInt16 sprmTSetShdTable         = 0xD660;
const sal_uInt16 sprmTWidthIndent         = 0xF661;
const sal_uInt16 sprmTCellBrcType         = 0xD662;
const sal_uInt16 sprmTFBiDi90             = 0x5664;
const sal_uInt16 sprmTFNoAllowOverlap     = 0x3465;
const sal_uInt16 sprmTFCantSplit          = 0x3466;
const sal_uInt16 sprmTPropRMark           = 0xD667;
const sal_uInt16 sprmTWall                = 0x3668;
const sal_uInt16 sprmTIpgp                = 0x7469;
const sal_uInt16 sprmTCnf                 = 0xD66A;
const sal_uInt16 sprmTDefTableShdRaw      = 0xD670;
const sal_uInt16 sprmTDefTableShdRaw2nd   = 0xD671;
const sal_uInt16 sprmTDefTableShdRaw3rd   = 0xD672;
const sal_uInt16 sprmTRsid                = 0x7479;
const sal_uInt16 sprmTCellVertAlignStyle  = 0x347C;
const sal_uInt16 sprmTCellNoWrapStyle     = 0x347D;
const sal_uInt16 sprmTCellBrcTopStyle     = 0xD47F;
const sal_uInt16 sprmTCellBrcBottomStyle  = 0xD680;
const sal_uInt16 sprmTCellBrcLeftStyle    = 0xD681;
const sal_uInt16 sprmTCellBrcRightStyle   = 0xD682;
const sal_uInt16 sprmTCellBrcInsideHStyle = 0xD683;
const sal_uInt16 sprmTCellBrcInsideVStyle = 0xD684;
const sal_uInt16 sprmTCellBrcTL2BRStyle   = 0xD685;
const sal_uInt16 sprmTCellBrcTR2BLStyle   = 0xD686;
const sal_uInt16 sprmTCellShdStyle        = 0xD687;
const sal_uInt16 sprmTCHorzBands          = 0x3488;
const sal_uInt16 sprmTCVertBands          = 0x3489;
const sal_uInt16 sprmTJc                  = 0x548A;

// [MS-DOC] - v20170112 Section 2.6.4
const sal_uInt16 sprmScnsPgn        = 0x3000;
const sal_uInt16 sprmSiHeadingPgn   = 0x3001;
const sal_uInt16 sprmSDxaColWidth   = 0xF203;
const sal_uInt16 sprmSDxaColSpacing = 0xF204;
const sal_uInt16 sprmSFEvenlySpaced = 0x3005;
const sal_uInt16 sprmSFProtected    = 0x3006;
const sal_uInt16 sprmSDmBinFirst    = 0x5007;
const sal_uInt16 sprmSDmBinOther    = 0x5008;
const sal_uInt16 sprmSBkc           = 0x3009;
const sal_uInt16 sprmSFTitlePage    = 0x300A;
const sal_uInt16 sprmSCcolumns      = 0x500B;
const sal_uInt16 sprmSDxaColumns    = 0x900C;
const sal_uInt16 sprmSNfcPgn        = 0x300E;
const sal_uInt16 sprmSFPgnRestart   = 0x3011;
const sal_uInt16 sprmSFEndnote      = 0x3012;
const sal_uInt16 sprmSLnc           = 0x3013;
const sal_uInt16 sprmSNLnnMod       = 0x5015;
const sal_uInt16 sprmSDxaLnn        = 0x9016;
const sal_uInt16 sprmSDyaHdrTop     = 0xB017;
const sal_uInt16 sprmSDyaHdrBottom  = 0xB018;
const sal_uInt16 sprmSLBetween      = 0x3019;
const sal_uInt16 sprmSVjc           = 0x301A;
const sal_uInt16 sprmSLnnMin        = 0x501B;
const sal_uInt16 sprmSPgnStart97    = 0x501C;
const sal_uInt16 sprmSBOrientation  = 0x301D;
const sal_uInt16 sprmSXaPage        = 0xB01F;
const sal_uInt16 sprmSYaPage        = 0xB020;
const sal_uInt16 sprmSDxaLeft       = 0xB021;
const sal_uInt16 sprmSDxaRight      = 0xB022;
const sal_uInt16 sprmSDyaTop        = 0x9023;
const sal_uInt16 sprmSDyaBottom     = 0x9024;
const sal_uInt16 sprmSDzaGutter     = 0xB025;
const sal_uInt16 sprmSDmPaperReq    = 0x5026;
const sal_uInt16 sprmSFBiDi         = 0x3228;
const sal_uInt16 sprmSFRTLGutter    = 0x322A;
const sal_uInt16 sprmSBrcTop80      = 0x702B;
const sal_uInt16 sprmSBrcLeft80     = 0x702C;
const sal_uInt16 sprmSBrcBottom80   = 0x702D;
const sal_uInt16 sprmSBrcRight80    = 0x702E;
const sal_uInt16 sprmSPgbProp       = 0x522F;
const sal_uInt16 sprmSDxtCharSpace  = 0x7030;
const sal_uInt16 sprmSDyaLinePitch  = 0x9031;
const sal_uInt16 sprmSClm           = 0x5032;
const sal_uInt16 sprmSTextFlow      = 0x5033;
const sal_uInt16 sprmSBrcTop        = 0xD234;
const sal_uInt16 sprmSBrcLeft       = 0xD235;
const sal_uInt16 sprmSBrcBottom     = 0xD236;
const sal_uInt16 sprmSBrcRight      = 0xD237;
const sal_uInt16 sprmSWall          = 0x3239;
const sal_uInt16 sprmSRsid          = 0x703A;
const sal_uInt16 sprmSFpc           = 0x303B;
const sal_uInt16 sprmSRncFtn        = 0x303C;
const sal_uInt16 sprmSRncEdn        = 0x303E;
const sal_uInt16 sprmSNFtn          = 0x503F;
const sal_uInt16 sprmSNfcFtnRef     = 0x5040;
const sal_uInt16 sprmSNEdn          = 0x5041;
const sal_uInt16 sprmSNfcEdnRef     = 0x5042;
const sal_uInt16 sprmSPropRMark     = 0xD243;
const sal_uInt16 sprmSPgnStart      = 0x7044;

// [MS-DOC] - v20170112 Section 2.6.5
const sal_uInt16 sprmPicBrcTop80    = 0x6C02;
const sal_uInt16 sprmPicBrcLeft80   = 0x6C03;
const sal_uInt16 sprmPicBrcBottom80 = 0x6C04;
const sal_uInt16 sprmPicBrcRight80  = 0x6C05;
const sal_uInt16 sprmPicBrcTop      = 0xCE08;
const sal_uInt16 sprmPicBrcLeft     = 0xCE09;
const sal_uInt16 sprmPicBrcBottom   = 0xCE0A;
const sal_uInt16 sprmPicBrcRight    = 0xCE0B;
}

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_SPRMIDS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
