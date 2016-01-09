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

#include "ww8scan.hxx"
#include "ww8par.hxx"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <algorithm>

#include <i18nlangtag/mslangid.hxx>
#include <sprmids.hxx>
#include <rtl/tencinfo.h>
#include <sal/macros.h>

#include <swerror.h>
#include <swtypes.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/localedatawrapper.hxx>
#include <i18nlangtag/lang.h>
#include <editeng/unolingu.hxx>

#include <tools/stream.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#ifdef DEBUGSPRMREADER
#include <stdio.h>
#endif

using namespace ::com::sun::star::lang;

//-begin
namespace SL
{
#   define IMPLCONSTSTRINGARRAY(X) const char a##X[] = #X
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

namespace
{
    /**
        winword strings are typically Belt and Braces strings preceded with a
        pascal style count, and ending with a c style 0 terminator. 16bit chars
        and count for ww8+ and 8bit chars and count for ww7-. The count and 0
        can be checked for integrity to catch errors (e.g. lotus created
        documents) where in error 8bit strings are used instead of 16bits
        strings for style names.
    */
    bool TestBeltAndBraces(SvStream& rStrm)
    {
        bool bRet = false;
        sal_uInt32 nOldPos = rStrm.Tell();
        sal_uInt16 nBelt(0);
        rStrm.ReadUInt16( nBelt );
        nBelt *= sizeof(sal_Unicode);
        if (rStrm.good() && (rStrm.remainingSize() >= (nBelt + sizeof(sal_Unicode))))
        {
            rStrm.SeekRel(nBelt);
            if (rStrm.good())
            {
                sal_Unicode cBraces(0);
                rStrm.ReadUtf16( cBraces );
                if (rStrm.good() && cBraces == 0)
                    bRet = true;
            }
        }
        rStrm.Seek(nOldPos);
        return bRet;
    }
}

const wwSprmSearcher *wwSprmParser::GetWW2SprmSearcher()
{
    //double lock me
    // WW7- Sprms
    static const SprmInfoRow aSprms[] =
    {
        {  0, { 0, L_FIX} }, // "Default-sprm", will be skipped
        {  2, { 1, L_FIX} }, // "sprmPIstd",  pap.istd (style code)
        {  3, { 0, L_VAR} }, // "sprmPIstdPermute pap.istd permutation
        {  4, { 1, L_FIX} }, // "sprmPIncLv1" pap.istddifference
        {  5, { 1, L_FIX} }, // "sprmPJc" pap.jc (justification)
        {  6, { 1, L_FIX} }, // "sprmPFSideBySide" pap.fSideBySide
        {  7, { 1, L_FIX} }, // "sprmPFKeep" pap.fKeep
        {  8, { 1, L_FIX} }, // "sprmPFKeepFollow " pap.fKeepFollow
        {  9, { 1, L_FIX} }, // "sprmPPageBreakBefore" pap.fPageBreakBefore
        { 10, { 1, L_FIX} }, // "sprmPBrcl" pap.brcl
        { 11, { 1, L_FIX} }, // "sprmPBrcp" pap.brcp
        { 12, { 1, L_FIX} }, // "sprmPNfcSeqNumb" pap.nfcSeqNumb
        { 13, { 1, L_FIX} }, // "sprmPNoSeqNumb" pap.nnSeqNumb
        { 14, { 1, L_FIX} }, // "sprmPFNoLineNumb" pap.fNoLnn
        { 15, { 0, L_VAR} }, // "?sprmPChgTabsPapx" pap.itbdMac, ...
        { 16, { 2, L_FIX} }, // "sprmPDxaRight" pap.dxaRight
        { 17, { 2, L_FIX} }, // "sprmPDxaLeft" pap.dxaLeft
        { 18, { 2, L_FIX} }, // "sprmPNest" pap.dxaLeft
        { 19, { 2, L_FIX} }, // "sprmPDxaLeft1" pap.dxaLeft1
        { 20, { 2, L_FIX} }, // "sprmPDyaLine" pap.lspd an LSPD
        { 21, { 2, L_FIX} }, // "sprmPDyaBefore" pap.dyaBefore
        { 22, { 2, L_FIX} }, // "sprmPDyaAfter" pap.dyaAfter
        { 23, { 0, L_VAR} }, // "?sprmPChgTabs" pap.itbdMac, pap.rgdxaTab, ...
        { 24, { 1, L_FIX} }, // "sprmPFInTable" pap.fInTable
        { 25, { 1, L_FIX} }, // "sprmPTtp" pap.fTtp
        { 26, { 2, L_FIX} }, // "sprmPDxaAbs" pap.dxaAbs
        { 27, { 2, L_FIX} }, // "sprmPDyaAbs" pap.dyaAbs
        { 28, { 2, L_FIX} }, // "sprmPDxaWidth" pap.dxaWidth
        { 29, { 1, L_FIX} }, // "sprmPPc" pap.pcHorz, pap.pcVert
        { 30, { 2, L_FIX} }, // "sprmPBrcTop10" pap.brcTop BRC10
        { 31, { 2, L_FIX} }, // "sprmPBrcLeft10" pap.brcLeft BRC10
        { 32, { 2, L_FIX} }, // "sprmPBrcBottom10" pap.brcBottom BRC10
        { 33, { 2, L_FIX} }, // "sprmPBrcRight10" pap.brcRight BRC10
        { 34, { 2, L_FIX} }, // "sprmPBrcBetween10" pap.brcBetween BRC10
        { 35, { 2, L_FIX} }, // "sprmPBrcBar10" pap.brcBar BRC10
        { 36, { 2, L_FIX} }, // "sprmPFromText10" pap.dxaFromText dxa
        { 37, { 1, L_FIX} }, // "sprmPWr" pap.wr wr
        { 38, { 2, L_FIX} }, // "sprmPBrcTop" pap.brcTop BRC
        { 39, { 2, L_FIX} }, // "sprmPBrcLeft" pap.brcLeft BRC
        { 40, { 2, L_FIX} }, // "sprmPBrcBottom" pap.brcBottom BRC
        { 41, { 2, L_FIX} }, // "sprmPBrcRight" pap.brcRight BRC
        { 42, { 2, L_FIX} }, // "sprmPBrcBetween" pap.brcBetween BRC
        { 43, { 2, L_FIX} }, // "sprmPBrcBar" pap.brcBar BRC word
        { 44, { 1, L_FIX} }, // "sprmPFNoAutoHyph" pap.fNoAutoHyph
        { 45, { 2, L_FIX} }, // "sprmPWHeightAbs" pap.wHeightAbs w
        { 46, { 2, L_FIX} }, // "sprmPDcs" pap.dcs DCS
        { 47, { 2, L_FIX} }, // "sprmPShd" pap.shd SHD
        { 48, { 2, L_FIX} }, // "sprmPDyaFromText" pap.dyaFromText dya
        { 49, { 2, L_FIX} }, // "sprmPDxaFromText" pap.dxaFromText dxa
        { 50, { 1, L_FIX} }, // "sprmPFBiDi" pap.fBiDi 0 or 1 byte
        { 51, { 1, L_FIX} }, // "sprmPFWidowControl" pap.fWidowControl 0 or 1 byte
        { 52, { 0, L_FIX} }, // "?sprmPRuler 52"
        { 53, { 1, L_FIX} }, // "sprmCFStrikeRM" chp.fRMarkDel 1 or 0 bit
        { 54, { 1, L_FIX} }, // "sprmCFRMark" chp.fRMark 1 or 0 bit
        { 55, { 1, L_FIX} }, // "sprmCFFieldVanish" chp.fFieldVanish 1 or 0 bit
        { 57, { 0, L_VAR} }, // "sprmCDefault" whole CHP
        { 58, { 0, L_FIX} }, // "sprmCPlain" whole CHP
        { 60, { 1, L_FIX} }, // "sprmCFBold" chp.fBold 0,1, 128, or 129
        { 61, { 1, L_FIX} }, // "sprmCFItalic" chp.fItalic 0,1, 128, or 129
        { 62, { 1, L_FIX} }, // "sprmCFStrike" chp.fStrike 0,1, 128, or 129
        { 63, { 1, L_FIX} }, // "sprmCFOutline" chp.fOutline 0,1, 128, or 129
        { 64, { 1, L_FIX} }, // "sprmCFShadow" chp.fShadow 0,1, 128, or 129
        { 65, { 1, L_FIX} }, // "sprmCFSmallCaps" chp.fSmallCaps 0,1, 128, or 129
        { 66, { 1, L_FIX} }, // "sprmCFCaps" chp.fCaps 0,1, 128, or 129
        { 67, { 1, L_FIX} }, // "sprmCFVanish" chp.fVanish 0,1, 128, or 129
        { 68, { 2, L_FIX} }, // "sprmCFtc" chp.ftc ftc word
        { 69, { 1, L_FIX} }, // "sprmCKul" chp.kul kul byte
        { 70, { 3, L_FIX} }, // "sprmCSizePos" chp.hps, chp.hpsPos
        { 71, { 2, L_FIX} }, // "sprmCDxaSpace" chp.dxaSpace dxa
        { 72, { 2, L_FIX} }, // "sprmCLid" chp.lid LID
        { 73, { 1, L_FIX} }, // "sprmCIco" chp.ico ico byte
        { 74, { 1, L_FIX} }, // "sprmCHps" chp.hps hps !word!
        { 75, { 1, L_FIX} }, // "sprmCHpsInc" chp.hps
        { 76, { 1, L_FIX} }, // "sprmCHpsPos" chp.hpsPos hps !word!
        { 77, { 1, L_FIX} }, // "sprmCHpsPosAdj" chp.hpsPos hps
        { 78, { 0, L_VAR} }, // "?sprmCMajority" chp.fBold, chp.fItalic, ...
        { 80, { 1, L_FIX} }, // "sprmCFBoldBi" chp.fBoldBi
        { 81, { 1, L_FIX} }, // "sprmCFItalicBi" chp.fItalicBi
        { 82, { 2, L_FIX} }, // "sprmCFtcBi" chp.ftcBi
        { 83, { 2, L_FIX} }, // "sprmClidBi" chp.lidBi
        { 84, { 1, L_FIX} }, // "sprmCIcoBi" chp.icoBi
        { 85, { 1, L_FIX} }, // "sprmCHpsBi" chp.hpsBi
        { 86, { 1, L_FIX} }, // "sprmCFBiDi" chp.fBiDi
        { 87, { 1, L_FIX} }, // "sprmCFDiacColor" chp.fDiacUSico
        { 94, { 1, L_FIX} }, // "sprmPicBrcl" pic.brcl brcl (see PIC definition)
        { 95, {12, L_VAR} }, // "sprmPicScale" pic.mx, pic.my, pic.dxaCropleft,
        { 96, { 2, L_FIX} }, // "sprmPicBrcTop" pic.brcTop BRC word
        { 97, { 2, L_FIX} }, // "sprmPicBrcLeft" pic.brcLeft BRC word
        { 98, { 2, L_FIX} }, // "sprmPicBrcBottom" pic.brcBottom BRC word
        { 99, { 2, L_FIX} }, // "sprmPicBrcRight" pic.brcRight BRC word
        {112, { 1, L_FIX} }, // "sprmSFRTLGutter", set to one if gutter is on
        {114, { 1, L_FIX} }, // "sprmSFBiDi" ;;;
        {115, { 2, L_FIX} }, // "sprmSDmBinFirst" sep.dmBinFirst  word
        {116, { 2, L_FIX} }, // "sprmSDmBinOther" sep.dmBinOther  word
        {117, { 1, L_FIX} }, // "sprmSBkc" sep.bkc bkc byte
        {118, { 1, L_FIX} }, // "sprmSFTitlePage" sep.fTitlePage 0 or 1 byte
        {119, { 2, L_FIX} }, // "sprmSCcolumns" sep.ccolM1 # of cols - 1 word
        {120, { 2, L_FIX} }, // "sprmSDxaColumns" sep.dxaColumns dxa word
        {121, { 1, L_FIX} }, // "sprmSFAutoPgn" sep.fAutoPgn obsolete byte
        {122, { 1, L_FIX} }, // "sprmSNfcPgn" sep.nfcPgn nfc byte
        {123, { 2, L_FIX} }, // "sprmSDyaPgn" sep.dyaPgn dya short
        {124, { 2, L_FIX} }, // "sprmSDxaPgn" sep.dxaPgn dya short
        {125, { 1, L_FIX} }, // "sprmSFPgnRestart" sep.fPgnRestart 0 or 1 byte
        {126, { 1, L_FIX} }, // "sprmSFEndnote" sep.fEndnote 0 or 1 byte
        {127, { 1, L_FIX} }, // "sprmSLnc" sep.lnc lnc byte
        {128, { 1, L_FIX} }, // "sprmSGprfIhdt" sep.grpfIhdt grpfihdt
        {129, { 2, L_FIX} }, // "sprmSNLnnMod" sep.nLnnMod non-neg int. word
        {130, { 2, L_FIX} }, // "sprmSDxaLnn" sep.dxaLnn dxa word
        {131, { 2, L_FIX} }, // "sprmSDyaHdrTop" sep.dyaHdrTop dya word
        {132, { 2, L_FIX} }, // "sprmSDyaHdrBottom" sep.dyaHdrBottom dya word
        {133, { 1, L_FIX} }, // "sprmSLBetween" sep.fLBetween 0 or 1 byte
        {134, { 1, L_FIX} }, // "sprmSVjc" sep.vjc vjc byte
        {135, { 2, L_FIX} }, // "sprmSLnnMin" sep.lnnMin lnn word
        {136, { 2, L_FIX} }, // "sprmSPgnStart" sep.pgnStart pgn word
        {137, { 1, L_FIX} }, // "sprmSBOrientation" sep.dmOrientPage dm byte
        {138, { 1, L_FIX} }, // "sprmSFFacingCol" ;;;
        {139, { 2, L_FIX} }, // "sprmSXaPage" sep.xaPage xa word
        {140, { 2, L_FIX} }, // "sprmSYaPage" sep.yaPage ya word
        {141, { 2, L_FIX} }, // "sprmSDxaLeft" sep.dxaLeft dxa word
        {142, { 2, L_FIX} }, // "sprmSDxaRight" sep.dxaRight dxa word
        {143, { 2, L_FIX} }, // "sprmSDyaTop" sep.dyaTop dya word
        {144, { 2, L_FIX} }, // "sprmSDyaBottom" sep.dyaBottom dya word
        {145, { 2, L_FIX} }, // "sprmSDzaGutter" sep.dzaGutter dza word
        {146, { 2, L_FIX} }, // "sprmTJc" tap.jc jc (low order byte is significant)
        {147, { 2, L_FIX} }, // "sprmTDxaLeft" tap.rgdxaCenter dxa word
        {148, { 2, L_FIX} }, // "sprmTDxaGapHalf" tap.dxaGapHalf, tap.rgdxaCenter
        {149, { 1, L_FIX} }, // "sprmTFBiDi" ;;;
        {152, { 0, L_VAR} }, // "sprmTDefTable10" tap.rgdxaCenter, tap.rgtc complex
        {153, { 2, L_FIX} }, // "sprmTDyaRowHeight" tap.dyaRowHeight dya word
        {154, { 0, L_VAR2} },// "sprmTDefTable" tap.rgtc complex
        {155, { 1, L_VAR} }, // "sprmTDefTableShd" tap.rgshd complex
        {157, { 5, L_FIX} }, // "sprmTSetBrc" tap.rgtc[].rgbrc complex 5 bytes
        {158, { 4, L_FIX} }, // "sprmTInsert" tap.rgdxaCenter,tap.rgtc complex
        {159, { 2, L_FIX} }, // "sprmTDelete" tap.rgdxaCenter, tap.rgtc complex
        {160, { 4, L_FIX} }, // "sprmTDxaCol" tap.rgdxaCenter complex
        {161, { 2, L_FIX} }, // "sprmTMerge" tap.fFirstMerged, tap.fMerged complex
        {162, { 2, L_FIX} }, // "sprmTSplit" tap.fFirstMerged, tap.fMerged complex
        {163, { 5, L_FIX} }, // "sprmTSetBrc10" tap.rgtc[].rgbrc complex 5 bytes
        {164, { 4, L_FIX} }, // "sprmTSetShd", tap.rgshd complex 4 bytes
    };

    static wwSprmSearcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
};

const wwSprmSearcher *wwSprmParser::GetWW6SprmSearcher()
{
    //double lock me
    // WW7- Sprms
    static const SprmInfoRow aSprms[] =
    {
        {  0, { 0, L_FIX} }, // "Default-sprm",  wird uebersprungen
        {  2, { 2, L_FIX} }, // "sprmPIstd",  pap.istd (style code)
        {  3, { 3, L_VAR} }, // "sprmPIstdPermute pap.istd permutation
        {  4, { 1, L_FIX} }, // "sprmPIncLv1" pap.istddifference
        {  5, { 1, L_FIX} }, // "sprmPJc" pap.jc (justification)
        {  6, { 1, L_FIX} }, // "sprmPFSideBySide" pap.fSideBySide
        {  7, { 1, L_FIX} }, // "sprmPFKeep" pap.fKeep
        {  8, { 1, L_FIX} }, // "sprmPFKeepFollow " pap.fKeepFollow
        {  9, { 1, L_FIX} }, // "sprmPPageBreakBefore" pap.fPageBreakBefore
        { 10, { 1, L_FIX} }, // "sprmPBrcl" pap.brcl
        { 11, { 1, L_FIX} }, // "sprmPBrcp" pap.brcp
        { 12, { 0, L_VAR} }, // "sprmPAnld" pap.anld (ANLD structure)
        { 13, { 1, L_FIX} }, // "sprmPNLvlAnm" pap.nLvlAnm nn
        { 14, { 1, L_FIX} }, // "sprmPFNoLineNumb" pap.fNoLnn
        { 15, { 0, L_VAR} }, // "?sprmPChgTabsPapx" pap.itbdMac, ...
        { 16, { 2, L_FIX} }, // "sprmPDxaRight" pap.dxaRight
        { 17, { 2, L_FIX} }, // "sprmPDxaLeft" pap.dxaLeft
        { 18, { 2, L_FIX} }, // "sprmPNest" pap.dxaLeft
        { 19, { 2, L_FIX} }, // "sprmPDxaLeft1" pap.dxaLeft1
        { 20, { 4, L_FIX} }, // "sprmPDyaLine" pap.lspd an LSPD
        { 21, { 2, L_FIX} }, // "sprmPDyaBefore" pap.dyaBefore
        { 22, { 2, L_FIX} }, // "sprmPDyaAfter" pap.dyaAfter
        { 23, { 0, L_VAR} }, // "?sprmPChgTabs" pap.itbdMac, pap.rgdxaTab, ...
        { 24, { 1, L_FIX} }, // "sprmPFInTable" pap.fInTable
        { 25, { 1, L_FIX} }, // "sprmPTtp" pap.fTtp
        { 26, { 2, L_FIX} }, // "sprmPDxaAbs" pap.dxaAbs
        { 27, { 2, L_FIX} }, // "sprmPDyaAbs" pap.dyaAbs
        { 28, { 2, L_FIX} }, // "sprmPDxaWidth" pap.dxaWidth
        { 29, { 1, L_FIX} }, // "sprmPPc" pap.pcHorz, pap.pcVert
        { 30, { 2, L_FIX} }, // "sprmPBrcTop10" pap.brcTop BRC10
        { 31, { 2, L_FIX} }, // "sprmPBrcLeft10" pap.brcLeft BRC10
        { 32, { 2, L_FIX} }, // "sprmPBrcBottom10" pap.brcBottom BRC10
        { 33, { 2, L_FIX} }, // "sprmPBrcRight10" pap.brcRight BRC10
        { 34, { 2, L_FIX} }, // "sprmPBrcBetween10" pap.brcBetween BRC10
        { 35, { 2, L_FIX} }, // "sprmPBrcBar10" pap.brcBar BRC10
        { 36, { 2, L_FIX} }, // "sprmPFromText10" pap.dxaFromText dxa
        { 37, { 1, L_FIX} }, // "sprmPWr" pap.wr wr
        { 38, { 2, L_FIX} }, // "sprmPBrcTop" pap.brcTop BRC
        { 39, { 2, L_FIX} }, // "sprmPBrcLeft" pap.brcLeft BRC
        { 40, { 2, L_FIX} }, // "sprmPBrcBottom" pap.brcBottom BRC
        { 41, { 2, L_FIX} }, // "sprmPBrcRight" pap.brcRight BRC
        { 42, { 2, L_FIX} }, // "sprmPBrcBetween" pap.brcBetween BRC
        { 43, { 2, L_FIX} }, // "sprmPBrcBar" pap.brcBar BRC word
        { 44, { 1, L_FIX} }, // "sprmPFNoAutoHyph" pap.fNoAutoHyph
        { 45, { 2, L_FIX} }, // "sprmPWHeightAbs" pap.wHeightAbs w
        { 46, { 2, L_FIX} }, // "sprmPDcs" pap.dcs DCS
        { 47, { 2, L_FIX} }, // "sprmPShd" pap.shd SHD
        { 48, { 2, L_FIX} }, // "sprmPDyaFromText" pap.dyaFromText dya
        { 49, { 2, L_FIX} }, // "sprmPDxaFromText" pap.dxaFromText dxa
        { 50, { 1, L_FIX} }, // "sprmPFLocked" pap.fLocked 0 or 1 byte
        { 51, { 1, L_FIX} }, // "sprmPFWidowControl" pap.fWidowControl 0 or 1 byte
        { 52, { 0, L_FIX} }, // "?sprmPRuler 52"
        { 64, { 0, L_VAR} }, // rtl property ?
        { 65, { 1, L_FIX} }, // "sprmCFStrikeRM" chp.fRMarkDel 1 or 0 bit
        { 66, { 1, L_FIX} }, // "sprmCFRMark" chp.fRMark 1 or 0 bit
        { 67, { 1, L_FIX} }, // "sprmCFFieldVanish" chp.fFieldVanish 1 or 0 bit
        { 68, { 0, L_VAR} }, // "sprmCPicLocation" chp.fcPic and chp.fSpec
        { 69, { 2, L_FIX} }, // "sprmCIbstRMark" chp.ibstRMark index into sttbRMark
        { 70, { 4, L_FIX} }, // "sprmCDttmRMark" chp.dttm DTTM long
        { 71, { 1, L_FIX} }, // "sprmCFData" chp.fData 1 or 0 bit
        { 72, { 2, L_FIX} }, // "sprmCRMReason" chp.idslRMReason an index to a table
        { 73, { 3, L_FIX} }, // "sprmCChse" chp.fChsDiff and chp.chse
        { 74, { 0, L_VAR} }, // "sprmCSymbol" chp.fSpec, chp.chSym and chp.ftcSym
        { 75, { 1, L_FIX} }, // "sprmCFOle2" chp.fOle2 1 or 0   bit
        { 77, { 0, L_VAR} }, // unknown
        { 79, { 0, L_VAR} }, // unknown
        { 80, { 2, L_FIX} }, // "sprmCIstd" chp.istd istd, see stylesheet definition
        { 81, { 0, L_VAR} }, // "sprmCIstdPermute" chp.istd permutation vector
        { 82, { 0, L_VAR} }, // "sprmCDefault" whole CHP
        { 83, { 0, L_FIX} }, // "sprmCPlain" whole CHP
        { 85, { 1, L_FIX} }, // "sprmCFBold" chp.fBold 0,1, 128, or 129
        { 86, { 1, L_FIX} }, // "sprmCFItalic" chp.fItalic 0,1, 128, or 129
        { 87, { 1, L_FIX} }, // "sprmCFStrike" chp.fStrike 0,1, 128, or 129
        { 88, { 1, L_FIX} }, // "sprmCFOutline" chp.fOutline 0,1, 128, or 129
        { 89, { 1, L_FIX} }, // "sprmCFShadow" chp.fShadow 0,1, 128, or 129
        { 90, { 1, L_FIX} }, // "sprmCFSmallCaps" chp.fSmallCaps 0,1, 128, or 129
        { 91, { 1, L_FIX} }, // "sprmCFCaps" chp.fCaps 0,1, 128, or 129
        { 92, { 1, L_FIX} }, // "sprmCFVanish" chp.fVanish 0,1, 128, or 129
        { 93, { 2, L_FIX} }, // "sprmCFtc" chp.ftc ftc word
        { 94, { 1, L_FIX} }, // "sprmCKul" chp.kul kul byte
        { 95, { 3, L_FIX} }, // "sprmCSizePos" chp.hps, chp.hpsPos
        { 96, { 2, L_FIX} }, // "sprmCDxaSpace" chp.dxaSpace dxa
        { 97, { 2, L_FIX} }, // "sprmCLid" chp.lid LID
        { 98, { 1, L_FIX} }, // "sprmCIco" chp.ico ico byte
        { 99, { 2, L_FIX} }, // "sprmCHps" chp.hps hps !word!
        {100, { 1, L_FIX} }, // "sprmCHpsInc" chp.hps
        {101, { 2, L_FIX} }, // "sprmCHpsPos" chp.hpsPos hps !word!
        {102, { 1, L_FIX} }, // "sprmCHpsPosAdj" chp.hpsPos hps
        {103, { 0, L_VAR} }, // "?sprmCMajority" chp.fBold, chp.fItalic, ...
        {104, { 1, L_FIX} }, // "sprmCIss" chp.iss iss
        {105, { 0, L_VAR} }, // "sprmCHpsNew50" chp.hps hps variable width
        {106, { 0, L_VAR} }, // "sprmCHpsInc1" chp.hps complex
        {107, { 2, L_FIX} }, // "sprmCHpsKern" chp.hpsKern hps
        {108, { 0, L_VAR} }, // "sprmCMajority50" chp.fBold, chp.fItalic, ...
        {109, { 2, L_FIX} }, // "sprmCHpsMul" chp.hps percentage to grow hps
        {110, { 2, L_FIX} }, // "sprmCCondHyhen" chp.ysri ysri
        {111, { 2, L_FIX} }, // ww7 font
        {112, { 2, L_FIX} }, // ww7 CJK font
        {113, { 2, L_FIX} }, // ww7 rtl font
        {114, { 2, L_FIX} }, // ww7 lid
        {115, { 2, L_FIX} }, // ww7 rtl colour ?
        {116, { 2, L_FIX} }, // ww7 fontsize
        {117, { 1, L_FIX} }, // "sprmCFSpec" chp.fSpec  1 or 0 bit
        {118, { 1, L_FIX} }, // "sprmCFObj" chp.fObj 1 or 0 bit
        {119, { 1, L_FIX} }, // "sprmPicBrcl" pic.brcl brcl (see PIC definition)
        {120, {12, L_VAR} }, // "sprmPicScale" pic.mx, pic.my, pic.dxaCropleft,
        {121, { 2, L_FIX} }, // "sprmPicBrcTop" pic.brcTop BRC word
        {122, { 2, L_FIX} }, // "sprmPicBrcLeft" pic.brcLeft BRC word
        {123, { 2, L_FIX} }, // "sprmPicBrcBottom" pic.brcBottom BRC word
        {124, { 2, L_FIX} }, // "sprmPicBrcRight" pic.brcRight BRC word
        {131, { 1, L_FIX} }, // "sprmSScnsPgn" sep.cnsPgn cns byte
        {132, { 1, L_FIX} }, // "sprmSiHeadingPgn" sep.iHeadingPgn
        {133, { 0, L_VAR} }, // "sprmSOlstAnm" sep.olstAnm OLST variable length
        {136, { 3, L_FIX} }, // "sprmSDxaColWidth" sep.rgdxaColWidthSpacing complex
        {137, { 3, L_FIX} }, // "sprmSDxaColSpacing" sep.rgdxaColWidthSpacing
        {138, { 1, L_FIX} }, // "sprmSFEvenlySpaced" sep.fEvenlySpaced 1 or 0
        {139, { 1, L_FIX} }, // "sprmSFProtected" sep.fUnlocked 1 or 0 byte
        {140, { 2, L_FIX} }, // "sprmSDmBinFirst" sep.dmBinFirst  word
        {141, { 2, L_FIX} }, // "sprmSDmBinOther" sep.dmBinOther  word
        {142, { 1, L_FIX} }, // "sprmSBkc" sep.bkc bkc byte
        {143, { 1, L_FIX} }, // "sprmSFTitlePage" sep.fTitlePage 0 or 1 byte
        {144, { 2, L_FIX} }, // "sprmSCcolumns" sep.ccolM1 # of cols - 1 word
        {145, { 2, L_FIX} }, // "sprmSDxaColumns" sep.dxaColumns dxa word
        {146, { 1, L_FIX} }, // "sprmSFAutoPgn" sep.fAutoPgn obsolete byte
        {147, { 1, L_FIX} }, // "sprmSNfcPgn" sep.nfcPgn nfc byte
        {148, { 2, L_FIX} }, // "sprmSDyaPgn" sep.dyaPgn dya short
        {149, { 2, L_FIX} }, // "sprmSDxaPgn" sep.dxaPgn dya short
        {150, { 1, L_FIX} }, // "sprmSFPgnRestart" sep.fPgnRestart 0 or 1 byte
        {151, { 1, L_FIX} }, // "sprmSFEndnote" sep.fEndnote 0 or 1 byte
        {152, { 1, L_FIX} }, // "sprmSLnc" sep.lnc lnc byte
        {153, { 1, L_FIX} }, // "sprmSGprfIhdt" sep.grpfIhdt grpfihdt
        {154, { 2, L_FIX} }, // "sprmSNLnnMod" sep.nLnnMod non-neg int. word
        {155, { 2, L_FIX} }, // "sprmSDxaLnn" sep.dxaLnn dxa word
        {156, { 2, L_FIX} }, // "sprmSDyaHdrTop" sep.dyaHdrTop dya word
        {157, { 2, L_FIX} }, // "sprmSDyaHdrBottom" sep.dyaHdrBottom dya word
        {158, { 1, L_FIX} }, // "sprmSLBetween" sep.fLBetween 0 or 1 byte
        {159, { 1, L_FIX} }, // "sprmSVjc" sep.vjc vjc byte
        {160, { 2, L_FIX} }, // "sprmSLnnMin" sep.lnnMin lnn word
        {161, { 2, L_FIX} }, // "sprmSPgnStart" sep.pgnStart pgn word
        {162, { 1, L_FIX} }, // "sprmSBOrientation" sep.dmOrientPage dm byte
        {163, { 0, L_FIX} }, // "?SprmSBCustomize 163"
        {164, { 2, L_FIX} }, // "sprmSXaPage" sep.xaPage xa word
        {165, { 2, L_FIX} }, // "sprmSYaPage" sep.yaPage ya word
        {166, { 2, L_FIX} }, // "sprmSDxaLeft" sep.dxaLeft dxa word
        {167, { 2, L_FIX} }, // "sprmSDxaRight" sep.dxaRight dxa word
        {168, { 2, L_FIX} }, // "sprmSDyaTop" sep.dyaTop dya word
        {169, { 2, L_FIX} }, // "sprmSDyaBottom" sep.dyaBottom dya word
        {170, { 2, L_FIX} }, // "sprmSDzaGutter" sep.dzaGutter dza word
        {171, { 2, L_FIX} }, // "sprmSDMPaperReq" sep.dmPaperReq dm word
        {179, { 0, L_VAR} }, // rtl property ?
        {181, { 0, L_VAR} }, // rtl property ?
        {182, { 2, L_FIX} }, // "sprmTJc" tap.jc jc (low order byte is significant)
        {183, { 2, L_FIX} }, // "sprmTDxaLeft" tap.rgdxaCenter dxa word
        {184, { 2, L_FIX} }, // "sprmTDxaGapHalf" tap.dxaGapHalf, tap.rgdxaCenter
        {185, { 1, L_FIX} }, // "sprmTFCantSplit" tap.fCantSplit 1 or 0 byte
        {186, { 1, L_FIX} }, // "sprmTTableHeader" tap.fTableHeader 1 or 0 byte
        {187, {12, L_FIX} }, // "sprmTTableBorders" tap.rgbrcTable complex 12 bytes
        {188, { 0, L_VAR} }, // "sprmTDefTable10" tap.rgdxaCenter, tap.rgtc complex
        {189, { 2, L_FIX} }, // "sprmTDyaRowHeight" tap.dyaRowHeight dya word
        {190, { 0, L_VAR2} },// "sprmTDefTable" tap.rgtc complex
        {191, { 1, L_VAR} }, // "sprmTDefTableShd" tap.rgshd complex
        {192, { 4, L_FIX} }, // "sprmTTlp" tap.tlp TLP 4 bytes
        {193, { 5, L_FIX} }, // "sprmTSetBrc" tap.rgtc[].rgbrc complex 5 bytes
        {194, { 4, L_FIX} }, // "sprmTInsert" tap.rgdxaCenter,tap.rgtc complex
        {195, { 2, L_FIX} }, // "sprmTDelete" tap.rgdxaCenter, tap.rgtc complex
        {196, { 4, L_FIX} }, // "sprmTDxaCol" tap.rgdxaCenter complex
        {197, { 2, L_FIX} }, // "sprmTMerge" tap.fFirstMerged, tap.fMerged complex
        {198, { 2, L_FIX} }, // "sprmTSplit" tap.fFirstMerged, tap.fMerged complex
        {199, { 5, L_FIX} }, // "sprmTSetBrc10" tap.rgtc[].rgbrc complex 5 bytes
        {200, { 4, L_FIX} }, // "sprmTSetShd", tap.rgshd complex 4 bytes
        {207, { 0, L_VAR} }  // rtl property ?
    };

    static wwSprmSearcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
};

const wwSprmSearcher *wwSprmParser::GetWW8SprmSearcher()
{
    //double lock me
    //WW8+ Sprms
    static const SprmInfoRow aSprms[] =
    {
        {     0, { 0, L_FIX} }, // "Default-sprm"/ wird uebersprungen
        {0x4600, { 2, L_FIX} }, // "sprmPIstd" pap.istd;istd (style code);short;
        {0xC601, { 0, L_VAR} }, // "sprmPIstdPermute" pap.istd;permutation vector
        {0x2602, { 1, L_FIX} }, // "sprmPIncLvl" pap.istd, pap.lvl;difference
                            // between istd of base PAP and istd of PAP to be
                            // produced
        {0x2403, { 1, L_FIX} }, // "sprmPJc" pap.jc;jc (justification);byte;
        {0x2404, { 1, L_FIX} }, // "sprmPFSideBySide" pap.fSideBySide;0 or 1;byte;
        {0x2405, { 1, L_FIX} }, // "sprmPFKeep" pap.fKeep;0 or 1;byte;
        {0x2406, { 1, L_FIX} }, // "sprmPFKeepFollow" pap.fKeepFollow;0 or 1;byte;
        {0x2407, { 1, L_FIX} }, // "sprmPFPageBreakBefore" pap.fPageBreakBefore;
                            // 0 or 1
        {0x2408, { 1, L_FIX} }, // "sprmPBrcl" pap.brcl;brcl;byte;
        {0x2409, { 1, L_FIX} }, // "sprmPBrcp" pap.brcp;brcp;byte;
        {0x260A, { 1, L_FIX} }, // "sprmPIlvl" pap.ilvl;ilvl;byte;
        {0x460B, { 2, L_FIX} }, // "sprmPIlfo" pap.ilfo;ilfo (list index) ;short;
        {0x240C, { 1, L_FIX} }, // "sprmPFNoLineNumb" pap.fNoLnn;0 or 1;byte;
        {0xC60D, { 0, L_VAR} }, // "sprmPChgTabsPapx" pap.itbdMac, pap.rgdxaTab,
                            // pap.rgtbd;complex
        {0x840E, { 2, L_FIX} }, // "sprmPDxaRight" pap.dxaRight;dxa;word;
        {0x840F, { 2, L_FIX} }, // "sprmPDxaLeft" pap.dxaLeft;dxa;word;
        {0x4610, { 2, L_FIX} }, // "sprmPNest" pap.dxaLeft;dxa
        {0x8411, { 2, L_FIX} }, // "sprmPDxaLeft1" pap.dxaLeft1;dxa;word;
        {0x6412, { 4, L_FIX} }, // "sprmPDyaLine" pap.lspd;an LSPD, a long word
                            // structure consisting of a short of dyaLine
                            // followed by a short of fMultLinespace
        {0xA413, { 2, L_FIX} }, // "sprmPDyaBefore" pap.dyaBefore;dya;word;
        {0xA414, { 2, L_FIX} }, // "sprmPDyaAfter" pap.dyaAfter;dya;word;
        {0xC615, { 0, L_VAR} }, // "sprmPChgTabs" pap.itbdMac, pap.rgdxaTab,
                            // pap.rgtbd;complex
        {0x2416, { 1, L_FIX} }, // "sprmPFInTable" pap.fInTable;0 or 1;byte;
        {0x2417, { 1, L_FIX} }, // "sprmPFTtp" pap.fTtp;0 or 1;byte;
        {0x8418, { 2, L_FIX} }, // "sprmPDxaAbs" pap.dxaAbs;dxa;word;
        {0x8419, { 2, L_FIX} }, // "sprmPDyaAbs" pap.dyaAbs;dya;word;
        {0x841A, { 2, L_FIX} }, // "sprmPDxaWidth" pap.dxaWidth;dxa;word;
        {0x261B, { 1, L_FIX} }, // "sprmPPc" pap.pcHorz, pap.pcVert;complex
        {0x461C, { 2, L_FIX} }, // "sprmPBrcTop10" pap.brcTop;BRC10;word;
        {0x461D, { 2, L_FIX} }, // "sprmPBrcLeft10" pap.brcLeft;BRC10;word;
        {0x461E, { 2, L_FIX} }, // "sprmPBrcBottom10" pap.brcBottom;BRC10;word;
        {0x461F, { 2, L_FIX} }, // "sprmPBrcRight10" pap.brcRight;BRC10;word;
        {0x4620, { 2, L_FIX} }, // "sprmPBrcBetween10" pap.brcBetween;BRC10;word;
        {0x4621, { 2, L_FIX} }, // "sprmPBrcBar10" pap.brcBar;BRC10;word;
        {0x4622, { 2, L_FIX} }, // "sprmPDxaFromText10" pap.dxaFromText;dxa;word;
        {0x2423, { 1, L_FIX} }, // "sprmPWr" pap.wr;wr
        {0x6424, { 4, L_FIX} }, // "sprmPBrcTop80" pap.brcTop;BRC;long;
        {0x6425, { 4, L_FIX} }, // "sprmPBrcLeft80" pap.brcLeft;BRC;long;
        {0x6426, { 4, L_FIX} }, // "sprmPBrcBottom80" pap.brcBottom;BRC;long;
        {0x6427, { 4, L_FIX} }, // "sprmPBrcRight80" pap.brcRight;BRC;long;
        {0x6428, { 4, L_FIX} }, // "sprmPBrcBetween80" pap.brcBetween;BRC;long;
        {0x6629, { 4, L_FIX} }, // "sprmPBrcBar80" pap.brcBar;BRC;long;
        {0x242A, { 1, L_FIX} }, // "sprmPFNoAutoHyph" pap.fNoAutoHyph;0 or 1;byte;
        {0x442B, { 2, L_FIX} }, // "sprmPWHeightAbs" pap.wHeightAbs;w;word;
        {0x442C, { 2, L_FIX} }, // "sprmPDcs" pap.dcs;DCS;short;
        {0x442D, { 2, L_FIX} }, // "sprmPShd" pap.shd;SHD;word;
        {0x842E, { 2, L_FIX} }, // "sprmPDyaFromText" pap.dyaFromText;dya;word;
        {0x842F, { 2, L_FIX} }, // "sprmPDxaFromText" pap.dxaFromText;dxa;word;
        {0x2430, { 1, L_FIX} }, // "sprmPFLocked" pap.fLocked;0 or 1;byte;
        {0x2431, { 1, L_FIX} }, // "sprmPFWidowControl" pap.fWidowControl;0 or 1
        {0xC632, { 0, L_VAR} }, // "sprmPRuler" ;;variable length;
        {0x2433, { 1, L_FIX} }, // "sprmPFKinsoku" pap.fKinsoku;0 or 1;byte;
        {0x2434, { 1, L_FIX} }, // "sprmPFWordWrap" pap.fWordWrap;0 or 1;byte;
        {0x2435, { 1, L_FIX} }, // "sprmPFOverflowPunct" pap.fOverflowPunct;0 or 1
        {0x2436, { 1, L_FIX} }, // "sprmPFTopLinePunct" pap.fTopLinePunct;0 or 1
        {0x2437, { 1, L_FIX} }, // "sprmPFAutoSpaceDE" pap.fAutoSpaceDE;0 or 1
        {0x2438, { 1, L_FIX} }, // "sprmPFAutoSpaceDN" pap.fAutoSpaceDN;0 or 1
        {NS_sprm::LN_PWAlignFont, { 2, L_FIX} }, // "sprmPWAlignFont" pap.wAlignFont;iFa
        {0x443A, { 2, L_FIX} }, // "sprmPFrameTextFlow" pap.fVertical pap.fBackward
                            // pap.fRotateFont;complex
        {0x243B, { 1, L_FIX} }, // "sprmPISnapBaseLine" obsolete: not applicable in
                            // Word97 and later versions;
        {0xC63E, { 0, L_VAR} }, // "sprmPAnld" pap.anld;;variable length;
        {0xC63F, { 0, L_VAR} }, // "sprmPPropRMark" pap.fPropRMark;complex
        {0x2640, { 1, L_FIX} }, // "sprmPOutLvl" pap.lvl;has no effect if pap.istd
                            // is < 1 or is > 9
        {0x2441, { 1, L_FIX} }, // "sprmPFBiDi" ;;byte;
        {0x2443, { 1, L_FIX} }, // "sprmPFNumRMIns" pap.fNumRMIns;1 or 0;bit;
        {0x2444, { 1, L_FIX} }, // "sprmPCrLf" ;;byte;
        {0xC645, { 0, L_VAR} }, // "sprmPNumRM" pap.numrm;;variable length;
        {0x6645, { 4, L_FIX} }, // "sprmPHugePapx" fc in the data stream to locate
                            // the huge grpprl
        {0x6646, { 4, L_FIX} }, // "sprmPHugePapx" fc in the data stream to locate
                            // the huge grpprl
        {0x2447, { 1, L_FIX} }, // "sprmPFUsePgsuSettings" pap.fUsePgsuSettings;
                            // 1 or 0
        {0x2448, { 1, L_FIX} }, // "sprmPFAdjustRight" pap.fAdjustRight;1 or 0;byte;
        {0x0800, { 1, L_FIX} }, // "sprmCFRMarkDel" chp.fRMarkDel;1 or 0;bit;
        {0x0801, { 1, L_FIX} }, // "sprmCFRMark" chp.fRMark;1 or 0;bit;
        {0x0802, { 1, L_FIX} }, // "sprmCFFieldVanish" chp.fFieldVanish;1 or 0;bit;
        {0x6A03, { 4, L_FIX} }, // "sprmCPicLocation" chp.fcPic and chp.fSpec;
        {0x4804, { 2, L_FIX} }, // "sprmCIbstRMark" chp.ibstRMark;index into
                            // sttbRMark
        {0x6805, { 4, L_FIX} }, // "sprmCDttmRMark" chp.dttmRMark;DTTM;long;
        {0x0806, { 1, L_FIX} }, // "sprmCFData" chp.fData;1 or 0;bit;
        {0x4807, { 2, L_FIX} }, // "sprmCIdslRMark" chp.idslRMReason;an index to a
                            // table of strings defined in Word 6.0
                            // executables;short;
        {0xEA08, { 1, L_FIX} }, // "sprmCChs" chp.fChsDiff and chp.chse;
        {0x6A09, { 4, L_FIX} }, // "sprmCSymbol" chp.fSpec, chp.xchSym and
                            // chp.ftcSym
        {0x080A, { 1, L_FIX} }, // "sprmCFOle2" chp.fOle2;1 or 0;bit;
        {0x480B, { 0, L_FIX} }, // "sprmCIdCharType" obsolete: not applicable in
                            // Word97 and later versions;;;
        {0x2A0C, { 1, L_FIX} }, // "sprmCHighlight" chp.fHighlight,
                            // chp.icoHighlight;ico (fHighlight is set to 1 iff
                            // ico is not 0)
        {0x680E, { 4, L_FIX} }, // "sprmCObjLocation" chp.fcObj;FC;long;
        {0x2A10, { 0, L_FIX} }, // "sprmCFFtcAsciSymb" ;;;
        {0x4A30, { 2, L_FIX} }, // "sprmCIstd" chp.istd;istd, see stylesheet def
        {0xCA31, { 0, L_VAR} }, // "sprmCIstdPermute" chp.istd;permutation vector
        {0x2A32, { 0, L_VAR} }, // "sprmCDefault" whole CHP;none;variable length;
        {0x2A33, { 0, L_FIX} }, // "sprmCPlain" whole CHP;none;0;
        {0x2A34, { 1, L_FIX} }, // "sprmCKcd" ;;;
        {0x0835, { 1, L_FIX} }, // "sprmCFBold" chp.fBold;0,1, 128, or 129
        {0x0836, { 1, L_FIX} }, // "sprmCFItalic" chp.fItalic;0,1, 128, or 129
        {0x0837, { 1, L_FIX} }, // "sprmCFStrike" chp.fStrike;0,1, 128, or 129
        {0x0838, { 1, L_FIX} }, // "sprmCFOutline" chp.fOutline;0,1, 128, or 129
        {0x0839, { 1, L_FIX} }, // "sprmCFShadow" chp.fShadow;0,1, 128, or 129
        {0x083A, { 1, L_FIX} }, // "sprmCFSmallCaps" chp.fSmallCaps;0,1, 128, or 129
        {0x083B, { 1, L_FIX} }, // "sprmCFCaps" chp.fCaps;0,1, 128, or 129
        {0x083C, { 1, L_FIX} }, // "sprmCFVanish" chp.fVanish;0,1, 128, or 129
        {0x4A3D, { 2, L_FIX} }, // "sprmCFtcDefault" ;ftc, only used internally
        {0x2A3E, { 1, L_FIX} }, // "sprmCKul" chp.kul;kul;byte;
        {0xEA3F, { 3, L_FIX} }, // "sprmCSizePos" chp.hps, chp.hpsPos;3 bytes;
        {0x8840, { 2, L_FIX} }, // "sprmCDxaSpace" chp.dxaSpace;dxa;word;
        {0x4A41, { 2, L_FIX} }, // "sprmCLid" ;only used internally never stored
        {0x2A42, { 1, L_FIX} }, // "sprmCIco" chp.ico;ico;byte;
        {0x4A43, { 2, L_FIX} }, // "sprmCHps" chp.hps;hps
        {0x2A44, { 1, L_FIX} }, // "sprmCHpsInc" chp.hps;
        {0x4845, { 2, L_FIX} }, // "sprmCHpsPos" chp.hpsPos;hps;short; (doc wrong)
        {0x2A46, { 1, L_FIX} }, // "sprmCHpsPosAdj" chp.hpsPos;hps
        {0xCA47, { 0, L_VAR} }, // "sprmCMajority" chp.fBold, chp.fItalic,
                            // chp.fSmallCaps, chp.fVanish, chp.fStrike,
                            // chp.fCaps, chp.rgftc, chp.hps, chp.hpsPos,
                            // chp.kul, chp.dxaSpace, chp.ico,
                            // chp.rglid;complex;variable length, length byte
                            // plus size of following grpprl;
        {0x2A48, { 1, L_FIX} }, // "sprmCIss" chp.iss;iss;byte;
        {0xCA49, { 0, L_VAR} }, // "sprmCHpsNew50" chp.hps;hps;variable width
        {0xCA4A, { 0, L_VAR} }, // "sprmCHpsInc1" chp.hps;complex
        {0x484B, { 2, L_FIX} }, // "sprmCHpsKern" chp.hpsKern;hps;short;
        {0xCA4C, { 2, L_FIX} }, // "sprmCMajority50" chp.fBold, chp.fItalic,
                            // chp.fSmallCaps, chp.fVanish, chp.fStrike,
                            // chp.fCaps, chp.ftc, chp.hps, chp.hpsPos, chp.kul,
                            // chp.dxaSpace, chp.ico,;complex
        {0x4A4D, { 2, L_FIX} }, // "sprmCHpsMul" chp.hps;percentage to grow hps
        {0x484E, { 2, L_FIX} }, // "sprmCYsri" chp.ysri;ysri;short;
        {0x4A4F, { 2, L_FIX} }, // "sprmCRgFtc0" chp.rgftc[0];ftc for ASCII text
        {0x4A50, { 2, L_FIX} }, // "sprmCRgFtc1" chp.rgftc[1];ftc for Far East text
        {0x4A51, { 2, L_FIX} }, // "sprmCRgFtc2" chp.rgftc[2];ftc for non-FE text
        {0x4852, { 2, L_FIX} }, // "sprmCCharScale"
        {0x2A53, { 1, L_FIX} }, // "sprmCFDStrike" chp.fDStrike;;byte;
        {0x0854, { 1, L_FIX} }, // "sprmCFImprint" chp.fImprint;1 or 0;bit;
        {0x0855, { 1, L_FIX} }, // "sprmCFSpec" chp.fSpec ;1 or 0;bit;
        {0x0856, { 1, L_FIX} }, // "sprmCFObj" chp.fObj;1 or 0;bit;
        {0xCA57, { 0, L_VAR} }, // "sprmCPropRMark" chp.fPropRMark,
                            // chp.ibstPropRMark, chp.dttmPropRMark;Complex
        {0x0858, { 1, L_FIX} }, // "sprmCFEmboss" chp.fEmboss;1 or 0;bit;
        {0x2859, { 1, L_FIX} }, // "sprmCSfxText" chp.sfxtText;text animation;byte;
        {0x085A, { 1, L_FIX} }, // "sprmCFBiDi" ;;;
        {0x085B, { 1, L_FIX} }, // "sprmCFDiacColor" ;;;
        {0x085C, { 1, L_FIX} }, // "sprmCFBoldBi" ;;;
        {0x085D, { 1, L_FIX} }, // "sprmCFItalicBi" ;;;
        {0x4A5E, { 2, L_FIX} },
        {0x485F, { 2, L_FIX} }, // "sprmCLidBi" ;;;
        {0x4A60, { 1, L_FIX} }, // "sprmCIcoBi" ;;;
        {0x4A61, { 2, L_FIX} }, // "sprmCHpsBi" ;;;
        {0xCA62, { 0, L_VAR} }, // "sprmCDispFieldRMark" chp.fDispFieldRMark,
                            // chp.ibstDispFieldRMark, chp.dttmDispFieldRMark ;
        {0x4863, { 2, L_FIX} }, // "sprmCIbstRMarkDel" chp.ibstRMarkDel;index into
                            // sttbRMark;short;
        {NS_sprm::LN_CDttmRMarkDel, { 4, L_FIX} }, // chp.dttmRMarkDel;DTTM;long;
        {0x6865, { 4, L_FIX} }, // "sprmCBrc80" chp.brc;BRC;long;
        {0x4866, { 2, L_FIX} }, // "sprmCShd80" chp.shd;SHD;short;
        {0x4867, { 2, L_FIX} }, // "sprmCIdslRMarkDel" chp.idslRMReasonDel;an index
                            // to a table of strings defined in Word 6.0
                            // executables;short;
        {0x0868, { 1, L_FIX} }, // "sprmCFUsePgsuSettings"
                            // chp.fUsePgsuSettings;1 or 0
        {0x486B, { 2, L_FIX} }, // "sprmCCpg" ;;word;
        {0x486D, { 2, L_FIX} }, // "sprmCRgLid0_80" chp.rglid[0];LID: for non-FE text
        {0x486E, { 2, L_FIX} }, // "sprmCRgLid1_80" chp.rglid[1];LID: for Far East text
        {0x286F, { 1, L_FIX} }, // "sprmCIdctHint" chp.idctHint;IDCT:
        {0x2E00, { 1, L_FIX} }, // "sprmPicBrcl" pic.brcl;brcl (see PIC definition)
        {0xCE01, { 0, L_VAR} }, // "sprmPicScale" pic.mx, pic.my, pic.dxaCropleft,
                            // pic.dyaCropTop pic.dxaCropRight,
                            // pic.dyaCropBottom;Complex
        {0x6C02, { 4, L_FIX} }, // "sprmPicBrcTop80" pic.brcTop;BRC;long;
        {0x6C03, { 4, L_FIX} }, // "sprmPicBrcLeft80" pic.brcLeft;BRC;long;
        {0x6C04, { 4, L_FIX} }, // "sprmPicBrcBottom80" pic.brcBottom;BRC;long;
        {0x6C05, { 4, L_FIX} }, // "sprmPicBrcRight80" pic.brcRight;BRC;long;
        {0x3000, { 1, L_FIX} }, // "sprmScnsPgn" sep.cnsPgn;cns;byte;
        {0x3001, { 1, L_FIX} }, // "sprmSiHeadingPgn" sep.iHeadingPgn;heading number
                            // level;byte;
        {0xD202, { 0, L_VAR} }, // "sprmSOlstAnm" sep.olstAnm;OLST;variable length;
        {0xF203, { 3, L_FIX} }, // "sprmSDxaColWidth" sep.rgdxaColWidthSpacing;
        {0xF204, { 3, L_FIX} }, // "sprmSDxaColSpacing" sep.rgdxaColWidthSpacing;
                            // complex
        {0x3005, { 1, L_FIX} }, // "sprmSFEvenlySpaced" sep.fEvenlySpaced;1 or 0
        {0x3006, { 1, L_FIX} }, // "sprmSFProtected" sep.fUnlocked;1 or 0;byte;
        {0x5007, { 2, L_FIX} }, // "sprmSDmBinFirst" sep.dmBinFirst;;word;
        {0x5008, { 2, L_FIX} }, // "sprmSDmBinOther" sep.dmBinOther;;word;
        {0x3009, { 1, L_FIX} }, // "sprmSBkc" sep.bkc;bkc;byte;
        {0x300A, { 1, L_FIX} }, // "sprmSFTitlePage" sep.fTitlePage;0 or 1;byte;
        {0x500B, { 2, L_FIX} }, // "sprmSCcolumns" sep.ccolM1;# of cols - 1;word;
        {0x900C, { 2, L_FIX} }, // "sprmSDxaColumns" sep.dxaColumns;dxa;word;
        {0x300D, { 1, L_FIX} }, // "sprmSFAutoPgn" sep.fAutoPgn;obsolete;byte;
        {0x300E, { 1, L_FIX} }, // "sprmSNfcPgn" sep.nfcPgn;nfc;byte;
        {0xB00F, { 2, L_FIX} }, // "sprmSDyaPgn" sep.dyaPgn;dya;short;
        {0xB010, { 2, L_FIX} }, // "sprmSDxaPgn" sep.dxaPgn;dya;short;
        {0x3011, { 1, L_FIX} }, // "sprmSFPgnRestart" sep.fPgnRestart;0 or 1;byte;
        {0x3012, { 1, L_FIX} }, // "sprmSFEndnote" sep.fEndnote;0 or 1;byte;
        {0x3013, { 1, L_FIX} }, // "sprmSLnc" sep.lnc;lnc;byte;
        {0x3014, { 1, L_FIX} }, // "sprmSGprfIhdt" sep.grpfIhdt;grpfihdt
        {0x5015, { 2, L_FIX} }, // "sprmSNLnnMod" sep.nLnnMod;non-neg int.;word;
        {0x9016, { 2, L_FIX} }, // "sprmSDxaLnn" sep.dxaLnn;dxa;word;
        {0xB017, { 2, L_FIX} }, // "sprmSDyaHdrTop" sep.dyaHdrTop;dya;word;
        {0xB018, { 2, L_FIX} }, // "sprmSDyaHdrBottom" sep.dyaHdrBottom;dya;word;
        {0x3019, { 1, L_FIX} }, // "sprmSLBetween" sep.fLBetween;0 or 1;byte;
        {0x301A, { 1, L_FIX} }, // "sprmSVjc" sep.vjc;vjc;byte;
        {0x501B, { 2, L_FIX} }, // "sprmSLnnMin" sep.lnnMin;lnn;word;
        {0x501C, { 2, L_FIX} }, // "sprmSPgnStart" sep.pgnStart;pgn;word;
        {0x301D, { 1, L_FIX} }, // "sprmSBOrientation" sep.dmOrientPage;dm;byte;
        {0x301E, { 1, L_FIX} }, // "sprmSBCustomize" ;;;
        {0xB01F, { 2, L_FIX} }, // "sprmSXaPage" sep.xaPage;xa;word;
        {0xB020, { 2, L_FIX} }, // "sprmSYaPage" sep.yaPage;ya;word;
        {0xB021, { 2, L_FIX} }, // "sprmSDxaLeft" sep.dxaLeft;dxa;word;
        {0xB022, { 2, L_FIX} }, // "sprmSDxaRight" sep.dxaRight;dxa;word;
        {0x9023, { 2, L_FIX} }, // "sprmSDyaTop" sep.dyaTop;dya;word;
        {0x9024, { 2, L_FIX} }, // "sprmSDyaBottom" sep.dyaBottom;dya;word;
        {0xB025, { 2, L_FIX} }, // "sprmSDzaGutter" sep.dzaGutter;dza;word;
        {0x5026, { 2, L_FIX} }, // "sprmSDmPaperReq" sep.dmPaperReq;dm;word;
        {0xD227, { 0, L_VAR} }, // "sprmSPropRMark" sep.fPropRMark,
                            // sep.ibstPropRMark, sep.dttmPropRMark ;complex
        {0x3228, { 1, L_FIX} }, // "sprmSFBiDi" ;;;
        {0x3229, { 1, L_FIX} }, // "sprmSFFacingCol" ;;;
        {0x322A, { 1, L_FIX} }, // "sprmSFRTLGutter", set to one if gutter is on
                            // right
        {0x702B, { 4, L_FIX} }, // "sprmSBrcTop80" sep.brcTop;BRC;long;
        {0x702C, { 4, L_FIX} }, // "sprmSBrcLeft80" sep.brcLeft;BRC;long;
        {0x702D, { 4, L_FIX} }, // "sprmSBrcBottom80" sep.brcBottom;BRC;long;
        {0x702E, { 4, L_FIX} }, // "sprmSBrcRight80" sep.brcRight;BRC;long;
        {0x522F, { 2, L_FIX} }, // "sprmSPgbProp" sep.pgbProp;;word;
        {0x7030, { 4, L_FIX} }, // "sprmSDxtCharSpace" sep.dxtCharSpace;dxt;long;
        {0x9031, { 2, L_FIX} }, // "sprmSDyaLinePitch"
                            // sep.dyaLinePitch;dya; WRONG:long; RIGHT:short; !
        {0x5032, { 2, L_FIX} }, // "sprmSClm" ;;;
        {0x5033, { 2, L_FIX} }, // "sprmSTextFlow" sep.wTextFlow;complex
        {0x5400, { 2, L_FIX} }, // "sprmTJc90" tap.jc;jc;word (low order byte is
                            // significant);
        {0x9601, { 2, L_FIX} }, // "sprmTDxaLeft" tap.rgdxaCenter
        {0x9602, { 2, L_FIX} }, // "sprmTDxaGapHalf" tap.dxaGapHalf,
                            // tap.rgdxaCenter
        {0x3403, { 1, L_FIX} }, // "sprmTFCantSplit" tap.fCantSplit;1 or 0;byte;
        {0x3404, { 1, L_FIX} }, // "sprmTTableHeader" tap.fTableHeader;1 or 0;byte;
        {0x3466, { 1, L_FIX} }, // "sprmTFCantSplit90" tap.fCantSplit90;1 or 0;byte;
        {0xD605, { 0, L_VAR} }, // "sprmTTableBorders80" tap.rgbrcTable;complex
        {0xD606, { 0, L_VAR} }, // "sprmTDefTable10" tap.rgdxaCenter,
                            // tap.rgtc;complex
        {0x9407, { 2, L_FIX} }, // "sprmTDyaRowHeight" tap.dyaRowHeight;dya;word;
        {0xD608, { 0, L_VAR} }, // "sprmTDefTable" tap.rgtc;complex
        {0xD609, { 0, L_VAR} }, // "sprmTDefTableShd80" tap.rgshd;complex
        {0x740A, { 4, L_FIX} }, // "sprmTTlp" tap.tlp;TLP;4 bytes;
        {0x560B, { 2, L_FIX} }, // "sprmTFBiDi" ;;;
        {0x740C, { 1, L_FIX} }, // "sprmTHTMLProps" ;;;
        {0xD620, { 0, L_VAR} }, // "sprmTSetBrc80" tap.rgtc[].rgbrc;complex
        {0x7621, { 4, L_FIX} }, // "sprmTInsert" tap.rgdxaCenter, tap.rgtc;complex
        {0x5622, { 2, L_FIX} }, // "sprmTDelete" tap.rgdxaCenter, tap.rgtc;complex
        {0x7623, { 4, L_FIX} }, // "sprmTDxaCol" tap.rgdxaCenter;complex
        {0x5624, { 0, L_VAR} }, // "sprmTMerge" tap.fFirstMerged, tap.fMerged;
        {0x5625, { 0, L_VAR} }, // "sprmTSplit" tap.fFirstMerged, tap.fMerged;
        {0xD626, { 0, L_VAR} }, // "sprmTSetBrc10" tap.rgtc[].rgbrc;complex
        {0x7627, { 0, L_VAR} }, // "sprmTSetShd80" tap.rgshd;complex
        {0x7628, { 0, L_VAR} }, // "sprmTSetShdOdd80" tap.rgshd;complex
        {0x7629, { 4, L_FIX} }, // "sprmTTextFlow" tap.rgtc[].fVerticaltap,
                            // rgtc[].fBackwardtap, rgtc[].fRotateFont;0 or 10
                            // or 10 or 1;word;
        {0xD62A, { 1, L_FIX} }, // "sprmTDiagLine" ;;;
        {0xD62B, { 0, L_VAR} }, // "sprmTVertMerge" tap.rgtc[].vertMerge
        {0xD62C, { 0, L_VAR} }, // "sprmTVertAlign" tap.rgtc[].vertAlign
        {NS_sprm::LN_CFELayout, { 0, L_VAR} },
        {0x6649, { 4, L_FIX} }, // undocumented
        {0xF614, { 3, L_FIX} }, // undocumented
        {0xD612, { 0, L_VAR} }, // "sprmTDefTableShd"
        {0xD613, { 0, L_VAR} }, // "sprmTTableBorders"
        {0xD61A, { 0, L_VAR} }, // undocumented
        {0xD61B, { 0, L_VAR} }, // undocumented
        {0xD61C, { 0, L_VAR} }, // undocumented
        {0xD61D, { 0, L_VAR} }, // undocumented
        {0xD632, { 0, L_VAR} }, // undocumented
        {0xD634, { 0, L_VAR} }, // undocumented
        {0xD238, { 0, L_VAR} }, // undocumented sep
        {0xC64E, { 0, L_VAR} }, // "sprmPBrcTop"
        {0xC64F, { 0, L_VAR} }, // "sprmPBrcLeft"
        {0xC650, { 0, L_VAR} }, // "sprmPBrcBottom"
        {0xC651, { 0, L_VAR} }, // "sprmPBrcRight"
        {0xC652, { 0, L_VAR} }, // "sprmPBrcBetween"
        {0xF661, { 3, L_FIX} }, // undocumented
        {0x4873, { 2, L_FIX} }, // "sprmCRgLid0" chp.rglid[0];LID: for non-FE text
        {0x4874, { 2, L_FIX} }, // "sprmCRgLid1" chp.rglid[1];LID: for Far East text
        {0x6463, { 4, L_FIX} }, // undocumented
        {0x2461, { 1, L_FIX} }, // undoc, must be asian version of "sprmPJc"
        {0x845D, { 2, L_FIX} }, // undoc, must be asian version of "sprmPDxaRight"
        {0x845E, { 2, L_FIX} }, // undoc, must be asian version of "sprmPDxaLeft"
        {0x8460, { 2, L_FIX} }, // undoc, must be asian version of "sprmPDxaLeft1"
        {0x3615, { 1, L_FIX} }, // undocumented
        {0x360D, { 1, L_FIX} }, // undocumented
        {0x703A, { 4, L_FIX} }, // undocumented, sep, perhaps related to textgrids ?
        {0x303B, { 1, L_FIX} }, // undocumented, sep
        {0x244B, { 1, L_FIX} }, // undocumented, subtable "sprmPFInTable" equiv ?
        {0x244C, { 1, L_FIX} }, // undocumented, subtable "sprmPFTtp" equiv ?
        {0x940E, { 2, L_FIX} }, // undocumented
        {0x940F, { 2, L_FIX} }, // undocumented
        {0x9410, { 2, L_FIX} }, // undocumented
        {0x6815, { 4, L_FIX} }, // undocumented
        {0x6816, { 4, L_FIX} }, // undocumented
        {NS_sprm::LN_CCv, { 4, L_FIX} }, // text colour
        {0xC64D, { 0, L_VAR} }, // undocumented, para back colour
        {0x6467, { 4, L_FIX} }, // undocumented
        {0x646B, { 4, L_FIX} }, // undocumented
        {0xF617, { 3, L_FIX} }, // undocumented
        {0xD660, { 0, L_VAR} }, // undocumented, something to do with colour.
        {0xD670, { 0, L_VAR} }, // undocumented, something to do with colour.
        {0xCA71, { 0, L_VAR} }, // "sprmCShd", text backcolour
        {0x303C, { 1, L_FIX} }, // undocumented, sep
        {0x245B, { 1, L_FIX} }, // undocumented, para autobefore
        {0x245C, { 1, L_FIX} }, // undocumented, para autoafter
        // "sprmPFContextualSpacing", don't add space between para of the same style
        {0x246D, { 1, L_FIX} }
    };

    static wwSprmSearcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
};

wwSprmParser::wwSprmParser(ww::WordVersion eVersion) : meVersion(eVersion)
{
   OSL_ENSURE((meVersion >= ww::eWW1 && meVersion <= ww::eWW8),
        "Impossible value for version");

    mnDelta = (ww::IsSevenMinus(meVersion)) ? 0 : 1;

    if (meVersion <= ww::eWW2)
        mpKnownSprms = GetWW2SprmSearcher();
    else if (meVersion < ww::eWW8)
        mpKnownSprms = GetWW6SprmSearcher();
    else
        mpKnownSprms = GetWW8SprmSearcher();
}

SprmInfo wwSprmParser::GetSprmInfo(sal_uInt16 nId) const
{
    const SprmInfo* pFound = mpKnownSprms->search(nId);
    if (pFound != nullptr)
    {
        return *pFound;
    }

    OSL_ENSURE(ww::IsEightPlus(meVersion),
               "Unknown ww7- sprm, dangerous, report to development");

    //All the unknown ww7 sprms appear to be variable (which makes sense)
    SprmInfo aSrch = { 0, L_VAR };
    if (ww::IsEightPlus(meVersion)) //We can recover perfectly in this case
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
    return aSrch;
}

//-end

inline sal_uInt8 Get_Byte( sal_uInt8 *& p )
{
    sal_uInt8 n = *p;
    p += 1;
    return n;
}

inline sal_uInt16 Get_UShort( sal_uInt8 *& p )
{
    const sal_uInt16 n = SVBT16ToShort( *reinterpret_cast<SVBT16*>(p) );
    p += 2;
    return n;
}

inline short Get_Short( sal_uInt8 *& p )
{
    return Get_UShort(p);
}

inline sal_uLong Get_ULong( sal_uInt8 *& p )
{
    sal_uLong n = SVBT32ToUInt32( *reinterpret_cast<SVBT32*>(p) );
    p += 4;
    return n;
}

inline long Get_Long( sal_uInt8 *& p )
{
    return Get_ULong(p);
}

WW8SprmIter::WW8SprmIter(const sal_uInt8* pSprms_, long nLen_,
    const wwSprmParser &rParser)
    :  mrSprmParser(rParser), pSprms( pSprms_), nRemLen( nLen_)
{
    UpdateMyMembers();
}

void WW8SprmIter::SetSprms(const sal_uInt8* pSprms_, long nLen_)
{
    pSprms = pSprms_;
    nRemLen = nLen_;
    UpdateMyMembers();
}

void WW8SprmIter::advance()
{
    if (nRemLen > 0 )
    {
        sal_uInt16 nSize = nAktSize;
        if (nSize > nRemLen)
            nSize = nRemLen;
        pSprms += nSize;
        nRemLen -= nSize;
        UpdateMyMembers();
    }
}

void WW8SprmIter::UpdateMyMembers()
{
    bool bValid = (pSprms && nRemLen >= mrSprmParser.MinSprmLen());

    if (bValid)
    {
        nAktId = mrSprmParser.GetSprmId(pSprms);
        nAktSize = mrSprmParser.GetSprmSize(nAktId, pSprms);
        pAktParams = pSprms + mrSprmParser.DistanceToData(nAktId);
        bValid = nAktSize <= nRemLen;
        SAL_WARN_IF(!bValid, "sw.ww8", "sprm longer than remaining bytes, doc or parser is wrong");
    }

    if (!bValid)
    {
        nAktId = 0;
        pAktParams = nullptr;
        nAktSize = 0;
        nRemLen = 0;
    }
}

const sal_uInt8* WW8SprmIter::FindSprm(sal_uInt16 nId)
{
    while (GetSprms())
    {
        if (GetAktId() == nId)
            return GetAktParams();              // SPRM found!
        advance();
    }

    return nullptr;                                   // SPRM _not_ found
}

// temporary test
// WW8PLCFx_PCDAttrs cling to WW8PLCF_Pcd and therefore do not have their own iterators.
// All methods relating to iterators are therefore dummies.
WW8PLCFx_PCDAttrs::WW8PLCFx_PCDAttrs(ww::WordVersion eVersion,
    WW8PLCFx_PCD* pPLCFx_PCD, const WW8ScannerBase* pBase)
    : WW8PLCFx(eVersion, true), pPcdI(pPLCFx_PCD->GetPLCFIter()),
    pPcd(pPLCFx_PCD), pGrpprls(pBase->aPieceGrpprls.data()),
    nGrpprls(pBase->aPieceGrpprls.size())
{
}

sal_uInt32 WW8PLCFx_PCDAttrs::GetIdx() const
{
    return 0;
}

void WW8PLCFx_PCDAttrs::SetIdx( sal_uLong )
{
}

bool WW8PLCFx_PCDAttrs::SeekPos(WW8_CP )
{
    return true;
}

void WW8PLCFx_PCDAttrs::advance()
{
}

WW8_CP WW8PLCFx_PCDAttrs::Where()
{
    return ( pPcd ) ? pPcd->Where() : WW8_CP_MAX;
}

void WW8PLCFx_PCDAttrs::GetSprms(WW8PLCFxDesc* p)
{
    void* pData;

    p->bRealLineEnd = false;
    if ( !pPcdI || !pPcdI->Get(p->nStartPos, p->nEndPos, pData) )
    {
        // PLCF fully processed
        p->nStartPos = p->nEndPos = WW8_CP_MAX;
        p->pMemPos = nullptr;
        p->nSprmsLen = 0;
        return;
    }

    const sal_uInt16 nPrm = SVBT16ToShort( static_cast<WW8_PCD*>(pData)->prm );
    if ( nPrm & 1 )
    {
        // PRM Variant 2
        const sal_uInt16 nSprmIdx = nPrm >> 1;

        if( nSprmIdx >= nGrpprls )
        {
            // Invalid Index
            p->nStartPos = p->nEndPos = WW8_CP_MAX;
            p->pMemPos = nullptr;
            p->nSprmsLen = 0;
            return;
        }
        const sal_uInt8* pSprms = pGrpprls[ nSprmIdx ];

        p->nSprmsLen = SVBT16ToShort( pSprms ); // Length
        pSprms += 2;
        p->pMemPos = pSprms;                    // Position
    }
    else
    {
        // SPRM is stored directly into members var
        /*
            These are the attr that are in the piece-table instead of in the text!
        */

        if (IsSevenMinus(GetFIBVersion()))
        {
            aShortSprm[0] = (sal_uInt8)( ( nPrm & 0xfe) >> 1 );
            aShortSprm[1] = (sal_uInt8)(   nPrm         >> 8 );
            p->nSprmsLen = ( nPrm ) ? 2 : 0;        // length

            // store Position of internal mini storage in Data Pointer
            p->pMemPos = aShortSprm;
        }
        else
        {
            p->pMemPos = nullptr;
            p->nSprmsLen = 0;
            sal_uInt8 nSprmListIdx = (sal_uInt8)((nPrm & 0xfe) >> 1);
            if( nSprmListIdx )
            {
                // process Sprm Id Matching as explained in MS Documentation

                // ''Property Modifier(variant 1) (PRM)''
                // see file: s62f39.htm

                // Since Sprm is 7 bits, rgsprmPrm can hold 0x80 entries.
                static const sal_uInt16 aSprmId[0x80] =
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
                    // sprmNoop, sprmCFStrikeRM, sprmCFRMark, sprmCFFieldVanish
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
                const sal_uInt16 nSprmId = aSprmId[ nSprmListIdx ];

                if( nSprmId )
                {
                    // move Sprm Id and Sprm Param to internal mini storage:
                    aShortSprm[0] = (sal_uInt8)( ( nSprmId & 0x00ff)      );
                    aShortSprm[1] = (sal_uInt8)( ( nSprmId & 0xff00) >> 8 );
                    aShortSprm[2] = (sal_uInt8)( nPrm >> 8 );

                    // store Sprm Length in member:
                    p->nSprmsLen = ( nPrm ) ? 3 : 0;

                    // store Position of internal mini storage in Data Pointer
                    p->pMemPos = aShortSprm;
                }
            }
        }
    }
}

WW8PLCFx_PCD::WW8PLCFx_PCD(ww::WordVersion eVersion, WW8PLCFpcd* pPLCFpcd,
    WW8_CP nStartCp, bool bVer67P)
    : WW8PLCFx(eVersion, false), nClipStart(-1)
{
    // construct own iterator
    pPcdI = new WW8PLCFpcd_Iter(*pPLCFpcd, nStartCp);
    bVer67= bVer67P;
}

WW8PLCFx_PCD::~WW8PLCFx_PCD()
{
    // pPcd-Dtor which in called from WW8ScannerBase
    delete pPcdI;
}

sal_uLong WW8PLCFx_PCD::GetIMax() const
{
    return pPcdI ? pPcdI->GetIMax() : 0;
}

sal_uInt32 WW8PLCFx_PCD::GetIdx() const
{
    return pPcdI ? pPcdI->GetIdx() : 0;
}

void WW8PLCFx_PCD::SetIdx( sal_uLong nIdx )
{
    if (pPcdI)
        pPcdI->SetIdx( nIdx );
}

bool WW8PLCFx_PCD::SeekPos(WW8_CP nCpPos)
{
    return pPcdI && pPcdI->SeekPos( nCpPos );
}

WW8_CP WW8PLCFx_PCD::Where()
{
    return pPcdI ? pPcdI->Where() : WW8_CP_MAX;
}

long WW8PLCFx_PCD::GetNoSprms( WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen )
{
    void* pData;
    rLen = 0;

    if ( !pPcdI || !pPcdI->Get(rStart, rEnd, pData) )
    {
        rStart = rEnd = WW8_CP_MAX;
        return -1;
    }
    return pPcdI->GetIdx();
}

void WW8PLCFx_PCD::advance()
{
    OSL_ENSURE(pPcdI , "missing pPcdI");
    if (pPcdI)
        pPcdI->advance();
}

WW8_FC WW8PLCFx_PCD::AktPieceStartCp2Fc( WW8_CP nCp )
{
    WW8_CP nCpStart, nCpEnd;
    void* pData;

    if ( !pPcdI->Get(nCpStart, nCpEnd, pData) )
    {
        OSL_ENSURE( false, "AktPieceStartCp2Fc() with false Cp found (1)" );
        return WW8_FC_MAX;
    }

    OSL_ENSURE( nCp >= nCpStart && nCp < nCpEnd,
        "AktPieceCp2Fc() with false Cp found (2)" );

    if( nCp < nCpStart )
        nCp = nCpStart;
    if( nCp >= nCpEnd )
        nCp = nCpEnd - 1;

    bool bIsUnicode = false;
    WW8_FC nFC = SVBT32ToUInt32( static_cast<WW8_PCD*>(pData)->fc );
    if( !bVer67 )
        nFC = WW8PLCFx_PCD::TransformPieceAddress( nFC, bIsUnicode );

    return nFC + (nCp - nCpStart) * (bIsUnicode ? 2 : 1);
}

void WW8PLCFx_PCD::AktPieceFc2Cp( WW8_CP& rStartPos, WW8_CP& rEndPos,
    const WW8ScannerBase *pSBase )
{
    //No point going anywhere with this
    if ((rStartPos == WW8_CP_MAX) && (rEndPos == WW8_CP_MAX))
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
        OSL_ENSURE( false, "AktPieceStartFc2Cp() - error" );
        return WW8_CP_MAX;
    }
    bool bIsUnicode = false;
    sal_Int32 nFcStart  = SVBT32ToUInt32( static_cast<WW8_PCD*>(pData)->fc );
    if( !bVer67 )
        nFcStart = WW8PLCFx_PCD::TransformPieceAddress( nFcStart, bIsUnicode );

