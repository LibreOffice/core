/*************************************************************************
 *
 *  $RCSfile: ww8scan.cxx,v $
 *
 *  $Revision: 1.101 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:44:49 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifdef PCH
#include "filt_pch.hxx"
#pragma hdrstop
#endif

#ifndef __SGI_STL_ALGORITHM
#include <algorithm>
#endif
#ifndef __SGI_STL_FUNCTIONAL
#include <functional>
#endif

#include <string.h>         // memset()

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifdef DUMP

#define ERR_SWG_READ_ERROR 1234
#define ASSERT( a, b )

#else                       // dump

#ifndef _SWSWERROR_H
#include <swerror.h>        // ERR_WW6_...
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>       // ASSERT()
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>      // DELETEZ
#endif

#endif                      // dump

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _WW8SCAN_HXX
#include "ww8scan.hxx"
#endif

#define ASSERT_RET_ON_FAIL( aCon, aError, aRet ) \
    ASSERT(aCon, aError); \
    if (!(aCon)) \
        return aRet;

//-begin
namespace SL
{
#   define IMPLCONSTSTRINGARRAY(X) const char a##X[] = "" #X ""
    IMPLCONSTSTRINGARRAY(ObjectPool);
    IMPLCONSTSTRINGARRAY(1Table);
    IMPLCONSTSTRINGARRAY(0Table);
    IMPLCONSTSTRINGARRAY(Data);
    IMPLCONSTSTRINGARRAY(CheckBox);
    IMPLCONSTSTRINGARRAY(ListBox);
    IMPLCONSTSTRINGARRAY(TextBox);
    IMPLCONSTSTRINGARRAY(TextField);
    IMPLCONSTSTRINGARRAY(MSMacroCmds);
}

template<class C> bool wwString<C>::TestBeltAndBraces(const SvStream& rStrm)
{
    bool bRet = false;
    sal_uInt32 nOldPos = rStrm.Tell();
    SvStream &rMutableStrm = const_cast<SvStream &>(rStrm);
    sal_uInt32 nLen = rMutableStrm.Seek(STREAM_SEEK_TO_END);
    rMutableStrm.Seek(nOldPos);
    C nBelt;
    rMutableStrm >> nBelt;
    nBelt *= sizeof(C);
    if (nOldPos + sizeof(C) + nBelt + sizeof(C) <= nLen &&
        !rStrm.GetError() && !rStrm.IsEof())
    {
        rMutableStrm.SeekRel(nBelt);
        if (!rStrm.GetError())
        {
            C cBraces;
            rMutableStrm >> cBraces;
            if (!rMutableStrm.GetError() && cBraces == 0)
                bRet = true;
        }
    }
    rMutableStrm.Seek(nOldPos);
    return bRet;
}

template<class C> class wwSortedArray
{
private:
    //The array e.g. of sprms.
    C *mpWwSprmTab;
    size_t mnNoElems;
public:
    wwSortedArray(C *pWwSprmTab, size_t nNoElems)
        : mpWwSprmTab(pWwSprmTab), mnNoElems(nNoElems)
    {
        ASSERT(mnNoElems && pWwSprmTab, "WW8: empty Array: Don't do that");
        std::sort(mpWwSprmTab, mpWwSprmTab + mnNoElems);
#if OSL_DEBUG_LEVEL > 1
        bool bBroken=false;
        rtl::OUString sError;
        const C *pIter = mpWwSprmTab;
        const C *pBeforeEnd = mpWwSprmTab + mnNoElems - 1;
        while (pIter < pBeforeEnd)
        {
            if (*pIter == *(pIter+1))
            {
                if (!bBroken)
                {
                    sError = rtl::OUString::createFromAscii(
                        "WW8: Duplicate in list, almost certainly don't want that!\n"
                        "(You will not see this message again unless you restart)\n"
                        "Extra entries are...\n");
                    bBroken=true;
                }

                size_t nSize = sizeof(C);
                const sal_uInt8 *pHack =
                    reinterpret_cast<const sal_uInt8 *>(&(*pIter));
                for (size_t i=0; i < nSize; ++i)
                {
                    sError += rtl::OUString::valueOf(
                        static_cast<sal_Int32>(pHack[i]), 16);
                    sError += rtl::OUString::valueOf(sal_Unicode(' '));
                }
                sError += rtl::OUString::valueOf(sal_Unicode('\n'));
                while (*pIter == *(pIter+1) && pIter < pBeforeEnd)
                    ++pIter;
            }
            else
                ++pIter;
        }
        if (bBroken)
            DbgError(rtl::OUStringToOString(sError, RTL_TEXTENCODING_ASCII_US));
#endif
    }

    //Find an entry, return its address if found and 0 if not
    const C *search(C aSrch) const;
};

template<class C> const C *wwSortedArray<C>::search(C aSrch) const
{
    std::pair<C *, C *> aPair =
        std::equal_range(mpWwSprmTab, mpWwSprmTab + mnNoElems, aSrch);
    if (aPair.first != aPair.second)
        return aPair.first;
    else
        return 0;
}

bool operator==(const SprmInfo &rFirst, const SprmInfo &rSecond)
{
    return (rFirst.nId == rSecond.nId);
}

bool operator<(const SprmInfo &rFirst, const SprmInfo &rSecond)
{
    return (rFirst.nId < rSecond.nId);
}

const wwSprmSearcher *wwSprmParser::GetWW6SprmSearcher()
{
    //double lock me
    // WW7- Sprms
    static SprmInfo aSprms[] =
    {
        {  0, 0, L_FIX}, // "Default-sprm",  wird uebersprungen
        {  2, 2, L_FIX}, // "sprmPIstd",  pap.istd (style code)
        {  3, 3, L_VAR}, // "sprmPIstdPermute pap.istd permutation
        {  4, 1, L_FIX}, // "sprmPIncLv1" pap.istddifference
        {  5, 1, L_FIX}, // "sprmPJc" pap.jc (justification)
        {  6, 1, L_FIX}, // "sprmPFSideBySide" pap.fSideBySide
        {  7, 1, L_FIX}, // "sprmPFKeep" pap.fKeep
        {  8, 1, L_FIX}, // "sprmPFKeepFollow " pap.fKeepFollow
        {  9, 1, L_FIX}, // "sprmPPageBreakBefore" pap.fPageBreakBefore
        { 10, 1, L_FIX}, // "sprmPBrcl" pap.brcl
        { 11, 1, L_FIX}, // "sprmPBrcp" pap.brcp
        { 12, 0, L_VAR}, // "sprmPAnld" pap.anld (ANLD structure)
        { 13, 1, L_FIX}, // "sprmPNLvlAnm" pap.nLvlAnm nn
        { 14, 1, L_FIX}, // "sprmPFNoLineNumb" pap.fNoLnn
        { 15, 0, L_VAR}, // "?sprmPChgTabsPapx" pap.itbdMac, ...
        { 16, 2, L_FIX}, // "sprmPDxaRight" pap.dxaRight
        { 17, 2, L_FIX}, // "sprmPDxaLeft" pap.dxaLeft
        { 18, 2, L_FIX}, // "sprmPNest" pap.dxaLeft
        { 19, 2, L_FIX}, // "sprmPDxaLeft1" pap.dxaLeft1
        { 20, 4, L_FIX}, // "sprmPDyaLine" pap.lspd an LSPD
        { 21, 2, L_FIX}, // "sprmPDyaBefore" pap.dyaBefore
        { 22, 2, L_FIX}, // "sprmPDyaAfter" pap.dyaAfter
        { 23, 0, L_VAR}, // "?sprmPChgTabs" pap.itbdMac, pap.rgdxaTab, ...
        { 24, 1, L_FIX}, // "sprmPFInTable" pap.fInTable
        { 25, 1, L_FIX}, // "sprmPTtp" pap.fTtp
        { 26, 2, L_FIX}, // "sprmPDxaAbs" pap.dxaAbs
        { 27, 2, L_FIX}, // "sprmPDyaAbs" pap.dyaAbs
        { 28, 2, L_FIX}, // "sprmPDxaWidth" pap.dxaWidth
        { 29, 1, L_FIX}, // "sprmPPc" pap.pcHorz, pap.pcVert
        { 30, 2, L_FIX}, // "sprmPBrcTop10" pap.brcTop BRC10
        { 31, 2, L_FIX}, // "sprmPBrcLeft10" pap.brcLeft BRC10
        { 32, 2, L_FIX}, // "sprmPBrcBottom10" pap.brcBottom BRC10
        { 33, 2, L_FIX}, // "sprmPBrcRight10" pap.brcRight BRC10
        { 34, 2, L_FIX}, // "sprmPBrcBetween10" pap.brcBetween BRC10
        { 35, 2, L_FIX}, // "sprmPBrcBar10" pap.brcBar BRC10
        { 36, 2, L_FIX}, // "sprmPFromText10" pap.dxaFromText dxa
        { 37, 1, L_FIX}, // "sprmPWr" pap.wr wr
        { 38, 2, L_FIX}, // "sprmPBrcTop" pap.brcTop BRC
        { 39, 2, L_FIX}, // "sprmPBrcLeft" pap.brcLeft BRC
        { 40, 2, L_FIX}, // "sprmPBrcBottom" pap.brcBottom BRC
        { 41, 2, L_FIX}, // "sprmPBrcRight" pap.brcRight BRC
        { 42, 2, L_FIX}, // "sprmPBrcBetween" pap.brcBetween BRC
        { 43, 2, L_FIX}, // "sprmPBrcBar" pap.brcBar BRC word
        { 44, 1, L_FIX}, // "sprmPFNoAutoHyph" pap.fNoAutoHyph
        { 45, 2, L_FIX}, // "sprmPWHeightAbs" pap.wHeightAbs w
        { 46, 2, L_FIX}, // "sprmPDcs" pap.dcs DCS
        { 47, 2, L_FIX}, // "sprmPShd" pap.shd SHD
        { 48, 2, L_FIX}, // "sprmPDyaFromText" pap.dyaFromText dya
        { 49, 2, L_FIX}, // "sprmPDxaFromText" pap.dxaFromText dxa
        { 50, 1, L_FIX}, // "sprmPFLocked" pap.fLocked 0 or 1 byte
        { 51, 1, L_FIX}, // "sprmPFWidowControl" pap.fWidowControl 0 or 1 byte
        { 52, 0, L_FIX}, // "?sprmPRuler 52"
        { 64, 0, L_VAR}, // rtl property ?
        { 65, 1, L_FIX}, // "sprmCFStrikeRM" chp.fRMarkDel 1 or 0 bit
        { 66, 1, L_FIX}, // "sprmCFRMark" chp.fRMark 1 or 0 bit
        { 67, 1, L_FIX}, // "sprmCFFldVanish" chp.fFldVanish 1 or 0 bit
        { 68, 0, L_VAR}, // "sprmCPicLocation" chp.fcPic and chp.fSpec
        { 69, 2, L_FIX}, // "sprmCIbstRMark" chp.ibstRMark index into sttbRMark
        { 70, 4, L_FIX}, // "sprmCDttmRMark" chp.dttm DTTM long
        { 71, 1, L_FIX}, // "sprmCFData" chp.fData 1 or 0 bit
        { 72, 2, L_FIX}, // "sprmCRMReason" chp.idslRMReason an index to a table
        { 73, 3, L_FIX}, // "sprmCChse" chp.fChsDiff and chp.chse
        { 74, 0, L_VAR}, // "sprmCSymbol" chp.fSpec, chp.chSym and chp.ftcSym
        { 75, 1, L_FIX}, // "sprmCFOle2" chp.fOle2 1 or 0   bit
        { 79, 0, L_VAR}, // unknown
        { 80, 2, L_FIX}, // "sprmCIstd" chp.istd istd, see stylesheet definition
        { 81, 0, L_VAR}, // "sprmCIstdPermute" chp.istd permutation vector
        { 82, 0, L_VAR}, // "sprmCDefault" whole CHP
        { 83, 0, L_FIX}, // "sprmCPlain" whole CHP
        { 85, 1, L_FIX}, // "sprmCFBold" chp.fBold 0,1, 128, or 129
        { 86, 1, L_FIX}, // "sprmCFItalic" chp.fItalic 0,1, 128, or 129
        { 87, 1, L_FIX}, // "sprmCFStrike" chp.fStrike 0,1, 128, or 129
        { 88, 1, L_FIX}, // "sprmCFOutline" chp.fOutline 0,1, 128, or 129
        { 89, 1, L_FIX}, // "sprmCFShadow" chp.fShadow 0,1, 128, or 129
        { 90, 1, L_FIX}, // "sprmCFSmallCaps" chp.fSmallCaps 0,1, 128, or 129
        { 91, 1, L_FIX}, // "sprmCFCaps" chp.fCaps 0,1, 128, or 129
        { 92, 1, L_FIX}, // "sprmCFVanish" chp.fVanish 0,1, 128, or 129
        { 93, 2, L_FIX}, // "sprmCFtc" chp.ftc ftc word
        { 94, 1, L_FIX}, // "sprmCKul" chp.kul kul byte
        { 95, 3, L_FIX}, // "sprmCSizePos" chp.hps, chp.hpsPos
        { 96, 2, L_FIX}, // "sprmCDxaSpace" chp.dxaSpace dxa
        { 97, 2, L_FIX}, // "sprmCLid" chp.lid LID
        { 98, 1, L_FIX}, // "sprmCIco" chp.ico ico byte
        { 99, 2, L_FIX}, // "sprmCHps" chp.hps hps !word!
        {100, 1, L_FIX}, // "sprmCHpsInc" chp.hps
        {101, 2, L_FIX}, // "sprmCHpsPos" chp.hpsPos hps !word!
        {102, 1, L_FIX}, // "sprmCHpsPosAdj" chp.hpsPos hps
        {103, 0, L_VAR}, // "?sprmCMajority" chp.fBold, chp.fItalic, ...
        {104, 1, L_FIX}, // "sprmCIss" chp.iss iss
        {105, 0, L_VAR}, // "sprmCHpsNew50" chp.hps hps variable width
        {106, 0, L_VAR}, // "sprmCHpsInc1" chp.hps complex
        {107, 2, L_FIX}, // "sprmCHpsKern" chp.hpsKern hps
        {108, 0, L_VAR}, // "sprmCMajority50" chp.fBold, chp.fItalic, ...
        {109, 2, L_FIX}, // "sprmCHpsMul" chp.hps percentage to grow hps
        {110, 2, L_FIX}, // "sprmCCondHyhen" chp.ysri ysri
        {111, 2, L_FIX}, // rtl bold
        {112, 2, L_FIX}, // rtl italic
        {113, 0, L_VAR}, // rtl property ?
        {115, 0, L_VAR}, // rtl property ?
        {116, 0, L_VAR}, // unknown
        {117, 1, L_FIX}, // "sprmCFSpec" chp.fSpec  1 or 0 bit
        {118, 1, L_FIX}, // "sprmCFObj" chp.fObj 1 or 0 bit
        {119, 1, L_FIX}, // "sprmPicBrcl" pic.brcl brcl (see PIC definition)
        {120,12, L_VAR}, // "sprmPicScale" pic.mx, pic.my, pic.dxaCropleft,
        {121, 2, L_FIX}, // "sprmPicBrcTop" pic.brcTop BRC word
        {122, 2, L_FIX}, // "sprmPicBrcLeft" pic.brcLeft BRC word
        {123, 2, L_FIX}, // "sprmPicBrcBottom" pic.brcBottom BRC word
        {124, 2, L_FIX}, // "sprmPicBrcRight" pic.brcRight BRC word
        {131, 1, L_FIX}, // "sprmSScnsPgn" sep.cnsPgn cns byte
        {132, 1, L_FIX}, // "sprmSiHeadingPgn" sep.iHeadingPgn
        {133, 0, L_VAR}, // "sprmSOlstAnm" sep.olstAnm OLST variable length
        {136, 3, L_FIX}, // "sprmSDxaColWidth" sep.rgdxaColWidthSpacing complex
        {137, 3, L_FIX}, // "sprmSDxaColSpacing" sep.rgdxaColWidthSpacing
        {138, 1, L_FIX}, // "sprmSFEvenlySpaced" sep.fEvenlySpaced 1 or 0
        {139, 1, L_FIX}, // "sprmSFProtected" sep.fUnlocked 1 or 0 byte
        {140, 2, L_FIX}, // "sprmSDmBinFirst" sep.dmBinFirst  word
        {141, 2, L_FIX}, // "sprmSDmBinOther" sep.dmBinOther  word
        {142, 1, L_FIX}, // "sprmSBkc" sep.bkc bkc byte
        {143, 1, L_FIX}, // "sprmSFTitlePage" sep.fTitlePage 0 or 1 byte
        {144, 2, L_FIX}, // "sprmSCcolumns" sep.ccolM1 # of cols - 1 word
        {145, 2, L_FIX}, // "sprmSDxaColumns" sep.dxaColumns dxa word
        {146, 1, L_FIX}, // "sprmSFAutoPgn" sep.fAutoPgn obsolete byte
        {147, 1, L_FIX}, // "sprmSNfcPgn" sep.nfcPgn nfc byte
        {148, 2, L_FIX}, // "sprmSDyaPgn" sep.dyaPgn dya short
        {149, 2, L_FIX}, // "sprmSDxaPgn" sep.dxaPgn dya short
        {150, 1, L_FIX}, // "sprmSFPgnRestart" sep.fPgnRestart 0 or 1 byte
        {151, 1, L_FIX}, // "sprmSFEndnote" sep.fEndnote 0 or 1 byte
        {152, 1, L_FIX}, // "sprmSLnc" sep.lnc lnc byte
        {153, 1, L_FIX}, // "sprmSGprfIhdt" sep.grpfIhdt grpfihdt
        {154, 2, L_FIX}, // "sprmSNLnnMod" sep.nLnnMod non-neg int. word
        {155, 2, L_FIX}, // "sprmSDxaLnn" sep.dxaLnn dxa word
        {156, 2, L_FIX}, // "sprmSDyaHdrTop" sep.dyaHdrTop dya word
        {157, 2, L_FIX}, // "sprmSDyaHdrBottom" sep.dyaHdrBottom dya word
        {158, 1, L_FIX}, // "sprmSLBetween" sep.fLBetween 0 or 1 byte
        {159, 1, L_FIX}, // "sprmSVjc" sep.vjc vjc byte
        {160, 2, L_FIX}, // "sprmSLnnMin" sep.lnnMin lnn word
        {161, 2, L_FIX}, // "sprmSPgnStart" sep.pgnStart pgn word
        {162, 1, L_FIX}, // "sprmSBOrientation" sep.dmOrientPage dm byte
        {163, 0, L_FIX}, // "?SprmSBCustomize 163"
        {164, 2, L_FIX}, // "sprmSXaPage" sep.xaPage xa word
        {165, 2, L_FIX}, // "sprmSYaPage" sep.yaPage ya word
        {166, 2, L_FIX}, // "sprmSDxaLeft" sep.dxaLeft dxa word
        {167, 2, L_FIX}, // "sprmSDxaRight" sep.dxaRight dxa word
        {168, 2, L_FIX}, // "sprmSDyaTop" sep.dyaTop dya word
        {169, 2, L_FIX}, // "sprmSDyaBottom" sep.dyaBottom dya word
        {170, 2, L_FIX}, // "sprmSDzaGutter" sep.dzaGutter dza word
        {171, 2, L_FIX}, // "sprmSDMPaperReq" sep.dmPaperReq dm word
        {179, 0, L_VAR}, // rtl property ?
        {181, 0, L_VAR}, // rtl property ?
        {182, 2, L_FIX}, // "sprmTJc" tap.jc jc (low order byte is significant)
        {183, 2, L_FIX}, // "sprmTDxaLeft" tap.rgdxaCenter dxa word
        {184, 2, L_FIX}, // "sprmTDxaGapHalf" tap.dxaGapHalf, tap.rgdxaCenter
        {185, 1, L_FIX}, // "sprmTFCantSplit" tap.fCantSplit 1 or 0 byte
        {186, 1, L_FIX}, // "sprmTTableHeader" tap.fTableHeader 1 or 0 byte
        {187,12, L_FIX}, // "sprmTTableBorders" tap.rgbrcTable complex 12 bytes
        {188, 0, L_VAR}, // "sprmTDefTable10" tap.rgdxaCenter, tap.rgtc complex
        {189, 2, L_FIX}, // "sprmTDyaRowHeight" tap.dyaRowHeight dya word
        {190, 0, L_VAR2},// "sprmTDefTable" tap.rgtc complex
        {191, 1, L_VAR}, // "sprmTDefTableShd" tap.rgshd complex
        {192, 4, L_FIX}, // "sprmTTlp" tap.tlp TLP 4 bytes
        {193, 5, L_FIX}, // "sprmTSetBrc" tap.rgtc[].rgbrc complex 5 bytes
        {194, 4, L_FIX}, // "sprmTInsert" tap.rgdxaCenter,tap.rgtc complex
        {195, 2, L_FIX}, // "sprmTDelete" tap.rgdxaCenter, tap.rgtc complex
        {196, 4, L_FIX}, // "sprmTDxaCol" tap.rgdxaCenter complex
        {197, 2, L_FIX}, // "sprmTMerge" tap.fFirstMerged, tap.fMerged complex
        {198, 2, L_FIX}, // "sprmTSplit" tap.fFirstMerged, tap.fMerged complex
        {199, 5, L_FIX}, // "sprmTSetBrc10" tap.rgtc[].rgbrc complex 5 bytes
        {200, 4, L_FIX}, // "sprmTSetShd", tap.rgshd complex 4 bytes
        {207, 0, L_VAR}  // rtl property ?
    };

    static wwSprmSearcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
};

const wwSprmSearcher *wwSprmParser::GetWW8SprmSearcher()
{
    //double lock me
    //WW8+ Sprms
    static SprmInfo aSprms[] =
    {
        {     0, 0, L_FIX}, // "Default-sprm"/ wird uebersprungen
        {0x4600, 2, L_FIX}, // "sprmPIstd" pap.istd;istd (style code);short;
        {0xC601, 0, L_VAR}, // "sprmPIstdPermute" pap.istd;permutation vector
        {0x2602, 1, L_FIX}, // "sprmPIncLvl" pap.istd, pap.lvl;difference
                            // between istd of base PAP and istd of PAP to be
                            // produced
        {0x2403, 1, L_FIX}, // "sprmPJc" pap.jc;jc (justification);byte;
        {0x2404, 1, L_FIX}, // "sprmPFSideBySide" pap.fSideBySide;0 or 1;byte;
        {0x2405, 1, L_FIX}, // "sprmPFKeep" pap.fKeep;0 or 1;byte;
        {0x2406, 1, L_FIX}, // "sprmPFKeepFollow" pap.fKeepFollow;0 or 1;byte;
        {0x2407, 1, L_FIX}, // "sprmPFPageBreakBefore" pap.fPageBreakBefore;
                            // 0 or 1
        {0x2408, 1, L_FIX}, // "sprmPBrcl" pap.brcl;brcl;byte;
        {0x2409, 1, L_FIX}, // "sprmPBrcp" pap.brcp;brcp;byte;
        {0x260A, 1, L_FIX}, // "sprmPIlvl" pap.ilvl;ilvl;byte;
        {0x460B, 2, L_FIX}, // "sprmPIlfo" pap.ilfo;ilfo (list index) ;short;
        {0x240C, 1, L_FIX}, // "sprmPFNoLineNumb" pap.fNoLnn;0 or 1;byte;
        {0xC60D, 0, L_VAR}, // "sprmPChgTabsPapx" pap.itbdMac, pap.rgdxaTab,
                            // pap.rgtbd;complex
        {0x840E, 2, L_FIX}, // "sprmPDxaRight" pap.dxaRight;dxa;word;
        {0x840F, 2, L_FIX}, // "sprmPDxaLeft" pap.dxaLeft;dxa;word;
        {0x4610, 2, L_FIX}, // "sprmPNest" pap.dxaLeft;dxa
        {0x8411, 2, L_FIX}, // "sprmPDxaLeft1" pap.dxaLeft1;dxa;word;
        {0x6412, 4, L_FIX}, // "sprmPDyaLine" pap.lspd;an LSPD, a long word
                            // structure consisting of a short of dyaLine
                            // followed by a short of fMultLinespace
        {0xA413, 2, L_FIX}, // "sprmPDyaBefore" pap.dyaBefore;dya;word;
        {0xA414, 2, L_FIX}, // "sprmPDyaAfter" pap.dyaAfter;dya;word;
        {0xC615, 0, L_VAR}, // "sprmPChgTabs" pap.itbdMac, pap.rgdxaTab,
                            // pap.rgtbd;complex
        {0x2416, 1, L_FIX}, // "sprmPFInTable" pap.fInTable;0 or 1;byte;
        {0x2417, 1, L_FIX}, // "sprmPFTtp" pap.fTtp;0 or 1;byte;
        {0x8418, 2, L_FIX}, // "sprmPDxaAbs" pap.dxaAbs;dxa;word;
        {0x8419, 2, L_FIX}, // "sprmPDyaAbs" pap.dyaAbs;dya;word;
        {0x841A, 2, L_FIX}, // "sprmPDxaWidth" pap.dxaWidth;dxa;word;
        {0x261B, 1, L_FIX}, // "sprmPPc" pap.pcHorz, pap.pcVert;complex
        {0x461C, 2, L_FIX}, // "sprmPBrcTop10" pap.brcTop;BRC10;word;
        {0x461D, 2, L_FIX}, // "sprmPBrcLeft10" pap.brcLeft;BRC10;word;
        {0x461E, 2, L_FIX}, // "sprmPBrcBottom10" pap.brcBottom;BRC10;word;
        {0x461F, 2, L_FIX}, // "sprmPBrcRight10" pap.brcRight;BRC10;word;
        {0x4620, 2, L_FIX}, // "sprmPBrcBetween10" pap.brcBetween;BRC10;word;
        {0x4621, 2, L_FIX}, // "sprmPBrcBar10" pap.brcBar;BRC10;word;
        {0x4622, 2, L_FIX}, // "sprmPDxaFromText10" pap.dxaFromText;dxa;word;
        {0x2423, 1, L_FIX}, // "sprmPWr" pap.wr;wr
        {0x6424, 4, L_FIX}, // "sprmPBrcTop" pap.brcTop;BRC;long;
        {0x6425, 4, L_FIX}, // "sprmPBrcLeft" pap.brcLeft;BRC;long;
        {0x6426, 4, L_FIX}, // "sprmPBrcBottom" pap.brcBottom;BRC;long;
        {0x6427, 4, L_FIX}, // "sprmPBrcRight" pap.brcRight;BRC;long;
        {0x6428, 4, L_FIX}, // "sprmPBrcBetween" pap.brcBetween;BRC;long;
        {0x6629, 4, L_FIX}, // "sprmPBrcBar" pap.brcBar;BRC;long;
        {0x242A, 1, L_FIX}, // "sprmPFNoAutoHyph" pap.fNoAutoHyph;0 or 1;byte;
        {0x442B, 2, L_FIX}, // "sprmPWHeightAbs" pap.wHeightAbs;w;word;
        {0x442C, 2, L_FIX}, // "sprmPDcs" pap.dcs;DCS;short;
        {0x442D, 2, L_FIX}, // "sprmPShd" pap.shd;SHD;word;
        {0x842E, 2, L_FIX}, // "sprmPDyaFromText" pap.dyaFromText;dya;word;
        {0x842F, 2, L_FIX}, // "sprmPDxaFromText" pap.dxaFromText;dxa;word;
        {0x2430, 1, L_FIX}, // "sprmPFLocked" pap.fLocked;0 or 1;byte;
        {0x2431, 1, L_FIX}, // "sprmPFWidowControl" pap.fWidowControl;0 or 1
        {0xC632, 0, L_VAR}, // "sprmPRuler" ;;variable length;
        {0x2433, 1, L_FIX}, // "sprmPFKinsoku" pap.fKinsoku;0 or 1;byte;
        {0x2434, 1, L_FIX}, // "sprmPFWordWrap" pap.fWordWrap;0 or 1;byte;
        {0x2435, 1, L_FIX}, // "sprmPFOverflowPunct" pap.fOverflowPunct;0 or 1
        {0x2436, 1, L_FIX}, // "sprmPFTopLinePunct" pap.fTopLinePunct;0 or 1
        {0x2437, 1, L_FIX}, // "sprmPFAutoSpaceDE" pap.fAutoSpaceDE;0 or 1
        {0x2438, 1, L_FIX}, // "sprmPFAutoSpaceDN" pap.fAutoSpaceDN;0 or 1
        {0x4439, 2, L_FIX}, // "sprmPWAlignFont" pap.wAlignFont;iFa
        {0x443A, 2, L_FIX}, // "sprmPFrameTextFlow" pap.fVertical pap.fBackward
                            // pap.fRotateFont;complex
        {0x243B, 1, L_FIX}, // "sprmPISnapBaseLine" obsolete: not applicable in
                            // Word97 and later versions;
        {0xC63E, 0, L_VAR}, // "sprmPAnld" pap.anld;;variable length;
        {0xC63F, 0, L_VAR}, // "sprmPPropRMark" pap.fPropRMark;complex
        {0x2640, 1, L_FIX}, // "sprmPOutLvl" pap.lvl;has no effect if pap.istd
                            // is < 1 or is > 9
        {0x2441, 1, L_FIX}, // "sprmPFBiDi" ;;byte;
        {0x2443, 1, L_FIX}, // "sprmPFNumRMIns" pap.fNumRMIns;1 or 0;bit;
        {0x2444, 1, L_FIX}, // "sprmPCrLf" ;;byte;
        {0xC645, 0, L_VAR}, // "sprmPNumRM" pap.numrm;;variable length;
        {0x6645, 4, L_FIX}, // "sprmPHugePapx" fc in the data stream to locate
                            // the huge grpprl
        {0x6646, 4, L_FIX}, // "sprmPHugePapx" fc in the data stream to locate
                            // the huge grpprl
        {0x2447, 1, L_FIX}, // "sprmPFUsePgsuSettings" pap.fUsePgsuSettings;
                            // 1 or 0
        {0x2448, 1, L_FIX}, // "sprmPFAdjustRight" pap.fAdjustRight;1 or 0;byte;
        {0x0800, 1, L_FIX}, // "sprmCFRMarkDel" chp.fRMarkDel;1 or 0;bit;
        {0x0801, 1, L_FIX}, // "sprmCFRMark" chp.fRMark;1 or 0;bit;
        {0x0802, 1, L_FIX}, // "sprmCFFldVanish" chp.fFldVanish;1 or 0;bit;
        {0x6A03, 4, L_FIX}, // "sprmCPicLocation" chp.fcPic and chp.fSpec;
        {0x4804, 2, L_FIX}, // "sprmCIbstRMark" chp.ibstRMark;index into
                            // sttbRMark
        {0x6805, 4, L_FIX}, // "sprmCDttmRMark" chp.dttmRMark;DTTM;long;
        {0x0806, 1, L_FIX}, // "sprmCFData" chp.fData;1 or 0;bit;
        {0x4807, 2, L_FIX}, // "sprmCIdslRMark" chp.idslRMReason;an index to a
                            // table of strings defined in Word 6.0
                            // executables;short;
        {0xEA08, 1, L_FIX}, // "sprmCChs" chp.fChsDiff and chp.chse;
        {0x6A09, 4, L_FIX}, // "sprmCSymbol" chp.fSpec, chp.xchSym and
                            // chp.ftcSym
        {0x080A, 1, L_FIX}, // "sprmCFOle2" chp.fOle2;1 or 0;bit;
        {0x480B, 0, L_FIX}, // "sprmCIdCharType" obsolete: not applicable in
                            // Word97 and later versions;;;
        {0x2A0C, 1, L_FIX}, // "sprmCHighlight" chp.fHighlight,
                            // chp.icoHighlight;ico (fHighlight is set to 1 iff
                            // ico is not 0)
        {0x680E, 4, L_FIX}, // "sprmCObjLocation" chp.fcObj;FC;long;
        {0x2A10, 0, L_FIX}, // "sprmCFFtcAsciSymb" ;;;
        {0x4A30, 2, L_FIX}, // "sprmCIstd" chp.istd;istd, see stylesheet def
        {0xCA31, 0, L_VAR}, // "sprmCIstdPermute" chp.istd;permutation vector
        {0x2A32, 0, L_VAR}, // "sprmCDefault" whole CHP;none;variable length;
        {0x2A33, 0, L_FIX}, // "sprmCPlain" whole CHP;none;0;
        {0x2A34, 1, L_FIX}, // "sprmCKcd" ;;;
        {0x0835, 1, L_FIX}, // "sprmCFBold" chp.fBold;0,1, 128, or 129
        {0x0836, 1, L_FIX}, // "sprmCFItalic" chp.fItalic;0,1, 128, or 129
        {0x0837, 1, L_FIX}, // "sprmCFStrike" chp.fStrike;0,1, 128, or 129
        {0x0838, 1, L_FIX}, // "sprmCFOutline" chp.fOutline;0,1, 128, or 129
        {0x0839, 1, L_FIX}, // "sprmCFShadow" chp.fShadow;0,1, 128, or 129
        {0x083A, 1, L_FIX}, // "sprmCFSmallCaps" chp.fSmallCaps;0,1, 128, or 129
        {0x083B, 1, L_FIX}, // "sprmCFCaps" chp.fCaps;0,1, 128, or 129
        {0x083C, 1, L_FIX}, // "sprmCFVanish" chp.fVanish;0,1, 128, or 129
        {0x4A3D, 2, L_FIX}, // "sprmCFtcDefault" ;ftc, only used internally
        {0x2A3E, 1, L_FIX}, // "sprmCKul" chp.kul;kul;byte;
        {0xEA3F, 3, L_FIX}, // "sprmCSizePos" chp.hps, chp.hpsPos;3 bytes;
        {0x8840, 2, L_FIX}, // "sprmCDxaSpace" chp.dxaSpace;dxa;word;
        {0x4A41, 2, L_FIX}, // "sprmCLid" ;only used internally never stored
        {0x2A42, 1, L_FIX}, // "sprmCIco" chp.ico;ico;byte;
        {0x4A43, 2, L_FIX}, // "sprmCHps" chp.hps;hps
        {0x2A44, 1, L_FIX}, // "sprmCHpsInc" chp.hps;
        {0x4845, 2, L_FIX}, // "sprmCHpsPos" chp.hpsPos;hps;short; (doc wrong)
        {0x2A46, 1, L_FIX}, // "sprmCHpsPosAdj" chp.hpsPos;hps
        {0xCA47, 0, L_VAR}, // "sprmCMajority" chp.fBold, chp.fItalic,
                            // chp.fSmallCaps, chp.fVanish, chp.fStrike,
                            // chp.fCaps, chp.rgftc, chp.hps, chp.hpsPos,
                            // chp.kul, chp.dxaSpace, chp.ico,
                            // chp.rglid;complex;variable length, length byte
                            // plus size of following grpprl;
        {0x2A48, 1, L_FIX}, // "sprmCIss" chp.iss;iss;byte;
        {0xCA49, 0, L_VAR}, // "sprmCHpsNew50" chp.hps;hps;variable width
        {0xCA4A, 0, L_VAR}, // "sprmCHpsInc1" chp.hps;complex
        {0x484B, 2, L_FIX}, // "sprmCHpsKern" chp.hpsKern;hps;short;
        {0xCA4C, 2, L_FIX}, // "sprmCMajority50" chp.fBold, chp.fItalic,
                            // chp.fSmallCaps, chp.fVanish, chp.fStrike,
                            // chp.fCaps, chp.ftc, chp.hps, chp.hpsPos, chp.kul,
                            // chp.dxaSpace, chp.ico,;complex
        {0x4A4D, 2, L_FIX}, // "sprmCHpsMul" chp.hps;percentage to grow hps
        {0x484E, 2, L_FIX}, // "sprmCYsri" chp.ysri;ysri;short;
        {0x4A4F, 2, L_FIX}, // "sprmCRgFtc0" chp.rgftc[0];ftc for ASCII text
        {0x4A50, 2, L_FIX}, // "sprmCRgFtc1" chp.rgftc[1];ftc for Far East text
        {0x4A51, 2, L_FIX}, // "sprmCRgFtc2" chp.rgftc[2];ftc for non-FE text
        {0x4852, 2, L_FIX}, // "sprmCCharScale"
        {0x2A53, 1, L_FIX}, // "sprmCFDStrike" chp.fDStrike;;byte;
        {0x0854, 1, L_FIX}, // "sprmCFImprint" chp.fImprint;1 or 0;bit;
        {0x0855, 1, L_FIX}, // "sprmCFSpec" chp.fSpec ;1 or 0;bit;
        {0x0856, 1, L_FIX}, // "sprmCFObj" chp.fObj;1 or 0;bit;
        {0xCA57, 0, L_VAR}, // "sprmCPropRMark" chp.fPropRMark,
                            // chp.ibstPropRMark, chp.dttmPropRMark;Complex
        {0x0858, 1, L_FIX}, // "sprmCFEmboss" chp.fEmboss;1 or 0;bit;
        {0x2859, 1, L_FIX}, // "sprmCSfxText" chp.sfxtText;text animation;byte;
        {0x085A, 1, L_FIX}, // "sprmCFBiDi" ;;;
        {0x085B, 1, L_FIX}, // "sprmCFDiacColor" ;;;
        {0x085C, 1, L_FIX}, // "sprmCFBoldBi" ;;;
        {0x085D, 1, L_FIX}, // "sprmCFItalicBi" ;;;
        {0x4A5E, 2, L_FIX},
        {0x485F, 2, L_FIX}, // "sprmCLidBi" ;;;
        {0x4A60, 1, L_FIX}, // "sprmCIcoBi" ;;;
        {0x4A61, 2, L_FIX}, // "sprmCHpsBi" ;;;
        {0xCA62, 0, L_VAR}, // "sprmCDispFldRMark" chp.fDispFldRMark,
                            // chp.ibstDispFldRMark, chp.dttmDispFldRMark ;
        {0x4863, 2, L_FIX}, // "sprmCIbstRMarkDel" chp.ibstRMarkDel;index into
                            // sttbRMark;short;
        {0x6864, 4, L_FIX}, // "sprmCDttmRMarkDel" chp.dttmRMarkDel;DTTM;long;
        {0x6865, 4, L_FIX}, // "sprmCBrc" chp.brc;BRC;long;
        {0x4866, 2, L_FIX}, // "sprmCShd" chp.shd;SHD;short;
        {0x4867, 2, L_FIX}, // "sprmCIdslRMarkDel" chp.idslRMReasonDel;an index
                            // to a table of strings defined in Word 6.0
                            // executables;short;
        {0x0868, 1, L_FIX}, // "sprmCFUsePgsuSettings"
                            // chp.fUsePgsuSettings;1 or 0
        {0x486B, 2, L_FIX}, // "sprmCCpg" ;;word;
        {0x486D, 2, L_FIX}, // "sprmCRgLid0" chp.rglid[0];LID: for non-FE text
        {0x486E, 2, L_FIX}, // "sprmCRgLid1" chp.rglid[1];LID: for Far East text
        {0x286F, 1, L_FIX}, // "sprmCIdctHint" chp.idctHint;IDCT:
        {0x2E00, 1, L_FIX}, // "sprmPicBrcl" pic.brcl;brcl (see PIC definition)
        {0xCE01, 0, L_VAR}, // "sprmPicScale" pic.mx, pic.my, pic.dxaCropleft,
                            // pic.dyaCropTop pic.dxaCropRight,
                            // pic.dyaCropBottom;Complex
        {0x6C02, 4, L_FIX}, // "sprmPicBrcTop" pic.brcTop;BRC;long;
        {0x6C03, 4, L_FIX}, // "sprmPicBrcLeft" pic.brcLeft;BRC;long;
        {0x6C04, 4, L_FIX}, // "sprmPicBrcBottom" pic.brcBottom;BRC;long;
        {0x6C05, 4, L_FIX}, // "sprmPicBrcRight" pic.brcRight;BRC;long;
        {0x3000, 1, L_FIX}, // "sprmScnsPgn" sep.cnsPgn;cns;byte;
        {0x3001, 1, L_FIX}, // "sprmSiHeadingPgn" sep.iHeadingPgn;heading number
                            // level;byte;
        {0xD202, 0, L_VAR}, // "sprmSOlstAnm" sep.olstAnm;OLST;variable length;
        {0xF203, 3, L_FIX}, // "sprmSDxaColWidth" sep.rgdxaColWidthSpacing;
        {0xF204, 3, L_FIX}, // "sprmSDxaColSpacing" sep.rgdxaColWidthSpacing;
                            // complex
        {0x3005, 1, L_FIX}, // "sprmSFEvenlySpaced" sep.fEvenlySpaced;1 or 0
        {0x3006, 1, L_FIX}, // "sprmSFProtected" sep.fUnlocked;1 or 0;byte;
        {0x5007, 2, L_FIX}, // "sprmSDmBinFirst" sep.dmBinFirst;;word;
        {0x5008, 2, L_FIX}, // "sprmSDmBinOther" sep.dmBinOther;;word;
        {0x3009, 1, L_FIX}, // "sprmSBkc" sep.bkc;bkc;byte;
        {0x300A, 1, L_FIX}, // "sprmSFTitlePage" sep.fTitlePage;0 or 1;byte;
        {0x500B, 2, L_FIX}, // "sprmSCcolumns" sep.ccolM1;# of cols - 1;word;
        {0x900C, 2, L_FIX}, // "sprmSDxaColumns" sep.dxaColumns;dxa;word;
        {0x300D, 1, L_FIX}, // "sprmSFAutoPgn" sep.fAutoPgn;obsolete;byte;
        {0x300E, 1, L_FIX}, // "sprmSNfcPgn" sep.nfcPgn;nfc;byte;
        {0xB00F, 2, L_FIX}, // "sprmSDyaPgn" sep.dyaPgn;dya;short;
        {0xB010, 2, L_FIX}, // "sprmSDxaPgn" sep.dxaPgn;dya;short;
        {0x3011, 1, L_FIX}, // "sprmSFPgnRestart" sep.fPgnRestart;0 or 1;byte;
        {0x3012, 1, L_FIX}, // "sprmSFEndnote" sep.fEndnote;0 or 1;byte;
        {0x3013, 1, L_FIX}, // "sprmSLnc" sep.lnc;lnc;byte;
        {0x3014, 1, L_FIX}, // "sprmSGprfIhdt" sep.grpfIhdt;grpfihdt
        {0x5015, 2, L_FIX}, // "sprmSNLnnMod" sep.nLnnMod;non-neg int.;word;
        {0x9016, 2, L_FIX}, // "sprmSDxaLnn" sep.dxaLnn;dxa;word;
        {0xB017, 2, L_FIX}, // "sprmSDyaHdrTop" sep.dyaHdrTop;dya;word;
        {0xB018, 2, L_FIX}, // "sprmSDyaHdrBottom" sep.dyaHdrBottom;dya;word;
        {0x3019, 1, L_FIX}, // "sprmSLBetween" sep.fLBetween;0 or 1;byte;
        {0x301A, 1, L_FIX}, // "sprmSVjc" sep.vjc;vjc;byte;
        {0x501B, 2, L_FIX}, // "sprmSLnnMin" sep.lnnMin;lnn;word;
        {0x501C, 2, L_FIX}, // "sprmSPgnStart" sep.pgnStart;pgn;word;
        {0x301D, 1, L_FIX}, // "sprmSBOrientation" sep.dmOrientPage;dm;byte;
        {0x301E, 1, L_FIX}, // "sprmSBCustomize" ;;;
        {0xB01F, 2, L_FIX}, // "sprmSXaPage" sep.xaPage;xa;word;
        {0xB020, 2, L_FIX}, // "sprmSYaPage" sep.yaPage;ya;word;
        {0xB021, 2, L_FIX}, // "sprmSDxaLeft" sep.dxaLeft;dxa;word;
        {0xB022, 2, L_FIX}, // "sprmSDxaRight" sep.dxaRight;dxa;word;
        {0x9023, 2, L_FIX}, // "sprmSDyaTop" sep.dyaTop;dya;word;
        {0x9024, 2, L_FIX}, // "sprmSDyaBottom" sep.dyaBottom;dya;word;
        {0xB025, 2, L_FIX}, // "sprmSDzaGutter" sep.dzaGutter;dza;word;
        {0x5026, 2, L_FIX}, // "sprmSDmPaperReq" sep.dmPaperReq;dm;word;
        {0xD227, 0, L_VAR}, // "sprmSPropRMark" sep.fPropRMark,
                            // sep.ibstPropRMark, sep.dttmPropRMark ;complex
        {0x3228, 1, L_FIX}, // "sprmSFBiDi" ;;;
        {0x3229, 1, L_FIX}, // "sprmSFFacingCol" ;;;
        {0x322A, 1, L_FIX}, // "sprmSFRTLGutter", set to one if gutter is on
                            // right
        {0x702B, 4, L_FIX}, // "sprmSBrcTop" sep.brcTop;BRC;long;
        {0x702C, 4, L_FIX}, // "sprmSBrcLeft" sep.brcLeft;BRC;long;
        {0x702D, 4, L_FIX}, // "sprmSBrcBottom" sep.brcBottom;BRC;long;
        {0x702E, 4, L_FIX}, // "sprmSBrcRight" sep.brcRight;BRC;long;
        {0x522F, 2, L_FIX}, // "sprmSPgbProp" sep.pgbProp;;word;
        {0x7030, 4, L_FIX}, // "sprmSDxtCharSpace" sep.dxtCharSpace;dxt;long;
        {0x9031, 2, L_FIX}, // "sprmSDyaLinePitch"
                            // sep.dyaLinePitch;dya; WRONG:long; RIGHT:short; !
        {0x5032, 2, L_FIX}, // "sprmSClm" ;;;
        {0x5033, 2, L_FIX}, // "sprmSTextFlow" sep.wTextFlow;complex
        {0x5400, 2, L_FIX}, // "sprmTJc" tap.jc;jc;word (low order byte is
                            // significant);
        {0x9601, 2, L_FIX}, // "sprmTDxaLeft" tap.rgdxaCenter
        {0x9602, 2, L_FIX}, // "sprmTDxaGapHalf" tap.dxaGapHalf,
                            // tap.rgdxaCenter
        {0x3403, 1, L_FIX}, // "sprmTFCantSplit" tap.fCantSplit;1 or 0;byte;
        {0x3404, 1, L_FIX}, // "sprmTTableHeader" tap.fTableHeader;1 or 0;byte;
        {0xD605, 0, L_VAR}, // "sprmTTableBorders" tap.rgbrcTable;complex
        {0xD606, 0, L_VAR}, // "sprmTDefTable10" tap.rgdxaCenter,
                            // tap.rgtc;complex
        {0x9407, 2, L_FIX}, // "sprmTDyaRowHeight" tap.dyaRowHeight;dya;word;
        {0xD608, 0, L_VAR}, // "sprmTDefTable" tap.rgtc;complex
        {0xD609, 0, L_VAR}, // "sprmTDefTableShd" tap.rgshd;complex
        {0x740A, 4, L_FIX}, // "sprmTTlp" tap.tlp;TLP;4 bytes;
        {0x560B, 1, L_FIX}, // "sprmTFBiDi" ;;;
        {0x740C, 1, L_FIX}, // "sprmTHTMLProps" ;;;
        {0xD620, 0, L_VAR}, // "sprmTSetBrc" tap.rgtc[].rgbrc;complex
        {0x7621, 4, L_FIX}, // "sprmTInsert" tap.rgdxaCenter, tap.rgtc;complex
        {0x5622, 2, L_FIX}, // "sprmTDelete" tap.rgdxaCenter, tap.rgtc;complex
        {0x7623, 4, L_FIX}, // "sprmTDxaCol" tap.rgdxaCenter;complex
        {0x5624, 0, L_VAR}, // "sprmTMerge" tap.fFirstMerged, tap.fMerged;
        {0x5625, 0, L_VAR}, // "sprmTSplit" tap.fFirstMerged, tap.fMerged;
        {0xD626, 0, L_VAR}, // "sprmTSetBrc10" tap.rgtc[].rgbrc;complex
        {0x7627, 0, L_VAR}, // "sprmTSetShd" tap.rgshd;complex
        {0x7628, 0, L_VAR}, // "sprmTSetShdOdd" tap.rgshd;complex
        {0x7629, 0, L_VAR}, // "sprmTTextFlow" tap.rgtc[].fVerticaltap,
                            // rgtc[].fBackwardtap, rgtc[].fRotateFont;0 or 10
                            // or 10 or 1;word;
        {0xD62A, 1, L_FIX}, // "sprmTDiagLine" ;;;
        {0xD62B, 0, L_VAR}, // "sprmTVertMerge" tap.rgtc[].vertMerge
        {0xD62C, 0, L_VAR}, // "sprmTVertAlign" tap.rgtc[].vertAlign
        {0xCA78, 0, L_VAR}, // undocumented "sprmCDoubleLine ?"
        {0x6649, 4, L_FIX}, // undocumented
        {0xF614, 3, L_FIX}, // undocumented
        {0xD612, 0, L_VAR}, // undocumented, new background colours.
        {0xD613, 0, L_VAR}, // undocumented
        {0xD61A, 0, L_VAR}, // undocumented
        {0xD61B, 0, L_VAR}, // undocumented
        {0xD61C, 0, L_VAR}, // undocumented
        {0xD61D, 0, L_VAR}, // undocumented
        {0xD632, 0, L_VAR}, // undocumented
        {0xD634, 0, L_VAR}, // undocumented
        {0xD238, 0, L_VAR}, // undocumented sep
        {0xC64E, 0, L_VAR}, // undocumented
        {0xC64F, 0, L_VAR}, // undocumented
        {0xC650, 0, L_VAR}, // undocumented
        {0xC651, 0, L_VAR}, // undocumented
        {0xF661, 3, L_FIX}, // undocumented
        {0x4873, 2, L_FIX}, // undocumented
        {0x4874, 2, L_FIX}, // undocumented
        {0x6463, 4, L_FIX}, // undocumented
        {0x2461, 1, L_FIX}, // undoc, must be asian version of "sprmPJc"
        {0x845D, 2, L_FIX}, // undoc, must be asian version of "sprmPDxaRight"
        {0x845E, 2, L_FIX}, // undoc, must be asian version of "sprmPDxaLeft"
        {0x8460, 2, L_FIX}, // undoc, must be asian version of "sprmPDxaLeft1"
        {0x3615, 1, L_FIX}, // undocumented
        {0x360D, 1, L_FIX}, // undocumented
        {0x703A, 4, L_FIX}, // undocumented, sep, perhaps related to textgrids ?
        {0x303B, 1, L_FIX}, // undocumented, sep
        {0x244B, 1, L_FIX}, // undocumented, subtable "sprmPFInTable" equiv ?
        {0x244C, 1, L_FIX}, // undocumented, subtable "sprmPFTtp" equiv ?
        {0x940E, 2, L_FIX}, // undocumented
        {0x940F, 2, L_FIX}, // undocumented
        {0x9410, 2, L_FIX}, // undocumented
        {0x6815, 4, L_FIX}, // undocumented
        {0x6816, 4, L_FIX}, // undocumented
        {0x6870, 4, L_FIX}, // undocumented, text colour
        {0xC64D, 0, L_VAR}, // undocumented, para back colour
        {0x6467, 4, L_FIX}, // undocumented
        {0x646B, 4, L_FIX}, // undocumented
        {0xF617, 3, L_FIX}, // undocumented
        {0xD660, 0, L_VAR}, // undocumented, something to do with colour.
        {0xD670, 0, L_VAR}, // undocumented, something to do with colour.
        {0xCA71, 0, L_VAR}, // undocumented, text backcolour
        {0x303C, 1, L_FIX}, // undocumented, sep
        {0x245B, 1, L_FIX}, // undocumented, para autobefore
        {0x245C, 1, L_FIX}  // undocumented, para autoafter
    };

    static wwSprmSearcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
};

wwSprmParser::wwSprmParser(int nVersion) : mnVersion(nVersion)
{
    ASSERT((mnVersion >= 6 && mnVersion <= 8), "Impossible value for version");

    mnDelta = (8 > mnVersion) ? 0 : 1;

    if (mnVersion < 8)
        mpKnownSprms = GetWW6SprmSearcher();
    else
        mpKnownSprms = GetWW8SprmSearcher();
}

SprmInfo wwSprmParser::GetSprmInfo(sal_uInt16 nId) const
{
    // Find sprm
    SprmInfo aSrch;
    aSrch.nId = nId;
    const SprmInfo* pFound = mpKnownSprms->search(aSrch);
    if (pFound == 0)
    {
        ASSERT(mnVersion >= 8,
           "Unknown ww6 sprm, dangerous, report to development");

        aSrch.nId = 0;
        aSrch.nLen = 0;
        //All the unknown ww7 sprms appear to be variable (which makes sense)
        aSrch.nVari = L_VAR;

        if (mnVersion == 8) //We can recover perfectly in this case
        {
            aSrch.nVari = L_FIX;
            switch (nId >> 13)
            {
                case 0:
                case 1:
                    aSrch.nLen = 1;
                    break;
                case 2:
                    aSrch.nLen = 2;
                    break;
                case 3:
                    aSrch.nLen = 4;
                    break;
                case 4:
                case 5:
                    aSrch.nLen = 2;
                    break;
                case 6:
                    aSrch.nLen = 0;
                    aSrch.nVari =  L_VAR;
                    break;
                case 7:
                default:
                    aSrch.nLen = 3;
                    break;
            }
        }

        pFound = &aSrch;
    }
    return *pFound;
}

//-end

inline BYTE Get_Byte( BYTE *& p )
{
    BYTE n = SVBT8ToByte( *(SVBT8*)p );
    p += 1;
    return n;
}

inline USHORT Get_UShort( BYTE *& p )
{
    USHORT n = SVBT16ToShort( *(SVBT16*)p );
    p += 2;
    return n;
}

inline short Get_Short( BYTE *& p )
{
    return Get_UShort(p);
}

inline ULONG Get_ULong( BYTE *& p )
{
    ULONG n = SVBT32ToLong( *(SVBT32*)p );
    p += 4;
    return n;
}

inline long Get_Long( BYTE *& p )
{
    return Get_ULong(p);
}

WW8SprmIter::WW8SprmIter(const BYTE* pSprms_, long nLen_,
    const wwSprmParser &rParser)
    :  mrSprmParser(rParser), pSprms( pSprms_), nRemLen( nLen_)
{
    UpdateMyMembers();
}

void WW8SprmIter::SetSprms(const BYTE* pSprms_, long nLen_)
{
    pSprms = pSprms_;
    nRemLen = nLen_;
    UpdateMyMembers();
}

const BYTE* WW8SprmIter::operator ++( int )
{
    if (nRemLen > 0)
    {
        pSprms += nAktSize;
        nRemLen -= nAktSize;
        UpdateMyMembers();
    }
    return pSprms;
}

void WW8SprmIter::UpdateMyMembers()
{
    if (pSprms && nRemLen > 0)
    {
        nAktId = mrSprmParser.GetSprmId(pSprms);
        pAktParams = pSprms + mrSprmParser.DistanceToData(nAktId);
        nAktSize = mrSprmParser.GetSprmSize(nAktId, pSprms);
    }
    else
    {
        nAktId = 0;
        pAktParams = 0;
        nAktSize = 0;
        nRemLen = 0;
    }
}

const BYTE* WW8SprmIter::FindSprm(USHORT nId)
{
    while(GetSprms())
    {
        if( GetAktId() == nId )
            return GetAktParams();              // SPRM found!
        operator ++(0);
    }

    return 0;                                   // SPRM _not_ found
}

//-----------------------------------------
//      temporaerer Test
//-----------------------------------------
// WW8PLCFx_PCDAttrs halten sich an WW8PLCF_Pcd fest und besitzen deshalb keine
// eigenen Iteratoren. Alle sich auf Iteratoren beziehenden Methoden
// sind deshalb Dummies.

WW8PLCFx_PCDAttrs::WW8PLCFx_PCDAttrs(BYTE nVersion, WW8PLCFx_PCD* pPLCFx_PCD,
    const WW8ScannerBase* pBase)
    : WW8PLCFx(nVersion, true), pPcdI(pPLCFx_PCD->GetPLCFIter()),
    pPcd(pPLCFx_PCD), pGrpprls(pBase->pPieceGrpprls),
    nGrpprls(pBase->nPieceGrpprls)
{
}

ULONG WW8PLCFx_PCDAttrs::GetIdx() const
{
    return 0;
}

void WW8PLCFx_PCDAttrs::SetIdx( ULONG )
{
}

bool WW8PLCFx_PCDAttrs::SeekPos(WW8_CP )
{
    return true;
}

WW8PLCFx& WW8PLCFx_PCDAttrs::operator ++( int )
{
    return *this;
}

WW8_CP WW8PLCFx_PCDAttrs::Where()
{
    return ( pPcd ) ? pPcd->Where() : LONG_MAX;
}

void WW8PLCFx_PCDAttrs::GetSprms(WW8PLCFxDesc* p)
{
    void* pData;

    p->bRealLineEnd = false;
    if ( !pPcdI || !pPcdI->Get(p->nStartPos, p->nEndPos, pData) )
    {
        // PLCF fully processed
        p->nStartPos = p->nEndPos = LONG_MAX;
        p->pMemPos = 0;
        p->nSprmsLen = 0;
        return;
    }

    UINT16 nPrm = SVBT16ToShort( ( (WW8_PCD*)pData )->prm );
    if ( nPrm & 1 )
    {
        // PRM Variant 2
        UINT16 nSprmIdx = nPrm >> 1;

        if( nSprmIdx >= nGrpprls )
        {
            // Invalid Index
            p->nStartPos = p->nEndPos = LONG_MAX;
            p->pMemPos = 0;
            p->nSprmsLen = 0;
            return;
        }
        const BYTE* pSprms = pGrpprls[ nSprmIdx ];

        p->nSprmsLen = SVBT16ToShort( pSprms ); // Length
        pSprms += 2;
        p->pMemPos = pSprms;                    // Position
    }
    else
    {
        // PRM Variante 1:  Sprm wird direkt in Member-Var abgelegt
        /*
            Dies sind die Attr, die in der Piece-Table stehen, statt im Text !
        */

        if(8 > GetVersion())
        {
            aShortSprm[0] = (BYTE)( ( nPrm & 0xfe) >> 1 );
            aShortSprm[1] = (BYTE)(   nPrm         >> 8 );
            p->nSprmsLen = ( nPrm ) ? 2 : 0;        // Laenge

            // store Postion of internal mini storage in Data Pointer
            p->pMemPos = aShortSprm;
        }
        else
        {
            p->pMemPos = 0;
            p->nSprmsLen = 0;
            BYTE nSprmListIdx = (BYTE)((nPrm & 0xfe) >> 1);
            if( nSprmListIdx )
            {
                // process Sprm Id Matching as explained in MS Doku
                //
                // ''Property Modifier(variant 1) (PRM)''
                // see file: s62f39.htm
                //
                // Since isprm is 7 bits, rgsprmPrm can hold 0x80 entries.
                static const USHORT aSprmId[0x80] =
                {
                    // sprmNoop, sprmNoop, sprmNoop, sprmNoop
                    0x0000,0x0000,0x0000,0x0000,
                    // sprmPIncLvl, sprmPJc, sprmPFSideBySide, sprmPFKeep
                    0x2402,0x2403,0x2404,0x2405,
                    // sprmPFKeepFollow, sprmPFPageBreakBefore, sprmPBrcl,
                    // sprmPBrcp
                    0x2406,0x2407,0x2408,0x2409,
                    // sprmPIlvl, sprmNoop, sprmPFNoLineNumb, sprmNoop
                    0x260A,0x0000,0x240C,0x0000,
                    // sprmNoop, sprmNoop, sprmNoop, sprmNoop
                    0x0000,0x0000,0x0000,0x0000,
                    // sprmNoop, sprmNoop, sprmNoop, sprmNoop
                    0x0000,0x0000,0x0000,0x0000,
                    // sprmPFInTable, sprmPFTtp, sprmNoop, sprmNoop
                    0x2416,0x2417,0x0000,0x0000,
                    // sprmNoop, sprmPPc,  sprmNoop, sprmNoop
                    0x0000,0x261B,0x0000,0x0000,
                    // sprmNoop, sprmNoop, sprmNoop, sprmNoop
                    0x0000,0x0000,0x0000,0x0000,
                    // sprmNoop, sprmPWr,  sprmNoop, sprmNoop
                    0x0000,0x2423,0x0000,0x0000,
                    // sprmNoop, sprmNoop, sprmNoop, sprmNoop
                    0x0000,0x0000,0x0000,0x0000,
                    // sprmPFNoAutoHyph, sprmNoop, sprmNoop, sprmNoop
                    0x242A,0x0000,0x0000,0x0000,
                    // sprmNoop, sprmNoop, sprmPFLocked, sprmPFWidowControl
                    0x0000,0x0000,0x2430,0x2431,
                    // sprmNoop, sprmPFKinsoku, sprmPFWordWrap,
                    // sprmPFOverflowPunct
                    0x0000,0x2433,0x2434,0x2435,
                    // sprmPFTopLinePunct, sprmPFAutoSpaceDE,
                    // sprmPFAutoSpaceDN, sprmNoop
                    0x2436,0x2437,0x2438,0x0000,
                    // sprmNoop, sprmPISnapBaseLine, sprmNoop, sprmNoop
                    0x0000,0x243B,0x000,0x0000,
                    // sprmNoop, sprmCFStrikeRM, sprmCFRMark, sprmCFFldVanish
                    0x0000,0x0800,0x0801,0x0802,
                    // sprmNoop, sprmNoop, sprmNoop, sprmCFData
                    0x0000,0x0000,0x0000,0x0806,
                    // sprmNoop, sprmNoop, sprmNoop, sprmCFOle2
                    0x0000,0x0000,0x0000,0x080A,
                    // sprmNoop, sprmCHighlight, sprmCFEmboss, sprmCSfxText
                    0x0000,0x2A0C,0x0858,0x2859,
                    // sprmNoop, sprmNoop, sprmNoop, sprmCPlain
                    0x0000,0x0000,0x0000,0x2A33,
                    // sprmNoop, sprmCFBold, sprmCFItalic, sprmCFStrike
                    0x0000,0x0835,0x0836,0x0837,
                    // sprmCFOutline, sprmCFShadow, sprmCFSmallCaps, sprmCFCaps,
                    0x0838,0x0839,0x083a,0x083b,
                    // sprmCFVanish, sprmNoop, sprmCKul, sprmNoop,
                    0x083C,0x0000,0x2A3E,0x0000,
                    // sprmNoop, sprmNoop, sprmCIco, sprmNoop,
                    0x0000,0x0000,0x2A42,0x0000,
                    // sprmCHpsInc, sprmNoop, sprmCHpsPosAdj, sprmNoop,
                    0x2A44,0x0000,0x2A46,0x0000,
                    // sprmCIss, sprmNoop, sprmNoop, sprmNoop,
                    0x2A48,0x0000,0x0000,0x0000,
                    // sprmNoop, sprmNoop, sprmNoop, sprmNoop,
                    0x0000,0x0000,0x0000,0x0000,
                    // sprmNoop, sprmNoop, sprmNoop, sprmCFDStrike,
                    0x0000,0x0000,0x0000,0x2A53,
                    // sprmCFImprint, sprmCFSpec, sprmCFObj, sprmPicBrcl,
                    0x0854,0x0855,0x0856,0x2E00,
                    // sprmPOutLvl, sprmPFBiDi, sprmNoop, sprmNoop,
                    0x2640,0x2441,0x0000,0x0000,
                    // sprmNoop, sprmNoop, sprmPPnbrRMarkNot
                    0x0000,0x0000,0x0000,0x0000
                };

                // find real Sprm Id:
                USHORT nSprmId = aSprmId[ nSprmListIdx ];

                if( nSprmId )
                {
                    // move Sprm Id and Sprm Param to internal mini storage:
                    aShortSprm[0] = (BYTE)( ( nSprmId & 0x00ff)      );
                    aShortSprm[1] = (BYTE)( ( nSprmId & 0xff00) >> 8 );
                    aShortSprm[2] = (BYTE)( nPrm >> 8 );

                    // store Sprm Length in member:
                    p->nSprmsLen = ( nPrm ) ? 3 : 0;

                    // store Postion of internal mini storage in Data Pointer
                    p->pMemPos = aShortSprm;
                }
            }
        }
    }
}

