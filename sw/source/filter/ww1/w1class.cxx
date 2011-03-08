/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <string.h>
#include <tools/stream.hxx>

#include <w1class.hxx>


#ifdef DUMP
static const sal_Char* pUnknown = "?";
#define DUMPNAME(s) s
#else
#define DUMPNAME(s) 0
#endif

Ww1SingleSprm* Ww1Sprm::aTab[ 256 ];
Ww1SingleSprm* Ww1Sprm::pSingleSprm = 0;



/////////////////////////////////////////////////////////////////// Fib
Ww1Fib::Ww1Fib( SvStream& _rStream )
    : rStream(_rStream)
{
    bOK = 0 == rStream.Seek(0) &&
          rStream.Read( &aFib, sizeof( aFib )) == sizeof( aFib );
}

///////////////////////////////////////////////////////////// PlainText

Ww1PlainText::Ww1PlainText(Ww1Fib& rWwFib, ULONG nFilePos, ULONG nCountBytes)
    : rFib(rWwFib), ulFilePos(nFilePos), ulCountBytes(nCountBytes),
    ulSeek(0), bOK(true)
{
}

sal_Unicode Ww1PlainText::operator [] ( ULONG ulOffset )
{
    DBG_ASSERT( ulOffset<Count(), "Ww1PlainText" );
    sal_Unicode cRet;
    sal_Char cRead;
    if( rFib.GetStream().Seek( ulFilePos + ulOffset ) == ulFilePos+ulOffset &&
        rFib.GetStream().Read( &cRead, sizeof( cRead ) ) == sizeof( cRead ) )
    {
        cRet = ByteString::ConvertToUnicode( cRead, RTL_TEXTENCODING_MS_1252 );
    }
    else
        cRet = ' ';
    return cRet;
}

String Ww1PlainText::GetText( ULONG ulOffset, ULONG nLen ) const
{
    String sRet;
    ByteString aStr;
    DBG_ASSERT(ulOffset+nLen<Count(), "Ww1PlainText");
    if( rFib.GetStream().Seek(ulFilePos+ulOffset) == ulFilePos+ulOffset &&
        rFib.GetStream().Read( aStr.AllocBuffer( static_cast< xub_StrLen >(nLen) ), nLen ) == nLen )
        sRet = String( aStr, RTL_TEXTENCODING_MS_1252 );
    return sRet;
}

///////////////////////////////////////////////////////////////// Style
Ww1Style::Ww1Style()
    : pPapx(0), pParent(0), stcBase(0), stcNext(0), bUsed(false)
{
}

Ww1Style::~Ww1Style()
{
    delete pPapx;
}

void Ww1Style::SetDefaults(BYTE stc)
{
    if( 222 == stc )
    {
        stcBase = 222;
        stcNext = 222;
        aChpx.hpsSet(20);
    }
}

USHORT Ww1Style::ReadName( BYTE*&p, USHORT& rnCountBytes, USHORT stc )
{
    BYTE nCountBytes = SVBT8ToByte(p);
    p++;
    rnCountBytes--;
    if( !nCountBytes ) // default
    {
        static const sal_Char* __READONLY_DATA names[] =
        {
            "W1 Null",  //222
            "W1 Annotation reference",  //223
            "W1 Annotation text",  //224
            "W1 Table of contents 8",  //225
            "W1 Table of contents 7",  //226
            "W1 Table of contents 6",  //227
            "W1 Table of contents 5",  //228
            "W1 Table of contents 4",  //229
            "W1 Table of contents 3",  //230
            "W1 Table of contents 2",  //231
            "W1 Table of contents 1",  //232
            "W1 Index 7",  //233
            "W1 Index 6",  //234
            "W1 Index 5",  //235
            "W1 Index 4",  //236
            "W1 Index 3",  //237
            "W1 Index 2",  //238
            "W1 Index 1",  //239
            "W1 Line number",  //240
            "W1 Index heading",  //241
            "W1 Footer",  //242
            "W1 Header",  //243
            "W1 Footnote reference",  //244
            "W1 Footnote text",  //245
            "W1 Heading 9",  //246
            "W1 Heading 8",  //247
            "W1 Heading 7",  //248
            "W1 Heading 6",  //249
            "W1 Heading 5",  //250
            "W1 Heading 4",  //251
            "W1 Heading 3",  //252
            "W1 Heading 2",  //253
            "W1 Heading 1",  //254
            "W1 Normal indent"  //255
            };//256

        const sal_Char* pStr;
        size_t nSize(stc);
        if (!nSize)
            pStr = "W1 Normal";
        else if (nSize - 222 >= sizeof(names) / sizeof(*names))
            pStr = "?";
        else
            pStr = names[nSize-222];
        SetName(String(pStr, RTL_TEXTENCODING_MS_1252));
    }
    else if( 255 > nCountBytes ) // unused
    {
        SetName( String( (sal_Char*)p, nCountBytes, RTL_TEXTENCODING_MS_1252 ));
        p += nCountBytes;
        DBG_ASSERT(rnCountBytes>=nCountBytes, "Ww1Style");
        rnCountBytes = rnCountBytes - nCountBytes;
    }
    return 0;
}

USHORT Ww1Style::ReadChpx( BYTE*&p, USHORT& rnCountBytes )
{
    USHORT nCountBytes = SVBT8ToByte(p);
    p++;
    rnCountBytes--;
    if (nCountBytes != 255 // unused
     && nCountBytes != 0) // default
    {
        if (nCountBytes > sizeof(aChpx))
            nCountBytes = sizeof(aChpx);
        memcpy( &aChpx, p, nCountBytes );
        p += nCountBytes;
        DBG_ASSERT(rnCountBytes>=nCountBytes, "Ww1Style");
        rnCountBytes = rnCountBytes - nCountBytes;
    }
    return 0;
}

USHORT Ww1Style::ReadPapx(BYTE*&p, USHORT& rnCountBytes)
{
    USHORT nCountBytes = SVBT8ToByte(p);
    p++;
    rnCountBytes--;
    if (nCountBytes != 255)
    {
        pPapx = new Ww1SprmPapx(p, nCountBytes);
        p += nCountBytes;
        DBG_ASSERT(rnCountBytes>=nCountBytes, "Ww1Style");
        rnCountBytes = rnCountBytes - nCountBytes;
    }
    else
        pPapx = new Ww1SprmPapx(p, 0);
    return 0;
}

USHORT Ww1Style::ReadEstcp(BYTE*&p, USHORT& rnCountBytes)
{
    stcNext = SVBT8ToByte(p);
    p++;
    rnCountBytes--;
    stcBase = SVBT8ToByte(p);
    p++;
    DBG_ASSERT(rnCountBytes>0, "Ww1Style");
    rnCountBytes--;
    return 0;
}