    sal_Int32 nUnicodeFactor = bIsUnicode ? 2 : 1;

    if( nStartPos < nFcStart )
        nStartPos = nFcStart;

    if( nStartPos >= nFcStart + (nCpEnd - nCpStart)     * nUnicodeFactor )
        nStartPos  = nFcStart + (nCpEnd - nCpStart - 1) * nUnicodeFactor;

    return nCpStart + (nStartPos - nFcStart) / nUnicodeFactor;
}

//      Helper routines for all

// Convert BRC from WW6 to WW8 format
WW8_BRC::WW8_BRC(const WW8_BRCVer6& brcVer6)
{
    sal_uInt8 _dptLineWidth = brcVer6.dxpLineWidth(),
              _brcType = brcVer6.brcType();

    if (_dptLineWidth > 5) // this signifies dashed(6) or dotted(7) line
    {
        _brcType = _dptLineWidth;
        _dptLineWidth = 1;
    }
    _dptLineWidth *= 6; // convert units from 0.75pt to 1/8pt

    *this = WW8_BRC(_dptLineWidth, _brcType, brcVer6.ico(), brcVer6.dxpSpace(),
        brcVer6.fShadow(), false);
}

// Convert BRC from WW8 to WW9 format
WW8_BRCVer9::WW8_BRCVer9(const WW8_BRC& brcVer8)
{
    if (brcVer8.isNil()) {
        UInt32ToSVBT32(0, aBits1);
        UInt32ToSVBT32(0xffffffff, aBits2);
    }
    else
    {
        sal_uInt32 _cv = brcVer8.ico() == 0 ? 0xff000000 // "auto" colour
            : wwUtility::RGBToBGR(SwWW8ImplReader::GetCol(brcVer8.ico()));
        *this = WW8_BRCVer9(_cv, brcVer8.dptLineWidth(), brcVer8.brcType(),
            brcVer8.dptSpace(), brcVer8.fShadow(), brcVer8.fFrame());
    }
}