//------------------------------------------------------------------------

WW8PLCFx_PCD::WW8PLCFx_PCD(BYTE nVersion, WW8PLCFpcd* pPLCFpcd,
    WW8_CP nStartCp, bool bVer67P)
    : WW8PLCFx(nVersion, false), nClipStart(-1)
{
    // eigenen Iterator konstruieren
    pPcdI = new WW8PLCFpcd_Iter(*pPLCFpcd, nStartCp);
    bVer67= bVer67P;
}

WW8PLCFx_PCD::~WW8PLCFx_PCD()
{
    // pPcd-Dtor which in called from WW8ScannerBase
    delete pPcdI;
}

ULONG WW8PLCFx_PCD::GetIMax() const
{
    return pPcdI ? pPcdI->GetIMax() : 0;
}

ULONG WW8PLCFx_PCD::GetIdx() const
{
    return pPcdI ? pPcdI->GetIdx() : 0;
}

void WW8PLCFx_PCD::SetIdx( ULONG nIdx )
{
    if (pPcdI)
        pPcdI->SetIdx( nIdx );
}

bool WW8PLCFx_PCD::SeekPos(WW8_CP nCpPos)
{
    return pPcdI ? pPcdI->SeekPos( nCpPos ) : false;
}

WW8_CP WW8PLCFx_PCD::Where()
{
    return pPcdI ? pPcdI->Where() : LONG_MAX;
}

long WW8PLCFx_PCD::GetNoSprms( long& rStart, long& rEnd, long& rLen )
{
    void* pData;
    rLen = 0;

    if ( !pPcdI || !pPcdI->Get(rStart, rEnd, pData) )
    {
        rStart = rEnd = LONG_MAX;
        return -1;
    }
    return pPcdI->GetIdx();
}

WW8PLCFx& WW8PLCFx_PCD::operator ++( int )
{
    if (pPcdI)
        (*pPcdI)++;
    else
        ASSERT( !this, "pPcdI fehlt");
    return *this;
}

WW8_FC WW8PLCFx_PCD::AktPieceStartCp2Fc( WW8_CP nCp )
{
    WW8_CP nCpStart, nCpEnd;
    void* pData;

    if ( !pPcdI->Get(nCpStart, nCpEnd, pData) )
    {
        ASSERT( !this, "AktPieceStartCp2Fc() with false Cp found (1)" );
        return LONG_MAX;
    }

    ASSERT( nCp >= nCpStart && nCp < nCpEnd,
        "AktPieceCp2Fc() with false Cp found (2)" );

    if( nCp < nCpStart )
        nCp = nCpStart;
    if( nCp >= nCpEnd )
        nCp = nCpEnd - 1;

    bool bIsUnicode = false;
    WW8_FC nFC = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
    if( !bVer67 )
        nFC = WW8PLCFx_PCD::TransformPieceAddress( nFC, bIsUnicode );

    return nFC + (nCp - nCpStart) * (bIsUnicode ? 2 : 1);
}