//////////////////////////////////////////////////////////// StyleSheet
Ww1StyleSheet::Ww1StyleSheet(Ww1Fib& _rFib)
    : cstcStd(0),
    rFib(_rFib),
    bOK(FALSE)
{
    USHORT cbStshf = rFib.GetFIB().cbStshfGet();
    DBG_ASSERT(cbStshf>=17, "Ww1StyleSheet");
    for (USHORT stc=0;stc<Count();stc++)
    {
        aStyles[stc].SetParent(this);
        aStyles[stc].SetDefaults((BYTE)stc);
    }
    BYTE* del = NULL;
    if (rFib.GetStream().Seek(rFib.GetFIB().fcStshfGet())
      == (ULONG)rFib.GetFIB().fcStshfGet()
     && (del = new BYTE[cbStshf]) != NULL
     && rFib.GetStream().Read(del, cbStshf) == (ULONG)cbStshf)
        {
        BYTE* p = del;
            cstcStd = SVBT16ToShort(p);
            p += sizeof(SVBT16);
            cbStshf -= sizeof(SVBT16);
            ReadNames(p, cbStshf);
            ReadChpx(p, cbStshf);
            ReadPapx(p, cbStshf);
            ReadEstcp(p, cbStshf);
            DBG_ASSERT(cbStshf==0, "Ww1StyleSheet");
            bOK = cbStshf == 0;
        }
    delete [] del;
}

USHORT Ww1StyleSheet::ReadNames( BYTE*& p, USHORT& rnCountBytes )
{
    USHORT nCountBytes = SVBT16ToShort(p);
    p += sizeof(SVBT16);
    DBG_ASSERT(rnCountBytes>=nCountBytes, "Ww1StyleSheet");
    rnCountBytes = rnCountBytes - nCountBytes;
    nCountBytes = nCountBytes - sizeof(SVBT16);
    USHORT stcp = 0;
    while (nCountBytes > 0)
    {
        USHORT stc = (stcp - cstcStd) & 255;
        aStyles[stc].ReadName(p, nCountBytes, stc);
        stcp++;
    }
    DBG_ASSERT(nCountBytes==0, "Ww1StyleSheet");
    return 0;
}

USHORT Ww1StyleSheet::ReadChpx(BYTE*& p, USHORT& rnCountBytes)
{
    USHORT nCountBytes = SVBT16ToShort(p);
    p += sizeof(SVBT16);
    DBG_ASSERT(rnCountBytes>=nCountBytes, "Ww1StyleSheet");
    rnCountBytes = rnCountBytes - nCountBytes;
    nCountBytes = nCountBytes - sizeof(SVBT16);
    USHORT stcp = 0;
    while (nCountBytes > 0)
    {
        USHORT stc = (stcp - cstcStd) & 255;
        aStyles[stc].ReadChpx(p, nCountBytes);
        stcp++;
    }
    DBG_ASSERT(nCountBytes == 0, "Ww1StyleSheet");
    return 0;
}

USHORT Ww1StyleSheet::ReadPapx(BYTE*& p, USHORT& rnCountBytes)
{
    USHORT nCountBytes = SVBT16ToShort(p);
    p += sizeof(SVBT16);
    DBG_ASSERT(rnCountBytes>=nCountBytes, "Ww1StyleSheet");
    rnCountBytes = rnCountBytes - nCountBytes;
    nCountBytes = nCountBytes - sizeof(SVBT16);
    USHORT stcp = 0;
    while (nCountBytes > 0)
    {
        USHORT stc = (stcp - cstcStd) & 255;
        aStyles[stc].ReadPapx(p, nCountBytes);
        stcp++;
    }
    DBG_ASSERT(nCountBytes == 0, "Ww1StyleSheet");
    return 0;
}

USHORT Ww1StyleSheet::ReadEstcp(BYTE*& p, USHORT& rnCountBytes)
{
    USHORT iMac = SVBT16ToShort(p);
    p += sizeof(SVBT16);
    DBG_ASSERT(rnCountBytes>=sizeof(SVBT16), "Ww1StyleSheet");
    rnCountBytes -= sizeof(SVBT16);
    for (USHORT stcp=0;stcp<iMac;stcp++)
    {
        USHORT stc = (stcp - cstcStd) & 255;
        aStyles[stc].ReadEstcp(p, rnCountBytes);
    }
    DBG_ASSERT(rnCountBytes==0, "Ww1StyleSheet");
    return 0;
}

///////////////////////////////////////////////////////////////// Fonts

Ww1Fonts::Ww1Fonts(Ww1Fib& rInFib, ULONG nFieldFlgs)
    : pFontA(0), rFib(rInFib), nFieldFlags(nFieldFlgs), nMax(0), bOK(false)
{
    if(rFib.GetFIB().cbSttbfffnGet() > 2 ) // ueberhaupt fonts?
    {
        SVBT16 nCountBytes;
        DBG_ASSERT(rFib.GetFIB().cbSttbfffnGet() > sizeof(nCountBytes), "Ww1Fonts");
        if (rFib.GetStream().Seek(rFib.GetFIB().fcSttbfffnGet())
         == (ULONG)rFib.GetFIB().fcSttbfffnGet())
            if (rFib.GetStream().Read(nCountBytes, sizeof(nCountBytes))
             == sizeof(nCountBytes)) // Laenge steht hier nochmal
            {
                DBG_ASSERT(SVBT16ToShort(nCountBytes)
                 == rFib.GetFIB().cbSttbfffnGet(), "redundant-size missmatch");
                 // hoffentlich sind sie immer gleich
                W1_FFN* pA = (W1_FFN*)new char[rFib.GetFIB().cbSttbfffnGet()
                 - sizeof(nCountBytes)]; // Alloziere Font-Array
                //~ Ww1: new-NULL
                if (rFib.GetStream().Read(pA, rFib.GetFIB().cbSttbfffnGet()
                 - sizeof(nCountBytes)) == (ULONG)rFib.GetFIB().cbSttbfffnGet()
                 - sizeof(nCountBytes)) // lese alle Fonts
                {} //nothing

                long nLeft = rFib.GetFIB().cbSttbfffnGet()
                 - sizeof(nCountBytes); // Zaehle, wieviele Fonts enthalten
                W1_FFN* p = pA;
                while (1)
                {
                    USHORT nNextSiz;
                    nNextSiz = p->cbFfnM1Get() + 1;
                    if(nNextSiz > nLeft)
                        break;
                    nMax++;
                    nLeft -= nNextSiz;
                    if(nLeft < 1)           // naechste Laenge muss gelesen werden koennen
                        break;
                    p = (W1_FFN *)(((char*)p) + nNextSiz);
                }
                if (nMax)
                {
                    pFontA = new W1_FFN*[nMax];         // alloziere Index-Array
                    //~ Ww1: new-NULL
                    pFontA[0] = pA;                     // fuelle Index-Array
                    USHORT i;
                    for(i=1, p=pA; i<nMax; i++)
                    {
                        p = (W1_FFN*)(((char*)p) + p->cbFfnM1Get() + 1);
                        pFontA[i] = p;
                    }
                }
                else
                    pFontA = 0; // Keine Eintraege -> kein Array
            }
    }
    bOK = TRUE;
}

W1_FFN* Ww1Fonts::GetFFN(USHORT nNum)
{
    W1_FFN* pRet = NULL;
    if (pFontA)
        if (nNum < nMax)
            pRet = pFontA[nNum];
    return pRet;
}