short WW8_BRC::DetermineBorderProperties(short *pSpace) const
{
    WW8_BRCVer9 brcVer9(*this);
    return brcVer9.DetermineBorderProperties(pSpace);
}

short WW8_BRCVer9::DetermineBorderProperties(short *pSpace) const
{
    /*
        Word does not factor the width of the border into the width/height
        stored in the information for graphic/table/object widths, so we need
        to figure out this extra width here and utilize the returned size in
        our calculations
    */
    short nMSTotalWidth;

    //Specification in 8ths of a point, 1 Point = 20 Twips, so by 2.5
    nMSTotalWidth  = (short)dptLineWidth() * 20 / 8;

    //Figure out the real size of the border according to word
    switch (brcType())
    {
        //Note that codes over 25 are undocumented, and I can't create
        //these 4 here in the wild.
        case 2:
        case 4:
        case 5:
        case 22:
            OSL_FAIL("Can't create these from the menus, please report");
        default:
        case 23:    //Only 3pt in the menus, but honours the size setting.
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
    }

    if (pSpace)
        *pSpace = (short)dptSpace() * 20; // convert from points to twips
    return nMSTotalWidth;
}

/*
 * WW8Cp2Fc is a good method, a CP always maps to a FC
 * WW8Fc2Cp on the other hand is more dubious, a random FC
 * may not map to a valid CP. Try and avoid WW8Fc2Cp where
 * possible
 */
WW8_CP WW8ScannerBase::WW8Fc2Cp( WW8_FC nFcPos ) const
{
    WW8_CP nFallBackCpEnd = WW8_CP_MAX;
    if( nFcPos == WW8_FC_MAX )
        return nFallBackCpEnd;

    bool bIsUnicode;
    if (pWw8Fib->nVersion >= 8)
        bIsUnicode = false;
    else
        bIsUnicode = pWw8Fib->fExtChar;

    if( pPieceIter )    // Complex File ?
    {
        sal_uLong nOldPos = pPieceIter->GetIdx();

        for (pPieceIter->SetIdx(0);
            pPieceIter->GetIdx() < pPieceIter->GetIMax(); pPieceIter->advance())
        {
            WW8_CP nCpStart, nCpEnd;
            void* pData;
            if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
            {   // outside PLCFfpcd ?
                OSL_ENSURE( false, "PLCFpcd-WW8Fc2Cp() went wrong" );
                break;
            }
            sal_Int32 nFcStart  = SVBT32ToUInt32( static_cast<WW8_PCD*>(pData)->fc );
            if (pWw8Fib->nVersion >= 8)
            {
                nFcStart = WW8PLCFx_PCD::TransformPieceAddress( nFcStart,
                                                                bIsUnicode );
            }
            else
            {
                bIsUnicode = pWw8Fib->fExtChar;
            }
            sal_Int32 nLen = (nCpEnd - nCpStart) * (bIsUnicode ? 2 : 1);

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
    if (!bIsUnicode)
        nFallBackCpEnd = (nFcPos - pWw8Fib->fcMin);
    else
        nFallBackCpEnd = (nFcPos - pWw8Fib->fcMin + 1) / 2;

    return nFallBackCpEnd;
}

WW8_FC WW8ScannerBase::WW8Cp2Fc(WW8_CP nCpPos, bool* pIsUnicode,
    WW8_CP* pNextPieceCp, bool* pTestFlag) const
{
    if( pTestFlag )
        *pTestFlag = true;
    if( WW8_CP_MAX == nCpPos )
        return WW8_CP_MAX;

    bool bIsUnicode;
    if( !pIsUnicode )
        pIsUnicode = &bIsUnicode;

    if (pWw8Fib->nVersion >= 8)
        *pIsUnicode = false;
    else
        *pIsUnicode = pWw8Fib->fExtChar;

    if( pPieceIter )
    {
        // Complex File
        if( pNextPieceCp )
            *pNextPieceCp = WW8_CP_MAX;

        if( !pPieceIter->SeekPos( nCpPos ) )
        {
            if( pTestFlag )
                *pTestFlag = false;
            else {
                OSL_ENSURE( false, "Handed over wrong CP to WW8Cp2Fc()" );
            }
            return WW8_FC_MAX;
        }
        WW8_CP nCpStart, nCpEnd;
        void* pData;
        if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
        {
            if( pTestFlag )
                *pTestFlag = false;
            else {
                OSL_ENSURE( false, "PLCFfpcd-Get went wrong" );
            }
            return WW8_FC_MAX;
        }
        if( pNextPieceCp )
            *pNextPieceCp = nCpEnd;

        WW8_FC nRet = SVBT32ToUInt32( static_cast<WW8_PCD*>(pData)->fc );
        if (pWw8Fib->nVersion >= 8)
            nRet = WW8PLCFx_PCD::TransformPieceAddress( nRet, *pIsUnicode );
        else
            *pIsUnicode = pWw8Fib->fExtChar;

        nRet += (nCpPos - nCpStart) * (*pIsUnicode ? 2 : 1);

        return nRet;
    }

    // No complex file
    return pWw8Fib->fcMin + nCpPos * (*pIsUnicode ? 2 : 1);
}

//      class WW8ScannerBase
WW8PLCFpcd* WW8ScannerBase::OpenPieceTable( SvStream* pStr, const WW8Fib* pWwF )
{
    if ( ((8 > pWw8Fib->nVersion) && !pWwF->fComplex) || !pWwF->lcbClx )
        return nullptr;

    WW8_FC nClxPos = pWwF->fcClx;

    if (!checkSeek(*pStr, nClxPos))
        return nullptr;

    sal_Int32 nClxLen = pWwF->lcbClx;
    sal_Int32 nLeft = nClxLen;

    while (true)
    {
        sal_uInt8 clxt(2);
        pStr->ReadUChar( clxt );
        nLeft--;
        if( 2 == clxt)                          // PLCFfpcd ?
            break;                              // PLCFfpcd found
        sal_uInt16 nLen(0);
        pStr->ReadUInt16( nLen );
        nLeft -= 2 + nLen;
        if( nLeft < 0 )
            return nullptr;                        // gone wrong
        if( 1 == clxt )                         // clxtGrpprl ?
        {
            if (aPieceGrpprls.size() == SHRT_MAX)
                return nullptr;
            if (nLen > pStr->remainingSize())
                return nullptr;
            sal_uInt8* p = new sal_uInt8[nLen+2];         // allocate
            ShortToSVBT16(nLen, p);             // add length
            if (!checkRead(*pStr, p+2, nLen))   // read grpprl
            {
                delete[] p;
                return nullptr;
            }
            aPieceGrpprls.push_back(p);    // add to array
        }
        else
            pStr->SeekRel( nLen );         // non-Grpprl left
    }

    // read Piece Table PLCF
    sal_Int32 nPLCFfLen(0);
    if (pWwF->GetFIBVersion() <= ww::eWW2)
    {
        sal_Int16 nWordTwoLen(0);
        pStr->ReadInt16( nWordTwoLen );
        nPLCFfLen = nWordTwoLen;
    }
    else
        pStr->ReadInt32( nPLCFfLen );
    OSL_ENSURE( 65536 > nPLCFfLen, "PLCFfpcd above 64 k" );
    return new WW8PLCFpcd( pStr, pStr->Tell(), nPLCFfLen, 8 );
}

void WW8ScannerBase::DeletePieceTable()
{
    for (auto pGrppl : aPieceGrpprls)
        delete[] pGrppl;
}

WW8ScannerBase::WW8ScannerBase( SvStream* pSt, SvStream* pTableSt,
    SvStream* pDataSt, WW8Fib* pWwFib )
    : pWw8Fib(pWwFib), pMainFdoa(nullptr), pHdFtFdoa(nullptr), pMainTxbx(nullptr),
    pMainTxbxBkd(nullptr), pHdFtTxbx(nullptr), pHdFtTxbxBkd(nullptr), pMagicTables(nullptr),
    pSubdocs(nullptr), pExtendedAtrds(nullptr)
{
    pPiecePLCF = OpenPieceTable( pTableSt, pWw8Fib );             // Complex
    if( pPiecePLCF )
    {
        pPieceIter = new WW8PLCFpcd_Iter( *pPiecePLCF );
        pPLCFx_PCD = new WW8PLCFx_PCD(pWwFib->GetFIBVersion(), pPiecePLCF, 0,
            IsSevenMinus(pWw8Fib->GetFIBVersion()));
        pPLCFx_PCDAttrs = new WW8PLCFx_PCDAttrs(pWwFib->GetFIBVersion(),
            pPLCFx_PCD, this);
    }
    else
    {
        pPieceIter = nullptr;
        pPLCFx_PCD = nullptr;
        pPLCFx_PCDAttrs = nullptr;
    }

    // pChpPLCF and pPapPLCF may NOT be created before pPLCFx_PCD !!
    pChpPLCF = new WW8PLCFx_Cp_FKP( pSt, pTableSt, pDataSt, *this, CHP ); // CHPX
    pPapPLCF = new WW8PLCFx_Cp_FKP( pSt, pTableSt, pDataSt, *this, PAP ); // PAPX

    pSepPLCF = new WW8PLCFx_SEPX(   pSt, pTableSt, *pWwFib, 0 );          // SEPX

    // Footnotes
    pFootnotePLCF = new WW8PLCFx_SubDoc( pTableSt, pWwFib->GetFIBVersion(), 0,
        pWwFib->fcPlcffndRef, pWwFib->lcbPlcffndRef, pWwFib->fcPlcffndText,
        pWwFib->lcbPlcffndText, 2 );
    // Endnotes
    pEdnPLCF = new WW8PLCFx_SubDoc( pTableSt, pWwFib->GetFIBVersion(), 0,
        pWwFib->fcPlcfendRef, pWwFib->lcbPlcfendRef, pWwFib->fcPlcfendText,
        pWwFib->lcbPlcfendText, 2 );
    // Comments
    pAndPLCF = new WW8PLCFx_SubDoc( pTableSt, pWwFib->GetFIBVersion(), 0,
        pWwFib->fcPlcfandRef, pWwFib->lcbPlcfandRef, pWwFib->fcPlcfandText,
        pWwFib->lcbPlcfandText, IsSevenMinus(pWwFib->GetFIBVersion()) ? 20 : 30);

    // Fields Main Text
    pFieldPLCF    = new WW8PLCFx_FLD(pTableSt, *pWwFib, MAN_MAINTEXT);
    // Fields Header / Footer
    pFieldHdFtPLCF= new WW8PLCFx_FLD(pTableSt, *pWwFib, MAN_HDFT);
    // Fields Footnote
    pFieldFootnotePLCF = new WW8PLCFx_FLD(pTableSt, *pWwFib, MAN_FTN);
    // Fields Endnote
    pFieldEdnPLCF = new WW8PLCFx_FLD(pTableSt, *pWwFib, MAN_EDN);
    // Fields Comments
    pFieldAndPLCF = new WW8PLCFx_FLD(pTableSt, *pWwFib, MAN_AND);
    // Fields in Textboxes in Main Text
    pFieldTxbxPLCF= new WW8PLCFx_FLD(pTableSt, *pWwFib, MAN_TXBX);
    // Fields in Textboxes in Header / Footer
    pFieldTxbxHdFtPLCF = new WW8PLCFx_FLD(pTableSt,*pWwFib,MAN_TXBX_HDFT);

    // Note: 6 stands for "6 OR 7",  7 stands for "ONLY 7"
    switch( pWw8Fib->nVersion )
    {
        case 6:
        case 7:
            if( pWwFib->fcPlcfdoaMom && pWwFib->lcbPlcfdoaMom )
            {
                pMainFdoa = new WW8PLCFspecial( pTableSt, pWwFib->fcPlcfdoaMom,
                    pWwFib->lcbPlcfdoaMom, 6 );
            }
            if( pWwFib->fcPlcfdoaHdr && pWwFib->lcbPlcfdoaHdr )
            {
                pHdFtFdoa = new WW8PLCFspecial( pTableSt, pWwFib->fcPlcfdoaHdr,
                pWwFib->lcbPlcfdoaHdr, 6 );
            }
            break;
        case 8:
            if( pWwFib->fcPlcfspaMom && pWwFib->lcbPlcfspaMom )
            {
                pMainFdoa = new WW8PLCFspecial( pTableSt, pWwFib->fcPlcfspaMom,
                    pWwFib->lcbPlcfspaMom, 26 );
            }
            if( pWwFib->fcPlcfspaHdr && pWwFib->lcbPlcfspaHdr )
            {
                pHdFtFdoa = new WW8PLCFspecial( pTableSt, pWwFib->fcPlcfspaHdr,
                    pWwFib->lcbPlcfspaHdr, 26 );
            }
            // PLCF for TextBox break-descriptors in the main text
            if( pWwFib->fcPlcftxbxBkd && pWwFib->lcbPlcftxbxBkd )
            {
                pMainTxbxBkd = new WW8PLCFspecial( pTableSt,
                    pWwFib->fcPlcftxbxBkd, pWwFib->lcbPlcftxbxBkd, 0);
            }
            // PLCF for TextBox break-descriptors in Header/Footer range
            if( pWwFib->fcPlcfHdrtxbxBkd && pWwFib->lcbPlcfHdrtxbxBkd )
            {
                pHdFtTxbxBkd = new WW8PLCFspecial( pTableSt,
                    pWwFib->fcPlcfHdrtxbxBkd, pWwFib->lcbPlcfHdrtxbxBkd, 0);
            }
            // Sub table cp positions
            if (pWwFib->fcPlcfTch && pWwFib->lcbPlcfTch)
            {
                pMagicTables = new WW8PLCFspecial( pTableSt,
                    pWwFib->fcPlcfTch, pWwFib->lcbPlcfTch, 4);
            }
            // Sub document cp positions
            if (pWwFib->fcPlcfwkb && pWwFib->lcbPlcfwkb)
            {
                pSubdocs = new WW8PLCFspecial( pTableSt,
                    pWwFib->fcPlcfwkb, pWwFib->lcbPlcfwkb, 12);
            }
            // Extended ATRD
            if (pWwFib->fcAtrdExtra && pWwFib->lcbAtrdExtra)
            {
                sal_Size nOldPos = pTableSt->Tell();
                if (checkSeek(*pTableSt, pWwFib->fcAtrdExtra) && (pTableSt->remainingSize() >= pWwFib->lcbAtrdExtra))
                {
                    pExtendedAtrds = new sal_uInt8[pWwFib->lcbAtrdExtra];
                    pWwFib->lcbAtrdExtra = pTableSt->Read(pExtendedAtrds, pWwFib->lcbAtrdExtra);
                }
                else
                    pWwFib->lcbAtrdExtra = 0;
                pTableSt->Seek(nOldPos);
            }

            break;
        default:
            OSL_ENSURE( false, "nVersion not implemented!" );
            break;
    }

    // PLCF for TextBox stories in main text
    sal_uInt32 nLenTxBxS = (8 > pWw8Fib->nVersion) ? 0 : 22;
    if( pWwFib->fcPlcftxbxText && pWwFib->lcbPlcftxbxText )
    {
        pMainTxbx = new WW8PLCFspecial( pTableSt, pWwFib->fcPlcftxbxText,
            pWwFib->lcbPlcftxbxText, nLenTxBxS );
    }

    // PLCF for TextBox stories in Header/Footer range
    if( pWwFib->fcPlcfHdrtxbxText && pWwFib->lcbPlcfHdrtxbxText )
    {
        pHdFtTxbx = new WW8PLCFspecial( pTableSt, pWwFib->fcPlcfHdrtxbxText,
            pWwFib->lcbPlcfHdrtxbxText, nLenTxBxS );
    }

    pBook = new WW8PLCFx_Book(pTableSt, *pWwFib);
    pAtnBook = new WW8PLCFx_AtnBook(pTableSt, *pWwFib);
    pFactoidBook = new WW8PLCFx_FactoidBook(pTableSt, *pWwFib);
}

WW8ScannerBase::~WW8ScannerBase()
{
    DeletePieceTable();
    delete pPLCFx_PCDAttrs;
    delete pPLCFx_PCD;
    delete pPieceIter;
    delete pPiecePLCF;
    delete pFactoidBook;
    delete pAtnBook;
    delete pBook;
    delete pFieldEdnPLCF;
    delete pFieldFootnotePLCF;
    delete pFieldAndPLCF;
    delete pFieldHdFtPLCF;
    delete pFieldPLCF;
    delete pFieldTxbxPLCF;
    delete pFieldTxbxHdFtPLCF;
    delete pEdnPLCF;
    delete pFootnotePLCF;
    delete pAndPLCF;
    delete pSepPLCF;
    delete pPapPLCF;
    delete pChpPLCF;
    delete pMainFdoa;
    delete pHdFtFdoa;
    delete pMainTxbx;
    delete pMainTxbxBkd;
    delete pHdFtTxbx;
    delete pHdFtTxbxBkd;
    delete pMagicTables;
    delete pSubdocs;
    delete [] pExtendedAtrds;
}

// Fields

static bool WW8SkipField(WW8PLCFspecial& rPLCF)
{
    void* pData;
    WW8_CP nP;

    if (!rPLCF.Get(nP, pData))              // End of PLCFspecial?
        return false;

    rPLCF.advance();

    if((static_cast<sal_uInt8*>(pData)[0] & 0x1f ) != 0x13 )    // No beginning?
        return true;                            // Do not terminate on error

    if( !rPLCF.Get( nP, pData ) )
        return false;

    while((static_cast<sal_uInt8*>(pData)[0] & 0x1f ) == 0x13 )
    {
        // still new (nested) beginnings ?
        WW8SkipField( rPLCF );              // nested Field in description
        if( !rPLCF.Get( nP, pData ) )
            return false;
    }

    if((static_cast<sal_uInt8*>(pData)[0] & 0x1f ) == 0x14 )
    {

        // Field Separator ?
        rPLCF.advance();

        if( !rPLCF.Get( nP, pData ) )
            return false;

        while ((static_cast<sal_uInt8*>(pData)[0] & 0x1f ) == 0x13)
        {
            // still new (nested) beginnings?
            WW8SkipField( rPLCF );          // nested Field in Results
            if( !rPLCF.Get( nP, pData ) )
                return false;
        }
    }
    rPLCF.advance();

    return true;
}

static bool WW8GetFieldPara(WW8PLCFspecial& rPLCF, WW8FieldDesc& rF)
{
    void* pData;
    sal_uLong nOldIdx = rPLCF.GetIdx();

    rF.nLen = rF.nId = rF.nOpt = 0;
    rF.bCodeNest = rF.bResNest = false;

    if( !rPLCF.Get( rF.nSCode, pData ) )             // end of PLCFspecial?
        goto Err;

    rPLCF.advance();

    if (!pData || (static_cast<sal_uInt8*>(pData)[0] & 0x1f) != 0x13)        // No beginning?
        goto Err;

    rF.nId = static_cast<sal_uInt8*>(pData)[1];

    if( !rPLCF.Get( rF.nLCode, pData ) )
        goto Err;

    rF.nSRes = rF.nLCode;                           // Default
    rF.nSCode++;                                    // without markers
    rF.nLCode -= rF.nSCode;                         // Pos -> length

    while((static_cast<sal_uInt8*>(pData)[0] & 0x1f ) == 0x13 )
    {
        // still new (nested) beginnings ?
        WW8SkipField( rPLCF );              // nested Field in description
        rF.bCodeNest = true;
        if( !rPLCF.Get( rF.nSRes, pData ) )
            goto Err;
    }

    if ((static_cast<sal_uInt8*>(pData)[0] & 0x1f ) == 0x14 )       // Field Separator?
    {
        rPLCF.advance();

        if( !rPLCF.Get( rF.nLRes, pData ) )
            goto Err;

        while((static_cast<sal_uInt8*>(pData)[0] & 0x1f ) == 0x13 )
        {
            // still new (nested) beginnings ?
            WW8SkipField( rPLCF );              // nested Field in results
            rF.bResNest = true;
            if( !rPLCF.Get( rF.nLRes, pData ) )
                goto Err;
        }
        rF.nLen = rF.nLRes - rF.nSCode + 2;         // nLRes is still the final position
        rF.nLRes -= rF.nSRes;                       // now: nLRes = length
        rF.nSRes++;                                 // Endpos including Markers
        rF.nLRes--;
    }else{
        rF.nLRes = 0;                               // no result found
        rF.nLen = rF.nSRes - rF.nSCode + 2;         // total length
    }

    rPLCF.advance();
    if((static_cast<sal_uInt8*>(pData)[0] & 0x1f ) == 0x15 )
    {
        // Field end ?
        // INDEX-Field has set Bit7?
        rF.nOpt = static_cast<sal_uInt8*>(pData)[1];                // yes -> copy flags
    }else{
        rF.nId = 0;                                      // no -> Field invalid
    }

    rPLCF.SetIdx( nOldIdx );
    return true;
Err:
    rPLCF.SetIdx( nOldIdx );
    return false;
}

OUString read_uInt8_BeltAndBracesString(SvStream& rStrm, rtl_TextEncoding eEnc)
{
    const OUString aRet = read_uInt8_lenPrefixed_uInt8s_ToOUString(rStrm, eEnc);
    rStrm.SeekRel(sizeof(sal_uInt8)); // skip null-byte at end
    return aRet;
}

OUString read_uInt16_BeltAndBracesString(SvStream& rStrm)
{
    const OUString aRet = read_uInt16_PascalString(rStrm);
    rStrm.SeekRel(sizeof(sal_Unicode)); // skip null-byte at end
    return aRet;
}

sal_Int32 WW8ScannerBase::WW8ReadString( SvStream& rStrm, OUString& rStr,
    WW8_CP nAktStartCp, long nTotalLen, rtl_TextEncoding eEnc ) const
{
    // Read in plain text, which can extend over several pieces
    rStr.clear();

    long nTotalRead = 0;
    WW8_CP nBehindTextCp = nAktStartCp + nTotalLen;
    WW8_CP nNextPieceCp  = nBehindTextCp; // Initialization, important for Ver6
    do
    {
        bool bIsUnicode(false), bPosOk(false);
        WW8_FC fcAct = WW8Cp2Fc(nAktStartCp,&bIsUnicode,&nNextPieceCp,&bPosOk);

        // Probably aimed beyond file end, doesn't matter!
        if( !bPosOk )
            break;

        rStrm.Seek( fcAct );

        long nLen = ( (nNextPieceCp < nBehindTextCp) ? nNextPieceCp
            : nBehindTextCp ) - nAktStartCp;

        if( nLen > USHRT_MAX - 1 )
            nLen = USHRT_MAX - 1;

        if( 0 >= nLen )
            break;

        rStr += bIsUnicode
             ? read_uInt16s_ToOUString(rStrm, nLen)
             : read_uInt8s_ToOUString(rStrm, nLen, eEnc);

        nTotalRead  += nLen;
        nAktStartCp += nLen;
        if ( nTotalRead != rStr.getLength() )
            break;
    }
    while( nTotalRead < nTotalLen );

    return rStr.getLength();
}

WW8PLCFspecial::WW8PLCFspecial(SvStream* pSt, sal_uInt32 nFilePos,
    sal_uInt32 nPLCF, sal_uInt32 nStruct)
    : nIdx(0), nStru(nStruct)
{
    const sal_uInt32 nValidMin=4;

    sal_Size nOldPos = pSt->Tell();

    bool bValid = checkSeek(*pSt, nFilePos);
    sal_Size nRemainingSize = pSt->remainingSize();
    if( !(nRemainingSize >= nValidMin && nPLCF >= nValidMin ))
        bValid = false;
    nPLCF = bValid ? std::min(nRemainingSize, static_cast<sal_Size>(nPLCF)) : nValidMin;

    // Pointer to Pos- and Struct-array
    pPLCF_PosArray = new sal_Int32[ ( nPLCF + 3 ) / 4 ];
    pPLCF_PosArray[0] = 0;

    nPLCF = bValid ? pSt->Read(pPLCF_PosArray, nPLCF) : nValidMin;

    nPLCF = std::max(nPLCF, nValidMin);

    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );
#ifdef OSL_BIGENDIAN
    for( nIdx = 0; nIdx <= nIMax; nIdx++ )
        pPLCF_PosArray[nIdx] = OSL_SWAPDWORD( pPLCF_PosArray[nIdx] );
    nIdx = 0;
#endif // OSL_BIGENDIAN
    if( nStruct ) // Pointer to content array
        pPLCF_Contents = reinterpret_cast<sal_uInt8*>(&pPLCF_PosArray[nIMax + 1]);
    else
        pPLCF_Contents = nullptr;                         // no content

    pSt->Seek(nOldPos);
}

// WW8PLCFspecial::SeekPos() sets WW8PLCFspecial to position nPos, while also the entry is used
// that begins before nPos and ends after nPos.
// Suitable for normal attributes. However, the beginning of the attribute is not corrected onto
// the position nPos.
bool WW8PLCFspecial::SeekPos(long nP)
{
    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return false;   // Not found: nP less than smallest entry
    }

    // Search from beginning?
    if( (1 > nIdx) || (nP < pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = nIMax;

    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                                   // search with an index that is incremented by 1
            if( nP < pPLCF_PosArray[nI] )
            {                               // found position
                nIdx = nI - 1;              // nI - 1 is the correct index
                return true;                // done
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
    nIdx = nIMax;               // not found, greater than all entries
    return false;
}

// WW8PLCFspecial::SeekPosExact() like SeekPos(), but it is ensured that no attribute is cut,
// i.e. the next given attribute begins at or after nPos.
// Is used for fields and bookmarks.
bool WW8PLCFspecial::SeekPosExact(long nP)
{
    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return false;       // Not found: nP less than smallest entry
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
            {                           // found position
                nIdx = nI;              // nI is the correct index
                return true;            // done
            }
        }
        nI   = 0;
        nEnd = nIdx;
    }
    nIdx = nIMax;               // Not found, greater than all entries
    return false;
}

bool WW8PLCFspecial::Get(WW8_CP& rPos, void*& rpValue) const
{
    return GetData( nIdx, rPos, rpValue );
}

bool WW8PLCFspecial::GetData(long nInIdx, WW8_CP& rPos, void*& rpValue) const
{
    if ( nInIdx >= nIMax )
    {
        rPos = WW8_CP_MAX;
        return false;
    }
    rPos = pPLCF_PosArray[nInIdx];
    rpValue = pPLCF_Contents ? static_cast<void*>(&pPLCF_Contents[nInIdx * nStru]) : nullptr;
    return true;
}

// WW8PLCF e.g. for SEPX
// Ctor for *others* than Fkps
// With nStartPos < 0, the first element of PLCFs will be taken
WW8PLCF::WW8PLCF(SvStream& rSt, WW8_FC nFilePos, sal_Int32 nPLCF, int nStruct,
    WW8_CP nStartPos) : pPLCF_PosArray(nullptr), nIdx(0), nStru(nStruct)
{
    OSL_ENSURE( nPLCF, "WW8PLCF: nPLCF is zero!" );

    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );

    ReadPLCF(rSt, nFilePos, nPLCF);

    if( nStartPos >= 0 )
        SeekPos( nStartPos );
}

// Ctor *only* for Fkps
// The last 2 parameters are needed for PLCF.Chpx and PLCF.Papx.
// If ncpN != 0, then an incomplete PLCF will be completed. This is always required for WW6 with
// lack of resources and for WordPad (W95).
// With nStartPos < 0, the first element of the PLCFs is taken.
WW8PLCF::WW8PLCF(SvStream& rSt, WW8_FC nFilePos, sal_Int32 nPLCF, int nStruct,
    WW8_CP nStartPos, sal_Int32 nPN, sal_Int32 ncpN): pPLCF_PosArray(nullptr), nIdx(0),
    nStru(nStruct)
{
    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );

    if( nIMax >= ncpN )
        ReadPLCF(rSt, nFilePos, nPLCF);
    else
        GeneratePLCF(rSt, nPN, ncpN);

    if( nStartPos >= 0 )
        SeekPos( nStartPos );
}

void WW8PLCF::ReadPLCF(SvStream& rSt, WW8_FC nFilePos, sal_uInt32 nPLCF)
{
    sal_Size nOldPos = rSt.Tell();
    bool bValid = nPLCF != 0 && checkSeek(rSt, nFilePos)
        && (rSt.remainingSize() >= nPLCF);

    if (bValid)
    {
        // Pointer to Pos-array
        pPLCF_PosArray = new WW8_CP[ ( nPLCF + 3 ) / 4 ];
        bValid = checkRead(rSt, pPLCF_PosArray, nPLCF);
    }

    if (bValid)
    {
#ifdef OSL_BIGENDIAN
        for( nIdx = 0; nIdx <= nIMax; nIdx++ )
            pPLCF_PosArray[nIdx] = OSL_SWAPDWORD( pPLCF_PosArray[nIdx] );
        nIdx = 0;
#endif // OSL_BIGENDIAN
        // Pointer to content array
        pPLCF_Contents = reinterpret_cast<sal_uInt8*>(&pPLCF_PosArray[nIMax + 1]);

        TruncToSortedRange();
    }

    OSL_ENSURE(bValid, "Document has corrupt PLCF, ignoring it");

    if (!bValid)
        MakeFailedPLCF();

    rSt.Seek(nOldPos);
}

void WW8PLCF::MakeFailedPLCF()
{
    nIMax = 0;
    delete[] pPLCF_PosArray;
    pPLCF_PosArray = new sal_Int32[2];
    pPLCF_PosArray[0] = pPLCF_PosArray[1] = WW8_CP_MAX;
    pPLCF_Contents = reinterpret_cast<sal_uInt8*>(&pPLCF_PosArray[nIMax + 1]);
}

void WW8PLCF::TruncToSortedRange()
{
    //Docs state that: ... all Plcs ... are sorted in ascending order.
    //So ensure that here for broken documents.
    for (auto nI = 0; nI < nIMax; ++nI)
    {
        if (pPLCF_PosArray[nI] > pPLCF_PosArray[nI+1])
        {
            SAL_WARN("sw.ww8", "Document has unsorted PLCF, truncated to sorted portion");
            nIMax = nI;
            break;
        }
    }
}

void WW8PLCF::GeneratePLCF(SvStream& rSt, sal_Int32 nPN, sal_Int32 ncpN)
{
    OSL_ENSURE( nIMax < ncpN, "Pcl.Fkp: Why is PLCF too big?" );

    bool failure = false;
    nIMax = ncpN;

    if ((nIMax < 1) || (nIMax > (WW8_CP_MAX - 4)/6) || ((nPN + ncpN) > USHRT_MAX) || nPN < 0)
        failure = true;

    if (!failure)
    {
        size_t nSiz = 6 * nIMax + 4;
        size_t nElems = ( nSiz + 3 ) / 4;
        pPLCF_PosArray = new sal_Int32[ nElems ]; // Pointer to Pos-array

        for (sal_Int32 i = 0; i < ncpN && !failure; ++i)
        {
            failure = true;
            // construct FC entries
            // first FC entry of each Fkp
            if (checkSeek(rSt, ( nPN + i ) << 9 ))
                continue;
            WW8_CP nFc(0);
            rSt.ReadInt32( nFc );
            pPLCF_PosArray[i] = nFc;
            failure = rSt.GetError();
        }
    }

    if (!failure)
    {
        do
        {
            failure = true;

            sal_Size nLastFkpPos = nPN + nIMax - 1;
            nLastFkpPos = nLastFkpPos << 9;
            // Anz. Fkp-Eintraege des letzten Fkp
            if (!checkSeek(rSt, nLastFkpPos + 511))
                break;

            sal_uInt8 nb(0);
            rSt.ReadUChar( nb );
            // letzer FC-Eintrag des letzten Fkp
            if (!checkSeek(rSt, nLastFkpPos + nb * 4))
                break;

            WW8_CP nFc(0);
            rSt.ReadInt32( nFc );
            pPLCF_PosArray[nIMax] = nFc;        // end of the last Fkp

            failure = rSt.GetError();
        } while(false);
    }

    if (!failure)
    {
        // Pointer to content array
        pPLCF_Contents = reinterpret_cast<sal_uInt8*>(&pPLCF_PosArray[nIMax + 1]);
        sal_uInt8* p = pPLCF_Contents;

        for (sal_Int32 i = 0; i < ncpN; ++i)         // construct PNs
        {
            ShortToSVBT16(static_cast<sal_uInt16>(nPN + i), p);
            p+=2;
        }
    }

    SAL_WARN_IF(failure, "sw.ww8", "Document has corrupt PLCF, ignoring it");

    if (failure)
        MakeFailedPLCF();
}