void WW8PLCFx_PCD::AktPieceFc2Cp( long& rStartPos, long& rEndPos,
    const WW8ScannerBase *pSBase )
{
    //No point going anywhere with this
    if ((rStartPos == LONG_MAX) && (rEndPos == LONG_MAX))
        return;

    rStartPos = pSBase->WW8Fc2Cp( rStartPos );
    rEndPos = pSBase->WW8Fc2Cp( rEndPos );
}

WW8_CP WW8PLCFx_PCD::AktPieceStartFc2Cp( WW8_FC nStartPos )
{
    WW8_CP nCpStart, nCpEnd;
    void* pData;
    if ( !pPcdI->Get( nCpStart, nCpEnd, pData ) )
    {
         ASSERT( !this, "AktPieceStartFc2Cp() - Fehler" );
        return LONG_MAX;
    }
    bool bIsUnicode = false;
    INT32 nFcStart  = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
    if( !bVer67 )
        nFcStart = WW8PLCFx_PCD::TransformPieceAddress( nFcStart, bIsUnicode );

    INT32 nUnicodeFactor = bIsUnicode ? 2 : 1;

    if( nStartPos < nFcStart )
        nStartPos = nFcStart;

    if( nStartPos >= nFcStart + (nCpEnd - nCpStart)     * nUnicodeFactor )
        nStartPos  = nFcStart + (nCpEnd - nCpStart - 1) * nUnicodeFactor;

    return nCpStart + (nStartPos - nFcStart) / nUnicodeFactor;
}

//-----------------------------------------
//      Hilfsroutinen fuer alle
//-----------------------------------------

DateTime WW8ScannerBase::WW8DTTM2DateTime(long lDTTM)
{
    /*
    mint    short   :6  0000003F    minutes (0-59)
    hr      short   :5  000007C0    hours (0-23)
    dom     short   :5  0000F800    days of month (1-31)
    mon     short   :4  000F0000    months (1-12)
    yr      short   :9  1FF00000    years (1900-2411)-1900
    wdy     short   :3  E0000000    weekday(Sunday=0
                                            Monday=1
    ( wdy can be ignored )                  Tuesday=2
                                            Wednesday=3
                                            Thursday=4
                                            Friday=5
                                            Saturday=6)
    */
    DateTime aDateTime(Date( 0 ), Time( 0 ));
    if( lDTTM )
    {
        USHORT lMin = (USHORT)(lDTTM & 0x0000003F);
        lDTTM >>= 6;
        USHORT lHour= (USHORT)(lDTTM & 0x0000001F);
        lDTTM >>= 5;
        USHORT lDay = (USHORT)(lDTTM & 0x0000001F);
        lDTTM >>= 5;
        USHORT lMon = (USHORT)(lDTTM & 0x0000000F);
        lDTTM >>= 4;
        USHORT lYear= (USHORT)(lDTTM & 0x000001FF) + 1900;
        aDateTime = DateTime(Date(lDay, lMon, lYear), Time(lHour, lMin));
    }
    return aDateTime;
}

short WW8_BRC::DetermineBorderProperties(bool bVer67, short *pSpace,
    BYTE *pCol, short *pIdx) const
{
    /*
        Word does not factor the width of the border into the width/height
        stored in the information for graphic/table/object widths, so we need
        to figure out this extra width here and utilize the returned size in
        our calculations
    */
    short nMSTotalWidth;
    BYTE nCol;
    short nIdx,nSpace;
    if( bVer67 )
    {
        UINT16 aBrc1 = SVBT16ToShort(aBits1);
        nCol = ((aBrc1 >> 6) & 0x1f);   // aBor.ico
        nSpace = (aBrc1 & 0xF800) >> 11;

        nMSTotalWidth = aBrc1 & 0x07;
        nIdx = (aBrc1 & 0x18) >> 3;
        //Dashed/Dotted unsets double/thick
        if (nMSTotalWidth > 5)
        {
            nMSTotalWidth=1;
            nIdx = 1;
        }
        nMSTotalWidth *= nIdx;
        nMSTotalWidth *= 15;
    }
    else
    {
        nIdx = aBits1[1];
        nCol = aBits2[0];   // aBor.ico
        nSpace = aBits2[1] & 0x1F; //space between line and object

        //Specification in 8ths of a point, 1 Point = 20 Twips, so by 2.5
        nMSTotalWidth  = aBits1[ 0 ] * 20 / 8;

        //Figure out the real size of the border according to word
        switch (nIdx)
        {
            //Note that codes over 25 are undocumented, and I can't create
            //these 4 here in the wild.
            default:
            case 2:
            case 4:
            case 5:
            case 22:
                DBG_WARNING("Can't create these from the menus, please report");
            case 1:
            case 6:
            case 7:
            case 8:
            case 9:
            case 23:    //Only 3pt in the menus, but honours the size setting.
                break;
            case 3:
                /*
                double line is three times the width of an ordinary line,
                except for the smallest 1/4 point size which appears to have
                exactly the same total border width as a 1/2 point size
                ordinary line, i.e. twice the nominal line width
                */
                nMSTotalWidth = (nMSTotalWidth == 5) ?
                    nMSTotalWidth*2 : nMSTotalWidth*3;
                break;
            case 10:
                /*
                triple line is five times the width of an ordinary line,
                except that the smallest 1/4 point size appears to have
                exactly the same total border width as a 3/4 point size
                ordinary line, i.e. three times the nominal line width.  The
                second smallest 1/2 point size appears to have exactly the
                total border width as a 2 1/4 border, i.e 4.5 times the size.
                */
                if (nMSTotalWidth == 5)
                    nMSTotalWidth*=3;
                else if (nMSTotalWidth == 10)
                    nMSTotalWidth = nMSTotalWidth*9/2;
                else
                    nMSTotalWidth*=5;
                break;
            case 11:
            case 12:
                /*
                small gap thin thick and thick thin appears to have a 3/4
                point line, a 3/4 point gap and a thick line of the specified
                width
                */
                nMSTotalWidth = nMSTotalWidth + 15*2;
                break;
            case 13:
                /*
                thin thick thin appears to have two outside 3/4 point lines,
                two 3/4 point gaps and a thick line of the specified width
                */
                nMSTotalWidth = nMSTotalWidth + 15*4;
                break;
            case 14:
            case 15:
                /*
                medium gap thin thick and thick thin appears to have a line
                50% of the thick line, and an equal sized gap and then the
                thick line of the specified width. But it appears to only
                use one of the existing predefined widths for the thin line,
                so the closest smallest existing border to the halved thick
                line is used.
                */
                switch (nMSTotalWidth)
                {
                    case 45:    //2 1/4, closest to half is 1
                        nMSTotalWidth += 20 + (nMSTotalWidth-1)/2;
                        break;
                    case 5:
                    case 10:
                        nMSTotalWidth += 5;
                        break;
                    case 15:    //3/4, closest to half is 1/4
                        nMSTotalWidth += 5 + (nMSTotalWidth-1)/2;
                        break;
                    default:
                        nMSTotalWidth*=2;
                        break;
                }
                break;
            case 16:
                /*
                medium gap thin thick thin appears to have a line
                50% of the thick line, and an equal sized gap and then the
                thick line of the specified width. But it appears to only
                use one of the existing predefined widths for the thin
                line, so the closest smallest existing border to the halved
                thick line is used. Though some fudging at smaller sizes is
                still required.
                */
                switch (nMSTotalWidth)
                {
                    case 45:    //2 1/4, closest to half is 1
                        nMSTotalWidth += nMSTotalWidth + 20 * 2;
                        break;
                    case 20:
                    case 15:
                        nMSTotalWidth += nMSTotalWidth + 7 * 2;
                        break;
                    case 10:
                    case 5:
                        nMSTotalWidth += 5 + 4;
                        break;
                    default:
                        nMSTotalWidth*=3;
                        break;
                }
                break;
            case 17:
            case 18:
                /*
                large gap thin thick and thick thin appears to have a thick
                line of 1 1/2 pt and a narrow of 3/4 point, with a distance
                between the two of the explicitly set line width
                */
                nMSTotalWidth+=15+30;
                break;
            case 19:
                /*
                large gap thin thick thin appears to have a thick line of 1
                1/2 pt and two narrows of 3/4 point, with a distance between
                the two of the explicitly set line width, though the narrowest
                line appears to behave as if it was even smaller
                */
                if (nMSTotalWidth == 5)
                    nMSTotalWidth = 3;
                nMSTotalWidth = nMSTotalWidth*2 + 15*2 + 30;
                break;
            case 20:
                /*
                wave, the dimensions appear to be created by the drawing of
                the wave, so we have only two possibilites in the menus, 3/4
                point is equal to solid 3 point. This calculation seems to
                match well to results.
                */
                nMSTotalWidth +=45;
                break;
            case 21:
                /*
                double wave, the dimensions appear to be created by the
                drawing of the wave, so we have only one possibilites in the
                menus, that of 3/4 point is equal to solid 3 point. This
                calculation seems to match well to results.
                */
                nMSTotalWidth += 45*2;
                break;
            case 24:
            case 25:
                /*
                emboss and engrave consist of a three lines, the central is of
                the explicit point width, the other two (of equal size to each
                other are the shadows and are either 3/4 pt of 1 1/2 depending
                on if the central line is greater of less than 2 1/4 pt
                */
                if (nMSTotalWidth <= 45)
                    nMSTotalWidth += 2*15;
                else
                    nMSTotalWidth += 2*30;
                break;
        }
    }

    if (pIdx)
        *pIdx = nIdx;
    if (pSpace)
        *pSpace = nSpace*20;
    if (pCol)
        *pCol = nCol;
    return nMSTotalWidth;
}

WW8_CP WW8ScannerBase::WW8Fc2Cp( WW8_FC nFcPos ) const
{
    WW8_CP nFallBackCpEnd = LONG_MAX;
    if( nFcPos == LONG_MAX )
        return nFallBackCpEnd;

    bool bIsUnicode = false;
    if( pPieceIter )    // Complex File ?
    {
        ULONG nOldPos = pPieceIter->GetIdx();

        for (pPieceIter->SetIdx(0);
            pPieceIter->GetIdx() < pPieceIter->GetIMax();(*pPieceIter)++)
        {
            long nCpStart, nCpEnd;
            void* pData;
            if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
            {   // ausserhalb PLCFfpcd ?
                ASSERT( !this, "PLCFpcd-WW8Fc2Cp() ging schief" );
                break;
            }
            INT32 nFcStart  = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
            if( 8 <= pWw8Fib->nVersion )
                nFcStart = WW8PLCFx_PCD::TransformPieceAddress( nFcStart,
                                                                bIsUnicode );
            INT32 nLen = (nCpEnd - nCpStart) * (bIsUnicode ? 2 : 1);

            /*
            If this cp is inside this piece, or its the last piece and we are
            on the very last cp of that piece
            */
            if (nFcPos >= nFcStart)
            {
                // found
                WW8_CP nTempCp =
                    nCpStart + ((nFcPos - nFcStart) / (bIsUnicode ? 2 : 1));
                if (nFcPos < nFcStart + nLen)
                {
                    pPieceIter->SetIdx( nOldPos );
                    return nTempCp;
                }
                else if (nFcPos == nFcStart + nLen)
                {
                    //Keep this cp as its on a piece boundary because we might
                    //need it if tests fail
                    nFallBackCpEnd = nTempCp;
                }
            }
        }
        // not found
        pPieceIter->SetIdx( nOldPos );      // not found
        /*
        If it was not found, then this is because it has fallen between two
        stools, i.e. either it is the last cp/fc of the last piece, or it is
        the last cp/fc of a disjoint piece.
        */
        return nFallBackCpEnd;
    }
    // No complex file
    if (pWw8Fib->fExtChar)
        bIsUnicode=true;
    return ((nFcPos - pWw8Fib->fcMin) / (bIsUnicode ? 2 : 1));
}

WW8_FC WW8ScannerBase::WW8Cp2Fc(WW8_CP nCpPos, bool* pIsUnicode,
    WW8_CP* pNextPieceCp, bool* pTestFlag) const
{
    if( pTestFlag )
        *pTestFlag = true;
    if( LONG_MAX == nCpPos )
        return LONG_MAX;

    bool bIsUnicode;
    if( !pIsUnicode )
        pIsUnicode = &bIsUnicode;

    if( pPieceIter )
    {   // Complex File
        if( pNextPieceCp )
            *pNextPieceCp = LONG_MAX;

        if( !pPieceIter->SeekPos( nCpPos ) )
        {
            if( pTestFlag )
                *pTestFlag = false;
            else
                ASSERT( !this, "Falscher CP an WW8Cp2Fc() uebergeben" );
            return LONG_MAX;
        }
        long nCpStart, nCpEnd;
        void* pData;
        if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
        {
            if( pTestFlag )
                *pTestFlag = false;
            else
                ASSERT( !this, "PLCFfpcd-Get ging schief" );
            return LONG_MAX;
        }
        if( pNextPieceCp )
            *pNextPieceCp = nCpEnd;

        WW8_FC nRet = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
        if (8 > pWw8Fib->nVersion)
            *pIsUnicode = false;
        else
            nRet = WW8PLCFx_PCD::TransformPieceAddress( nRet, *pIsUnicode );


        nRet += (nCpPos - nCpStart) * (*pIsUnicode ? 2 : 1);

        return nRet;
    }

    // No complex file
    if (pWw8Fib->fExtChar)
        *pIsUnicode = true;
    else
        *pIsUnicode = false;
    return pWw8Fib->fcMin + nCpPos * (*pIsUnicode ? 2 : 1);
}

//-----------------------------------------
//      class WW8ScannerBase
//-----------------------------------------

WW8PLCFpcd* WW8ScannerBase::OpenPieceTable( SvStream* pStr, const WW8Fib* pWwF )
{
    if ( ((8 > pWw8Fib->nVersion) && !pWwF->fComplex) || !pWwF->lcbClx )
        return 0;

    WW8_FC nClxPos = pWwF->fcClx;
    INT32 nClxLen = pWwF->lcbClx;
    register INT32 nLeft = nClxLen;
    INT16 nGrpprl = 0;
    BYTE clxt;

    pStr->Seek( nClxPos );
    while( 1 ) // Zaehle Zahl der Grpprls
    {
        *pStr >> clxt;
        nLeft--;
        if( 2 == clxt )                         // PLCFfpcd ?
            break;                              // PLCFfpcd gefunden
        if( 1 == clxt )                         // clxtGrpprl ?
            nGrpprl++;
        UINT16 nLen;
        *pStr >> nLen;
        nLeft -= 2 + nLen;
        if( nLeft < 0 )
            return 0;                           // schiefgegangen
        pStr->SeekRel( nLen );                  // ueberlies grpprl
    }
    pStr->Seek( nClxPos );
    nLeft = nClxLen;
    pPieceGrpprls = new BYTE*[nGrpprl + 1];
    memset( pPieceGrpprls, 0, ( nGrpprl + 1 ) * 4 );
    nPieceGrpprls = nGrpprl;
    INT16 nAktGrpprl = 0;                       // lies Grpprls ein
    while( 1 )
    {
        *pStr >> clxt;
        nLeft--;
        if( 2 == clxt)                          // PLCFfpcd ?
            break;                              // PLCFfpcd gefunden
        UINT16 nLen;
        *pStr >> nLen;
        nLeft -= 2 + nLen;
        if( nLeft < 0 )
            return 0;                           // schiefgegangen
        if( 1 == clxt )                         // clxtGrpprl ?
        {
            BYTE* p = new BYTE[nLen+2];         // alloziere
            ShortToSVBT16(nLen, p);             // trage Laenge ein
            pStr->Read( p+2, nLen );            // lies grpprl
            pPieceGrpprls[nAktGrpprl++] = p;    // trage in Array ein
        }
        else
            pStr->SeekRel( nLen );              // ueberlies nicht-Grpprl
    }
    // lies Piece Table PLCF ein
    INT32 nPLCFfLen;
    *pStr >> nPLCFfLen;
    ASSERT( 65536 > nPLCFfLen, "PLCFfpcd ueber 64 k" );
    return new WW8PLCFpcd( pStr, pStr->Tell(), nPLCFfLen, 8 );
}

void WW8ScannerBase::DeletePieceTable()
{
    if( pPieceGrpprls )
    {
        for( BYTE** p = pPieceGrpprls; *p; p++ )
            delete[] (*p);
        delete[] pPieceGrpprls;
        pPieceGrpprls = 0;
    }
}

WW8ScannerBase::WW8ScannerBase( SvStream* pSt, SvStream* pTblSt,
    SvStream* pDataSt, const WW8Fib* pWwFib )
    : pWw8Fib(pWwFib), pMainFdoa(0), pHdFtFdoa(0), pMainTxbx(0),
    pMainTxbxBkd(0), pHdFtTxbx(0), pHdFtTxbxBkd(0), pMagicTables(0),
    pPieceGrpprls(0)
{
    pPiecePLCF = OpenPieceTable( pTblSt, pWw8Fib );             // Complex
    if( pPiecePLCF )
    {
        pPieceIter = new WW8PLCFpcd_Iter( *pPiecePLCF );
        pPLCFx_PCD = new WW8PLCFx_PCD( pWwFib->nVersion, pPiecePLCF, 0,
            8 > pWw8Fib->nVersion );
        pPLCFx_PCDAttrs = new WW8PLCFx_PCDAttrs( pWwFib->nVersion, pPLCFx_PCD,
            this);
    }
    else
    {
        pPieceIter = 0;
        pPLCFx_PCD = 0;
        pPLCFx_PCDAttrs = 0;
    }

    // pChpPLCF and pPapPLCF may NOT be created before pPLCFx_PCD !!
    pChpPLCF = new WW8PLCFx_Cp_FKP( pSt, pTblSt, pDataSt, *this, CHP ); // CHPX
    pPapPLCF = new WW8PLCFx_Cp_FKP( pSt, pTblSt, pDataSt, *this, PAP ); // PAPX

    pSepPLCF = new WW8PLCFx_SEPX(   pSt, pTblSt, *pWwFib, 0 );          // SEPX

    // Footnotes
    pFtnPLCF = new WW8PLCFx_SubDoc( pTblSt, pWwFib->nVersion, 0,
        pWwFib->fcPlcffndRef, pWwFib->lcbPlcffndRef, pWwFib->fcPlcffndTxt,
        pWwFib->lcbPlcffndTxt, 2 );
    // Endnotes
    pEdnPLCF = new WW8PLCFx_SubDoc( pTblSt, pWwFib->nVersion, 0,
        pWwFib->fcPlcfendRef, pWwFib->lcbPlcfendRef, pWwFib->fcPlcfendTxt,
        pWwFib->lcbPlcfendTxt, 2 );
    // Anmerkungen
    pAndPLCF = new WW8PLCFx_SubDoc( pTblSt, pWwFib->nVersion, 0,
        pWwFib->fcPlcfandRef, pWwFib->lcbPlcfandRef, pWwFib->fcPlcfandTxt,
        pWwFib->lcbPlcfandTxt, (8 > pWwFib->nVersion) ? 20 : 30 );

    // Fields Main Text
    pFldPLCF    = new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_MAINTEXT);
    // Fields Header / Footer
    pFldHdFtPLCF= new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_HDFT);
    // Fields Footnote
    pFldFtnPLCF = new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_FTN);
    // Fields Endnote
    pFldEdnPLCF = new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_EDN);
    // Fields Anmerkungen
    pFldAndPLCF = new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_AND);
    // Fields in Textboxes in Main Text
    pFldTxbxPLCF= new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_TXBX);
    // Fields in Textboxes in Header / Footer
    pFldTxbxHdFtPLCF = new WW8PLCFx_FLD(pTblSt,*pWwFib,MAN_TXBX_HDFT);

    // Note: 6 stands for "6 OR 7",  7 stands for "ONLY 7"
    switch( pWw8Fib->nVersion )
    {
        case 6:
        case 7:
            if( pWwFib->fcPlcfdoaMom && pWwFib->lcbPlcfdoaMom )
            {
                pMainFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfdoaMom,
                    pWwFib->lcbPlcfdoaMom, 6 );
            }
            if( pWwFib->fcPlcfdoaHdr && pWwFib->lcbPlcfdoaHdr )
            {
                pHdFtFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfdoaHdr,
                pWwFib->lcbPlcfdoaHdr, 6 );
            }
            break;
        case 8:
            if( pWwFib->fcPlcfspaMom && pWwFib->lcbPlcfspaMom )
            {
                pMainFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfspaMom,
                    pWwFib->lcbPlcfspaMom, 26 );
            }
            if( pWwFib->fcPlcfspaHdr && pWwFib->lcbPlcfspaHdr )
            {
                pHdFtFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfspaHdr,
                    pWwFib->lcbPlcfspaHdr, 26 );
            }
            // PLCF fuer TextBox-Break-Deskriptoren im Maintext
            if( pWwFib->fcPlcftxbxBkd && pWwFib->lcbPlcftxbxBkd )
            {
                pMainTxbxBkd = new WW8PLCFspecial( pTblSt,
                    pWwFib->fcPlcftxbxBkd, pWwFib->lcbPlcftxbxBkd, 0);
            }
            // PLCF fuer TextBox-Break-Deskriptoren im Header-/Footer-Bereich
            if( pWwFib->fcPlcfHdrtxbxBkd && pWwFib->lcbPlcfHdrtxbxBkd )
            {
                pHdFtTxbxBkd = new WW8PLCFspecial( pTblSt,
                    pWwFib->fcPlcfHdrtxbxBkd, pWwFib->lcbPlcfHdrtxbxBkd, 0);
            }
            // Sub table cp positions
            if (pWwFib->fcMagicTable && pWwFib->lcbMagicTable)
            {
                pMagicTables = new WW8PLCFspecial( pTblSt,
                    pWwFib->fcMagicTable, pWwFib->lcbMagicTable, 4);
            }
            break;
        default:
            ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
            break;
    }

    // PLCF fuer TextBox-Stories im Maintext
    long nLenTxBxS = (8 > pWw8Fib->nVersion) ? 0 : 22;
    if( pWwFib->fcPlcftxbxTxt && pWwFib->lcbPlcftxbxTxt )
    {
        pMainTxbx = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcftxbxTxt,
            pWwFib->lcbPlcftxbxTxt, nLenTxBxS );
    }

    // PLCF fuer TextBox-Stories im Header-/Footer-Bereich
    if( pWwFib->fcPlcfHdrtxbxTxt && pWwFib->lcbPlcfHdrtxbxTxt )
    {
        pHdFtTxbx = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfHdrtxbxTxt,
            pWwFib->lcbPlcfHdrtxbxTxt, nLenTxBxS );
    }

    pBook = new WW8PLCFx_Book(pTblSt, *pWwFib);
}

WW8ScannerBase::~WW8ScannerBase()
{
    DeletePieceTable();
    delete pPLCFx_PCDAttrs;
    delete pPLCFx_PCD;
    delete pPieceIter;
    delete pPiecePLCF;
    delete pBook;
    delete pFldEdnPLCF;
    delete pFldFtnPLCF;
    delete pFldAndPLCF;
    delete pFldHdFtPLCF;
    delete pFldPLCF;
    delete pFldTxbxPLCF;
    delete pFldTxbxHdFtPLCF;
    delete pEdnPLCF;
    delete pFtnPLCF;
    delete pAndPLCF;
    delete pSepPLCF;
    delete pPapPLCF;
    delete pChpPLCF;
    // vergessene Schaeflein
    delete pMainFdoa;
    delete pHdFtFdoa;
    delete pMainTxbx;
    delete pMainTxbxBkd;
    delete pHdFtTxbx;
    delete pHdFtTxbxBkd;
    delete pMagicTables;
}

//-----------------------------------------
//          Fields
//-----------------------------------------
static bool WW8SkipField(WW8PLCFspecial& rPLCF)
{
    void* pData;
    long nP;

    if (!rPLCF.Get(nP, pData))              // Ende des PLCFspecial ?
        return false;

    rPLCF++;

    if((((BYTE*)pData)[0] & 0x1f ) != 0x13 )    // Kein Anfang ?
        return true;                            // Bei Fehler nicht abbrechen

    if( !rPLCF.Get( nP, pData ) )
        return false;


    while((((BYTE*)pData)[0] & 0x1f ) == 0x13 )
    {
        // immer noch neue (nested) Anfaenge ?
        WW8SkipField( rPLCF );              // nested Field im Beschreibungsteil
        if( !rPLCF.Get( nP, pData ) )
            return false;
    }

    if((((BYTE*)pData)[0] & 0x1f ) == 0x14 )
    {

        // Field Separator ?
        rPLCF++;

        if( !rPLCF.Get( nP, pData ) )
            return false;

        while ((((BYTE*)pData)[0] & 0x1f ) == 0x13)
        {
            // immer noch neue (nested) Anfaenge ?
            WW8SkipField( rPLCF );          // nested Field im Resultatteil
            if( !rPLCF.Get( nP, pData ) )
                return false;
        }
    }
    rPLCF++;

    return true;
}

static bool WW8GetFieldPara(WW8PLCFspecial& rPLCF, WW8FieldDesc& rF)
{
    void* pData;
    ULONG nOldIdx = rPLCF.GetIdx();

    rF.nLen = rF.nId = rF.nOpt = rF.bCodeNest = rF.bResNest = 0;

    if( !rPLCF.Get( rF.nSCode, pData ) )             // Ende des PLCFspecial ?
        goto Err;

    rPLCF++;

    if((((BYTE*)pData)[0] & 0x1f ) != 0x13 )        // Kein Anfang ?
        goto Err;

    rF.nId = ((BYTE*)pData)[1];

    if( !rPLCF.Get( rF.nLCode, pData ) )
        goto Err;

    rF.nSRes = rF.nLCode;                           // Default
    rF.nSCode++;                                    // ohne Marken
    rF.nLCode -= rF.nSCode;                         // Pos zu Laenge

    while((((BYTE*)pData)[0] & 0x1f ) == 0x13 )
    {
        // immer noch neue (nested) Anfaenge ?
        WW8SkipField( rPLCF );              // nested Field im Beschreibungsteil
        rF.bCodeNest = true;
        if( !rPLCF.Get( rF.nSRes, pData ) )
            goto Err;
    }

    if((((BYTE*)pData)[0] & 0x1f ) == 0x14 ){       // Field Separator ?
        rPLCF++;

        if( !rPLCF.Get( rF.nLRes, pData ) )
            goto Err;

        while((((BYTE*)pData)[0] & 0x1f ) == 0x13 )
        {
            // immer noch neue (nested) Anfaenge ?
            WW8SkipField( rPLCF );              // nested Field im Resultatteil
            rF.bResNest = true;
            if( !rPLCF.Get( rF.nLRes, pData ) )
                goto Err;
        }
        rF.nLen = rF.nLRes - rF.nSCode + 2; // nLRes ist noch die Endposition
        rF.nLRes -= rF.nSRes;                       // nun: nLRes = Laenge
        rF.nSRes++;                                 // Endpos encl. Marken
        rF.nLRes--;

    }else{
        rF.nLRes = 0;                               // Kein Result vorhanden
        rF.nLen = rF.nSRes - rF.nSCode + 2;         // Gesamtlaenge
    }

    rPLCF++;
    if((((BYTE*)pData)[0] & 0x1f ) == 0x15 )
    {
        // Field Ende ?
        // INDEX-Fld hat Bit7 gesetzt!?!
        rF.nOpt = ((BYTE*)pData)[1];                // Ja -> Flags uebernehmen
    }else{
        rF.nId = 0;                                 // Nein -> Feld ungueltig
    }

    rPLCF.SetIdx( nOldIdx );
    return true;
Err:
    rPLCF.SetIdx( nOldIdx );
    return false;
}


//-----------------------------------------


// WW8ReadPString liest einen Pascal-String ein und gibt ihn zurueck. Der
// Pascal- String hat am Ende ein \0, der aber im Laengenbyte nicht
// mitgezaehlt wird.  Der Speicher fuer den Pascalstring wird alloziert.
String WW8ReadPString(SvStream& rStrm, rtl_TextEncoding eEnc,
    bool bAtEndSeekRel1)
{
    ByteString aByteStr;
    UINT8 b;
    rStrm >> b;

    if (b)
    {
        // Alloc methode automatically sets Zero at the end
        sal_Char*  pByteData = aByteStr.AllocBuffer( b );

        ULONG nWasRead = rStrm.Read( pByteData, b );
        if( nWasRead != b )
            aByteStr.ReleaseBufferAccess(static_cast<xub_StrLen>(nWasRead));
    }

    if( bAtEndSeekRel1 )
        rStrm.SeekRel( 1 ); // ueberspringe das Null-Byte am Ende.


    return String( aByteStr, eEnc );
}

String WW8Read_xstz(SvStream& rStrm, USHORT nChars, bool bAtEndSeekRel1)
{
    UINT16 b;

    if( nChars )
        b = nChars;
    else
        rStrm >> b;

    String aStr;
    if (b)
    {
        // Alloc methode automatically sets Zero at the end
        sal_Unicode* pData = aStr.AllocBuffer( b );

        ULONG nWasRead = rStrm.Read( (sal_Char*)pData, b * 2 );
        if( nWasRead != static_cast<ULONG>(b*2) )
        {
            b = static_cast<UINT16>(nWasRead / 2);
            aStr.ReleaseBufferAccess( b );
            pData = aStr.GetBufferAccess();
        }

#ifdef __BIGENDIAN
        ULONG n;
        sal_Unicode *pWork;
        for( n = 0, pWork = pData; n < b; ++n, ++pWork )
            *pWork = SWAPSHORT( *pWork );
#endif // ifdef __BIGENDIAN
    }

    if( bAtEndSeekRel1 )
        rStrm.SeekRel( 2 ); // ueberspringe das Null-Character am Ende.

    return aStr;
}

static ULONG SafeReadString(ByteString &rStr,USHORT nLen,SvStream &rStrm)
{
    ULONG nWasRead=0;
    if (nLen)
    {
        nWasRead = rStrm.Read( rStr.AllocBuffer( nLen ), nLen);
        if( nWasRead != nLen )
            rStr.ReleaseBufferAccess(static_cast<xub_StrLen>(nWasRead));
    }
    return nWasRead;
}

USHORT WW8ScannerBase::WW8ReadString( SvStream& rStrm, String& rStr,
    WW8_CP nAktStartCp, long nTotalLen, rtl_TextEncoding eEnc ) const
{
    // Klartext einlesen, der sich ueber mehrere Pieces erstrecken kann
    rStr.Erase();

    long nTotalRead = 0;
    WW8_CP nBehindTextCp = nAktStartCp + nTotalLen;
    WW8_CP nNextPieceCp  = nBehindTextCp; // Initialisierung wichtig fuer Ver6
    do
    {
        bool bIsUnicode, bPosOk;
        WW8_FC fcAct = WW8Cp2Fc(nAktStartCp,&bIsUnicode,&nNextPieceCp,&bPosOk);

        // vermutlich uebers Dateiende hinaus gezielt, macht nix!
        if( !bPosOk )
            break;

        rStrm.Seek( fcAct );

        long nLen = ( (nNextPieceCp < nBehindTextCp) ? nNextPieceCp
            : nBehindTextCp ) - nAktStartCp;

        if( 0 >= nLen )
            break;

        if( nLen > USHRT_MAX - 1 )
            nLen = USHRT_MAX - 1;

        if( bIsUnicode )
            rStr.Append(WW8Read_xstz(rStrm, (USHORT)nLen, false));
        else
        {
            // Alloc method automatically sets Zero at the end
            ByteString aByteStr;
            SafeReadString(aByteStr,(USHORT)nLen,rStrm);
            rStr += String( aByteStr, eEnc );
        }
        nTotalRead  += nLen;
        nAktStartCp += nLen;
        if ( nTotalRead != rStr.Len() )
            break;
    }
    while( nTotalRead < nTotalLen );

    return rStr.Len();
}

//-----------------------------------------
//              WW8PLCFspecial
//-----------------------------------------

// Bei nStartPos < 0 wird das erste Element des PLCFs genommen
WW8PLCFspecial::WW8PLCFspecial(SvStream* pSt, long nFilePos, long nPLCF,
    long nStruct, long nStartPos, bool bNoEnd)
    : nIdx(0), nStru(nStruct)
{
    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );
    // Pointer auf Pos- u. Struct-Array
    pPLCF_PosArray = new INT32[ ( nPLCF + 3 ) / 4 ];

    long nOldPos = pSt->Tell();

    pSt->Seek( nFilePos );
    pSt->Read( pPLCF_PosArray, nPLCF );
#ifdef __BIGENDIAN
    for( nIdx = 0; nIdx <= nIMax; nIdx++ )
        pPLCF_PosArray[nIdx] = SWAPLONG( pPLCF_PosArray[nIdx] );
    nIdx = 0;
#endif // __BIGENDIAN
    if( bNoEnd )
        nIMax++;
    if( nStruct ) // Pointer auf Inhalts-Array
        pPLCF_Contents = (BYTE*)&pPLCF_PosArray[nIMax + 1];
    else
        pPLCF_Contents = 0;                         // kein Inhalt
    if( nStartPos >= 0 )
        SeekPos( nStartPos );

    pSt->Seek( nOldPos );
}