/////////////////////////////////////////////////////////////////// DOP
Ww1Dop::Ww1Dop(Ww1Fib& _rFib)
    : rFib(_rFib)
{
    long nRead;
    memset(&aDop, 0, sizeof(aDop)); // set defaults
    if(rFib.GetFIB().cbDopGet() >= sizeof(aDop))
        nRead = sizeof(aDop);
    else
        nRead = rFib.GetFIB().cbDopGet();
    bOK = rFib.GetStream().Seek(rFib.GetFIB().fcDopGet()) ==
                (ULONG)rFib.GetFIB().fcDopGet() &&
            rFib.GetStream().Read(&aDop, nRead) == (ULONG)nRead;
}

/////////////////////////////////////////////////////////////// Picture
Ww1Picture::Ww1Picture(SvStream& rStream, ULONG ulFilePos)
    : bOK(false), pPic(0)
{
    ulFilePos &= 0xffffff; //~ ww1: warum auch immer - im highbyte steht eine 5?!?!
    SVBT32 lcb;
    if (rStream.Seek(ulFilePos) == (ULONG)ulFilePos)
        if (rStream.Read(&lcb, sizeof(lcb)) == (ULONG)sizeof(lcb))
            if (sizeof(int)>=4 || SVBT32ToUInt32(lcb) < 0x8000) //~ mdt: 64K & 16bit
                if ((pPic = (W1_PIC*)(new BYTE[SVBT32ToUInt32(lcb)])) != NULL)
                    if (rStream.Seek(ulFilePos) == (ULONG)ulFilePos)
                        if (rStream.Read(pPic, SVBT32ToUInt32(lcb)) == (ULONG)SVBT32ToUInt32(lcb))
                        {
                            DBG_ASSERT(pPic->cbHeaderGet()==sizeof(*pPic)-sizeof(pPic->rgb), "Ww1Picture");
                            bOK = true;
                        }
}

////////////////////////////////////////////////////////////////// Sprm
Ww1Sprm::Ww1Sprm(BYTE* x, USHORT _nCountBytes)
    : p(NULL),
    nCountBytes(_nCountBytes),
    bOK(FALSE),
    pArr(NULL),
    count(0)
{
    if (nCountBytes == 0)
        bOK = TRUE;
    else
        if ((p = new BYTE[nCountBytes]) != NULL)
        {
            memcpy(p, x, nCountBytes);
            if (ReCalc())
                bOK = TRUE;
        }
}

Ww1Sprm::Ww1Sprm(SvStream& rStream, ULONG ulFilePos)
    : p(NULL),
    nCountBytes(0),
    bOK(FALSE),
    pArr(NULL),
    count(0)
{
    SVBT8 x;
    ByteToSVBT8(0, x);
    if (rStream.Seek(ulFilePos) == (ULONG)ulFilePos)
        if (rStream.Read(&x, sizeof(x)) == (ULONG)sizeof(x))
            if ((nCountBytes = SVBT8ToByte(x)) == 255
             || !nCountBytes
             || (p = new BYTE[nCountBytes]) != NULL)
                if (nCountBytes == 255
                 || !nCountBytes
                 || rStream.Read(p, nCountBytes) == (ULONG)nCountBytes)
                    if (ReCalc())
                        bOK = TRUE;
}

Ww1Sprm::~Ww1Sprm()
{
    delete pArr;
    delete p;
}

USHORT Ww1SingleSprm::Size(BYTE* /*pSprm*/)
{
    return nCountBytes;
}

USHORT Ww1SingleSprmTab::Size(BYTE* pSprm) // Doc 24/25, Fastsave-Sprm
{
    DBG_ASSERT(nCountBytes==0, "Ww1SingleSprmTab");
    USHORT nRet = sizeof(SVBT8);
    USHORT nSize = SVBT8ToByte(pSprm);
    if (nSize != 255)
        nRet = nRet + nSize;
    else
    {
        USHORT nDel = SVBT8ToByte(pSprm+1) * 4;
        USHORT nIns = SVBT8ToByte(pSprm + 3 + nDel) * 3;
        nRet += nDel + nIns;
    }
    DBG_ASSERT(nRet <= 354, "Ww1SingleSprmTab");
    if (nRet > 354)
        nRet = 0;
    return nRet;
}

USHORT Ww1SingleSprmByteSized::Size(BYTE* pSprm)
{
    USHORT nRet;
    nRet = SVBT8ToByte(pSprm);
    nRet += sizeof(SVBT8);  // var. l. byte-size
//  pSprm += sizeof(SVBT8); // var. l. byte-size
    nRet = nRet + nCountBytes;
    return nRet;
}

USHORT Ww1SingleSprmWordSized::Size(BYTE* pSprm)
{
    USHORT nRet;
    nRet = SVBT16ToShort(pSprm);
    nRet += sizeof(SVBT16);  // var. l. word-size
//  pSprm += sizeof(SVBT16); // var. l. word-size
    nRet = nRet + nCountBytes;
    return nRet;
}

static BYTE nLast = 0;
static BYTE nCurrent = 0;
USHORT Ww1Sprm::GetSize(BYTE nId, BYTE* pSprm)
{
    //DBG_ASSERT( nId < sizeof(aTab) / sizeof(*aTab), "Ww1Sprm" );
    USHORT nL = 0;
    nL = GetTab(nId).Size(pSprm);
    nLast = nCurrent;
    nCurrent = nId;
    return nL;
}

BOOL Ww1Sprm::Fill(USHORT index, BYTE& nId, USHORT& nL, BYTE*& pSprm)
{
    //DBG_ASSERT( nId < sizeof(aTab) / sizeof(*aTab), "Ww1Sprm");
    DBG_ASSERT(index < Count(), "Ww1Sprm");
    pSprm = p + pArr[index];
    nId = SVBT8ToByte(pSprm);
    pSprm++;
    nL = GetTab(nId).Size(pSprm);
    return TRUE;
}

BOOL Ww1Sprm::ReCalc()
{
    BOOL bRet = TRUE;
    delete pArr;
    pArr = NULL;
    count = 0;
    if (nCountBytes != 255) // not unused?
    {
        USHORT cbsik = nCountBytes;
        BYTE* psik = p;
        while (cbsik > 0)
        {
            USHORT iLen = GetSizeBrutto(psik);
            DBG_ASSERT(iLen<=cbsik, "Ww1Sprm");
            if (iLen > cbsik)
                cbsik = 0; // ignore the rest: we are wrong...
            else
            {
                psik += iLen;
                cbsik = cbsik - iLen;
                count++;
            }
        }
        if (bRet
         && (pArr = new USHORT[count]) != NULL)
        {
            cbsik = nCountBytes;
            USHORT offset = 0;
            USHORT current = 0;
            psik = p;
            while (current<count)
            {
                pArr[current++] = offset;
                USHORT iLen = GetSizeBrutto(psik);
                psik += iLen;
                if (iLen > cbsik)
                    cbsik = 0;
                else
                    cbsik = cbsik - iLen;
                offset = offset + iLen;
            }

        }
        else
            count = 0;
    }
    return bRet;
}

void Ww1Sprm::DeinitTab()
{
    for (size_t i=0; i < sizeof(aTab)/sizeof(*aTab); ++i)
        delete aTab[i];
    memset(aTab, 0, sizeof(aTab)/sizeof(*aTab));
    delete pSingleSprm;
}