bool WW8PLCF::SeekPos(WW8_CP nPos)
{
    WW8_CP nP = nPos;

    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        // not found: nPos less than smallest entry
        return false;
    }

    // Search from beginning?
    if( (1 > nIdx) || (nP < pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    sal_Int32 nI   = nIdx ? nIdx : 1;
    sal_Int32 nEnd = nIMax;

    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)             // search with an index that is incremented by 1
        {
            if( nP < pPLCF_PosArray[nI] )   // found position
            {
                nIdx = nI - 1;              // nI - 1 is the correct index
                return true;                // done
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }

    nIdx = nIMax;               // not found, greater than all entries
    return false;
}

bool WW8PLCF::Get(WW8_CP& rStart, WW8_CP& rEnd, void*& rpValue) const
{
    if ( nIdx >= nIMax )
    {
        rStart = rEnd = WW8_CP_MAX;
        return false;
    }
    rStart = pPLCF_PosArray[ nIdx ];
    rEnd   = pPLCF_PosArray[ nIdx + 1 ];
    rpValue = static_cast<void*>(&pPLCF_Contents[nIdx * nStru]);
    return true;
}

WW8_CP WW8PLCF::Where() const
{
    if ( nIdx >= nIMax )
        return WW8_CP_MAX;

    return pPLCF_PosArray[nIdx];
}

WW8PLCFpcd::WW8PLCFpcd(SvStream* pSt, sal_uInt32 nFilePos,
    sal_uInt32 nPLCF, sal_uInt32 nStruct)
    : nStru( nStruct )
{
    const sal_uInt32 nValidMin=4;

    sal_Size nOldPos = pSt->Tell();

    bool bValid = checkSeek(*pSt, nFilePos);
    sal_Size nRemainingSize = pSt->remainingSize();
    if( !(nRemainingSize >= nValidMin && nPLCF >= nValidMin ))
        bValid = false;
    nPLCF = bValid ? std::min(nRemainingSize, static_cast<sal_Size>(nPLCF)) : nValidMin;

    pPLCF_PosArray = new sal_Int32[ ( nPLCF + 3 ) / 4 ];    // Pointer to Pos-array
    pPLCF_PosArray[0] = 0;

    nPLCF = bValid ? pSt->Read(pPLCF_PosArray, nPLCF) : nValidMin;
    nPLCF = std::max(nPLCF, nValidMin);

    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );
#ifdef OSL_BIGENDIAN
    for( long nI = 0; nI <= nIMax; nI++ )
      pPLCF_PosArray[nI] = OSL_SWAPDWORD( pPLCF_PosArray[nI] );
#endif // OSL_BIGENDIAN

    // Pointer to content array
    pPLCF_Contents = reinterpret_cast<sal_uInt8*>(&pPLCF_PosArray[nIMax + 1]);

    pSt->Seek( nOldPos );
}

// If nStartPos < 0, the first element of PLCFs will be taken
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
        return false;       // not found: nPos less than smallest entry
    }
    // Search from beginning?
    if( (1 > nIdx) || (nP < rPLCF.pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = rPLCF.nIMax;

    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                               // search with an index that is incremented by 1
            if( nP < rPLCF.pPLCF_PosArray[nI] )
            {                           // found position
                nIdx = nI - 1;          // nI - 1 is the correct index
                return true;            // done
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
    nIdx = rPLCF.nIMax;         // not found, greater than all entries
    return false;
}

bool WW8PLCFpcd_Iter::Get(WW8_CP& rStart, WW8_CP& rEnd, void*& rpValue) const
{
    if( nIdx >= rPLCF.nIMax )
    {
        rStart = rEnd = WW8_CP_MAX;
        return false;
    }
    rStart = rPLCF.pPLCF_PosArray[nIdx];
    rEnd = rPLCF.pPLCF_PosArray[nIdx + 1];
    rpValue = static_cast<void*>(&rPLCF.pPLCF_Contents[nIdx * rPLCF.nStru]);
    return true;
}

sal_Int32 WW8PLCFpcd_Iter::Where() const
{
    if ( nIdx >= rPLCF.nIMax )
        return SAL_MAX_INT32;

    return rPLCF.pPLCF_PosArray[nIdx];
}

bool WW8PLCFx_Fc_FKP::WW8Fkp::Entry::operator<
    (const WW8PLCFx_Fc_FKP::WW8Fkp::Entry& rSecond) const
{
    return (mnFC < rSecond.mnFC);
}

bool IsReplaceAllSprm(sal_uInt16 nSpId)
{
    return (0x6645 == nSpId || 0x6646 == nSpId);
}

bool IsExpandableSprm(sal_uInt16 nSpId)
{
    return 0x646B == nSpId;
}

void WW8PLCFx_Fc_FKP::WW8Fkp::FillEntry(WW8PLCFx_Fc_FKP::WW8Fkp::Entry &rEntry,
    sal_Size nDataOffset, sal_uInt16 nLen)
{
    bool bValidPos = (nDataOffset < sizeof(maRawData));

    OSL_ENSURE(bValidPos, "sprm sequence offset is out of range, ignoring");

    if (!bValidPos)
    {
        rEntry.mnLen = 0;
        return;
    }

    const sal_uInt16 nAvailableData = sizeof(maRawData)-nDataOffset;
    OSL_ENSURE(nLen <= nAvailableData, "srpm sequence len is out of range, clipping");
    rEntry.mnLen = std::min(nLen, nAvailableData);
    rEntry.mpData = maRawData + nDataOffset;
}

WW8PLCFx_Fc_FKP::WW8Fkp::WW8Fkp(ww::WordVersion eVersion, SvStream* pSt,
    SvStream* pDataSt, long _nFilePos, long nItemSiz, ePLCFT ePl,
    WW8_FC nStartFc)
    : nItemSize(nItemSiz), nFilePos(_nFilePos),  mnIdx(0), ePLCF(ePl),
    maSprmParser(eVersion)
{
    memset(maRawData, 0, 512);

    sal_Size nOldPos = pSt->Tell();

    bool bCouldSeek = checkSeek(*pSt, nFilePos);
    bool bCouldRead = bCouldSeek && checkRead(*pSt, maRawData, 512);

    mnIMax = bCouldRead ? maRawData[511] : 0;

    sal_uInt8 *pStart = maRawData;
    // Offset-Location in maRawData
    const size_t nRawDataStart = (mnIMax + 1) * 4;

    for (mnIdx = 0; mnIdx < mnIMax; ++mnIdx)
    {
        const size_t nRawDataOffset = nRawDataStart + mnIdx * nItemSize;

        //clip to available data, corrupt fkp
        if (nRawDataOffset >= 511)
        {
            mnIMax = mnIdx;
            break;
        }

        unsigned int nOfs = maRawData[nRawDataOffset] * 2;

        //clip to available data, corrupt fkp
        if (nOfs >= 511)
        {
            mnIMax = mnIdx;
            break;
        }

        Entry aEntry(Get_Long(pStart));

        if (nOfs)
        {
            switch (ePLCF)
            {
                case CHP:
                {
                    aEntry.mnLen = maRawData[nOfs];

                    //len byte
                    sal_Size nDataOffset = nOfs + 1;

                    FillEntry(aEntry, nDataOffset, aEntry.mnLen);

                    if (aEntry.mnLen && eVersion <= ww::eWW2)
                    {
                        Word2CHPX aChpx = ReadWord2Chpx(*pSt, nFilePos + nOfs + 1, static_cast< sal_uInt8 >(aEntry.mnLen));
                        std::vector<sal_uInt8> aSprms = ChpxToSprms(aChpx);
                        aEntry.mnLen = static_cast< sal_uInt16 >(aSprms.size());
                        if (aEntry.mnLen)
                        {
                            aEntry.mpData = new sal_uInt8[aEntry.mnLen];
                            memcpy(aEntry.mpData, &(aSprms[0]), aEntry.mnLen);
                            aEntry.mbMustDelete = true;
                        }
                    }
                    break;
                }
                case PAP:
                    {
                        sal_uInt8 nDelta = 0;

                        aEntry.mnLen = maRawData[nOfs];
                        if (IsEightPlus(eVersion) && !aEntry.mnLen)
                        {
                            aEntry.mnLen = maRawData[nOfs+1];
                            nDelta++;
                        }
                        aEntry.mnLen *= 2;

                        //stylecode, std/istd
                        if (eVersion <= ww::eWW2)
                        {
                            if (aEntry.mnLen >= 1)
                            {
                                aEntry.mnIStd = *(maRawData+nOfs+1+nDelta);
                                aEntry.mnLen--;  //style code
                                if (aEntry.mnLen >= 6)
                                {
                                    aEntry.mnLen-=6; //PHE
                                    //skipi stc, len byte + 6 byte PHE
                                    unsigned int nOffset = nOfs + 8;
                                    if (nOffset >= 511) //Bad offset
                                        aEntry.mnLen=0;
                                    if (aEntry.mnLen)   //start is ok
                                    {
                                        if (nOffset + aEntry.mnLen > 512)   //Bad end, clip
                                            aEntry.mnLen = 512 - nOffset;
                                        aEntry.mpData = maRawData + nOffset;
                                    }
                                }
                                else
                                    aEntry.mnLen=0; //Too short
                            }
                        }
                        else
                        {
                            if (aEntry.mnLen >= 2)
                            {
                                //len byte + optional extra len byte
                                sal_Size nDataOffset = nOfs + 1 + nDelta;
                                aEntry.mnIStd = nDataOffset <= sizeof(maRawData)-sizeof(aEntry.mnIStd) ?
                                    SVBT16ToShort(maRawData+nDataOffset) : 0;
                                aEntry.mnLen-=2; //istd
                                if (aEntry.mnLen)
                                {
                                    //additional istd
                                    nDataOffset += sizeof(aEntry.mnIStd);

                                    FillEntry(aEntry, nDataOffset, aEntry.mnLen);
                                }
                            }
                            else
                                aEntry.mnLen=0; //Too short, ignore
                        }

                        const sal_uInt16 nSpId = aEntry.mnLen
                            ? maSprmParser.GetSprmId(aEntry.mpData) : 0;

                        /*
                         If we replace then we throw away the old data, if we
                         are expanding, then we tack the old data onto the end
                         of the new data
                        */
                        bool bExpand = IsExpandableSprm(nSpId);
                        if (IsReplaceAllSprm(nSpId) || bExpand)
                        {
                            sal_uInt32 nCurr = pDataSt->Tell();
                            sal_uInt32 nPos = SVBT32ToUInt32(aEntry.mpData + 2);
                            sal_uInt16 nLen(0);

                            bool bOk = checkSeek(*pDataSt, nPos);
                            if (bOk)
                            {
                                pDataSt->ReadUInt16( nLen );
                                bOk = nLen <= pDataSt->remainingSize();
                            }

                            if (bOk)
                            {
                                const sal_uInt16 nOrigLen = bExpand ? aEntry.mnLen : 0;
                                sal_uInt8 *pOrigData = bExpand ? aEntry.mpData : nullptr;

                                aEntry.mnLen = nLen;
                                aEntry.mpData =
                                    new sal_uInt8[aEntry.mnLen + nOrigLen];
                                aEntry.mbMustDelete = true;
                                aEntry.mnLen =
                                    pDataSt->Read(aEntry.mpData, aEntry.mnLen);

                                pDataSt->Seek( nCurr );

                                if (pOrigData)
                                {
                                    memcpy(aEntry.mpData + aEntry.mnLen,
                                        pOrigData, nOrigLen);
                                    aEntry.mnLen = aEntry.mnLen + nOrigLen;
                                }
                            }
                        }
                    }
                    break;
                default:
                    OSL_FAIL("sweet god, what have you done!");
                    break;
            }
        }

        maEntries.push_back(aEntry);

#ifdef DEBUGSPRMREADER
        {
            sal_Int32 nLen;
            sal_uInt8* pSprms = GetLenAndIStdAndSprms( nLen );

            WW8SprmIter aIter(pSprms, nLen, maSprmParser);
            while (aIter.GetSprms())
            {
                fprintf(stderr, "id is %x\n", aIter.GetAktId());
                aIter.advance();
            }
        }
#endif
    }

    //one more FC than grrpl entries
    maEntries.push_back(Entry(Get_Long(pStart)));

    //we expect them sorted, but it appears possible for them to arrive unsorted
    std::sort(maEntries.begin(), maEntries.end());

    mnIdx = 0;

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
    if (this == &rEntry)
        return *this;

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
        mnIdx = 0;
        return false;       // not found: nPos less than smallest entry
    }

    // Search from beginning?
    if ((1 > mnIdx) || (nFc < maEntries[mnIdx-1].mnFC))
        mnIdx = 1;

    sal_uInt8 nI   = mnIdx ? mnIdx : 1;
    sal_uInt8 nEnd = mnIMax;

    for(sal_uInt8 n = (1==mnIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                               // search with an index that is incremented by 1
            if (nFc < maEntries[nI].mnFC)
            {                           // found position
                mnIdx = nI - 1;          // nI - 1 is the correct index
                return true;            // done
            }
        }
        nI = 1;
        nEnd = mnIdx-1;
    }
    mnIdx = mnIMax;               // not found, greater than all entries
    return false;
}

sal_uInt8* WW8PLCFx_Fc_FKP::WW8Fkp::Get(WW8_FC& rStart, WW8_FC& rEnd, sal_Int32& rLen)
    const
{
    rLen = 0;

    if (mnIdx >= mnIMax)
    {
        rStart = WW8_FC_MAX;
        return nullptr;
    }

    rStart = maEntries[mnIdx].mnFC;
    rEnd   = maEntries[mnIdx + 1].mnFC;

    sal_uInt8* pSprms = GetLenAndIStdAndSprms( rLen );
    return pSprms;
}

bool WW8PLCFx_Fc_FKP::WW8Fkp::SetIdx(sal_uInt8 nI)
{
    if (nI < mnIMax)
    {
        mnIdx = nI;
        return true;
    }
    return false;
}

sal_uInt8* WW8PLCFx_Fc_FKP::WW8Fkp::GetLenAndIStdAndSprms(sal_Int32& rLen) const
{
    rLen = maEntries[mnIdx].mnLen;
    return maEntries[mnIdx].mpData;
}

const sal_uInt8* WW8PLCFx_Fc_FKP::WW8Fkp::HasSprm( sal_uInt16 nId )
{
    if (mnIdx >= mnIMax)
        return nullptr;

    sal_Int32 nLen;
    sal_uInt8* pSprms = GetLenAndIStdAndSprms( nLen );

    WW8SprmIter aIter(pSprms, nLen, maSprmParser);
    return aIter.FindSprm(nId);
}

bool WW8PLCFx_Fc_FKP::WW8Fkp::HasSprm(sal_uInt16 nId,
    std::vector<const sal_uInt8 *> &rResult)
{
    if (mnIdx >= mnIMax)
       return false;

    sal_Int32 nLen;
    sal_uInt8* pSprms = GetLenAndIStdAndSprms( nLen );

    WW8SprmIter aIter(pSprms, nLen, maSprmParser);

    while(aIter.GetSprms())
    {
        if (aIter.GetAktId() == nId)
            rResult.push_back(aIter.GetAktParams());
        aIter.advance();
    };
    return !rResult.empty();
}

void WW8PLCFx::GetSprms( WW8PLCFxDesc* p )
{
    OSL_ENSURE( false, "Called wrong GetSprms" );
    p->nStartPos = p->nEndPos = WW8_CP_MAX;
    p->pMemPos = nullptr;
    p->nSprmsLen = 0;
    p->bRealLineEnd = false;
    return;
}

long WW8PLCFx::GetNoSprms( WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen )
{
    OSL_ENSURE( false, "Called wrong GetNoSprms" );
    rStart = rEnd = WW8_CP_MAX;
    rLen = 0;
    return 0;
}

// ...Idx2: Default: ignore
sal_uLong WW8PLCFx::GetIdx2() const
{
    return 0;
}

void WW8PLCFx::SetIdx2(sal_uLong )
{
}

class SamePos :
    public std::unary_function<const WW8PLCFx_Fc_FKP::WW8Fkp *, bool>
{
private:
    long mnPo;
public:
    explicit SamePos(long nPo) : mnPo(nPo) {}
    bool operator()(const WW8PLCFx_Fc_FKP::WW8Fkp *pFkp)
        {return mnPo == pFkp->GetFilePos();}
};

bool WW8PLCFx_Fc_FKP::NewFkp()
{
    WW8_CP nPLCFStart, nPLCFEnd;
    void* pPage;

    static const int WW8FkpSizeTabVer2[ PLCF_END ] =
    {
        1,  1, 0 /*, 0, 0, 0*/
    };
    static const int WW8FkpSizeTabVer6[ PLCF_END ] =
    {
        1,  7, 0 /*, 0, 0, 0*/
    };
    static const int WW8FkpSizeTabVer8[ PLCF_END ] =
    {
        1, 13, 0 /*, 0, 0, 0*/
    };
    const int* pFkpSizeTab;

    switch (GetFIBVersion())
    {
        case ww::eWW1:
        case ww::eWW2:
            pFkpSizeTab = WW8FkpSizeTabVer2;
            break;
        case ww::eWW6:
        case ww::eWW7:
            pFkpSizeTab = WW8FkpSizeTabVer6;
            break;
        case ww::eWW8:
            pFkpSizeTab = WW8FkpSizeTabVer8;
            break;
        default:
            // program error!
            OSL_ENSURE( false, "nVersion not implemented!" );
            return false;
    }

    if (!pPLCF->Get( nPLCFStart, nPLCFEnd, pPage ))
    {
        pFkp = nullptr;
        return false;                           // PLCF completely processed
    }
    pPLCF->advance();
    long nPo = SVBT16ToShort( static_cast<sal_uInt8 *>(pPage) );
    nPo <<= 9;                                  // shift as LONG

    long nAktFkpFilePos = pFkp ? pFkp->GetFilePos() : -1;
    if (nAktFkpFilePos == nPo)
        pFkp->Reset(GetStartFc());
    else
    {
        myiter aIter =
            std::find_if(maFkpCache.begin(), maFkpCache.end(), SamePos(nPo));
        if (aIter != maFkpCache.end())
        {
            pFkp = *aIter;
            pFkp->Reset(GetStartFc());
        }
        else
        {
            pFkp = new WW8Fkp(GetFIBVersion(), pFKPStrm, pDataStrm, nPo,
                pFkpSizeTab[ ePLCF ], ePLCF, GetStartFc());
            maFkpCache.push_back(pFkp);

            if (maFkpCache.size() > eMaxCache)
            {
                delete maFkpCache.front();
                maFkpCache.pop_front();
            }
        }
    }

    SetStartFc( -1 );                                   // only the first time
    return true;
}

WW8PLCFx_Fc_FKP::WW8PLCFx_Fc_FKP(SvStream* pSt, SvStream* pTableSt,
    SvStream* pDataSt, const WW8Fib& rFib, ePLCFT ePl, WW8_FC nStartFcL)
    : WW8PLCFx(rFib.GetFIBVersion(), true), pFKPStrm(pSt), pDataStrm(pDataSt),
    pFkp(nullptr), ePLCF(ePl), pPCDAttrs(nullptr)
{
    SetStartFc(nStartFcL);
    long nLenStruct = (8 > rFib.nVersion) ? 2 : 4;
    if (ePl == CHP)
    {
        pPLCF = new WW8PLCF(*pTableSt, rFib.fcPlcfbteChpx, rFib.lcbPlcfbteChpx,
            nLenStruct, GetStartFc(), rFib.pnChpFirst, rFib.cpnBteChp);
    }
    else
    {
        pPLCF = new WW8PLCF(*pTableSt, rFib.fcPlcfbtePapx, rFib.lcbPlcfbtePapx,
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

sal_uInt32 WW8PLCFx_Fc_FKP::GetIdx() const
{
    sal_uInt32 u = pPLCF->GetIdx() << 8;
    if (pFkp)
        u |= pFkp->GetIdx();
    return u;
}

void WW8PLCFx_Fc_FKP::SetIdx( sal_uLong nIdx )
{
    if( !( nIdx & 0xffffff00L ) )
    {
        pPLCF->SetIdx( nIdx >> 8 );
        pFkp = nullptr;
    }
    else
    {                                   // there was a Fkp
        // Set PLCF one position back to retrieve the address of the Fkp
        pPLCF->SetIdx( ( nIdx >> 8 ) - 1 );
        if (NewFkp())                       // read Fkp again
        {
            sal_uInt8 nFkpIdx = static_cast<sal_uInt8>(nIdx & 0xff);
            pFkp->SetIdx(nFkpIdx);          // set Fkp-Pos again
        }
    }
}

bool WW8PLCFx_Fc_FKP::SeekPos(WW8_FC nFcPos)
{
    // StartPos for next Where()
    SetStartFc( nFcPos );

    // find StartPos for next pPLCF->Get()
    bool bRet = pPLCF->SeekPos(nFcPos);

    // make FKP invalid?
    WW8_CP nPLCFStart, nPLCFEnd;
    void* pPage;
    if( pFkp && pPLCF->Get( nPLCFStart, nPLCFEnd, pPage ) )
    {
        long nPo = SVBT16ToShort( static_cast<sal_uInt8 *>(pPage) );
        nPo <<= 9;                                          // shift as LONG
        if (nPo != pFkp->GetFilePos())
            pFkp = nullptr;
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
            return WW8_FC_MAX;
    }
    WW8_FC nP = pFkp ? pFkp->Where() : WW8_FC_MAX;
    if( nP != WW8_FC_MAX )
        return nP;

    pFkp = nullptr;                   // FKP finished -> get new
    return Where();                     // easiest way: do it recursively
}

sal_uInt8* WW8PLCFx_Fc_FKP::GetSprmsAndPos(WW8_FC& rStart, WW8_FC& rEnd, sal_Int32& rLen)
{
    rLen = 0;                               // Default
    rStart = rEnd = WW8_FC_MAX;

    if( !pFkp )     // Fkp not there ?
    {
        if( !NewFkp() )
            return nullptr;
    }

    sal_uInt8* pPos = pFkp ? pFkp->Get( rStart, rEnd, rLen ) : nullptr;
    if( rStart == WW8_FC_MAX )    //Not found
        return nullptr;
    return pPos;
}

void WW8PLCFx_Fc_FKP::advance()
{
    if( !pFkp )
    {
        if( !NewFkp() )
            return;
    }

    if (!pFkp)
        return;

    pFkp->advance();
    if( pFkp->Where() == WW8_FC_MAX )
        (void)NewFkp();
}

sal_uInt16 WW8PLCFx_Fc_FKP::GetIstd() const
{
    return pFkp ? pFkp->GetIstd() : 0xFFFF;
}

void WW8PLCFx_Fc_FKP::GetPCDSprms( WW8PLCFxDesc& rDesc )
{
    rDesc.pMemPos   = nullptr;
    rDesc.nSprmsLen = 0;
    if( pPCDAttrs )
    {
        if( !pFkp )
        {
            OSL_FAIL("+Problem: GetPCDSprms: NewFkp necessary (not possible!)" );
            if( !NewFkp() )
                return;
        }
        pPCDAttrs->GetSprms(&rDesc);
    }
}

const sal_uInt8* WW8PLCFx_Fc_FKP::HasSprm( sal_uInt16 nId )
{
    // const would be nicer, but for that, NewFkp() would need to be replaced or eliminated
    if( !pFkp )
    {
        OSL_FAIL( "+Motz: HasSprm: NewFkp needed ( no const possible )" );
        // happens in BugDoc 31722
        if( !NewFkp() )
            return nullptr;
    }

    if (!pFkp)
        return nullptr;

    const sal_uInt8* pRes = pFkp->HasSprm( nId );

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

bool WW8PLCFx_Fc_FKP::HasSprm(sal_uInt16 nId, std::vector<const sal_uInt8 *> &rResult)
{
    // const would be nicer, but for that, NewFkp() would need to be replaced or eliminated
    if (!pFkp)
    {
       OSL_FAIL( "+Motz: HasSprm: NewFkp needed ( no const possible )" );
       // happens in BugDoc 31722
       if( !NewFkp() )
           return false;
    }

    if (!pFkp)
        return false;

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
            aIter.advance();
        };
    }
    return !rResult.empty();
}

WW8PLCFx_Cp_FKP::WW8PLCFx_Cp_FKP( SvStream* pSt, SvStream* pTableSt,
    SvStream* pDataSt, const WW8ScannerBase& rBase, ePLCFT ePl )
    : WW8PLCFx_Fc_FKP(pSt, pTableSt, pDataSt, *rBase.pWw8Fib, ePl,
    rBase.WW8Cp2Fc(0)), rSBase(rBase), nAttrStart(-1), nAttrEnd(-1),
    bLineEnd(false),
    bComplex( (7 < rBase.pWw8Fib->nVersion) || rBase.pWw8Fib->fComplex )
{
    ResetAttrStartEnd();

    pPcd = rSBase.pPiecePLCF ? new WW8PLCFx_PCD(GetFIBVersion(),
        rBase.pPiecePLCF, 0, IsSevenMinus(GetFIBVersion())) : nullptr;

    /*
    Make a copy of the piece attributes for so that the calls to HasSprm on a
    Fc_FKP will be able to take into account the current piece attributes,
    despite the fact that such attributes can only be found through a cp based
    mechanism.
    */
    if (pPcd)
    {
        pPCDAttrs = rSBase.pPLCFx_PCDAttrs ? new WW8PLCFx_PCDAttrs(
            rSBase.pWw8Fib->GetFIBVersion(), pPcd, &rSBase) : nullptr;
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

sal_uLong WW8PLCFx_Cp_FKP::GetPCDIMax() const
{
    return pPcd ? pPcd->GetIMax() : 0;
}

sal_uLong WW8PLCFx_Cp_FKP::GetPCDIdx() const
{
    return pPcd ? pPcd->GetIdx() : 0;
}

void WW8PLCFx_Cp_FKP::SetPCDIdx( sal_uLong nIdx )
{
    if( pPcd )
        pPcd->SetIdx( nIdx );
}

bool WW8PLCFx_Cp_FKP::SeekPos(WW8_CP nCpPos)
{
    if( pPcd )  // Complex
    {
        if( !pPcd->SeekPos( nCpPos ) )  // set piece
            return false;
        if (pPCDAttrs && !pPCDAttrs->GetIter()->SeekPos(nCpPos))
            return false;
        return WW8PLCFx_Fc_FKP::SeekPos(pPcd->AktPieceStartCp2Fc(nCpPos));
    }
                                    // NO piece table !!!
    return WW8PLCFx_Fc_FKP::SeekPos( rSBase.WW8Cp2Fc(nCpPos) );
}

WW8_CP WW8PLCFx_Cp_FKP::Where()
{
    WW8_FC nFc = WW8PLCFx_Fc_FKP::Where();
    if( pPcd )
        return pPcd->AktPieceStartFc2Cp( nFc ); // identify piece
    return rSBase.WW8Fc2Cp( nFc );      // NO piece table !!!
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
        For the odd case where we have a location in a fastsaved file which
        does not have an entry in the FKP, perhaps its para end is in the next
        piece, or perhaps the cp just doesn't exist at all in this document.
        AdvSprm doesn't know so it sets the PLCF as dirty and we figure out
        in this method what the situation is

        It doesn't exist then the piece iterator will not be able to find it.
        Otherwise our cool fastsave algorithm can be brought to bear on the
        problem.
        */
        if( !pPieceIter )
            return;
        sal_uLong nOldPos = pPieceIter->GetIdx();
        bool bOk = pPieceIter->SeekPos(nOrigCp);
        pPieceIter->SetIdx( nOldPos );
        if (!bOk)
            return;
    }

    if( pPcd )  // piece table available
    {
        // Init ( no ++ called, yet )
        if( (nAttrStart >  nAttrEnd) || (nAttrStart == -1) )
        {
            p->bRealLineEnd = (ePLCF == PAP);

            if ( ((ePLCF == PAP ) || (ePLCF == CHP)) && (nOrigCp != WW8_CP_MAX) )
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
                sal_uLong nOldPos = pPieceIter->GetIdx();
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
                WW8_CP nCpStart, nCpEnd;
                void* pData=nullptr;
                bool bOk = pPieceIter->Get(nCpStart, nCpEnd, pData);

                if (!bOk)
                {
                    pPieceIter->SetIdx(nOldPos);
                    return;
                }

                WW8_FC nLimitFC = SVBT32ToUInt32( static_cast<WW8_PCD*>(pData)->fc );
                WW8_FC nBeginLimitFC = nLimitFC;
                if (IsEightPlus(GetFIBVersion()))
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

                        pPieceIter->advance();

                        for (;pPieceIter->GetIdx() < pPieceIter->GetIMax();
                            pPieceIter->advance())
                        {
                            if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
                            {
                                OSL_ENSURE( false, "piece iter broken!" );
                                break;
                            }
                            bIsUnicode = false;
                            sal_Int32 nFcStart=SVBT32ToUInt32(static_cast<WW8_PCD*>(pData)->fc);

                            if (IsEightPlus(GetFIBVersion()))
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
                                WW8_CP nEndPos = nCpEnd -
                                    (nLimitFC-nSmallest) / (bIsUnicode ? 2 : 1);

                                OSL_ENSURE(nEndPos >= p->nStartPos, "EndPos before StartPos");

                                if (nEndPos >= p->nStartPos)
                                    p->nEndPos = nEndPos;

                                break;
                            }
                        }
                    }
                }
                pPieceIter->SetIdx( nOldPos );
            }
            else
                WW8PLCFx_PCD::AktPieceFc2Cp( p->nStartPos, p->nEndPos,&rSBase );
        }
        else
        {
            p->nStartPos = nAttrStart;
            p->nEndPos = nAttrEnd;
            p->bRealLineEnd = bLineEnd;
        }
    }
    else        // NO piece table !!!
    {
        p->nStartPos = rSBase.WW8Fc2Cp( p->nStartPos );
        p->nEndPos   = rSBase.WW8Fc2Cp( p->nEndPos );
        p->bRealLineEnd = ePLCF == PAP;
    }
}

void WW8PLCFx_Cp_FKP::advance()
{
    WW8PLCFx_Fc_FKP::advance();
    // !pPcd: emergency break
    if ( !bComplex || !pPcd )
        return;

    if( GetPCDIdx() >= GetPCDIMax() )           // End of PLCF
    {
        nAttrStart = nAttrEnd = WW8_CP_MAX;
        return;
    }

    sal_Int32 nFkpLen;                               // Fkp entry
    // get Fkp entry
    WW8PLCFx_Fc_FKP::GetSprmsAndPos(nAttrStart, nAttrEnd, nFkpLen);

    WW8PLCFx_PCD::AktPieceFc2Cp( nAttrStart, nAttrEnd, &rSBase );
    bLineEnd = (ePLCF == PAP);
}

WW8PLCFx_SEPX::WW8PLCFx_SEPX(SvStream* pSt, SvStream* pTableSt,
    const WW8Fib& rFib, WW8_CP nStartCp)
    : WW8PLCFx(rFib.GetFIBVersion(), true), maSprmParser(rFib.GetFIBVersion()),
    pStrm(pSt), nArrMax(256), nSprmSiz(0)
{
    pPLCF =   rFib.lcbPlcfsed
            ? new WW8PLCF(*pTableSt, rFib.fcPlcfsed, rFib.lcbPlcfsed,
              GetFIBVersion() <= ww::eWW2 ? 6 : 12, nStartCp)
            : nullptr;

    pSprms = new sal_uInt8[nArrMax];     // maximum length
}

WW8PLCFx_SEPX::~WW8PLCFx_SEPX()
{
    delete pPLCF;
    delete[] pSprms;
}

sal_uInt32 WW8PLCFx_SEPX::GetIdx() const
{
    return pPLCF ? pPLCF->GetIdx() : 0;
}

void WW8PLCFx_SEPX::SetIdx( sal_uLong nIdx )
{
    if( pPLCF ) pPLCF->SetIdx( nIdx );
}

bool WW8PLCFx_SEPX::SeekPos(WW8_CP nCpPos)
{
    return pPLCF && pPLCF->SeekPos( nCpPos );
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
        p->nStartPos = p->nEndPos = WW8_CP_MAX;       // PLCF completely processed
        p->pMemPos = nullptr;
        p->nSprmsLen = 0;
    }
    else
    {
        sal_uInt32 nPo =  SVBT32ToUInt32( static_cast<sal_uInt8*>(pData)+2 );
        if (nPo == 0xFFFFFFFF)
        {
            p->nStartPos = p->nEndPos = WW8_CP_MAX;   // Sepx empty
            p->pMemPos = nullptr;
            p->nSprmsLen = 0;
        }
        else
        {
            pStrm->Seek( nPo );

            // read len
            if (GetFIBVersion() <= ww::eWW2)    // eWW6 ?, docs say yes, but...
            {
                sal_uInt8 nSiz(0);
                pStrm->ReadUChar( nSiz );
                nSprmSiz = nSiz;
            }
            else
            {
                pStrm->ReadUInt16( nSprmSiz );
            }

            sal_Size nRemaining = pStrm->remainingSize();
            if (nSprmSiz > nRemaining)
                nSprmSiz = nRemaining;

            if( nSprmSiz > nArrMax )
            {               // does not fit
                delete[] pSprms;
                nArrMax = nSprmSiz;                 // Get more memory
                pSprms = new sal_uInt8[nArrMax];
            }
            nSprmSiz = pStrm->Read(pSprms, nSprmSiz); // read Sprms

            p->nSprmsLen = nSprmSiz;
            p->pMemPos = pSprms;                    // return Position
        }
    }
}

void WW8PLCFx_SEPX::advance()
{
    if (pPLCF)
        pPLCF->advance();
}

const sal_uInt8* WW8PLCFx_SEPX::HasSprm( sal_uInt16 nId ) const
{
    return HasSprm( nId, pSprms, nSprmSiz);
}

const sal_uInt8* WW8PLCFx_SEPX::HasSprm( sal_uInt16 nId, const sal_uInt8*  pOtherSprms,
    long nOtherSprmSiz ) const
{
    const sal_uInt8 *pRet = nullptr;
    if (pPLCF)
    {
        WW8SprmIter aIter(pOtherSprms, nOtherSprmSiz, maSprmParser);
        pRet = aIter.FindSprm(nId);
    }
    return pRet;
}

bool WW8PLCFx_SEPX::Find4Sprms(sal_uInt16 nId1,sal_uInt16 nId2,sal_uInt16 nId3,sal_uInt16 nId4,
    sal_uInt8*& p1, sal_uInt8*& p2, sal_uInt8*& p3, sal_uInt8*& p4) const
{
    if( !pPLCF )
        return false;

    bool bFound = false;
    p1 = nullptr;
    p2 = nullptr;
    p3 = nullptr;
    p4 = nullptr;

    sal_uInt8* pSp = pSprms;
    sal_uInt16 i=0;
    while (i + maSprmParser.MinSprmLen() <= nSprmSiz)
    {
        // Sprm found?
        const sal_uInt16 nAktId = maSprmParser.GetSprmId(pSp);
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
        // increment pointer so that it points to next SPRM
        const sal_uInt16 x = maSprmParser.GetSprmSize(nAktId, pSp);
        i += x;
        pSp += x;
    }
    return bFound;
}

const sal_uInt8* WW8PLCFx_SEPX::HasSprm( sal_uInt16 nId, sal_uInt8 n2nd ) const
{
    if( !pPLCF )
        return nullptr;

    sal_uInt8* pSp = pSprms;

    sal_uInt16 i=0;
    while (i + maSprmParser.MinSprmLen() <= nSprmSiz)
    {
        // Sprm found?
        const sal_uInt16 nAktId = maSprmParser.GetSprmId(pSp);
        if (nAktId == nId)
        {
            sal_uInt8 *pRet = pSp + maSprmParser.DistanceToData(nId);
            if (*pRet == n2nd)
                return pRet;
        }
        // increment pointer so that it points to next SPRM
        const sal_uInt16 x = maSprmParser.GetSprmSize(nAktId, pSp);
        i += x;
        pSp += x;
    }

    return nullptr;   // Sprm not found
}

WW8PLCFx_SubDoc::WW8PLCFx_SubDoc(SvStream* pSt, ww::WordVersion eVersion,
    WW8_CP nStartCp, long nFcRef, long nLenRef, long nFcText, long nLenText,
    long nStruct)
    : WW8PLCFx(eVersion, true), pRef(nullptr), pText(nullptr)
{
    if( nLenRef && nLenText )
    {
        pRef = new WW8PLCF(*pSt, nFcRef, nLenRef, nStruct, nStartCp);
        pText = new WW8PLCF(*pSt, nFcText, nLenText, 0, nStartCp);
    }
}

WW8PLCFx_SubDoc::~WW8PLCFx_SubDoc()
{
    delete pRef;
    delete pText;
}

sal_uInt32 WW8PLCFx_SubDoc::GetIdx() const
{
    // Probably pText ... no need for it
    if( pRef )
        return ( pRef->GetIdx() << 16 | pText->GetIdx() );
    return 0;
}

void WW8PLCFx_SubDoc::SetIdx( sal_uLong nIdx )
{
    if( pRef )
    {
        pRef->SetIdx( nIdx >> 16 );
        // Probably pText ... no need for it
        pText->SetIdx( nIdx & 0xFFFF );
    }
}

bool WW8PLCFx_SubDoc::SeekPos( WW8_CP nCpPos )
{
    return pRef && pRef->SeekPos( nCpPos );
}

WW8_CP WW8PLCFx_SubDoc::Where()
{
    return ( pRef ) ? pRef->Where() : WW8_CP_MAX;
}

void WW8PLCFx_SubDoc::GetSprms(WW8PLCFxDesc* p)
{
    p->nStartPos = p->nEndPos = WW8_CP_MAX;
    p->pMemPos = nullptr;
    p->nSprmsLen = 0;
    p->bRealLineEnd = false;

    if (!pRef)
        return;

    sal_uLong nNr = pRef->GetIdx();

    void *pData;
    WW8_CP nFoo;
    if (!pRef->Get(p->nStartPos, nFoo, pData))
    {
        p->nEndPos = p->nStartPos = WW8_CP_MAX;
        return;
    }

    p->nEndPos = p->nStartPos + 1;

    if (!pText)
        return;

    pText->SetIdx(nNr);

    if (!pText->Get(p->nCp2OrIdx, p->nSprmsLen, pData))
    {
        p->nEndPos = p->nStartPos = WW8_CP_MAX;
        p->nSprmsLen = 0;
        return;
    }

    p->nSprmsLen -= p->nCp2OrIdx;
}

void WW8PLCFx_SubDoc::advance()
{
    if (pRef && pText)
    {
        pRef->advance();
        pText->advance();
    }
}

