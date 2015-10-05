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
const sal_uInt16 LN_PContextualSpacing = 0x246d;
const sal_uInt16 LN_PIstdPermute = 0xc601;
const sal_uInt16 LN_PTabProps = 0x646b;
const sal_uInt16 LN_TFCantSplit90 = 0x3466;
const sal_uInt16 LN_TCantSplit = 0x3644;
const sal_uInt16 LN_PIncLvl = 0x2602;
const sal_uInt16 LN_PJcExtra = 0x2461;
const sal_uInt16 LN_PJc = 0x2403;
const sal_uInt16 LN_PFSideBySide = 0x2404;
const sal_uInt16 LN_PFKeep = 0x2405;
const sal_uInt16 LN_PFKeepFollow = 0x2406;
const sal_uInt16 LN_PFPageBreakBefore = 0x2407;
const sal_uInt16 LN_PBrcl = 0x2408;
const sal_uInt16 LN_PBrcp = 0x2409;
const sal_uInt16 LN_PIlvl = 0x260a;
const sal_uInt16 LN_PIlfo = 0x460b;
const sal_uInt16 LN_PFNoLineNumb = 0x240c;
const sal_uInt16 LN_PChgTabsPapx = 0xc60d;
const sal_uInt16 LN_PDxaRight = 0x840e;
const sal_uInt16 LN_PDxaLeft = 0x840f;
const sal_uInt16 LN_PNest = 0x4610;
const sal_uInt16 LN_PDxaLeft180 = 0x8411;
const sal_uInt16 LN_PDyaLine = 0x6412;
const sal_uInt16 LN_PDyaBefore = 0xa413;
const sal_uInt16 LN_PDyaAfter = 0xa414;
const sal_uInt16 LN_PChgTabs = 0xc615;
const sal_uInt16 LN_PFInTable = 0x2416;
const sal_uInt16 LN_PCell = 0x244b;
const sal_uInt16 LN_PRow = 0x244c;
const sal_uInt16 LN_PFTtp = 0x2417;
const sal_uInt16 LN_PDxaAbs = 0x8418;
const sal_uInt16 LN_PDyaAbs = 0x8419;
const sal_uInt16 LN_PDxaWidth = 0x841a;
const sal_uInt16 LN_PPc = 0x261b;
const sal_uInt16 LN_PBrcTop10 = 0x461c;
const sal_uInt16 LN_PBrcLeft10 = 0x461d;
const sal_uInt16 LN_PBrcBottom10 = 0x461e;
const sal_uInt16 LN_PBrcRight10 = 0x461f;
const sal_uInt16 LN_PBrcBetween10 = 0x4620;
const sal_uInt16 LN_PBrcBar10 = 0x4621;
const sal_uInt16 LN_PDxaFromText10 = 0x4622;
const sal_uInt16 LN_PWr = 0x2423;
const sal_uInt16 LN_PBrcBar = 0x6629;
const sal_uInt16 LN_PFNoAutoHyph = 0x242a;
const sal_uInt16 LN_PWHeightAbs = 0x442b;
const sal_uInt16 LN_PDcs = 0x442c;
const sal_uInt16 LN_PShd = 0x442d;
const sal_uInt16 LN_PDyaFromText = 0x842e;
const sal_uInt16 LN_PDxaFromText = 0x842f;
const sal_uInt16 LN_PFLocked = 0x2430;
const sal_uInt16 LN_PFWidowControl = 0x2431;
const sal_uInt16 LN_PRuler = 0xc632;
const sal_uInt16 LN_PFKinsoku = 0x2433;
const sal_uInt16 LN_PFWordWrap = 0x2434;
const sal_uInt16 LN_PFOverflowPunct = 0x2435;
const sal_uInt16 LN_PFTopLinePunct = 0x2436;
const sal_uInt16 LN_PFAutoSpaceDE = 0x2437;
const sal_uInt16 LN_PFAutoSpaceDN = 0x2438;
const sal_uInt16 LN_PWAlignFont = 0x4439;
const sal_uInt16 LN_PFrameTextFlow = 0x443a;
const sal_uInt16 LN_PISnapBaseLine = 0x243b;
const sal_uInt16 LN_PAnld = 0xc63e;
const sal_uInt16 LN_PPropRMark = 0xc63f;
const sal_uInt16 LN_POutLvl = 0x2640;
const sal_uInt16 LN_PFBiDi = 0x2441;
const sal_uInt16 LN_PFNumRMIns = 0x2443;
const sal_uInt16 LN_PCrLf = 0x2444;
const sal_uInt16 LN_PNumRM = 0xc645;
const sal_uInt16 LN_PHugePapx = 0x6645;
const sal_uInt16 LN_PFUsePgsuSettings = 0x2447;
const sal_uInt16 LN_PFAdjustRight = 0x2448;
const sal_uInt16 LN_CFRMarkDel = 0x0800;
const sal_uInt16 LN_CFRMark = 0x0801;
const sal_uInt16 LN_CFFieldVanish = 0x0802;
const sal_uInt16 LN_CPicLocation = 0x6a03;
const sal_uInt16 LN_CIbstRMark = 0x4804;
const sal_uInt16 LN_CDttmRMark = 0x6805;
const sal_uInt16 LN_CFData = 0x0806;
const sal_uInt16 LN_CIdslRMark = 0x4807;
const sal_uInt16 LN_CChs = 0xea08;
const sal_uInt16 LN_CSymbol = 0x6a09;
const sal_uInt16 LN_CFOle2 = 0x080a;
const sal_uInt16 LN_CIdCharType = 0x480b;
const sal_uInt16 LN_CHighlight = 0x2a0c;
const sal_uInt16 LN_CObjLocation = 0x680e;
const sal_uInt16 LN_CFFtcAsciSymb = 0x2a10;
const sal_uInt16 LN_CIstd = 0x4a30;
const sal_uInt16 LN_CIstdPermute = 0xca31;
const sal_uInt16 LN_CDefault = 0x2a32;
const sal_uInt16 LN_CPlain = 0x2a33;
const sal_uInt16 LN_CKcd = 0x2a34;
const sal_uInt16 LN_CFBold = 0x0835;
const sal_uInt16 LN_CFItalic = 0x0836;
const sal_uInt16 LN_CFStrike = 0x0837;
const sal_uInt16 LN_CFOutline = 0x0838;
const sal_uInt16 LN_CFShadow = 0x0839;
const sal_uInt16 LN_CFSmallCaps = 0x083a;
const sal_uInt16 LN_CFCaps = 0x083b;
const sal_uInt16 LN_CFVanish = 0x083c;
const sal_uInt16 LN_CFtcDefault = 0x4a3d;
const sal_uInt16 LN_CKul = 0x2a3e;
const sal_uInt16 LN_CSizePos = 0xea3f;
const sal_uInt16 LN_CDxaSpace = 0x8840;
const sal_uInt16 LN_CLid = 0x4a41;
const sal_uInt16 LN_CIco = 0x2a42;
const sal_uInt16 LN_CHps = 0x4a43;
const sal_uInt16 LN_CHpsInc = 0x2a44;
const sal_uInt16 LN_CHpsPos = 0x4845;
const sal_uInt16 LN_CHpsPosAdj = 0x2a46;
const sal_uInt16 LN_CMajority = 0xca47;
const sal_uInt16 LN_CIss = 0x2a48;
const sal_uInt16 LN_CHpsNew50 = 0xca49;
const sal_uInt16 LN_CHpsInc1 = 0xca4a;
const sal_uInt16 LN_CHpsKern = 0x484b;
const sal_uInt16 LN_CMajority50 = 0xca4c;
const sal_uInt16 LN_CHpsMul = 0x4a4d;
const sal_uInt16 LN_CYsri = 0x484e;
const sal_uInt16 LN_CRgFtc0 = 0x4a4f;
const sal_uInt16 LN_CRgFtc1 = 0x4a50;
const sal_uInt16 LN_CRgFtc2 = 0x4a51;
const sal_uInt16 LN_CCharScale = 0x4852;
const sal_uInt16 LN_CFDStrike = 0x2a53;
const sal_uInt16 LN_CFImprint = 0x0854;
const sal_uInt16 LN_CFNoProof = 0x0875;
const sal_uInt16 LN_CFspecVanish = 0x0818;
const sal_uInt16 LN_CFitText = 0xca76;
const sal_uInt16 LN_CComplexScript = 0x882;
const sal_uInt16 LN_PTightWrap = 0x2471;
const sal_uInt16 LN_CFSpec = 0x0855;
const sal_uInt16 LN_CFObj = 0x0856;
const sal_uInt16 LN_CPropRMark = 0xca57;
const sal_uInt16 LN_CFEmboss = 0x0858;
const sal_uInt16 LN_CSfxText = 0x2859;
const sal_uInt16 LN_CFBiDi = 0x085a;
const sal_uInt16 LN_CFDiacColor = 0x085b;
const sal_uInt16 LN_CFBoldBi = 0x085c;
const sal_uInt16 LN_CFItalicBi = 0x085d;
const sal_uInt16 LN_CFtcBi = 0x4a5e;
const sal_uInt16 LN_CLidBi = 0x485f;
const sal_uInt16 LN_CIcoBi = 0x4a60;
const sal_uInt16 LN_CHpsBi = 0x4a61;
const sal_uInt16 LN_CDispFieldRMark = 0xca62;
const sal_uInt16 LN_CIbstRMarkDel = 0x4863;
const sal_uInt16 LN_CDttmRMarkDel = 0x6864;
const sal_uInt16 LN_CBrc80 = 0x6865;
const sal_uInt16 LN_CShd80 = 0x4866;
const sal_uInt16 LN_CShd = 0xca71;
const sal_uInt16 LN_CIdslRMarkDel = 0x4867;
const sal_uInt16 LN_CFUsePgsuSettings = 0x0868;
const sal_uInt16 LN_CCpg = 0x486b;
const sal_uInt16 LN_CPbiGrf = 0x4888;
const sal_uInt16 LN_CPbiIBullet = 0x6887;
const sal_uInt16 LN_CRgLid0_80 = 0x486d;
const sal_uInt16 LN_CRgLid1_80 = 0x486e;
const sal_uInt16 LN_CIdctHint = 0x286f;
const sal_uInt16 LN_PicBrcl = 0x2e00;
const sal_uInt16 LN_PicScale = 0xce01;
const sal_uInt16 LN_PicBrcTop80 = 0x6c02;
const sal_uInt16 LN_PicBrcLeft80 = 0x6c03;
const sal_uInt16 LN_PicBrcBottom80 = 0x6c04;
const sal_uInt16 LN_PicBrcRight80 = 0x6c05;
const sal_uInt16 LN_ScnsPgn = 0x3000;
const sal_uInt16 LN_SiHeadingPgn = 0x3001;
const sal_uInt16 LN_SOlstAnm = 0xd202;
const sal_uInt16 LN_SDxaColWidth = 0xf203;
const sal_uInt16 LN_SDxaColSpacing = 0xf204;
const sal_uInt16 LN_SFEvenlySpaced = 0x3005;
const sal_uInt16 LN_SFProtected = 0x3006;
const sal_uInt16 LN_SDmBinFirst = 0x5007;
const sal_uInt16 LN_SDmBinOther = 0x5008;
const sal_uInt16 LN_SBkc = 0x3009;
const sal_uInt16 LN_SFTitlePage = 0x300a;
const sal_uInt16 LN_SCcolumns = 0x500b;
const sal_uInt16 LN_SDxaColumns = 0x900c;
const sal_uInt16 LN_SFAutoPgn = 0x300d;
const sal_uInt16 LN_SNfcPgn = 0x300e;
const sal_uInt16 LN_SDyaPgn = 0xb00f;
const sal_uInt16 LN_SDxaPgn = 0xb010;
const sal_uInt16 LN_SFPgnRestart = 0x3011;
const sal_uInt16 LN_SFEndnote = 0x3012;
const sal_uInt16 LN_SLnc = 0x3013;
const sal_uInt16 LN_SGprfIhdt = 0x3014;
const sal_uInt16 LN_SNLnnMod = 0x5015;
const sal_uInt16 LN_SDxaLnn = 0x9016;
const sal_uInt16 LN_SDyaHdrTop = 0xb017;
const sal_uInt16 LN_SDyaHdrBottom = 0xb018;
const sal_uInt16 LN_SLBetween = 0x3019;
const sal_uInt16 LN_SVjc = 0x301a;
const sal_uInt16 LN_SLnnMin = 0x501b;
const sal_uInt16 LN_SPgnStart = 0x501c;
const sal_uInt16 LN_SBOrientation = 0x301d;
const sal_uInt16 LN_SBCustomize = 0x301e;
const sal_uInt16 LN_SXaPage = 0xb01f;
const sal_uInt16 LN_SYaPage = 0xb020;
const sal_uInt16 LN_SDxaLeft = 0xb021;
const sal_uInt16 LN_SDxaRight = 0xb022;
const sal_uInt16 LN_SDyaTop = 0x9023;
const sal_uInt16 LN_SDyaBottom = 0x9024;
const sal_uInt16 LN_SDzaGutter = 0xb025;
const sal_uInt16 LN_SDmPaperReq = 0x5026;
const sal_uInt16 LN_SPropRMark = 0xd227;
const sal_uInt16 LN_SFBiDi = 0x3228;
const sal_uInt16 LN_SFFacingCol = 0x3229;
const sal_uInt16 LN_SFRTLGutter = 0x322a;
const sal_uInt16 LN_SBrcTop80 = 0x702b;
const sal_uInt16 LN_SBrcLeft80 = 0x702c;
const sal_uInt16 LN_SBrcBottom80 = 0x702d;
const sal_uInt16 LN_SBrcRight80 = 0x702e;
const sal_uInt16 LN_SBrcTop = 0xd234;
const sal_uInt16 LN_SBrcLeft = 0xd235;
const sal_uInt16 LN_SBrcBottom = 0xd236;
const sal_uInt16 LN_SBrcRight = 0xd237;
const sal_uInt16 LN_SPgbProp = 0x522f;
const sal_uInt16 LN_SDxtCharSpace = 0x7030;
const sal_uInt16 LN_SDyaLinePitch = 0x9031;
const sal_uInt16 LN_SClm = 0x5032;
const sal_uInt16 LN_STextFlow = 0x5033;
const sal_uInt16 LN_TJc = 0x548A;
const sal_uInt16 LN_TJc90 = 0x5400;
const sal_uInt16 LN_TDxaLeft = 0x9601;
const sal_uInt16 LN_TDxaGapHalf = 0x9602;
const sal_uInt16 LN_TFCantSplit = 0x3403;
const sal_uInt16 LN_TTableHeader = 0x3404;
const sal_uInt16 LN_TTableBorders80 = 0xd605;
const sal_uInt16 LN_TDefTable10 = 0xd606;
const sal_uInt16 LN_TDyaRowHeight = 0x9407;
const sal_uInt16 LN_TDefTable = 0xd608;
const sal_uInt16 LN_TDefTableShd80 = 0xd609;
const sal_uInt16 LN_TDefTableShd = 0xd612;
const sal_uInt16 LN_TTlp = 0x740a;
const sal_uInt16 LN_TFBiDi = 0x560b;
const sal_uInt16 LN_THTMLProps = 0x740c;
const sal_uInt16 LN_TSetBrc80 = 0xd620;
const sal_uInt16 LN_TSetBrc = 0xd62f;
const sal_uInt16 LN_TInsert = 0x7621;
const sal_uInt16 LN_TDelete = 0x5622;
const sal_uInt16 LN_TDxaCol = 0x7623;
const sal_uInt16 LN_TMerge = 0x5624;
const sal_uInt16 LN_TSplit = 0x5625;
const sal_uInt16 LN_TSetBrc10 = 0xd626;
const sal_uInt16 LN_TSetShd80 = 0x7627;
const sal_uInt16 LN_TSetShdOdd80 = 0x7628;
const sal_uInt16 LN_TTextFlow = 0x7629;
const sal_uInt16 LN_TDiagLine = 0xd62a;
const sal_uInt16 LN_TVertMerge = 0xd62b;
const sal_uInt16 LN_TVertAlign = 0xd62c;
const sal_uInt16 LN_TCellTopColor = 0xd61a;
const sal_uInt16 LN_TCellLeftColor = 0xd61b;
const sal_uInt16 LN_TCellBottomColor = 0xd61c;
const sal_uInt16 LN_TCellRightColor = 0xd61d;
const sal_uInt16 LN_TTableBorders = 0xd613;
const sal_uInt16 LN_Cdelrsid = 0x6817;
const sal_uInt16 LN_PTableDepth = 0x6649;
const sal_uInt16 LN_TDefTableShdRaw = 0xd670;
const sal_uInt16 LN_PBrcTop80 = 0x6424;
const sal_uInt16 LN_PBrcLeft80 = 0x6425;
const sal_uInt16 LN_PBrcBottom80 = 0x6426;
const sal_uInt16 LN_PBrcRight80 = 0x6427;
const sal_uInt16 LN_PBrcBetween80 = 0x6428;
const sal_uInt16 LN_PBrcTop = 0xc64e;
const sal_uInt16 LN_PBrcLeft = 0xc64f;
const sal_uInt16 LN_PBrcBottom = 0xc650;
const sal_uInt16 LN_PBrcRight = 0xc651;
const sal_uInt16 LN_PBrcBetween = 0xc652;
const sal_uInt16 LN_CBrc = 0xca72;
const sal_uInt16 LN_TTableWidth = 0xf614;
const sal_uInt16 LN_CFELayout = 0xca78;
const sal_uInt16 LN_TWidthBefore = 0xf617;
const sal_uInt16 LN_TWidthAfter = 0xf618;
const sal_uInt16 LN_PDxaLeft1 = 0x8460;
const sal_uInt16 LN_PRsid = 0x6467;
const sal_uInt16 LN_CRsidProp = 0x6815;
const sal_uInt16 LN_CRgLid0 = 0x4873;
const sal_uInt16 LN_CRgLid1 = 0x4874;
const sal_uInt16 LN_TCellPadding = 0xd632;
const sal_uInt16 LN_TCellPaddingDefault = 0xd634;
const sal_uInt16 LN_TRsid = 0x7479;
const sal_uInt16 LN_TFAutofit = 0x3615;
const sal_uInt16 LN_TPc = 0x360d;
const sal_uInt16 LN_TDyaAbs = 0x940f;
const sal_uInt16 LN_TDxaFromText = 0x9410;
const sal_uInt16 LN_TDyaFromText = 0x9411;
const sal_uInt16 LN_TDxaFromTextRight = 0x941e;
const sal_uInt16 LN_TDyaFromTextBottom = 0x941f;
const sal_uInt16 LN_TFNoAllowOverlap = 0x3465;
const sal_uInt16 LN_CCv = 0x6870;
const sal_uInt16 LN_CCvUl = 0x6877;
}

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_SPRMIDS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