void Ww1Sprm::InitTab()
{
    memset(aTab, 0, sizeof(aTab)/sizeof(*aTab));
    pSingleSprm = new Ww1SingleSprm( 0, DUMPNAME(pUnknown));

    aTab[  2] = new Ww1SingleSprmByte(DUMPNAME("sprmPStc")); //   2 pap.istd (style code)
    aTab[  3] = new Ww1SingleSprmByteSized(0, DUMPNAME("sprmPIstdPermute")); //   3 pap.istd    permutation
    aTab[  4] = new Ww1SingleSprmByte(DUMPNAME("sprmPIncLevel")); //   4 pap.istddifference
    aTab[  5] = new Ww1SingleSprmPJc(DUMPNAME("sprmPJc")); //   5 pap.jc (justification)
    aTab[  6] = new Ww1SingleSprmBool(DUMPNAME("sprmPFSideBySide")); //   6 pap.fSideBySide
    aTab[  7] = new Ww1SingleSprmPFKeep(DUMPNAME("sprmPFKeep")); //   7 pap.fKeep
    aTab[  8] = new Ww1SingleSprmPFKeepFollow(DUMPNAME("sprmPFKeepFollow")); //   8 pap.fKeepFollow
    aTab[  9] = new Ww1SingleSprmPPageBreakBefore(DUMPNAME("sprmPPageBreakBefore")); //   9 pap.fPageBreakBefore
    aTab[ 10] = new Ww1SingleSprmByte(DUMPNAME("sprmPBrcl")); //  10 pap.brcl
    aTab[ 11] = new Ww1SingleSprmByte(DUMPNAME("sprmPBrcp")); //  11 pap.brcp
    aTab[ 12] = new Ww1SingleSprmByteSized(0, DUMPNAME("sprmPAnld")); //  12 pap.anld (ANLD structure)
    aTab[ 13] = new Ww1SingleSprmByte(DUMPNAME("sprmPNLvlAnm")); //  13 pap.nLvlAnm nn
    aTab[ 14] = new Ww1SingleSprmBool(DUMPNAME("sprmPFNoLineNumb")); //  14 ap.fNoLnn
    aTab[ 15] = new Ww1SingleSprmPChgTabsPapx(DUMPNAME("sprmPChgTabsPapx")); //  15 pap.itbdMac, ...
    aTab[ 16] = new Ww1SingleSprmPDxaRight(DUMPNAME("sprmPDxaRight")); //  16 pap.dxaRight
    aTab[ 17] = new Ww1SingleSprmPDxaLeft(DUMPNAME("sprmPDxaLeft")); //  17 pap.dxaLeft
    aTab[ 18] = new Ww1SingleSprmWord(DUMPNAME("sprmPNest")); //  18 pap.dxaNest
    aTab[ 19] = new Ww1SingleSprmPDxaLeft1(DUMPNAME("sprmPDxaLeft1")); //  19 pap.dxaLeft1
    aTab[ 20] = new Ww1SingleSprmPDyaLine(DUMPNAME("sprmPDyaLine")); //  20 pap.lspd    an LSPD
    aTab[ 21] = new Ww1SingleSprmPDyaBefore(DUMPNAME("sprmPDyaBefore")); //  21 pap.dyaBefore
    aTab[ 22] = new Ww1SingleSprmPDyaAfter(DUMPNAME("sprmPDyaAfter")); //  22 pap.dyaAfter
    aTab[ 23] = new Ww1SingleSprmTab(0, DUMPNAME(pUnknown)); // 23 pap.itbdMac, pap.rgdxaTab
    aTab[ 24] = new Ww1SingleSprmPFInTable(DUMPNAME("sprmPFInTable")); //  24 pap.fInTable
    aTab[ 25] = new Ww1SingleSprmPTtp(DUMPNAME("sprmPTtp")); //  25 pap.fTtp
    aTab[ 26] = new Ww1SingleSprmPDxaAbs(DUMPNAME("sprmPDxaAbs")); //  26 pap.dxaAbs
    aTab[ 27] = new Ww1SingleSprmPDyaAbs(DUMPNAME("sprmPDyaAbs")); //  27 pap.dyaAbs
    aTab[ 28] = new Ww1SingleSprmPDxaWidth(DUMPNAME("sprmPDxaWidth")); //  28 pap.dxaWidth
    aTab[ 29] = new Ww1SingleSprmPpc(DUMPNAME("sprmPPc")); //  29 pap.pcHorz, pap.pcVert
    aTab[ 30] = new Ww1SingleSprmPBrc10(BRC_TOP, DUMPNAME("sprmPBrcTop10")); //  30 pap.brcTop BRC10
    aTab[ 31] = new Ww1SingleSprmPBrc10(BRC_LEFT, DUMPNAME("sprmPBrcLeft10")); //  31 pap.brcLeft BRC10
    aTab[ 32] = new Ww1SingleSprmPBrc10(BRC_BOTTOM, DUMPNAME("sprmPBrcBottom10")); //  32 pap.brcBottom BRC10
    aTab[ 33] = new Ww1SingleSprmPBrc10(BRC_RIGHT, DUMPNAME("sprmPBrcRight10")); //  33 pap.brcRight BRC10
    aTab[ 34] = new Ww1SingleSprmWord(DUMPNAME("sprmPBrcBetween10")); //  34 pap.brcBetween BRC10
    aTab[ 35] = new Ww1SingleSprmPBrc10(BRC_LEFT, DUMPNAME("sprmPBrcBar10")); //  35 pap.brcBar BRC10
    aTab[ 36] = new Ww1SingleSprmPFromText(DUMPNAME("sprmPFromText10")); //  36 pap.dxaFromText dxa
    aTab[ 37] = new Ww1SingleSprmByte(DUMPNAME("sprmPWr")); //  37 pap.wr wr
    aTab[ 38] = new Ww1SingleSprmWord(DUMPNAME("sprmPBrcTop")); //  38 pap.brcTop BRC
    aTab[ 39] = new Ww1SingleSprmWord(DUMPNAME("sprmPBrcLeft")); //  39 pap.brcLeft BRC
    aTab[ 40] = new Ww1SingleSprmWord(DUMPNAME("sprmPBrcBottom")); //  40 pap.brcBottom BRC
    aTab[ 41] = new Ww1SingleSprmWord(DUMPNAME("sprmPBrcRight")); //  41 pap.brcRight BRC
    aTab[ 42] = new Ww1SingleSprmWord(DUMPNAME("sprmPBrcBetween")); //  42 pap.brcBetween BRC
    aTab[ 43] = new Ww1SingleSprmWord(DUMPNAME("sprmPBrcBar")); //  43 pap.brcBar BRC word
    aTab[ 44] = new Ww1SingleSprmBool(DUMPNAME("sprmPFNoAutoHyph")); //  44 pap.fNoAutoHyph
    aTab[ 45] = new Ww1SingleSprmWord(DUMPNAME("sprmPWHeightAbs")); //  45 pap.wHeightAbs w
    aTab[ 46] = new Ww1SingleSprmWord(DUMPNAME("sprmPDcs")); //  46 pap.dcs DCS
    aTab[ 47] = new Ww1SingleSprmWord(DUMPNAME("sprmPShd")); //  47 pap.shd SHD
    aTab[ 48] = new Ww1SingleSprmWord(DUMPNAME("sprmPDyaFromText")); //  48 pap.dyaFromText dya
    aTab[ 49] = new Ww1SingleSprmWord(DUMPNAME("sprmPDxaFromText")); //  49 pap.dxaFromText dxa
    aTab[ 50] = new Ww1SingleSprmBool(DUMPNAME("sprmPFLocked")); //  50 pap.fLocked 0 or 1 byte
    aTab[ 51] = new Ww1SingleSprmBool(DUMPNAME("sprmPFWidowControl")); //  51 pap.fWidowControl 0 or 1 byte

    aTab[ 57] = new Ww1SingleSprmByteSized(0, DUMPNAME("sprmCDefault")); //  57 whole CHP (see below) none variable length
    aTab[ 58] = new Ww1SingleSprm(0, DUMPNAME("sprmCPlain")); //  58 whole CHP (see below) none 0

    aTab[ 60] = new Ww1SingleSprm4State(DUMPNAME("sprmCFBold")); //  60 chp.fBold 0,1, 128, or 129 (see below) byte
    aTab[ 61] = new Ww1SingleSprm4State(DUMPNAME("sprmCFItalic")); //  61 chp.fItalic 0,1, 128, or 129 (see below) byte
    aTab[ 62] = new Ww1SingleSprm4State(DUMPNAME("sprmCFStrike")); //  62 chp.fStrike 0,1, 128, or 129 (see below) byte
    aTab[ 63] = new Ww1SingleSprm4State(DUMPNAME("sprmCFOutline")); //  63 chp.fOutline 0,1, 128, or 129 (see below) byte
    aTab[ 64] = new Ww1SingleSprm4State(DUMPNAME("sprmCFShadow")); //  64 chp.fShadow 0,1, 128, or 129 (see below) byte
    aTab[ 65] = new Ww1SingleSprm4State(DUMPNAME("sprmCFSmallCaps")); //  65 chp.fSmallCaps 0,1, 128, or 129 (see below) byte
    aTab[ 66] = new Ww1SingleSprm4State(DUMPNAME("sprmCFCaps")); //  66 chp.fCaps 0,1, 128, or 129 (see below) byte
    aTab[ 67] = new Ww1SingleSprm4State(DUMPNAME("sprmCFVanish")); //  67 chp.fVanish 0,1, 128, or 129 (see below) byte
    aTab[ 68] = new Ww1SingleSprmWord(DUMPNAME("sprmCFtc")); //  68 chp.ftc ftc word
    aTab[ 69] = new Ww1SingleSprmByte(DUMPNAME("sprmCKul")); //  69 chp.kul kul byte
    aTab[ 70] = new Ww1SingleSprm(3, DUMPNAME("sprmCSizePos")); //  70 chp.hps, chp.hpsPos (see below) 3 bytes
    aTab[ 71] = new Ww1SingleSprmWord(DUMPNAME("sprmCDxaSpace")); //  71 chp.dxaSpace dxa word
    aTab[ 72] = new Ww1SingleSprmWord(DUMPNAME("//")); //  72 //
    aTab[ 73] = new Ww1SingleSprmByte(DUMPNAME("sprmCIco")); //  73 chp.ico ico byte
    aTab[ 74] = new Ww1SingleSprmByte(DUMPNAME("sprmCHps")); //  74 chp.hps hps !byte!
    aTab[ 75] = new Ww1SingleSprmByte(DUMPNAME("sprmCHpsInc")); //  75 chp.hps (see below) byte
    aTab[ 76] = new Ww1SingleSprmWord(DUMPNAME("sprmCHpsPos")); //  76 chp.hpsPos hps !word!
    aTab[ 77] = new Ww1SingleSprmByte(DUMPNAME("sprmCHpsPosAdj")); //  77 chp.hpsPos hps (see below) byte
    aTab[ 78] = new Ww1SingleSprmByteSized(0, DUMPNAME(pUnknown)); //  78 ?chp.fBold, chp.fItalic, chp.fSmallCaps, ...

    aTab[ 94] = new Ww1SingleSprmByte(DUMPNAME("sprmPicBrcl")); //  94 pic.brcl brcl (see PIC structure definition) byte
    aTab[ 95] = new Ww1SingleSprmByteSized(0, DUMPNAME("sprmPicScale")); //  95 pic.mx, pic.my, pic.dxaCropleft,

    aTab[117] = new Ww1SingleSprmByte(DUMPNAME("sprmSBkc")); // 117 sep.bkc bkc byte
    aTab[118] = new Ww1SingleSprmBool(DUMPNAME("sprmSFTitlePage")); // 118 sep.fTitlePage 0 or 1 byte
    aTab[119] = new Ww1SingleSprmSColumns(DUMPNAME("sprmSCcolumns")); // 119 sep.ccolM1 # of cols - 1 word
    aTab[120] = new Ww1SingleSprmWord(DUMPNAME("sprmSDxaColumns")); // 120 sep.dxaColumns dxa word

    aTab[122] = new Ww1SingleSprmByte(DUMPNAME("sprmSNfcPgn")); // 122 sep.nfcPgn nfc byte

    aTab[125] = new Ww1SingleSprmBool(DUMPNAME("sprmSFPgnRestart")); // 125 sep.fPgnRestart 0 or 1 byte
    aTab[126] = new Ww1SingleSprmBool(DUMPNAME("sprmSFEndnote")); // 126 sep.fEndnote 0 or 1 byte
    aTab[127] = new Ww1SingleSprmByte(DUMPNAME("sprmSLnc")); // 127 sep.lnc lnc byte
    aTab[128] = new Ww1SingleSprmSGprfIhdt(DUMPNAME("sprmSGprfIhdt")); // 128 sep.grpfIhdt grpfihdt (see Headers and Footers topic) byte
    aTab[129] = new Ww1SingleSprmWord(DUMPNAME("sprmSNLnnMod")); // 129 sep.nLnnMod non-neg int. word
    aTab[130] = new Ww1SingleSprmWord(DUMPNAME("sprmSDxaLnn")); // 130 sep.dxaLnn dxa word
    aTab[131] = new Ww1SingleSprmWord(DUMPNAME("sprmSDyaHdrTop")); // 131 sep.dyaHdrTop dya word
    aTab[132] = new Ww1SingleSprmWord(DUMPNAME("sprmSDyaHdrBottom")); // 132 sep.dyaHdrBottom dya word
    aTab[133] = new Ww1SingleSprmBool(DUMPNAME("sprmSLBetween")); // 133 sep.fLBetween 0 or 1 byte
    aTab[134] = new Ww1SingleSprmByte(DUMPNAME("sprmSVjc")); // 134 sep.vjc vjc byte
    aTab[135] = new Ww1SingleSprmWord(DUMPNAME("sprmSLnnMin")); // 135 sep.lnnMin lnn word
    aTab[136] = new Ww1SingleSprmWord(DUMPNAME("sprmSPgnStart")); // 136 sep.pgnStart pgn word

    aTab[146] = new Ww1SingleSprmWord(DUMPNAME("sprmTJc")); // 146 tap.jc jc word (low order byte is significant)
    aTab[147] = new Ww1SingleSprmWord(DUMPNAME("sprmTDxaLeft")); // 147 tap.rgdxaCenter (see below) dxa word
    aTab[148] = new Ww1SingleSprmWord(DUMPNAME("sprmTDxaGapHalf")); // 148 tap.dxaGapHalf, tap.rgdxaCenter (see below) dxa word

    aTab[152] = new Ww1SingleSprmTDefTable10(DUMPNAME("sprmTDefTable10")); // 152 tap.rgdxaCenter, tap.rgtc complex (see below) variable length
    aTab[153] = new Ww1SingleSprmWord(DUMPNAME("sprmTDyaRowHeight")); // 153 tap.dyaRowHeight dya word

    aTab[158] = new Ww1SingleSprm(4, DUMPNAME("sprmTInsert")); // 158 tap.rgdxaCenter,tap.rgtc complex (see below) 4 bytes
    aTab[159] = new Ww1SingleSprmWord(DUMPNAME("sprmTDelete")); // 159 tap.rgdxaCenter, tap.rgtc complex (see below) word
    aTab[160] = new Ww1SingleSprm(4, DUMPNAME("sprmTDxaCol")); // 160 tap.rgdxaCenter complex (see below) 4 bytes
    aTab[161] = new Ww1SingleSprmWord(DUMPNAME("sprmTMerge")); // 161 tap.fFirstMerged, tap.fMerged complex (see below) word
    aTab[162] = new Ww1SingleSprmWord(DUMPNAME("sprmTSplit")); // 162 tap.fFirstMerged, tap.fMerged complex (see below) word
    aTab[163] = new Ww1SingleSprm(5, DUMPNAME("sprmTSetBrc10")); // 163 tap.rgtc[].rgbrc complex (see below) 5 bytes
}