// fields
WW8PLCFx_FLD::WW8PLCFx_FLD( SvStream* pSt, const WW8Fib& rMyFib, short nType)
    : WW8PLCFx(rMyFib.GetFIBVersion(), true), pPLCF(nullptr), rFib(rMyFib)
{
    long nFc, nLen;

    switch( nType )
    {
    case MAN_HDFT:
        nFc = rFib.fcPlcffldHdr;
        nLen = rFib.lcbPlcffldHdr;
        break;
    case MAN_FTN:
        nFc = rFib.fcPlcffldFootnote;
        nLen = rFib.lcbPlcffldFootnote;
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

sal_uInt32 WW8PLCFx_FLD::GetIdx() const
{
    return pPLCF ? pPLCF->GetIdx() : 0;
}

void WW8PLCFx_FLD::SetIdx( sal_uLong nIdx )
{
    if( pPLCF )
        pPLCF->SetIdx( nIdx );
}

bool WW8PLCFx_FLD::SeekPos(WW8_CP nCpPos)
{
    return pPLCF && pPLCF->SeekPosExact( nCpPos );
}

WW8_CP WW8PLCFx_FLD::Where()
{
    return pPLCF ? pPLCF->Where() : WW8_CP_MAX;
}

bool WW8PLCFx_FLD::StartPosIsFieldStart()
{
    void* pData;
    sal_Int32 nTest;
    if (
         (!pPLCF || !pPLCF->Get(nTest, pData) ||
         ((static_cast<sal_uInt8*>(pData)[0] & 0x1f) != 0x13))
       )
        return false;
    return true;
}

bool WW8PLCFx_FLD::EndPosIsFieldEnd(WW8_CP& nCP)
{
    bool bRet = false;

    if (pPLCF)
    {
        long n = pPLCF->GetIdx();

        pPLCF->advance();

        void* pData;
        sal_Int32 nTest;
        if ( pPLCF->Get(nTest, pData) && ((static_cast<sal_uInt8*>(pData)[0] & 0x1f) == 0x15) )
        {
            nCP = nTest;
            bRet = true;
        }

        pPLCF->SetIdx(n);
    }

    return bRet;
}

void WW8PLCFx_FLD::GetSprms(WW8PLCFxDesc* p)
{
    p->nStartPos = p->nEndPos = WW8_CP_MAX;
    p->pMemPos = nullptr;
    p->nSprmsLen = 0;
    p->bRealLineEnd = false;

    if (!pPLCF)
    {
        p->nStartPos = WW8_CP_MAX;                    // there are no fields
        return;
    }

    long n = pPLCF->GetIdx();

    sal_Int32 nP;
    void *pData;
    if (!pPLCF->Get(nP, pData))             // end of PLCFspecial?
    {
        p->nStartPos = WW8_CP_MAX;            // PLCF completely processed
        return;
    }

    p->nStartPos = nP;

    pPLCF->advance();
    if (!pPLCF->Get(nP, pData))             // end of PLCFspecial?
    {
        p->nStartPos = WW8_CP_MAX;            // PLCF completely processed
        return;
    }

    p->nEndPos = nP;

    pPLCF->SetIdx(n);

    p->nCp2OrIdx = pPLCF->GetIdx();
}

void WW8PLCFx_FLD::advance()
{
    SAL_WARN_IF(!pPLCF, "sw.ww8", "Call without PLCFspecial field");
    if( !pPLCF )
        return;
    pPLCF->advance();
}

bool WW8PLCFx_FLD::GetPara(long nIdx, WW8FieldDesc& rF)
{
    SAL_WARN_IF(!pPLCF, "sw.ww8", "Call without PLCFspecial field");
    if( !pPLCF )
        return false;

    long n = pPLCF->GetIdx();
    pPLCF->SetIdx(nIdx);

    bool bOk = WW8GetFieldPara(*pPLCF, rF);

    pPLCF->SetIdx(n);
    return bOk;
}

// WW8PLCF_Book
void WW8ReadSTTBF(bool bVer8, SvStream& rStrm, sal_uInt32 nStart, sal_Int32 nLen,
    sal_uInt16 nExtraLen, rtl_TextEncoding eCS, std::vector<OUString> &rArray,
    std::vector<ww::bytes>* pExtraArray, ::std::vector<OUString>* pValueArray)
{
    if (nLen==0)     // Handle Empty STTBF
        return;

    sal_Size nOldPos = rStrm.Tell();
    if (checkSeek(rStrm, nStart))
    {
        sal_uInt16 nLen2(0);
        rStrm.ReadUInt16( nLen2 ); // bVer67: total length of structure
                        // bVer8 : count of strings

        if( bVer8 )
        {
            sal_uInt16 nStrings(0);
            bool bUnicode = (0xFFFF == nLen2);
            if (bUnicode)
                rStrm.ReadUInt16( nStrings );
            else
                nStrings = nLen2;

            rStrm.ReadUInt16( nExtraLen );

            const size_t nMinStringLen = bUnicode ? sizeof(sal_uInt16) : sizeof(sal_uInt8);
            const size_t nMinRecordSize = nExtraLen + nMinStringLen;
            const size_t nMaxPossibleStrings = rStrm.remainingSize() / nMinRecordSize;
            if (nStrings > nMaxPossibleStrings)
            {
                SAL_WARN("sw.ww8", "STTBF claims " << nStrings << " entries, but only " << nMaxPossibleStrings << "are possible");
                nStrings = nMaxPossibleStrings;
            }

            if (nExtraLen && nStrings)
            {
                const size_t nMaxExtraLen = (rStrm.remainingSize() - (nStrings * nMinStringLen)) / nStrings;
                if (nExtraLen > nMaxExtraLen)
                {
                    SAL_WARN("sw.ww8", "STTBF claims " << nMaxExtraLen << " extra len, but only " << nMaxExtraLen << "are possible");
                    nExtraLen = nMaxExtraLen;
                }
            }

            for (sal_uInt16 i=0; i < nStrings; ++i)
            {
                if (bUnicode)
                    rArray.push_back(read_uInt16_PascalString(rStrm));
                else
                {
                    OString aTmp = read_uInt8_lenPrefixed_uInt8s_ToOString(rStrm);
                    rArray.push_back(OStringToOUString(aTmp, eCS));
                }

                // Skip the extra data
                if (nExtraLen)
                {
                    if (pExtraArray)
                    {
                        ww::bytes extraData(nExtraLen);
                        rStrm.Read(extraData.data(), nExtraLen);
                        pExtraArray->push_back(extraData);
                    }
                    else
                        rStrm.SeekRel( nExtraLen );
                }
            }
            // read the value of the document variables, if requested.
            if (pValueArray)
            {
                for (sal_uInt16 i=0; i < nStrings; ++i)
                {
                    if( bUnicode )
                        pValueArray->push_back(read_uInt16_PascalString(rStrm));
                    else
                    {
                        OString aTmp = read_uInt8_lenPrefixed_uInt8s_ToOString(rStrm);
                        pValueArray->push_back(OStringToOUString(aTmp, eCS));
                    }
                }
            }
        }
        else
        {
            if( nLen2 != nLen )
            {
                OSL_ENSURE(nLen2 == nLen,
                    "Fib length and read length are different");
                if (nLen > USHRT_MAX)
                    nLen = USHRT_MAX;
                else if (nLen < 2 )
                    nLen = 2;
                nLen2 = static_cast<sal_uInt16>(nLen);
            }
            sal_uLong nRead = 0;
            for( nLen2 -= 2; nRead < nLen2;  )
            {
                sal_uInt8 nBChar(0);
                rStrm.ReadUChar( nBChar );
                ++nRead;
                if (nBChar)
                {
                    OString aTmp = read_uInt8s_ToOString(rStrm, nBChar);
                    nRead += aTmp.getLength();
                    rArray.push_back(OStringToOUString(aTmp, eCS));
                }
                else
                    rArray.push_back(OUString());

                // Skip the extra data (for bVer67 versions this must come from
                // external knowledge)
                if (nExtraLen)
                {
                    if (pExtraArray)
                    {
                        ww::bytes extraData(nExtraLen);
                        rStrm.Read(extraData.data(), nExtraLen);
                        pExtraArray->push_back(extraData);
                    }
                    else
                        rStrm.SeekRel( nExtraLen );
                    nRead+=nExtraLen;
                }
            }
        }
    }
    rStrm.Seek(nOldPos);
}

WW8PLCFx_Book::WW8PLCFx_Book(SvStream* pTableSt, const WW8Fib& rFib)
    : WW8PLCFx(rFib.GetFIBVersion(), false), nIsEnd(0), nBookmarkId(1)
{
    if( !rFib.fcPlcfbkf || !rFib.lcbPlcfbkf || !rFib.fcPlcfbkl ||
        !rFib.lcbPlcfbkl || !rFib.fcSttbfbkmk || !rFib.lcbSttbfbkmk )
    {
        pBook[0] = pBook[1] = nullptr;
        nIMax = 0;
    }
    else
    {
        pBook[0] = new WW8PLCFspecial(pTableSt,rFib.fcPlcfbkf,rFib.lcbPlcfbkf,4);

        pBook[1] = new WW8PLCFspecial(pTableSt,rFib.fcPlcfbkl,rFib.lcbPlcfbkl,0);

        rtl_TextEncoding eStructChrSet = WW8Fib::GetFIBCharset(rFib.chseTables, rFib.lid);

        WW8ReadSTTBF( (7 < rFib.nVersion), *pTableSt, rFib.fcSttbfbkmk,
            rFib.lcbSttbfbkmk, 0, eStructChrSet, aBookNames );

        nIMax = aBookNames.size();

        if( pBook[0]->GetIMax() < nIMax )   // Count of Bookmarks
            nIMax = pBook[0]->GetIMax();
        if( pBook[1]->GetIMax() < nIMax )
            nIMax = pBook[1]->GetIMax();
        aStatus.resize(nIMax);
    }
}

WW8PLCFx_Book::~WW8PLCFx_Book()
{
    delete pBook[1];
    delete pBook[0];
}

sal_uInt32 WW8PLCFx_Book::GetIdx() const
{
    return nIMax ? pBook[0]->GetIdx() : 0;
}

void WW8PLCFx_Book::SetIdx( sal_uLong nI )
{
    if( nIMax )
        pBook[0]->SetIdx( nI );
}

sal_uLong WW8PLCFx_Book::GetIdx2() const
{
    return nIMax ? ( pBook[1]->GetIdx() | ( ( nIsEnd ) ? 0x80000000 : 0 ) ) : 0;
}

void WW8PLCFx_Book::SetIdx2( sal_uLong nI )
{
    if( nIMax )
    {
        pBook[1]->SetIdx( nI & 0x7fffffff );
        nIsEnd = (sal_uInt16)( ( nI >> 31 ) & 1 );  // 0 oder 1
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

long WW8PLCFx_Book::GetNoSprms( WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen )
{
    void* pData;
    rEnd = WW8_CP_MAX;
    rLen = 0;

    if (!pBook[0] || !pBook[1] || !nIMax || (pBook[nIsEnd]->GetIdx()) >= nIMax)
    {
        rStart = rEnd = WW8_CP_MAX;
        return -1;
    }

    (void)pBook[nIsEnd]->Get( rStart, pData );    // query position
    return pBook[nIsEnd]->GetIdx();
}

// The operator ++ has a pitfall: If 2 bookmarks adjoin each other,
// we should first go to the end of the first one
// and then to the beginning of the second one.
// But if 2 bookmarks with the length of 0 lie on top of each other,
// we *must* first find the start and end of each bookmark.
// The case of: ][
//               [...]
//              ][
// is not solved yet.
// Because I must jump back and forth in the start- and end-indices then.
// This would require one more index or bitfield to remember
// the already processed bookmarks.

void WW8PLCFx_Book::advance()
{
    if( pBook[0] && pBook[1] && nIMax )
    {
        (*pBook[nIsEnd]).advance();

        sal_uLong l0 = pBook[0]->Where();
        sal_uLong l1 = pBook[1]->Where();
        if( l0 < l1 )
            nIsEnd = 0;
        else if( l1 < l0 )
            nIsEnd = 1;
        else
        {
            const void * p = pBook[0]->GetData(pBook[0]->GetIdx());
            long nPairFor = (p == nullptr)? 0L : SVBT16ToShort(*static_cast<SVBT16 const *>(p));
            if (nPairFor == pBook[1]->GetIdx())
                nIsEnd = 0;
            else
                nIsEnd = ( nIsEnd ) ? 0 : 1;
        }
    }
}

long WW8PLCFx_Book::GetLen() const
{
    if( nIsEnd )
    {
        OSL_ENSURE( false, "Incorrect call (1) of PLCF_Book::GetLen()" );
        return 0;
    }
    void * p;
    WW8_CP nStartPos;
    if( !pBook[0]->Get( nStartPos, p ) )
    {
        OSL_ENSURE( false, "Incorrect call (2) of PLCF_Book::GetLen()" );
        return 0;
    }
    const sal_uInt16 nEndIdx = SVBT16ToShort( *static_cast<SVBT16*>(p) );
    long nNum = pBook[1]->GetPos( nEndIdx );
    nNum -= nStartPos;
    return nNum;
}

void WW8PLCFx_Book::SetStatus(sal_uInt16 nIndex, eBookStatus eStat)
{
    SAL_WARN_IF(nIndex >= nIMax, "sw.ww8",
                "bookmark index " << nIndex << " invalid");
    eBookStatus eStatus = aStatus.at(nIndex);
    aStatus[nIndex] = static_cast<eBookStatus>(eStatus | eStat);
}

eBookStatus WW8PLCFx_Book::GetStatus() const
{
    if (aStatus.empty())
        return BOOK_NORMAL;
    long nEndIdx = GetHandle();
    return ( nEndIdx < nIMax ) ? aStatus[nEndIdx] : BOOK_NORMAL;
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
            return SVBT16ToShort( *static_cast<SVBT16 const *>(p) );
        else
            return LONG_MAX;
    }
}

OUString WW8PLCFx_Book::GetBookmark(long nStart,long nEnd, sal_uInt16 &nIndex)
{
    bool bFound = false;
    sal_uInt16 i = 0;
    if (pBook[0] && pBook[1])
    {
        WW8_CP nStartAkt, nEndAkt;
        while (i < aBookNames.size())
        {
            void* p;
            sal_uInt16 nEndIdx;

            if( pBook[0]->GetData( i, nStartAkt, p ) && p )
                nEndIdx = SVBT16ToShort( *static_cast<SVBT16*>(p) );
            else
            {
                OSL_ENSURE( false, "Bookmark-EndIdx not readable" );
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
    }
    return bFound ? aBookNames[i] : OUString();
}

OUString WW8PLCFx_Book::GetUniqueBookmarkName(const OUString &rSuggestedName)
{
    OUString aRet(rSuggestedName.isEmpty() ? OUString("Unnamed") : rSuggestedName);
    size_t i = 0;
    while (i < aBookNames.size())
    {
        if (aRet.equals(aBookNames[i]))
        {
            sal_Int32 len = aRet.getLength();
            sal_Int32 p = len - 1;
            while (p > 0 && aRet[p] >= '0' && aRet[p] <= '9')
                --p;
            aRet = aRet.copy(0, p+1) + OUString::number(nBookmarkId++);
            i = 0; // start search from beginning
        }
        else
            ++i;
    }
    return aRet;
}

bool WW8PLCFx_Book::MapName(OUString& rName)
{
    if( !pBook[0] || !pBook[1] )
        return false;

    bool bFound = false;

    size_t i = 0;
    while (i < aBookNames.size())
    {
        if (rName.equalsIgnoreAsciiCase(aBookNames[i]))
        {
            rName = aBookNames[i];
            bFound = true;
            break;
        }
        ++i;
    }
    return bFound;
}

const OUString* WW8PLCFx_Book::GetName() const
{
    const OUString *pRet = nullptr;
    if (!nIsEnd && (pBook[0]->GetIdx() < nIMax))
        pRet = &(aBookNames[pBook[0]->GetIdx()]);
    return pRet;
}

WW8PLCFx_AtnBook::WW8PLCFx_AtnBook(SvStream* pTableSt, const WW8Fib& rFib)
    : WW8PLCFx(rFib.GetFIBVersion(), /*bSprm=*/false),
    m_bIsEnd(false)
{
    if (!rFib.fcPlcfAtnbkf || !rFib.lcbPlcfAtnbkf || !rFib.fcPlcfAtnbkl || !rFib.lcbPlcfAtnbkl)
    {
        m_pBook[0] = m_pBook[1] = nullptr;
        nIMax = 0;
    }
    else
    {
        m_pBook[0] = new WW8PLCFspecial(pTableSt, rFib.fcPlcfAtnbkf, rFib.lcbPlcfAtnbkf, 4);
        m_pBook[1] = new WW8PLCFspecial(pTableSt, rFib.fcPlcfAtnbkl, rFib.lcbPlcfAtnbkl, 0);

        nIMax = m_pBook[0]->GetIMax();
        if (m_pBook[1]->GetIMax() < nIMax)
            nIMax = m_pBook[1]->GetIMax();
    }
}

WW8PLCFx_AtnBook::~WW8PLCFx_AtnBook()
{
    delete m_pBook[1];
    delete m_pBook[0];
}

sal_uInt32 WW8PLCFx_AtnBook::GetIdx() const
{
    return nIMax ? m_pBook[0]->GetIdx() : 0;
}

void WW8PLCFx_AtnBook::SetIdx( sal_uLong nI )
{
    if( nIMax )
        m_pBook[0]->SetIdx( nI );
}

sal_uLong WW8PLCFx_AtnBook::GetIdx2() const
{
    if (nIMax)
        return m_pBook[1]->GetIdx() | ( m_bIsEnd ? 0x80000000 : 0 );
    else
        return 0;
}

void WW8PLCFx_AtnBook::SetIdx2( sal_uLong nI )
{
    if( nIMax )
    {
        m_pBook[1]->SetIdx( nI & 0x7fffffff );
        m_bIsEnd = static_cast<bool>(( nI >> 31 ) & 1);
    }
}

bool WW8PLCFx_AtnBook::SeekPos(WW8_CP nCpPos)
{
    if (!m_pBook[0])
        return false;

    bool bOk = m_pBook[0]->SeekPosExact(nCpPos);
    bOk &= m_pBook[1]->SeekPosExact(nCpPos);
    m_bIsEnd = false;

    return bOk;
}

WW8_CP WW8PLCFx_AtnBook::Where()
{
    return m_pBook[static_cast<int>(m_bIsEnd)]->Where();
}

long WW8PLCFx_AtnBook::GetNoSprms( WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen )
{
    void* pData;
    rEnd = WW8_CP_MAX;
    rLen = 0;

    if (!m_pBook[0] || !m_pBook[1] || !nIMax || (m_pBook[static_cast<int>(m_bIsEnd)]->GetIdx()) >= nIMax)
    {
        rStart = rEnd = WW8_CP_MAX;
        return -1;
    }

    (void)m_pBook[static_cast<int>(m_bIsEnd)]->Get(rStart, pData);
    return m_pBook[static_cast<int>(m_bIsEnd)]->GetIdx();
}

void WW8PLCFx_AtnBook::advance()
{
    if( m_pBook[0] && m_pBook[1] && nIMax )
    {
        (*m_pBook[static_cast<int>(m_bIsEnd)]).advance();

        sal_uLong l0 = m_pBook[0]->Where();
        sal_uLong l1 = m_pBook[1]->Where();
        if( l0 < l1 )
            m_bIsEnd = false;
        else if( l1 < l0 )
            m_bIsEnd = true;
        else
        {
            const void * p = m_pBook[0]->GetData(m_pBook[0]->GetIdx());
            long nPairFor = (p == nullptr)? 0L : SVBT16ToShort(*static_cast<SVBT16 const *>(p));
            if (nPairFor == m_pBook[1]->GetIdx())
                m_bIsEnd = false;
            else
                m_bIsEnd = !m_bIsEnd;
        }
    }
}

long WW8PLCFx_AtnBook::getHandle() const
{
    if (!m_pBook[0] || !m_pBook[1])
        return LONG_MAX;

    if (m_bIsEnd)
        return m_pBook[1]->GetIdx();
    else
    {
        if (const void* p = m_pBook[0]->GetData(m_pBook[0]->GetIdx()))
            return SVBT16ToShort(*(static_cast<const SVBT16*>(p)));
        else
            return LONG_MAX;
    }
}

bool WW8PLCFx_AtnBook::getIsEnd() const
{
    return m_bIsEnd;
}

WW8PLCFx_FactoidBook::WW8PLCFx_FactoidBook(SvStream* pTableSt, const WW8Fib& rFib)
    : WW8PLCFx(rFib.GetFIBVersion(), /*bSprm=*/false),
    m_bIsEnd(false)
{
    if (!rFib.fcPlcfBkfFactoid || !rFib.lcbPlcfBkfFactoid || !rFib.fcPlcfBklFactoid || !rFib.lcbPlcfBklFactoid)
    {
        m_pBook[0] = m_pBook[1] = nullptr;
        m_nIMax = 0;
    }
    else
    {
        m_pBook[0] = new WW8PLCFspecial(pTableSt, rFib.fcPlcfBkfFactoid, rFib.lcbPlcfBkfFactoid, 6);
        m_pBook[1] = new WW8PLCFspecial(pTableSt, rFib.fcPlcfBklFactoid, rFib.lcbPlcfBklFactoid, 4);

        m_nIMax = m_pBook[0]->GetIMax();
        if (m_pBook[1]->GetIMax() < m_nIMax)
            m_nIMax = m_pBook[1]->GetIMax();
    }
}

WW8PLCFx_FactoidBook::~WW8PLCFx_FactoidBook()
{
    delete m_pBook[1];
    delete m_pBook[0];
}

sal_uInt32 WW8PLCFx_FactoidBook::GetIdx() const
{
    return m_nIMax ? m_pBook[0]->GetIdx() : 0;
}

void WW8PLCFx_FactoidBook::SetIdx(sal_uLong nI)
{
    if (m_nIMax)
        m_pBook[0]->SetIdx(nI);
}

sal_uLong WW8PLCFx_FactoidBook::GetIdx2() const
{
    if (m_nIMax)
        return m_pBook[1]->GetIdx() | (m_bIsEnd ? 0x80000000 : 0);
    else
        return 0;
}

void WW8PLCFx_FactoidBook::SetIdx2(sal_uLong nI)
{
    if (m_nIMax)
    {
        m_pBook[1]->SetIdx(nI & 0x7fffffff);
        m_bIsEnd = static_cast<bool>((nI >> 31) & 1);
    }
}

bool WW8PLCFx_FactoidBook::SeekPos(WW8_CP nCpPos)
{
    if (!m_pBook[0])
        return false;

    bool bOk = m_pBook[0]->SeekPosExact(nCpPos);
    bOk &= m_pBook[1]->SeekPosExact(nCpPos);
    m_bIsEnd = false;

    return bOk;
}

WW8_CP WW8PLCFx_FactoidBook::Where()
{
    return m_pBook[static_cast<int>(m_bIsEnd)]->Where();
}

long WW8PLCFx_FactoidBook::GetNoSprms(WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen)
{
    void* pData;
    rEnd = WW8_CP_MAX;
    rLen = 0;

    if (!m_pBook[0] || !m_pBook[1] || !m_nIMax || (m_pBook[static_cast<int>(m_bIsEnd)]->GetIdx()) >= m_nIMax)
    {
        rStart = rEnd = WW8_CP_MAX;
        return -1;
    }

    (void)m_pBook[static_cast<int>(m_bIsEnd)]->Get(rStart, pData);
    return m_pBook[static_cast<int>(m_bIsEnd)]->GetIdx();
}

void WW8PLCFx_FactoidBook::advance()
{
    if (m_pBook[0] && m_pBook[1] && m_nIMax)
    {
        (*m_pBook[static_cast<int>(m_bIsEnd)]).advance();

        sal_uLong l0 = m_pBook[0]->Where();
        sal_uLong l1 = m_pBook[1]->Where();
        if (l0 < l1)
            m_bIsEnd = false;
        else if (l1 < l0)
            m_bIsEnd = true;
        else
        {
            const void * p = m_pBook[0]->GetData(m_pBook[0]->GetIdx());
            long nPairFor = (p == nullptr)? 0L : SVBT16ToShort(*static_cast<SVBT16 const *>(p));
            if (nPairFor == m_pBook[1]->GetIdx())
                m_bIsEnd = false;
            else
                m_bIsEnd = !m_bIsEnd;
        }
    }
}

long WW8PLCFx_FactoidBook::getHandle() const
{
    if (!m_pBook[0] || !m_pBook[1])
        return LONG_MAX;

    if (m_bIsEnd)
        return m_pBook[1]->GetIdx();
    else
    {
        if (const void* p = m_pBook[0]->GetData(m_pBook[0]->GetIdx()))
            return SVBT16ToShort(*(static_cast<const SVBT16*>(p)));
        else
            return LONG_MAX;
    }
}

bool WW8PLCFx_FactoidBook::getIsEnd() const
{
    return m_bIsEnd;
}

// In the end of an paragraph in WW6 the attribute extends after the <CR>.
// This will be reset by one character to be used with SW,
// if we don't expect trouble thereby.
void WW8PLCFMan::AdjustEnds( WW8PLCFxDesc& rDesc )
{
    // might be necessary to do this for pChp and/or pSep as well,
    // but its definitely the case for paragraphs that EndPos > StartPos
    // for a well formed paragraph as those always have a paragraph
    // <cr> in them
    if (&rDesc == pPap && rDesc.bRealLineEnd)
    {
        if (rDesc.nStartPos == rDesc.nEndPos && rDesc.nEndPos != WW8_CP_MAX)
        {
            SAL_WARN("sw.ww8", "WW8PLCFxDesc End same as Start, abandoning to avoid looping");
            rDesc.nEndPos = WW8_CP_MAX;
        }
    }

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
        if ( pPap->nEndPos != WW8_CP_MAX )    // Para adjust
        {
            nLineEnd = pPap->nEndPos;// nLineEnd points *after* the <CR>
            pPap->nEndPos--;        // shorten paragraph end by one character

            // Is there already a sep end, which points to the current paragraph end?
            // Then we also must shorten by one character
            if( pSep->nEndPos == nLineEnd )
                pSep->nEndPos--;
        }
    }
    else if (&rDesc == pSep)
    {
        // Sep Adjust if end Char-Attr == paragraph end ...
        if( (rDesc.nEndPos == nLineEnd) && (rDesc.nEndPos > rDesc.nStartPos) )
            rDesc.nEndPos--;            // ... then shorten by one character
    }
}

void WW8PLCFxDesc::ReduceByOffset()
{
    SAL_WARN_IF(WW8_CP_MAX != nStartPos && nStartPos > nEndPos, "sw.ww8",
                "End " << nEndPos << " before Start " << nStartPos);

    if( nStartPos != WW8_CP_MAX )
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
    if (nEndPos != WW8_CP_MAX)
    {
        if (nCpOfs > nEndPos)
        {
            SAL_WARN("sw.ww8", "broken subdocument piece entry");
            nEndPos = WW8_CP_MAX;
        }
        else
            nEndPos -= nCpOfs;
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

    SAL_WARN_IF(WW8_CP_MAX != rDesc.nStartPos && rDesc.nStartPos > rDesc.nEndPos, "sw.ww8",
                "End " << rDesc.nEndPos << " before Start " << rDesc.nStartPos);

    rDesc.ReduceByOffset();

    rDesc.bFirstSprm = true;
    rDesc.nOrigSprmsLen = rDesc.nSprmsLen;
}

sal_uInt16 WW8PLCFMan::GetId(const WW8PLCFxDesc* p) const
{
    sal_uInt16 nId = 0;        // Id = 0 for empty attributes

    if (p == pField)
        nId = eFLD;
    else if (p == pFootnote)
        nId = eFTN;
    else if (p == pEdn)
        nId = eEDN;
    else if (p == pAnd)
        nId = eAND;
    else if (p->nSprmsLen >= maSprmParser.MinSprmLen())
        nId = maSprmParser.GetSprmId(p->pMemPos);

    return nId;
}

WW8PLCFMan::WW8PLCFMan(WW8ScannerBase* pBase, ManTypes nType, long nStartCp,
    bool bDoingDrawTextBox)
    : maSprmParser(pBase->pWw8Fib->GetFIBVersion()),
    mbDoingDrawTextBox(bDoingDrawTextBox)
{
    pWwFib = pBase->pWw8Fib;

    nLastWhereIdxCp = 0;
    memset( aD, 0, sizeof( aD ) );
    nLineEnd = WW8_CP_MAX;
    nManType = nType;

    if( MAN_MAINTEXT == nType )
    {
        // search order of the attributes
        nPLCF = MAN_ANZ_PLCF;
        pField = &aD[0];
        pBkm = &aD[1];
        pEdn = &aD[2];
        pFootnote = &aD[3];
        pAnd = &aD[4];

        pPcd = ( pBase->pPLCFx_PCD ) ? &aD[5] : nullptr;
        //pPcdA index == pPcd index + 1
        pPcdA = ( pBase->pPLCFx_PCDAttrs ) ? &aD[6] : nullptr;

        pChp = &aD[7];
        pPap = &aD[8];
        pSep = &aD[9];
        pAtnBkm = &aD[10];
        pFactoidBkm = &aD[11];

        pSep->pPLCFx = pBase->pSepPLCF;
        pFootnote->pPLCFx = pBase->pFootnotePLCF;
        pEdn->pPLCFx = pBase->pEdnPLCF;
        pBkm->pPLCFx = pBase->pBook;
        pAnd->pPLCFx = pBase->pAndPLCF;
        pAtnBkm->pPLCFx = pBase->pAtnBook;
        pFactoidBkm->pPLCFx = pBase->pFactoidBook;

    }
    else
    {
        // search order of the attributes
        nPLCF = 7;
        pField = &aD[0];
        pBkm = ( pBase->pBook ) ? &aD[1] : nullptr;

        pPcd = ( pBase->pPLCFx_PCD ) ? &aD[2] : nullptr;
        //pPcdA index == pPcd index + 1
        pPcdA= ( pBase->pPLCFx_PCDAttrs ) ? &aD[3] : nullptr;

        pChp = &aD[4];
        pPap = &aD[5];
        pSep = &aD[6]; // Dummy

        pAnd = pAtnBkm = pFactoidBkm = pFootnote = pEdn = nullptr;     // not used at SpezText
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
    pSubdocs = pBase->pSubdocs;
    pExtendedAtrds = pBase->pExtendedAtrds;

    switch( nType )                 // field initialization
    {
        case MAN_HDFT:
            pField->pPLCFx = pBase->pFieldHdFtPLCF;
            pFdoa = pBase->pHdFtFdoa;
            pTxbx = pBase->pHdFtTxbx;
            pTxbxBkd = pBase->pHdFtTxbxBkd;
            break;
        case MAN_FTN:
            pField->pPLCFx = pBase->pFieldFootnotePLCF;
            pFdoa = pTxbx = pTxbxBkd = nullptr;
            break;
        case MAN_EDN:
            pField->pPLCFx = pBase->pFieldEdnPLCF;
            pFdoa = pTxbx = pTxbxBkd = nullptr;
            break;
        case MAN_AND:
            pField->pPLCFx = pBase->pFieldAndPLCF;
            pFdoa = pTxbx = pTxbxBkd = nullptr;
            break;
        case MAN_TXBX:
            pField->pPLCFx = pBase->pFieldTxbxPLCF;
            pTxbx = pBase->pMainTxbx;
            pTxbxBkd = pBase->pMainTxbxBkd;
            pFdoa = nullptr;
            break;
        case MAN_TXBX_HDFT:
            pField->pPLCFx = pBase->pFieldTxbxHdFtPLCF;
            pTxbx = pBase->pHdFtTxbx;
            pTxbxBkd = pBase->pHdFtTxbxBkd;
            pFdoa = nullptr;
            break;
        default:
            pField->pPLCFx = pBase->pFieldPLCF;
            pFdoa = pBase->pMainFdoa;
            pTxbx = pBase->pMainTxbx;
            pTxbxBkd = pBase->pMainTxbxBkd;
            break;
    }

    WW8_CP cp = 0;
    pWwFib->GetBaseCp(nType, &cp); //TODO: check return value
    nCpO = cp;

    if( nStartCp || nCpO )
        SeekPos( nStartCp );    // adjust PLCFe at text StartPos

    // initialization to the member vars Low-Level
    GetChpPLCF()->ResetAttrStartEnd();
    GetPapPLCF()->ResetAttrStartEnd();
    for( sal_uInt16 i=0; i < nPLCF; ++i)
    {
        WW8PLCFxDesc* p = &aD[i];

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
        p->pIdStack = nullptr;

        if ((p == pChp) || (p == pPap))
            p->nStartPos = p->nEndPos = nStartCp;
        else
            p->nStartPos = p->nEndPos = WW8_CP_MAX;
    }

    // initialization to the member vars High-Level
    for( sal_uInt16 i=0; i<nPLCF; ++i){
        WW8PLCFxDesc* p = &aD[i];

        if( !p->pPLCFx )
        {
            p->nStartPos = p->nEndPos = WW8_CP_MAX;
            continue;
        }

        if( p->pPLCFx->IsSprm() )
        {
            // Careful: nEndPos must be
            p->pIdStack = new std::stack<sal_uInt16>;
            if ((p == pChp) || (p == pPap))
            {
                WW8_CP nTemp = p->nEndPos+p->nCpOfs;
                p->pMemPos = nullptr;
                p->nSprmsLen = 0;
                p->nStartPos = nTemp;
                if (!(*p->pPLCFx).SeekPos(p->nStartPos))
                    p->nEndPos = p->nStartPos = WW8_CP_MAX;
                else
                    GetNewSprms( *p );
            }
            else
                GetNewSprms( *p );      // initialized at all PLCFs
        }
        else if( p->pPLCFx )
            GetNewNoSprms( *p );
    }
}

WW8PLCFMan::~WW8PLCFMan()
{
    for( sal_uInt16 i=0; i<nPLCF; i++)
        delete aD[i].pIdStack;
}

// 0. which attr class,
// 1. if it's a attr start,
// 2. CP, where is next attr change
sal_uInt16 WW8PLCFMan::WhereIdx(bool* pbStart, long* pPos) const
{
    OSL_ENSURE(nPLCF,"What the hell");
    long nNext = LONG_MAX;  // search order:
    sal_uInt16 nNextIdx = nPLCF;// first ending found ( CHP, PAP, ( SEP ) ),
    bool bStart = true;     // now find beginnings ( ( SEP ), PAP, CHP )
    const WW8PLCFxDesc* pD;
    for (sal_uInt16 i=0; i < nPLCF; ++i)
    {
        pD = &aD[i];
        if (pD != pPcdA)
        {
            if( (pD->nEndPos < nNext) && (pD->nStartPos == WW8_CP_MAX) )
            {
                // otherwise start = end
                nNext = pD->nEndPos;
                nNextIdx = i;
                bStart = false;
            }
        }
    }
    for (sal_uInt16 i=nPLCF; i > 0; --i)
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

// gives the CP pos of the next attr change
WW8_CP WW8PLCFMan::Where() const
{
    long l;
    WhereIdx(nullptr, &l);
    return l;
}

void WW8PLCFMan::SeekPos( long nNewCp )
{
    pChp->pPLCFx->SeekPos( nNewCp + nCpO ); // create new attr
    pPap->pPLCFx->SeekPos( nNewCp + nCpO );
    pField->pPLCFx->SeekPos( nNewCp );
    if( pPcd )
        pPcd->pPLCFx->SeekPos( nNewCp + nCpO );
    if( pBkm )
        pBkm->pPLCFx->SeekPos( nNewCp + nCpO );
}

void WW8PLCFMan::SaveAllPLCFx( WW8PLCFxSaveAll& rSave ) const
{
    sal_uInt16 n=0;
    if( pPcd )
        pPcd->Save(  rSave.aS[n++] );
    if( pPcdA )
        pPcdA->Save( rSave.aS[n++] );

    for(sal_uInt16 i=0; i<nPLCF; ++i)
        if( pPcd != &aD[i] && pPcdA != &aD[i] )
            aD[i].Save( rSave.aS[n++] );
}

void WW8PLCFMan::RestoreAllPLCFx( const WW8PLCFxSaveAll& rSave )
{
    sal_uInt16 n=0;
    if( pPcd )
        pPcd->Restore(  rSave.aS[n++] );
    if( pPcdA )
        pPcdA->Restore( rSave.aS[n++] );

    for(sal_uInt16 i=0; i<nPLCF; ++i)
        if( pPcd != &aD[i] && pPcdA != &aD[i] )
            aD[i].Restore( rSave.aS[n++] );
}

void WW8PLCFMan::GetSprmStart( short nIdx, WW8PLCFManResult* pRes ) const
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );

    // verifying !!!

    pRes->nMemLen = 0;

    const WW8PLCFxDesc* p = &aD[nIdx];

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
    if ((p == pFootnote) || (p == pEdn) || (p == pAnd))
        pRes->nMemLen = p->nSprmsLen;
    else if (p->nSprmsLen >= maSprmParser.MinSprmLen()) //normal
    {
        // Length of actual sprm
        pRes->nMemLen = maSprmParser.GetSprmSize(pRes->nSprmId, pRes->pMemPos);
    }
}

void WW8PLCFMan::GetSprmEnd( short nIdx, WW8PLCFManResult* pRes ) const
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );

    const WW8PLCFxDesc* p = &aD[nIdx];

    if (!(p->pIdStack->empty()))
        pRes->nSprmId = p->pIdStack->top();       // get end position
    else
    {
        OSL_ENSURE( false, "No Id on the Stack" );
        pRes->nSprmId = 0;
    }
}

void WW8PLCFMan::GetNoSprmStart( short nIdx, WW8PLCFManResult* pRes ) const
{
    const WW8PLCFxDesc* p = &aD[nIdx];

    pRes->nCpPos = p->nStartPos;
    pRes->nMemLen = p->nSprmsLen;
    pRes->nCp2OrIdx = p->nCp2OrIdx;

    if( p == pField )
        pRes->nSprmId = eFLD;
    else if( p == pFootnote )
        pRes->nSprmId = eFTN;
    else if( p == pEdn )
        pRes->nSprmId = eEDN;
    else if( p == pBkm )
        pRes->nSprmId = eBKN;
    else if (p == pAtnBkm)
        pRes->nSprmId = eATNBKN;
    else if (p == pFactoidBkm)
        pRes->nSprmId = eFACTOIDBKN;
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
    pRes->nMemLen = -1;     // end tag

    if( &aD[nIdx] == pBkm )
        pRes->nSprmId = eBKN;
    else if (&aD[nIdx] == pAtnBkm)
        pRes->nSprmId = eATNBKN;
    else if (&aD[nIdx] == pFactoidBkm)
        pRes->nSprmId = eFACTOIDBKN;
    else if( &aD[nIdx] == pPcd )
    {
        //We slave the piece table attributes to the piece table, the piece
        //table attribute iterator contains the sprms for this piece.
        GetSprmEnd( nIdx+1, pRes );
    }
    else
        pRes->nSprmId = 0;
}

bool WW8PLCFMan::TransferOpenSprms(std::stack<sal_uInt16> &rStack)
{
    for (sal_uInt16 i = 0; i < nPLCF; ++i)
    {
        WW8PLCFxDesc* p = &aD[i];
        if (!p || !p->pIdStack)
            continue;
        while (!p->pIdStack->empty())
        {
            rStack.push(p->pIdStack->top());
            p->pIdStack->pop();
        }
    }
    return rStack.empty();
}

void WW8PLCFMan::AdvSprm(short nIdx, bool bStart)
{
    WW8PLCFxDesc* p = &aD[nIdx];    // determine sprm class(!)

    p->bFirstSprm = false;
    if( bStart )
    {
        const sal_uInt16 nLastId = GetId(p);
        p->pIdStack->push(nLastId);   // remember Id for attribute end

        if( p->nSprmsLen )
        {   /*
                Check, if we have to process more sprm(s).
            */
            if( p->pMemPos )
            {
                // Length of last sprm
                const sal_uInt16 nSprmL = maSprmParser.GetSprmSize(nLastId, p->pMemPos);

                // Reduce length of all sprms by length of last sprm
                p->nSprmsLen -= nSprmL;

                // pos of next possible sprm
                if (p->nSprmsLen < maSprmParser.MinSprmLen())
                {
                    // preventively set to 0, because the end follows!
                    p->pMemPos = nullptr;
                    p->nSprmsLen = 0;
                }
                else
                    p->pMemPos += nSprmL;
            }
            else
                p->nSprmsLen = 0;
        }
        if (p->nSprmsLen < maSprmParser.MinSprmLen())
            p->nStartPos = WW8_CP_MAX;    // the ending follows
    }
    else
    {
        if (!(p->pIdStack->empty()))
            p->pIdStack->pop();
        if (p->pIdStack->empty())
        {
            if ( (p == pChp) || (p == pPap) )
            {
                p->pMemPos = nullptr;
                p->nSprmsLen = 0;
                p->nStartPos = p->nOrigEndPos+p->nCpOfs;

                /*
                On failed seek we have run out of sprms, probably.  But if its
                a fastsaved file (has pPcd) then we may be just in a sprm free
                gap between pieces that have them, so set dirty flag in sprm
                finder to consider than.
                */
                if (!(*p->pPLCFx).SeekPos(p->nStartPos))
                {
                    p->nEndPos = WW8_CP_MAX;
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
                and the existence of a clipstart will force the pPcd iterator
                to reread the current set of sprms instead of advancing to its
                next set. Then the clipstart will be set as the starting
                position which will force them to be applied directly after
                the pap and chps.
                */
                if (pPcd && ((p->nStartPos > pPcd->nStartPos) ||
                    (pPcd->nStartPos == WW8_CP_MAX)) &&
                    (pPcd->nEndPos != p->nStartPos))
                {
                    pPcd->nEndPos = p->nStartPos;
                    static_cast<WW8PLCFx_PCD *>(pPcd->pPLCFx)->SetClipStart(
                        p->nStartPos);
                }

            }
            else
            {
                p->pPLCFx->advance(); // next Group of Sprms
                p->pMemPos = nullptr;       // !!!
                p->nSprmsLen = 0;
                GetNewSprms( *p );
            }
            SAL_WARN_IF(p->nStartPos > p->nEndPos, "sw.ww8",
                        "End " << p->nEndPos << " before Start " << p->nStartPos);
        }
    }
}

void WW8PLCFMan::AdvNoSprm(short nIdx, bool bStart)
{
    /*
    For the case of a piece table we slave the piece table attribute iterator
    to the piece table and access it through that only. They are two separate
    structures, but act together as one logical one. The attributes only go
    to the next entry when the piece changes
    */
    WW8PLCFxDesc* p = &aD[nIdx];

    if( p == pPcd )
    {
        AdvSprm(nIdx+1,bStart);
        if( bStart )
            p->nStartPos = aD[nIdx+1].nStartPos;
        else
        {
            if (aD[nIdx+1].pIdStack->empty())
            {
                WW8PLCFx_PCD *pTemp = static_cast<WW8PLCFx_PCD*>(pPcd->pPLCFx);
                /*
                #i2325#
                As per normal, go on to the next set of properties, i.e. we
                have traversed over to the next piece.  With a clipstart set
                we are being told to reread the current piece sprms so as to
                reapply them to a new chp or pap range.
                */
                if (pTemp->GetClipStart() == -1)
                    p->pPLCFx->advance();
                p->pMemPos = nullptr;
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
    {                                  // NoSprm without end
        p->pPLCFx->advance();
        p->pMemPos = nullptr;                     // MemPos invalid
        p->nSprmsLen = 0;
        GetNewNoSprms( *p );
    }
}

void WW8PLCFMan::advance()
{
    bool bStart;
    const sal_uInt16 nIdx = WhereIdx(&bStart);
    if (nIdx < nPLCF)
    {
        WW8PLCFxDesc* p = &aD[nIdx];

        p->bFirstSprm = true;                       // Default

        if( p->pPLCFx->IsSprm() )
            AdvSprm( nIdx, bStart );
        else                                        // NoSprm
            AdvNoSprm( nIdx, bStart );
    }
}

// return true for the beginning of an attribute or error,
//           false for the end of an attribute
// remaining return values are delivered to the caller from WW8PclxManResults.
bool WW8PLCFMan::Get(WW8PLCFManResult* pRes) const
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );
    bool bStart;
    const sal_uInt16 nIdx = WhereIdx(&bStart);

    if( nIdx >= nPLCF )
    {
        OSL_ENSURE( false, "Position not found" );
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

sal_uInt16 WW8PLCFMan::GetColl() const
{
    if( pPap->pPLCFx )
        return  pPap->pPLCFx->GetIstd();
    else
    {
        OSL_ENSURE( false, "GetColl without PLCF_Pap" );
        return 0;
    }
}

WW8PLCFx_FLD* WW8PLCFMan::GetField() const
{
    return static_cast<WW8PLCFx_FLD*>(pField->pPLCFx);
}

const sal_uInt8* WW8PLCFMan::HasParaSprm( sal_uInt16 nId ) const
{
    return static_cast<WW8PLCFx_Cp_FKP*>(pPap->pPLCFx)->HasSprm( nId );
}

const sal_uInt8* WW8PLCFMan::HasCharSprm( sal_uInt16 nId ) const
{
    return static_cast<WW8PLCFx_Cp_FKP*>(pChp->pPLCFx)->HasSprm( nId );
}

bool WW8PLCFMan::HasCharSprm(sal_uInt16 nId,
    std::vector<const sal_uInt8 *> &rResult) const
{
    return static_cast<WW8PLCFx_Cp_FKP*>(pChp->pPLCFx)->HasSprm(nId, rResult);
}

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

sal_uLong WW8PLCFx_Cp_FKP::GetIdx2() const
{
    return GetPCDIdx();
}

void WW8PLCFx_Cp_FKP::SetIdx2( sal_uLong nIdx )
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
                aD.nEndPos = WW8_CP_MAX;
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
                aD.nEndPos = WW8_CP_MAX;
                pPLCFx->SetDirty(true);
            }
            pPLCFx->GetSprms(&aD);
            pPLCFx->SetDirty(false);
            aD.ReduceByOffset();
            pMemPos = aD.pMemPos + rSave.nPLCFxMemOfs;
            nSprmsLen = nOrigSprmsLen - rSave.nPLCFxMemOfs;
        }
    }
}

namespace
{
    sal_uInt32 Readcb(SvStream& rSt, ww::WordVersion eVer)
    {
        if (eVer <= ww::eWW2)
        {
            sal_uInt16 nShort(0);
            rSt.ReadUInt16(nShort);
            return nShort;
        }
        else
        {
            sal_uInt32 nLong(0);
            rSt.ReadUInt32(nLong);
            return nLong;
        }
    }
}

bool WW8Fib::GetBaseCp(ManTypes nType, WW8_CP * cp) const
{
    assert(cp != nullptr);
    WW8_CP nOffset = 0;

    switch( nType )
    {
        default:
        case MAN_TXBX_HDFT:
            nOffset = ccpTxbx;
            // fall through
        case MAN_TXBX:
            if (ccpEdn > std::numeric_limits<WW8_CP>::max() - nOffset) {
                return false;
            }
            nOffset += ccpEdn;
            // fall through
        case MAN_EDN:
            if (ccpAtn > std::numeric_limits<WW8_CP>::max() - nOffset) {
                return false;
            }
            nOffset += ccpAtn;
            // fall through
        case MAN_AND:
            if (ccpMcr > std::numeric_limits<WW8_CP>::max() - nOffset) {
                return false;
            }
            nOffset += ccpMcr;
        /*
            // fall through

         A subdocument of this kind (MAN_MACRO) probably exists in some defunct
         version of MSWord, but now ccpMcr is always 0. If some example that
         uses this comes to light, this is the likely calculation required

        case MAN_MACRO:
        */
            if (ccpHdr > std::numeric_limits<WW8_CP>::max() - nOffset) {
                return false;
            }
            nOffset += ccpHdr;
            // fall through
        case MAN_HDFT:
            if (ccpFootnote > std::numeric_limits<WW8_CP>::max() - nOffset) {
                return false;
            }
            nOffset += ccpFootnote;
            // fall through
        case MAN_FTN:
            if (ccpText > std::numeric_limits<WW8_CP>::max() - nOffset) {
                return false;
            }
            nOffset += ccpText;
            // fall through
        case MAN_MAINTEXT:
            break;
    }
    *cp = nOffset;
    return true;
}

ww::WordVersion WW8Fib::GetFIBVersion() const
{
    ww::WordVersion eVer = ww::eWW8;
    /*
     * Word for Windows 2 I think (1.X might work too if anyone has an example.
     *
     * 0xA59B for Word 1 for Windows
     * 0xA59C for Word 1 for OS/2 "PM Word"
     *
     * Various pages claim that the fileformats of Word 1 and 2 for Windows are
     * equivalent to Word for Macintosh 4 and 5. On the other hand
     *
     * wIdents for Word for Mac versions...
     * 0xFE32 for Word 1
     * 0xFE34 for Word 3
     * 0xFE37 for Word 4 et 5.
     *
     * and this document
     * http://cmsdoc.cern.ch/documents/docformat/CMS_CERN_LetterHead.word is
     * claimed to be "Word 5 for Mac" by Office etc and has that wIdent, but
     * its format isn't the same as that of Word 2 for windows. Nor is it
     * the same as that of Word for DOS/PCWord 5
     */
    if (wIdent == 0xa59b || wIdent == 0xa59c)
        eVer = ww::eWW1;
    else if (wIdent == 0xa5db)
        eVer = ww::eWW2;
    else
    {
        switch (nVersion)
        {
            case 6:
                eVer = ww::eWW6;
                break;
            case 7:
                eVer = ww::eWW7;
                break;
            case 8:
                eVer = ww::eWW8;
                break;
        }
    }
    return eVer;
}

WW8Fib::WW8Fib(SvStream& rSt, sal_uInt8 nWantedVersion, sal_uInt32 nOffset)
    : nFibError( 0 )
{
    memset(this, 0, sizeof(*this));
    sal_uInt8 aBits1;
    sal_uInt8 aBits2;
    sal_uInt8 aVer8Bits1;    // only used starting with WinWord 8
    rSt.Seek( nOffset );
    /*
        note desired number, identify file version number
        and check against desired number!
    */
    nVersion = nWantedVersion;
    rSt.ReadUInt16( wIdent );
    rSt.ReadUInt16( nFib );
    rSt.ReadUInt16( nProduct );
    if( 0 != rSt.GetError() )
    {
        sal_Int16 nFibMin;
        sal_Int16 nFibMax;
        // note: 6 stands for "6 OR 7",  7 stands for "ONLY 7"
        switch( nVersion )
        {
            case 6:
                nFibMin = 0x0065;   // from 101 WinWord 6.0
                                    //     102    "
                                    // and 103 WinWord 6.0 for Macintosh
                                    //     104    "
                nFibMax = 0x0069;   // to 105 WinWord 95
                break;
            case 7:
                nFibMin = 0x0069;   // from 105 WinWord 95
                nFibMax = 0x0069;   // to 105 WinWord 95
                break;
            case 8:
                nFibMin = 0x006A;   // from 106 WinWord 97
                nFibMax = 0x00c1;   // to 193 WinWord 97 (?)
                break;
            default:
                nFibMin = 0;            // programm error!
                nFibMax = 0;
                nFib    = 1;
                OSL_ENSURE( false, "nVersion not implemented!" );
                break;
        }
        if ( (nFib < nFibMin) || (nFib > nFibMax) )
        {
            nFibError = ERR_SWG_READ_ERROR; // report error
            return;
        }
    }

    ww::WordVersion eVer = GetFIBVersion();

    // helper vars for Ver67:
    sal_Int16 pnChpFirst_Ver67=0;
    sal_Int16 pnPapFirst_Ver67=0;
    sal_Int16 cpnBteChp_Ver67=0;
    sal_Int16 cpnBtePap_Ver67=0;

    // read FIB
    rSt.ReadInt16( lid );
    rSt.ReadInt16( pnNext );
    rSt.ReadUChar( aBits1 );
    rSt.ReadUChar( aBits2 );
    rSt.ReadUInt16( nFibBack );
    rSt.ReadUInt16( nHash );
    rSt.ReadUInt16( nKey );
    rSt.ReadUChar( envr );
    rSt.ReadUChar( aVer8Bits1 );      // only have an empty reserve field under Ver67
                            // content from aVer8Bits1

                            // sal_uInt8 fMac              :1;
                            // sal_uInt8 fEmptySpecial     :1;
                            // sal_uInt8 fLoadOverridePage :1;
                            // sal_uInt8 fFuturesavedUndo  :1;
                            // sal_uInt8 fWord97Saved      :1;
                            // sal_uInt8 :3;
    rSt.ReadUInt16( chse );
    rSt.ReadUInt16( chseTables );
    rSt.ReadInt32( fcMin );
    rSt.ReadInt32( fcMac );

// insertion for WW8
    if (IsEightPlus(eVer))
    {
        rSt.ReadUInt16( csw );

        // Marke: "rgsw"  Beginning of the array of shorts
        rSt.ReadUInt16( wMagicCreated );
        rSt.ReadUInt16( wMagicRevised );
        rSt.ReadUInt16( wMagicCreatedPrivate );
        rSt.ReadUInt16( wMagicRevisedPrivate );
        rSt.SeekRel( 9 * sizeof( sal_Int16 ) );

        /*
        // these are the 9 unused fields:
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
        rSt.ReadInt16( lidFE );
        rSt.ReadUInt16( clw );
    }

// end of the insertion for WW8

        // Marke: "rglw"  Beginning of the array of longs
    rSt.ReadInt32( cbMac );

        // ignore 2 longs, because they are unimportant
    rSt.SeekRel( 2 * sizeof( sal_Int32) );

        // skipping 2 more longs only at Ver67
    if (IsSevenMinus(eVer))
        rSt.SeekRel( 2 * sizeof( sal_Int32) );

    rSt.ReadInt32( ccpText );
    rSt.ReadInt32( ccpFootnote );
    rSt.ReadInt32( ccpHdr );
    rSt.ReadInt32( ccpMcr );
    rSt.ReadInt32( ccpAtn );
    rSt.ReadInt32( ccpEdn );
    rSt.ReadInt32( ccpTxbx );
    rSt.ReadInt32( ccpHdrTxbx );

        // only skip one more long at Ver67
    if (IsSevenMinus(eVer))
        rSt.SeekRel( 1 * sizeof( sal_Int32) );
    else
    {
// insertion for WW8
        rSt.ReadInt32( pnFbpChpFirst );
        rSt.ReadInt32( pnChpFirst );
        rSt.ReadInt32( cpnBteChp );
        rSt.ReadInt32( pnFbpPapFirst );
        rSt.ReadInt32( pnPapFirst );
        rSt.ReadInt32( cpnBtePap );
        rSt.ReadInt32( pnFbpLvcFirst );
        rSt.ReadInt32( pnLvcFirst );
        rSt.ReadInt32( cpnBteLvc );
        rSt.ReadInt32( fcIslandFirst );
        rSt.ReadInt32( fcIslandLim );
        rSt.ReadUInt16( cfclcb );
    }

// end of the insertion for WW8

    // Marke: "rgfclcb" Beginning of array of FC/LCB pairs.
    rSt.ReadInt32( fcStshfOrig );
    lcbStshfOrig = Readcb(rSt, eVer);
    rSt.ReadInt32( fcStshf );
    lcbStshf = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffndRef );
    lcbPlcffndRef = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffndText );
    lcbPlcffndText = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfandRef );
    lcbPlcfandRef = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfandText );
    lcbPlcfandText = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfsed );
    lcbPlcfsed = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfpad );
    lcbPlcfpad = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfphe );
    lcbPlcfphe = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfglsy );
    lcbSttbfglsy = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfglsy );
    lcbPlcfglsy = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfhdd );
    lcbPlcfhdd = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfbteChpx );
    lcbPlcfbteChpx = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfbtePapx );
    lcbPlcfbtePapx = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfsea );
    lcbPlcfsea = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfffn );
    lcbSttbfffn = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffldMom );
    lcbPlcffldMom = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffldHdr );
    lcbPlcffldHdr = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffldFootnote );
    lcbPlcffldFootnote = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffldAtn );
    lcbPlcffldAtn = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffldMcr );
    lcbPlcffldMcr = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfbkmk );
    lcbSttbfbkmk = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfbkf );
    lcbPlcfbkf = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfbkl );
    lcbPlcfbkl = Readcb(rSt, eVer);
    rSt.ReadInt32( fcCmds );
    lcbCmds = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfmcr );
    lcbPlcfmcr = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfmcr );
    lcbSttbfmcr = Readcb(rSt, eVer);
    if (eVer >= ww::eWW2)
    {
        rSt.ReadInt32( fcPrDrvr );
        lcbPrDrvr = Readcb(rSt, eVer);
        rSt.ReadInt32( fcPrEnvPort );
        lcbPrEnvPort = Readcb(rSt, eVer);
        rSt.ReadInt32( fcPrEnvLand );
        lcbPrEnvLand = Readcb(rSt, eVer);
    }
    else
    {
        rSt.ReadInt32( fcPrEnvPort );
        lcbPrEnvPort = Readcb(rSt, eVer);
    }
    rSt.ReadInt32( fcWss );
    lcbWss = Readcb(rSt, eVer);
    rSt.ReadInt32( fcDop );
    lcbDop = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfAssoc );
    lcbSttbfAssoc = Readcb(rSt, eVer);
    rSt.ReadInt32( fcClx );
    lcbClx = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfpgdFootnote );
    lcbPlcfpgdFootnote = Readcb(rSt, eVer);
    rSt.ReadInt32( fcAutosaveSource );
    lcbAutosaveSource = Readcb(rSt, eVer);
    rSt.ReadInt32( fcGrpStAtnOwners );
    lcbGrpStAtnOwners = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfAtnbkmk );
    lcbSttbfAtnbkmk = Readcb(rSt, eVer);

    // only skip more shot at Ver67
    if (IsSevenMinus(eVer))
    {
        if (eVer == ww::eWW1)
            rSt.SeekRel(1*sizeof(sal_Int32));
        rSt.SeekRel(1*sizeof(sal_Int16));

        if (eVer >= ww::eWW2)
        {
            rSt.ReadInt16(pnChpFirst_Ver67);
            rSt.ReadInt16(pnPapFirst_Ver67);
        }
        rSt.ReadInt16(cpnBteChp_Ver67);
        rSt.ReadInt16(cpnBtePap_Ver67);
    }

    if (eVer > ww::eWW2)
    {
        rSt.ReadInt32( fcPlcfdoaMom );
        rSt.ReadInt32( lcbPlcfdoaMom );
        rSt.ReadInt32( fcPlcfdoaHdr );
        rSt.ReadInt32( lcbPlcfdoaHdr );
        rSt.ReadInt32( fcPlcfspaMom );
        rSt.ReadInt32( lcbPlcfspaMom );
        rSt.ReadInt32( fcPlcfspaHdr );
        rSt.ReadInt32( lcbPlcfspaHdr );

        rSt.ReadInt32( fcPlcfAtnbkf );
        rSt.ReadInt32( lcbPlcfAtnbkf );
        rSt.ReadInt32( fcPlcfAtnbkl );
        rSt.ReadInt32( lcbPlcfAtnbkl );
        rSt.ReadInt32( fcPms );
        rSt.ReadInt32( lcbPMS );
        rSt.ReadInt32( fcFormFieldSttbf );
        rSt.ReadInt32( lcbFormFieldSttbf );
        rSt.ReadInt32( fcPlcfendRef );
        rSt.ReadInt32( lcbPlcfendRef );
        rSt.ReadInt32( fcPlcfendText );
        rSt.ReadInt32( lcbPlcfendText );
        rSt.ReadInt32( fcPlcffldEdn );
        rSt.ReadInt32( lcbPlcffldEdn );
        rSt.ReadInt32( fcPlcfpgdEdn );
        rSt.ReadInt32( lcbPlcfpgdEdn );
        rSt.ReadInt32( fcDggInfo );
        rSt.ReadInt32( lcbDggInfo );
        rSt.ReadInt32( fcSttbfRMark );
        rSt.ReadInt32( lcbSttbfRMark );
        rSt.ReadInt32( fcSttbfCaption );
        rSt.ReadInt32( lcbSttbfCaption );
        rSt.ReadInt32( fcSttbAutoCaption );
        rSt.ReadInt32( lcbSttbAutoCaption );
        rSt.ReadInt32( fcPlcfwkb );
        rSt.ReadInt32( lcbPlcfwkb );
        rSt.ReadInt32( fcPlcfspl );
        rSt.ReadInt32( lcbPlcfspl );
        rSt.ReadInt32( fcPlcftxbxText );
        rSt.ReadInt32( lcbPlcftxbxText );
        rSt.ReadInt32( fcPlcffldTxbx );
        rSt.ReadInt32( lcbPlcffldTxbx );
        rSt.ReadInt32( fcPlcfHdrtxbxText );
        rSt.ReadInt32( lcbPlcfHdrtxbxText );
        rSt.ReadInt32( fcPlcffldHdrTxbx );
        rSt.ReadInt32( lcbPlcffldHdrTxbx );
        rSt.ReadInt32( fcStwUser );
        rSt.ReadUInt32( lcbStwUser );
        rSt.ReadInt32( fcSttbttmbd );
        rSt.ReadUInt32( lcbSttbttmbd );
    }

    if( 0 == rSt.GetError() )
    {
        // set bit flag
        fDot        =   aBits1 & 0x01       ;
        fGlsy       = ( aBits1 & 0x02 ) >> 1;
        fComplex    = ( aBits1 & 0x04 ) >> 2;
        fHasPic     = ( aBits1 & 0x08 ) >> 3;
        cQuickSaves = ( aBits1 & 0xf0 ) >> 4;
        fEncrypted  =   aBits2 & 0x01       ;
        fWhichTableStm= ( aBits2 & 0x02 ) >> 1;
        fReadOnlyRecommended = (aBits2 & 0x4) >> 2;
        fWriteReservation = (aBits2 & 0x8) >> 3;
        fExtChar    = ( aBits2 & 0x10 ) >> 4;
        // dummy    = ( aBits2 & 0x20 ) >> 5;
        fFarEast    = ( aBits2 & 0x40 ) >> 6; // #i90932#
        // dummy    = ( aBits2 & 0x80 ) >> 7;

        /*
            p.r.n. fill targeted variable with xxx_Ver67
            or set flags
        */
        if (IsSevenMinus(eVer))
        {
            pnChpFirst = pnChpFirst_Ver67;
            pnPapFirst = pnPapFirst_Ver67;
            cpnBteChp = cpnBteChp_Ver67;
            cpnBtePap = cpnBtePap_Ver67;
        }
        else if (IsEightPlus(eVer))
        {
          fMac              =   aVer8Bits1  & 0x01           ;
          fEmptySpecial     = ( aVer8Bits1  & 0x02 ) >> 1;
          fLoadOverridePage = ( aVer8Bits1  & 0x04 ) >> 2;
          fFuturesavedUndo  = ( aVer8Bits1  & 0x08 ) >> 3;
          fWord97Saved      = ( aVer8Bits1  & 0x10 ) >> 4;
          fWord2000Saved    = ( aVer8Bits1  & 0x20 ) >> 5;

            /*
                especially for WW8:
                identify the values for PLCF and PLF LFO
                and PLCF for the textbox break descriptors
            */
            long nOldPos = rSt.Tell();

            rSt.Seek( 0x02da );
            rSt.ReadInt32( fcSttbFnm );
            rSt.ReadInt32( lcbSttbFnm );
            rSt.ReadInt32( fcPlcfLst );
            rSt.ReadInt32( lcbPlcfLst );
            rSt.ReadInt32( fcPlfLfo );
            rSt.ReadInt32( lcbPlfLfo );
            rSt.ReadInt32( fcPlcftxbxBkd );
            rSt.ReadInt32( lcbPlcftxbxBkd );
            rSt.ReadInt32( fcPlcfHdrtxbxBkd );
            rSt.ReadInt32( lcbPlcfHdrtxbxBkd );
            if( 0 != rSt.GetError() )
            {
                nFibError = ERR_SWG_READ_ERROR;
            }

            rSt.Seek( 0x372 );          // fcSttbListNames
            rSt.ReadInt32( fcSttbListNames );
            rSt.ReadInt32( lcbSttbListNames );

            if (cfclcb > 93)
            {
                rSt.Seek( 0x382 );          // MagicTables
                rSt.ReadInt32( fcPlcfTch );
                rSt.ReadInt32( lcbPlcfTch );
            }

            if (cfclcb > 113)
            {
                rSt.Seek( 0x41A );          // new ATRD
                rSt.ReadInt32( fcAtrdExtra );
                rSt.ReadUInt32( lcbAtrdExtra );
            }

            // Factoid bookmarks
            if (cfclcb > 134)
            {
                rSt.Seek(0x432);
                rSt.ReadInt32(fcPlcfBkfFactoid);
                rSt.ReadUInt32(lcbPlcfBkfFactoid);

                rSt.Seek(0x442);
                rSt.ReadInt32(fcPlcfBklFactoid);
                rSt.ReadUInt32(lcbPlcfBklFactoid);

                rSt.Seek(0x44a);
                rSt.ReadInt32(fcFactoidData);
                rSt.ReadUInt32(lcbFactoidData);
            }

            if( 0 != rSt.GetError() )
                nFibError = ERR_SWG_READ_ERROR;

            rSt.Seek( 0x5bc );          // Actual nFib introduced in Word 2003
            rSt.ReadUInt16( nFib_actual );

            rSt.Seek( nOldPos );
        }
    }
    else
    {
        nFibError = ERR_SWG_READ_ERROR;     // report error
    }
}

WW8Fib::WW8Fib(sal_uInt8 nVer, bool bDot)
{
    memset(this, 0, sizeof(*this));
    nVersion = nVer;
    if (8 == nVer)
    {
        fcMin = 0x800;
        wIdent = 0xa5ec;
        nFib = 0x0101;
        nFibBack = 0xbf;
        nProduct = 0x204D;
        fDot = bDot;

        csw = 0x0e;     // Is this really necessary???
        cfclcb = 0x88;  //      -""-
        clw = 0x16;     //      -""-
        pnFbpChpFirst = pnFbpPapFirst = pnFbpLvcFirst = 0x000fffff;
        fExtChar = true;
        fWord97Saved = fWord2000Saved = true;

        wMagicCreated = 0x6143;
        wMagicRevised = 0x6C6F;
        wMagicCreatedPrivate = 0x6E61;
        wMagicRevisedPrivate = 0x3038;
    }
    else
    {
        fcMin = 0x300;
        wIdent = 0xa5dc;
        nFib = nFibBack = 0x65;
        nProduct = 0xc02d;
    }

    //If nFib is 0x00D9 or greater, then cQuickSaves MUST be 0xF
    cQuickSaves = nFib >= 0x00D9 ? 0xF : 0;

    // --> #i90932#
    lid = 0x409; // LANGUAGE_ENGLISH_US

    LanguageType nLang = Application::GetSettings().GetLanguageTag().getLanguageType();
    fFarEast = MsLangId::isCJK(nLang);
    if (fFarEast)
        lidFE = nLang;
    else
        lidFE = lid;

    LanguageTag aLanguageTag( lid );
    LocaleDataWrapper aLocaleWrapper( aLanguageTag );
    nNumDecimalSep = aLocaleWrapper.getNumDecimalSep()[0];
}


bool WW8Fib::WriteHeader(SvStream& rStrm)
{
    bool bVer8 = 8 == nVersion;

    size_t nUnencryptedHdr = bVer8 ? 0x44 : 0x24;
    sal_uInt8 *pDataPtr = new sal_uInt8[ nUnencryptedHdr ];
    sal_uInt8 *pData = pDataPtr;
    memset( pData, 0, nUnencryptedHdr );

    sal_uLong nPos = rStrm.Tell();
    cbMac = rStrm.Seek( STREAM_SEEK_TO_END );
    rStrm.Seek( nPos );

    Set_UInt16( pData, wIdent );
    Set_UInt16( pData, nFib );
    Set_UInt16( pData, nProduct );
    Set_UInt16( pData, lid );
    Set_UInt16( pData, pnNext );

    sal_uInt16 nBits16 = 0;
    if( fDot )          nBits16 |= 0x0001;
    if( fGlsy)          nBits16 |= 0x0002;
    if( fComplex )      nBits16 |= 0x0004;
    if( fHasPic )       nBits16 |= 0x0008;
    nBits16 |= (0xf0 & ( cQuickSaves << 4 ));
    if( fEncrypted )    nBits16 |= 0x0100;
    if( fWhichTableStm )  nBits16 |= 0x0200;

    if (fReadOnlyRecommended)
        nBits16 |= 0x0400;
    if (fWriteReservation)
        nBits16 |= 0x0800;

    if( fExtChar )      nBits16 |= 0x1000;
    if( fFarEast )      nBits16 |= 0x4000;  // #i90932#
    if( fObfuscated )   nBits16 |= 0x8000;
    Set_UInt16( pData, nBits16 );

    Set_UInt16( pData, nFibBack );
    Set_UInt16( pData, nHash );
    Set_UInt16( pData, nKey );
    Set_UInt8( pData, envr );

    sal_uInt8 nBits8 = 0;
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

// insertion for WW8

    // Marke: "rgsw"  Beginning of the array of shorts
    if( bVer8 )
    {
        Set_UInt16( pData, csw );
        Set_UInt16( pData, wMagicCreated );
        Set_UInt16( pData, wMagicRevised );
        Set_UInt16( pData, wMagicCreatedPrivate );
        Set_UInt16( pData, wMagicRevisedPrivate );
        pData += 9 * sizeof( sal_Int16 );
        Set_UInt16( pData, lidFE );
        Set_UInt16( pData, clw );
    }

// end of the insertion for WW8

    // Marke: "rglw"  Beginning of the array of longs
    Set_UInt32( pData, cbMac );

    rStrm.Write( pDataPtr, nUnencryptedHdr );
    delete[] pDataPtr;
    return 0 == rStrm.GetError();
}

bool WW8Fib::Write(SvStream& rStrm)
{
    bool bVer8 = 8 == nVersion;

    WriteHeader( rStrm );

    size_t nUnencryptedHdr = bVer8 ? 0x44 : 0x24;

    sal_uInt8 *pDataPtr = new sal_uInt8[ fcMin - nUnencryptedHdr ];
    sal_uInt8 *pData = pDataPtr;
    memset( pData, 0, fcMin - nUnencryptedHdr );

    sal_uLong nPos = rStrm.Tell();
    cbMac = rStrm.Seek( STREAM_SEEK_TO_END );
    rStrm.Seek( nPos );

    // ignore 2 longs, because they are unimportant
    pData += 2 * sizeof( sal_Int32);

    // skipping 2 more longs only at Ver67
    if( !bVer8 )
        pData += 2 * sizeof( sal_Int32);

    Set_UInt32( pData, ccpText );
    Set_UInt32( pData, ccpFootnote );
    Set_UInt32( pData, ccpHdr );
    Set_UInt32( pData, ccpMcr );
    Set_UInt32( pData, ccpAtn );
    Set_UInt32( pData, ccpEdn );
    Set_UInt32( pData, ccpTxbx );
    Set_UInt32( pData, ccpHdrTxbx );

        // only skip one more long at Ver67
    if( !bVer8 )
        pData += 1 * sizeof( sal_Int32);

// insertion for WW8
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
// end of the insertion for WW8

    // Marke: "rgfclcb" Beginning of array of FC/LCB pairs.
    Set_UInt32( pData, fcStshfOrig );
    Set_UInt32( pData, lcbStshfOrig );
    Set_UInt32( pData, fcStshf );
    Set_UInt32( pData, lcbStshf );
    Set_UInt32( pData, fcPlcffndRef );
    Set_UInt32( pData, lcbPlcffndRef );
    Set_UInt32( pData, fcPlcffndText );
    Set_UInt32( pData, lcbPlcffndText );
    Set_UInt32( pData, fcPlcfandRef );
    Set_UInt32( pData, lcbPlcfandRef );
    Set_UInt32( pData, fcPlcfandText );
    Set_UInt32( pData, lcbPlcfandText );
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
    Set_UInt32( pData, fcPlcffldFootnote );
    Set_UInt32( pData, lcbPlcffldFootnote );
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
    Set_UInt32( pData, lcbSttbfAssoc );
    Set_UInt32( pData, fcClx );
    Set_UInt32( pData, lcbClx );
    Set_UInt32( pData, fcPlcfpgdFootnote );
    Set_UInt32( pData, lcbPlcfpgdFootnote );
    Set_UInt32( pData, fcAutosaveSource );
    Set_UInt32( pData, lcbAutosaveSource );
    Set_UInt32( pData, fcGrpStAtnOwners );
    Set_UInt32( pData, lcbGrpStAtnOwners );
    Set_UInt32( pData, fcSttbfAtnbkmk );
    Set_UInt32( pData, lcbSttbfAtnbkmk );

    // only skip one more short at Ver67
    if( !bVer8 )
    {
        pData += 1*sizeof( sal_Int16);
        Set_UInt16( pData, (sal_uInt16)pnChpFirst );
        Set_UInt16( pData, (sal_uInt16)pnPapFirst );
        Set_UInt16( pData, (sal_uInt16)cpnBteChp );
        Set_UInt16( pData, (sal_uInt16)cpnBtePap );
    }

    Set_UInt32( pData, fcPlcfdoaMom ); // only at Ver67, in Ver8 unused
    Set_UInt32( pData, lcbPlcfdoaMom ); // only at Ver67, in Ver8 unused
    Set_UInt32( pData, fcPlcfdoaHdr ); // only at Ver67, in Ver8 unused
    Set_UInt32( pData, lcbPlcfdoaHdr ); // only at Ver67, in Ver8 unused

    Set_UInt32( pData, fcPlcfspaMom ); // in Ver67 empty reserve
    Set_UInt32( pData, lcbPlcfspaMom ); // in Ver67 empty reserve
    Set_UInt32( pData, fcPlcfspaHdr ); // in Ver67 empty reserve
    Set_UInt32( pData, lcbPlcfspaHdr ); // in Ver67 empty reserve

    Set_UInt32( pData, fcPlcfAtnbkf );
    Set_UInt32( pData, lcbPlcfAtnbkf );
    Set_UInt32( pData, fcPlcfAtnbkl );
    Set_UInt32( pData, lcbPlcfAtnbkl );
    Set_UInt32( pData, fcPms );
    Set_UInt32( pData, lcbPMS );
    Set_UInt32( pData, fcFormFieldSttbf );
    Set_UInt32( pData, lcbFormFieldSttbf );
    Set_UInt32( pData, fcPlcfendRef );
    Set_UInt32( pData, lcbPlcfendRef );
    Set_UInt32( pData, fcPlcfendText );
    Set_UInt32( pData, lcbPlcfendText );
    Set_UInt32( pData, fcPlcffldEdn );
    Set_UInt32( pData, lcbPlcffldEdn );
    Set_UInt32( pData, fcPlcfpgdEdn );
    Set_UInt32( pData, lcbPlcfpgdEdn );
    Set_UInt32( pData, fcDggInfo ); // in Ver67 empty reserve
    Set_UInt32( pData, lcbDggInfo ); // in Ver67 empty reserve
    Set_UInt32( pData, fcSttbfRMark );
    Set_UInt32( pData, lcbSttbfRMark );
    Set_UInt32( pData, fcSttbfCaption );
    Set_UInt32( pData, lcbSttbfCaption );
    Set_UInt32( pData, fcSttbAutoCaption );
    Set_UInt32( pData, lcbSttbAutoCaption );
    Set_UInt32( pData, fcPlcfwkb );
    Set_UInt32( pData, lcbPlcfwkb );
    Set_UInt32( pData, fcPlcfspl ); // in Ver67 empty reserve
    Set_UInt32( pData, lcbPlcfspl ); // in Ver67 empty reserve
    Set_UInt32( pData, fcPlcftxbxText );
    Set_UInt32( pData, lcbPlcftxbxText );
    Set_UInt32( pData, fcPlcffldTxbx );
    Set_UInt32( pData, lcbPlcffldTxbx );
    Set_UInt32( pData, fcPlcfHdrtxbxText );
    Set_UInt32( pData, lcbPlcfHdrtxbxText );
    Set_UInt32( pData, fcPlcffldHdrTxbx );
    Set_UInt32( pData, lcbPlcffldHdrTxbx );

    if( bVer8 )
    {
        pData += 0x2da - 0x27a;         // Pos + Offset (fcPlcfLst - fcStwUser)
        Set_UInt32( pData, fcSttbFnm);
        Set_UInt32( pData, lcbSttbFnm);
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
        Set_UInt32( pData, fcPlcfTch );
        Set_UInt32( pData, lcbPlcfTch );

        pData += 0x3FA - 0x38A;
        Set_UInt16( pData, (sal_uInt16)0x0002);
        Set_UInt16( pData, (sal_uInt16)0x00D9);

        pData += 0x41A - 0x3FE;
        Set_UInt32( pData, fcAtrdExtra );
        Set_UInt32( pData, lcbAtrdExtra );

        pData += 0x42a - 0x422;
        Set_UInt32(pData, fcSttbfBkmkFactoid);
        Set_UInt32(pData, lcbSttbfBkmkFactoid);
        Set_UInt32(pData, fcPlcfBkfFactoid);
        Set_UInt32(pData, lcbPlcfBkfFactoid);

        pData += 0x442 - 0x43A;
        Set_UInt32(pData, fcPlcfBklFactoid);
        Set_UInt32(pData, lcbPlcfBklFactoid);
        Set_UInt32(pData, fcFactoidData);
        Set_UInt32(pData, lcbFactoidData);

        pData += 0x4BA - 0x452;
        Set_UInt32(pData, fcPlcffactoid);
        Set_UInt32(pData, lcbPlcffactoid);

        pData += 0x4DA - 0x4c2;
        Set_UInt32( pData, fcHplxsdr );
        Set_UInt32( pData, 0);
    }

    rStrm.Write( pDataPtr, fcMin - nUnencryptedHdr );
    delete[] pDataPtr;
    return 0 == rStrm.GetError();
}

rtl_TextEncoding WW8Fib::GetFIBCharset(sal_uInt16 chs, sal_uInt16 nLidLocale)
{
    OSL_ENSURE(chs <= 0x100, "overflowed winword charset set");
    if (chs == 0x0100)
        return RTL_TEXTENCODING_APPLE_ROMAN;
    if (chs == 0 && nLidLocale >= 999)
    {
        /*
         nLidLocale:
            language stamp -- localized version In pre-WinWord 2.0 files this
            value was the nLocale. If value is < 999, then it is the nLocale,
            otherwise it is the lid.
        */
        css::lang::Locale aLocale(LanguageTag::convertToLocale(nLidLocale));
        return msfilter::util::getBestTextEncodingFromLocale(aLocale);
    }
    return rtl_getTextEncodingFromWindowsCharset(static_cast<sal_uInt8>(chs));
}

MSOFactoidType::MSOFactoidType()
    : m_nId(0)
{
}

namespace MSOPBString
{
OUString Read(SvStream& rStream)
{
    OUString aRet;

    sal_uInt16 nBuf(0);
    rStream.ReadUInt16(nBuf);
    sal_uInt16 nCch = nBuf & 0x7fff; // Bits 1..15.
    bool bAnsiString = (nBuf & (1 << 15)) >> 15; // 16th bit.
    if (bAnsiString)
        aRet = OStringToOUString(read_uInt8s_ToOString(rStream, nCch), RTL_TEXTENCODING_ASCII_US);
    else
        aRet = read_uInt16s_ToOUString(rStream, nCch);

    return aRet;
}

void Write(const OUString& rString, SvStream& rStream)
{
    sal_uInt16 nBuf = 0;
    nBuf |= rString.getLength(); // cch, 0..14th bits.
    nBuf |= 0x8000; // fAnsiString, 15th bit.
    rStream.WriteUInt16(nBuf);
    SwWW8Writer::WriteString8(rStream, rString, false, RTL_TEXTENCODING_ASCII_US);
}
};

void MSOFactoidType::Read(SvStream& rStream)
{
    sal_uInt32 cbFactoid(0);
    rStream.ReadUInt32(cbFactoid);
    rStream.ReadUInt32(m_nId);
    m_aUri = MSOPBString::Read(rStream);
    m_aTag = MSOPBString::Read(rStream);
    MSOPBString::Read(rStream); // rgbDownloadURL
}

void MSOFactoidType::Write(WW8Export& rExport)
{
    SvStream& rStream = *rExport.pTableStrm;

    SvMemoryStream aStream;
    aStream.WriteUInt32(m_nId); // id
    MSOPBString::Write(m_aUri, aStream);
    MSOPBString::Write(m_aTag, aStream);
    MSOPBString::Write("", aStream); // rgbDownloadURL
    rStream.WriteUInt32(aStream.Tell());
    aStream.Seek(0);
    rStream.WriteStream(aStream);
}

void MSOPropertyBagStore::Read(SvStream& rStream)
{
    sal_uInt32 cFactoidType(0);
    rStream.ReadUInt32(cFactoidType);
    for (sal_uInt32 i = 0; i < cFactoidType; ++i)
    {
        MSOFactoidType aFactoidType;
        aFactoidType.Read(rStream);
        m_aFactoidTypes.push_back(aFactoidType);
    }
    sal_uInt16 cbHdr(0);
    rStream.ReadUInt16(cbHdr);
    SAL_WARN_IF(cbHdr != 0xc, "sw.ww8", "MSOPropertyBagStore::Read: unexpected cbHdr");
    sal_uInt16 nVer(0);
    rStream.ReadUInt16(nVer);
    SAL_WARN_IF(nVer != 0x0100, "sw.ww8", "MSOPropertyBagStore::Read: unexpected nVer");
    rStream.SeekRel(4); // cfactoid
    sal_uInt32 nCste(0);
    rStream.ReadUInt32(nCste);

    for (sal_uInt32 i = 0; i < nCste; ++i)
    {
        OUString aString = MSOPBString::Read(rStream);
        m_aStringTable.push_back(aString);
    }
}

void MSOPropertyBagStore::Write(WW8Export& rExport)
{
    SvStream& rStream = *rExport.pTableStrm;
    rStream.WriteUInt32(m_aFactoidTypes.size()); // cFactoidType
    for (MSOFactoidType& rType : m_aFactoidTypes)
        rType.Write(rExport);
    rStream.WriteUInt16(0xc); // cbHdr
    rStream.WriteUInt16(0x0100); // sVer
    rStream.WriteUInt32(0); // cfactoid
    rStream.WriteUInt32(m_aStringTable.size()); // cste
    for (const OUString& rString : m_aStringTable)
        MSOPBString::Write(rString, rStream);
}

MSOProperty::MSOProperty()
    : m_nKey(0)
    , m_nValue(0)
{
}

void MSOProperty::Read(SvStream& rStream)
{
    rStream.ReadUInt32(m_nKey);
    rStream.ReadUInt32(m_nValue);
}

void MSOProperty::Write(SvStream& rStream)
{
    rStream.WriteUInt32(m_nKey);
    rStream.WriteUInt32(m_nValue);
}

MSOPropertyBag::MSOPropertyBag()
    : m_nId(0)
{
}

void MSOPropertyBag::Read(SvStream& rStream)
{
    rStream.ReadUInt16(m_nId);
    sal_uInt16 cProp(0);
    rStream.ReadUInt16(cProp);
    rStream.SeekRel(2); // cbUnknown
    //each MSOProperty is 8 bytes in size
    size_t nMaxPossibleRecords = rStream.remainingSize() / 8;
    if (cProp > nMaxPossibleRecords)
    {
        SAL_WARN("sw.ww8", cProp << " records claimed, but max possible is " << nMaxPossibleRecords);
        cProp = nMaxPossibleRecords;
    }
    for (sal_uInt16 i = 0; i < cProp; ++i)
    {
        MSOProperty aProperty;
        aProperty.Read(rStream);
        m_aProperties.push_back(aProperty);
    }
}

void MSOPropertyBag::Write(WW8Export& rExport)
{
    SvStream& rStream = *rExport.pTableStrm;
    rStream.WriteUInt16(m_nId);
    rStream.WriteUInt16(m_aProperties.size());
    rStream.WriteUInt16(0); // cbUnknown
    for (MSOProperty& rProperty : m_aProperties)
        rProperty.Write(rStream);
}

void WW8SmartTagData::Read(SvStream& rStream, WW8_FC fcFactoidData, sal_uInt32 lcbFactoidData)
{
    sal_uInt64 nOldPosition = rStream.Tell();
    if (!checkSeek(rStream, fcFactoidData))
        return;

    m_aPropBagStore.Read(rStream);
    while (rStream.Tell() < fcFactoidData + lcbFactoidData)
    {
        MSOPropertyBag aPropertyBag;
        aPropertyBag.Read(rStream);
        m_aPropBags.push_back(aPropertyBag);
    }

    rStream.Seek(nOldPosition);
}

void WW8SmartTagData::Write(WW8Export& rExport)
{
    m_aPropBagStore.Write(rExport);
    for (MSOPropertyBag& rPropertyBag : m_aPropBags)
        rPropertyBag.Write(rExport);
}

WW8Style::WW8Style(SvStream& rStream, WW8Fib& rFibPara)
    : rFib(rFibPara), rSt(rStream), cstd(0), cbSTDBaseInFile(0), fStdStylenamesWritten(0)
    , stiMaxWhenSaved(0), istdMaxFixedWhenSaved(0), nVerBuiltInNamesWhenSaved(0)
    , ftcAsci(0), ftcFE(0), ftcOther(0), ftcBi(0)
{
    if (!checkSeek(rSt, rFib.fcStshf))
        return;

    sal_uInt16 cbStshi = 0; //  2 bytes size of the following STSHI structure
    sal_uInt32 nRemaining = rFib.lcbStshf;
    const sal_uInt32 nMinValidStshi = 4;

    if (rFib.GetFIBVersion() <= ww::eWW2)
    {
        cbStshi = 0;
        cstd = 256;
    }
    else
    {
        if (rFib.nFib < 67) // old Version ? (need to find this again to fix)
            cbStshi = nMinValidStshi;
        else    // new version
        {
            if (nRemaining < sizeof(cbStshi))
                return;
            // reads the length of the structure in the file
            rSt.ReadUInt16( cbStshi );
            nRemaining-=2;
        }
    }

    cbStshi = std::min(static_cast<sal_uInt32>(cbStshi), nRemaining);
    if (cbStshi < nMinValidStshi)
        return;

    const sal_uInt16 nRead = cbStshi;
    do
    {
        rSt.ReadUInt16( cstd );

        rSt.ReadUInt16( cbSTDBaseInFile );

        if(  6 > nRead ) break;

        sal_uInt16 a16Bit;
        rSt.ReadUInt16( a16Bit );
        fStdStylenamesWritten = a16Bit & 0x0001;

        if(  8 > nRead ) break;
        rSt.ReadUInt16( stiMaxWhenSaved );

        if( 10 > nRead ) break;
        rSt.ReadUInt16( istdMaxFixedWhenSaved );

        if( 12 > nRead ) break;
        rSt.ReadUInt16( nVerBuiltInNamesWhenSaved );

        if( 14 > nRead ) break;
        rSt.ReadUInt16( ftcAsci );

        if( 16 > nRead ) break;
        rSt.ReadUInt16( ftcFE );

        if ( 18 > nRead ) break;
        rSt.ReadUInt16( ftcOther );

        ftcBi = ftcOther;

        if ( 20 > nRead ) break;
        rSt.ReadUInt16( ftcBi );

        // p.r.n. ignore the rest
        if( 20 < nRead )
            rSt.SeekRel( nRead-20 );
    }
    while( false ); // trick: the block above will be passed through exactly one time
                //            and that's why we can early exit with "break".

    nRemaining -= cbStshi;

    //There will be stshi.cstd (cbSTD, STD) pairs in the file following the
    //STSHI. Note that styles can be empty, i.e. cbSTD == 0
    const sal_uInt32 nMinRecordSize = sizeof(sal_uInt16);
    const sal_uInt16 nMaxPossibleRecords = nRemaining/nMinRecordSize;

    OSL_ENSURE(cstd <= nMaxPossibleRecords,
        "allegedly more styles that available data\n");
    cstd = std::min(cstd, nMaxPossibleRecords);
}

// Read1STDFixed() reads a style. If the style is completely existent,
// so it has no empty slot, we should allocate memory and a pointer should
// reference to STD (perhaps filled with 0). If the slot is empty,
// it will return a null pointer.
WW8_STD* WW8Style::Read1STDFixed( short& rSkip, short* pcbStd )
{
    WW8_STD* pStd = nullptr;

    sal_uInt16 cbStd(0);
    rSt.ReadUInt16( cbStd );   // read length

    const sal_uInt16 nRead = cbSTDBaseInFile;
    if( cbStd >= cbSTDBaseInFile )
    {
        // Fixed part completely available

        // read fixed part of STD
        pStd = new WW8_STD;
        memset( pStd, 0, sizeof( *pStd ) );

        do
        {
            if( 2 > nRead ) break;

            sal_uInt16 a16Bit = 0;
            rSt.ReadUInt16( a16Bit );
            pStd->sti          =        a16Bit & 0x0fff  ;
            pStd->fScratch     = sal_uInt16(0 != ( a16Bit & 0x1000 ));
            pStd->fInvalHeight = sal_uInt16(0 != ( a16Bit & 0x2000 ));
            pStd->fHasUpe      = sal_uInt16(0 != ( a16Bit & 0x4000 ));
            pStd->fMassCopy    = sal_uInt16(0 != ( a16Bit & 0x8000 ));

            if( 4 > nRead ) break;
            a16Bit = 0;
            rSt.ReadUInt16( a16Bit );
            pStd->sgc      =   a16Bit & 0x000f       ;
            pStd->istdBase = ( a16Bit & 0xfff0 ) >> 4;

            if( 6 > nRead ) break;
            a16Bit = 0;
            rSt.ReadUInt16( a16Bit );
            pStd->cupx     =   a16Bit & 0x000f       ;
            pStd->istdNext = ( a16Bit & 0xfff0 ) >> 4;

            if( 8 > nRead ) break;
            a16Bit = 0;
            rSt.ReadUInt16( pStd->bchUpe );

            // from Ver8 this two fields should be added:
            if(10 > nRead ) break;
            a16Bit = 0;
            rSt.ReadUInt16( a16Bit );
            pStd->fAutoRedef =   a16Bit & 0x0001       ;
            pStd->fHidden    = ( a16Bit & 0x0002 ) >> 1;
            // You never know: cautionary skipped
            if( 10 < nRead )
                rSt.SeekRel( nRead-10 );
        }
        while( false ); // trick: the block above will passed through exactly one time
                    //   and can be left early with a "break"

        if( (0 != rSt.GetError()) || !nRead )
            DELETEZ( pStd );        // report error with NULL

      rSkip = cbStd - cbSTDBaseInFile;
    }
    else
    {           // Fixed part too short
        if( cbStd )
            rSt.SeekRel( cbStd );           // skip leftovers
        rSkip = 0;
    }
    if( pcbStd )
        *pcbStd = cbStd;
    return pStd;
}

WW8_STD* WW8Style::Read1Style( short& rSkip, OUString* pString, short* pcbStd )
{
    // Attention: MacWord-Documents have their Stylenames
    // always in ANSI, even if eStructCharSet == CHARSET_MAC !!

    WW8_STD* pStd = Read1STDFixed( rSkip, pcbStd );         // read STD

    // string desired?
    if( pString )
    {   // real style?
        if ( pStd )
        {
            switch( rFib.nVersion )
            {
                case 6:
                case 7:
                    // read pascal string
                    *pString = read_uInt8_BeltAndBracesString(rSt, RTL_TEXTENCODING_MS_1252);
                    // leading len and trailing zero --> 2
                    rSkip -= pString->getLength() + 2;
                    break;
                case 8:
                    // handle Unicode-String with leading length short and
                    // trailing zero
                    if (TestBeltAndBraces(rSt))
                    {
                        *pString = read_uInt16_BeltAndBracesString(rSt);
                        rSkip -= (pString->getLength() + 2) * 2;
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
                        *pString = read_uInt8_BeltAndBracesString(rSt,RTL_TEXTENCODING_MS_1252);
                        // leading len and trailing zero --> 2
                        rSkip -= pString->getLength() + 2;
                    }
                    break;
                default:
                    OSL_ENSURE(false, "Es wurde vergessen, nVersion zu kodieren!");
                    break;
            }
        }
        else
            pString->clear();   // can not return a name
    }
    return pStd;
}

namespace {
const sal_uInt16 maxStrSize = 65;
}

struct WW8_FFN_Ver6
{
    WW8_FFN_BASE base;
    // from Ver6
    sal_Char szFfn[maxStrSize]; // 0x6 bzw. 0x40 ab Ver8 zero terminated string that
                        // records name of font.
                        // Maximal size of szFfn is 65 characters.
                        // Vorsicht: Dieses Array kann auch kleiner sein!!!
                        // Possibly followed by a second sz which records the
                        // name of an alternate font to use if the first named
                        // font does not exist on this system.
};
struct WW8_FFN_Ver8 : public WW8_FFN_BASE
{
    // from Ver8 two more fields are present,
    // we ignore this.
    sal_Char panose[ 10 ];  //  0x6   PANOSE
    sal_Char fs[ 24     ];  //  0x10  FONTSIGNATURE

    // from Ver8 as unicode
    sal_uInt16 szFfn[65];   // 0x6 bzw. 0x40 ab Ver8 zero terminated string that
                        // records name of font.
                        // Maximal size of szFfn is 65 characters.
                        // Attention: This array can be smaller!!!
                        // Possibly followed by a second sz which records the
                        // name of an alternate font to use if the first named
                        // font does not exist on this system.
};

// #i43762# check font name for illegal characters
static void lcl_checkFontname( OUString& sString )
{
    // for efficiency, we'd like to use String methods as far as possible.
    // Hence, we will:
    // 1) convert all invalid chars to \u0001
    // 2) then erase all \u0001 chars (if any were found), and
    // 3) erase leading/trailing ';', in case a font name was
    //    completely removed

    // convert all invalid chars to \u0001
    OUStringBuffer aBuf(sString);
    const sal_Int32 nLen = aBuf.getLength();
    bool bFound = false;
    for ( sal_Int32 n = 0; n < nLen; ++n )
    {
        if ( aBuf[n] < 0x20 )
        {
            aBuf[n] = 1;
            bFound = true;
        }
    }
    sString = aBuf.makeStringAndClear();

    // if anything was found, remove \u0001 + leading/trailing ';'
    if( bFound )
    {
        sString = comphelper::string::strip(sString.replaceAll("\001", ""), ';');
    }
}

namespace
{
    sal_uInt16 calcMaxFonts(sal_uInt8 *p, sal_Int32 nFFn)
    {
        // Figure out the max number of fonts defined here
        sal_uInt16 nMax = 0;
        sal_Int32 nRemaining = nFFn;
        while (nRemaining)
        {
            //p[0] is cbFfnM1, the alleged total length of FFN - 1.
            //i.e. length after cbFfnM1
            const sal_uInt16 cbFfnM1 = *p++;
            --nRemaining;

            if (cbFfnM1 > nRemaining)
                break;

            nMax++;
            nRemaining -= cbFfnM1;
            p += cbFfnM1;
        }
        return nMax;
    }

    template<typename T> bool readU8(
        sal_uInt8 const * p, std::size_t offset, sal_uInt8 const * pEnd,
        T * value)
    {
        assert(p <= pEnd);
        assert(value != nullptr);
        if (offset >= static_cast<std::size_t>(pEnd - p)) {
            return false;
        }
        *value = p[offset];
        return true;
    }

    bool readS16(
        sal_uInt8 const * p, std::size_t offset, sal_uInt8 const * pEnd,
        short * value)
    {
        assert(p <= pEnd);
        assert(value != nullptr);
        if (offset > static_cast<std::size_t>(pEnd - p)
            || static_cast<std::size_t>(pEnd - p) - offset < 2)
        {
            return false;
        }
        *value = unsigned(p[offset]) + (unsigned(p[offset + 1]) << 8);
        return true;
    }

    sal_Int32 getStringLength(
        sal_uInt8 const * p, std::size_t offset, sal_uInt8 const * pEnd)
    {
        assert(p <= pEnd);
        assert(pEnd - p <= SAL_MAX_INT32);
        if (offset >= static_cast<std::size_t>(pEnd - p)) {
            return -1;
        }
        void const * p2 = std::memchr(
            p + offset, 0, static_cast<std::size_t>(pEnd - p) - offset);
        if (p2 == nullptr) {
            return -1;
        }
        return static_cast<sal_uInt8 const *>(p2) - (p + offset);
    }
}

WW8Fonts::WW8Fonts( SvStream& rSt, WW8Fib& rFib )
    : pFontA(nullptr), nMax(0)
{
    // Attention: MacWord-Documents have their Fontnames
    // always in ANSI, even if eStructCharSet == CHARSET_MAC !!
    if( rFib.lcbSttbfffn <= 2 )
    {
        OSL_ENSURE( false, "font table is broken! (rFib.lcbSttbfffn < 2)" );
        return;
    }

    if (!checkSeek(rSt, rFib.fcSttbfffn))
        return;

    sal_Int32 nFFn = rFib.lcbSttbfffn - 2;

    // allocate Font Array
    sal_uInt8* pA = new sal_uInt8[nFFn];
    memset(pA, 0, nFFn);

    ww::WordVersion eVersion = rFib.GetFIBVersion();

    if( eVersion >= ww::eWW8 )
    {
        // bVer8: read the count of strings in nMax
        rSt.ReadUInt16( nMax );
    }

    // Ver8:  skip undefined uint16
    // Ver67: skip the herein stored total byte of structure
    //        - we already got that information in rFib.lcbSttbfffn
    rSt.SeekRel( 2 );

    // read all font information
    nFFn = rSt.Read(pA, nFFn);
    sal_uInt8 * const pEnd = pA + nFFn;
    const sal_uInt16 nCalcMax = calcMaxFonts(pA, nFFn);

    if (eVersion < ww::eWW8)
        nMax = nCalcMax;
    else
    {
        //newer versions include supportive count of fonts, so take min of that
        //and calced max
        nMax = std::min(nMax, nCalcMax);
    }

    if( nMax )
    {
        // allocate Index Array
        pFontA = new WW8_FFN[ nMax ];
        WW8_FFN* p = pFontA;

        if( eVersion <= ww::eWW2 )
        {
            sal_uInt8 const * pVer2 = pA;
            sal_uInt16 i = 0;
            for(; i<nMax; ++i, ++p)
            {
                if (!readU8(
                        pVer2, offsetof(WW8_FFN_BASE, cbFfnM1), pEnd,
                        &p->cbFfnM1))
                {
                    break;
                }

                p->prg       =  0;
                p->fTrueType = 0;
                p->ff        = 0;

                if (!(readU8(pVer2, 1, pEnd, &p->wWeight)
                      && readU8(pVer2, 2, pEnd, &p->chs)))
                {
                    break;
                }
                /*
                 #i8726# 7- seems to encode the name in the same encoding as
                 the font, e.g load the doc in 97 and save to see the unicode
                 ver of the asian fontnames in that example to confirm.
                */
                rtl_TextEncoding eEnc = WW8Fib::GetFIBCharset(p->chs, rFib.lid);
                if ((eEnc == RTL_TEXTENCODING_SYMBOL) || (eEnc == RTL_TEXTENCODING_DONTKNOW))
                    eEnc = RTL_TEXTENCODING_MS_1252;

                sal_Int32 n = getStringLength(pVer2, 1 + 2, pEnd);
                if (n == -1) {
                    break;
                }
                p->sFontname = OUString(
                    reinterpret_cast<char const *>(pVer2 + 1 + 2), n, eEnc);
                pVer2 = pVer2 + p->cbFfnM1 + 1;
            }
            nMax = i;
        }
        else if( eVersion < ww::eWW8 )
        {
            sal_uInt8 const * pVer6 = pA;
            sal_uInt16 i = 0;
            for(; i<nMax; ++i, ++p)
            {
                if (!readU8(
                        pVer6, offsetof(WW8_FFN_BASE, cbFfnM1), pEnd,
                        &p->cbFfnM1))
                {
                    break;
                }
                sal_uInt8 c2;
                if (!readU8(pVer6, 1, pEnd, &c2)) {
                    break;
                }

                p->prg       =  c2 & 0x02;
                p->fTrueType = (c2 & 0x04) >> 2;
                // skip a reserve bit
                p->ff        = (c2 & 0x70) >> 4;

                if (!(readS16(
                          pVer6, offsetof(WW8_FFN_BASE, wWeight), pEnd,
                          &p->wWeight)
                      && readU8(
                          pVer6, offsetof(WW8_FFN_BASE, chs), pEnd, &p->chs)
                      && readU8(
                          pVer6, offsetof(WW8_FFN_BASE, ibszAlt), pEnd,
                          &p->ibszAlt)))
                {
                    break;
                }
                /*
                 #i8726# 7- seems to encode the name in the same encoding as
                 the font, e.g load the doc in 97 and save to see the unicode
                 ver of the asian fontnames in that example to confirm.
                 */
                rtl_TextEncoding eEnc = WW8Fib::GetFIBCharset(p->chs, rFib.lid);
                if ((eEnc == RTL_TEXTENCODING_SYMBOL) || (eEnc == RTL_TEXTENCODING_DONTKNOW))
                    eEnc = RTL_TEXTENCODING_MS_1252;
                sal_Int32 n = getStringLength(
                    pVer6, offsetof(WW8_FFN_Ver6, szFfn), pEnd);
                if (n == -1) {
                    break;
                }
                p->sFontname = OUString(
                    reinterpret_cast<char const *>(
                        pVer6 + offsetof(WW8_FFN_Ver6, szFfn)),
                    n, eEnc);
                if (p->ibszAlt && p->ibszAlt < maxStrSize) //don't start after end of string
                {
                    n = getStringLength(
                        pVer6, offsetof(WW8_FFN_Ver6, szFfn) + p->ibszAlt,
                        pEnd);
                    if (n == -1) {
                        break;
                    }
                    p->sFontname += ";" + OUString(
                        reinterpret_cast<char const *>(
                            pVer6 + offsetof(WW8_FFN_Ver6, szFfn) + p->ibszAlt),
                        n, eEnc);
                }
                else
                {
                    //#i18369# if it's a symbol font set Symbol as fallback
                    if (
                         RTL_TEXTENCODING_SYMBOL == WW8Fib::GetFIBCharset(p->chs, rFib.lid)
                         && p->sFontname!="Symbol"
                       )
                    {
                        p->sFontname += ";Symbol";
                    }
                }
                pVer6 = pVer6 + p->cbFfnM1 + 1;
            }
            nMax = i;
        }
        else
        {
            //count of bytes in minimum FontFamilyInformation payload
            const sal_uInt8 cbMinFFNPayload = 41;
            sal_uInt16 nValidFonts = 0;
            sal_Int32 nRemainingFFn = nFFn;
            sal_uInt8* pRaw = pA;
            for (sal_uInt16 i=0; i < nMax && nRemainingFFn; ++i, ++p)
            {
                //pRaw[0] is cbFfnM1, the alleged total length of FFN - 1
                //i.e. length after cbFfnM1
                sal_uInt8 cbFfnM1 = *pRaw++;
                --nRemainingFFn;

                if (cbFfnM1 > nRemainingFFn)
                    break;

                if (cbFfnM1 < cbMinFFNPayload)
                    break;

                p->cbFfnM1 = cbFfnM1;

                sal_uInt8 *pVer8 = pRaw;

                sal_uInt8 c2 = *pVer8++;
                --cbFfnM1;

                p->prg = c2 & 0x02;
                p->fTrueType = (c2 & 0x04) >> 2;
                // skip a reserve bit
                p->ff = (c2 & 0x70) >> 4;

                p->wWeight = SVBT16ToShort(*reinterpret_cast<SVBT16*>(pVer8));
                pVer8+=2;
                cbFfnM1-=2;

                p->chs = *pVer8++;
                --cbFfnM1;

                p->ibszAlt = *pVer8++;
                --cbFfnM1;

                pVer8 += 10; //PANOSE
                cbFfnM1-=10;
                pVer8 += 24; //FONTSIGNATURE
                cbFfnM1-=24;

                OSL_ASSERT(cbFfnM1 >= 2);

                sal_uInt8 nMaxNullTerminatedPossible = cbFfnM1/2 - 1;
                sal_Unicode *pPrimary = reinterpret_cast<sal_Unicode*>(pVer8);
                pPrimary[nMaxNullTerminatedPossible] = 0;
#ifdef OSL_BIGENDIAN
                swapEndian(pPrimary);
#endif
                p->sFontname = pPrimary;
                if (p->ibszAlt && p->ibszAlt < nMaxNullTerminatedPossible)
                {
                    sal_Unicode *pSecondary = pPrimary + p->ibszAlt;
#ifdef OSL_BIGENDIAN
                    swapEndian(pSecondary);
#endif
                    p->sFontname += ";" + OUString(pSecondary);
                }

                // #i43762# check font name for illegal characters
                lcl_checkFontname( p->sFontname );

                // set pointer one font back to original array
                pRaw += p->cbFfnM1;
                nRemainingFFn -= p->cbFfnM1;
                ++nValidFonts;
            }
            OSL_ENSURE(nMax == nValidFonts, "Font count differs with availability");
            nMax = std::min(nMax, nValidFonts);
        }
    }
    delete[] pA;
}

const WW8_FFN* WW8Fonts::GetFont( sal_uInt16 nNum ) const
{
    if( !pFontA || nNum >= nMax )
        return nullptr;

    return &pFontA[ nNum ];
}

// Search after a header/footer for a index in the ww list from header/footer

// specials for WinWord6 and -7:
//
// 1) At the start of reading we must build WWPLCF_HdFt with Fib and Dop
// 2) The main text must be read sequentially over all sections
// 3) For every header/footer in the main text, we must call UpdateIndex()
//    exactly once with the parameter from the attribute.
//    (per section can be maximally one). This call must take place *after*
//    the last call from GetTextPos().
// 4) GetTextPos() can be called with exactly one flag
//    out of WW8_{FOOTER,HEADER}_{ODD,EVEN,FIRST} (Do not change!)
//  -> maybe we can get a right result then

WW8PLCF_HdFt::WW8PLCF_HdFt( SvStream* pSt, WW8Fib& rFib, WW8Dop& rDop )
    : aPLCF(*pSt, rFib.fcPlcfhdd , rFib.lcbPlcfhdd , 0)
{
    nIdxOffset = 0;

     /*
      This dop.grpfIhdt has a bit set for each special
      footnote *and endnote!!* separator,continuation separator, and
      continuation notice entry, the documentation does not mention the
      endnote separators, the documentation also gets the index numbers
      backwards when specifying which bits to test. The bottom six bits
      of this value must be tested and skipped over. Each section's
      grpfIhdt is then tested for the existence of the appropriate headers
      and footers, at the end of each section the nIdxOffset must be updated
      to point to the beginning of the next section's group of headers and
      footers in this PLCF, UpdateIndex does that task.
      */
    for( sal_uInt8 nI = 0x1; nI <= 0x20; nI <<= 1 )
        if( nI & rDop.grpfIhdt )                // bit set?
            nIdxOffset++;

    nTextOfs = rFib.ccpText + rFib.ccpFootnote;  // size of the main text
                                            // and from the footnotes
}

bool WW8PLCF_HdFt::GetTextPos(sal_uInt8 grpfIhdt, sal_uInt8 nWhich, WW8_CP& rStart,
    WW8_CP& rLen)
{
    sal_uInt8 nI = 0x01;
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
                                        // nIdx is HdFt-Index
    WW8_CP nEnd;
    void* pData;

    aPLCF.SetIdx( nIdx );               // Lookup suitable CP
    aPLCF.Get( rStart, nEnd, pData );
    if (nEnd < rStart)
    {
        SAL_WARN("sw.ww8", "End " << nEnd << " before Start " << rStart);
        return false;
    }
    rLen = nEnd - rStart;
    aPLCF.advance();

    return true;
}

bool WW8PLCF_HdFt::GetTextPosExact(short nIdx, WW8_CP& rStart, WW8_CP& rLen)
{
    WW8_CP nEnd;
    void* pData;

    aPLCF.SetIdx( nIdx );               // Lookup suitable CP
    aPLCF.Get( rStart, nEnd, pData );
    if (nEnd < rStart)
    {
        SAL_WARN("sw.ww8", "End " << nEnd << " before Start " << rStart);
        return false;
    }
    rLen = nEnd - rStart;

    return true;
}

void WW8PLCF_HdFt::UpdateIndex( sal_uInt8 grpfIhdt )
{
    // Caution: Description is not correct
    for( sal_uInt8 nI = 0x01; nI <= 0x20; nI <<= 1 )
        if( nI & grpfIhdt )
            nIdxOffset++;
}

WW8Dop::WW8Dop(SvStream& rSt, sal_Int16 nFib, sal_Int32 nPos, sal_uInt32 nSize)
{
    memset(this, 0, sizeof(WW8Dop));
    fDontUseHTMLAutoSpacing = true; //default
    fAcetateShowAtn = true; //default
    const sal_uInt32 nMaxDopSize = 0x268;
    sal_uInt8* pDataPtr = new sal_uInt8[ nMaxDopSize ];
    sal_uInt8* pData = pDataPtr;

    sal_uInt32 nRead = nMaxDopSize < nSize ? nMaxDopSize : nSize;
    if (nSize < 2 || !checkSeek(rSt, nPos) || nRead != rSt.Read(pData, nRead))
        nDopError = ERR_SWG_READ_ERROR;     // report error
    else
    {
        if (nMaxDopSize > nRead)
            memset( pData + nRead, 0, nMaxDopSize - nRead );

        // interpret the data
        sal_uInt32 a32Bit;
        sal_uInt16 a16Bit;
        sal_uInt8   a8Bit;

        a16Bit = Get_UShort( pData );        // 0 0x00
        fFacingPages        = 0 != ( a16Bit  &  0x0001 )     ;
        fWidowControl       = 0 != ( a16Bit  &  0x0002 )     ;
        fPMHMainDoc         = 0 != ( a16Bit  &  0x0004 )     ;
        grfSuppression      =      ( a16Bit  &  0x0018 ) >> 3;
        fpc                 =      ( a16Bit  &  0x0060 ) >> 5;
        grpfIhdt            =      ( a16Bit  &  0xff00 ) >> 8;

        a16Bit = Get_UShort( pData );        // 2 0x02
        rncFootnote              =   a16Bit  &  0x0003        ;
        nFootnote                = ( a16Bit  & ~0x0003 ) >> 2 ;

        a8Bit = Get_Byte( pData );           // 4 0x04
        fOutlineDirtySave      = 0 != ( a8Bit  &  0x01   );

        a8Bit = Get_Byte( pData );           // 5 0x05
        fOnlyMacPics           = 0 != ( a8Bit  &  0x01   );
        fOnlyWinPics           = 0 != ( a8Bit  &  0x02   );
        fLabelDoc              = 0 != ( a8Bit  &  0x04   );
        fHyphCapitals          = 0 != ( a8Bit  &  0x08   );
        fAutoHyphen            = 0 != ( a8Bit  &  0x10   );
        fFormNoFields          = 0 != ( a8Bit  &  0x20   );
        fLinkStyles            = 0 != ( a8Bit  &  0x40   );
        fRevMarking            = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );           // 6 0x06
        fBackup                = 0 != ( a8Bit  &  0x01   );
        fExactCWords           = 0 != ( a8Bit  &  0x02   );
        fPagHidden             = 0 != ( a8Bit  &  0x04   );
        fPagResults            = 0 != ( a8Bit  &  0x08   );
        fLockAtn               = 0 != ( a8Bit  &  0x10   );
        fMirrorMargins         = 0 != ( a8Bit  &  0x20   );
        fReadOnlyRecommended   = 0 != ( a8Bit  &  0x40   );
        fDfltTrueType          = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );           // 7 0x07
        fPagSuppressTopSpacing = 0 != ( a8Bit  &  0x01   );
        fProtEnabled           = 0 != ( a8Bit  &  0x02   );
        fDispFormFieldSel        = 0 != ( a8Bit  &  0x04   );
        fRMView                = 0 != ( a8Bit  &  0x08   );
        fRMPrint               = 0 != ( a8Bit  &  0x10   );
        fWriteReservation      = 0 != ( a8Bit  &  0x20   );
        fLockRev               = 0 != ( a8Bit  &  0x40   );
        fEmbedFonts            = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );           // 8 0x08
        copts_fNoTabForInd           = 0 != ( a8Bit  &  0x01   );
        copts_fNoSpaceRaiseLower     = 0 != ( a8Bit  &  0x02   );
        copts_fSupressSpbfAfterPgBrk = 0 != ( a8Bit  &  0x04   );
        copts_fWrapTrailSpaces       = 0 != ( a8Bit  &  0x08   );
        copts_fMapPrintTextColor     = 0 != ( a8Bit  &  0x10   );
        copts_fNoColumnBalance       = 0 != ( a8Bit  &  0x20   );
        copts_fConvMailMergeEsc      = 0 != ( a8Bit  &  0x40   );
        copts_fSupressTopSpacing     = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );           // 9 0x09
        copts_fOrigWordTableRules    = 0 != ( a8Bit  &  0x01   );
        copts_fTransparentMetafiles  = 0 != ( a8Bit  &  0x02   );
        copts_fShowBreaksInFrames    = 0 != ( a8Bit  &  0x04   );
        copts_fSwapBordersFacingPgs  = 0 != ( a8Bit  &  0x08   );
        copts_fExpShRtn              = 0 != ( a8Bit  &  0x20   );  // #i56856#

        dxaTab = Get_Short( pData );         // 10 0x0a
        wSpare = Get_UShort( pData );        // 12 0x0c
        dxaHotZ = Get_UShort( pData );       // 14 0x0e
        cConsecHypLim = Get_UShort( pData ); // 16 0x10
        wSpare2 = Get_UShort( pData );       // 18 0x12
        dttmCreated = Get_Long( pData );     // 20 0x14
        dttmRevised = Get_Long( pData );     // 24 0x18
        dttmLastPrint = Get_Long( pData );   // 28 0x1c
        nRevision = Get_Short( pData );      // 32 0x20
        tmEdited = Get_Long( pData );        // 34 0x22
        cWords = Get_Long( pData );          // 38 0x26
        cCh = Get_Long( pData );             // 42 0x2a
        cPg = Get_Short( pData );            // 46 0x2e
        cParas = Get_Long( pData );          // 48 0x30

        a16Bit = Get_UShort( pData );        // 52 0x34
        rncEdn =   a16Bit &  0x0003       ;
        nEdn   = ( a16Bit & ~0x0003 ) >> 2;

        a16Bit = Get_UShort( pData );        // 54 0x36
        epc            =   a16Bit &  0x0003       ;
        nfcFootnoteRef      = ( a16Bit &  0x003c ) >> 2;
        nfcEdnRef      = ( a16Bit &  0x03c0 ) >> 6;
        fPrintFormData = 0 != ( a16Bit &  0x0400 );
        fSaveFormData  = 0 != ( a16Bit &  0x0800 );
        fShadeFormData = 0 != ( a16Bit &  0x1000 );
        fWCFootnoteEdn      = 0 != ( a16Bit &  0x8000 );

        cLines = Get_Long( pData );          // 56 0x38
        cWordsFootnoteEnd = Get_Long( pData );    // 60 0x3c
        cChFootnoteEdn = Get_Long( pData );       // 64 0x40
        cPgFootnoteEdn = Get_Short( pData );      // 68 0x44
        cParasFootnoteEdn = Get_Long( pData );    // 70 0x46
        cLinesFootnoteEdn = Get_Long( pData );    // 74 0x4a
        lKeyProtDoc = Get_Long( pData );     // 78 0x4e

        a16Bit = Get_UShort( pData );        // 82 0x52
        wvkSaved    =   a16Bit & 0x0007        ;
        wScaleSaved = ( a16Bit & 0x0ff8 ) >> 3 ;
        zkSaved     = ( a16Bit & 0x3000 ) >> 12;
        fRotateFontW6 = ( a16Bit & 0x4000 ) >> 14;
        iGutterPos = ( a16Bit &  0x8000 ) >> 15;

        if (nFib >= 103) // Word 6/32bit, 95, 97, 2000, 2002, 2003, 2007
        {
            a32Bit = Get_ULong( pData );     // 84 0x54
            SetCompatibilityOptions(a32Bit);
        }

        //#i22436#, for all WW7- documents
        if (nFib <= 104) // Word 95
            fUsePrinterMetrics = true;

        if (nFib > 105) // Word 97, 2000, 2002, 2003, 2007
        {
            adt = Get_Short( pData );            // 88 0x58

            doptypography.ReadFromMem(pData);    // 90 0x5a

            memcpy( &dogrid, pData, sizeof( WW8_DOGRID )); // 400 0x190
            pData += sizeof( WW8_DOGRID );

            a16Bit = Get_UShort( pData );        // 410 0x19a
            // the following 9 bit are uninteresting
            fHtmlDoc                = ( a16Bit &  0x0200 ) >>  9 ;
            fSnapBorder             = ( a16Bit &  0x0800 ) >> 11 ;
            fIncludeHeader          = ( a16Bit &  0x1000 ) >> 12 ;
            fIncludeFooter          = ( a16Bit &  0x2000 ) >> 13 ;
            fForcePageSizePag       = ( a16Bit &  0x4000 ) >> 14 ;
            fMinFontSizePag         = ( a16Bit &  0x8000 ) >> 15 ;

            a16Bit = Get_UShort( pData );        // 412 0x19c
            fHaveVersions   = 0 != ( a16Bit  &  0x0001 );
            fAutoVersion    = 0 != ( a16Bit  &  0x0002 );

            pData += 12;                         // 414 0x19e

            cChWS = Get_Long( pData );           // 426 0x1aa
            cChWSFootnoteEdn = Get_Long( pData );     // 430 0x1ae
            grfDocEvents = Get_Long( pData );    // 434 0x1b2

            pData += 4+30+8;  // 438 0x1b6; 442 0x1ba; 472 0x1d8; 476 0x1dc

            cDBC = Get_Long( pData );            // 480 0x1e0
            cDBCFootnoteEdn = Get_Long( pData );      // 484 0x1e4

            pData += 1 * sizeof( sal_Int32);         // 488 0x1e8

            nfcFootnoteRef = Get_Short( pData );      // 492 0x1ec
            nfcEdnRef = Get_Short( pData );      // 494 0x1ee
            hpsZoonFontPag = Get_Short( pData ); // 496 0x1f0
            dywDispPag = Get_Short( pData );     // 498 0x1f2

            if (nRead >= 516)
            {
                //500 -> 508, Appear to be repeated here in 2000+
                pData += 8;                      // 500 0x1f4
                a32Bit = Get_Long( pData );      // 508 0x1fc
                SetCompatibilityOptions(a32Bit);
                a32Bit = Get_Long( pData );      // 512 0x200

                // i#78591#
                SetCompatibilityOptions2(a32Bit);
            }
            if (nRead >= 550)
            {
                pData += 32;
                a16Bit = Get_UShort( pData );
                fDoNotEmbedSystemFont = ( a16Bit &  0x0001 );
                fWordCompat = ( a16Bit &  0x0002 ) >> 1;
                fLiveRecover = ( a16Bit &  0x0004 ) >> 2;
                fEmbedFactoids = ( a16Bit &  0x0008 ) >> 3;
                fFactoidXML = ( a16Bit &  0x00010 ) >> 4;
                fFactoidAllDone = ( a16Bit &  0x0020 ) >> 5;
                fFolioPrint = ( a16Bit &  0x0040 ) >> 6;
                fReverseFolio = ( a16Bit &  0x0080 ) >> 7;
                iTextLineEnding = ( a16Bit &  0x0700 ) >> 8;
                fHideFcc = ( a16Bit &  0x0800 ) >> 11;
                fAcetateShowMarkup = ( a16Bit &  0x1000 ) >> 12;
                fAcetateShowAtn = ( a16Bit &  0x2000 ) >> 13;
                fAcetateShowInsDel = ( a16Bit &  0x4000 ) >> 14;
                fAcetateShowProps = ( a16Bit &  0x8000 ) >> 15;
            }
            if (nRead >= 600)
            {
                pData += 48;
                a16Bit = Get_Short(pData);
                fUseBackGroundInAllmodes = (a16Bit & 0x0080) >> 7;
            }
        }
    }
    delete[] pDataPtr;
}