// WW8PLCFspecial::SeekPos() stellt den WW8PLCFspecial auf die Stelle nPos, wobei auch noch der
// Eintrag benutzt wird, der vor nPos beginnt und bis hinter nPos reicht.
// geeignet fuer normale Attribute. Allerdings wird der Attributanfang nicht
// auf die Position nPos korrigiert.
bool WW8PLCFspecial::SeekPos(long nP)
{
    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return false;   // Not found: nP unterhalb kleinstem Eintrag
    }

    // Search from beginning?
    if( (1 > nIdx) || (nP < pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = nIMax;

    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                                   // Suchen mit um 1 erhoehtem Index
            if( nP < pPLCF_PosArray[nI] )
            {                               // Position gefunden
                nIdx = nI - 1;              // nI - 1 ist der richtige Index
                return true;                // ... und fertig
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
    nIdx = nIMax;               // Nicht gefunden, groesser als alle Eintraege
    return false;
}

// WW8PLCFspecial::SeekPosExact() wie SeekPos(), aber es wird sichergestellt,
// dass kein Attribut angeschnitten wird, d.h. das naechste gelieferte
// Attribut beginnt auf oder hinter nPos. Wird benutzt fuer Felder +
// Bookmarks.
bool WW8PLCFspecial::SeekPosExact(long nP)
{
    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return false;       // Not found: nP unterhalb kleinstem Eintrag
    }
    // Search from beginning?
    if( nP <=pPLCF_PosArray[nIdx] )
        nIdx = 0;

    long nI   = nIdx ? nIdx-1 : 0;
    long nEnd = nIMax;

    for(int n = (0==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI < nEnd; ++nI)
        {
            if( nP <=pPLCF_PosArray[nI] )
            {                           // Position gefunden
                nIdx = nI;              // nI     ist der richtige Index
                return true;            // ... und fertig
            }
        }
        nI   = 0;
        nEnd = nIdx;
    }
    nIdx = nIMax;               // Not found, groesser als alle Eintraege
    return false;
}

bool WW8PLCFspecial::Get(long& rPos, void*& rpValue) const
{
    return GetData( nIdx, rPos, rpValue );
}

bool WW8PLCFspecial::GetData(long nInIdx, long& rPos, void*& rpValue) const
{
    if ( nInIdx >= nIMax )
    {
        rPos = LONG_MAX;
        return false;
    }
    rPos = pPLCF_PosArray[nInIdx];
    rpValue = pPLCF_Contents ? (void*)&pPLCF_Contents[nInIdx * nStru] : 0;
    return true;
}

//-----------------------------------------
//              WW8PLCF z.B. fuer SEPX
//-----------------------------------------

// Ctor fuer *andere* als Fkps
// Bei nStartPos < 0 wird das erste Element des PLCFs genommen
WW8PLCF::WW8PLCF( SvStream* pSt, long nFilePos, long nPLCF, long nStruct,
    long nStartPos ) :nIdx( 0 ), nStru( nStruct )
{
    ASSERT( nPLCF, "WW8PLCF: nPLCF ist Null!" );

    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );

    ReadPLCF( pSt, nFilePos, nPLCF );

    if( nStartPos >= 0 )
        SeekPos( nStartPos );
}

// Ctor *nur* fuer Fkps
// Die letzten 2 Parameter sind fuer PLCF.Chpx und PLCF.Papx noetig.  ist ncpN
// != 0, dann wird ein unvollstaendiger PLCF vervollstaendigt.  Das ist bei
// WW6 bei Resourcenmangel und bei WordPad (W95) immer noetig.  Bei nStartPos
// < 0 wird das erste Element des PLCFs genommen
WW8PLCF::WW8PLCF( SvStream* pSt, long nFilePos, long nPLCF, long nStruct,
    long nStartPos, long nPN, long ncpN ) :nIdx( 0 ), nStru( nStruct )
{
    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );

    if( nIMax >= (long) ncpN )
        ReadPLCF( pSt, nFilePos, nPLCF );
    else
        GeneratePLCF( pSt, nPN, ncpN );

    if( nStartPos >= 0 )
        SeekPos( nStartPos );
}

void WW8PLCF::ReadPLCF( SvStream* pSt, long nFilePos, long nPLCF )
{
    // Pointer auf Pos-Array
    pPLCF_PosArray = new INT32[ ( nPLCF + 3 ) / 4 ];

    long nOldPos = pSt->Tell();

    pSt->Seek( nFilePos );
    pSt->Read( pPLCF_PosArray, nPLCF );
#ifdef __BIGENDIAN
    for( nIdx = 0; nIdx <= nIMax; nIdx++ )
        pPLCF_PosArray[nIdx] = SWAPLONG( pPLCF_PosArray[nIdx] );
    nIdx = 0;
#endif // __BIGENDIAN
    // Pointer auf Inhalts-Array
    pPLCF_Contents = (BYTE*)&pPLCF_PosArray[nIMax + 1];

    pSt->Seek( nOldPos );
}

void WW8PLCF::GeneratePLCF( SvStream* pSt, long nPN, long ncpN )
{
    ASSERT( nIMax < (long)ncpN, "Pcl.Fkp: Warum ist PLCF zu gross ?" );
    nIMax = ncpN;
    long nSiz = 6 * nIMax + 4;
    pPLCF_PosArray = new INT32[ ( nSiz + 3 ) / 4 ]; // Pointer auf Pos-Array
    memset( pPLCF_PosArray, 0, (size_t)nSiz );

    INT32 nFc;
    USHORT i;

    for( i = 0; i < ncpN; i++ ){        // Baue FC-Eintraege
        pSt->Seek( ( nPN + i ) << 9 );  // erster FC-Eintrag jedes Fkp
        *pSt >> nFc;
        pPLCF_PosArray[i] = nFc;
    }
    ULONG nLastFkpPos = ( ( nPN + nIMax - 1 ) << 9 );
    pSt->Seek( nLastFkpPos + 511 );     // Anz. Fkp-Eintraege des letzten Fkp
    BYTE nb;
    *pSt >> nb;
    pSt->Seek( nLastFkpPos + nb * 4 );  // letzer FC-Eintrag des letzten Fkp
    *pSt >> nFc;
    pPLCF_PosArray[nIMax] = nFc;        // Ende des letzten Fkp

    // Pointer auf Inhalts-Array
    pPLCF_Contents = (BYTE*)&pPLCF_PosArray[nIMax + 1];
    BYTE* p = pPLCF_Contents;

    for( i = 0; i < ncpN; i++ )         // Baue PNs
    {
        ShortToSVBT16(nPN + i, p);
        p+=2;
    }
}

bool WW8PLCF::SeekPos(long nPos)
{
    long nP = nPos;

    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        // Nicht gefunden: nPos unterhalb kleinstem Eintrag
        return false;
    }

    // Search from beginning?
    if( (1 > nIdx) || (nP < pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = nIMax;

    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)             // Suchen mit um 1 erhoehtem Index
        {
            if( nP < pPLCF_PosArray[nI] )   // Position gefunden
            {
                nIdx = nI - 1;              // nI - 1 ist der richtige Index
                return true;                // ... und fertig
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }

    nIdx = nIMax;               // Nicht gefunden, groesser als alle Eintraege
    return false;
}

bool WW8PLCF::Get(long& rStart, long& rEnd, void*& rpValue) const
{
    if ( nIdx >= nIMax )
    {
        rStart = rEnd = LONG_MAX;
        return false;
    }
    rStart = pPLCF_PosArray[ nIdx ];
    rEnd   = pPLCF_PosArray[ nIdx + 1 ];
    rpValue = (void*)&pPLCF_Contents[nIdx * nStru];
    return true;
}

long WW8PLCF::Where() const
{
    if ( nIdx >= nIMax )
        return LONG_MAX;

    return pPLCF_PosArray[nIdx];
}

//-----------------------------------------
//              WW8PLCFpcd
//-----------------------------------------

WW8PLCFpcd::WW8PLCFpcd( SvStream* pSt, long nFilePos, long nPLCF, long nStruct )
    :nStru( nStruct )
{
    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );
    pPLCF_PosArray = new INT32[ ( nPLCF + 3 ) / 4 ];    // Pointer auf Pos-Array

    long nOldPos = pSt->Tell();

    pSt->Seek( nFilePos );
    pSt->Read( pPLCF_PosArray, nPLCF );
#ifdef __BIGENDIAN
    for( long nI = 0; nI <= nIMax; nI++ )
      pPLCF_PosArray[nI] = SWAPLONG( pPLCF_PosArray[nI] );
#endif // __BIGENDIAN

    // Pointer auf Inhalts-Array
    pPLCF_Contents = (BYTE*)&pPLCF_PosArray[nIMax + 1];

    pSt->Seek( nOldPos );
}

// Bei nStartPos < 0 wird das erste Element des PLCFs genommen
WW8PLCFpcd_Iter::WW8PLCFpcd_Iter( WW8PLCFpcd& rPLCFpcd, long nStartPos )
    :rPLCF( rPLCFpcd ), nIdx( 0 )
{
    if( nStartPos >= 0 )
        SeekPos( nStartPos );
}