////////////////////////////////////////////////////////////// SprmPapx
Ww1SprmPapx::Ww1SprmPapx(BYTE* pByte, USHORT nSize) :
    Ww1Sprm(Sprm(pByte, nSize), SprmSize(pByte, nSize))
{
    memset(&aPapx, 0, sizeof(aPapx));
    memcpy(&aPapx, pByte, nSize<sizeof(aPapx)?nSize:sizeof(aPapx));
}

USHORT Ww1SprmPapx::SprmSize(BYTE*, USHORT nSize)
{
    USHORT nRet = 0;
    if (nSize >= sizeof(W1_PAPX))
        nRet = nSize - ( sizeof(W1_PAPX) - 1 ); // im W1_PAPX ist das
                                                // 1. SprmByte enthalten
    return nRet;
}

BYTE* Ww1SprmPapx::Sprm(BYTE* pByte, USHORT nSize)
{
    BYTE* pRet = NULL;
    if (nSize >= sizeof(W1_PAPX))
        pRet = ((W1_PAPX*)(pByte))->grpprlGet();
    return pRet;
}

/////////////////////////////////////////////////////////////////// Plc
Ww1Plc::Ww1Plc(Ww1Fib& rInFib, ULONG ulFilePos, USHORT nInCountBytes,
    USHORT nInItemSize)
    : p(0), nCountBytes(nInCountBytes), iMac(0), nItemSize(nInItemSize),
    bOK(false), rFib(rInFib)
{
    if (!nCountBytes)
        bOK = true;
    else
    {
        if (rFib.GetStream().Seek(ulFilePos) == (ULONG)ulFilePos)
        {
            if ((p = new BYTE[nCountBytes]) != NULL)
            {
                if (rFib.GetStream().Read(p, nCountBytes) == (ULONG)nCountBytes)
                {
                    bOK = true;
                    iMac = (nCountBytes -
                        sizeof(SVBT32)) / (sizeof(SVBT32) + nItemSize);
                    DBG_ASSERT(iMac * ((USHORT)sizeof(ULONG) + nItemSize) +
                     (USHORT)sizeof(SVBT32) == nCountBytes, "Ww1Plc");
                }
            }
        }
    }
}