WW8Dop::WW8Dop()
{
    // first set everything to a default of 0
    memset(this, 0, sizeof(WW8Dop));

    fWidowControl = true;
    fpc = 1;
    nFootnote = 1;
    fOutlineDirtySave = true;
    fHyphCapitals = true;
    fBackup = true;
    fPagHidden = true;
    fPagResults = true;
    fDfltTrueType = true;

    /*
    Writer acts like this all the time at the moment, ideally we need an
    option for these two as well to import word docs that are not like
    this by default
    */
    fNoLeading = true;
    fUsePrinterMetrics = true;

    fRMView = true;
    fRMPrint = true;
    dxaTab = 0x2d0;
    dxaHotZ = 0x168;
    nRevision = 1;
    nEdn = 1;

    epc = 3;
    nfcEdnRef = 2;
    fShadeFormData = true;

    wvkSaved = 2;
    wScaleSaved = 100;
    zkSaved = 0;

    lvl = 9;
    fIncludeHeader = true;
    fIncludeFooter = true;

    cChWS = /**!!**/ 0;
    cChWSFootnoteEdn = /**!!**/ 0;

    cDBC = /**!!**/ 0;
    cDBCFootnoteEdn = /**!!**/ 0;

    fAcetateShowAtn = true;
}

void WW8Dop::SetCompatibilityOptions(sal_uInt32 a32Bit)
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
    fCompatibilityOptions_Unknown1_13       = ( a32Bit &  0x00001000 ) >> 12 ;
    fExpShRtn                   = ( a32Bit &  0x00002000 ) >> 13 ; // #i56856#
    fCompatibilityOptions_Unknown1_15       = ( a32Bit &  0x00004000 ) >> 14 ;
    fCompatibilityOptions_Unknown1_16       = ( a32Bit &  0x00008000 ) >> 15 ;
    fSuppressTopSpacingMac5     = ( a32Bit &  0x00010000 ) >> 16 ;
    fTruncDxaExpand             = ( a32Bit &  0x00020000 ) >> 17 ;
    fPrintBodyBeforeHdr         = ( a32Bit &  0x00040000 ) >> 18 ;
    fNoLeading                  = ( a32Bit &  0x00080000 ) >> 19 ;
    fCompatibilityOptions_Unknown1_21       = ( a32Bit &  0x00100000 ) >> 20 ;
    fMWSmallCaps                = ( a32Bit &  0x00200000 ) >> 21 ;
    fCompatibilityOptions_Unknown1_23       = ( a32Bit &  0x00400000 ) >> 22 ;
    fCompatibilityOptions_Unknown1_24       = ( a32Bit &  0x00800800 ) >> 23 ;
    fCompatibilityOptions_Unknown1_25       = ( a32Bit &  0x01000000 ) >> 24 ;
    fCompatibilityOptions_Unknown1_26       = ( a32Bit &  0x02000000 ) >> 25 ;
    fCompatibilityOptions_Unknown1_27       = ( a32Bit &  0x04000000 ) >> 26 ;
    fCompatibilityOptions_Unknown1_28       = ( a32Bit &  0x08000000 ) >> 27 ;
    fCompatibilityOptions_Unknown1_29       = ( a32Bit &  0x10000000 ) >> 28 ;
    fCompatibilityOptions_Unknown1_30       = ( a32Bit &  0x20000000 ) >> 29 ;
    fCompatibilityOptions_Unknown1_31       = ( a32Bit &  0x40000000 ) >> 30 ;

    fUsePrinterMetrics          = ( a32Bit &  0x80000000 ) >> 31 ;
}

