/*************************************************************************
 *
 *  $RCSfile: ww8darr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-24 14:56:06 $
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

#include <stdlib.h>

#include "ww8dout.hxx"              // OutShort()
#include "ww8darr.hxx"




//-----------------------------------------



SprmDumpInfo aSprmDumpTab[] = {
    0, (FnOut)0, "NOP(0)",
    2, OutShort, "sprmPIstd",  // pap.istd (style code)
    3, (FnOut)0, "sprmPIstdPermute", // pap.istd    permutation
    4, OutByte,  "sprmPIncLv1", // pap.istddifference
    5, OutByte,  "sprmPJc", // pap.jc (justification)
    6, OutBool,  "sprmPFSideBySide", // pap.fSideBySide
    7, OutBool,  "sprmPFKeep", // pap.fKeep
    8, OutBool,  "sprmPFKeepFollow ", //    pap.fKeepFollow
    9, OutBool,  "sprmPPageBreakBefore", // pap.fPageBreakBefore

    10, (FnOut)0, "sprmPBrcl", //   pap.brcl
    11, (FnOut)0, "sprmPBrcp ", // pap.brcp
    12, (FnOut)0, "sprmPAnld", //   pap.anld (ANLD structure)
    13, OutByte,  "sprmPNLvlAnm", // pap.nLvlAnm nn
    14, OutBool,  "sprmPFNoLineNumb", //ap.fNoLnn

    //??
    15, OutTab,   "?sprmPChgTabsPapx", // pap.itbdMac, ...


    16, OutShort,  "sprmPDxaRight", // pap.dxaRight
    17, OutShort,  "sprmPDxaLeft", // pap.dxaLeft
    18, OutWord,  "sprmPNest", // pap.dxaLeft
    19, OutShort,  "sprmPDxaLeft1", // pap.dxaLeft1

    20, OutShorts, "sprmPDyaLine", // pap.lspd  an LSPD
    21, OutWord,  "sprmPDyaBefore", // pap.dyaBefore
    22, OutWord,  "sprmPDyaAfter", //   pap.dyaAfter


    //??
    23, (FnOut)0, "?sprmPChgTabs", // pap.itbdMac, pap.rgdxaTab, ...


    24, OutBool,  "sprmPFInTable", // pap.fInTable
    25, OutBool,  "sprmPTtp", // pap.fTtp
    26, OutWord,  "sprmPDxaAbs", // pap.dxaAbs
    27, OutWord,  "sprmPDyaAbs", // pap.dyaAbs
    28, OutWord, "sprmPDxaWidth", // pap.dxaWidth
    29, (FnOut)0, "sprmPPc", // pap.pcHorz, pap.pcVert

    30, (FnOut)0, "sprmPBrcTop10", // pap.brcTop BRC10
    31, (FnOut)0, "sprmPBrcLeft10", // pap.brcLeft BRC10
    32, (FnOut)0, "sprmPBrcBottom10", // pap.brcBottom BRC10
    33, (FnOut)0, "sprmPBrcRight10", // pap.brcRight BRC10
    34, (FnOut)0, "sprmPBrcBetween10", // pap.brcBetween BRC10
    35, (FnOut)0, "sprmPBrcBar10", // pap.brcBar BRC10
    36, OutWord, "sprmPFromText10", // pap.dxaFromText dxa
    37, (FnOut)0, "sprmPWr", // pap.wr wr
    38, OutWordHex, "sprmPBrcTop", // pap.brcTop BRC
    39, OutWordHex, "sprmPBrcLeft", // pap.brcLeft BRC

    40, OutWordHex, "sprmPBrcBottom", // pap.brcBottom BRC
    41, OutWordHex, "sprmPBrcRight", // pap.brcRight BRC
    42, (FnOut)0, "sprmPBrcBetween", // pap.brcBetween BRC
    43, (FnOut)0, "sprmPBrcBar",//pap.brcBar BRC word
    44, OutBool,  "sprmPFNoAutoHyph",//pap.fNoAutoHyph
    45, OutWord,  "sprmPWHeightAbs",//pap.wHeightAbs w
    46, (FnOut)0, "sprmPDcs",//pap.dcs DCS
    47, OutWordHex, "sprmPShd",//pap.shd SHD
    48, OutWord,  "sprmPDyaFromText",//pap.dyaFromText dya
    49, OutWord,  "sprmPDxaFromText",//pap.dxaFromText dxa

    50, OutBool,  "sprmPFLocked", // pap.fLocked 0 or 1 byte
    51, OutBool,  "sprmPFWidowControl", // pap.fWidowControl 0 or 1 byte


    //??
    52, (FnOut)0, "?sprmPRuler 52", //???


    65, OutBool,  "sprmCFStrikeRM", // chp.fRMarkDel 1 or 0 bit
    66, OutBool,  "sprmCFRMark", // chp.fRMark 1 or 0 bit
    67, OutBool,  "sprmCFFldVanish", // chp.fFldVanish 1 or 0 bit
    68, OutTab68, "sprmCPicLocation", // chp.fcPic and chp.fSpec
    69, OutShort, "sprmCIbstRMark", // chp.ibstRMark index into sttbRMark

    70, (FnOut)0, "sprmCDttmRMark", // chp.dttm DTTM long
    71, OutBool,  "sprmCFData", // chp.fData 1 or 0 bit
    72, OutShort, "sprmCRMReason", // chp.idslRMReason an index to a table
    73, (FnOut)0, "sprmCChse", // chp.fChsDiff and chp.chse see below 3 bytes
    74, (FnOut)0, "sprmCSymbol", // chp.fSpec, chp.chSym and chp.ftcSym
    75, OutBool,  "sprmCFOle2", // chp.fOle2 1 or 0 bit

    80, OutShort, "sprmCIstd", // chp.istd istd, see stylesheet definition short
    81, (FnOut)0, "sprmCIstdPermute", // chp.istd permutation vector (see below)
    82, (FnOut)0, "sprmCDefault", // whole CHP (see below) none variable length
    83, (FnOut)0, "sprmCPlain", // whole CHP (see below) none 0
    84, (FnOut)0, "??84",
    85, OutBool4, "sprmCFBold", // chp.fBold 0,1, 128, or 129 (see below) byte
    86, OutBool4, "sprmCFItalic", // chp.fItalic 0,1, 128, or 129 (see below) byte
    87, OutBool4, "sprmCFStrike", // chp.fStrike 0,1, 128, or 129 (see below) byte
    88, OutBool4, "sprmCFOutline", // chp.fOutline 0,1, 128, or 129 (see below) byte
    89, OutBool4, "sprmCFShadow", // chp.fShadow 0,1, 128, or 129 (see below) byte

    90, OutBool4, "sprmCFSmallCaps", // chp.fSmallCaps 0,1, 128, or 129 (see below) byte
    91, OutBool4, "sprmCFCaps", // chp.fCaps 0,1, 128, or 129 (see below) byte
    92, OutBool4, "sprmCFVanish", // chp.fVanish 0,1, 128, or 129 (see below) byte
    93, OutWord,  "sprmCFtc", // chp.ftc ftc word
    94, OutByte,  "sprmCKul", // chp.kul kul byte
    95, (FnOut)0, "sprmCSizePos", // chp.hps, chp.hpsPos (see below) 3 bytes
    96, OutWord,  "sprmCDxaSpace", // chp.dxaSpace dxa word
    97, OutWord,  "sprmCLid", // chp.lid LID word
    98, OutByte,  "sprmCIco", // chp.ico ico byte
    99, OutWord,  "sprmCHps", // chp.hps hps word!

    100, OutByte,  "sprmCHpsInc", // chp.hps (see below) byte
    101, OutWord,  "sprmCHpsPos", // chp.hpsPos hps byte
    102, OutByte,  "sprmCHpsPosAdj", // chp.hpsPos hps (see below) byte


    //??
    103, (FnOut)0, "?sprmCMajority", // chp.fBold, chp.fItalic, chp.fSmallCaps, ...


    104, OutByte,  "sprmCIss", // chp.iss iss byte
    105, (FnOut)0, "sprmCHpsNew50", // chp.hps hps variable width, length always recorded as 2
    106, (FnOut)0, "sprmCHpsInc1", // chp.hps complex (see below) variable width, length always recorded as 2
    107, OutShort, "sprmCHpsKern", // chp.hpsKern hps short
    108, (FnOut)0, "sprmCMajority50", // chp.fBold, chp.fItalic, chp.fSmallCaps, chp.fVanish, ...
    109, OutShort, "sprmCHpsMul", // chp.hps percentage to grow hps short

    110, OutShort, "sprmCCondHyhen", // chp.ysri ysri short
    117, OutBool,  "sprmCFSpec", // chp.fSpec  1 or 0 bit
    118, OutBool,  "sprmCFObj", // chp.fObj 1 or 0 bit
    119, (FnOut)0, "sprmPicBrcl", // pic.brcl brcl (see PIC structure definition) byte

    120,    (FnOut)0, "sprmPicScale", // pic.mx, pic.my, pic.dxaCropleft,
    121, (FnOut)0, "sprmPicBrcTop", // pic.brcTop BRC word
    122, (FnOut)0, "sprmPicBrcLeft", // pic.brcLeft BRC word
    123, (FnOut)0, "sprmPicBrcBottom", // pic.brcBottom BRC word
    124, (FnOut)0, "sprmPicBrcRight", // pic.brcRight BRC word
    131, OutByte,  "sprmSScnsPgn", // sep.cnsPgn cns byte
    132, (FnOut)0, "sprmSiHeadingPgn", // sep.iHeadingPgn heading number level byte
    133, (FnOut)0, "sprmSOlstAnm", // sep.olstAnm OLST variable length
    136, (FnOut)0, "sprmSDxaColWidth", // sep.rgdxaColWidthSpacing complex (see below) 3 bytes
    137, (FnOut)0, "sprmSDxaColSpacing", // sep.rgdxaColWidthSpacing complex (see below) 3 bytes
    138, OutBool,  "sprmSFEvenlySpaced", // sep.fEvenlySpaced 1 or 0 byte
    139, OutBool,  "sprmSFProtected", // sep.fUnlocked 1 or 0 byte

    140, OutWord,  "sprmSDmBinFirst", // sep.dmBinFirst  word
    141, OutWord,  "sprmSDmBinOther", // sep.dmBinOther  word
    142, OutByte,  "sprmSBkc", // sep.bkc bkc byte
    143, OutBool,  "sprmSFTitlePage", // sep.fTitlePage 0 or 1 byte
    144, OutWord,  "sprmSCcolumns", // sep.ccolM1 # of cols - 1 word
    145, OutWord,  "sprmSDxaColumns", // sep.dxaColumns dxa word
    146, (FnOut)0, "sprmSFAutoPgn", // sep.fAutoPgn obsolete byte
    147, OutByte,  "sprmSNfcPgn", // sep.nfcPgn nfc byte
    148, OutShort, "sprmSDyaPgn", // sep.dyaPgn dya short
    149, OutShort, "sprmSDxaPgn", // sep.dxaPgn dya short

    150, OutBool,  "sprmSFPgnRestart", // sep.fPgnRestart 0 or 1 byte
    151, OutBool,  "sprmSFEndnote", // sep.fEndnote 0 or 1 byte
    152, OutByte,  "sprmSLnc", // sep.lnc lnc byte
    153, (FnOut)0, "sprmSGprfIhdt", // sep.grpfIhdt grpfihdt (see Headers and Footers topic) byte
    154, OutWord,  "sprmSNLnnMod", // sep.nLnnMod non-neg int. word
    155, OutWord, "sprmSDxaLnn", // sep.dxaLnn dxa word
    156, OutWord,  "sprmSDyaHdrTop", // sep.dyaHdrTop dya word
    157, OutWord,  "sprmSDyaHdrBottom", // sep.dyaHdrBottom dya word
    158, OutBool,  "sprmSLBetween", // sep.fLBetween 0 or 1 byte
    159, OutByte,  "sprmSVjc", // sep.vjc vjc byte

    160, OutWord,  "sprmSLnnMin", // sep.lnnMin lnn word
    161, OutWord,  "sprmSPgnStart", // sep.pgnStart pgn word
    162, OutByte,  "sprmSBOrientation", // sep.dmOrientPage dm byte


    //??
    163, OutWord,  "?SprmSBCustomize 163", //???


    164, OutWord,  "sprmSXaPage", // sep.xaPage xa word
    165, OutWord,  "sprmSYaPage", // sep.yaPage ya word
    166, OutWord,  "sprmSDxaLeft", // sep.dxaLeft dxa word
    167, OutWord,  "sprmSDxaRight", // sep.dxaRight dxa word
    168, OutWord,  "sprmSDyaTop", // sep.dyaTop dya word
    169, OutWord,  "sprmSDyaBottom", // sep.dyaBottom dya word

    170, OutWord,  "sprmSDzaGutter", // sep.dzaGutter dza word
    171, OutWord,  "sprmSDMPaperReq", // sep.dmPaperReq dm word
    182, OutWord,  "sprmTJc", // tap.jc jc word (low order byte is significant)
    183, OutWord,  "sprmTDxaLeft", // tap.rgdxaCenter (see below) dxa word
    184, OutWord,  "sprmTDxaGapHalf", // tap.dxaGapHalf, tap.rgdxaCenter (see below) dxa word
    185, OutBool,  "sprmTFCantSplit", // tap.fCantSplit 1 or 0 byte
    186, OutBool,  "sprmTTableHeader", // tap.fTableHeader 1 or 0 byte
    187, OutWordsHex, "sprmTTableBorders", // tap.rgbrcTable complex(see below) 12 bytes
    188, (FnOut)0, "sprmTDefTable10", // tap.rgdxaCenter, tap.rgtc complex (see below) variable length
    189, OutShort,  "sprmTDyaRowHeight", // tap.dyaRowHeight dya word


    //??
    190, OutTab190, "sprmTDefTable", // tap.rgtc complex (see below)


    //??
    191, OutTab191, "sprmTDefTableShd", // tap.rgshd complex (see below)


    192, OutTab192, "sprmTTlp", // tap.tlp TLP 4 bytes
    193, (FnOut)0, "sprmTSetBrc", // tap.rgtc[].rgbrc complex (see below) 5 bytes
    194, (FnOut)0, "sprmTInsert", // tap.rgdxaCenter,tap.rgtc complex (see below) 4 bytes
    195, (FnOut)0, "sprmTDelete", // tap.rgdxaCenter, tap.rgtc complex (see below) word
    196, (FnOut)0, "sprmTDxaCol", // tap.rgdxaCenter complex (see below) 4 bytes
    197, (FnOut)0, "sprmTMerge", // tap.fFirstMerged, tap.fMerged complex (see below) word
    198, (FnOut)0, "sprmTSplit", // tap.fFirstMerged, tap.fMerged complex (see below) word
    199, (FnOut)0, "sprmTSetBrc10", // tap.rgtc[].rgbrc complex (see below) 5 bytes

    200, (FnOut)0, "sprmTSetShd", // tap.rgshd complex (see below) 4 bytes



    // ab hier Selbstdefinierte Ids

    256, (FnOut)0, "FootNote",
    257, (FnOut)0, "EndNote",
    258, (FnOut)0, "??258 selbstdef.",
    259, (FnOut)0, "??259 selbstdef.",
    260, (FnOut)0, "Field",

//- neue ab Ver8 ------------------------------------------------------------

    0x4600, OutShort,"sprmPIstd", // pap.istd;istd (style code);short;
    0xC601, (FnOut)0,"sprmPIstdPermute", // pap.istd;permutation vector (see below);variable length;
    0x2602, OutByte, "sprmPIncLvl", // pap.istd, pap.lvl;difference between istd of base PAP and istd of PAP to be produced (see below);byte;
    0x2403, OutByte, "sprmPJc", // pap.jc;jc (justification);byte;
    0x2404, OutByte, "sprmPFSideBySide", // pap.fSideBySide;0 or 1;byte;
    0x2405, OutByte, "sprmPFKeep", // pap.fKeep;0 or 1;byte;
    0x2406, OutByte, "sprmPFKeepFollow", // pap.fKeepFollow;0 or 1;byte;
    0x2407, OutByte, "sprmPFPageBreakBefore", // pap.fPageBreakBefore;0 or 1;byte;
    0x2408, OutByte, "sprmPBrcl", // pap.brcl;brcl;byte;
    0x2409, OutByte, "sprmPBrcp", // pap.brcp;brcp;byte;
    0x260A, OutByte, "sprmPIlvl", // pap.ilvl;ilvl;byte;
    0x460B, OutShort, "sprmPIlfo", // pap.ilfo;ilfo (list index) ;short;
    0x240C, OutByte, "sprmPFNoLineNumb", // pap.fNoLnn;0 or 1;byte;
    0xC60D, OutTab,  "sprmPChgTabsPapx", // pap.itbdMac, pap.rgdxaTab, pap.rgtbd;complex - see below;variable length
    0x840E, OutWord, "sprmPDxaRight", // pap.dxaRight;dxa;word;
    0x840F, OutWord, "sprmPDxaLeft", // pap.dxaLeft;dxa;word;
    0x4610, OutWord, "sprmPNest", // pap.dxaLeft;dxa-see below;word;
    0x8411, OutWord, "sprmPDxaLeft1", // pap.dxaLeft1;dxa;word;
    0x6412,OutLongHex,"sprmPDyaLine", // pap.lspd;an LSPD, a long word structure consisting of a short of dyaLine followed by a short of fMultLinespace - see below;long;
    0xA413, OutWord, "sprmPDyaBefore", // pap.dyaBefore;dya;word;
    0xA414, OutWord, "sprmPDyaAfter", // pap.dyaAfter;dya;word;
    0xC615, (FnOut)0,"sprmPChgTabs", // pap.itbdMac, pap.rgdxaTab, pap.rgtbd;complex - see below;variable length;
    0x2416, OutByte, "sprmPFInTable", // pap.fInTable;0 or 1;byte;
    0x2417, OutByte, "sprmPFTtp", // pap.fTtp;0 or 1;byte;
    0x8418, OutWord, "sprmPDxaAbs", // pap.dxaAbs;dxa;word;
    0x8419, OutWord, "sprmPDyaAbs", // pap.dyaAbs;dya;word;
    0x841A, OutWord, "sprmPDxaWidth", // pap.dxaWidth;dxa;word;
    0x261B, OutByte, "sprmPPc", // pap.pcHorz, pap.pcVert;complex - see below;byte;
    0x461C, OutWord, "sprmPBrcTop10", // pap.brcTop;BRC10;word;
    0x461D, OutWord, "sprmPBrcLeft10", // pap.brcLeft;BRC10;word;
    0x461E, OutWord, "sprmPBrcBottom10", // pap.brcBottom;BRC10;word;
    0x461F, OutWord, "sprmPBrcRight10", // pap.brcRight;BRC10;word;
    0x4620, OutWord, "sprmPBrcBetween10", // pap.brcBetween;BRC10;word;
    0x4621, OutWord, "sprmPBrcBar10", // pap.brcBar;BRC10;word;
    0x4622, OutWord, "sprmPDxaFromText10", // pap.dxaFromText;dxa;word;
    0x2423, OutByte, "sprmPWr", // pap.wr;wr (see description of PAP for definition;byte;
    0x6424,OutLongHex,"sprmPBrcTop", // pap.brcTop;BRC;long;
    0x6425,OutLongHex,"sprmPBrcLeft", // pap.brcLeft;BRC;long;
    0x6426,OutLongHex,"sprmPBrcBottom", // pap.brcBottom;BRC;long;
    0x6427,OutLongHex,"sprmPBrcRight", // pap.brcRight;BRC;long;
    0x6428,OutLongHex,"sprmPBrcBetween", // pap.brcBetween;BRC;long;
    0x6629,OutLongHex,"sprmPBrcBar", // pap.brcBar;BRC;long;
    0x242A, OutByte, "sprmPFNoAutoHyph", // pap.fNoAutoHyph;0 or 1;byte;
    0x442B, OutWord, "sprmPWHeightAbs", // pap.wHeightAbs;w;word;
    0x442C, OutShort,"sprmPDcs", // pap.dcs;DCS;short;
    0x442D, OutWord, "sprmPShd", // pap.shd;SHD;word;
    0x842E, OutWord, "sprmPDyaFromText", // pap.dyaFromText;dya;word;
    0x842F, OutWord, "sprmPDxaFromText", // pap.dxaFromText;dxa;word;
    0x2430, OutByte, "sprmPFLocked", // pap.fLocked;0 or 1;byte;
    0x2431, OutByte, "sprmPFWidowControl", // pap.fWidowControl;0 or 1;byte;
    0xC632, (FnOut)0,"sprmPRuler", // ;;variable length;
    0x2433, OutByte, "sprmPFKinsoku", // pap.fKinsoku;0 or 1;byte;
    0x2434, OutByte, "sprmPFWordWrap", // pap.fWordWrap;0 or 1;byte;
    0x2435, OutByte, "sprmPFOverflowPunct", // pap.fOverflowPunct;0 or 1;byte;
    0x2436, OutByte, "sprmPFTopLinePunct", // pap.fTopLinePunct;0 or 1;byte;
    0x2437, OutByte, "sprmPFAutoSpaceDE", // pap.fAutoSpaceDE;0 or 1;byte;
    0x2438, OutByte, "sprmPFAutoSpaceDN", // pap.fAutoSpaceDN;0 or 1;byte;
    0x4439, OutWord, "sprmPWAlignFont", // pap.wAlignFont;iFa (see description of PAP for definition);word;
    0x443A, OutWord, "sprmPFrameTextFlow", // pap.fVertical pap.fBackward pap.fRotateFont;complex (see description of PAP for definition);word;
    0x243B, OutByte, "sprmPISnapBaseLine", // obsolete: not applicable in Word97 and later versions;;byte;
    0xC63E, (FnOut)0,"sprmPAnld", // pap.anld;;variable length;
    0xC63F, (FnOut)0,"sprmPPropRMark", // pap.fPropRMark;complex (see below);variable length;
    0x2640, OutByte, "sprmPOutLvl", // pap.lvl;has no effect if pap.istd is < 1 or is > 9;byte;
    0x2441, OutByte, "sprmPFBiDi", // ;;byte;
    0x2443, OutBool, "sprmPFNumRMIns", // pap.fNumRMIns;1 or 0;bit;
    0x2444, OutByte, "sprmPCrLf", // ;;byte;
    0xC645, (FnOut)0,"sprmPNumRM", // pap.numrm;;variable length;
    0x6645,OutHugeHex,"sprmPHugePapx", // see below;fc in the data stream to locate the huge grpprl (see below);long;
    0x6646,OutHugeHex,"sprmPHugePapx", // see below;fc in the data stream to locate the huge grpprl (see below);long;
    0x2447, OutByte, "sprmPFUsePgsuSettings", // pap.fUsePgsuSettings;1 or 0;byte;
    0x2448, OutByte, "sprmPFAdjustRight", // pap.fAdjustRight;1 or 0;byte;
    0x0800, OutBool, "sprmCFRMarkDel", // chp.fRMarkDel;1 or 0;bit;
    0x0801, OutBool, "sprmCFRMark", // chp.fRMark;1 or 0;bit;
    0x0802, OutBool, "sprmCFFldVanish", // chp.fFldVanish;1 or 0;bit;
    0x6A03, OutTab68,"sprmCPicLocation", // chp.fcPic and chp.fSpec;see below;variable length, length recorded is always 4;
    0x4804, OutShort,"sprmCIbstRMark", // chp.ibstRMark;index into sttbRMark;short;
    0x6805,OutLongHex,"sprmCDttmRMark", // chp.dttmRMark;DTTM;long;
    0x0806, OutBool, "sprmCFData", // chp.fData;1 or 0;bit;
    0x4807, OutShort, "sprmCIdslRMark", // chp.idslRMReason;an index to a table of strings defined in Word 6.0 executables;short;
    0xEA08, (FnOut)0,"sprmCChs", // chp.fChsDiff and chp.chse;see below;3 bytes;
    0x6A09, (FnOut)0,"sprmCSymbol", // chp.fSpec, chp.xchSym and chp.ftcSym;see below;variable length, length recorded is always 4;
    0x080A, OutBool, "sprmCFOle2", // chp.fOle2;1 or 0;bit;
//0x480B, obsolete,"sprmCIdCharType", // obsolete: not applicable in Word97 and later versions;;;
    0x2A0C, OutByte, "sprmCHighlight", // chp.fHighlight, chp.icoHighlight;ico (fHighlight is set to 1 iff ico is not 0);byte;
    0x680E,OutLongHex,"sprmCObjLocation", // chp.fcObj;FC;long;
//0x2A10, ? ? ?  , "sprmCFFtcAsciSymb", // ;;;
    0x4A30, OutShort, "sprmCIstd", // chp.istd;istd, see stylesheet definition;short;
    0xCA31, (FnOut)0,"sprmCIstdPermute", // chp.istd;permutation vector (see below);variable length;
    0x2A32, (FnOut)0,"sprmCDefault", // whole CHP (see below);none;variable length;
    0x2A33, (FnOut)0,"sprmCPlain", // whole CHP (see below);none; Laenge: 0;
    0x2A34, OutByte, "sprmCKcd", // ;;;
    0x0835, OutByte, "sprmCFBold", // chp.fBold;0,1, 128, or 129 (see below);byte;
    0x0836, OutByte, "sprmCFItalic", // chp.fItalic;0,1, 128, or 129 (see below);byte;
    0x0837, OutByte, "sprmCFStrike", // chp.fStrike;0,1, 128, or 129 (see below);byte;
    0x0838, OutByte, "sprmCFOutline", // chp.fOutline;0,1, 128, or 129 (see below);byte;
    0x0839, OutByte, "sprmCFShadow", // chp.fShadow;0,1, 128, or 129 (see below);byte;
    0x083A, OutByte, "sprmCFSmallCaps", // chp.fSmallCaps;0,1, 128, or 129 (see below);byte;
    0x083B, OutByte, "sprmCFCaps", // chp.fCaps;0,1, 128, or 129 (see below);byte;
    0x083C, OutByte, "sprmCFVanish", // chp.fVanish;0,1, 128, or 129 (see below);byte;
    0x4A3D, OutWord, "sprmCFtcDefault", // ;ftc, only used internally, never stored in file;word;
    0x2A3E, OutByte, "sprmCKul", // chp.kul;kul;byte;
    0xEA3F, (FnOut)0,"sprmCSizePos", // chp.hps, chp.hpsPos;(see below);3 bytes;
    0x8840, OutWord, "sprmCDxaSpace", // chp.dxaSpace;dxa;word;
    0x4A41, OutWord, "sprmCLid", // ;only used internally never stored;word;
    0x2A42, OutByte, "sprmCIco", // chp.ico;ico;byte;
    0x4A43, OutWord, "sprmCHps", // chp.hps;hps;word;
    0x2A44, OutByte, "sprmCHpsInc", // chp.hps;(see below);byte;
    0x4845, OutByte, "sprmCHpsPos", // chp.hpsPos;hps;byte;
    0x2A46, OutByte, "sprmCHpsPosAdj", // chp.hpsPos;hps (see below);byte;
    0xCA47, (FnOut)0,"sprmCMajority", // chp.fBold, chp.fItalic, chp.fSmallCaps, chp.fVanish, chp.fStrike, chp.fCaps, chp.rgftc, chp.hps, chp.hpsPos, chp.kul, chp.dxaSpace, chp.ico, chp.rglid;complex (see below);variable length, length byte plus size of following grpprl;
    0x2A48, OutByte, "sprmCIss", // chp.iss;iss;byte;
    0xCA49, (FnOut)0,"sprmCHpsNew50", // chp.hps;hps;variable width, length always recorded as 2;
    0xCA4A, (FnOut)0,"sprmCHpsInc1", // chp.hps;complex (see below);variable width, length always recorded as 2;
    0x484B, OutShort,"sprmCHpsKern", // chp.hpsKern;hps;short;
    0xCA4C, (FnOut)0,"sprmCMajority50", // chp.fBold, chp.fItalic, chp.fSmallCaps, chp.fVanish, chp.fStrike, chp.fCaps, chp.ftc, chp.hps, chp.hpsPos, chp.kul, chp.dxaSpace, chp.ico,;complex (see below);variable length;
    0x4A4D, OutShort,"sprmCHpsMul", // chp.hps;percentage to grow hps;short;
    0x484E, OutShort,"sprmCYsri", // chp.ysri;ysri;short;
    0x4A4F, OutShort,"sprmCRgFtc0", // chp.rgftc[0];ftc for ASCII text (see below);short;
    0x4A50, OutShort,"sprmCRgFtc1", // chp.rgftc[1];ftc for Far East text (see below);short;
    0x4A51, OutShort,"sprmCRgFtc2", // chp.rgftc[2];ftc for non-Far East text (see below);short;
//0x4852, ? ? ?  , "sprmCCharScale", // ;;;
    0x2A53, OutByte, "sprmCFDStrike", // chp.fDStrike;;byte;
    0x0854, OutBool, "sprmCFImprint", // chp.fImprint;1 or 0;bit;
    0x0855, OutBool, "sprmCFSpec", // chp.fSpec ;1 or 0;bit;
    0x0856, OutBool, "sprmCFObj", // chp.fObj;1 or 0;bit;
    0xCA57, (FnOut)0,"sprmCPropRMark", // chp.fPropRMark, chp.ibstPropRMark, chp.dttmPropRMark;Complex (see below);variable length always recorded as 7 bytes;
    0x0858, OutBool, "sprmCFEmboss", // chp.fEmboss;1 or 0;bit;
    0x2859, OutByte, "sprmCSfxText", // chp.sfxtText;text animation;byte;
//0x085A, ? ? ?  , "sprmCFBiDi", // ;;;
//0x085B, ? ? ?  , "sprmCFDiacColor", // ;;;
//0x085C, ? ? ?  , "sprmCFBoldBi", // ;;;
//0x085D, ? ? ?  , "sprmCFItalicBi", // ;;;
    0x4A5E, OutWord, "sprmCFtcBi", // ;;;
    0x485F, OutWord, "sprmCLidBi", // ;;;
//0x4A60, ? ? ?  , "sprmCIcoBi", // ;;;
    0x4A61, OutWord, "sprmCHpsBi", // ;;;
    0xCA62, (FnOut)0,"sprmCDispFldRMark", // chp.fDispFldRMark, chp.ibstDispFldRMark, chp.dttmDispFldRMark ;Complex (see below);variable length always recorded as 39 bytes;
    0x4863, OutShort,"sprmCIbstRMarkDel", // chp.ibstRMarkDel;index into sttbRMark;short;
    0x6864,OutLongHex,"sprmCDttmRMarkDel", // chp.dttmRMarkDel;DTTM;long;
    0x6865,OutLongHex,"sprmCBrc", // chp.brc;BRC;long;
    0x4866, OutShort,"sprmCShd", // chp.shd;SHD;short;
    0x4867, OutShort,"sprmCIdslRMarkDel", // chp.idslRMReasonDel;an index to a table of strings defined in Word 6.0 executables;short;
    0x0868, OutBool, "sprmCFUsePgsuSettings", // chp.fUsePgsuSettings;1 or 0;bit;
    0x486B, OutWord, "sprmCCpg", // ;;word;
    0x486D, OutWord, "sprmCRgLid0", // chp.rglid[0];LID: for non-Far East text;word;
    0x486E, OutWord, "sprmCRgLid1", // chp.rglid[1];LID: for Far East text;word;
    0x286F, OutByte, "sprmCIdctHint", // chp.idctHint;IDCT: (see below);byte;
    0x2E00, OutByte, "sprmPicBrcl", // pic.brcl;brcl (see PIC structure definition);byte;
    0xCE01, (FnOut)0,"sprmPicScale", // pic.mx, pic.my, pic.dxaCropleft, pic.dyaCropTop pic.dxaCropRight, pic.dyaCropBottom;Complex (see below);length byte plus 12 bytes;
    0x6C02,OutLongHex,"sprmPicBrcTop", // pic.brcTop;BRC;long;
    0x6C03,OutLongHex,"sprmPicBrcLeft", // pic.brcLeft;BRC;long;
    0x6C04,OutLongHex,"sprmPicBrcBottom", // pic.brcBottom;BRC;long;
    0x6C05,OutLongHex,"sprmPicBrcRight", // pic.brcRight;BRC;long;
    0x3000, OutByte, "sprmScnsPgn", // sep.cnsPgn;cns;byte;
    0x3001, OutByte, "sprmSiHeadingPgn", // sep.iHeadingPgn;heading number level;byte;
    0xD202, (FnOut)0,"sprmSOlstAnm", // sep.olstAnm;OLST;variable length;
    0xF203, (FnOut)0,"sprmSDxaColWidth", // sep.rgdxaColWidthSpacing;complex (see below);3 bytes;
    0xF204, (FnOut)0,"sprmSDxaColSpacing", // sep.rgdxaColWidthSpacing;complex (see below);3 bytes;
    0x3005, OutByte, "sprmSFEvenlySpaced", // sep.fEvenlySpaced;1 or 0;byte;
    0x3006, OutByte, "sprmSFProtected", // sep.fUnlocked;1 or 0;byte;
    0x5007, OutWord, "sprmSDmBinFirst", // sep.dmBinFirst;;word;
    0x5008, OutWord, "sprmSDmBinOther", // sep.dmBinOther;;word;
    0x3009, OutByte, "sprmSBkc", // sep.bkc;bkc;byte;
    0x300A, OutByte, "sprmSFTitlePage", // sep.fTitlePage;0 or 1;byte;
    0x500B, OutWord, "sprmSCcolumns", // sep.ccolM1;# of cols - 1;word;
    0x900C, OutWord, "sprmSDxaColumns", // sep.dxaColumns;dxa;word;
    0x300D, OutByte, "sprmSFAutoPgn", // sep.fAutoPgn;obsolete;byte;
    0x300E, OutByte, "sprmSNfcPgn", // sep.nfcPgn;nfc;byte;
    0xB00F, OutShort,"sprmSDyaPgn", // sep.dyaPgn;dya;short;
    0xB010, OutShort,"sprmSDxaPgn", // sep.dxaPgn;dya;short;
    0x3011, OutByte, "sprmSFPgnRestart", // sep.fPgnRestart;0 or 1;byte;
    0x3012, OutByte, "sprmSFEndnote", // sep.fEndnote;0 or 1;byte;
    0x3013, OutByte, "sprmSLnc", // sep.lnc;lnc;byte;
    0x3014, OutByte, "sprmSGprfIhdt", // sep.grpfIhdt;grpfihdt (see Headers and Footers topic);byte;
    0x5015, OutWord, "sprmSNLnnMod", // sep.nLnnMod;non-neg int.;word;
    0x9016, OutWord, "sprmSDxaLnn", // sep.dxaLnn;dxa;word;
    0xB017, OutWord, "sprmSDyaHdrTop", // sep.dyaHdrTop;dya;word;
    0xB018, OutWord, "sprmSDyaHdrBottom", // sep.dyaHdrBottom;dya;word;
    0x3019, OutByte, "sprmSLBetween", // sep.fLBetween;0 or 1;byte;
    0x301A, OutByte, "sprmSVjc", // sep.vjc;vjc;byte;
    0x501B, OutWord, "sprmSLnnMin", // sep.lnnMin;lnn;word;
    0x501C, OutWord, "sprmSPgnStart", // sep.pgnStart;pgn;word;
    0x301D, OutByte, "sprmSBOrientation", // sep.dmOrientPage;dm;byte;
//0x301E, ? ? ?  , "sprmSBCustomize", // ;;;
    0xB01F, OutWord, "sprmSXaPage", // sep.xaPage;xa;word;
    0xB020, OutWord, "sprmSYaPage", // sep.yaPage;ya;word;
    0xB021, OutWord, "sprmSDxaLeft", // sep.dxaLeft;dxa;word;
    0xB022, OutWord, "sprmSDxaRight", // sep.dxaRight;dxa;word;
    0x9023, OutWord, "sprmSDyaTop", // sep.dyaTop;dya;word;
    0x9024, OutWord, "sprmSDyaBottom", // sep.dyaBottom;dya;word;
    0xB025, OutWord, "sprmSDzaGutter", // sep.dzaGutter;dza;word;
    0x5026, OutWord, "sprmSDmPaperReq", // sep.dmPaperReq;dm;word;
    0xD227, (FnOut)0,"sprmSPropRMark", // sep.fPropRMark, sep.ibstPropRMark, sep.dttmPropRMark ;complex (see below);variable length always recorded as 7 bytes;
//0x3228, ? ? ?  , "sprmSFBiDi", // ;;;
//0x3229, ? ? ?  , "sprmSFFacingCol", // ;;;
//0x322A, ? ? ?  , "sprmSFRTLGutter", // ;;;
    0x702B,OutLongHex,"sprmSBrcTop", // sep.brcTop;BRC;long;
    0x702C,OutLongHex,"sprmSBrcLeft", // sep.brcLeft;BRC;long;
    0x702D,OutLongHex,"sprmSBrcBottom", // sep.brcBottom;BRC;long;
    0x702E,OutLongHex,"sprmSBrcRight", // sep.brcRight;BRC;long;
    0x522F, OutWord, "sprmSPgbProp", // sep.pgbProp;;word;
    0x7030,OutLongHex,"sprmSDxtCharSpace", // sep.dxtCharSpace;dxt;long;
    0x9031,OutLongHex,"sprmSDyaLinePitch", // sep.dyaLinePitch;dya;long;
//0x5032, ? ? ?  , "sprmSClm", // ;;;
    0x5033, OutShort,"sprmSTextFlow", // sep.wTextFlow;complex (see below);short;
    0x5400, OutWord, "sprmTJc", // tap.jc;jc;word (low order byte is significant);
    0x9601, OutWord, "sprmTDxaLeft", // tap.rgdxaCenter (see below);dxa;word;
    0x9602, OutWord, "sprmTDxaGapHalf", // tap.dxaGapHalf, tap.rgdxaCenter (see below);dxa;word;
    0x3403, OutByte, "sprmTFCantSplit", // tap.fCantSplit;1 or 0;byte;
    0x3404, OutByte, "sprmTTableHeader", // tap.fTableHeader;1 or 0;byte;
    0xD605, (FnOut)0,"sprmTTableBorders", // tap.rgbrcTable;complex(see below);24 bytes;
    0xD606, (FnOut)0,"sprmTDefTable10", // tap.rgdxaCenter, tap.rgtc;complex (see below);variable length;
    0x9407, OutShort, "sprmTDyaRowHeight", // tap.dyaRowHeight;dya;word;
    0xD608, OutTabD608,"sprmTDefTable", // tap.rgtc;complex (see below);;
    0xD609, OutTabD609,"sprmTDefTableShd", // tap.rgshd;complex (see below);;
    0x740A, (FnOut)0,"sprmTTlp", // tap.tlp;TLP;4 bytes;
//0x560B, ? ? ?  , "sprmTFBiDi", // ;;;
//0x740C, ? ? ?  , "sprmTHTMLProps", // ;;;
    0xD620, (FnOut)0,"sprmTSetBrc", // tap.rgtc[].rgbrc;complex (see below);5 bytes;
    0x7621, (FnOut)0,"sprmTInsert", // tap.rgdxaCenter, tap.rgtc;complex (see below);4 bytes;
    0x5622, OutWord, "sprmTDelete", // tap.rgdxaCenter, tap.rgtc;complex (see below);word;
    0x7623, (FnOut)0,"sprmTDxaCol", // tap.rgdxaCenter;complex (see below);4 bytes;
    0x5624, OutWord, "sprmTMerge", // tap.fFirstMerged, tap.fMerged;complex (see below);word;
    0x5625, OutWord, "sprmTSplit", // tap.fFirstMerged, tap.fMerged;complex (see below);word;
    0xD626, (FnOut)0,"sprmTSetBrc10", // tap.rgtc[].rgbrc;complex (see below);5 bytes;
    0x7627, (FnOut)0,"sprmTSetShd", // tap.rgshd;complex (see below);4 bytes;
    0x7628, (FnOut)0,"sprmTSetShdOdd", // tap.rgshd;complex (see below);4 bytes;
    0x7629, OutWord, "sprmTTextFlow", // tap.rgtc[].fVerticaltap.rgtc[].fBackwardtap.rgtc[].fRotateFont;0 or 10 or 10 or 1;word;
//0xD62A, ? ? ?  , "sprmTDiagLine", // ;;;
    0xD62B, (FnOut)0,"sprmTVertMerge", // tap.rgtc[].vertMerge;complex (see below);variable length always recorded as 2 bytes;
    0xD62C, (FnOut)0,"sprmTVertAlign", // tap.rgtc[].vertAlign;complex (see below);variable length always recorded as 3 byte;

    0x4873, OutWord, "sprm?? CJK ??", // CJK-Unknown
    0x4874, OutWord, "sprm?? CJK ??", // CJK-Unknown

};


static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC )
 _Optlink
#endif
    CompSprmDumpId( const void *pFirst, const void *pSecond)
{
    return( ((SprmDumpInfo*)pFirst )->nId - ((SprmDumpInfo*)pSecond)->nId );
}


SprmDumpInfo& WW8GetSprmDumpInfo( USHORT nId )
{
    // ggfs. Tab sortieren
    static BOOL bInit = FALSE;
    if( !bInit )
    {
        qsort( (void*)aSprmDumpTab,
            sizeof( aSprmDumpTab      ) / sizeof (aSprmDumpTab[ 0 ]),
            sizeof( aSprmDumpTab[ 0 ] ),
            CompSprmDumpId );
        bInit = TRUE;
    }
    // Sprm heraussuchen
    void* pFound;
    SprmDumpInfo aSrch;
    aSrch.nId = nId;
    if( 0 == ( pFound = bsearch( (char *) &aSrch,
                        (void*) aSprmDumpTab,
                        sizeof( aSprmDumpTab      ) / sizeof (aSprmDumpTab[ 0 ]),
                        sizeof( aSprmDumpTab[ 0 ] ),
                        CompSprmDumpId )))
    {
        // im Fehlerfall auf Nulltes Element verweisen
        pFound = (void*)aSprmDumpTab;
    }
    return *(SprmDumpInfo*) pFound;
}





/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/dump/ww8darr.cxx,v 1.2 2000-10-24 14:56:06 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:59  hr
      initial import

      Revision 1.8  2000/09/18 16:05:03  willem.vandorp
      OpenOffice header added.

      Revision 1.7  1999/10/28 20:35:11  jp
      write table shading vakues

      Revision 1.6  1999/10/28 18:13:47  jp
      write table height as short

      Revision 1.5  1999/10/27 17:57:53  jp
      changes for tables

      Revision 1.4  1999/08/16 10:22:38  JP
      neu: Dump EscherObjects


      Rev 1.3   16 Aug 1999 12:22:38   JP
   neu: Dump EscherObjects

      Rev 1.2   15 Jun 1999 14:12:36   JP
   new: ListTable and other Contents

      Rev 1.1   10 Jun 1998 17:22:32   KHZ
   Zwischenstand-Sicherung Dumper

      Rev 1.0   27 May 1998 15:30:10   KHZ
   Initial revision.


*************************************************************************/