Ww1Plc::~Ww1Plc()
{
    delete p;
}

void Ww1Plc::Seek(ULONG ulSeek, USHORT& nIndex)
{
    if (iMac)
        for (;nIndex <= iMac && Where(nIndex) < ulSeek;nIndex++)
            ;
}

ULONG Ww1Plc::Where(USHORT nIndex)
{
    ULONG ulRet = 0xffffffff;
    DBG_ASSERT(nIndex <= iMac, "index out of bounds");
    if (iMac && nIndex <= iMac)
        ulRet = SVBT32ToUInt32(p + sizeof(SVBT32) * nIndex);
    return ulRet;
}

BYTE* Ww1Plc::GetData(USHORT nIndex)
{
    BYTE* pRet = NULL;
    DBG_ASSERT(nIndex < iMac, "index out of bounds");
    if (nIndex < iMac)
        pRet = p + (iMac + 1) * sizeof(SVBT32) +
         nIndex * nItemSize; // Pointer auf Inhalts-Array
    return pRet;
}

/////////////////////////////////////////////////////////////////// Sep

//////////////////////////////////////////////////////////////// PlcSep

//////////////////////////////////////////////////////////////// PlcPap

//////////////////////////////////////////////////////////////// PlcChp

//////////////////////////////////////////////////////////////// PlcFnr

///////////////////////////////////////////////////////////// PlcFnText

///////////////////////////////////////////////////////////// PlcFields

///////////////////////////////////////////////////////////// PlcBookmarks
// class Ww1StringList liest im Ctor eine Anzahl von P-Strings aus dem Stream
// in den Speicher und patcht sie zu C-Strings um.
// Die Anzahl wird in nMax zurueckgeliefert.
// im Index 0 stehen alle Strings nacheinander, ab Index 1 werden
// die einzelnen Strings referenziert.
Ww1StringList::Ww1StringList( SvStream& rSt, ULONG nFc, USHORT nCb )
    : pIdxA(0), nMax(0)
{
    if( nCb > 2 )            // ueberhaupt Eintraege ?
    {
        SVBT16 nCountBytes;
        DBG_ASSERT(nCb > sizeof(nCountBytes), "Ww1StringList");
        if (rSt.Seek(nFc) == (ULONG)nFc)
            if (rSt.Read(nCountBytes, sizeof(nCountBytes))
                     == sizeof(nCountBytes)) // Laenge steht hier nochmal
            {
                DBG_ASSERT(SVBT16ToShort(nCountBytes)
                         == nCb, "redundant-size missmatch");
                                    // hoffentlich sind sie immer gleich
                sal_Char* pA = new sal_Char[nCb - sizeof(nCountBytes) + 1];
                                    // Alloziere PString-Array
                //~ Ww1: new-NULL
                if (rSt.Read(pA, nCb - sizeof(nCountBytes))
                        == (ULONG)nCb - sizeof(nCountBytes))    // lese alle
                {}// do nothing
                                    // Zaehle, wieviele Fonts enthalten
                long nLeft = nCb - sizeof(nCountBytes);
                sal_Char* p = pA;
                while (1)
                {
                    USHORT nNextSiz;
                    nNextSiz = *p + 1;
                    if(nNextSiz > nLeft)
                        break;
                    nMax++;
                    nLeft -= nNextSiz;
                    if(nLeft < 1)           // naechste Laenge muss gelesen werden koennen
                        break;
                    p = p + nNextSiz;
                }
                if (nMax)
                {
                    pIdxA = new sal_Char*[nMax+1];      // alloziere Index-Array
                    pIdxA[0] = pA;                      // Index 0 : alles
                                                        // ab Index 1 C-Strings
                    pIdxA[1] = pA + 1;                  // fuelle Index-Array
                    USHORT i = 2;
                    p = pA;
                    BYTE nL = *p;
                    while(1)
                    {
                        p += nL + 1;                    // Neues Laengen-Byte
                        nL = *p;                        // merke Laenge
                        *p = '\0';                      // mach C-String draus
                        if( i > nMax )
                            break;
                        pIdxA[i] = p + 1;               // Ptr auf C-String
                        i++;
                    }
                }
                else
                    pIdxA = 0;  // Keine Eintraege -> kein Array
            }
    }
}
const String Ww1StringList::GetStr( USHORT nNum ) const
{
    String sRet;
    if( nNum <= nMax )
        sRet = String( pIdxA[ nNum+1 ], RTL_TEXTENCODING_MS_1252 );
    return sRet;
}