sal_uInt32 WW8Dop::GetCompatibilityOptions() const
{
    sal_uInt32 a32Bit = 0;
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
    if (fCompatibilityOptions_Unknown1_13)          a32Bit |= 0x00001000;
    if (fExpShRtn)                      a32Bit |= 0x00002000; // #i56856#
    if (fCompatibilityOptions_Unknown1_15)          a32Bit |= 0x00004000;
    if (fCompatibilityOptions_Unknown1_16)          a32Bit |= 0x00008000;
    if (fSuppressTopSpacingMac5)        a32Bit |= 0x00010000;
    if (fTruncDxaExpand)                a32Bit |= 0x00020000;
    if (fPrintBodyBeforeHdr)            a32Bit |= 0x00040000;
    if (fNoLeading)                     a32Bit |= 0x00080000;
    if (fCompatibilityOptions_Unknown1_21)          a32Bit |= 0x00100000;
    if (fMWSmallCaps)                   a32Bit |= 0x00200000;
    if (fCompatibilityOptions_Unknown1_23)          a32Bit |= 0x00400000;
    if (fCompatibilityOptions_Unknown1_24)          a32Bit |= 0x00800000;
    if (fCompatibilityOptions_Unknown1_25)          a32Bit |= 0x01000000;
    if (fCompatibilityOptions_Unknown1_26)          a32Bit |= 0x02000000;
    if (fCompatibilityOptions_Unknown1_27)          a32Bit |= 0x04000000;
    if (fCompatibilityOptions_Unknown1_28)          a32Bit |= 0x08000000;
    if (fCompatibilityOptions_Unknown1_29)          a32Bit |= 0x10000000;
    if (fCompatibilityOptions_Unknown1_30)          a32Bit |= 0x20000000;
    if (fCompatibilityOptions_Unknown1_31)          a32Bit |= 0x40000000;
    if (fUsePrinterMetrics)             a32Bit |= 0x80000000;
    return a32Bit;
}