bool WW8PLCFpcd_Iter::SeekPos(long nPos)
{
    long nP = nPos;

    if( nP < rPLCF.pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return false;       // Nicht gefunden: nPos unterhalb kleinstem Eintrag
    }
    // Search from beginning?
    if( (1 > nIdx) || (nP < rPLCF.pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = rPLCF.nIMax;

    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                               // Suchen mit um 1 erhoehtem Index
            if( nP < rPLCF.pPLCF_PosArray[nI] )
            {                           // Position gefunden
                nIdx = nI - 1;          // nI - 1 ist der richtige Index
                return true;            // ... und fertig
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
    nIdx = rPLCF.nIMax;         // Nicht gefunden, groesser als alle Eintraege
    return false;
}

bool WW8PLCFpcd_Iter::Get(long& rStart, long& rEnd, void*& rpValue) const
{
    if( nIdx >= rPLCF.nIMax )
    {
        rStart = rEnd = LONG_MAX;
        return false;
    }
    rStart = rPLCF.pPLCF_PosArray[nIdx];
    rEnd = rPLCF.pPLCF_PosArray[nIdx + 1];
    rpValue = (void*)&rPLCF.pPLCF_Contents[nIdx * rPLCF.nStru];
    return true;
}

long WW8PLCFpcd_Iter::Where() const
{
    if ( nIdx >= rPLCF.nIMax )
        return LONG_MAX;

    return rPLCF.pPLCF_PosArray[nIdx];
}

//-----------------------------------------
bool WW8PLCFx_Fc_FKP::WW8Fkp::Entry::operator<
    (const WW8PLCFx_Fc_FKP::WW8Fkp::Entry& rSecond) const
{
    return (mnFC < rSecond.mnFC);
}

WW8PLCFx_Fc_FKP::WW8Fkp::WW8Fkp(BYTE nFibVer, SvStream* pSt, SvStream* pDataSt,
    long _nFilePos, long nItemSiz, ePLCFT ePl, WW8_FC nStartFc)
    : nItemSize(nItemSiz), nFilePos(_nFilePos),  nIdx(0), ePLCF(ePl),
    maSprmParser(nFibVer)
{
    long nOldPos = pSt->Tell();

    pSt->Seek(nFilePos);
    pSt->Read(maRawData, 512);
    nIMax = maRawData[511];

    sal_uInt8 *pStart = maRawData;
    // Pointer to Offset-Location in maRawData
    sal_uInt8* pOfs = maRawData + (nIMax + 1) * 4;

    for (nIdx = 0; nIdx < nIMax; ++nIdx)
    {
        USHORT nOfs = (*(pOfs + nIdx * nItemSize)) * 2;
        Entry aEntry(Get_Long(pStart));

        if (nOfs)
        {
            switch (ePLCF)
            {
                case CHP:
                    aEntry.mnLen  = maRawData[nOfs];
                    aEntry.mpData = maRawData + nOfs + 1;
                    break;
                case PAP:
                    sal_uInt8 nDelta = 0;

                    aEntry.mnLen = maRawData[nOfs];
                    if (8 <= nFibVer && !aEntry.mnLen)
                    {
                        aEntry.mnLen = maRawData[ nOfs+1 ];
                        nDelta++;
                    }

                    aEntry.mnIStd = SVBT16ToShort(maRawData+nOfs+1+nDelta);

                    aEntry.mpData = maRawData + nOfs + 3+ nDelta;

                    USHORT nSpId = maSprmParser.GetSprmId(aEntry.mpData);

                    if (0x6645 == nSpId || 0x6646 == nSpId)
                    {
                        UINT32 nCurr = pDataSt->Tell();

                        UINT32 nPos = SVBT32ToLong(aEntry.mpData + 2);
                        pDataSt->Seek(nPos);
                        *pDataSt >> aEntry.mnLen;
                        aEntry.mpData = new sal_uInt8[aEntry.mnLen];
                        aEntry.mbMustDelete = true;
                        pDataSt->Read(aEntry.mpData, aEntry.mnLen);

                        pDataSt->Seek( nCurr );
                    }
                    else
                    {
                        aEntry.mnLen *= 2;
                        aEntry.mnLen -= 2;
                    }
                    break;
            }
        }

        maEntries.push_back(aEntry);
    }

    //one more FC than grrpl entries
    maEntries.push_back(Entry(Get_Long(pStart)));

    //#104773#, we expect them sorted, but it appears possible
    //for them to arive unsorted
    std::sort(maEntries.begin(), maEntries.end());

    nIdx = 0;

    if (nStartFc >= 0)
        SeekPos(nStartFc);

    pSt->Seek(nOldPos);
}

WW8PLCFx_Fc_FKP::WW8Fkp::Entry::Entry(const Entry &rEntry)
    : mnFC(rEntry.mnFC), mnLen(rEntry.mnLen), mnIStd(rEntry.mnIStd),
    mbMustDelete(rEntry.mbMustDelete)
{
    if (mbMustDelete)
    {
        mpData = new sal_uInt8[mnLen];
        memcpy(mpData, rEntry.mpData, mnLen);
    }
    else
        mpData = rEntry.mpData;
}

WW8PLCFx_Fc_FKP::WW8Fkp::Entry&
    WW8PLCFx_Fc_FKP::WW8Fkp::Entry::operator=(const Entry &rEntry)
{
    if (mbMustDelete)
        delete[] mpData;

    mnFC = rEntry.mnFC;
    mnLen = rEntry.mnLen;
    mnIStd = rEntry.mnIStd;
    mbMustDelete = rEntry.mbMustDelete;

    if (mbMustDelete)
    {
        mpData = new sal_uInt8[mnLen];
        memcpy(mpData, rEntry.mpData, mnLen);
    }
    else
        mpData = rEntry.mpData;
    return *this;
}

WW8PLCFx_Fc_FKP::WW8Fkp::Entry::~Entry()
{
    if (mbMustDelete)
        delete[] mpData;
}

void WW8PLCFx_Fc_FKP::WW8Fkp::Reset(WW8_FC nFc)
{
    SetIdx(0);
    if (nFc >= 0)
        SeekPos(nFc);
}

bool WW8PLCFx_Fc_FKP::WW8Fkp::SeekPos(WW8_FC nFc)
{
    if (nFc < maEntries[0].mnFC)
    {
        nIdx = 0;
        return false;       // Nicht gefunden: nPos unterhalb kleinstem Eintrag
    }
    // Search from beginning?
    if( (1 > nIdx) || (nFc < maEntries[nIdx-1].mnFC) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = nIMax;

    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                               // Suchen mit um 1 erhoehtem Index
            if (nFc < maEntries[nI].mnFC)
            {                           // Position gefunden
                nIdx = nI - 1;          // nI - 1 ist der richtige Index
                return true;            // ... und fertig
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
    nIdx = nIMax;               // Nicht gefunden, groesser als alle Eintraege
    return false;
}

BYTE* WW8PLCFx_Fc_FKP::WW8Fkp::Get( WW8_FC& rStart, WW8_FC& rEnd, long& rLen )
    const
{
    rLen = 0;

    if( nIdx >= nIMax )
    {
        rStart = LONG_MAX;
        return 0;
    }

    rStart = maEntries[nIdx].mnFC;
    rEnd   = maEntries[nIdx + 1].mnFC;

    BYTE* pSprms = GetLenAndIStdAndSprms( rLen );
    return pSprms;
}

void WW8PLCFx_Fc_FKP::WW8Fkp::SetIdx( ULONG nI )
{
    if( nI < nIMax)
        nIdx = (short)nI;
}

BYTE* WW8PLCFx_Fc_FKP::WW8Fkp::GetLenAndIStdAndSprms(long& rLen) const
{
    rLen = maEntries[nIdx].mnLen;
    return maEntries[nIdx].mpData;
}

const BYTE* WW8PLCFx_Fc_FKP::WW8Fkp::HasSprm( USHORT nId )
{
    if( nIdx >= nIMax )
        return 0;

    long nLen;
    BYTE* pSprms = GetLenAndIStdAndSprms( nLen );

    WW8SprmIter aIter(pSprms, nLen, maSprmParser);
    return aIter.FindSprm(nId);
}

bool WW8PLCFx_Fc_FKP::WW8Fkp::HasSprm(USHORT nId,
    std::vector<const BYTE *> &rResult)
{
    if (nIdx >= nIMax)
       return false;

    long nLen;
    BYTE* pSprms = GetLenAndIStdAndSprms( nLen );

    WW8SprmIter aIter(pSprms, nLen, maSprmParser);

    while(aIter.GetSprms())
    {
        if (aIter.GetAktId() == nId)
            rResult.push_back(aIter.GetAktParams());
        aIter++;
    };
    return !rResult.empty();
}

//-----------------------------------------
void WW8PLCFx::GetSprms( WW8PLCFxDesc* p )
{
    ASSERT( !this, "Falsches GetSprms gerufen" );
    p->nStartPos = p->nEndPos = LONG_MAX;
    p->pMemPos = 0;
    p->nSprmsLen = 0;
    p->bRealLineEnd = false;
    return;
}

long WW8PLCFx::GetNoSprms( long& rStart, long& rEnd, long& rLen )
{
    ASSERT( !this, "Falsches GetNoSprms gerufen" );
    rStart = rEnd = LONG_MAX;
    rLen = 0;
    return 0;
}

// ...Idx2: Default: ignorieren
ULONG WW8PLCFx::GetIdx2() const
{
    return 0;
}

void WW8PLCFx::SetIdx2(ULONG )
{
}

class SamePos :
    public std::unary_function<const WW8PLCFx_Fc_FKP::WW8Fkp *, bool>
{
private:
    long mnPo;
public:
    SamePos(long nPo) : mnPo(nPo) {};
    bool operator()(const WW8PLCFx_Fc_FKP::WW8Fkp *pFkp)
        {return mnPo == pFkp->GetFilePos();}
};

//-----------------------------------------
bool WW8PLCFx_Fc_FKP::NewFkp()
{
    long nPLCFStart, nPLCFEnd;
    void* pPage;

    static const int WW8FkpSizeTabVer6[ PLCF_END ] =
    {
        1,  7, 0 /*, 0, 0, 0*/
    };
    static const int WW8FkpSizeTabVer8[ PLCF_END ] =
    {
        1, 13, 0 /*, 0, 0, 0*/
    };
    const int* pFkpSizeTab;
    switch (GetVersion())
    {
        case 6:
        case 7:
            pFkpSizeTab = WW8FkpSizeTabVer6;
            break;
        case 8:
            pFkpSizeTab = WW8FkpSizeTabVer8;
            break;
        default:
            // Programm-Fehler!
            ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
            return false;
    }

    if (!pPLCF->Get( nPLCFStart, nPLCFEnd, pPage ))
    {
        pFkp = 0;
        return false;                           // PLCF fertig abgearbeitet
    }
    (*pPLCF)++;
    long nPo = SVBT16ToShort( (BYTE *)pPage );
    nPo <<= 9;                                  // shift als LONG

    long nAktFkpFilePos = pFkp ? pFkp->GetFilePos() : -1;
    if (nAktFkpFilePos == nPo)
        pFkp->Reset(GetStartFc()); // #79464# //
    else
    {
        myiter aIter =
            std::find_if(maFkpCache.begin(), maFkpCache.end(), SamePos(nPo));
        if (aIter != maFkpCache.end())
        {
            pFkp = *aIter;
            pFkp->Reset(GetStartFc());
        }
        else if ((pFkp = new WW8Fkp(GetVersion(), pFKPStrm, pDataStrm, nPo,
            pFkpSizeTab[ ePLCF ], ePLCF, GetStartFc())))
        {
            maFkpCache.push_back(pFkp);

            if (maFkpCache.size() > eMaxCache)
            {
                delete maFkpCache.front();
                maFkpCache.pop_front();
            }
        }
    }

    SetStartFc( -1 );                                   // Nur das erste Mal
    return true;
}

WW8PLCFx_Fc_FKP::WW8PLCFx_Fc_FKP(SvStream* pSt, SvStream* pTblSt,
    SvStream* pDataSt, const WW8Fib& rFib, ePLCFT ePl, WW8_FC nStartFcL)
    : WW8PLCFx(rFib.nVersion, true), pFKPStrm(pSt), pDataStrm(pDataSt),
    pFkp(0), ePLCF(ePl), pPCDAttrs(0)
{
    SetStartFc(nStartFcL);
    long nLenStruct = (8 > rFib.nVersion) ? 2 : 4;
    if (ePl == CHP)
    {
        pPLCF = new WW8PLCF(pTblSt, rFib.fcPlcfbteChpx, rFib.lcbPlcfbteChpx,
            nLenStruct, GetStartFc(), rFib.pnChpFirst, rFib.cpnBteChp);
    }
    else
    {
        pPLCF = new WW8PLCF(pTblSt, rFib.fcPlcfbtePapx, rFib.lcbPlcfbtePapx,
            nLenStruct, GetStartFc(), rFib.pnPapFirst, rFib.cpnBtePap);
    }
}

WW8PLCFx_Fc_FKP::~WW8PLCFx_Fc_FKP()
{
    myiter aEnd = maFkpCache.end();
    for (myiter aIter = maFkpCache.begin(); aIter != aEnd; ++aIter)
        delete *aIter;
    delete pPLCF;
    delete pPCDAttrs;
}

ULONG WW8PLCFx_Fc_FKP::GetIdx() const
{
    ULONG u = pPLCF->GetIdx() << 8;
    if (pFkp)
        u |= pFkp->GetIdx();
    return u;
}

void WW8PLCFx_Fc_FKP::SetIdx( ULONG nIdx )
{
    if( !( nIdx & 0xffffff00L ) )
    {
        pPLCF->SetIdx( nIdx >> 8 );
        pFkp = 0;
    }
    else
    {                                   //Es gab einen Fkp
        //Lese PLCF um 1 Pos zurueck, um die Adresse des Fkp wiederzubekommen
        pPLCF->SetIdx( ( nIdx >> 8 ) - 1 );
        if ( NewFkp() )                     // und lese Fkp wieder ein
            pFkp->SetIdx( nIdx & 0xff );    // Dann stelle Fkp-Pos wieder ein
    }
}

bool WW8PLCFx_Fc_FKP::SeekPos(WW8_FC nFcPos)
{
    // StartPos for next Where()
    SetStartFc( nFcPos );

    // find StartPos for next pPLCF->Get()
    bool bRet = pPLCF->SeekPos(nFcPos);

    // make FKP invalid?
    long nPLCFStart, nPLCFEnd;
    void* pPage;
    if( pFkp && pPLCF->Get( nPLCFStart, nPLCFEnd, pPage ) )
    {
        long nPo = SVBT16ToShort( (BYTE *)pPage );
        nPo <<= 9;                                          // shift als LONG
        if (nPo != pFkp->GetFilePos())
            pFkp = 0;
        else
            pFkp->SeekPos( nFcPos );
    }
    return bRet;
}

WW8_FC WW8PLCFx_Fc_FKP::Where()
{
    if( !pFkp )
    {
        if( !NewFkp() )
            return LONG_MAX;
    }
    WW8_FC nP = pFkp->Where();
    if( nP != LONG_MAX )
        return nP;

    pFkp = 0;                   // FKP beendet -> hole neuen
    return Where();                     // am einfachsten rekursiv
}

BYTE* WW8PLCFx_Fc_FKP::GetSprmsAndPos(WW8_FC& rStart, WW8_FC& rEnd, long& rLen)
{
    rLen = 0;                               // Default
    rStart = rEnd = LONG_MAX;

    if( !pFkp )     // Fkp not there ?
    {
        if( !NewFkp() )
            return 0;
    }

    BYTE* pPos = pFkp->Get( rStart, rEnd, rLen );
    if( rStart == LONG_MAX )    //Not found
        return 0;
    return pPos;
}

WW8PLCFx& WW8PLCFx_Fc_FKP::operator ++( int )
{
    if( !pFkp )
    {
        if( !NewFkp() )
            return *this;
    }

    (*pFkp)++;
    if( pFkp->Where() == LONG_MAX )
        NewFkp();

    return *this;
}

USHORT WW8PLCFx_Fc_FKP::GetIstd() const
{
    return pFkp ? pFkp->GetIstd() : 0xFFFF;
}

void WW8PLCFx_Fc_FKP::GetPCDSprms( WW8PLCFxDesc& rDesc )
{
    rDesc.pMemPos   = 0;
    rDesc.nSprmsLen = 0;
    if( pPCDAttrs )
    {
        if( !pFkp )
        {
            DBG_WARNING(
                "+Problem: GetPCDSprms: NewFkp necessay (not possible!)" );
            if( !NewFkp() )
                return;
        }
        pPCDAttrs->GetSprms(&rDesc);
    }
}

const BYTE* WW8PLCFx_Fc_FKP::HasSprm( USHORT nId )
{
    // const waere schoener, aber dafuer muesste NewFkp() ersetzt werden oder
    // wegfallen
    if( !pFkp )
    {
        DBG_WARNING( "+Motz: HasSprm: NewFkp noetig ( kein const moeglich )" );
        // Passiert bei BugDoc 31722
        if( !NewFkp() )
            return 0;
    }

    const BYTE* pRes = pFkp->HasSprm( nId );

    if( !pRes )
    {
        WW8PLCFxDesc aDesc;
        GetPCDSprms( aDesc );

        if (aDesc.pMemPos)
        {
            WW8SprmIter aIter(aDesc.pMemPos, aDesc.nSprmsLen,
                pFkp->GetSprmParser());
            pRes = aIter.FindSprm(nId);
        }
    }

    return pRes;
}

bool WW8PLCFx_Fc_FKP::HasSprm(USHORT nId, std::vector<const BYTE *> &rResult)
{
    // const waere schoener, aber dafuer muesste NewFkp() ersetzt werden oder
    // wegfallen
    if (!pFkp)
    {
       DBG_WARNING( "+Motz: HasSprm: NewFkp noetig ( kein const moeglich )" );
       // Passiert bei BugDoc 31722
       if( !NewFkp() )
           return 0;
    }

    pFkp->HasSprm(nId, rResult);

    WW8PLCFxDesc aDesc;
    GetPCDSprms( aDesc );

    if (aDesc.pMemPos)
    {
        WW8SprmIter aIter(aDesc.pMemPos, aDesc.nSprmsLen,
            pFkp->GetSprmParser());
        while(aIter.GetSprms())
        {
            if (aIter.GetAktId() == nId)
                rResult.push_back(aIter.GetAktParams());
            aIter++;
        };
    }
    return !rResult.empty();
}

//-----------------------------------------

WW8PLCFx_Cp_FKP::WW8PLCFx_Cp_FKP( SvStream* pSt, SvStream* pTblSt,
    SvStream* pDataSt, const WW8ScannerBase& rBase, ePLCFT ePl )
    : WW8PLCFx_Fc_FKP(pSt, pTblSt, pDataSt, *rBase.pWw8Fib, ePl,
    rBase.WW8Cp2Fc(0)), rSBase(rBase), nAttrStart(-1), nAttrEnd(-1),
    bLineEnd(false),
    bComplex( (7 < rBase.pWw8Fib->nVersion) || (0 != rBase.pWw8Fib->fComplex) )
{
    ResetAttrStartEnd();

    pPcd = rSBase.pPiecePLCF ? new WW8PLCFx_PCD( rBase.pWw8Fib->nVersion,
        rBase.pPiecePLCF, 0, ((6 == GetVersion()) || (7 == GetVersion())) )
        : 0;

    /*
    Make a copy of the piece attributes for so that the calls to HasSprm on a
    Fc_FKP will be able to take into account the current piece attributes,
    despite the fact that such attributes can only be found through a cp based
    mechanism.
    */
    if (pPcd)
    {
        pPCDAttrs = rSBase.pPLCFx_PCDAttrs ? new WW8PLCFx_PCDAttrs(
            rSBase.pWw8Fib->nVersion, pPcd, &rSBase) : 0;
    }

    pPieceIter = rSBase.pPieceIter;
}

WW8PLCFx_Cp_FKP::~WW8PLCFx_Cp_FKP()
{
    delete pPcd;
}

void WW8PLCFx_Cp_FKP::ResetAttrStartEnd()
{
    nAttrStart = -1;
    nAttrEnd   = -1;
    bLineEnd   = false;
}

ULONG WW8PLCFx_Cp_FKP::GetPCDIMax() const
{
    return pPcd ? pPcd->GetIMax() : 0;
}

ULONG WW8PLCFx_Cp_FKP::GetPCDIdx() const
{
    return pPcd ? pPcd->GetIdx() : 0;
}

void WW8PLCFx_Cp_FKP::SetPCDIdx( ULONG nIdx )
{
    if( pPcd )
        pPcd->SetIdx( nIdx );
}

bool WW8PLCFx_Cp_FKP::SeekPos(WW8_CP nCpPos)
{
    if( pPcd )  // Complex
    {
        if( !pPcd->SeekPos( nCpPos ) )  // Piece setzen
            return false;
        if (pPCDAttrs && !pPCDAttrs->GetIter()->SeekPos(nCpPos))
            return false;
        return WW8PLCFx_Fc_FKP::SeekPos(pPcd->AktPieceStartCp2Fc(nCpPos));
    }
                                    // KEINE Piece-Table !!!
    return WW8PLCFx_Fc_FKP::SeekPos( rSBase.WW8Cp2Fc(nCpPos) );
}

WW8_CP WW8PLCFx_Cp_FKP::Where()
{
    WW8_FC nFc = WW8PLCFx_Fc_FKP::Where();
    if( pPcd )
        return pPcd->AktPieceStartFc2Cp( nFc ); // Piece ermitteln
    return rSBase.WW8Fc2Cp( nFc );      // KEINE Piece-Table !!!
}

void WW8PLCFx_Cp_FKP::GetSprms(WW8PLCFxDesc* p)
{
    WW8_CP nOrigCp = p->nStartPos;

    if (!GetDirty())        //Normal case
    {
        p->pMemPos = WW8PLCFx_Fc_FKP::GetSprmsAndPos(p->nStartPos, p->nEndPos,
            p->nSprmsLen);
    }
    else
    {
        /*
        #93702#
        For the odd case where we have a location in a fastsaved file which
        does not have an entry in the FKP, perhaps its para end is in the next
        piece, or perhaps the cp just doesn't exist at all in this document.
        AdvSprm doesn't know so it sets the PLCF as dirty and we figure out
        in this method what the situation is

        It doesn't exist then the piece iterator will not be able to find it.
        Otherwise our cool fastsave algorithm can be brought to bear on the
        problem.
        */
        ULONG nOldPos = pPieceIter->GetIdx();
        bool bOk = pPieceIter->SeekPos(nOrigCp);
        pPieceIter->SetIdx( nOldPos );
        if (!bOk)
            return;
    }

    if( pPcd )  // Piece-Table vorhanden !!!
    {
        // Init ( noch kein ++ gerufen )
        if( (nAttrStart >  nAttrEnd) || (nAttrStart == -1) )
        {
            p->bRealLineEnd = (ePLCF == PAP);

            if ( ((ePLCF == PAP ) || (ePLCF == CHP)) && (nOrigCp != LONG_MAX) )
            {
                bool bIsUnicode=false;
                /*
                To find the end of a paragraph for a character in a
                complex format file.

                It is necessary to know the piece that contains the
                character and the FC assigned to the character.
                */

                //We set the piece iterator to the piece that contains the
                //character, now we have the correct piece for this character
                ULONG nOldPos = pPieceIter->GetIdx();
                p->nStartPos = nOrigCp;
                pPieceIter->SeekPos( p->nStartPos);

                //This is the FC assigned to the character, but we already
                //have the result of the next stage, so we can skip this step
                //WW8_FC nStartFc = rSBase.WW8Cp2Fc(p->nStartPos, &bIsUnicode);

                /*
                Using the FC of the character, first search the FKP that
                describes the character to find the smallest FC in the rgfc
                that is larger than the character FC.
                */
                //But the search has already been done, the next largest FC is
                //p->nEndPos.
                WW8_FC nOldEndPos = p->nEndPos;

                /*
                If the FC found in the FKP is less than or equal to the limit
                FC of the piece, the end of the paragraph that contains the
                character is at the FKP FC minus 1.
                */
                long nCpStart, nCpEnd;
                void* pData;
                pPieceIter->Get(nCpStart, nCpEnd, pData);

                WW8_FC nLimitFC = SVBT32ToLong( ((WW8_PCD*)pData)->fc );
                WW8_FC nBeginLimitFC = nLimitFC;
                if( 8 <= GetVersion() )
                {
                    nBeginLimitFC =
                        WW8PLCFx_PCD::TransformPieceAddress(nLimitFC,
                        bIsUnicode);
                }
                nLimitFC = nBeginLimitFC +
                    (nCpEnd - nCpStart) * (bIsUnicode ? 2 : 1);

                if (nOldEndPos <= nLimitFC)
                {
                    p->nEndPos = nCpEnd -
                        (nLimitFC-nOldEndPos) / (bIsUnicode ? 2 : 1);
                }
                else
                {
                    if (ePLCF == CHP)
                        p->nEndPos = nCpEnd;
                    else
                    {
                        /*
                        If the FKP FC that was found was greater than the FC
                        of the end of the piece, scan piece by piece toward
                        the end of the document until a piece is found that
                        contains a  paragraph end mark.
                        */

                        /*
                        It's possible to check if a piece contains a paragraph
                        mark by using the FC of the beginning of the piece to
                        search in the FKPs for the smallest FC in the FKP rgfc
                        that is greater than the FC of the beginning of the
                        piece. If the FC found is less than or equal to the
                        limit FC of the piece, then the character that ends
                        the paragraph is the character immediately before the
                        FKP fc
                        */

                        (*pPieceIter)++;

                        for (;pPieceIter->GetIdx() < pPieceIter->GetIMax();
                            (*pPieceIter)++)
                        {
                            if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
                            {
                                ASSERT( !this, "piece iter broken!" );
                                break;
                            }
                            bIsUnicode = false;
                            INT32 nFcStart=SVBT32ToLong(((WW8_PCD*)pData)->fc);

                            if( 8 <= GetVersion() )
                            {
                                nFcStart =
                                    WW8PLCFx_PCD::TransformPieceAddress(
                                    nFcStart,bIsUnicode );
                            }
                            nLimitFC = nFcStart + (nCpEnd - nCpStart) *
                                (bIsUnicode ? 2 : 1);

                            //if it doesn't exist, skip it
                            if (!SeekPos(nCpStart))
                                continue;

                            WW8_FC nOne,nSmallest;
                            p->pMemPos = WW8PLCFx_Fc_FKP::GetSprmsAndPos(nOne,
                                nSmallest, p->nSprmsLen);

                            if (nSmallest <= nLimitFC)
                            {
                                p->nEndPos = nCpEnd -
                                    (nLimitFC-nSmallest) / (bIsUnicode ? 2 : 1);
                                break;
                            }
                        }
                    }
                }
                pPieceIter->SetIdx( nOldPos );
            }
            else
                pPcd->AktPieceFc2Cp( p->nStartPos, p->nEndPos,&rSBase );
        }
        else
        {
            p->nStartPos = nAttrStart;
            p->nEndPos = nAttrEnd;
            p->bRealLineEnd = bLineEnd;
        }
    }
    else        // KEINE Piece-Table !!!
    {
        p->nStartPos = rSBase.WW8Fc2Cp( p->nStartPos );
        p->nEndPos   = rSBase.WW8Fc2Cp( p->nEndPos );
        p->bRealLineEnd = ePLCF == PAP;
    }
}

WW8PLCFx& WW8PLCFx_Cp_FKP::operator ++( int )
{
    WW8PLCFx_Fc_FKP::operator ++( 0 );
    // !pPcd: Notbremse
    if ( !bComplex || !pPcd )
        return *this;

    if( GetPCDIdx() >= GetPCDIMax() )           // End of PLCF
    {
        nAttrStart = nAttrEnd = LONG_MAX;
        return *this;
    }

    long nFkpLen;                               // Fkp-Eintrag
    // Fkp-Eintrag holen
    WW8PLCFx_Fc_FKP::GetSprmsAndPos(nAttrStart, nAttrEnd, nFkpLen);

    pPcd->AktPieceFc2Cp( nAttrStart, nAttrEnd, &rSBase );
    bLineEnd = (ePLCF == PAP);
    return *this;
}

//-----------------------------------------
//-----------------------------------------

WW8PLCFx_SEPX::WW8PLCFx_SEPX(SvStream* pSt, SvStream* pTblSt,
    const WW8Fib& rFib, WW8_CP nStartCp)
    : WW8PLCFx(rFib.nVersion, true), maSprmParser(rFib.nVersion), pStrm(pSt),
    nArrMax(256), nSprmSiz(0)
{
    pPLCF =   rFib.lcbPlcfsed
            ? new WW8PLCF(pTblSt, rFib.fcPlcfsed, rFib.lcbPlcfsed, 12, nStartCp)
            : 0;

    pSprms = new BYTE[nArrMax];     // maximum length
}

WW8PLCFx_SEPX::~WW8PLCFx_SEPX()
{
    delete pPLCF;
    delete[] pSprms;
}

ULONG WW8PLCFx_SEPX::GetIdx() const
{
    return pPLCF ? pPLCF->GetIdx() : 0;
}

void WW8PLCFx_SEPX::SetIdx( ULONG nIdx )
{
    if( pPLCF ) pPLCF->SetIdx( nIdx );
}

bool WW8PLCFx_SEPX::SeekPos(WW8_CP nCpPos)
{
    return pPLCF ? pPLCF->SeekPos( nCpPos ) : 0;
}

WW8_CP WW8PLCFx_SEPX::Where()
{
    return pPLCF ? pPLCF->Where() : 0;
}

void WW8PLCFx_SEPX::GetSprms(WW8PLCFxDesc* p)
{
    if( !pPLCF ) return;

    void* pData;

    p->bRealLineEnd = false;
    if (!pPLCF->Get( p->nStartPos, p->nEndPos, pData ))
    {
        p->nStartPos = p->nEndPos = LONG_MAX;       // PLCF fertig abgearbeitet
        p->pMemPos = 0;
        p->nSprmsLen = 0;
    }
    else
    {
        long nPo =  SVBT32ToLong( (BYTE*)pData+2 );
        if (nPo == -1L)
        {
            p->nStartPos = p->nEndPos = LONG_MAX;       // Sepx empty
            p->pMemPos = 0;
            p->nSprmsLen = 0;
        }
        else
        {
            pStrm->Seek( nPo );
            *pStrm >> nSprmSiz; // read len

            if( nSprmSiz > nArrMax )
            {               // passt nicht
                delete[] pSprms;
                nArrMax = nSprmSiz;                 // Hole mehr Speicher
                pSprms = new BYTE[nArrMax];
            }
            pStrm->Read( pSprms, nSprmSiz );        // read Sprms

            p->nSprmsLen = nSprmSiz;
            p->pMemPos = pSprms;                    // return Position
        }
    }
}

WW8PLCFx& WW8PLCFx_SEPX::operator ++( int )
{
    if( pPLCF )
        (*pPLCF)++;
    return *this;
}

const BYTE* WW8PLCFx_SEPX::HasSprm( USHORT nId ) const
{
    return HasSprm( nId, pSprms, nSprmSiz);
}

const BYTE* WW8PLCFx_SEPX::HasSprm( USHORT nId, const BYTE*  pOtherSprms,
    long nOtherSprmSiz ) const
{
    const BYTE *pRet = 0;
    if (pPLCF)
    {
        WW8SprmIter aIter(pOtherSprms, nOtherSprmSiz, maSprmParser);
        pRet = aIter.FindSprm(nId);
    }
    return pRet;
}

bool WW8PLCFx_SEPX::Find4Sprms(USHORT nId1,USHORT nId2,USHORT nId3,USHORT nId4,
    BYTE*& p1, BYTE*& p2, BYTE*& p3, BYTE*& p4) const
{
    if( !pPLCF )
        return 0;

    bool bFound = false;
    p1 = 0;
    p2 = 0;
    p3 = 0;
    p4 = 0;

    BYTE* pSp = pSprms;
    USHORT i=0;
    while (i + maSprmParser.MinSprmLen() <= nSprmSiz)
    {
        // Sprm gefunden?
        USHORT nAktId = maSprmParser.GetSprmId(pSp);
        bool bOk = true;
        if( nAktId  == nId1 )
            p1 = pSp + maSprmParser.DistanceToData(nId1);
        else if( nAktId  == nId2 )
            p2 = pSp + maSprmParser.DistanceToData(nId2);
        else if( nAktId  == nId3 )
            p3 = pSp + maSprmParser.DistanceToData(nId3);
        else if( nAktId  == nId4 )
            p4 = pSp + maSprmParser.DistanceToData(nId4);
        else
            bOk = false;
        bFound |= bOk;
        // erhoehe Zeiger, so dass er auf naechsten Sprm zeigt
        USHORT x = maSprmParser.GetSprmSize(nAktId, pSp);
        i += x;
        pSp += x;
    }
    return bFound;
}

const BYTE* WW8PLCFx_SEPX::HasSprm( USHORT nId, BYTE n2nd ) const
{
    if( !pPLCF )
        return 0;

    BYTE* pSp = pSprms;

    USHORT i=0;
    while (i + maSprmParser.MinSprmLen() <= nSprmSiz)
    {
        // Sprm gefunden?
        USHORT nAktId = maSprmParser.GetSprmId(pSp);
        if (nAktId == nId)
        {
            BYTE *pRet = pSp + maSprmParser.DistanceToData(nId);
            if (*pRet == n2nd)
                return pRet;
        }
        // erhoehe Zeiger, so dass er auf naechsten Sprm zeigt
        USHORT x = maSprmParser.GetSprmSize(nAktId, pSp);
        i += x;
        pSp += x;
    }

    return 0;   // Sprm nicht gefunden
}

//-----------------------------------------
WW8PLCFx_SubDoc::WW8PLCFx_SubDoc(SvStream* pSt, BYTE nVersion,
    WW8_CP nStartCp, long nFcRef, long nLenRef, long nFcTxt, long nLenTxt,
    long nStruct)
    : WW8PLCFx(nVersion, true), pRef(0), pTxt(0)
{
    if( nLenRef && nLenTxt )
    {
        pRef = new WW8PLCF( pSt, nFcRef, nLenRef, nStruct, nStartCp );
        pTxt = new WW8PLCF( pSt, nFcTxt, nLenTxt, 0, nStartCp );
    }
}

WW8PLCFx_SubDoc::~WW8PLCFx_SubDoc()
{
    delete pRef;
    delete pTxt;
}

ULONG WW8PLCFx_SubDoc::GetIdx() const
{
    // Wahrscheinlich pTxt... nicht noetig
    if( pRef )
        return ( pRef->GetIdx() << 16 | pTxt->GetIdx() );
    return 0;
}

void WW8PLCFx_SubDoc::SetIdx( ULONG nIdx )
{
    if( pRef )
    {
        pRef->SetIdx( nIdx >> 16 );
        // Wahrscheinlich pTxt... nicht noetig
        pTxt->SetIdx( nIdx & 0xFFFF );
    }
}

bool WW8PLCFx_SubDoc::SeekPos( WW8_CP nCpPos )
{
    return ( pRef ) ? pRef->SeekPos( nCpPos ) : false;
}

WW8_CP WW8PLCFx_SubDoc::Where()
{
    return ( pRef ) ? pRef->Where() : LONG_MAX;
}

void WW8PLCFx_SubDoc::GetSprms(WW8PLCFxDesc* p)
{
    p->nStartPos = p->nEndPos = LONG_MAX;
    p->pMemPos = 0;
    p->nSprmsLen = 0;
    p->bRealLineEnd = false;

    if (!pRef)
        return;

    ULONG nNr = pRef->GetIdx();

    void *pData;
    long nFoo;
    if (!pRef->Get(p->nStartPos, nFoo, pData))
    {
        p->nEndPos = p->nStartPos = LONG_MAX;
        return;
    }

    p->nEndPos = p->nStartPos + 1;

    if (!pTxt)
        return;

    pTxt->SetIdx(nNr);

    if (!pTxt->Get(p->nCp2OrIdx, p->nSprmsLen, pData))
    {
        p->nEndPos = p->nStartPos = LONG_MAX;
        p->nSprmsLen = 0;
        return;
    }

    p->nSprmsLen -= p->nCp2OrIdx;
}

WW8PLCFx& WW8PLCFx_SubDoc::operator ++( int )
{
    if( pRef && pTxt )
    {
        (*pRef)++;
        (*pTxt)++;
    }
    return *this;
}

//-----------------------------------------
//          Felder
//-----------------------------------------

WW8PLCFx_FLD::WW8PLCFx_FLD( SvStream* pSt, const WW8Fib& rMyFib, short nType)
    : WW8PLCFx(rMyFib.nVersion, true), pPLCF(0), rFib(rMyFib)
{
    long nFc, nLen;

    switch( nType )
    {
    case MAN_HDFT:
        nFc = rFib.fcPlcffldHdr;
        nLen = rFib.lcbPlcffldHdr;
        break;
    case MAN_FTN:
        nFc = rFib.fcPlcffldFtn;
        nLen = rFib.lcbPlcffldFtn;
        break;
    case MAN_EDN:
        nFc = rFib.fcPlcffldEdn;
        nLen = rFib.lcbPlcffldEdn;
        break;
    case MAN_AND:
        nFc = rFib.fcPlcffldAtn;
        nLen = rFib.lcbPlcffldAtn;
        break;
    case MAN_TXBX:
        nFc = rFib.fcPlcffldTxbx;
        nLen = rFib.lcbPlcffldTxbx;
        break;
    case MAN_TXBX_HDFT:
        nFc = rFib.fcPlcffldHdrTxbx;
        nLen = rFib.lcbPlcffldHdrTxbx;
        break;
    default:
        nFc = rFib.fcPlcffldMom;
        nLen = rFib.lcbPlcffldMom;
        break;
    }

    if( nLen )
        pPLCF = new WW8PLCFspecial( pSt, nFc, nLen, 2 );
}

WW8PLCFx_FLD::~WW8PLCFx_FLD()
{
    delete pPLCF;
}

ULONG WW8PLCFx_FLD::GetIdx() const
{
    return pPLCF ? pPLCF->GetIdx() : 0;
}

void WW8PLCFx_FLD::SetIdx( ULONG nIdx )
{
    if( pPLCF )
        pPLCF->SetIdx( nIdx );
}

bool WW8PLCFx_FLD::SeekPos(WW8_CP nCpPos)
{
    return pPLCF ? pPLCF->SeekPosExact( nCpPos ) : false;
}

WW8_CP WW8PLCFx_FLD::Where()
{
    return pPLCF ? pPLCF->Where() : LONG_MAX;
}

bool WW8PLCFx_FLD::StartPosIsFieldStart()
{
    void* pData;
    long nTest;
    if (
         (!pPLCF || !pPLCF->Get(nTest, pData) ||
         ((((BYTE*)pData)[0] & 0x1f) != 0x13))
       )
        return false;
    return true;
}

bool WW8PLCFx_FLD::EndPosIsFieldEnd()
{
    bool bRet = false;

    if (pPLCF)
    {
        long n = pPLCF->GetIdx();

        (*pPLCF)++;

        void* pData;
        long nTest;
        if ( pPLCF->Get(nTest, pData) && ((((BYTE*)pData)[0] & 0x1f) == 0x15) )
            bRet = true;

        pPLCF->SetIdx(n);
    }

    return bRet;
}

void WW8PLCFx_FLD::GetSprms(WW8PLCFxDesc* p)
{
    p->nStartPos = p->nEndPos = LONG_MAX;
    p->pMemPos = 0;
    p->nSprmsLen = 0;
    p->bRealLineEnd = false;

    if (!pPLCF)
    {
        p->nStartPos = LONG_MAX;                    // Es gibt keine Felder
        return;
    }

    long n = pPLCF->GetIdx();

    long nP;
    void *pData;
    if (!pPLCF->Get(nP, pData))             // Ende des PLCFspecial ?
    {
        p->nStartPos = LONG_MAX;            // PLCF fertig abgearbeitet
        return;
    }

    p->nStartPos = nP;

    (*pPLCF)++;
    if (!pPLCF->Get(nP, pData))             // Ende des PLCFspecial ?
    {
        p->nStartPos = LONG_MAX;            // PLCF fertig abgearbeitet
        return;
    }

    p->nEndPos = nP;

    pPLCF->SetIdx(n);

    p->nCp2OrIdx = pPLCF->GetIdx();
}

WW8PLCFx& WW8PLCFx_FLD::operator ++( int )
{
    (*pPLCF)++;
    return *this;
}

bool WW8PLCFx_FLD::GetPara(long nIdx, WW8FieldDesc& rF)
{
    ASSERT( pPLCF, "Aufruf ohne Feld PLCFspecial" );
    if( !pPLCF )
        return false;

    long n = pPLCF->GetIdx();
    pPLCF->SetIdx(nIdx);

    bool bOk = WW8GetFieldPara(*pPLCF, rF);

    pPLCF->SetIdx(n);
    return bOk;
}

//-----------------------------------------
//      class WW8PLCF_Book
//-----------------------------------------

/*  to be optimized like this:    */
void WW8ReadSTTBF(bool bVer8, SvStream& rStrm, UINT32 nStart, INT32 nLen,
    USHORT nExtraLen, rtl_TextEncoding eCS, std::vector<String> &rArray,
    std::vector<String>* pExtraArray)
{
    ULONG nOldPos = rStrm.Tell();
    rStrm.Seek( nStart );

    UINT16 nLen2;
    rStrm >> nLen2; // bVer67: total length of structure
                    // bVer8 : count of strings

    if( bVer8 )
    {
        UINT16 nStrings;
        bool bUnicode = (0xFFFF == nLen2);
        if( bUnicode )
            rStrm >> nStrings;
        else
            nStrings = nLen2;

        rStrm >> nExtraLen;

        for( USHORT i=0; i < nStrings; i++ )
        {
            if( bUnicode )
                rArray.push_back(WW8Read_xstz(rStrm, 0, false));
            else
            {
                BYTE nBChar;
                rStrm >> nBChar;
                ByteString aTmp;
                SafeReadString(aTmp,nBChar,rStrm);
                rArray.push_back(String(aTmp, eCS));
            }

            // Skip the extra data
            if( nExtraLen )
            {
                if (pExtraArray)
                {
                    ByteString aTmp;
                    SafeReadString(aTmp,nExtraLen,rStrm);
                    pExtraArray->push_back(String(aTmp, eCS));
                }
                else
                    rStrm.SeekRel( nExtraLen );
            }
        }
    }
    else
    {
        BYTE nBChar;
        if( nLen2 != nLen )
        {
            ASSERT( nLen2 == nLen, "Fib length and read length are different" );
            if (nLen > USHRT_MAX)
                nLen = USHRT_MAX;
            else if (nLen < 2 )
                nLen = 2;
            nLen2 = static_cast<UINT16>(nLen);
        }
        ULONG nRead = 0;
        for( nLen2 -= 2; nRead < nLen2;  )
        {
            rStrm >> nBChar; ++nRead;
            if (nBChar)
            {
                ByteString aTmp;
                nRead += SafeReadString(aTmp,nBChar,rStrm);
                rArray.push_back(String(aTmp, eCS));
            }
            else
                rArray.push_back(aEmptyStr);

            // #89125# Skip the extra data (for bVer67 versions this must come
            // from external knowledge)
            if (nExtraLen)
            {
                if (pExtraArray)
                {
                    ByteString aTmp;
                    SafeReadString(aTmp,nExtraLen,rStrm);
                    pExtraArray->push_back(String(aTmp, eCS));
                }
                else
                    rStrm.SeekRel( nExtraLen );
                nRead+=nExtraLen;
            }
        }
    }
    rStrm.Seek( nOldPos );
}

WW8PLCFx_Book::WW8PLCFx_Book(SvStream* pTblSt, const WW8Fib& rFib)
    : WW8PLCFx(rFib.nVersion, false), pStatus(0), nIsEnd(0)
{
    if( !rFib.fcPlcfbkf || !rFib.lcbPlcfbkf || !rFib.fcPlcfbkl ||
        !rFib.lcbPlcfbkl || !rFib.fcSttbfbkmk || !rFib.lcbSttbfbkmk )
    {
        pBook[0] = pBook[1] = 0;
        nIMax = 0;
    }
    else
    {
        pBook[0] = new WW8PLCFspecial(pTblSt,rFib.fcPlcfbkf,rFib.lcbPlcfbkf,4);

        pBook[1] = new WW8PLCFspecial( pTblSt, rFib.fcPlcfbkl, rFib.lcbPlcfbkl,
            0, -1, true);

        rtl_TextEncoding eStructChrSet = WW8Fib::GetFIBCharset(rFib.chseTables);

        WW8ReadSTTBF( (7 < rFib.nVersion), *pTblSt, rFib.fcSttbfbkmk,
            rFib.lcbSttbfbkmk, 0, eStructChrSet, aBookNames );

        nIMax = aBookNames.size();

        if( pBook[0]->GetIMax() < nIMax )   // Count of Bookmarks
            nIMax = pBook[0]->GetIMax();
        if( pBook[1]->GetIMax() < nIMax )
            nIMax = pBook[1]->GetIMax();
        pStatus = new eBookStatus[ nIMax ];
        memset( pStatus, 0, nIMax * sizeof( eBookStatus ) );
    }
}

WW8PLCFx_Book::~WW8PLCFx_Book()
{
    delete[] pStatus;
    delete pBook[1];
    delete pBook[0];
}

ULONG WW8PLCFx_Book::GetIdx() const
{
    return nIMax ? pBook[0]->GetIdx() : 0;
}

void WW8PLCFx_Book::SetIdx( ULONG nI )
{
    if( nIMax )
        pBook[0]->SetIdx( nI );
}

ULONG WW8PLCFx_Book::GetIdx2() const
{
    return nIMax ? ( pBook[1]->GetIdx() | ( ( nIsEnd ) ? 0x80000000 : 0 ) ) : 0;
}

void WW8PLCFx_Book::SetIdx2( ULONG nI )
{
    if( nIMax )
    {
        pBook[1]->SetIdx( nI & 0x7fffffff );
        nIsEnd = (USHORT)( ( nI >> 31 ) & 1 );  // 0 oder 1
    }
}

bool WW8PLCFx_Book::SeekPos(WW8_CP nCpPos)
{
    if( !pBook[0] )
        return false;

    bool bOk = pBook[0]->SeekPosExact( nCpPos );
    bOk &= pBook[1]->SeekPosExact( nCpPos );
    nIsEnd = 0;

    return bOk;
}

WW8_CP WW8PLCFx_Book::Where()
{
    return pBook[nIsEnd]->Where();
}

long WW8PLCFx_Book::GetNoSprms( long& rStart, long& rEnd, long& rLen )
{
    void* pData;
    rEnd = LONG_MAX;
    rLen = 0;

    if (!pBook[0] || !pBook[1] || !nIMax || (pBook[nIsEnd]->GetIdx()) >= nIMax)
    {
        rStart = rEnd = LONG_MAX;
        return -1;
    }

    pBook[nIsEnd]->Get( rStart, pData );    // Pos. abfragen

    return pBook[nIsEnd]->GetIdx();
}

// Der Operator ++ hat eine Tuecke: Wenn 2 Bookmarks aneinandergrenzen, dann
// sollte erst das Ende des ersten und dann der Anfang des 2. erreicht werden.
// Liegen jedoch 2 Bookmarks der Laenge 0 aufeinander, *muss* von jedem Bookmark
// erst der Anfang und dann das Ende gefunden werden.
// Der Fall: ][
//            [...]
//           ][
// ist noch nicht geloest, dabei muesste ich in den Anfangs- und Endindices
// vor- und zurueckspringen, wobei ein weiterer Index oder ein Bitfeld
// oder etwas aehnliches zum Merken der bereits abgearbeiteten Bookmarks
// noetig wird.
WW8PLCFx& WW8PLCFx_Book::operator ++( int )
{
    if( pBook[0] && pBook[1] && nIMax )
    {
        (*pBook[nIsEnd])++;

        register ULONG l0 = pBook[0]->Where();
        register ULONG l1 = pBook[1]->Where();
        if( l0 < l1 )
            nIsEnd = 0;
        else if( l1 < l0 )
            nIsEnd = 1;
        else
            nIsEnd = ( nIsEnd ) ? 0 : 1;
    }
    return *this;
}

long WW8PLCFx_Book::GetLen() const
{
    if( nIsEnd )
    {
        ASSERT( !this, "Falscher Aufruf (1) von PLCF_Book::GetLen()" );
        return 0;
    }
    void * p;
    WW8_CP nStartPos;
    if( !pBook[0]->Get( nStartPos, p ) )
    {
        ASSERT( !this, "Falscher Aufruf (2) von PLCF_Book::GetLen()" );
        return 0;
    }
    USHORT nEndIdx = SVBT16ToShort( *((SVBT16*)p) );
    long nNum = pBook[1]->GetPos( nEndIdx );
    nNum -= nStartPos;
    return nNum;
}

void WW8PLCFx_Book::SetStatus(USHORT nIndex, eBookStatus eStat )
{
    ASSERT(nIndex < nIMax, "set status of non existing bookmark!");
    pStatus[nIndex] = (eBookStatus)( pStatus[nIndex] | eStat );
}

eBookStatus WW8PLCFx_Book::GetStatus() const
{
    if( !pStatus )
        return BOOK_NORMAL;
    long nEndIdx = GetHandle();
    return ( nEndIdx < nIMax ) ? pStatus[nEndIdx] : BOOK_NORMAL;
}

long WW8PLCFx_Book::GetHandle() const
{
    if( !pBook[0] || !pBook[1] )
        return LONG_MAX;

    if( nIsEnd )
        return pBook[1]->GetIdx();
    else
    {
        if (const void* p = pBook[0]->GetData(pBook[0]->GetIdx()))
            return SVBT16ToShort( *((SVBT16*)p) );
        else
            return LONG_MAX;
    }
}

String WW8PLCFx_Book::GetBookmark(long nStart,long nEnd, USHORT &nIndex)
{
    bool bFound = false;
    USHORT i = 0;
    if( pBook[0] && pBook[1] )
    {
        WW8_CP nStartAkt, nEndAkt;
        do
        {
            void* p;
            USHORT nEndIdx;

            if( pBook[0]->GetData( i, nStartAkt, p ) && p )
                nEndIdx = SVBT16ToShort( *((SVBT16*)p) );
            else
            {
                ASSERT( !this, "Bookmark-EndIdx nicht lesbar" );
                nEndIdx = i;
            }

            nEndAkt = pBook[1]->GetPos( nEndIdx );

            if ((nStartAkt >= nStart) && (nEndAkt <= nEnd))
            {
                nIndex = i;
                bFound=true;
                break;
            }
            ++i;
        }
        while (i < pBook[0]->GetIMax());
    }
    return bFound ? aBookNames[i] : aEmptyStr;
}

bool WW8PLCFx_Book::MapName(String& rName)
{
    if( !pBook[0] || !pBook[1] )
        return false;

    bool bFound = false;
    USHORT i = 0;
    WW8_CP nStartAkt, nEndAkt;
    do
    {
        void* p;
        USHORT nEndIdx;

        if( pBook[0]->GetData( i, nStartAkt, p ) && p )
            nEndIdx = SVBT16ToShort( *((SVBT16*)p) );
        else
        {
            ASSERT( !this, "Bookmark-EndIdx nicht lesbar" );
            nEndIdx = i;
        }
        nEndAkt = pBook[1]->GetPos( nEndIdx );
        if (COMPARE_EQUAL == rName.CompareIgnoreCaseToAscii(aBookNames[i]))
        {
            rName = aBookNames[i];
            bFound = true;
        }
        ++i;
    }
    while (!bFound && i < pBook[0]->GetIMax());
    return bFound;
}

const String* WW8PLCFx_Book::GetName() const
{
    const String *pRet = 0;
    if (!nIsEnd && (pBook[0]->GetIdx() < nIMax))
        pRet = &(aBookNames[pBook[0]->GetIdx()]);
    return pRet;
}

//-----------------------------------------
//          WW8PLCFMan
//-----------------------------------------

#ifndef DUMP

// Am Ende eines Absatzes reichen bei WW6 die Attribute bis hinter das <CR>.
// Das wird fuer die Verwendung mit dem SW um 1 Zeichen zurueckgesetzt, wenn
// dadurch kein AErger zu erwarten ist.
void WW8PLCFMan::AdjustEnds( WW8PLCFxDesc& rDesc )
{
    //Store old end position for supercool new property finder that uses
    //cp instead of fc's as nature intended
    rDesc.nOrigEndPos = rDesc.nEndPos;
    rDesc.nOrigStartPos = rDesc.nStartPos;

    /*
     Normally given ^XXX{para end}^ we don't actually insert a para end
     character into the document, so we clip the para end property one to the
     left to make the para properties end when the paragraph text does. In a
     drawing textbox we actually do insert a para end character, so we don't
     clip it. Making the para end properties end after the para end char.
    */
    if (GetDoingDrawTextBox())
        return;

    if ( (&rDesc == pPap) && rDesc.bRealLineEnd )
    {
        if ( pPap->nEndPos != LONG_MAX )    // Para adjust
        {
            nLineEnd = pPap->nEndPos;// nLineEnd zeigt *hinter* das <CR>
            pPap->nEndPos--;        // Absatzende um 1 Zeichen verkuerzen

            // gibt es bereits ein CharAttr-Ende das auf das jetzige
            // Absatzende zeigt ?  ... dann auch um 1 Zeichen verkuerzen
            if (pChp->nEndPos == nLineEnd)
                pChp->nEndPos--;

            // gibt es bereits ein Sep-Ende, das auf das jetzige Absatzende
            // zeigt ?  ... dann auch um 1 Zeichen verkuerzen
            if( pSep->nEndPos == nLineEnd )
                pSep->nEndPos--;
        }
    }
    else if ( (&rDesc == pChp) || (&rDesc == pSep) )
    {
        // Char Adjust oder Sep Adjust Wenn Ende Char-Attr == Absatzende ...
        if( (rDesc.nEndPos == nLineEnd) && (rDesc.nEndPos > rDesc.nStartPos) )
            rDesc.nEndPos--;            // ... dann um 1 Zeichen verkuerzen
    }
}

void WW8PLCFxDesc::ReduceByOffset()
{
    ASSERT((LONG_MAX == nStartPos) || (nStartPos <= nEndPos),
            "Attr-Anfang und -Ende ueber Kreuz" );

    if( nStartPos != LONG_MAX )
    {
        /*
        ##516##,##517##
        Force the property change to happen at the beginning of this
        subdocument, same as in GetNewNoSprms, except that the target type is
        attributes attached to a piece that might span subdocument boundaries
        */
        if (nCpOfs > nStartPos)
            nStartPos = 0;
        else
            nStartPos -= nCpOfs;
    }
    if( nEndPos   != LONG_MAX )
    {
        ASSERT(nCpOfs <= nEndPos,
            "oh oh, so much for the subdocument piece theory");
        nEndPos   -= nCpOfs;
    }
}

void WW8PLCFMan::GetNewSprms( WW8PLCFxDesc& rDesc )
{
    rDesc.pPLCFx->GetSprms(&rDesc);
    rDesc.ReduceByOffset();

    rDesc.bFirstSprm = true;
    AdjustEnds( rDesc );
    rDesc.nOrigSprmsLen = rDesc.nSprmsLen;
}

void WW8PLCFMan::GetNewNoSprms( WW8PLCFxDesc& rDesc )
{
    rDesc.nCp2OrIdx = rDesc.pPLCFx->GetNoSprms(rDesc.nStartPos, rDesc.nEndPos,
        rDesc.nSprmsLen);

    ASSERT((LONG_MAX == rDesc.nStartPos) || (rDesc.nStartPos <= rDesc.nEndPos),
            "Attr-Anfang und -Ende ueber Kreuz" );

    rDesc.ReduceByOffset();

    rDesc.bFirstSprm = true;
    rDesc.nOrigSprmsLen = rDesc.nSprmsLen;
}

USHORT WW8PLCFMan::GetId(const WW8PLCFxDesc* p) const
{
    USHORT nId;

    if (p == pFld)
        nId = eFLD;
    else if (p == pFtn)
        nId = eFTN;
    else if (p == pEdn)
        nId = eEDN;
    else if (p->nSprmsLen > 0)
        nId = maSprmParser.GetSprmId(p->pMemPos);
    else
        nId = 0;        // Id = 0 for empty attributes

    return nId;
}

WW8PLCFMan::WW8PLCFMan(WW8ScannerBase* pBase, short nType, long nStartCp,
    bool bDoingDrawTextBox)
    : maSprmParser(pBase->pWw8Fib->nVersion),
    mbDoingDrawTextBox(bDoingDrawTextBox)
{
    pWwFib = pBase->pWw8Fib;

    nLastWhereIdxCp = 0;
    memset( aD, 0, sizeof( aD ) );
    nLineEnd = LONG_MAX;
    nManType = nType;
    USHORT i;

    if( MAN_MAINTEXT == nType )
    {
        // Suchreihenfolge der Attribute
        nPLCF = MAN_ANZ_PLCF;
        pFld = &aD[0];
        pBkm = &aD[1];
        pEdn = &aD[2];
        pFtn = &aD[3];
        pAnd = &aD[4];

        pPcd = ( pBase->pPLCFx_PCD ) ? &aD[5] : 0;
        //pPcdA index == pPcd index + 1
        pPcdA = ( pBase->pPLCFx_PCDAttrs ) ? &aD[6] : 0;

        pChp = &aD[7];
        pPap = &aD[8];
        pSep = &aD[9];

        pSep->pPLCFx = pBase->pSepPLCF;
        pFtn->pPLCFx = pBase->pFtnPLCF;
        pEdn->pPLCFx = pBase->pEdnPLCF;
        pBkm->pPLCFx = pBase->pBook;
        pAnd->pPLCFx = pBase->pAndPLCF;

    }
    else
    {
        // Suchreihenfolge der Attribute
        nPLCF = 7;
        pFld = &aD[0];
        pBkm = ( pBase->pBook ) ? &aD[1] : 0;

        pPcd = ( pBase->pPLCFx_PCD ) ? &aD[2] : 0;
        //pPcdA index == pPcd index + 1
        pPcdA= ( pBase->pPLCFx_PCDAttrs ) ? &aD[3] : 0;

        pChp = &aD[4];
        pPap = &aD[5];
        pSep = &aD[6]; // Dummy

        pAnd = pFtn = pEdn = 0;     // unbenutzt bei SpezText
    }

    pChp->pPLCFx = pBase->pChpPLCF;
    pPap->pPLCFx = pBase->pPapPLCF;
    if( pPcd )
        pPcd->pPLCFx = pBase->pPLCFx_PCD;
    if( pPcdA )
        pPcdA->pPLCFx= pBase->pPLCFx_PCDAttrs;
    if( pBkm )
        pBkm->pPLCFx = pBase->pBook;

    pMagicTables = pBase->pMagicTables;

    switch( nType )                 // Feld-Initialisierung
    {
        case MAN_HDFT:
            pFld->pPLCFx = pBase->pFldHdFtPLCF;
            nCpO = pWwFib->ccpText + pWwFib->ccpFtn;
            pFdoa = pBase->pHdFtFdoa;
            pTxbx = pBase->pHdFtTxbx;
            pTxbxBkd = pBase->pHdFtTxbxBkd;
            break;
        case MAN_FTN:
            pFld->pPLCFx = pBase->pFldFtnPLCF;
            nCpO = pWwFib->ccpText;
            pFdoa = pTxbx = pTxbxBkd = 0;
            break;
        case MAN_EDN:
            pFld->pPLCFx = pBase->pFldEdnPLCF;
            nCpO = pWwFib->ccpText + pWwFib->ccpFtn + pWwFib->ccpHdr +
                pWwFib->ccpAtn;
            pFdoa = pTxbx = pTxbxBkd = 0;
            break;
        case MAN_AND:
            pFld->pPLCFx = pBase->pFldAndPLCF;
            nCpO = pWwFib->ccpText + pWwFib->ccpFtn + pWwFib->ccpHdr;
            pFdoa = pTxbx = pTxbxBkd = 0;
            break;
        case MAN_TXBX:
            pFld->pPLCFx = pBase->pFldTxbxPLCF;
            nCpO = pWwFib->ccpText + pWwFib->ccpFtn + pWwFib->ccpHdr +
                pWwFib->ccpMcr + pWwFib->ccpAtn + pWwFib->ccpEdn;
            pTxbx = pBase->pMainTxbx;
            pTxbxBkd = pBase->pMainTxbxBkd;
            pFdoa = 0;
            break;
        case MAN_TXBX_HDFT:
            pFld->pPLCFx = pBase->pFldTxbxHdFtPLCF;
            nCpO = pWwFib->ccpText + pWwFib->ccpFtn + pWwFib->ccpHdr +
                pWwFib->ccpMcr + pWwFib->ccpAtn + pWwFib->ccpEdn +
                pWwFib->ccpTxbx;
            pTxbx = pBase->pHdFtTxbx;
            pTxbxBkd = pBase->pHdFtTxbxBkd;
            pFdoa = 0;
            break;
        default:
            pFld->pPLCFx = pBase->pFldPLCF;
            nCpO = 0;
            pFdoa = pBase->pMainFdoa;
            pTxbx = pBase->pMainTxbx;
            pTxbxBkd = pBase->pMainTxbxBkd;
            break;
    }

    if( nStartCp || nCpO )
        SeekPos( nStartCp );    // PLCFe auf Text-StartPos einstellen

    // initialisieren der Member-Vars Low-Level
    GetChpPLCF()->ResetAttrStartEnd();
    GetPapPLCF()->ResetAttrStartEnd();
    for( i=0; i < nPLCF; i++)
    {
        register WW8PLCFxDesc* p = &aD[i];

        /*
        ##516##,##517##
        For subdocuments we modify the cp of properties to be relative to
        the beginning of subdocuments, we should also do the same for
        piecetable changes, and piecetable properties, otherwise a piece
        change that happens in a subdocument is lost.
        */
        p->nCpOfs = ( p == pChp || p == pPap || p == pBkm || p == pPcd ||
            p == pPcdA ) ? nCpO : 0;

        p->nCp2OrIdx = 0;
        p->bFirstSprm = false;
        p->pIdStk = 0;

        if ((p == pChp) || (p == pPap))
            p->nStartPos = p->nEndPos = nStartCp;
        else
            p->nStartPos = p->nEndPos = LONG_MAX;
    }

    // initialisieren der Member-Vars High-Level
    for( i=0; i<nPLCF; i++){
        register WW8PLCFxDesc* p = &aD[i];

        if( !p->pPLCFx )
        {
            p->nStartPos = p->nEndPos = LONG_MAX;
            continue;
        }

        if( p->pPLCFx->IsSprm() )
        {
            // Vorsicht: nEndPos muss bereits
            p->pIdStk = new std::stack<USHORT>;
            if ((p == pChp) || (p == pPap))
            {
                WW8_CP nTemp = p->nEndPos+p->nCpOfs;
                p->pMemPos = 0;
                p->nSprmsLen = 0;
                p->nStartPos = nTemp;
                if (!(*p->pPLCFx).SeekPos(p->nStartPos))
                    p->nEndPos = p->nStartPos = LONG_MAX;
                else
                    GetNewSprms( *p );
            }
            else
                GetNewSprms( *p );      // bei allen PLCFen initialisiert sein
        }
        else if( p->pPLCFx )
            GetNewNoSprms( *p );
    }
}

WW8PLCFMan::~WW8PLCFMan()
{
    for( USHORT i=0; i<nPLCF; i++)
        delete aD[i].pIdStk;
}

// 0. welche Attr.-Klasse,
// 1. ob ein Attr.-Start ist,
// 2. CP, wo ist naechste Attr.-Aenderung
USHORT WW8PLCFMan::WhereIdx(bool* pbStart, long* pPos) const
{
    ASSERT(nPLCF,"What the hell");
    long nNext = LONG_MAX;  // SuchReihenfolge:
    USHORT nNextIdx = nPLCF;// first ending found ( CHP, PAP, ( SEP ) ),
    bool bStart = true;     // dann Anfaenge finden ( ( SEP ), PAP, CHP )
    USHORT i;
    register const WW8PLCFxDesc* pD;
    for (i=0; i < nPLCF; i++)
    {
        pD = &aD[i];
        if (pD != pPcdA)
        {
            if( (pD->nEndPos < nNext) && (pD->nStartPos == LONG_MAX) )
            {
                // sonst ist Anfang = Ende
                nNext = pD->nEndPos;
                nNextIdx = i;
                bStart = false;
            }
        }
    }
    for (i=nPLCF; i > 0; i--)
    {
        pD = &aD[i-1];
        if (pD != pPcdA)
        {
            if( pD->nStartPos < nNext )
            {
                nNext = pD->nStartPos;
                nNextIdx = i-1;
                bStart = true;
            }
        }
    }
    if( pPos )
        *pPos = nNext;
    if( pbStart )
        *pbStart = bStart;
    return nNextIdx;
}

// gibt die CP-Pos der naechsten Attribut-Aenderung zurueck
WW8_CP WW8PLCFMan::Where() const
{
    long l;
    WhereIdx(0, &l);
    return l;
}

void WW8PLCFMan::SeekPos( long nNewCp )
{
    pChp->pPLCFx->SeekPos( nNewCp + nCpO ); // Attribute neu
    pPap->pPLCFx->SeekPos( nNewCp + nCpO ); // aufsetzen
    pFld->pPLCFx->SeekPos( nNewCp );
    if( pPcd )
        pPcd->pPLCFx->SeekPos( nNewCp + nCpO );
    if( pBkm )
        pBkm->pPLCFx->SeekPos( nNewCp + nCpO );
}

void WW8PLCFMan::SaveAllPLCFx( WW8PLCFxSaveAll& rSave ) const
{
    USHORT i, n=0;
    if( pPcd )
        pPcd->Save(  rSave.aS[n++] );
    if( pPcdA )
        pPcdA->Save( rSave.aS[n++] );

    for(i=0; i<nPLCF; ++i)
        if( pPcd != &aD[i] && pPcdA != &aD[i] )
            aD[i].Save( rSave.aS[n++] );
}

void WW8PLCFMan::RestoreAllPLCFx( const WW8PLCFxSaveAll& rSave )
{
    USHORT i, n=0;
    if( pPcd )
        pPcd->Restore(  rSave.aS[n++] );
    if( pPcdA )
        pPcdA->Restore( rSave.aS[n++] );

    for(i=0; i<nPLCF; ++i)
        if( pPcd != &aD[i] && pPcdA != &aD[i] )
            aD[i].Restore( rSave.aS[n++] );
}

void WW8PLCFMan::GetSprmStart( short nIdx, WW8PLCFManResult* pRes ) const
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );

    // Pruefen !!!

    pRes->nMemLen = 0;

    register const WW8PLCFxDesc* p = &aD[nIdx];

    // first Sprm in a Group
    if( p->bFirstSprm )
    {
        if( p == pPap )
            pRes->nFlags |= MAN_MASK_NEW_PAP;
        else if( p == pSep )
            pRes->nFlags |= MAN_MASK_NEW_SEP;
    }
    pRes->pMemPos = p->pMemPos;
    pRes->nSprmId = GetId(p);
    pRes->nCp2OrIdx = p->nCp2OrIdx;
    if ((p == pFtn) || (p == pEdn))
        pRes->nMemLen = p->nSprmsLen;
    else if (p->nSprmsLen)  //Normal
    {
        // Length of actual sprm
        pRes->nMemLen = maSprmParser.GetSprmSize(pRes->nSprmId, pRes->pMemPos);
    }
}

void WW8PLCFMan::GetSprmEnd( short nIdx, WW8PLCFManResult* pRes ) const
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );

    register const WW8PLCFxDesc* p = &aD[nIdx];

    if (!(p->pIdStk->empty()))
        pRes->nSprmId = p->pIdStk->top();       // get end position
    else
    {
        ASSERT( !this, "No Id on the Stack" );
        pRes->nSprmId = 0;
    }
}

void WW8PLCFMan::GetNoSprmStart( short nIdx, WW8PLCFManResult* pRes ) const
{
    register const WW8PLCFxDesc* p = &aD[nIdx];

    pRes->nCpPos = p->nStartPos;
    pRes->nMemLen = p->nSprmsLen;
    pRes->nCp2OrIdx = p->nCp2OrIdx;

    if( p == pFld )
        pRes->nSprmId = eFLD;
    else if( p == pFtn )
        pRes->nSprmId = eFTN;
    else if( p == pEdn )
        pRes->nSprmId = eEDN;
    else if( p == pBkm )
        pRes->nSprmId = eBKN;
    else if( p == pAnd )
        pRes->nSprmId = eAND;
    else if( p == pPcd )
    {
        //We slave the piece table attributes to the piece table, the piece
        //table attribute iterator contains the sprms for this piece.
        GetSprmStart( nIdx+1, pRes );
    }
    else
        pRes->nSprmId = 0;          // default: not found
}

void WW8PLCFMan::GetNoSprmEnd( short nIdx, WW8PLCFManResult* pRes ) const
{
    pRes->nMemLen = -1;     // Ende-Kennzeichen

    if( &aD[nIdx] == pBkm )
        pRes->nSprmId = eBKN;
    else if( &aD[nIdx] == pPcd )
    {
        //We slave the piece table attributes to the piece table, the piece
        //table attribute iterator contains the sprms for this piece.
        GetSprmEnd( nIdx+1, pRes );
    }
    else
        pRes->nSprmId = 0;
}

bool WW8PLCFMan::TransferOpenSprms(std::stack<USHORT> &rStack)
{
    for (int i = 0; i < nPLCF; ++i)
    {
        WW8PLCFxDesc* p = &aD[i];
        if (!p || !p->pIdStk)
            continue;
        while (!p->pIdStk->empty())
        {
            rStack.push(p->pIdStk->top());
            p->pIdStk->pop();
        }
    }
    return rStack.empty();
}

void WW8PLCFMan::AdvSprm(short nIdx, bool bStart)
{
    WW8PLCFxDesc* p = &aD[nIdx];    // Sprm-Klasse(!) ermitteln

    p->bFirstSprm = false;
    if( bStart )
    {
        USHORT nLastId = GetId(p);
        p->pIdStk->push(nLastId);   // merke Id fuer Attribut-Ende

        if( p->nSprmsLen )
        {   /*
                Pruefe, ob noch Sprm(s) abzuarbeiten sind
            */
            if( p->pMemPos )
            {
                // Length of last sprm
                USHORT nSprmL = maSprmParser.GetSprmSize(nLastId, p->pMemPos);

                // Gesamtlaenge Sprms um SprmLaenge verringern
                p->nSprmsLen -= nSprmL;

                // Pos des evtl. naechsten Sprm
                if (p->nSprmsLen < maSprmParser.MinSprmLen())
                {
                    // sicherheitshalber auf Null setzen, da Enden folgen!
                    p->pMemPos = 0;
                    p->nSprmsLen = 0;
                }
                else
                    p->pMemPos += nSprmL;
            }
            else
                p->nSprmsLen = 0;
        }
        if (p->nSprmsLen < maSprmParser.MinSprmLen())
            p->nStartPos = LONG_MAX;    // es folgen Enden
    }
    else
    {
        if (!(p->pIdStk->empty()))
            p->pIdStk->pop();
        if (p->pIdStk->empty())
        {
            if ( (p == pChp) || (p == pPap) )
            {
                p->pMemPos = 0;
                p->nSprmsLen = 0;
                p->nStartPos = p->nOrigEndPos+p->nCpOfs;

                /*
                #93702#
                On failed seek we have run out of sprms, probably.  But if its
                a fastsaved file (has pPcd) then we may be just in a sprm free
                gap between pieces that have them, so set dirty flag in sprm
                finder to consider than.
                */
                if (!(*p->pPLCFx).SeekPos(p->nStartPos))
                {
                    p->nEndPos = LONG_MAX;
                    p->pPLCFx->SetDirty(true);
                }
                if (!p->pPLCFx->GetDirty() || pPcd)
                    GetNewSprms( *p );
                p->pPLCFx->SetDirty(false);

                /*
                #i2325#
                To get the character and paragraph properties you first get
                the pap and chp and then apply the fastsaved pPcd properties
                to the range. If a pap or chp starts inside the pPcd range
                then we must bring the current pPcd range to a halt so as to
                end those sprms, then the pap/chp will be processed, and then
                we must force a restart of the pPcd on that pap/chp starting
                boundary. Doing that effectively means that the pPcd sprms will
                be applied to the new range. Not doing it means that the pPcd
                sprms will only be applied to the first pap/chp set of
                properties contained in the pap/chp range.

                So we bring the pPcd to a halt on this location here, by
                settings its end to the current start, then store the starting
                position of the current range to clipstart. The pPcd sprms
                will end as normal (albeit earlier than originally expected),
                and the existance of a clipstart will force the pPcd iterater
                to reread the current set of sprms instead of advancing to its
                next set. Then the clipstart will be set as the starting
                position which will force them to be applied directly after
                the pap and chps.
                */
                if (pPcd && ((p->nStartPos > pPcd->nStartPos) ||
                    (pPcd->nStartPos == LONG_MAX)) &&
                    (pPcd->nEndPos != p->nStartPos))
                {
                    pPcd->nEndPos = p->nStartPos;
                    ((WW8PLCFx_PCD *)(pPcd->pPLCFx))->SetClipStart(
                        p->nStartPos);
                }

            }
            else
            {
                (*p->pPLCFx)++;     // next Group of Sprms
                p->pMemPos = 0;     // !!!
                p->nSprmsLen = 0;
                GetNewSprms( *p );
            }
            ASSERT( p->nStartPos <= p->nEndPos, "Attribut ueber Kreuz" );
        }
    }
}

void WW8PLCFMan::AdvNoSprm(short nIdx, bool bStart)
{
    /*
    For the case of a piece table we slave the piece table attribute iterator
    to the piece table and access it through that only. They are two seperate
    structures, but act together as one logical one. The attributes only go
    to the next entry when the piece changes
    */
    register WW8PLCFxDesc* p = &aD[nIdx];

    if( p == pPcd )
    {
        AdvSprm(nIdx+1,bStart);
        if( bStart )
            p->nStartPos = aD[nIdx+1].nStartPos;
        else
        {
            if (aD[nIdx+1].pIdStk->empty())
            {
                WW8PLCFx_PCD *pTemp = (WW8PLCFx_PCD*)(pPcd->pPLCFx);
                /*
                #i2325#
                As per normal, go on to the next set of properties, i.e. we
                have traversed over to the next piece.  With a clipstart set
                we are being told to reread the current piece sprms so as to
                reapply them to a new chp or pap range.
                */
                if (pTemp->GetClipStart() == -1)
                    (*p->pPLCFx)++;
                p->pMemPos = 0;
                p->nSprmsLen = 0;
                GetNewSprms( aD[nIdx+1] );
                GetNewNoSprms( *p );
                if (pTemp->GetClipStart() != -1)
                {
                    /*
                    #i2325#, now we will force our starting position to the
                    clipping start so as to force the application of these
                    sprms after the current pap/chp sprms so as to apply the
                    fastsave sprms to the current range.
                    */
                    p->nStartPos = pTemp->GetClipStart();
                    pTemp->SetClipStart(-1);
                }
            }
        }
    }
    else
    {                                  // NoSprm ohne Ende
        (*p->pPLCFx)++;
        p->pMemPos = 0;                     // MemPos ungueltig
        p->nSprmsLen = 0;
        GetNewNoSprms( *p );
    }
}

WW8PLCFMan& WW8PLCFMan::operator ++(int)
{
    bool bStart;
    USHORT nIdx = WhereIdx(&bStart);
    if (nIdx < nPLCF)
    {
        register WW8PLCFxDesc* p = &aD[nIdx];

        p->bFirstSprm = true;                       // Default

        if( p->pPLCFx->IsSprm() )
            AdvSprm( nIdx, bStart );
        else                                        // NoSprm
            AdvNoSprm( nIdx, bStart );
    }
    return *this;
}

// Rueckgabe true fuer Anfang eines Attributes oder Fehler,
//           false fuer Ende d. Attr
// Restliche Rueckgabewerte werden in der vom Aufrufer zu stellenden Struktur
// WW8PclxManResults geliefert.
bool WW8PLCFMan::Get(WW8PLCFManResult* pRes) const
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );
    bool bStart;
    USHORT nIdx = WhereIdx(&bStart);

    if( nIdx >= nPLCF )
    {
        ASSERT( !this, "Position not found" );
        return true;
    }

    if( aD[nIdx].pPLCFx->IsSprm() )
    {
        if( bStart )
        {
            GetSprmStart( nIdx, pRes );
            return true;
        }
        else
        {
            GetSprmEnd( nIdx, pRes );
            return false;
        }
    }
    else
    {
        if( bStart )
        {
            GetNoSprmStart( nIdx, pRes );
            return true;
        }
        else
        {
            GetNoSprmEnd( nIdx, pRes );
            return false;
        }
    }
}