Ww1Bookmarks::Ww1Bookmarks(Ww1Fib& rInFib)
    : aNames(rInFib), rFib(rInFib), nIsEnd(0)
{
    pPos[0] = new Ww1PlcBookmarkPos(rFib, rFib.GetFIB().fcPlcfbkfGet(),
                                    rFib.GetFIB().cbPlcfbkfGet(), FALSE);
    nPlcIdx[0] = 0;
    pPos[1] = new Ww1PlcBookmarkPos(rFib, rFib.GetFIB().fcPlcfbklGet(),
                                    rFib.GetFIB().cbPlcfbklGet(), TRUE);
    nPlcIdx[1] = 0;
    bOK = !aNames.GetError() && !pPos[0]->GetError() && !pPos[1]->GetError();
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
void Ww1Bookmarks::operator ++( int )
{
    if( bOK )
    {
        nPlcIdx[nIsEnd]++;

        ULONG l0 = pPos[0]->Where(nPlcIdx[0]);
        ULONG l1 = pPos[1]->Where(nPlcIdx[1]);
        if( l0 < l1 )
            nIsEnd = 0;
        else if( l1 < l0 )
            nIsEnd = 1;
        else
            nIsEnd = ( nIsEnd ) ? 0 : 1;
    }
}

long Ww1Bookmarks::GetHandle() const
{
    if( bOK )
    {
        if( nIsEnd )
            return nPlcIdx[1];

        const BYTE* p = pPos[0]->GetData( nPlcIdx[0] );
        if( p )
            return SVBT16ToShort( p );
    }
    return LONG_MAX;
}

long Ww1Bookmarks::Len() const
{
    if( nIsEnd )
    {
        DBG_ASSERT( FALSE, "Falscher Aufruf (1) von Ww1Bookmarks::Len()" );
        return 0;
    }
    USHORT nEndIdx = SVBT16ToShort(pPos[0]->GetData(nPlcIdx[0]));
    return pPos[1]->Where(nEndIdx) - pPos[0]->Where(nPlcIdx[0]);
}

const String Ww1Bookmarks::GetName() const
{
    if( nIsEnd )
        return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "???" ));
    return aNames.GetStr( nPlcIdx[0] );
}

/////////////////////////////////////////////////////////////////// Fkp
Ww1Fkp::Ww1Fkp(SvStream& rStream, ULONG ulFilePos, USHORT _nItemSize) :
    nItemSize(_nItemSize),
    bOK(FALSE)
{
    if (rStream.Seek(ulFilePos) == (ULONG)ulFilePos)
        if (rStream.Read(aFkp, sizeof(aFkp)) == sizeof(aFkp))
            bOK = TRUE;
}

ULONG Ww1Fkp::Where(USHORT nIndex)
{
    ULONG lRet = 0xffffffff;
    DBG_ASSERT(nIndex<=Count(), "index out of bounds");
    if (nIndex<=Count())
        lRet = SVBT32ToUInt32(aFkp+nIndex*sizeof(SVBT32));
    return lRet;
}

BYTE* Ww1Fkp::GetData(USHORT nIndex)
{
    BYTE* pRet = NULL;
    DBG_ASSERT(nIndex<=Count(), "index out of bounds");
    if (nIndex<=Count())
        pRet = aFkp + (Count()+1) * sizeof(SVBT32) +
         nIndex * nItemSize; // beginn der strukturen
    return pRet;
}