// i#78591#
void WW8Dop::SetCompatibilityOptions2(sal_uInt32 a32Bit)
{
    fCompatibilityOptions_Unknown2_1                        = ( a32Bit &  0x00000001 );
    fCompatibilityOptions_Unknown2_2                        = ( a32Bit &  0x00000002 ) >>  1 ;
    fDontUseHTMLAutoSpacing     = ( a32Bit &  0x00000004 ) >>  2 ;
    fCompatibilityOptions_Unknown2_4                    = ( a32Bit &  0x00000008 ) >>  3 ;
       fCompatibilityOptions_Unknown2_5                 = ( a32Bit &  0x00000010 ) >>  4 ;
       fCompatibilityOptions_Unknown2_6                 = ( a32Bit &  0x00000020 ) >>  5 ;
       fCompatibilityOptions_Unknown2_7                 = ( a32Bit &  0x00000040 ) >>  6 ;
       fCompatibilityOptions_Unknown2_8                 = ( a32Bit &  0x00000080 ) >>  7 ;
       fCompatibilityOptions_Unknown2_9                 = ( a32Bit &  0x00000100 ) >>  8 ;
       fCompatibilityOptions_Unknown2_10                    = ( a32Bit &  0x00000200 ) >>  9 ;
       fCompatibilityOptions_Unknown2_11                    = ( a32Bit &  0x00000400 ) >> 10 ;
       fCompatibilityOptions_Unknown2_12                    = ( a32Bit &  0x00000800 ) >> 11 ;
    fCompatibilityOptions_Unknown2_13                   = ( a32Bit &  0x00001000 ) >> 12 ;
    fCompatibilityOptions_Unknown2_14                   = ( a32Bit &  0x00002000 ) >> 13 ;
    fCompatibilityOptions_Unknown2_15                   = ( a32Bit &  0x00004000 ) >> 14 ;
    fCompatibilityOptions_Unknown2_16                   = ( a32Bit &  0x00008000 ) >> 15 ;
       fCompatibilityOptions_Unknown2_17                    = ( a32Bit &  0x00010000 ) >> 16 ;
       fCompatibilityOptions_Unknown2_18                    = ( a32Bit &  0x00020000 ) >> 17 ;
       fCompatibilityOptions_Unknown2_19                    = ( a32Bit &  0x00040000 ) >> 18 ;
       fCompatibilityOptions_Unknown2_20                    = ( a32Bit &  0x00080000 ) >> 19 ;
    fCompatibilityOptions_Unknown2_21                   = ( a32Bit &  0x00100000 ) >> 20 ;
       fCompatibilityOptions_Unknown2_22                    = ( a32Bit &  0x00200000 ) >> 21 ;
    fCompatibilityOptions_Unknown2_23                   = ( a32Bit &  0x00400000 ) >> 22 ;
    fCompatibilityOptions_Unknown2_24                   = ( a32Bit &  0x00800800 ) >> 23 ;
    fCompatibilityOptions_Unknown2_25                   = ( a32Bit &  0x01000800 ) >> 24 ;
    fCompatibilityOptions_Unknown2_26                   = ( a32Bit &  0x02000800 ) >> 25 ;
    fCompatibilityOptions_Unknown2_27                   = ( a32Bit &  0x04000800 ) >> 26 ;
    fCompatibilityOptions_Unknown2_28                   = ( a32Bit &  0x08000800 ) >> 27 ;
    fCompatibilityOptions_Unknown2_29                   = ( a32Bit &  0x10000800 ) >> 28 ;
    fCompatibilityOptions_Unknown2_30                   = ( a32Bit &  0x20000800 ) >> 29 ;
    fCompatibilityOptions_Unknown2_31                   = ( a32Bit &  0x40000800 ) >> 30 ;
       fCompatibilityOptions_Unknown2_32                    = ( a32Bit &  0x80000000 ) >> 31 ;
}

sal_uInt32 WW8Dop::GetCompatibilityOptions2() const
{
    sal_uInt32 a32Bit = 0;
    if (fCompatibilityOptions_Unknown2_1)           a32Bit |= 0x00000001;
    if (fCompatibilityOptions_Unknown2_2)           a32Bit |= 0x00000002;
    if (fDontUseHTMLAutoSpacing)     a32Bit |= 0x00000004;
    if (fCompatibilityOptions_Unknown2_4)           a32Bit |= 0x00000008;
    if (fCompatibilityOptions_Unknown2_5)           a32Bit |= 0x00000010;
    if (fCompatibilityOptions_Unknown2_6)           a32Bit |= 0x00000020;
    if (fCompatibilityOptions_Unknown2_7)           a32Bit |= 0x00000040;
    if (fCompatibilityOptions_Unknown2_8)           a32Bit |= 0x00000080;
    if (fCompatibilityOptions_Unknown2_9)           a32Bit |= 0x00000100;
    if (fCompatibilityOptions_Unknown2_10)          a32Bit |= 0x00000200;
    if (fCompatibilityOptions_Unknown2_11)          a32Bit |= 0x00000400;
    if (fCompatibilityOptions_Unknown2_12)          a32Bit |= 0x00000800;
    if (fCompatibilityOptions_Unknown2_13)          a32Bit |= 0x00001000;
    //#i42909# set thai "line breaking rules" compatibility option
    // pflin, wonder whether bUseThaiLineBreakingRules is correct
    // when importing word document.
    if (bUseThaiLineBreakingRules)          a32Bit |= 0x00002000;
    else if (fCompatibilityOptions_Unknown2_14)         a32Bit |= 0x00002000;
    if (fCompatibilityOptions_Unknown2_15)          a32Bit |= 0x00004000;
    if (fCompatibilityOptions_Unknown2_16)          a32Bit |= 0x00008000;
    if (fCompatibilityOptions_Unknown2_17)          a32Bit |= 0x00010000;
    if (fCompatibilityOptions_Unknown2_18)          a32Bit |= 0x00020000;
    if (fCompatibilityOptions_Unknown2_19)          a32Bit |= 0x00040000;
    if (fCompatibilityOptions_Unknown2_20)          a32Bit |= 0x00080000;
    if (fCompatibilityOptions_Unknown2_21)          a32Bit |= 0x00100000;
    if (fCompatibilityOptions_Unknown2_22)          a32Bit |= 0x00200000;
    if (fCompatibilityOptions_Unknown2_23)          a32Bit |= 0x00400000;
    if (fCompatibilityOptions_Unknown2_24)          a32Bit |= 0x00800000;
    if (fCompatibilityOptions_Unknown2_25)          a32Bit |= 0x01000000;
    if (fCompatibilityOptions_Unknown2_26)          a32Bit |= 0x02000000;
    if (fCompatibilityOptions_Unknown2_27)          a32Bit |= 0x04000000;
    if (fCompatibilityOptions_Unknown2_28)          a32Bit |= 0x08000000;
    if (fCompatibilityOptions_Unknown2_29)          a32Bit |= 0x10000000;
    if (fCompatibilityOptions_Unknown2_30)          a32Bit |= 0x20000000;
    if (fCompatibilityOptions_Unknown2_31)          a32Bit |= 0x40000000;
    if (fCompatibilityOptions_Unknown2_32)          a32Bit |= 0x80000000;
    return a32Bit;
}

bool WW8Dop::Write(SvStream& rStrm, WW8Fib& rFib) const
{
    const int nMaxDopLen = 610;
    sal_uInt32 nLen = 8 == rFib.nVersion ? nMaxDopLen : 84;
    rFib.fcDop =  rStrm.Tell();
    rFib.lcbDop = nLen;

    sal_uInt8 aData[ nMaxDopLen ];
    memset( aData, 0, nMaxDopLen );
    sal_uInt8* pData = aData;

    // analyse the data
    sal_uInt16 a16Bit;
    sal_uInt8   a8Bit;

    a16Bit = 0;                         // 0 0x00
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

    a16Bit = 0;                         // 2 0x02
    a16Bit |= ( 0x0003 & rncFootnote );
    a16Bit |= ( ~0x0003 & (nFootnote << 2));
    Set_UInt16( pData, a16Bit );

    a8Bit = 0;                          // 4 0x04
    if( fOutlineDirtySave ) a8Bit |= 0x01;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;                          // 5 0x05
    if( fOnlyMacPics )  a8Bit |= 0x01;
    if( fOnlyWinPics )  a8Bit |= 0x02;
    if( fLabelDoc )     a8Bit |= 0x04;
    if( fHyphCapitals ) a8Bit |= 0x08;
    if( fAutoHyphen )   a8Bit |= 0x10;
    if( fFormNoFields ) a8Bit |= 0x20;
    if( fLinkStyles )   a8Bit |= 0x40;
    if( fRevMarking )   a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;                          // 6 0x06
    if( fBackup )               a8Bit |= 0x01;
    if( fExactCWords )          a8Bit |= 0x02;
    if( fPagHidden )            a8Bit |= 0x04;
    if( fPagResults )           a8Bit |= 0x08;
    if( fLockAtn )              a8Bit |= 0x10;
    if( fMirrorMargins )        a8Bit |= 0x20;
    if( fReadOnlyRecommended )  a8Bit |= 0x40;
    if( fDfltTrueType )         a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;                          // 7 0x07
    if( fPagSuppressTopSpacing )    a8Bit |= 0x01;
    if( fProtEnabled )              a8Bit |= 0x02;
    if( fDispFormFieldSel )           a8Bit |= 0x04;
    if( fRMView )                   a8Bit |= 0x08;
    if( fRMPrint )                  a8Bit |= 0x10;
    if( fWriteReservation )         a8Bit |= 0x20;
    if( fLockRev )                  a8Bit |= 0x40;
    if( fEmbedFonts )               a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;                          // 8 0x08
    if( copts_fNoTabForInd )            a8Bit |= 0x01;
    if( copts_fNoSpaceRaiseLower )      a8Bit |= 0x02;
    if( copts_fSupressSpbfAfterPgBrk )  a8Bit |= 0x04;
    if( copts_fWrapTrailSpaces )        a8Bit |= 0x08;
    if( copts_fMapPrintTextColor )      a8Bit |= 0x10;
    if( copts_fNoColumnBalance )        a8Bit |= 0x20;
    if( copts_fConvMailMergeEsc )       a8Bit |= 0x40;
    if( copts_fSupressTopSpacing )      a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;                          // 9 0x09
    if( copts_fOrigWordTableRules )     a8Bit |= 0x01;
    if( copts_fTransparentMetafiles )   a8Bit |= 0x02;
    if( copts_fShowBreaksInFrames )     a8Bit |= 0x04;
    if( copts_fSwapBordersFacingPgs )   a8Bit |= 0x08;
    if( copts_fExpShRtn )               a8Bit |= 0x20;  // #i56856#
    Set_UInt8( pData, a8Bit );

    Set_UInt16( pData, dxaTab );        // 10 0x0a
    Set_UInt16( pData, wSpare );        // 12 0x0c
    Set_UInt16( pData, dxaHotZ );       // 14 0x0e
    Set_UInt16( pData, cConsecHypLim ); // 16 0x10
    Set_UInt16( pData, wSpare2 );       // 18 0x12
    Set_UInt32( pData, dttmCreated );   // 20 0x14
    Set_UInt32( pData, dttmRevised );   // 24 0x18
    Set_UInt32( pData, dttmLastPrint ); // 28 0x1c
    Set_UInt16( pData, nRevision );     // 32 0x20
    Set_UInt32( pData, tmEdited );      // 34 0x22
    Set_UInt32( pData, cWords );        // 38 0x26
    Set_UInt32( pData, cCh );           // 42 0x2a
    Set_UInt16( pData, cPg );           // 46 0x2e
    Set_UInt32( pData, cParas );        // 48 0x30

    a16Bit = 0;                         // 52 0x34
    a16Bit |= ( 0x0003 & rncEdn );
    a16Bit |= (~0x0003 & ( nEdn << 2));
    Set_UInt16( pData, a16Bit );

    a16Bit = 0;                         // 54 0x36
    a16Bit |= (0x0003 & epc );
    a16Bit |= (0x003c & (nfcFootnoteRef << 2));
    a16Bit |= (0x03c0 & (nfcEdnRef << 6));
    if( fPrintFormData )    a16Bit |= 0x0400;
    if( fSaveFormData )     a16Bit |= 0x0800;
    if( fShadeFormData )    a16Bit |= 0x1000;
    if( fWCFootnoteEdn )         a16Bit |= 0x8000;
    Set_UInt16( pData, a16Bit );

    Set_UInt32( pData, cLines );        // 56 0x38
    Set_UInt32( pData, cWordsFootnoteEnd );  // 60 0x3c
    Set_UInt32( pData, cChFootnoteEdn );     // 64 0x40
    Set_UInt16( pData, cPgFootnoteEdn );     // 68 0x44
    Set_UInt32( pData, cParasFootnoteEdn );  // 70 0x46
    Set_UInt32( pData, cLinesFootnoteEdn );  // 74 0x4a
    Set_UInt32( pData, lKeyProtDoc );   // 78 0x4e

    a16Bit = 0;                         // 82 0x52
    if (wvkSaved)
        a16Bit |= 0x0007;
    a16Bit |= (0x0ff8 & (wScaleSaved << 3));
    a16Bit |= (0x3000 & (zkSaved << 12));
    Set_UInt16( pData, a16Bit );

    if( 8 == rFib.nVersion )
    {
        Set_UInt32(pData, GetCompatibilityOptions());  // 84 0x54

        Set_UInt16( pData, adt );                      // 88 0x58

        doptypography.WriteToMem(pData);               // 400 0x190

        memcpy( pData, &dogrid, sizeof( WW8_DOGRID ));
        pData += sizeof( WW8_DOGRID );

        a16Bit = 0x12;      // set lvl to 9            // 410 0x19a
        if( fHtmlDoc )          a16Bit |= 0x0200;
        if( fSnapBorder )       a16Bit |= 0x0800;
        if( fIncludeHeader )    a16Bit |= 0x1000;
        if( fIncludeFooter )    a16Bit |= 0x2000;
        if( fForcePageSizePag ) a16Bit |= 0x4000;
        if( fMinFontSizePag )   a16Bit |= 0x8000;
        Set_UInt16( pData, a16Bit );

        a16Bit = 0;                                    // 412 0x19c
        if( fHaveVersions ) a16Bit |= 0x0001;
        if( fAutoVersion )  a16Bit |= 0x0002;
        Set_UInt16( pData, a16Bit );

        pData += 12;                                   // 414 0x19e

        Set_UInt32( pData, cChWS );                    // 426 0x1aa
        Set_UInt32( pData, cChWSFootnoteEdn );              // 430 0x1ae
        Set_UInt32( pData, grfDocEvents );             // 434 0x1b2

        pData += 4+30+8;  // 438 0x1b6; 442 0x1ba; 472 0x1d8; 476 0x1dc

        Set_UInt32( pData, cDBC );                     // 480 0x1e0
        Set_UInt32( pData, cDBCFootnoteEdn );               // 484 0x1e4

        pData += 1 * sizeof( sal_Int32);                   // 488 0x1e8

        Set_UInt16( pData, nfcFootnoteRef );                // 492 0x1ec
        Set_UInt16( pData, nfcEdnRef );                // 494 0x1ee
        Set_UInt16( pData, hpsZoonFontPag );           // 496 0x1f0
        Set_UInt16( pData, dywDispPag );               // 498 0x1f2

        //500 -> 508, Appear to be repeated here in 2000+
        pData += 8;
        Set_UInt32(pData, GetCompatibilityOptions());
        Set_UInt32(pData, GetCompatibilityOptions2());
        pData += 32;

        a16Bit = 0;
        if (fEmbedFactoids)
            a16Bit |= 0x8;
        if (fAcetateShowMarkup)
            a16Bit |= 0x1000;
        //Word XP at least requires fAcetateShowMarkup to honour fAcetateShowAtn
        if (fAcetateShowAtn)
        {
            a16Bit |= 0x1000;
            a16Bit |= 0x2000;
        }
        Set_UInt16(pData, a16Bit);

        pData += 48;
        a16Bit = 0x0080;
        Set_UInt16(pData, a16Bit);
    }
    rStrm.Write( aData, nLen );
    return 0 == rStrm.GetError();
}

void WW8DopTypography::ReadFromMem(sal_uInt8 *&pData)
{
    sal_uInt16 a16Bit = Get_UShort(pData);
    fKerningPunct = (a16Bit & 0x0001);
    iJustification = (a16Bit & 0x0006) >>  1;
    iLevelOfKinsoku = (a16Bit & 0x0018) >>  3;
    f2on1 = (a16Bit & 0x0020) >>  5;
    reserved1 = (a16Bit & 0x03C0) >>  6;
    reserved2 = (a16Bit & 0xFC00) >>  10;

    cchFollowingPunct = Get_Short(pData);
    cchLeadingPunct = Get_Short(pData);

    sal_Int16 i;
    for (i=0; i < nMaxFollowing; ++i)
        rgxchFPunct[i] = Get_Short(pData);
    for (i=0; i < nMaxLeading; ++i)
        rgxchLPunct[i] = Get_Short(pData);

    if (cchFollowingPunct >= 0 && cchFollowingPunct < nMaxFollowing)
        rgxchFPunct[cchFollowingPunct]=0;
    else
        rgxchFPunct[nMaxFollowing - 1]=0;

    if (cchLeadingPunct >= 0 && cchLeadingPunct < nMaxLeading)
        rgxchLPunct[cchLeadingPunct]=0;
    else
        rgxchLPunct[nMaxLeading - 1]=0;

}

void WW8DopTypography::WriteToMem(sal_uInt8 *&pData) const
{
    sal_uInt16 a16Bit = sal_uInt16(fKerningPunct);
    a16Bit |= (iJustification << 1) & 0x0006;
    a16Bit |= (iLevelOfKinsoku << 3) & 0x0018;
    a16Bit |= (int(f2on1) << 5) & 0x0020;
    a16Bit |= (reserved1 << 6) & 0x03C0;
    a16Bit |= (reserved2 << 10) & 0xFC00;
    Set_UInt16(pData,a16Bit);

    Set_UInt16(pData,cchFollowingPunct);
    Set_UInt16(pData,cchLeadingPunct);

    sal_Int16 i;
    for (i=0; i < nMaxFollowing; ++i)
        Set_UInt16(pData,rgxchFPunct[i]);
    for (i=0; i < nMaxLeading; ++i)
        Set_UInt16(pData,rgxchLPunct[i]);
}

sal_uInt16 WW8DopTypography::GetConvertedLang() const
{
    sal_uInt16 nLang;
    //I have assumed peoples republic/taiwan == simplified/traditional

    //This isn't a documented issue, so we might have it all wrong,
    //i.e. i.e. whats with the powers of two ?

    /*
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
            OSL_ENSURE(false, "Unknown MS Asian Typography language, report");
            nLang = LANGUAGE_CHINESE_SIMPLIFIED_LEGACY;
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

//              Sprms

sal_uInt16 wwSprmParser::GetSprmTailLen(sal_uInt16 nId, const sal_uInt8* pSprm)
    const
{
    SprmInfo aSprm = GetSprmInfo(nId);
    sal_uInt16 nL = 0;                      // number of Bytes to read

    //sprmPChgTabs
    switch( nId )
    {
        case 23:
        case 0xC615:
            if( pSprm[1 + mnDelta] != 255 )
                nL = static_cast< sal_uInt16 >(pSprm[1 + mnDelta] + aSprm.nLen);
            else
            {
                sal_uInt8 nDel = pSprm[2 + mnDelta];
                sal_uInt8 nIns = pSprm[3 + mnDelta + 4 * nDel];

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
                    nL = static_cast< sal_uInt16 >(pSprm[1 + mnDelta] + aSprm.nLen);
                    break;
                case L_VAR2:
                    // Variable 2-Byte Length?
                    // Excl. Token + Var-Lengthbyte
                    nL = static_cast< sal_uInt16 >(SVBT16ToShort( &pSprm[1 + mnDelta] ) + aSprm.nLen - 1);
                    break;
                default:
                    OSL_ENSURE(false, "Unknown sprm variant");
                    break;
            }
            break;
    }
    return nL;
}

// one or two bytes at the beginning at the sprm id
sal_uInt16 wwSprmParser::GetSprmId(const sal_uInt8* pSp) const
{
    OSL_ENSURE(pSp, "Why GetSprmId with pSp of 0");
    if (!pSp)
        return 0;

    sal_uInt16 nId = 0;

    if (ww::IsSevenMinus(meVersion))
    {
        nId = *pSp;
        if (0x0100 < nId)
            nId = 0;
    }
    else
    {
        nId = SVBT16ToShort(pSp);
        if (0x0800 > nId)
            nId = 0;
    }

    return nId;
}

// with tokens and length byte
sal_uInt16 wwSprmParser::GetSprmSize(sal_uInt16 nId, const sal_uInt8* pSprm) const
{
    return GetSprmTailLen(nId, pSprm) + 1 + mnDelta + SprmDataOfs(nId);
}

sal_uInt8 wwSprmParser::SprmDataOfs(sal_uInt16 nId) const
{
    return GetSprmInfo(nId).nVari;
}

sal_uInt16 wwSprmParser::DistanceToData(sal_uInt16 nId) const
{
    return 1 + mnDelta + SprmDataOfs(nId);
}

sal_uInt8* wwSprmParser::findSprmData(sal_uInt16 nId, sal_uInt8* pSprms,
    sal_uInt16 nLen) const
{
    while (nLen >= MinSprmLen())
    {
        const sal_uInt16 nAktId = GetSprmId(pSprms);
        // set pointer to data
        sal_uInt16 nSize = GetSprmSize(nAktId, pSprms);

        bool bValid = nSize <= nLen;

        SAL_WARN_IF(!bValid, "sw.ww8",
            "sprm 0x" << std::hex << nAktId << std::dec << " longer than remaining bytes, " <<
            nSize << " vs " << nLen << "doc or parser is wrong");

        if (nAktId == nId && bValid) // Sprm found
            return pSprms + DistanceToData(nId);

        //Clip to available size if wrong
        nSize = std::min(nSize, nLen);
        pSprms += nSize;
        nLen -= nSize;
    }
    // Sprm not found
    return nullptr;
}

SEPr::SEPr() :
    bkc(2), fTitlePage(0), fAutoPgn(0), nfcPgn(0), fUnlocked(0), cnsPgn(0),
    fPgnRestart(0), fEndNote(1), lnc(0), grpfIhdt(0), nLnnMod(0), dxaLnn(0),
    dxaPgn(720), dyaPgn(720), fLBetween(0), vjc(0), dmBinFirst(0),
    dmBinOther(0), dmPaperReq(0), fPropRMark(0), ibstPropRMark(0),
    dttmPropRMark(0), dxtCharSpace(0), dyaLinePitch(0), clm(0), reserved1(0),
    dmOrientPage(0), iHeadingPgn(0), pgnStart(1), lnnMin(0), wTextFlow(0),
    reserved2(0), pgbApplyTo(0), pgbPageDepth(0), pgbOffsetFrom(0),
    xaPage(lLetterWidth), yaPage(lLetterHeight), xaPageNUp(lLetterWidth), yaPageNUp(lLetterHeight),
    dxaLeft(1800), dxaRight(1800), dyaTop(1440), dyaBottom(1440), dzaGutter(0),
    dyaHdrTop(720), dyaHdrBottom(720), ccolM1(0), fEvenlySpaced(1),
    reserved3(0), fBiDi(0), fFacingCol(0), fRTLGutter(0), fRTLAlignment(0),
    dxaColumns(720), dxaColumnWidth(0), dmOrientFirst(0), fLayout(0),
    reserved4(0)
{
    memset(rgdxaColumnWidthSpacing, 0, sizeof(rgdxaColumnWidthSpacing));
}

bool checkSeek(SvStream &rSt, sal_uInt32 nOffset)
{
    const sal_uInt64 nMaxSeek(rSt.Tell() + rSt.remainingSize());
    return (nOffset <= nMaxSeek && rSt.Seek(nOffset) == nOffset);
}

bool checkRead(SvStream &rSt, void *pDest, sal_uInt32 nLength)
{
    return (rSt.Read(pDest, nLength) == static_cast<sal_Size>(nLength));
}

#ifdef OSL_BIGENDIAN
void swapEndian(sal_Unicode *pString)
{
    for (sal_Unicode *pWork = pString; *pWork; ++pWork)
        *pWork = OSL_SWAPWORD(*pWork);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