USHORT WW8PLCFMan::GetColl() const
{
    if( pPap->pPLCFx )
        return  pPap->pPLCFx->GetIstd();
    else
    {
        ASSERT( !this, "GetColl ohne PLCF_Pap" );
        return 0;
    }
}

WW8PLCFx_FLD* WW8PLCFMan::GetFld() const
{
    return (WW8PLCFx_FLD*)pFld->pPLCFx;
}

const BYTE* WW8PLCFMan::HasParaSprm( USHORT nId ) const
{
    return ((WW8PLCFx_Cp_FKP*)pPap->pPLCFx)->HasSprm( nId );
}

const BYTE* WW8PLCFMan::HasCharSprm( USHORT nId ) const
{
    return ((WW8PLCFx_Cp_FKP*)pChp->pPLCFx)->HasSprm( nId );
}

bool WW8PLCFMan::HasCharSprm(USHORT nId,
    std::vector<const BYTE *> &rResult) const
{
    return ((WW8PLCFx_Cp_FKP*)pChp->pPLCFx)->HasSprm(nId, rResult);
}

#endif // !DUMP

void WW8PLCFx::Save( WW8PLCFxSave1& rSave ) const
{
     rSave.nPLCFxPos    = GetIdx();
    rSave.nPLCFxPos2   = GetIdx2();
    rSave.nPLCFxMemOfs = 0;
    rSave.nStartFC     = GetStartFc();
}

void WW8PLCFx::Restore( const WW8PLCFxSave1& rSave )
{
    SetIdx(     rSave.nPLCFxPos  );
    SetIdx2(    rSave.nPLCFxPos2 );
    SetStartFc( rSave.nStartFC   );
}

ULONG WW8PLCFx_Cp_FKP::GetIdx2() const
{
    return GetPCDIdx();
}

void WW8PLCFx_Cp_FKP::SetIdx2( ULONG nIdx )
{
    SetPCDIdx( nIdx );
}

void WW8PLCFx_Cp_FKP::Save( WW8PLCFxSave1& rSave ) const
{
    WW8PLCFx::Save( rSave );

    rSave.nAttrStart = nAttrStart;
    rSave.nAttrEnd   = nAttrEnd;
    rSave.bLineEnd   = bLineEnd;
}

void WW8PLCFx_Cp_FKP::Restore( const WW8PLCFxSave1& rSave )
{
    WW8PLCFx::Restore( rSave );

    nAttrStart = rSave.nAttrStart;
    nAttrEnd   = rSave.nAttrEnd;
    bLineEnd   = rSave.bLineEnd;
}

void WW8PLCFxDesc::Save( WW8PLCFxSave1& rSave ) const
{
    if( pPLCFx )
    {
        pPLCFx->Save( rSave );
        if( pPLCFx->IsSprm() )
        {
            WW8PLCFxDesc aD;
            aD.nStartPos = nOrigStartPos+nCpOfs;
            aD.nCpOfs = rSave.nCpOfs = nCpOfs;
            if (!(pPLCFx->SeekPos(aD.nStartPos)))
            {
                aD.nEndPos = LONG_MAX;
                pPLCFx->SetDirty(true);
            }
            pPLCFx->GetSprms(&aD);
            pPLCFx->SetDirty(false);
            aD.ReduceByOffset();
            rSave.nStartCp = aD.nStartPos;
            rSave.nPLCFxMemOfs = nOrigSprmsLen - nSprmsLen;
        }
    }
}

void WW8PLCFxDesc::Restore( const WW8PLCFxSave1& rSave )
{
    if( pPLCFx )
    {
        pPLCFx->Restore( rSave );
        if( pPLCFx->IsSprm() )
        {
            WW8PLCFxDesc aD;
            aD.nStartPos = rSave.nStartCp+rSave.nCpOfs;
            nCpOfs = aD.nCpOfs = rSave.nCpOfs;
            if (!(pPLCFx->SeekPos(aD.nStartPos)))
            {
                aD.nEndPos = LONG_MAX;
                pPLCFx->SetDirty(true);
            }
            pPLCFx->GetSprms(&aD);
            pPLCFx->SetDirty(false);
            aD.ReduceByOffset();
            pMemPos = aD.pMemPos + rSave.nPLCFxMemOfs;
        }
    }
}

//-----------------------------------------


WW8Fib::WW8Fib( SvStream& rSt, BYTE nWantedVersion,UINT32 nOffset )
    : nFibError( 0 )
{
    BYTE aBits1;
    BYTE aBits2;
    BYTE aVer8Bits1;    // nur ab WinWord 8 benutzt
    rSt.Seek( nOffset );
    /*
        Wunsch-Nr vermerken, File-Versionsnummer ermitteln
        und gegen Wunsch-Nr. checken !
    */
    nVersion = nWantedVersion;
    rSt >> wIdent;
    rSt >> nFib;
    rSt >> nProduct;
    if( 0 != rSt.GetError() )
    {
        INT16 nFibMin;
        INT16 nFibMax;
        // note: 6 stands for "6 OR 7",  7 stands for "ONLY 7"
        switch( nVersion )
        {
            case 6:
                nFibMin = 0x0065;   // von 101 WinWord 6.0
                                    //     102    "
                                    // und 103 WinWord 6.0 fuer Macintosh
                                    //     104    "
                nFibMax = 0x0069;   // bis 105 WinWord 95
                break;
            case 7:
                nFibMin = 0x0069;   // von 105 WinWord 95
                nFibMax = 0x0069;   // bis 105 WinWord 95
                break;
            case 8:
                nFibMin = 0x006A;   // von 106 WinWord 97
                nFibMax = 0x00c1;   // bis 193 WinWord 97 (?)
                break;
            default:
                nFibMin = 0;            // Programm-Fehler!
                nFibMax = 0;
                nFib    = 1;
                ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
                break;
        }
        if ( (nFib < nFibMin) || (nFib > nFibMax) )
        {
            nFibError = ERR_SWG_READ_ERROR; // Error melden
            return;                         // und hopp raus!
        }
    }

    // praktische Hilfsvariablen:
    bool bVer67 = ((6 == nVersion) || (7 == nVersion));
    bool bVer8  =  (8 == nVersion);

    // Hilfs-Varis fuer Ver67:
    INT16 pnChpFirst_Ver67=0;
    INT16 pnPapFirst_Ver67=0;
    INT16 cpnBteChp_Ver67=0;
    INT16 cpnBtePap_Ver67=0;

    // und auf gehts: FIB einlesen
    rSt >> lid;
    rSt >> pnNext;
    rSt >> aBits1;
    rSt >> aBits2;
    rSt >> nFibBack;
    rSt >> lKey1;
    rSt >> lKey2;
    rSt >> envr;
    rSt >> aVer8Bits1;      // unter Ver67  nur leeres Reservefeld
                            // Inhalt von aVer8Bits1
                            //
                            // BYTE fMac              :1;
                            // BYTE fEmptySpecial     :1;
                            // BYTE fLoadOverridePage :1;
                            // BYTE fFuturesavedUndo  :1;
                            // BYTE fWord97Saved      :1;
                            // BYTE :3;
    rSt >> chse;
    rSt >> chseTables;
    rSt >> fcMin;
    rSt >> fcMac;

// Einschub fuer WW8 *****************************************************
    if( !bVer67 )
    {
        rSt >> csw;

        // Marke: "rgsw"  Beginning of the array of shorts
        rSt >> wMagicCreated;
        rSt >> wMagicRevised;
        rSt >> wMagicCreatedPrivate;
        rSt >> wMagicRevisedPrivate;
        rSt.SeekRel( 9 * sizeof( INT16 ) );

        /*
        // dies sind die 9 unused Felder:
        && (bVer67 || WW8ReadINT16(  rSt, pnFbpChpFirst_W6          ))  // 1
        && (bVer67 || WW8ReadINT16(  rSt, pnChpFirst_W6                 ))  // 2
        && (bVer67 || WW8ReadINT16(  rSt, cpnBteChp_W6                  ))  // 3
        && (bVer67 || WW8ReadINT16(  rSt, pnFbpPapFirst_W6          ))  // 4
        && (bVer67 || WW8ReadINT16(  rSt, pnPapFirst_W6                 ))  // 5
        && (bVer67 || WW8ReadINT16(  rSt, cpnBtePap_W6                  ))  // 6
        && (bVer67 || WW8ReadINT16(  rSt, pnFbpLvcFirst_W6          ))  // 7
        && (bVer67 || WW8ReadINT16(  rSt, pnLvcFirst_W6                 ))  // 8
        && (bVer67 || WW8ReadINT16(  rSt, cpnBteLvc_W6                  ))  // 9
        */
        rSt >> lidFE;
        rSt >> clw;
    }

// Ende des Einschubs fuer WW8 *******************************************

        // Marke: "rglw"  Beginning of the array of longs
    rSt >> cbMac;

        // 2 Longs uebergehen, da unwichtiger Quatsch
    rSt.SeekRel( 2 * sizeof( INT32) );

        // weitere 2 Longs nur bei Ver67 ueberspringen
    if( bVer67 )
        rSt.SeekRel( 2 * sizeof( INT32) );

    rSt >> ccpText;
    rSt >> ccpFtn;
    rSt >> ccpHdr;
    rSt >> ccpMcr;
    rSt >> ccpAtn;
    rSt >> ccpEdn;
    rSt >> ccpTxbx;
    rSt >> ccpHdrTxbx;

        // weiteres Long nur bei Ver67 ueberspringen
    if( bVer67 )
        rSt.SeekRel( 1 * sizeof( INT32) );
    else
    {
// Einschub fuer WW8 *****************************************************
        rSt >> pnFbpChpFirst;
        rSt >> pnChpFirst;
        rSt >> cpnBteChp;
        rSt >> pnFbpPapFirst;
        rSt >> pnPapFirst;
        rSt >> cpnBtePap;
        rSt >> pnFbpLvcFirst;
        rSt >> pnLvcFirst;
        rSt >> cpnBteLvc;
        rSt >> fcIslandFirst;
        rSt >> fcIslandLim;
        rSt >> cfclcb;
    }

// Ende des Einschubs fuer WW8 *******************************************

        // Marke: "rgfclcb" Beginning of array of FC/LCB pairs.
    rSt >> fcStshfOrig;
    rSt >> lcbStshfOrig;
    rSt >> fcStshf;
    rSt >> lcbStshf;
    rSt >> fcPlcffndRef;
    rSt >> lcbPlcffndRef;
    rSt >> fcPlcffndTxt;
    rSt >> lcbPlcffndTxt;
    rSt >> fcPlcfandRef;
    rSt >> lcbPlcfandRef;
    rSt >> fcPlcfandTxt;
    rSt >> lcbPlcfandTxt;
    rSt >> fcPlcfsed;
    rSt >> lcbPlcfsed;
    rSt >> fcPlcfpad;
    rSt >> lcbPlcfpad;
    rSt >> fcPlcfphe;
    rSt >> lcbPlcfphe;
    rSt >> fcSttbfglsy;
    rSt >> lcbSttbfglsy;
    rSt >> fcPlcfglsy;
    rSt >> lcbPlcfglsy;
    rSt >> fcPlcfhdd;
    rSt >> lcbPlcfhdd;
    rSt >> fcPlcfbteChpx;
    rSt >> lcbPlcfbteChpx;
    rSt >> fcPlcfbtePapx;
    rSt >> lcbPlcfbtePapx;
    rSt >> fcPlcfsea;
    rSt >> lcbPlcfsea;
    rSt >> fcSttbfffn;
    rSt >> lcbSttbfffn;
    rSt >> fcPlcffldMom;
    rSt >> lcbPlcffldMom;
    rSt >> fcPlcffldHdr;
    rSt >> lcbPlcffldHdr;
    rSt >> fcPlcffldFtn;
    rSt >> lcbPlcffldFtn;
    rSt >> fcPlcffldAtn;
    rSt >> lcbPlcffldAtn;
    rSt >> fcPlcffldMcr;
    rSt >> lcbPlcffldMcr;
    rSt >> fcSttbfbkmk;
    rSt >> lcbSttbfbkmk;
    rSt >> fcPlcfbkf;
    rSt >> lcbPlcfbkf;
    rSt >> fcPlcfbkl;
    rSt >> lcbPlcfbkl;
    rSt >> fcCmds;
    rSt >> lcbCmds;
    rSt >> fcPlcfmcr;
    rSt >> lcbPlcfmcr;
    rSt >> fcSttbfmcr;
    rSt >> lcbSttbfmcr;
    rSt >> fcPrDrvr;
    rSt >> lcbPrDrvr;
    rSt >> fcPrEnvPort;
    rSt >> lcbPrEnvPort;
    rSt >> fcPrEnvLand;
    rSt >> lcbPrEnvLand;
    rSt >> fcWss;
    rSt >> lcbWss;
    rSt >> fcDop;
    rSt >> lcbDop;
    rSt >> fcSttbfAssoc;
    rSt >> cbSttbfAssoc;
    rSt >> fcClx;
    rSt >> lcbClx;
    rSt >> fcPlcfpgdFtn;
    rSt >> lcbPlcfpgdFtn;
    rSt >> fcAutosaveSource;
    rSt >> lcbAutosaveSource;
    rSt >> fcGrpStAtnOwners;
    rSt >> lcbGrpStAtnOwners;
    rSt >> fcSttbfAtnbkmk;
    rSt >> lcbSttbfAtnbkmk;

        // weiteres short nur bei Ver67 ueberspringen
    if (bVer67)
    {
        rSt.SeekRel( 1*sizeof( INT16) );

        // folgende 4 Shorts existieren nur bei Ver67;
        rSt >> pnChpFirst_Ver67;
        rSt >> pnPapFirst_Ver67;
        rSt >> cpnBteChp_Ver67;
        rSt >> cpnBtePap_Ver67;
    }

    rSt >> fcPlcfdoaMom;
    rSt >> lcbPlcfdoaMom;
    rSt >> fcPlcfdoaHdr;
    rSt >> lcbPlcfdoaHdr;
    rSt >> fcPlcfspaMom;
    rSt >> lcbPlcfspaMom;
    rSt >> fcPlcfspaHdr;
    rSt >> lcbPlcfspaHdr;

    rSt >> fcPlcfAtnbkf;
    rSt >> lcbPlcfAtnbkf;
    rSt >> fcPlcfAtnbkl;
    rSt >> lcbPlcfAtnbkl;
    rSt >> fcPms;
    rSt >> lcbPMS;
    rSt >> fcFormFldSttbf;
    rSt >> lcbFormFldSttbf;
    rSt >> fcPlcfendRef;
    rSt >> lcbPlcfendRef;
    rSt >> fcPlcfendTxt;
    rSt >> lcbPlcfendTxt;
    rSt >> fcPlcffldEdn;
    rSt >> lcbPlcffldEdn;
    rSt >> fcPlcfpgdEdn;
    rSt >> lcbPlcfpgdEdn;
    rSt >> fcDggInfo;
    rSt >> lcbDggInfo;
    rSt >> fcSttbfRMark;
    rSt >> lcbSttbfRMark;
    rSt >> fcSttbfCaption;
    rSt >> lcbSttbfCaption;
    rSt >> fcSttbAutoCaption;
    rSt >> lcbSttbAutoCaption;
    rSt >> fcPlcfwkb;
    rSt >> lcbPlcfwkb;
    rSt >> fcPlcfspl;
    rSt >> lcbPlcfspl;
    rSt >> fcPlcftxbxTxt;
    rSt >> lcbPlcftxbxTxt;
    rSt >> fcPlcffldTxbx;
    rSt >> lcbPlcffldTxbx;
    rSt >> fcPlcfHdrtxbxTxt;
    rSt >> lcbPlcfHdrtxbxTxt;
    rSt >> fcPlcffldHdrTxbx;
    rSt >> lcbPlcffldHdrTxbx;

    if( 0 == rSt.GetError() )
    {
        // Bit-Flags setzen
        fDot        =   aBits1 & 0x01       ;
        fGlsy       = ( aBits1 & 0x02 ) >> 1;
        fComplex    = ( aBits1 & 0x04 ) >> 2;
        fHasPic     = ( aBits1 & 0x08 ) >> 3;
        cQuickSaves = ( aBits1 & 0xf0 ) >> 4;
        fEncrypted  =   aBits2 & 0x01       ;
        fWhichTblStm= ( aBits2 & 0x02 ) >> 1;
        // dummy    = ( aBits2 & 0x0e ) >> 1;
        fExtChar    = ( aBits2 & 0x10 ) >> 4;
        // dummy    = ( aBits2 & 0xe0 ) >> 5;

        /*
            ggfs. Ziel-Varaiblen, aus xxx_Ver67 fuellen
            oder Flags setzen
        */
        if (bVer67)
        {
            pnChpFirst = pnChpFirst_Ver67;
            pnPapFirst = pnPapFirst_Ver67;
            cpnBteChp = cpnBteChp_Ver67;
            cpnBtePap = cpnBtePap_Ver67;
        }
        else if( bVer8 )
        {
          fMac              =   aVer8Bits1  & 0x01           ;
          fEmptySpecial     = ( aVer8Bits1  & 0x02 ) >> 1;
          fLoadOverridePage = ( aVer8Bits1  & 0x04 ) >> 2;
          fFuturesavedUndo  = ( aVer8Bits1  & 0x08 ) >> 3;
          fWord97Saved      = ( aVer8Bits1  & 0x10 ) >> 4;
          fWord2000Saved    = ( aVer8Bits1  & 0x20 ) >> 5;

            /*
                speziell fuer WW8:
                ermittle die Werte fuer PLCF LST und PLF LFO
                und PLCF fuer TextBox-Break-Deskriptoren
            */
            long nOldPos = rSt.Tell();
            rSt.Seek( 0x02e2 );
            rSt >> fcPlcfLst;
            rSt >> lcbPlcfLst;
            rSt >> fcPlfLfo;
            rSt >> lcbPlfLfo;
            rSt >> fcPlcftxbxBkd;
            rSt >> lcbPlcftxbxBkd;
            rSt >> fcPlcfHdrtxbxBkd;
            rSt >> lcbPlcfHdrtxbxBkd;
            if( 0 != rSt.GetError() )
            {
                nFibError = ERR_SWG_READ_ERROR;
            }

            rSt.Seek( 0x372 );          // fcSttbListNames
            rSt >> fcSttbListNames;
            rSt >> lcbSttbListNames;
            rSt.Seek( 0x382 );          // MagicTables
            rSt >> fcMagicTable;
            rSt >> lcbMagicTable;
            if( 0 != rSt.GetError() )
                nFibError = ERR_SWG_READ_ERROR;

            rSt.Seek( nOldPos );
        }
    }
    else
    {
        nFibError = ERR_SWG_READ_ERROR;     // Error melden
    }
}