//////////////////////////////////////////////////////////////// FkpPap
BOOL Ww1FkpPap::Fill(USHORT nIndex, BYTE*& p, USHORT& rnCountBytes)
{
    DBG_ASSERT( nIndex < Count(), "Ww1FkpPap::Fill() Index out of Range" );
    USHORT nOffset = SVBT8ToByte(GetData(nIndex)) * 2;
    if (nOffset)
    {
        DBG_ASSERT(nOffset>(USHORT)(Count()*sizeof(SVBT32)), "calc error");
//      rnCountBytes = SVBT8ToByte(aFkp+nOffset) * 2 + 1;  // SH: +1 ?????
        rnCountBytes = SVBT8ToByte(aFkp+nOffset) * 2;
        nOffset += sizeof(SVBT8);
        if( nOffset + rnCountBytes < 511 )  // SH: Assert schlug 1 zu frueh zu
            rnCountBytes++;                 // SH: Ich weiss nicht genau,
                                            // ob das letzte Byte des PAPX
                                            // genutzt wird, aber so vergessen
                                            // wir keins und sind trotzdem
                                            // auf der sicheren Seite
        DBG_ASSERT(nOffset+rnCountBytes <= 511, "calc error");
        p = aFkp + nOffset;
    }
    else
    {
        p = NULL;
        rnCountBytes = 0;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////////// FkpChp
BOOL Ww1FkpChp::Fill(USHORT nIndex, W1_CHP& aChp)
{
    DBG_ASSERT( nIndex < Count(), "Ww1FkpChp::Fill() Index out of Range" );
    memset(&aChp, 0, sizeof(aChp)); // Default, da verkuerzt gespeichert
    USHORT nOffset = GetData(nIndex)[0] * 2;
    if (nOffset)
    {
        DBG_ASSERT(nOffset>(USHORT)(Count()*sizeof(SVBT32)), "calc error");
        USHORT nCountBytes = aFkp[nOffset];
        nOffset += sizeof(SVBT8);
        DBG_ASSERT(nCountBytes <= 511-nOffset, "calc error");
        DBG_ASSERT(nCountBytes <= sizeof(aChp), "calc error");
        memcpy(&aChp, aFkp+nOffset, nCountBytes);
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////// Assoc
Ww1Assoc::Ww1Assoc(Ww1Fib& _rFib)
    : rFib(_rFib), pBuffer(NULL), bOK(FALSE)
{
    USHORT cb = rFib.GetFIB().cbSttbfAssocGet();
    USHORT i;

    for ( i = 0; i < MaxFields; i++ )
        pStrTbl[i] = NULL;
    if ((pBuffer = new sal_Char[cb]) != NULL
     && rFib.GetStream().Seek(rFib.GetFIB().fcSttbfAssocGet()) ==
        rFib.GetFIB().fcSttbfAssocGet()
     && rFib.GetStream().Read(pBuffer, cb) == cb)
    {
        USHORT j;
        DBG_ASSERT( cb == SVBT16ToShort( *(SVBT16*)pBuffer ), "size missmatch");
        for (i=0,j=sizeof(SVBT16);j<cb && i<Criteria1;i++)
        {
            pStrTbl[i] = pBuffer+j;
            j += (*pBuffer + j) + 1;
        }
        bOK = TRUE;
    }
}

String Ww1Assoc::GetStr(USHORT code)
{
    String sRet;
    DBG_ASSERT(code<MaxFields, "out of range");
    if (pStrTbl[code] != NULL)
        for( USHORT i=0;i<pStrTbl[code][0];i++ )
            sRet += String( pStrTbl[code][i+1], RTL_TEXTENCODING_MS_1252 );
    return sRet;
}

/////////////////////////////////////////////////////////////////// Pap
Ww1Pap::Ww1Pap(Ww1Fib& _rFib)
    : Ww1PlcPap(_rFib), nPlcIndex(0), nPushedPlcIndex(0xffff), nFkpIndex(0),
    nPushedFkpIndex(0xffff), ulOffset(0), pPap(0)
{
}

void Ww1Pap::Seek(ULONG ulSeek)
{
    while (ulSeek > Where())
        (*this)++;
}

// SH: Where hat einen Parameter mitbekommen, der sagt, ob bei Neuanlegen eines
// Fkps der zugehoerige Index auf 0 gesetzt werden soll
// ( darf fuer Push/Pop nicht passieren )
// Ein eleganterer Weg faellt mir auf die Schnelle nicht ein
ULONG Ww1Pap::Where( BOOL bSetIndex )
{
    ULONG ulRet = 0xffffffff;
    if (pPap == NULL)
        if (nPlcIndex < Count())
        {
            pPap = new Ww1FkpPap(rFib.GetStream(),
                        SVBT16ToShort(GetData(nPlcIndex)) << 9);
            if( bSetIndex )
                nFkpIndex = 0;
        }
    if (pPap != NULL)
//      if (nFkpIndex < pPap->Count())
        if (nFkpIndex <= pPap->Count())
            ulRet = pPap->Where(nFkpIndex) - rFib.GetFIB().fcMinGet();
    return ulRet;
}

void Ww1Pap::operator++(int)
{
    if (pPap != NULL)
//      if (++nFkpIndex >= pPap->Count())
        if (++nFkpIndex > pPap->Count())
        {
            delete pPap;
            pPap = NULL;
            nPlcIndex++;
        }
}

// SH: FindSprm sucht in grpprl nach Sprm nId
// Rueckgabe: Pointer oder 0
BOOL Ww1Pap::FindSprm(USHORT nId, BYTE* pStart, BYTE* pEnd)
{
    Ww1Sprm aSprm( pStart, static_cast< USHORT >(pEnd-pStart) );
    USHORT nC = aSprm.Count();
    USHORT i;
    BYTE nI;
    USHORT nLen;
    BYTE *pData;
    for( i = 0; i < nC; i++ ){
        aSprm.Fill( i, nI, nLen, pData );
        if( nI == nId )
            return TRUE;
    }
    return FALSE;
}

BOOL Ww1Pap::HasId0(USHORT nId)
{
    BOOL bRet = FALSE;
    UpdateIdx();

    if( !pPap ){
        DBG_ASSERT( FALSE, "Ww1Pap::HasId():: kann kein pPap erzeugen" );
        return FALSE;
    }

    BYTE* pByte;
    USHORT n;
    if( pPap->Fill(nFkpIndex, pByte, n) ){
        BYTE* p2 = ((W1_PAPX*)(pByte))->grpprlGet(); // SH: Offset fehlte
        bRet = FindSprm( nId, p2, pByte + n );
    }
    return bRet;
}

BOOL Ww1Pap::HasId(USHORT nId)
{
    BOOL bRet = FALSE;
    USHORT nPushedPlcIndex2 = nPlcIndex;
    USHORT nPushedFkpIndex2 = nFkpIndex;
    bRet = HasId0( nId );
    if (nPlcIndex != nPushedPlcIndex2)
    {
        delete pPap;
        pPap = NULL;
    }
    nPlcIndex = nPushedPlcIndex2;
    nFkpIndex = nPushedFkpIndex2;
    Where( FALSE );
    return bRet;
}

/////////////////////////////////////////////////////////////////// Chp
Ww1Chp::Ww1Chp(Ww1Fib& _rFib)
    : Ww1PlcChp(_rFib), nPlcIndex(0), nPushedPlcIndex(0xffff), nFkpIndex(0),
    nPushedFkpIndex(0xffff), ulOffset(0), pChp(0)
{
}

void Ww1Chp::Seek(ULONG ulSeek)
{
    while (ulSeek > Where())
        (*this)++;
}

// SH: Where hat einen Parameter mitbekommen, der sagt, ob bei Neuanlegen eines
// Fkps der zugehoerige Index auf 0 gesetzt werden soll
// ( darf fuer Push/Pop nicht passieren )
// Ein eleganterer Weg faellt mir auf die Schnelle nicht ein
ULONG Ww1Chp::Where( BOOL bSetIndex )
{
    ULONG ulRet = 0xffffffff;
    if (pChp == NULL)
        if (nPlcIndex < Count())
        {
            pChp = new Ww1FkpChp(rFib.GetStream(),
                        SVBT16ToShort(GetData(nPlcIndex)) << 9);
            if( bSetIndex )
                nFkpIndex = 0;
        }
    if (pChp != NULL)
//      if (nFkpIndex < pChp->Count())
        if (nFkpIndex <= pChp->Count())
            ulRet = pChp->Where(nFkpIndex) -
                     rFib.GetFIB().fcMinGet() - ulOffset;
    return ulRet;
}

void Ww1Chp::operator++(int)
{
    if (pChp != NULL)
//      if (++nFkpIndex >= pChp->Count())
        if (++nFkpIndex > pChp->Count())
        {
            delete pChp;
            pChp = NULL;
            nPlcIndex++;
        }
}

////////////////////////////////////////////////////////////// Manager
Ww1Manager::Ww1Manager(SvStream& rStrm, ULONG nFieldFlgs)
    : bOK(FALSE), bInTtp(FALSE), bInStyle(FALSE), bStopAll(FALSE), aFib(rStrm),
    aDop(aFib), aFonts(aFib, nFieldFlgs), aDoc(aFib), pDoc(&aDoc),
    ulDocSeek(0), pSeek(&ulDocSeek), aFld(aFib), pFld(&aFld), aChp(aFib),
    aPap(aFib), aFtn(aFib), aBooks(aFib),
    aSep(aFib, aDop.GetDOP().grpfIhdtGet())
{
    bOK =  !aFib.GetError()
        && !aFib.GetFIB().fComplexGet()
        && !aDoc.GetError()
        && !aSep.GetError()
        && !aPap.GetError()
        && !aChp.GetError()
        && !aFld.GetError()
        && !aFtn.GetError()
        && !aBooks.GetError();
}

BOOL Ww1Manager::HasInTable()
{
    return aPap.HasId(24); // Ww1SingleSprmPFInTable
}

BOOL Ww1Manager::HasTtp()
{
    return aPap.HasId(25); // Ww1SingleSprmPTtp
}

BOOL Ww1Manager::HasPPc()
{
    return aPap.HasId(29); // Ww1SingleSprmPPc
}

BOOL Ww1Manager::HasPDxaAbs()
{
    return aPap.HasId(26); // Ww1SingleSprmPDxaAbs
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