WW8Fib::WW8Fib(BYTE nVer)
{
    memset(this, 0, sizeof(*this));
    nVersion = nVer;
    if (8 == nVer)
    {
        fcMin = 0x400;
        wIdent = 0xa5ec;
        nFib = 0xc2;
        nFibBack = 0xbf;
        nProduct = 0x204D;

        csw = 0x0e;     // muss das sein ???
        cfclcb = 0x6c;  //      -""-
        clw = 0x16;     //      -""-
        pnFbpChpFirst = pnFbpPapFirst = pnFbpLvcFirst = 0x000fffff;
        fExtChar = true;
        fWord97Saved = fWord2000Saved = true;

        // diese Flags muessen nicht gesetzt werden; koennen aber.
        //  wMagicCreated = wMagicRevised = 0x6a62;
        //  wMagicCreatedPrivate = wMagicRevisedPrivate = 0xb3b2;
        //

        wMagicCreated = 0x6143;
        wMagicRevised = 0x6C6F;
        wMagicCreatedPrivate = 0x6E61;
        wMagicRevisedPrivate = 0x3037;
    }
    else
    {
        fcMin = 0x300;
        wIdent = 0xa5dc;
        nFib = nFibBack = 0x65;
        nProduct = 0xc02d;
    }

    lid = lidFE = 0x407;
}

bool WW8Fib::Write(SvStream& rStrm)
{
    BYTE *pDataPtr = new BYTE[ fcMin ];
    BYTE *pData = pDataPtr;
    memset( pData, 0, fcMin );

    bool bVer8 = 8 == nVersion;

    ULONG nPos = rStrm.Tell();
    cbMac = rStrm.Seek( STREAM_SEEK_TO_END );
    rStrm.Seek( nPos );

    Set_UInt16( pData, wIdent );
    Set_UInt16( pData, nFib );
    Set_UInt16( pData, nProduct );
    Set_UInt16( pData, lid );
    Set_UInt16( pData, pnNext );

    UINT16 nBits16 = 0;
    if( fDot )          nBits16 |= 0x0001;
    if( fGlsy)          nBits16 |= 0x0002;
    if( fComplex )      nBits16 |= 0x0004;
    if( fHasPic )       nBits16 |= 0x0008;
    nBits16 |= (0xf0 & ( cQuickSaves << 4 ));
    if( fEncrypted )    nBits16 |= 0x0100;
    if( fWhichTblStm )  nBits16 |= 0x0200;
    if( fExtChar )      nBits16 |= 0x1000;
    Set_UInt16( pData, nBits16 );

    Set_UInt16( pData, nFibBack );
    Set_UInt16( pData, lKey1 );
    Set_UInt16( pData, lKey2 );
    Set_UInt8( pData, envr );

    BYTE nBits8 = 0;
    if( bVer8 )
    {
        if( fMac )                  nBits8 |= 0x0001;
        if( fEmptySpecial )         nBits8 |= 0x0002;
        if( fLoadOverridePage )     nBits8 |= 0x0004;
        if( fFuturesavedUndo )      nBits8 |= 0x0008;
        if( fWord97Saved )          nBits8 |= 0x0010;
        if( fWord2000Saved )        nBits8 |= 0x0020;
    }
    // unter Ver67 these are only reserved
    Set_UInt8( pData, nBits8  );

    Set_UInt16( pData, chse );
    Set_UInt16( pData, chseTables );
    Set_UInt32( pData, fcMin );
    Set_UInt32( pData, fcMac );

// Einschub fuer WW8 *****************************************************

    // Marke: "rgsw"  Beginning of the array of shorts
    if( bVer8 )
    {
        Set_UInt16( pData, csw );
        Set_UInt16( pData, wMagicCreated );
        Set_UInt16( pData, wMagicRevised );
        Set_UInt16( pData, wMagicCreatedPrivate );
        Set_UInt16( pData, wMagicRevisedPrivate );
        pData += 9 * sizeof( INT16 );
        Set_UInt16( pData, lidFE );
        Set_UInt16( pData, clw );
    }

// Ende des Einschubs fuer WW8 *******************************************

    // Marke: "rglw"  Beginning of the array of longs
    Set_UInt32( pData, cbMac );

    // 2 Longs uebergehen, da unwichtiger Quatsch
    pData += 2 * sizeof( INT32);

    // weitere 2 Longs nur bei Ver67 ueberspringen
    if( !bVer8 )
        pData += 2 * sizeof( INT32);

    Set_UInt32( pData, ccpText );
    Set_UInt32( pData, ccpFtn );
    Set_UInt32( pData, ccpHdr );
    Set_UInt32( pData, ccpMcr );
    Set_UInt32( pData, ccpAtn );
    Set_UInt32( pData, ccpEdn );
    Set_UInt32( pData, ccpTxbx );
    Set_UInt32( pData, ccpHdrTxbx );

        // weiteres Long nur bei Ver67 ueberspringen
    if( !bVer8 )
        pData += 1 * sizeof( INT32);

// Einschub fuer WW8 *****************************************************
    if( bVer8 )
    {
        Set_UInt32( pData, pnFbpChpFirst );
        Set_UInt32( pData, pnChpFirst );
        Set_UInt32( pData, cpnBteChp );
        Set_UInt32( pData, pnFbpPapFirst );
        Set_UInt32( pData, pnPapFirst );
        Set_UInt32( pData, cpnBtePap );
        Set_UInt32( pData, pnFbpLvcFirst );
        Set_UInt32( pData, pnLvcFirst );
        Set_UInt32( pData, cpnBteLvc );
        Set_UInt32( pData, fcIslandFirst );
        Set_UInt32( pData, fcIslandLim );
        Set_UInt16( pData, cfclcb );
    }
// Ende des Einschubs fuer WW8 *******************************************

    // Marke: "rgfclcb" Beginning of array of FC/LCB pairs.
    Set_UInt32( pData, fcStshfOrig );
    Set_UInt32( pData, lcbStshfOrig );
    Set_UInt32( pData, fcStshf );
    Set_UInt32( pData, lcbStshf );
    Set_UInt32( pData, fcPlcffndRef );
    Set_UInt32( pData, lcbPlcffndRef );
    Set_UInt32( pData, fcPlcffndTxt );
    Set_UInt32( pData, lcbPlcffndTxt );
    Set_UInt32( pData, fcPlcfandRef );
    Set_UInt32( pData, lcbPlcfandRef );
    Set_UInt32( pData, fcPlcfandTxt );
    Set_UInt32( pData, lcbPlcfandTxt );
    Set_UInt32( pData, fcPlcfsed );
    Set_UInt32( pData, lcbPlcfsed );
    Set_UInt32( pData, fcPlcfpad );
    Set_UInt32( pData, lcbPlcfpad );
    Set_UInt32( pData, fcPlcfphe );
    Set_UInt32( pData, lcbPlcfphe );
    Set_UInt32( pData, fcSttbfglsy );
    Set_UInt32( pData, lcbSttbfglsy );
    Set_UInt32( pData, fcPlcfglsy );
    Set_UInt32( pData, lcbPlcfglsy );
    Set_UInt32( pData, fcPlcfhdd );
    Set_UInt32( pData, lcbPlcfhdd );
    Set_UInt32( pData, fcPlcfbteChpx );
    Set_UInt32( pData, lcbPlcfbteChpx );
    Set_UInt32( pData, fcPlcfbtePapx );
    Set_UInt32( pData, lcbPlcfbtePapx );
    Set_UInt32( pData, fcPlcfsea );
    Set_UInt32( pData, lcbPlcfsea );
    Set_UInt32( pData, fcSttbfffn );
    Set_UInt32( pData, lcbSttbfffn );
    Set_UInt32( pData, fcPlcffldMom );
    Set_UInt32( pData, lcbPlcffldMom );
    Set_UInt32( pData, fcPlcffldHdr );
    Set_UInt32( pData, lcbPlcffldHdr );
    Set_UInt32( pData, fcPlcffldFtn );
    Set_UInt32( pData, lcbPlcffldFtn );
    Set_UInt32( pData, fcPlcffldAtn );
    Set_UInt32( pData, lcbPlcffldAtn );
    Set_UInt32( pData, fcPlcffldMcr );
    Set_UInt32( pData, lcbPlcffldMcr );
    Set_UInt32( pData, fcSttbfbkmk );
    Set_UInt32( pData, lcbSttbfbkmk );
    Set_UInt32( pData, fcPlcfbkf );
    Set_UInt32( pData, lcbPlcfbkf );
    Set_UInt32( pData, fcPlcfbkl );
    Set_UInt32( pData, lcbPlcfbkl );
    Set_UInt32( pData, fcCmds );
    Set_UInt32( pData, lcbCmds );
    Set_UInt32( pData, fcPlcfmcr );
    Set_UInt32( pData, lcbPlcfmcr );
    Set_UInt32( pData, fcSttbfmcr );
    Set_UInt32( pData, lcbSttbfmcr );
    Set_UInt32( pData, fcPrDrvr );
    Set_UInt32( pData, lcbPrDrvr );
    Set_UInt32( pData, fcPrEnvPort );
    Set_UInt32( pData, lcbPrEnvPort );
    Set_UInt32( pData, fcPrEnvLand );
    Set_UInt32( pData, lcbPrEnvLand );
    Set_UInt32( pData, fcWss );
    Set_UInt32( pData, lcbWss );
    Set_UInt32( pData, fcDop );
    Set_UInt32( pData, lcbDop );
    Set_UInt32( pData, fcSttbfAssoc );
    Set_UInt32( pData, cbSttbfAssoc );
    Set_UInt32( pData, fcClx );
    Set_UInt32( pData, lcbClx );
    Set_UInt32( pData, fcPlcfpgdFtn );
    Set_UInt32( pData, lcbPlcfpgdFtn );
    Set_UInt32( pData, fcAutosaveSource );
    Set_UInt32( pData, lcbAutosaveSource );
    Set_UInt32( pData, fcGrpStAtnOwners );
    Set_UInt32( pData, lcbGrpStAtnOwners );
    Set_UInt32( pData, fcSttbfAtnbkmk );
    Set_UInt32( pData, lcbSttbfAtnbkmk );

    // weiteres short nur bei Ver67 ueberspringen
    if( !bVer8 )
    {
        pData += 1*sizeof( INT16);
        Set_UInt16( pData, (UINT16)pnChpFirst );
        Set_UInt16( pData, (UINT16)pnPapFirst );
        Set_UInt16( pData, (UINT16)cpnBteChp );
        Set_UInt16( pData, (UINT16)cpnBtePap );
    }

    Set_UInt32( pData, fcPlcfdoaMom ); // nur bei Ver67, in Ver8 unused
    Set_UInt32( pData, lcbPlcfdoaMom ); // nur bei Ver67, in Ver8 unused
    Set_UInt32( pData, fcPlcfdoaHdr ); // nur bei Ver67, in Ver8 unused
    Set_UInt32( pData, lcbPlcfdoaHdr ); // nur bei Ver67, in Ver8 unused

    Set_UInt32( pData, fcPlcfspaMom ); // in Ver67 leere Reserve
    Set_UInt32( pData, lcbPlcfspaMom ); // in Ver67 leere Reserve
    Set_UInt32( pData, fcPlcfspaHdr ); // in Ver67 leere Reserve
    Set_UInt32( pData, lcbPlcfspaHdr ); // in Ver67 leere Reserve

    Set_UInt32( pData, fcPlcfAtnbkf );
    Set_UInt32( pData, lcbPlcfAtnbkf );
    Set_UInt32( pData, fcPlcfAtnbkl );
    Set_UInt32( pData, lcbPlcfAtnbkl );
    Set_UInt32( pData, fcPms );
    Set_UInt32( pData, lcbPMS );
    Set_UInt32( pData, fcFormFldSttbf );
    Set_UInt32( pData, lcbFormFldSttbf );
    Set_UInt32( pData, fcPlcfendRef );
    Set_UInt32( pData, lcbPlcfendRef );
    Set_UInt32( pData, fcPlcfendTxt );
    Set_UInt32( pData, lcbPlcfendTxt );
    Set_UInt32( pData, fcPlcffldEdn );
    Set_UInt32( pData, lcbPlcffldEdn );
    Set_UInt32( pData, fcPlcfpgdEdn );
    Set_UInt32( pData, lcbPlcfpgdEdn );
    Set_UInt32( pData, fcDggInfo ); // in Ver67 leere Reserve
    Set_UInt32( pData, lcbDggInfo ); // in Ver67 leere Reserve
    Set_UInt32( pData, fcSttbfRMark );
    Set_UInt32( pData, lcbSttbfRMark );
    Set_UInt32( pData, fcSttbfCaption );
    Set_UInt32( pData, lcbSttbfCaption );
    Set_UInt32( pData, fcSttbAutoCaption );
    Set_UInt32( pData, lcbSttbAutoCaption );
    Set_UInt32( pData, fcPlcfwkb );
    Set_UInt32( pData, lcbPlcfwkb );
    Set_UInt32( pData, fcPlcfspl ); // in Ver67 leere Reserve
    Set_UInt32( pData, lcbPlcfspl ); // in Ver67 leere Reserve
    Set_UInt32( pData, fcPlcftxbxTxt );
    Set_UInt32( pData, lcbPlcftxbxTxt );
    Set_UInt32( pData, fcPlcffldTxbx );
    Set_UInt32( pData, lcbPlcffldTxbx );
    Set_UInt32( pData, fcPlcfHdrtxbxTxt );
    Set_UInt32( pData, lcbPlcfHdrtxbxTxt );
    Set_UInt32( pData, fcPlcffldHdrTxbx );
    Set_UInt32( pData, lcbPlcffldHdrTxbx );

    if( bVer8 )
    {
        pData += 0x2e2 - 0x27a;         // Pos + Offset (fcPlcfLst - fcStwUser)
        Set_UInt32( pData, fcPlcfLst );
        Set_UInt32( pData, lcbPlcfLst );
        Set_UInt32( pData, fcPlfLfo );
        Set_UInt32( pData, lcbPlfLfo );
        Set_UInt32( pData, fcPlcftxbxBkd );
        Set_UInt32( pData, lcbPlcftxbxBkd );
        Set_UInt32( pData, fcPlcfHdrtxbxBkd );
        Set_UInt32( pData, lcbPlcfHdrtxbxBkd );

        pData += 0x372 - 0x302; // Pos + Offset (fcSttbListNames - fcDocUndo)
        Set_UInt32( pData, fcSttbListNames );
        Set_UInt32( pData, lcbSttbListNames );

        pData += 0x382 - 0x37A;
        Set_UInt32( pData, fcMagicTable );
        Set_UInt32( pData, lcbMagicTable );

        pData += 0x3FA - 0x38A;
        Set_UInt16( pData, (UINT16)0x0002);
        Set_UInt16( pData, (UINT16)0x00D9);
    }

    rStrm.Write( pDataPtr, fcMin );
    delete[] pDataPtr;
    return 0 == rStrm.GetError();
}

rtl_TextEncoding WW8Fib::GetFIBCharset(UINT16 chs)
{
    ASSERT(chs <= 0x100, "overflowed winword charset set");
    rtl_TextEncoding eCharSet =
        (0x0100 == chs)
        ? RTL_TEXTENCODING_APPLE_ROMAN
        : rtl_getTextEncodingFromWindowsCharset( static_cast<BYTE>(chs) );
    return eCharSet;
}

WW8Style::WW8Style(SvStream& rStream, WW8Fib& rFibPara)
    : rFib(rFibPara), rSt(rStream), cstd(0), cbSTDBaseInFile(0),
    stiMaxWhenSaved(0), istdMaxFixedWhenSaved(0), nVerBuiltInNamesWhenSaved(0),
    ftcStandardChpStsh(0), ftcStandardChpCJKStsh(0), ftcStandardChpCTLStsh(0)
{
    nStyleStart = rFib.fcStshf;
    nStyleLen = rFib.lcbStshf;

    rSt.Seek(nStyleStart);

    USHORT cbStshi = 0; //  2 bytes size of the following STSHI structure

    // alte Version ?
    if (rFib.nFib < 67)
        cbStshi = 4;    // -> Laengenfeld fehlt
    else    // neue Version:
        // lies die Laenge der in der Datei gespeicherten Struktur
        rSt >> cbStshi;

    UINT16 nRead = cbStshi;
    do
    {
        UINT16 a16Bit;

        if(  2 > nRead ) break;
        rSt >> cstd;

        if(  4 > nRead ) break;
        rSt >> cbSTDBaseInFile;

        if(  6 > nRead ) break;
        rSt >> a16Bit;
        fStdStylenamesWritten = a16Bit & 0x0001;

        if(  8 > nRead ) break;
        rSt >> stiMaxWhenSaved;

        if( 10 > nRead ) break;
        rSt >> istdMaxFixedWhenSaved;

        if( 12 > nRead ) break;
        rSt >> nVerBuiltInNamesWhenSaved;

        if( 14 > nRead ) break;
        rSt >> ftcStandardChpStsh;

        if( 16 > nRead ) break;
        rSt >> ftcStandardChpCJKStsh;

        if ( 18 > nRead ) break;
        rSt >> ftcStandardChpCTLStsh;

        // ggfs. den Rest ueberlesen
        if( 18 < nRead )
            rSt.SeekRel( nRead-18 );
    }
    while( !this ); // Trick: obiger Block wird genau einmal durchlaufen
                    //   und kann vorzeitig per "break" verlassen werden.

    if( 0 != rSt.GetError() )
    {
        // wie denn nun den Error melden?
    }
}

// Read1STDFixed() liest ein Style ein. Wenn der Style vollstaendig vorhanden
// ist, d.h. kein leerer Slot, dann wird Speicher alloziert und ein Pointer auf
// die ( evtl. mit Nullen aufgefuellten ) STD geliefert. Ist es ein leerer
// Slot, dann wird ein Nullpointer zurueckgeliefert.
WW8_STD* WW8Style::Read1STDFixed( short& rSkip, short* pcbStd )
{
    WW8_STD* pStd = 0;

    UINT16 cbStd;
    rSt >> cbStd;   // lies Laenge

    UINT16 nRead = cbSTDBaseInFile;
    if( cbStd >= cbSTDBaseInFile )
    {
        // Fixed part vollst. vorhanden

        // read fixed part of STD
        pStd = new WW8_STD;
        memset( pStd, 0, sizeof( *pStd ) );

        do
        {
            UINT16 a16Bit;

            if( 2 > nRead ) break;
            rSt >> a16Bit;
            pStd->sti          =        a16Bit & 0x0fff  ;
            pStd->fScratch     = 0 != ( a16Bit & 0x1000 );
            pStd->fInvalHeight = 0 != ( a16Bit & 0x2000 );
            pStd->fHasUpe      = 0 != ( a16Bit & 0x4000 );
            pStd->fMassCopy    = 0 != ( a16Bit & 0x8000 );

            if( 4 > nRead ) break;
            rSt >> a16Bit;
            pStd->sgc      =   a16Bit & 0x000f       ;
            pStd->istdBase = ( a16Bit & 0xfff0 ) >> 4;

            if( 6 > nRead ) break;
            rSt >> a16Bit;
            pStd->cupx     =   a16Bit & 0x000f       ;
            pStd->istdNext = ( a16Bit & 0xfff0 ) >> 4;

            if( 8 > nRead ) break;
            rSt >> pStd->bchUpe;

            // ab Ver8 sollten diese beiden Felder dazukommen:
            if(10 > nRead ) break;
            rSt >> a16Bit;
            pStd->fAutoRedef =   a16Bit & 0x0001       ;
            pStd->fHidden    = ( a16Bit & 0x0002 ) >> 2;

            // man kann nie wissen: vorsichtshalber ueberlesen
            // wir eventuelle Fuellsel, die noch zum BASE-Part gehoeren...
            if( 10 < nRead )
                rSt.SeekRel( nRead-10 );
        }
        while( !this ); // Trick: obiger Block wird genau einmal durchlaufen
                        //   und kann vorzeitig per "break" verlassen werden.

        if( (0 != rSt.GetError()) || !nRead )
            DELETEZ( pStd );        // per NULL den Error melden

      rSkip = cbStd - cbSTDBaseInFile;
    }
    else
    {           // Fixed part zu kurz
        if( cbStd )
            rSt.SeekRel( cbStd );           // ueberlies Reste
        rSkip = 0;
    }
    if( pcbStd )
        *pcbStd = cbStd;
    return pStd;
}

WW8_STD* WW8Style::Read1Style( short& rSkip, String* pString, short* pcbStd )
{
    // Attention: MacWord-Documents have their Stylenames
    // always in ANSI, even if eStructCharSet == CHARSET_MAC !!

    WW8_STD* pStd = Read1STDFixed( rSkip, pcbStd );         // lese STD

    // String gewuenscht ?
    if( pString )
    {   // echter Style ?
        if ( pStd )
        {
            switch( rFib.nVersion )
            {
                case 6:
                case 7:
                    // lies Pascal-String
                    *pString = WW8ReadPString( rSt, RTL_TEXTENCODING_MS_1252 );
                    // leading len and trailing zero --> 2
                    rSkip -= 2+ pString->Len();
                    break;
                case 8:
                    // handle Unicode-String with leading length short and
                    // trailing zero
                    if (ww8String::TestBeltAndBraces(rSt))
                    {
                        *pString = WW8Read_xstz(rSt, 0, true);
                        rSkip -= (pString->Len() + 2) * 2;
                    }
                    else
                    {
                        /*
                        #i8114#
                        This is supposed to be impossible, its just supposed
                        to be 16 bit count followed by the string and ending
                        in a 0 short. But "Lotus SmartSuite Product: Word Pro"
                        is creating invalid style names in ww7- format. So we
                        use the belt and braces of the ms strings to see if
                        they are not corrupt. If they are then we try them as
                        8bit ones
                        */
                        *pString = WW8ReadPString(rSt,RTL_TEXTENCODING_MS_1252);
                        // leading len and trailing zero --> 2
                        rSkip -= 2+ pString->Len();
                    }
                    break;
                default:
                    ASSERT(!this, "Es wurde vergessen, nVersion zu kodieren!");
                    break;
            }
        }
        else
            *pString = aEmptyStr;   // Kann keinen Namen liefern
    }
    return pStd;
}


//-----------------------------------------


struct WW8_FFN_Ver6 : public WW8_FFN_BASE
{
    // ab Ver6
    sal_Char szFfn[65]; // 0x6 bzw. 0x40 ab Ver8 zero terminated string that
                        // records name of font.
                        // Maximal size of szFfn is 65 characters.
                        // Vorsicht: Dieses Array kann auch kleiner sein!!!
                        // Possibly followed by a second sz which records the
                        // name of an alternate font to use if the first named
                        // font does not exist on this system.
};
struct WW8_FFN_Ver8 : public WW8_FFN_BASE
{
    // ab Ver8 sind folgende beiden Felder eingeschoben,
    // werden von uns ignoriert.
    sal_Char panose[ 10 ];  //  0x6   PANOSE
    sal_Char fs[ 24     ];  //  0x10  FONTSIGNATURE

    // ab Ver8 als Unicode
    UINT16 szFfn[65];   // 0x6 bzw. 0x40 ab Ver8 zero terminated string that
                        // records name of font.
                        // Maximal size of szFfn is 65 characters.
                        // Vorsicht: Dieses Array kann auch kleiner sein!!!
                        // Possibly followed by a second sz which records the
                        // name of an alternate font to use if the first named
                        // font does not exist on this system.
};

WW8Fonts::WW8Fonts( SvStream& rSt, WW8Fib& rFib )
    : pFontA(0), nMax(0)
{
    // Attention: MacWord-Documents have their Fontnames
    // always in ANSI, even if eStructCharSet == CHARSET_MAC !!
    if( rFib.lcbSttbfffn <= 2 )
    {
        ASSERT( !this, "Fonttabelle kaputt! (rFib.lcbSttbfffn < 2)" );
        return;
    }

    bool bVer67 = (8 > rFib.nVersion);

    rSt.Seek( rFib.fcSttbfffn );

    // allocate Font Array
    BYTE* pA   = new BYTE[ rFib.lcbSttbfffn - 2 ];
    WW8_FFN* p = (WW8_FFN*)pA;

    if( !bVer67 )
    {
        // bVer8: read the count of strings in nMax
        rSt >> nMax;
    }

    // Ver8:  skip undefined uint16
    // Ver67: skip the herein stored total byte of structure
    //        - we already got that information in rFib.lcbSttbfffn
    rSt.SeekRel( 2 );

    // read all font information
    rSt.Read( pA, rFib.lcbSttbfffn - 2 );

    if( bVer67 )
    {
        // try to figure out how many fonts are defined here
        nMax = 0;
        long nLeft = rFib.lcbSttbfffn - 2;
        for(;;)
        {
            short nNextSiz;

            nNextSiz = p->cbFfnM1 + 1;
            if( nNextSiz > nLeft )
                break;
            nMax++;
            nLeft -= nNextSiz;
            if( nLeft < 1 )     // can we read the given ammount of bytes ?
                break;
            // increase p by nNextSiz Bytes
            p = (WW8_FFN *)( ( (BYTE*)p ) + nNextSiz );
        }
    }

    if( nMax )
    {
        // allocate Index Array
        pFontA = new WW8_FFN[ nMax ];
        p = pFontA;

        if( bVer67 )
        {
            WW8_FFN_Ver6* pVer6 = (WW8_FFN_Ver6*)pA;
            BYTE c2;
            for(USHORT i=0; i<nMax; ++i, ++p)
            {
                p->cbFfnM1   = pVer6->cbFfnM1;
                c2           = *(((BYTE*)pVer6) + 1);

                p->prg       =  c2 & 0x02;
                p->fTrueType = (c2 & 0x04) >> 2;
                // ein Reserve-Bit ueberspringen
                p->ff        = (c2 & 0x70) >> 4;

                p->wWeight   = SVBT16ToShort( *(SVBT16*)&pVer6->wWeight );
                p->chs       = pVer6->chs;
                p->ibszAlt   = pVer6->ibszAlt;
                /*
                 #i8726# 7- seems to encode the name in the same encoding as
                 the font, e.g load the doc in 97 and save to see the unicode
                 ver of the asian fontnames in that example to confirm.
                 */
                rtl_TextEncoding eEnc = WW8Fib::GetFIBCharset(p->chs);
                if ((eEnc == RTL_TEXTENCODING_SYMBOL) || (eEnc == RTL_TEXTENCODING_DONTKNOW))
                    eEnc = RTL_TEXTENCODING_MS_1252;
                p->sFontname = String(pVer6->szFfn, eEnc);
//              p->sFontname = String(pVer6->szFfn, RTL_TEXTENCODING_MS_1252);
                if (p->ibszAlt)
                {
                    p->sFontname.Append(';');
                    p->sFontname += String(pVer6->szFfn+p->ibszAlt, eEnc);
//                      RTL_TEXTENCODING_MS_1252 );
                }
                pVer6 = (WW8_FFN_Ver6*)( ((BYTE*)pVer6) + pVer6->cbFfnM1 + 1 );
            }
        }
        else
        {
            WW8_FFN_Ver8* pVer8 = (WW8_FFN_Ver8*)pA;
            BYTE c2;
            for(USHORT i=0; i<nMax; ++i, ++p)
            {
                p->cbFfnM1   = pVer8->cbFfnM1;
                c2           = *(((BYTE*)pVer8) + 1);

                p->prg       =  c2 & 0x02;
                p->fTrueType = (c2 & 0x04) >> 2;
                // ein Reserve-Bit ueberspringen
                p->ff        = (c2 & 0x70) >> 4;

                p->wWeight   = SVBT16ToShort( *(SVBT16*)&pVer8->wWeight );
                p->chs       = pVer8->chs;
                p->ibszAlt   = pVer8->ibszAlt;

#ifdef __WW8_NEEDS_COPY
                {
                    BYTE nLen = 0x28;
                    for( UINT16* pTmp = pVer8->szFfn;
                        nLen < pVer8->cbFfnM1 + 1 ; ++pTmp, nLen+=2 )
                    {
                        *pTmp = SVBT16ToShort( *(SVBT16*)pTmp );
                    }
                }
#endif // defined __WW8_NEEDS_COPY

                p->sFontname = pVer8->szFfn;
                if (p->ibszAlt)
                {
                    p->sFontname.Append(';');
                    p->sFontname.Append(pVer8->szFfn+p->ibszAlt);
                }

                // Zeiger auf Ursprungsarray einen Font nach hinten setzen
                pVer8 = (WW8_FFN_Ver8*)( ((BYTE*)pVer8) + pVer8->cbFfnM1 + 1 );
            }
        }
    }
    delete[] pA;
}

const WW8_FFN* WW8Fonts::GetFont( USHORT nNum ) const
{
    if( !pFontA || nNum >= nMax )
        return 0;

    return &pFontA[ nNum ];
}



//-----------------------------------------


// Suche zu einem Header / Footer den Index in der WW-Liste von Headern / Footern
//
// Pferdefuesse bei WinWord6 und -7:
// 1) Am Anfang des Einlesens muss WWPLCF_HdFt mit Fib und Dop konstruiert werden
// 2) Der Haupttext muss sequentiell ueber alle Sections gelesen werden
// 3) Fuer jedes vorkommende Header / Footer - Attribut des Haupttextes
//  ( Darf pro Section maximal eins sein ) muss UpdateIndex() genau einmal
//  mit dem Parameter des Attributes gerufen werden. Dieser Aufruf muss *nach*
//  dem letzten Aufruf von GetTextPos() passieren.
// 4) GetTextPos() darf mit genau einem der obenstehen WW_... aufgerufen werden
//   ( nicht verodern ! )
// -> dann liefert GetTextPos() vielleicht auch ein richtiges Ergebnis

WW8PLCF_HdFt::WW8PLCF_HdFt( SvStream* pSt, WW8Fib& rFib, WW8Dop& rDop )
    : aPLCF( pSt, rFib.fcPlcfhdd , rFib.lcbPlcfhdd , 0 )
{
    nIdxOffset = 0;

     /*
      cmc 23/02/2000: This dop.grpfIhdt has a bit set for each special
      footnote *and endnote!!* seperator,continuation seperator, and
      continuation notice entry, the documentation does not mention the
      endnote seperators, the documentation also gets the index numbers
      backwards when specifiying which bits to test. The bottom six bits
      of this value must be tested and skipped over. Each section's
      grpfIhdt is then tested for the existence of the appropiate headers
      and footers, at the end of each section the nIdxOffset must be updated
      to point to the beginning of the next section's group of headers and
      footers in this PLCF, UpdateIndex does that task.
      */
    for( BYTE nI = 0x1; nI <= 0x20; nI <<= 1 )
        if( nI & rDop.grpfIhdt )                // Bit gesetzt ?
            nIdxOffset++;

    nTextOfs = rFib.ccpText + rFib.ccpFtn;  // Groesse des Haupttextes
                                            // und der Fussnoten
}

bool WW8PLCF_HdFt::GetTextPos(BYTE grpfIhdt, BYTE nWhich, WW8_CP& rStart,
    long& rLen)
{
    BYTE nI = 0x01;
    short nIdx = nIdxOffset;
    while (true)
    {
        if( nI & nWhich )
            break;                      // found
        if( grpfIhdt & nI )
            nIdx++;                     // uninteresting Header / Footer
        nI <<= 1;                       // text next bit
        if( nI > 0x20 )
            return false;               // not found
    }
                                        // nIdx ist HdFt-Index
    WW8_CP nEnd;
    void* pData;

    aPLCF.SetIdx( nIdx );               // Lookup suitable CP
    aPLCF.Get( rStart, nEnd, pData );
    rLen = nEnd - rStart;
    aPLCF++;

    return true;
}

bool WW8PLCF_HdFt::GetTextPosExact(short nIdx, WW8_CP& rStart, long& rLen)
{
    WW8_CP nEnd;
    void* pData;

    aPLCF.SetIdx( nIdx );               // Lookup suitable CP
    aPLCF.Get( rStart, nEnd, pData );
    rLen = nEnd - rStart;
    return true;
}

void WW8PLCF_HdFt::UpdateIndex( BYTE grpfIhdt )
{
    // Caution: Description is not correct
    for( BYTE nI = 0x01; nI <= 0x20; nI <<= 1 )
        if( nI & grpfIhdt )
            nIdxOffset++;
}

//-----------------------------------------
//          WW8Dop
//-----------------------------------------

WW8Dop::WW8Dop( SvStream& rSt, INT16 nFib, INT32 nPos, INT32 nSize )
{
    BYTE* pDataPtr = new BYTE[ 600 ];
    BYTE* pData = pDataPtr;

    UINT32 nRead = 600 < nSize ? 600 : nSize;
    rSt.Seek( nPos );
    if( 2 > nSize || nRead != rSt.Read( pData, nRead ))
    {
        memset( &nDataStart, 0, (&nDataEnd - &nDataStart) );
        nDopError = ERR_SWG_READ_ERROR;     // Error melden
    }
    else
    {
        if( 600 > nRead )
            memset( pData + nRead, 0, 600 - nRead );

        // dann mal die Daten auswerten
        UINT32 a32Bit;
        UINT16 a16Bit;
        BYTE   a8Bit;

        a16Bit = Get_UShort( pData );
        fFacingPages        = 0 != ( a16Bit  &  0x0001 )     ;
        fWidowControl       = 0 != ( a16Bit  &  0x0002 )     ;
        fPMHMainDoc         = 0 != ( a16Bit  &  0x0004 )     ;
        grfSuppression      =      ( a16Bit  &  0x0018 ) >> 3;
        fpc                 =      ( a16Bit  &  0x0060 ) >> 5;
        grpfIhdt            =      ( a16Bit  &  0xff00 ) >> 8;

        a16Bit = Get_UShort( pData );
        rncFtn              =   a16Bit  &  0x0003        ;
        nFtn                = ( a16Bit  & ~0x0003 ) >> 2 ;

        a8Bit = Get_Byte( pData );
        fOutlineDirtySave      = 0 != ( a8Bit  &  0x01   );

        a8Bit = Get_Byte( pData );
        fOnlyMacPics           = 0 != ( a8Bit  &  0x01   );
        fOnlyWinPics           = 0 != ( a8Bit  &  0x02   );
        fLabelDoc              = 0 != ( a8Bit  &  0x04   );
        fHyphCapitals          = 0 != ( a8Bit  &  0x08   );
        fAutoHyphen            = 0 != ( a8Bit  &  0x10   );
        fFormNoFields          = 0 != ( a8Bit  &  0x20   );
        fLinkStyles            = 0 != ( a8Bit  &  0x40   );
        fRevMarking            = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );
        fBackup                = 0 != ( a8Bit  &  0x01   );
        fExactCWords           = 0 != ( a8Bit  &  0x02   );
        fPagHidden             = 0 != ( a8Bit  &  0x04   );
        fPagResults            = 0 != ( a8Bit  &  0x08   );
        fLockAtn               = 0 != ( a8Bit  &  0x10   );
        fMirrorMargins         = 0 != ( a8Bit  &  0x20   );
        fReadOnlyRecommended   = 0 != ( a8Bit  &  0x40   );
        fDfltTrueType          = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );
        fPagSuppressTopSpacing = 0 != ( a8Bit  &  0x01   );
        fProtEnabled           = 0 != ( a8Bit  &  0x02   );
        fDispFormFldSel        = 0 != ( a8Bit  &  0x04   );
        fRMView                = 0 != ( a8Bit  &  0x08   );
        fRMPrint               = 0 != ( a8Bit  &  0x10   );
        fWriteReservation      = 0 != ( a8Bit  &  0x20   );
        fLockRev               = 0 != ( a8Bit  &  0x40   );
        fEmbedFonts            = 0 != ( a8Bit  &  0x80   );


        a8Bit = Get_Byte( pData );
        copts_fNoTabForInd           = 0 != ( a8Bit  &  0x01   );
        copts_fNoSpaceRaiseLower     = 0 != ( a8Bit  &  0x02   );
        copts_fSupressSpbfAfterPgBrk = 0 != ( a8Bit  &  0x04   );
        copts_fWrapTrailSpaces       = 0 != ( a8Bit  &  0x08   );
        copts_fMapPrintTextColor     = 0 != ( a8Bit  &  0x10   );
        copts_fNoColumnBalance       = 0 != ( a8Bit  &  0x20   );
        copts_fConvMailMergeEsc      = 0 != ( a8Bit  &  0x40   );
        copts_fSupressTopSpacing     = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );
        copts_fOrigWordTableRules    = 0 != ( a8Bit  &  0x01   );
        copts_fTransparentMetafiles  = 0 != ( a8Bit  &  0x02   );
        copts_fShowBreaksInFrames    = 0 != ( a8Bit  &  0x04   );
        copts_fSwapBordersFacingPgs  = 0 != ( a8Bit  &  0x08   );

        dxaTab = Get_Short( pData );
        wSpare = Get_UShort( pData );
        dxaHotZ = Get_UShort( pData );
        cConsecHypLim = Get_UShort( pData );
        wSpare2 = Get_UShort( pData );
        dttmCreated = Get_Long( pData );
        dttmRevised = Get_Long( pData );
        dttmLastPrint = Get_Long( pData );
        nRevision = Get_Short( pData );
        tmEdited = Get_Long( pData );
        cWords = Get_Long( pData );
        cCh = Get_Long( pData );
        cPg = Get_Short( pData );
        cParas = Get_Long( pData );

        a16Bit = Get_UShort( pData );
        rncEdn =   a16Bit &  0x0003       ;
        nEdn   = ( a16Bit & ~0x0003 ) >> 2;

        a16Bit = Get_UShort( pData );
        epc            =   a16Bit &  0x0003       ;
        nfcFtnRef      = ( a16Bit &  0x003c ) >> 2;
        nfcEdnRef      = ( a16Bit &  0x03c0 ) >> 6;
        fPrintFormData = 0 != ( a16Bit &  0x0400 );
        fSaveFormData  = 0 != ( a16Bit &  0x0800 );
        fShadeFormData = 0 != ( a16Bit &  0x1000 );
        fWCFtnEdn      = 0 != ( a16Bit &  0x8000 );

        cLines = Get_Long( pData );
        cWordsFtnEnd = Get_Long( pData );
        cChFtnEdn = Get_Long( pData );
        cPgFtnEdn = Get_Short( pData );
        cParasFtnEdn = Get_Long( pData );
        cLinesFtnEdn = Get_Long( pData );
        lKeyProtDoc = Get_Long( pData );

        a16Bit = Get_UShort( pData );
        wvkSaved    =   a16Bit & 0x0007        ;
        wScaleSaved = ( a16Bit & 0x0ff8 ) >> 3 ;
        zkSaved     = ( a16Bit & 0x3000 ) >> 12;
        fRotateFontW6 = ( a16Bit & 0x4000 ) >> 14;
        iGutterPos = ( a16Bit &  0x8000 ) >> 15;
        /*
            bei nFib >= 103 gehts weiter:
        */
        if (nFib >= 103)
        {
            a32Bit = Get_ULong( pData );
            SetCompatabilityOptions(a32Bit);
        }

        /*
            bei nFib > 105 gehts weiter:
        */
        if (nFib > 105)
        {
            adt = Get_Short( pData );

            doptypography.ReadFromMem(pData);

            memcpy( &dogrid, pData, sizeof( WW8_DOGRID ));
            pData += sizeof( WW8_DOGRID );

            a16Bit = Get_UShort( pData );
            // die untersten 9 Bit sind uninteressant
            fHtmlDoc                = ( a16Bit &  0x0200 ) >>  9 ;
            fSnapBorder             = ( a16Bit &  0x0800 ) >> 11 ;
            fIncludeHeader          = ( a16Bit &  0x1000 ) >> 12 ;
            fIncludeFooter          = ( a16Bit &  0x2000 ) >> 13 ;
            fForcePageSizePag       = ( a16Bit &  0x4000 ) >> 14 ;
            fMinFontSizePag         = ( a16Bit &  0x8000 ) >> 15 ;

            a16Bit = Get_UShort( pData );
            fHaveVersions   = 0 != ( a16Bit  &  0x0001 );
            fAutoVersion    = 0 != ( a16Bit  &  0x0002 );

            pData += 12;

            cChWS = Get_Long( pData );
            cChWSFtnEdn = Get_Long( pData );
            grfDocEvents = Get_Long( pData );

            pData += 4+30+8;

            cDBC = Get_Long( pData );
            cDBCFtnEdn = Get_Long( pData );

            pData += 1 * sizeof( long );

            nfcFtnRef = Get_Short( pData );
            nfcEdnRef = Get_Short( pData );
            hpsZoonFontPag = Get_Short( pData );
            dywDispPag = Get_Short( pData );

            if (nRead >= 512)
            {
                //500 -> 508, Appear to be repeated here in 2000+
                pData += 8;
                a32Bit = Get_Long( pData );
                SetCompatabilityOptions(a32Bit);
                a32Bit = Get_Long( pData );
                fDontUseHTMLAutoSpacing = (a32Bit & 0x4) >> 2;
            }
        }
    }
    delete[] pDataPtr;
}

WW8Dop::WW8Dop()
{
    // first set everything to a default of 0
    memset( &nDataStart, 0, (&nDataEnd - &nDataStart) );

    fWidowControl = 1;
    fpc = 1;
    nFtn = 1;
    fOutlineDirtySave = 1;
    fHyphCapitals = 1;
    fBackup = 1;
    fPagHidden = 1;
    fPagResults = 1;
    fDfltTrueType = 1;

    /*
    Writer acts like this all the time at the moment, ideally we need an
    option for these two as well to import word docs that are not like
    this by default
    */
    fNoLeading = 1;
    fUsePrinterMetrics = 1;

    fRMView = 1;
    fRMPrint = 1;
    dxaTab = 0x2d0;
    dxaHotZ = 0x168;
    dttmCreated = 0x45FBAC69;
    dttmRevised = 0x45FBAC69;
    nRevision = 1;
    nEdn = 1;

    epc = 3;
    nfcEdnRef = 2;
    fShadeFormData = 1;

    wvkSaved = 2;
    wScaleSaved = 100;
    zkSaved = 0;

    lvl = 9;
    fIncludeHeader = 1;
    fIncludeFooter = 1;

    cChWS = /**!!**/ 0;
    cChWSFtnEdn = /**!!**/ 0;

    cDBC = /**!!**/ 0;
    cDBCFtnEdn = /**!!**/ 0;
}

void WW8Dop::SetCompatabilityOptions(UINT32 a32Bit)
{
    fNoTabForInd                = ( a32Bit &  0x00000001 )       ;
    fNoSpaceRaiseLower          = ( a32Bit &  0x00000002 ) >>  1 ;
    fSupressSpbfAfterPageBreak  = ( a32Bit &  0x00000004 ) >>  2 ;
    fWrapTrailSpaces            = ( a32Bit &  0x00000008 ) >>  3 ;
    fMapPrintTextColor          = ( a32Bit &  0x00000010 ) >>  4 ;
    fNoColumnBalance            = ( a32Bit &  0x00000020 ) >>  5 ;
    fConvMailMergeEsc           = ( a32Bit &  0x00000040 ) >>  6 ;
    fSupressTopSpacing          = ( a32Bit &  0x00000080 ) >>  7 ;
    fOrigWordTableRules         = ( a32Bit &  0x00000100 ) >>  8 ;
    fTransparentMetafiles       = ( a32Bit &  0x00000200 ) >>  9 ;
    fShowBreaksInFrames         = ( a32Bit &  0x00000400 ) >> 10 ;
    fSwapBordersFacingPgs       = ( a32Bit &  0x00000800 ) >> 11 ;
    fSuppressTopSpacingMac5     = ( a32Bit &  0x00010000 ) >> 16 ;
    fTruncDxaExpand             = ( a32Bit &  0x00020000 ) >> 17 ;
    fPrintBodyBeforeHdr         = ( a32Bit &  0x00040000 ) >> 18 ;
    fNoLeading                  = ( a32Bit &  0x00080000 ) >> 19 ;
    fMWSmallCaps                = ( a32Bit &  0x00200000 ) >> 21 ;

    fUsePrinterMetrics          = ( a32Bit &  0x80000000 ) >> 31 ;
}

UINT32 WW8Dop::GetCompatabilityOptions() const
{
    UINT32 a32Bit = 0;
    if (fNoTabForInd)                   a32Bit |= 0x00000001;
    if (fNoSpaceRaiseLower)             a32Bit |= 0x00000002;
    if (fSupressSpbfAfterPageBreak)     a32Bit |= 0x00000004;
    if (fWrapTrailSpaces)               a32Bit |= 0x00000008;
    if (fMapPrintTextColor)             a32Bit |= 0x00000010;
    if (fNoColumnBalance)               a32Bit |= 0x00000020;
    if (fConvMailMergeEsc)              a32Bit |= 0x00000040;
    if (fSupressTopSpacing)             a32Bit |= 0x00000080;
    if (fOrigWordTableRules)            a32Bit |= 0x00000100;
    if (fTransparentMetafiles)          a32Bit |= 0x00000200;
    if (fShowBreaksInFrames)            a32Bit |= 0x00000400;
    if (fSwapBordersFacingPgs)          a32Bit |= 0x00000800;
    if (fSuppressTopSpacingMac5)        a32Bit |= 0x00010000;
    if (fTruncDxaExpand)                a32Bit |= 0x00020000;
    if (fPrintBodyBeforeHdr)            a32Bit |= 0x00040000;
    if (fNoLeading)                     a32Bit |= 0x00080000;
    if (fMWSmallCaps)                   a32Bit |= 0x00200000;
    if (fUsePrinterMetrics)             a32Bit |= 0x80000000;
    return a32Bit;
}

bool WW8Dop::Write(SvStream& rStrm, WW8Fib& rFib) const
{
    INT32 nLen = 8 == rFib.nVersion ? 0x220 : 84;
    rFib.fcDop =  rStrm.Tell();
    rFib.lcbDop = nLen;

    BYTE aData[ 0x220 ];
    memset( aData, 0, 0x220 );
    BYTE* pData = aData;

    // dann mal die Daten auswerten
    UINT16 a16Bit;
    BYTE   a8Bit;

    a16Bit = 0;
    if (fFacingPages)
        a16Bit |= 0x0001;
    if (fWidowControl)
        a16Bit |= 0x0002;
    if (fPMHMainDoc)
        a16Bit |= 0x0004;
    a16Bit |= ( 0x0018 & (grfSuppression << 3));
    a16Bit |= ( 0x0060 & (fpc << 5));
    a16Bit |= ( 0xff00 & (grpfIhdt << 8));
    Set_UInt16( pData, a16Bit );

    a16Bit = 0;
    a16Bit |= ( 0x0003 & rncFtn );
    a16Bit |= ( ~0x0003 & (nFtn << 2));
    Set_UInt16( pData, a16Bit );

    a8Bit = 0;
    if( fOutlineDirtySave ) a8Bit |= 0x01;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;
    if( fOnlyMacPics )  a8Bit |= 0x01;
    if( fOnlyWinPics )  a8Bit |= 0x02;
    if( fLabelDoc )     a8Bit |= 0x04;
    if( fHyphCapitals ) a8Bit |= 0x08;
    if( fAutoHyphen )   a8Bit |= 0x10;
    if( fFormNoFields ) a8Bit |= 0x20;
    if( fLinkStyles )   a8Bit |= 0x40;
    if( fRevMarking )   a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;
    if( fBackup )               a8Bit |= 0x01;
    if( fExactCWords )          a8Bit |= 0x02;
    if( fPagHidden )            a8Bit |= 0x04;
    if( fPagResults )           a8Bit |= 0x08;
    if( fLockAtn )              a8Bit |= 0x10;
    if( fMirrorMargins )        a8Bit |= 0x20;
    if( fReadOnlyRecommended )  a8Bit |= 0x40;
    if( fDfltTrueType )         a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;
    if( fPagSuppressTopSpacing )    a8Bit |= 0x01;
    if( fProtEnabled )              a8Bit |= 0x02;
    if( fDispFormFldSel )           a8Bit |= 0x04;
    if( fRMView )                   a8Bit |= 0x08;
    if( fRMPrint )                  a8Bit |= 0x10;
    if( fWriteReservation )         a8Bit |= 0x20;
    if( fLockRev )                  a8Bit |= 0x40;
    if( fEmbedFonts )               a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );


    a8Bit = 0;
    if( copts_fNoTabForInd )            a8Bit |= 0x01;
    if( copts_fNoSpaceRaiseLower )      a8Bit |= 0x02;
    if( copts_fSupressSpbfAfterPgBrk )  a8Bit |= 0x04;
    if( copts_fWrapTrailSpaces )        a8Bit |= 0x08;
    if( copts_fMapPrintTextColor )      a8Bit |= 0x10;
    if( copts_fNoColumnBalance )        a8Bit |= 0x20;
    if( copts_fConvMailMergeEsc )       a8Bit |= 0x40;
    if( copts_fSupressTopSpacing )      a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;
    if( copts_fOrigWordTableRules )     a8Bit |= 0x01;
    if( copts_fTransparentMetafiles )   a8Bit |= 0x02;
    if( copts_fShowBreaksInFrames )     a8Bit |= 0x04;
    if( copts_fSwapBordersFacingPgs )   a8Bit |= 0x08;
    Set_UInt8( pData, a8Bit );

    Set_UInt16( pData, dxaTab );
    Set_UInt16( pData, wSpare );
    Set_UInt16( pData, dxaHotZ );
    Set_UInt16( pData, cConsecHypLim );
    Set_UInt16( pData, wSpare2 );
    Set_UInt32( pData, dttmCreated );
    Set_UInt32( pData, dttmRevised );
    Set_UInt32( pData, dttmLastPrint );
    Set_UInt16( pData, nRevision );
    Set_UInt32( pData, tmEdited );
    Set_UInt32( pData, cWords );
    Set_UInt32( pData, cCh );
    Set_UInt16( pData, cPg );
    Set_UInt32( pData, cParas );

    a16Bit = 0;
    a16Bit |= ( 0x0003 & rncEdn );
    a16Bit |= (~0x0003 & ( nEdn << 2));
    Set_UInt16( pData, a16Bit );

    a16Bit = 0;
    a16Bit |= (0x0003 & epc );
    a16Bit |= (0x003c & (nfcFtnRef << 2));
    a16Bit |= (0x03c0 & (nfcEdnRef << 6));
    if( fPrintFormData )    a16Bit |= 0x0400;
    if( fSaveFormData )     a16Bit |= 0x0800;
    if( fShadeFormData )    a16Bit |= 0x1000;
    if( fWCFtnEdn )         a16Bit |= 0x8000;
    Set_UInt16( pData, a16Bit );

    Set_UInt32( pData, cLines );
    Set_UInt32( pData, cWordsFtnEnd );
    Set_UInt32( pData, cChFtnEdn );
    Set_UInt16( pData, cPgFtnEdn );
    Set_UInt32( pData, cParasFtnEdn );
    Set_UInt32( pData, cLinesFtnEdn );
    Set_UInt32( pData, lKeyProtDoc );

    a16Bit = 0;
    if (wvkSaved)
        a16Bit |= 0x0007;
    a16Bit |= (0x0ff8 & (wScaleSaved << 3));
    a16Bit |= (0x3000 & (zkSaved << 12));
    Set_UInt16( pData, a16Bit );

    if( 8 == rFib.nVersion )
    {
        Set_UInt32(pData, GetCompatabilityOptions());

        Set_UInt16( pData, adt );

        doptypography.WriteToMem(pData);

        memcpy( pData, &dogrid, sizeof( WW8_DOGRID ));
        pData += sizeof( WW8_DOGRID );

        a16Bit = 0x12;      // lvl auf 9 setzen
        if( fHtmlDoc )          a16Bit |= 0x0200;
        if( fSnapBorder )       a16Bit |= 0x0800;
        if( fIncludeHeader )    a16Bit |= 0x1000;
        if( fIncludeFooter )    a16Bit |= 0x2000;
        if( fForcePageSizePag ) a16Bit |= 0x4000;
        if( fMinFontSizePag )   a16Bit |= 0x8000;
        Set_UInt16( pData, a16Bit );

        a16Bit = 0;
        if( fHaveVersions ) a16Bit |= 0x0001;
        if( fAutoVersion )  a16Bit |= 0x0002;
        Set_UInt16( pData, a16Bit );

        pData += 12;

        Set_UInt32( pData, cChWS );
        Set_UInt32( pData, cChWSFtnEdn );
        Set_UInt32( pData, grfDocEvents );

        pData += 4+30+8;

        Set_UInt32( pData, cDBC );
        Set_UInt32( pData, cDBCFtnEdn );

        pData += 1 * sizeof( long );

        Set_UInt16( pData, nfcFtnRef );
        Set_UInt16( pData, nfcEdnRef );
        Set_UInt16( pData, hpsZoonFontPag );
        Set_UInt16( pData, dywDispPag );

        //500 -> 508, Appear to be repeated here in 2000+
        pData += 8;
        Set_UInt32(pData, GetCompatabilityOptions());
        sal_uInt32 a32Bit = 0;
        if (fDontUseHTMLAutoSpacing)
            a32Bit |= 0x0004;
        Set_UInt32(pData, a32Bit);
    }
    rStrm.Write( aData, nLen );
    return 0 == rStrm.GetError();
}

void WW8DopTypography::ReadFromMem(BYTE *&pData)
{
    USHORT a16Bit = Get_UShort(pData);
    fKerningPunct = (a16Bit & 0x0001);
    iJustification = (a16Bit & 0x0006) >>  1;
    iLevelOfKinsoku = (a16Bit & 0x0018) >>  3;
    f2on1 = (a16Bit & 0x0020) >>  5;
    reserved1 = (a16Bit & 0x03C0) >>  6;
    reserved2 = (a16Bit & 0xFC00) >>  10;

    cchFollowingPunct = Get_Short(pData);
    cchLeadingPunct = Get_Short(pData);

    INT16 i;
    for (i=0; i < nMaxFollowing; ++i)
        rgxchFPunct[i] = Get_Short(pData);
    for (i=0; i < nMaxLeading; ++i)
        rgxchLPunct[i] = Get_Short(pData);

    rgxchFPunct[cchFollowingPunct]=0;
    rgxchLPunct[cchLeadingPunct]=0;
}

void WW8DopTypography::WriteToMem(BYTE *&pData) const
{
    USHORT a16Bit = fKerningPunct;
    a16Bit |= (iJustification << 1) & 0x0006;
    a16Bit |= (iLevelOfKinsoku << 3) & 0x0018;
    a16Bit |= (f2on1 << 5) & 0x002;
    a16Bit |= (reserved1 << 6) & 0x03C0;
    a16Bit |= (reserved2 << 10) & 0xFC00;
    Set_UInt16(pData,a16Bit);

    Set_UInt16(pData,cchFollowingPunct);
    Set_UInt16(pData,cchLeadingPunct);

    INT16 i;
    for (i=0; i < nMaxFollowing; ++i)
        Set_UInt16(pData,rgxchFPunct[i]);
    for (i=0; i < nMaxLeading; ++i)
        Set_UInt16(pData,rgxchLPunct[i]);
}

USHORT WW8DopTypography::GetConvertedLang() const
{
    USHORT nLang;
    //I have assumed peoples republic/taiwan == simplified/traditional

    //This isn't a documented issue, so we might have it all wrong,
    //i.e. i.e. whats with the powers of two ?

    /*
    #84082#
    One example of 3 for reserved1 which was really Japanese, perhaps last bit
    is for some other use ?, or redundant. If more examples trigger the assert
    we might be able to figure it out.
    */
    switch(reserved1 & 0xE)
    {
        case 2:     //Japan
            nLang = LANGUAGE_JAPANESE;
            break;
        case 4:     //Chinese (Peoples Republic)
            nLang = LANGUAGE_CHINESE_SIMPLIFIED;
            break;
        case 6:     //Korean
            nLang = LANGUAGE_KOREAN;
            break;
        case 8:     //Chinese (Taiwan)
            nLang = LANGUAGE_CHINESE_TRADITIONAL;
            break;
        default:
            ASSERT(!this, "Unknown MS Asian Typography language, report");
            nLang = LANGUAGE_CHINESE;
            break;
        case 0:
            //And here we have the possibility that it says 2, but its really
            //a bug and only japanese level 2 has been selected after a custom
            //version was chosen on last save!
            nLang = LANGUAGE_JAPANESE;
            break;
    }
    return nLang;
}

//-----------------------------------------
//              Sprms
//-----------------------------------------
USHORT wwSprmParser::GetSprmTailLen(sal_uInt16 nId, const sal_uInt8* pSprm)
    const
{
    SprmInfo aSprm = GetSprmInfo(nId);
    USHORT nL = 0;                      // number of Bytes to read

    //sprmPChgTabs
    switch( nId )
    {
        case 23:
        case 0xC615:
            if( pSprm[1 + mnDelta] != 255 )
                nL = pSprm[1 + mnDelta] + aSprm.nLen;
            else
            {
                BYTE nDel = pSprm[2 + mnDelta];
                BYTE nIns = pSprm[3 + mnDelta + 4 * nDel];

                nL = 2 + 4 * nDel + 3 * nIns;
            }
            break;
        case 0xD608:
            nL = SVBT16ToShort( &pSprm[1 + mnDelta] );
            break;
        default:
            switch (aSprm.nVari)
            {
                case L_FIX:
                    nL = aSprm.nLen;        // Excl. Token
                    break;
                case L_VAR:
                    // Variable 1-Byte Length?
                    // Excl. Token + Var-Lengthbyte
                    nL = pSprm[1 + mnDelta] + aSprm.nLen;
                    break;
                case L_VAR2:
                    // Variable 2-Byte Length?
                    // Excl. Token + Var-Lengthbyte
                    nL = SVBT16ToShort( &pSprm[1 + mnDelta] ) + aSprm.nLen - 1;
                    break;
                default:
                    ASSERT(!this, "Unknown sprm varient");
                    break;
            }
            break;
    }
    return nL;
}

// one or two bytes at the beginning at the sprm id
USHORT wwSprmParser::GetSprmId(const sal_uInt8* pSp) const
{
    ASSERT_RET_ON_FAIL(pSp, "Why GetSprmId with pSp of 0", 0);

    USHORT nId = 0;
    switch (mnVersion)  // 6 stands for "6 OR 7",  7 stands for "ONLY 7"
    {
        case 6:
        case 7:
            nId = *pSp;
            if (0x0100 < nId)
                nId = 0;
            break;
        default:
        case 8:
            nId = SVBT16ToShort(pSp);
            if (0x0800 > nId)
                nId = 0;
            break;
    }
    return nId;
}

// with tokens and length byte
USHORT wwSprmParser::GetSprmSize(sal_uInt16 nId, const sal_uInt8* pSprm) const
{
    return GetSprmTailLen(nId, pSprm) + 1 + mnDelta + SprmDataOfs(nId);
}

BYTE wwSprmParser::SprmDataOfs(USHORT nId) const
{
    return GetSprmInfo(nId).nVari;
}

USHORT wwSprmParser::DistanceToData(USHORT nId) const
{
    return 1 + mnDelta + SprmDataOfs(nId);
}

#if 0
ALNV::ANLV()
    : nfc(0), cbTextBefore(0), cbTextAfter(0), jc(0), fPrev(0), fHang(0),
    fSetBold(0), fSetItalic(0), fSetSmallCaps(0), fSetCaps(0), fSetStrike(0),
    fSetKul(0), fPrevSpace(0), fBold(0), fItalic(0), fSmallCaps(0), fCaps(0),
    fStrike(0), kul(0), ico(0), ftc(0), hps(0), iStartAt(0), dxaIndent(0),
    dxaSpace(0)
{}

void ANLV::ReadFromMem(const sal_uInt8 *&pData)
{
    nfc = Get_Byte(pData);
    cbTextBefore = Get_Byte(pData);
    cbTextAfter = Get_Byte(pData);
    sal_uInt8 nTemp = Get_Byte(pData);
    jc = nTemp & 0x03;
    fPrev = (nTemp & 0x04) >> 2;
    fHang = (nTemp & 0x08) >> 3;
    fSetBold = (nTemp & 0x10) >> 4;
    fSetItalic = (nTemp & 0x20) >> 5;
    fSetSmallCaps = (nTemp & 0x40) >> 6;
    fSetCaps = (nTemp & 0x80) >> 7;
    nTemp = Get_Byte(pData);
    fSetStrike = nTemp & 0x01;
    fSetKul = (nTemp & 0x02) >> 1;
    fPrevSpace = (nTemp & 0x04) >> 2;
    fBold = (nTemp & 0x08) >> 3;
    fItalic = (nTemp & 0x10) >> 4;
    fSmallCaps = (nTemp & 0x20) >> 5;
    fCaps = (nTemp & 0x40) >> 6;
    fStrike = (nTemp & 0x80) >> 7;
    nTemp = Get_Byte(pData);
    kul = nTemp & 0x07;
    ico = (nTemp & 0xF1) >> 3;
    ftc = Get_Short(pData);
    hps = Get_Short(pData);
    iStartAt = Get_Short(pData);
    dxaIndent = Get_Short(pData);
    dxaSpace = Get_Short(pData);
}

OLST::OLST() :
    fRestartHdr(0), fSpareOlst2(0), fSpareOlst3(0), fSpareOlst4(0),
{
    memset(rgxch, 0, sizeof(rgxch));
}

void OLST::ReadFromMem(const sal_uInt8 *&pData, bool bVer67)
{
    for (int i = 0;i < 9; ++i)
        rganlv[i].ReadFromMem(pData);
    fRestartHdr = Get_Byte(pData);
    fSpareOlst2 = Get_Byte(pData);
    fSpareOlst3 = Get_Byte(pData);
    fSpareOlst4 = Get_Byte(pData);
    if (bVer67)
    {
        for (int j = 0; j < 64; ++j)
            rgxch[j] = Get_Byte(pData);
    }
    else
    {
        for (int j = 0; j < 32; ++j)
            rgxch[j] = Get_Short(pData);
    }
}
#endif

SEPr::SEPr() :
    bkc(2), fTitlePage(0), fAutoPgn(0), nfcPgn(0), fUnlocked(0), cnsPgn(0),
    fPgnRestart(0), fEndNote(1), lnc(0), grpfIhdt(0), nLnnMod(0), dxaLnn(0),
    dxaPgn(720), dyaPgn(720), fLBetween(0), vjc(0), dmBinFirst(0),
    dmBinOther(0), dmPaperReq(0), fPropRMark(0), ibstPropRMark(0),
    dttmPropRMark(0), dxtCharSpace(0), dyaLinePitch(0), clm(0), reserved1(0),
    dmOrientPage(0), iHeadingPgn(0), pgnStart(1), lnnMin(0), wTextFlow(0),
    reserved2(0), pgbApplyTo(0), pgbPageDepth(0), pgbOffsetFrom(0),
    xaPage(12240), yaPage(15840), xaPageNUp(12240), yaPageNUp(15840),
    dxaLeft(1800), dxaRight(1800), dyaTop(1440), dyaBottom(1440), dzaGutter(0),
    dyaHdrTop(720), dyaHdrBottom(720), ccolM1(0), fEvenlySpaced(1),
    reserved3(0), fBiDi(0), fFacingCol(0), fRTLGutter(0), fRTLAlignment(0),
    dxaColumns(720), dxaColumnWidth(0), dmOrientFirst(0), fLayout(0),
    reserved4(0)
{
    memset(rgdxaColumnWidthSpacing, 0, sizeof(rgdxaColumnWidthSpacing));
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
