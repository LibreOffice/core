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

#include <iostream>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <unotools/ucbstreamhelper.hxx>

#include <algorithm>

#include <hintids.hxx>
#include <string.h>             // memcpy()
#include <osl/endian.h>
#include <docsh.hxx>

#define _SVSTDARR_BOOLS
#include <svl/svstdarr.hxx>

#include <unotools/fltrcfg.hxx>
#include <vcl/salbtype.hxx>
#include <sot/storage.hxx>
#include <svl/zformat.hxx>
#include <sfx2/docinf.hxx>
#include <editeng/tstpitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/langitem.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <filter/msfilter/msocximex.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <swtypes.hxx>
#include <swrect.hxx>
#include <swtblfmt.hxx>
#include <txatbase.hxx>
#include <fmtcntnt.hxx>
#include <fmtpdsc.hxx>
#include <fmtrowsplt.hxx>
#include <frmatr.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <docstat.hxx>
#include <pagedesc.hxx>
#include <IMark.hxx>
#include <swtable.hxx>
#include <wrtww8.hxx>
#include <ww8par.hxx>
#include <fltini.hxx>
#include <swmodule.hxx>
#include <section.hxx>
#include <swfltopt.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <fmturl.hxx>
#include <fesh.hxx>
#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>
#include <tools/urlobj.hxx>
#include <mdiexp.hxx>           // Progress
#include <statstr.hrc>          // ResId fuer Statusleiste
#include <fmtline.hxx>
#include <fmtfsize.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/stlunosequence.hxx>
#include <writerfilter/doctok/sprmids.hxx>

#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "ww8attributeoutput.hxx"

#include <IDocumentMarkAccess.hxx>
#include <xmloff/odffields.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include "dbgoutsw.hxx"

#include <sfx2/docfile.hxx>
#include <svl/stritem.hxx>
#include <unotools/tempfile.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <osl/time.h>
#include <rtl/random.h>
#include "WW8Sttbf.hxx"
#include "WW8FibData.hxx"

using namespace sw::util;
using namespace sw::types;

/** FKP - Formatted disK Page
*/
class WW8_WrFkp
{
    BYTE* pFkp;         // gesamter Fkp ( zuerst nur FCs und Sprms )
    BYTE* pOfs;         // Pointer auf Offset-Bereich, spaeter nach pFkp kopiert
    ePLCFT ePlc;
    short nStartGrp;    // ab hier grpprls
    short nOldStartGrp;
    BYTE nItemSize;
    BYTE nIMax;         // Anzahl der Eintrags-Paare
    BYTE nOldVarLen;
    BYTE nMark;
    bool bCombined;     // true : Einfuegen verboten

    BYTE SearchSameSprm( USHORT nVarLen, const BYTE* pSprms );
public:
    WW8_WrFkp(ePLCFT ePl, WW8_FC nStartFc, bool bWrtWW8);
    ~WW8_WrFkp();
    bool Append( WW8_FC nEndFc, USHORT nVarLen = 0, const BYTE* pSprms = 0 );
    bool Combine();
    void Write( SvStream& rStrm, SwWW8WrGrf& rGrf );

    bool IsEqualPos(WW8_FC nEndFc) const
    {   return !bCombined && nIMax && nEndFc == ((INT32*)pFkp)[nIMax]; }
    void MergeToNew( short& rVarLen, BYTE *& pNewSprms );
    bool IsEmptySprm() const
    {   return !bCombined && nIMax && !nOldVarLen;  }
    void SetNewEnd( WW8_FC nEnd )
    {   ((INT32*)pFkp)[nIMax] = nEnd; }

#ifdef __WW8_NEEDS_COPY
    WW8_FC GetStartFc() const;
    WW8_FC GetEndFc() const;
#else
    WW8_FC GetStartFc() const { return ((INT32*)pFkp)[0]; };
    WW8_FC GetEndFc() const { return ((INT32*)pFkp)[nIMax]; };
#endif // defined __WW8_NEEDS_COPY

    BYTE *CopyLastSprms(BYTE &rLen, bool bVer8);
};


// -------------------------------------------------------------------------
// class WW8_WrPc sammelt alle Piece-Eintraege fuer ein Piece
// -------------------------------------------------------------------------

class WW8_WrPc
{
    WW8_CP nStartCp;                    // Start ZeichenPosition vom Text
    WW8_FC nStartFc;                    // Start File Position vom Text
    USHORT nStatus;                     // Absatzende im Piece ?

public:
    WW8_WrPc(WW8_FC nSFc, WW8_CP nSCp )
        : nStartCp( nSCp ), nStartFc( nSFc ), nStatus( 0x0040 )
    {}

    void SetStatus()                { nStatus = 0x0050; }
    USHORT GetStatus()  const       { return nStatus; }
    WW8_CP GetStartCp() const       { return nStartCp; }
    WW8_FC GetStartFc() const       { return nStartFc; }
};

class WW8_WrtBookmarks
{
private:
    SvULongs aSttCps, aEndCps;      // Array of Start- and End CPs
    SvBools aFieldMarks;       // If the bookmark is in a field result
    std::vector<String> maSwBkmkNms;    // Array of Sw - Bookmarknames
    typedef std::vector<String>::iterator myIter;

    USHORT GetPos( const String& rNm );

    //No copying
    WW8_WrtBookmarks(const WW8_WrtBookmarks&);
    WW8_WrtBookmarks& operator=(const WW8_WrtBookmarks&);
public:
    WW8_WrtBookmarks();
    ~WW8_WrtBookmarks();

    void Append( WW8_CP nStartCp, const String& rNm, const ::sw::mark::IMark* pBkmk=NULL );
    void Write( WW8Export& rWrt );
    void MoveFieldMarks(ULONG nFrom,ULONG nTo);

};

#define ANZ_DEFAULT_STYLES 16

// die Namen der StorageStreams
#define sMainStream CREATE_CONST_ASC("WordDocument")
#define sCompObj CREATE_CONST_ASC("\1CompObj")


SV_IMPL_VARARR( WW8Bytes, BYTE )
SV_IMPL_PTRARR( WW8_WrFkpPtrs, WW8_FkpPtr )

typedef WW8_WrPc* WW8_WrPcPtr;
SV_DECL_PTRARR_DEL( WW8_WrPcPtrs, WW8_WrPcPtr, 4, 4 )
SV_IMPL_PTRARR( WW8_WrPcPtrs, WW8_WrPcPtr )

static void WriteDop( WW8Export& rWrt )
{
    WW8Dop& rDop = *rWrt.pDop;

    // i#78951#, store the value of  unknown compatability options
    rDop.SetCompatabilityOptions( rWrt.pDoc->Getn32DummyCompatabilityOptions1());
    rDop.SetCompatabilityOptions2( rWrt.pDoc->Getn32DummyCompatabilityOptions2());

    rDop.fNoLeading = !rWrt.pDoc->get(IDocumentSettingAccess::ADD_EXT_LEADING);
    rDop.fUsePrinterMetrics = !rWrt.pDoc->get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE);

    // default TabStop schreiben
    const SvxTabStopItem& rTabStop =
        DefaultItemGet<SvxTabStopItem>(*rWrt.pDoc, RES_PARATR_TABSTOP);
    rDop.dxaTab = (USHORT)rTabStop[0].GetTabPos();


    // Werte aus der DocStatistik (werden aufjedenfall fuer die
    // DocStat-Felder benoetigt!)
    rDop.fWCFtnEdn = true; // because they are included in StarWriter

    const SwDocStat& rDStat = rWrt.pDoc->GetDocStat();
    rDop.cWords = rDStat.nWord;
    rDop.cCh = rDStat.nChar;
    rDop.cPg = static_cast< INT16 >(rDStat.nPage);
    rDop.cParas = rDStat.nPara;
    rDop.cLines = rDStat.nPara;

    SwDocShell *pDocShell(rWrt.pDoc->GetDocShell());
    DBG_ASSERT(pDocShell, "no SwDocShell");
    uno::Reference<document::XDocumentProperties> xDocProps;
    uno::Reference<beans::XPropertySet> xProps;
    if (pDocShell) {
        uno::Reference<lang::XComponent> xModelComp(pDocShell->GetModel(),
           uno::UNO_QUERY);
        xProps = uno::Reference<beans::XPropertySet>(xModelComp,
           uno::UNO_QUERY);
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            xModelComp, uno::UNO_QUERY_THROW);
        xDocProps = xDPS->getDocumentProperties();
        DBG_ASSERT(xDocProps.is(), "DocumentProperties is null");

        rDop.lKeyProtDoc = pDocShell->GetModifyPasswordHash();
    }

    if ((rWrt.pSepx && rWrt.pSepx->DocumentIsProtected()) ||
        rDop.lKeyProtDoc != 0)
    {
        rDop.fProtEnabled =  1;
    }
    else
    {
        rDop.fProtEnabled = 0;
    }

    if (!xDocProps.is()) {
        rDop.dttmCreated = rDop.dttmRevised = rDop.dttmLastPrint = 0x45FBAC69;
    } else {
        ::util::DateTime uDT = xDocProps->getCreationDate();
        Date aD(uDT.Day, uDT.Month, uDT.Year);
        Time aT(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
        rDop.dttmCreated = sw::ms::DateTime2DTTM(DateTime(aD,aT));
        uDT = xDocProps->getModificationDate();
        Date aD2(uDT.Day, uDT.Month, uDT.Year);
        Time aT2(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
        rDop.dttmRevised = sw::ms::DateTime2DTTM(DateTime(aD2,aT2));
        uDT = xDocProps->getPrintDate();
        Date aD3(uDT.Day, uDT.Month, uDT.Year);
        Time aT3(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
        rDop.dttmLastPrint = sw::ms::DateTime2DTTM(DateTime(aD3,aT3));

    }

//  auch damit werden die DocStat-Felder in Kopf-/Fusszeilen nicht korrekt
//  berechnet.
//  ( we do not have this fields! )

    // und noch fuer die Header und Footers
    rDop.cWordsFtnEnd   = rDStat.nWord;
    rDop.cChFtnEdn      = rDStat.nChar;
    rDop.cPgFtnEdn      = (INT16)rDStat.nPage;
    rDop.cParasFtnEdn   = rDStat.nPara;
    rDop.cLinesFtnEdn   = rDStat.nPara;

    rDop.fDontUseHTMLAutoSpacing = (rWrt.pDoc->get(IDocumentSettingAccess::PARA_SPACE_MAX) != 0);

    rDop.fExpShRtn = !rWrt.pDoc->get(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK); // #i56856#

    rDop.Write( *rWrt.pTableStrm, *rWrt.pFib );
}

const sal_Unicode *WW8DopTypography::GetJapanNotBeginLevel1()
{
    static const sal_Unicode aJapanNotBeginLevel1[nMaxFollowing] =
    //Japanese Level 1
    {
        0x0021, 0x0025, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f,
        0x005d, 0x007d, 0x00a2, 0x00b0, 0x2019, 0x201d, 0x2030, 0x2032,
        0x2033, 0x2103, 0x3001, 0x3002, 0x3005, 0x3009, 0x300b, 0x300d,
        0x300f, 0x3011, 0x3015, 0x309b, 0x309c, 0x309d, 0x309e, 0x30fb,
        0x30fd, 0x30fe, 0xff01, 0xff05, 0xff09, 0xff0c, 0xff0e, 0xff1a,
        0xff1b, 0xff1f, 0xff3d, 0xff5d, 0xff61, 0xff63, 0xff64, 0xff65,
        0xff9e, 0xff9f, 0xffe0
    };
    return &aJapanNotBeginLevel1[0];
}

const sal_Unicode *WW8DopTypography::GetJapanNotEndLevel1()
{
    static const sal_Unicode aJapanNotEndLevel1[nMaxLeading] =
    //Japanese Level 1
    {
        0x0024, 0x0028, 0x005b, 0x005c, 0x007b, 0x00a3, 0x00a5, 0x2018,
        0x201c, 0x3008, 0x300a, 0x300c, 0x300e, 0x3010, 0x3014, 0xff04,
        0xff08, 0xff3b, 0xff5b, 0xff62, 0xffe1, 0xffe5
    };
    return &aJapanNotEndLevel1[0];
}

int lcl_CmpBeginEndChars( const rtl::OUString& rSWStr,
    const sal_Unicode* pMSStr, int nMSStrByteLen )
{
    nMSStrByteLen /= sizeof( sal_Unicode );
    if( nMSStrByteLen > rSWStr.getLength() )
        nMSStrByteLen = rSWStr.getLength()+1;
    nMSStrByteLen *= sizeof( sal_Unicode );

    return memcmp( rSWStr.getStr(), pMSStr, nMSStrByteLen );
}

/*
Converts the OOo Asian Typography into a best fit match for Microsoft
Asian typography. This structure is actually dumped to disk within the
Dop Writer. Assumption is that rTypo is cleared to 0 on entry
*/
void WW8Export::ExportDopTypography(WW8DopTypography &rTypo)
{
    static const sal_Unicode aLangNotBegin[4][WW8DopTypography::nMaxFollowing]=
    {
        //Japanese Level 1
        {
            0x0021, 0x0025, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f,
            0x005d, 0x007d, 0x00a2, 0x00b0, 0x2019, 0x201d, 0x2030, 0x2032,
            0x2033, 0x2103, 0x3001, 0x3002, 0x3005, 0x3009, 0x300b, 0x300d,
            0x300f, 0x3011, 0x3015, 0x3041, 0x3043, 0x3045, 0x3047, 0x3049,
            0x3063, 0x3083, 0x3085, 0x3087, 0x308e, 0x309b, 0x309c, 0x309d,
            0x309e, 0x30a1, 0x30a3, 0x30a5, 0x30a7, 0x30a9, 0x30c3, 0x30e3,
            0x30e5, 0x30e7, 0x30ee, 0x30f5, 0x30f6, 0x30fb, 0x30fc, 0x30fd,
            0x30fe, 0xff01, 0xff05, 0xff09, 0xff0c, 0xff0e, 0xff1a, 0xff1b,
            0xff1f, 0xff3d, 0xff5d, 0xff61, 0xff63, 0xff64, 0xff65, 0xff67,
            0xff68, 0xff69, 0xff6a, 0xff6b, 0xff6c, 0xff6d, 0xff6e, 0xff6f,
            0xff70, 0xff9e, 0xff9f, 0xffe0
        },
        //Simplified Chinese
        {
            0x0021, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f, 0x005d,
            0x007d, 0x00a8, 0x00b7, 0x02c7, 0x02c9, 0x2015, 0x2016, 0x2019,
            0x201d, 0x2026, 0x2236, 0x3001, 0x3002, 0x3003, 0x3005, 0x3009,
            0x300b, 0x300d, 0x300f, 0x3011, 0x3015, 0x3017, 0xff01, 0xff02,
            0xff07, 0xff09, 0xff0c, 0xff0e, 0xff1a, 0xff1b, 0xff1f, 0xff3d,
            0xff40, 0xff5c, 0xff5d, 0xff5e, 0xffe0
        },
        //Korean
        {
            0x0021, 0x0025, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f,
            0x005d, 0x007d, 0x00a2, 0x00b0, 0x2019, 0x201d, 0x2032, 0x2033,
            0x2103, 0x3009, 0x300b, 0x300d, 0x300f, 0x3011, 0x3015, 0xff01,
            0xff05, 0xff09, 0xff0c, 0xff0e, 0xff1a, 0xff1b, 0xff1f, 0xff3d,
            0xff5d, 0xffe0
        },
        //Traditional Chinese
        {
            0x0021, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f, 0x005d,
            0x007d, 0x00a2, 0x00b7, 0x2013, 0x2014, 0x2019, 0x201d, 0x2022,
            0x2025, 0x2026, 0x2027, 0x2032, 0x2574, 0x3001, 0x3002, 0x3009,
            0x300b, 0x300d, 0x300f, 0x3011, 0x3015, 0x301e, 0xfe30, 0xfe31,
            0xfe33, 0xfe34, 0xfe36, 0xfe38, 0xfe3a, 0xfe3c, 0xfe3e, 0xfe40,
            0xfe42, 0xfe44, 0xfe4f, 0xfe50, 0xfe51, 0xfe52, 0xfe54, 0xfe55,
            0xfe56, 0xfe57, 0xfe5a, 0xfe5c, 0xfe5e, 0xff01, 0xff09, 0xff0c,
            0xff0e, 0xff1a, 0xff1b, 0xff1f, 0xff5c, 0xff5d, 0xff64
        },
    };

    static const sal_Unicode aLangNotEnd[4][WW8DopTypography::nMaxLeading] =
    {
        //Japanese Level 1
        {
            0x0024, 0x0028, 0x005b, 0x005c, 0x007b, 0x00a3, 0x00a5, 0x2018,
            0x201c, 0x3008, 0x300a, 0x300c, 0x300e, 0x3010, 0x3014, 0xff04,
            0xff08, 0xff3b, 0xff5b, 0xff62, 0xffe1, 0xffe5
        },
        //Simplified Chinese
        {
            0x0028, 0x005b, 0x007b, 0x00b7, 0x2018, 0x201c, 0x3008, 0x300a,
            0x300c, 0x300e, 0x3010, 0x3014, 0x3016, 0xff08, 0xff0e, 0xff3b,
            0xff5b, 0xffe1, 0xffe5
        },
        //Korean
        {
            0x0028, 0x005b, 0x005c, 0x007b, 0x00a3, 0x00a5, 0x2018, 0x201c,
            0x3008, 0x300a, 0x300c, 0x300e, 0x3010, 0x3014, 0xff04, 0xff08,
            0xff3b, 0xff5b, 0xffe6
        },
        //Traditional Chinese
        {
            0x0028, 0x005b, 0x007b, 0x00a3, 0x00a5, 0x2018, 0x201c, 0x2035,
            0x3008, 0x300a, 0x300c, 0x300e, 0x3010, 0x3014, 0x301d, 0xfe35,
            0xfe37, 0xfe39, 0xfe3b, 0xfe3d, 0xfe3f, 0xfe41, 0xfe43, 0xfe59,
            0xfe5b, 0xfe5d, 0xff08, 0xff5b
        },
    };

    const i18n::ForbiddenCharacters *pForbidden = 0;
    const i18n::ForbiddenCharacters *pUseMe = 0;
    BYTE nUseReserved=0;
    int nNoNeeded=0;
    /*
    Now we have some minor difficult issues, to wit...
    a. MicroSoft Office can only store one set of begin and end characters in
    a given document, not one per language.
    b. StarOffice has only a concept of one set of begin and end characters for
    a given language, i.e. not the two levels of kinsoku in japanese

    What is unknown as yet is if our default begin and end chars for
    japanese, chinese tradition, chinese simplified and korean are different
    in Word and Writer. I already suspect that they are different between
    different version of word itself.

    So what have come up with is to simply see if any of the four languages
    in OOo have been changed away from OUR defaults, and if one has then
    export that. If more than one has in the future we may hack in something
    which examines our document properties to see which language is used the
    most and choose that, for now we choose the first and throw an ASSERT
    */

    /*Our default Japanese Level is 2, this is a special MS hack to set this*/
    rTypo.reserved2 = 1;

    for (rTypo.reserved1=8;rTypo.reserved1>0;rTypo.reserved1-=2)
    {
        if (0 != (pForbidden = pDoc->getForbiddenCharacters(rTypo.GetConvertedLang(),
            false)))
        {
            int nIdx = (rTypo.reserved1-2)/2;
            if( lcl_CmpBeginEndChars( pForbidden->endLine,
                    aLangNotEnd[ nIdx ], sizeof(aLangNotEnd[ nIdx ]) ) ||
                lcl_CmpBeginEndChars( pForbidden->beginLine,
                    aLangNotBegin[ nIdx ], sizeof(aLangNotBegin[ nIdx ]) ) )
            {
                //One exception for Japanese, if it matches a level 1 we
                //can use one extra flag for that, rather than use a custom
                if (rTypo.GetConvertedLang() == LANGUAGE_JAPANESE)
                {
                    if (
                          !lcl_CmpBeginEndChars
                            (
                                pForbidden->endLine,
                                rTypo.GetJapanNotEndLevel1(),
                                rTypo.nMaxLeading * sizeof(sal_Unicode)
                            )
                        &&
                          !lcl_CmpBeginEndChars
                            (
                                pForbidden->beginLine,
                                rTypo.GetJapanNotBeginLevel1(),
                                rTypo.nMaxFollowing * sizeof(sal_Unicode)
                            )
                        )
                    {
                        rTypo.reserved2 = 0;
                        continue;
                    }
                }

                if (!pUseMe)
                {
                    pUseMe = pForbidden;
                    nUseReserved = rTypo.reserved1;
                    rTypo.iLevelOfKinsoku = 2;
                }
                nNoNeeded++;
            }
        }
    }

    OSL_ENSURE( nNoNeeded<=1, "Example of unexportable forbidden chars" );
    rTypo.reserved1=nUseReserved;
    if (rTypo.iLevelOfKinsoku)
    {
        rTypo.cchFollowingPunct = msword_cast<sal_Int16>
            (pUseMe->beginLine.getLength());
        if (rTypo.cchFollowingPunct > WW8DopTypography::nMaxFollowing - 1)
            rTypo.cchFollowingPunct = WW8DopTypography::nMaxFollowing - 1;

        rTypo.cchLeadingPunct = msword_cast<sal_Int16>
            (pUseMe->endLine.getLength());
        if (rTypo.cchLeadingPunct > WW8DopTypography::nMaxLeading - 1)
            rTypo.cchLeadingPunct = WW8DopTypography::nMaxLeading -1;

        memcpy(rTypo.rgxchFPunct,pUseMe->beginLine.getStr(),
            (rTypo.cchFollowingPunct+1)*2);

        memcpy(rTypo.rgxchLPunct,pUseMe->endLine.getStr(),
            (rTypo.cchLeadingPunct+1)*2);
    }

    const IDocumentSettingAccess* pIDocumentSettingAccess = GetWriter().getIDocumentSettingAccess();

    rTypo.fKerningPunct = pIDocumentSettingAccess->get(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION);
    rTypo.iJustification = pDoc->getCharacterCompressionType();
}

// HasItem ist fuer die Zusammenfassung der Doppel-Attribute
// Underline / WordLineMode und Box / Shadow.
// Es kann nur etwas gefunden werden, wenn diese Methode innerhalb
// der aufgerufenen Methoden WW8_SwAttrIter::OutAttr() und
// WW8Export::OutputItemSet() benutzt wird.
const SfxPoolItem* MSWordExportBase::HasItem( USHORT nWhich ) const
{
    const SfxPoolItem* pItem=0;
    if (pISet)
    {
        // if write a EditEngine text, then the WhichIds are greater as
        // ourer own Ids. So the Id have to translate from our into the
        // EditEngine Range
        nWhich = sw::hack::GetSetWhichFromSwDocWhich(*pISet, *pDoc, nWhich);
        if (nWhich && SFX_ITEM_SET != pISet->GetItemState(nWhich, true, &pItem))
            pItem = 0;
    }
    else if( pChpIter )
        pItem = pChpIter->HasTextItem( nWhich );
    else
    {
        OSL_ENSURE( !this, "Wo ist mein ItemSet / pChpIter ?" );
        pItem = 0;
    }
    return pItem;
}

const SfxPoolItem& MSWordExportBase::GetItem(USHORT nWhich) const
{
    const SfxPoolItem* pItem;
    if (pISet)
    {
        // if write a EditEngine text, then the WhichIds are greater as
        // ourer own Ids. So the Id have to translate from our into the
        // EditEngine Range
        nWhich = sw::hack::GetSetWhichFromSwDocWhich(*pISet, *pDoc, nWhich);
        OSL_ENSURE(nWhich != 0, "All broken, Impossible");
        pItem = &pISet->Get(nWhich, true);
    }
    else if( pChpIter )
        pItem = &pChpIter->GetItem( nWhich );
    else
    {
        OSL_ENSURE( !this, "Wo ist mein ItemSet / pChpIter ?" );
        pItem = 0;
    }
    return *pItem;
}

//------------------------------------------------------------------------------

WW8_WrPlc1::WW8_WrPlc1( USHORT nStructSz )
    : aPos( 16, 16 ), nStructSiz( nStructSz )
{
    nDataLen = 16 * nStructSz;
    pData = new BYTE[ nDataLen ];
}

WW8_WrPlc1::~WW8_WrPlc1()
{
    delete[] pData;
}

WW8_CP WW8_WrPlc1::Prev() const
{
    USHORT nLen = aPos.Count();
    OSL_ENSURE(nLen,"Prev called on empty list");
    return nLen ? aPos[nLen-1] : 0;
}

void WW8_WrPlc1::Append( WW8_CP nCp, const void* pNewData )
{
    ULONG nInsPos = aPos.Count() * nStructSiz;
    aPos.Insert( nCp, aPos.Count() );
    if( nDataLen < nInsPos + nStructSiz )
    {
        BYTE* pNew = new BYTE[ 2 * nDataLen ];
        memmove( pNew, pData, nDataLen );
        delete[] pData;
        pData = pNew;
        nDataLen *= 2;
    }
    memcpy( pData + nInsPos, pNewData, nStructSiz );
}

void WW8_WrPlc1::Finish( ULONG nLastCp, ULONG nSttCp )
{
    if( aPos.Count() )
    {
        aPos.Insert( nLastCp, aPos.Count() );
        if( nSttCp )
            for( USHORT n = 0; n < aPos.Count(); ++n )
                aPos[ n ] -= nSttCp;
    }
}


void WW8_WrPlc1::Write( SvStream& rStrm )
{
    USHORT i;
    for( i = 0; i < aPos.Count(); ++i )
        SwWW8Writer::WriteLong( rStrm, aPos[i] );
    if( i )
        rStrm.Write( pData, (i-1) * nStructSiz );
}

//------------------------------------------------------------------------------
//  Klasse WW8_WrPlcFld fuer Felder
//------------------------------------------------------------------------------


bool WW8_WrPlcFld::Write( WW8Export& rWrt )
{
    if( WW8_WrPlc1::Count() <= 1 )
        return false;

    WW8_FC *pfc;
    sal_Int32 *plc;
    switch (nTxtTyp)
    {
        case TXT_MAINTEXT:
            pfc = &rWrt.pFib->fcPlcffldMom;
            plc = &rWrt.pFib->lcbPlcffldMom;
            break;
        case TXT_HDFT:
            pfc = &rWrt.pFib->fcPlcffldHdr;
            plc = &rWrt.pFib->lcbPlcffldHdr;
            break;

        case TXT_FTN:
            pfc = &rWrt.pFib->fcPlcffldFtn;
            plc = &rWrt.pFib->lcbPlcffldFtn;
            break;

        case TXT_EDN:
            pfc = &rWrt.pFib->fcPlcffldEdn;
            plc = &rWrt.pFib->lcbPlcffldEdn;
            break;

        case TXT_ATN:
            pfc = &rWrt.pFib->fcPlcffldAtn;
            plc = &rWrt.pFib->lcbPlcffldAtn;
            break;

        case TXT_TXTBOX:
            pfc = &rWrt.pFib->fcPlcffldTxbx;
            plc = &rWrt.pFib->lcbPlcffldTxbx;
            break;

        case TXT_HFTXTBOX:
            pfc = &rWrt.pFib->fcPlcffldHdrTxbx;
            plc = &rWrt.pFib->lcbPlcffldHdrTxbx;
            break;

        default:
            pfc = plc = 0;
            break;
    }

    if( pfc && plc )
    {
        ULONG nFcStart = rWrt.pTableStrm->Tell();
        WW8_WrPlc1::Write( *rWrt.pTableStrm );
        *pfc = nFcStart;
        *plc = rWrt.pTableStrm->Tell() - nFcStart;
    }
    return true;
}

bool WW8_WrMagicTable::Write( WW8Export& rWrt )
{
    if( WW8_WrPlc1::Count() <= 1 )
        return false;
    ULONG nFcStart = rWrt.pTableStrm->Tell();
    WW8_WrPlc1::Write( *rWrt.pTableStrm );
    rWrt.pFib->fcPlcfTch = nFcStart;
    rWrt.pFib->lcbPlcfTch = rWrt.pTableStrm->Tell() - nFcStart;
    return true;
}

void WW8_WrMagicTable::Append( WW8_CP nCp, ULONG nData)
{
    SVBT32 nLittle;
    /*
    Tell the undocumented table hack that everything between here and the last
    table position is nontable text, don't do it if the previous position is
    the same as this one, as that would be a region of 0 length
    */
    if ((!Count()) || (Prev() != nCp))
    {
        UInt32ToSVBT32(nData,nLittle);
        WW8_WrPlc1::Append(nCp, nLittle);
    }
}

//--------------------------------------------------------------------------

void SwWW8Writer::FillCount( SvStream& rStrm, ULONG nCount )
{
    static const UINT32 aNulls[16] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 // 64 Byte
    };

    while (nCount > 64)
    {
        rStrm.Write( aNulls, 64 );          // in 64-Byte-Schritten
        nCount -= 64;
    }
    rStrm.Write( aNulls, nCount );          // Rest ( 0 .. 64 Bytes ) schreiben
}

ULONG SwWW8Writer::FillUntil( SvStream& rStrm, ULONG nEndPos )
{
    ULONG nCurPos = rStrm.Tell();
    if( !nEndPos )                          // nEndPos == 0 -> next Page
        nEndPos = (nCurPos + 0x1ff) & ~0x1ffUL;

    if( nEndPos > nCurPos )
        SwWW8Writer::FillCount( rStrm, nEndPos - nCurPos );
#if OSL_DEBUG_LEVEL > 1
    else
        OSL_ENSURE( nEndPos == nCurPos, "Falsches FillUntil()" );
#endif
    return rStrm.Tell();
}


//--------------------------------------------------------------------------
/*  */

WW8_WrPlcPn::WW8_WrPlcPn( WW8Export& rWr, ePLCFT ePl, WW8_FC nStartFc )
    : rWrt(rWr), nFkpStartPage(0), ePlc(ePl), nMark(0)
{
    WW8_FkpPtr pF = new WW8_WrFkp( ePlc, nStartFc, rWrt.bWrtWW8 );
    aFkps.Insert( pF, aFkps.Count() );
}

WW8_WrPlcPn::~WW8_WrPlcPn()
{
    aFkps.DeleteAndDestroy( 0, aFkps.Count() );
}

BYTE *WW8_WrPlcPn::CopyLastSprms(BYTE &rLen)
{
    WW8_FkpPtr pF = aFkps.GetObject(aFkps.Count() - 1);
    return pF->CopyLastSprms(rLen, rWrt.bWrtWW8);
}

void WW8_WrPlcPn::AppendFkpEntry(WW8_FC nEndFc,short nVarLen,const BYTE* pSprms)
{
    WW8_FkpPtr pF = aFkps.GetObject( aFkps.Count() - 1 );

    // big sprm? build the sprmPHugePapx
    BYTE* pNewSprms = (BYTE*)pSprms;
    BYTE aHugePapx[ 8 ];
    if( rWrt.bWrtWW8 && PAP == ePlc && 488 < nVarLen )
    {
        BYTE* p = aHugePapx;
        *p++ = *pSprms++;           // set style Id
        *p++ = *pSprms++;
        nVarLen -= 2;

        long nDataPos = rWrt.pDataStrm->Tell();
        SwWW8Writer::WriteShort( *rWrt.pDataStrm, nVarLen );
        rWrt.pDataStrm->Write( pSprms, nVarLen );

        Set_UInt16( p, 0x6646 );    // set SprmCode
        Set_UInt32( p, nDataPos );  // set startpos (FC) in the datastream
        nVarLen = static_cast< short >(p - aHugePapx);
        pSprms = pNewSprms = aHugePapx;
    }
    // if append at the same FC-EndPos and there are sprms, then get the old
    // sprms and erase it; they will append now with the new sprms
    else if( nVarLen && pF->IsEqualPos( nEndFc ))
        pF->MergeToNew( nVarLen, pNewSprms );
    // has the prev EndFC an empty sprm and the current is empty too, then
    // expand only the old EndFc to the new EndFc
    else if( !nVarLen && pF->IsEmptySprm() )
    {
        pF->SetNewEnd( nEndFc );
        return ;
    }

    bool bOk = pF->Append(nEndFc, nVarLen, pNewSprms);
    if( !bOk )
    {
        pF->Combine();
        pF = new WW8_WrFkp( ePlc, pF->GetEndFc(), rWrt.bWrtWW8 );// Anfang neuer Fkp
                                                    // == Ende alter Fkp
        aFkps.Insert( pF, aFkps.Count() );
        if( !pF->Append( nEndFc, nVarLen, pNewSprms ) )
        {
            OSL_ENSURE( !this, "Sprm liess sich nicht einfuegen" );
        }
    }
    if( pNewSprms != pSprms )   //Merge to new has created a new block
        delete[] pNewSprms;
}

void WW8_WrPlcPn::WriteFkps()
{
    nFkpStartPage = (USHORT) ( SwWW8Writer::FillUntil( rWrt.Strm() ) >> 9 );

    for( USHORT i = 0; i < aFkps.Count(); i++ )
        aFkps.GetObject( i )->Write( rWrt.Strm(), *rWrt.pGrf );

    if( CHP == ePlc )
    {
        rWrt.pFib->pnChpFirst = nFkpStartPage;
        rWrt.pFib->cpnBteChp = aFkps.Count();
    }
    else
    {
        rWrt.pFib->pnPapFirst = nFkpStartPage;
        rWrt.pFib->cpnBtePap = aFkps.Count();
    }
}

void WW8_WrPlcPn::WritePlc()
{
    ULONG nFcStart = rWrt.pTableStrm->Tell();
    USHORT i;

    for( i = 0; i < aFkps.Count(); i++ )
        SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                aFkps.GetObject( i )->GetStartFc() );

    SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                aFkps.GetObject( i - 1 )->GetEndFc() );

    // fuer jedes FKP die Page ausgeben
    if( rWrt.bWrtWW8)                   // fuer WW97 Long-Ausgabe
        for ( i = 0; i < aFkps.Count(); i++)
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, i + nFkpStartPage );
    else                            // fuer WW95 Short-Ausgabe
        for ( i = 0; i < aFkps.Count(); i++)
            SwWW8Writer::WriteShort( *rWrt.pTableStrm, i + nFkpStartPage );

    if( CHP == ePlc )
    {
        rWrt.pFib->fcPlcfbteChpx = nFcStart;
        rWrt.pFib->lcbPlcfbteChpx = rWrt.pTableStrm->Tell() - nFcStart;
    }
    else
    {
        rWrt.pFib->fcPlcfbtePapx = nFcStart;
        rWrt.pFib->lcbPlcfbtePapx = rWrt.pTableStrm->Tell() - nFcStart;
    }
}

//--------------------------------------------------------------------------
/*  */

WW8_WrFkp::WW8_WrFkp(ePLCFT ePl, WW8_FC nStartFc, bool bWrtWW8)
    : ePlc(ePl), nStartGrp(511), nOldStartGrp(511),
    nItemSize( ( CHP == ePl ) ? 1 : ( bWrtWW8 ? 13 : 7 )),
    nIMax(0), nOldVarLen(0), nMark(0), bCombined(false)
{
    pFkp = (BYTE*)new INT32[128];           // 512 Byte
    pOfs = (BYTE*)new INT32[128];           // 512 Byte
    memset( pFkp, 0, 4 * 128 );
    memset( pOfs, 0, 4 * 128 );
    ( (INT32*)pFkp )[0] = nStartFc;         // 0. FC-Eintrag auf nStartFc
}

WW8_WrFkp::~WW8_WrFkp()
{
    delete[] (INT32 *)pFkp;
    delete[] (INT32 *)pOfs;
}

BYTE WW8_WrFkp::SearchSameSprm( USHORT nVarLen, const BYTE* pSprms )
{
    if( 3 < nVarLen )
    {
        // if the sprms contained picture-references then never equal!
        for( BYTE n = static_cast< BYTE >(nVarLen - 1); 3 < n; --n )
            if( pSprms[ n ] == GRF_MAGIC_3 &&
                pSprms[ n-1 ] == GRF_MAGIC_2 &&
                pSprms[ n-2 ] == GRF_MAGIC_1 )
                    return 0;
    }

    short i;
    for( i = 0; i < nIMax; i++ )
    {
        BYTE nStart = pOfs[i * nItemSize];
        if( nStart )
        {                               // Hat Sprms
            const BYTE* p = pFkp + ( (USHORT)nStart << 1 );
            if( ( CHP == ePlc
                    ? (*p++ == nVarLen)
                    : (((USHORT)*p++ << 1 ) == (( nVarLen+1) & 0xfffe)) )
                && !memcmp( p, pSprms, nVarLen ) )
                    return nStart;                      // gefunden
        }
    }
    return 0;           // nicht gefunden
}

BYTE *WW8_WrFkp::CopyLastSprms(BYTE &rLen, bool bVer8)
{
    rLen=0;
    BYTE *pStart=0,*pRet=0;

    if (!bCombined)
        pStart = pOfs;
    else
        pStart = pFkp + ( nIMax + 1 ) * 4;

    BYTE nStart = *(pStart + (nIMax-1) * nItemSize);

    const BYTE* p = pFkp + ( (USHORT)nStart << 1 );

    if (!*p && bVer8)
        p++;

    if (*p)
    {
        rLen = *p++;
        if (PAP == ePlc)
            rLen *= 2;
        pRet = new BYTE[rLen];
        memcpy(pRet,p,rLen);
    }
    return pRet;
}

bool WW8_WrFkp::Append( WW8_FC nEndFc, USHORT nVarLen, const BYTE* pSprms )
{
    OSL_ENSURE( !nVarLen || pSprms, "Item-Pointer fehlt" );
    OSL_ENSURE( nVarLen < ( ( ePlc == PAP ) ? 497U : 502U ), "Sprms zu lang !" );

    if( bCombined )
    {
        OSL_ENSURE( !this, "Fkp::Append: Fkp is already combined" );
        return false;
    }
    INT32 n = ((INT32*)pFkp)[nIMax];        // letzter Eintrag
    if( nEndFc <= n )
    {
        OSL_ENSURE( nEndFc >= n, "+Fkp: FC rueckwaerts" );
        OSL_ENSURE( !nVarLen || !pSprms || nEndFc != n,
                                    "+Fkp: selber FC mehrfach benutzt" );
                        // selber FC ohne Sprm wird ohne zu mosern ignoriert.

        return true;    // ignorieren, keinen neuen Fkp anlegen
    }

    BYTE nOldP = ( nVarLen ) ? SearchSameSprm( nVarLen, pSprms ) : 0;
                                            // Kombinieren gleicher Eintraege
    short nOffset=0, nPos = nStartGrp;
    if (nVarLen && !nOldP)
    {
        nPos = PAP == ePlc
                ? ( 13 == nItemSize     // HACK: PAP und bWrtWW8 !!
                     ? (nStartGrp & 0xFFFE ) - nVarLen - 1
                     : (nStartGrp - (((nVarLen + 1) & 0xFFFE)+1)) & 0xFFFE )
                : ((nStartGrp - nVarLen - 1) & 0xFFFE);
        if( nPos < 0 )
            return false;           // Passt absolut nicht
        nOffset = nPos;             // Offset merken (kann auch ungerade sein!)
        nPos &= 0xFFFE;             // Pos fuer Sprms ( gerade Pos )
    }

    if( (USHORT)nPos <= ( nIMax + 2U ) * 4U + ( nIMax + 1U ) * nItemSize )
                                            // Passt hinter CPs und Offsets ?
        return false;                       // Nein

    ((INT32*)pFkp)[nIMax + 1] = nEndFc;     // FC eintragen

    nOldVarLen = (BYTE)nVarLen;
    if( nVarLen && !nOldP )
    {               // echt eintragen
        nOldStartGrp = nStartGrp;

        nStartGrp = nPos;
        pOfs[nIMax * nItemSize] = (BYTE)( nStartGrp >> 1 );
                                            // ( DatenAnfg >> 1 ) eintragen
        BYTE nCnt = static_cast< BYTE >(CHP == ePlc
                        ? ( nVarLen < 256 ) ? (BYTE) nVarLen : 255
                        : ( ( nVarLen + 1 ) >> 1 ));

        pFkp[ nOffset ] = nCnt;                     // DatenLaenge eintragen
        memcpy( pFkp + nOffset + 1, pSprms, nVarLen );  // Sprms speichern
    }
    else
    {
        // nicht echt eintragen ( keine Sprms oder Wiederholung )
        // DatenAnfg 0 ( keine Daten ) oder Wiederholung
        pOfs[nIMax * nItemSize] = nOldP;
    }
    nIMax++;
    return true;
}

bool WW8_WrFkp::Combine()
{
    if( bCombined )
        return false;
    if( nIMax )
        memcpy( pFkp + ( nIMax + 1 ) * 4, pOfs, nIMax * nItemSize );
    delete[] pOfs;
    pOfs = 0;
    ((BYTE*)pFkp)[511] = nIMax;
    bCombined = true;

#if defined OSL_BIGENDIAN         // Hier werden nur die FCs gedreht, die
    USHORT i;          // Sprms muessen an anderer Stelle gedreht
                                // werden
    UINT32* p;
    for( i = 0, p = (UINT32*)pFkp; i <= nIMax; i++, p++ )
        *p = SWAPLONG( *p );
#endif // ifdef OSL_BIGENDIAN

    return true;
}

void WW8_WrFkp::Write( SvStream& rStrm, SwWW8WrGrf& rGrf )
{
    Combine();                      // Falls noch nicht Combined

    BYTE* p;               //  Suche Magic fuer nPicLocFc
    BYTE* pEnd = pFkp + nStartGrp;
    for( p = pFkp + 511 - 4; p >= pEnd; p-- )
    {
        if( *p != GRF_MAGIC_1 )     // Suche nach Signatur 0x12 0x34 0x56 0xXX
            continue;
        if( *(p+1) != GRF_MAGIC_2 )
            continue;
        if( *(p+2) != GRF_MAGIC_3 )
            continue;

        SVBT32 nPos;                // Signatur gefunden
        UInt32ToSVBT32( rGrf.GetFPos(), nPos );   // FilePos der Grafik
        memcpy( p, nPos, 4 );       // Patche FilePos ueber Signatur
    }
    rStrm.Write( pFkp, 512 );
}

void WW8_WrFkp::MergeToNew( short& rVarLen, BYTE *& rpNewSprms )
{
    BYTE nStart = pOfs[ (nIMax-1) * nItemSize ];
    if( nStart )
    {   // Hat Sprms
        BYTE* p = pFkp + ( (USHORT)nStart << 1 );

        // old and new equal? Then copy only one into the new sprms
        if( nOldVarLen == rVarLen && !memcmp( p+1, rpNewSprms, nOldVarLen ))
        {
            BYTE* pNew = new BYTE[ nOldVarLen ];
            memcpy( pNew, p+1, nOldVarLen );
            rpNewSprms = pNew;
        }
        else
        {
            BYTE* pNew = new BYTE[ nOldVarLen + rVarLen ];
            memcpy( pNew, p+1, nOldVarLen );
            memcpy( pNew + nOldVarLen, rpNewSprms, rVarLen );

            rpNewSprms = pNew;
            rVarLen = rVarLen + nOldVarLen;
        }
        --nIMax;
        // if this sprms dont used from others, remove it
        bool bFnd = false;
        for (USHORT n = 0; n < nIMax; ++n)
        {
            if (nStart == pOfs[n * nItemSize])
            {
                bFnd = true;
                break;
            }
        }
        if (!bFnd)
        {
            nStartGrp = nOldStartGrp;
            memset( p, 0, nOldVarLen+1 );
        }
    }
}

#ifdef __WW8_NEEDS_COPY

WW8_FC WW8_WrFkp::GetStartFc() const
{
// wenn bCombined, dann ist das Array ab pFkp schon Bytemaessig auf LittleEndian
// umgedreht, d.h. zum Herausholen der Anfangs- und Endpositionen muss
// zurueckgedreht werden.
    if( bCombined )
        return SVBT32ToUInt32( pFkp );        // 0. Element
    return ((INT32*)pFkp)[0];
}

WW8_FC WW8_WrFkp::GetEndFc() const
{
    if( bCombined )
        return SVBT32ToUInt32( &(pFkp[nIMax*4]) );    // nIMax-tes SVBT32-Element
    return ((INT32*)pFkp)[nIMax];
}

#endif // defined __WW8_NEEDS_COPY



//--------------------------------------------------------------------------
// Methoden fuer Piece-Table-Verwaltung
//--------------------------------------------------------------------------

WW8_WrPct::WW8_WrPct(WW8_FC nfcMin, bool bSaveUniCode)
    : pPcts(new WW8_WrPcPtrs), nOldFc(nfcMin), bIsUni(bSaveUniCode)
{
    AppendPc( nOldFc, bIsUni );
}

WW8_WrPct::~WW8_WrPct()
{
    delete pPcts;
}

// Piece fuellen und neues Piece erzeugen
void WW8_WrPct::AppendPc(WW8_FC nStartFc, bool bIsUnicode)
{
    WW8_CP nStartCp = nStartFc - nOldFc;    // Textbeginn abziehen
    if ( !nStartCp )
    {
        if ( 0 != pPcts->Count() )
        {
            OSL_ENSURE( 1 == pPcts->Count(), "Leeres Piece !!");
            pPcts->DeleteAndDestroy( pPcts->Count() - 1 , 1);
        }
    }

    nOldFc = nStartFc;                      // StartFc als alten merken

    if( bIsUni )
        nStartCp >>= 1;                 // Bei Unicode Anzahl der Zeichen / 2


    if ( !bIsUnicode )
    {
        nStartFc <<= 1;                 // Adresse * 2
        nStartFc |= 0x40000000;         // Vorletztes Bit setzen fuer !Unicode
    }

    if( pPcts->Count() )
        nStartCp += pPcts->GetObject( pPcts->Count()- 1 )->GetStartCp();

    WW8_WrPcPtr pPc = new WW8_WrPc( nStartFc, nStartCp );
    pPcts->Insert( pPc, pPcts->Count() );

    bIsUni = bIsUnicode;
}


void WW8_WrPct::WritePc( WW8Export& rWrt )
{
    ULONG nPctStart;
    ULONG nOldPos, nEndPos;
    USHORT i;

    nPctStart = rWrt.pTableStrm->Tell();                    // Beginn Piece-Table
    *rWrt.pTableStrm << ( char )0x02;                       // Statusbyte PCT
    nOldPos = nPctStart + 1;                                // Position merken
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );          // Laenge folgt
    for( i = 0; i < pPcts->Count(); ++i )                   // Bereiche
        SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                pPcts->GetObject( i )->GetStartCp() );


    // die letzte Pos noch errechnen
    ULONG nStartCp = rWrt.pFib->fcMac - nOldFc;
    if( bIsUni )
        nStartCp >>= 1;             // Bei Unicode Anzahl der Zeichen / 2
    nStartCp += pPcts->GetObject( i-1 )->GetStartCp();
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nStartCp );

    // Pieceverweise
    for ( i = 0; i < pPcts->Count(); ++i )
    {
        WW8_WrPcPtr pPc = pPcts->GetObject( i );

        SwWW8Writer::WriteShort( *rWrt.pTableStrm, pPc->GetStatus());
        SwWW8Writer::WriteLong( *rWrt.pTableStrm, pPc->GetStartFc());
        SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0);          // PRM=0
    }

    // Eintraege im FIB
    rWrt.pFib->fcClx = nPctStart;
    nEndPos = rWrt.pTableStrm->Tell();
    rWrt.pFib->lcbClx = nEndPos - nPctStart;

    // und noch die Laenge eintragen
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nOldPos,
                            nEndPos - nPctStart-5 );

}

void WW8_WrPct::SetParaBreak()
{
    OSL_ENSURE( pPcts->Count(),"SetParaBreak : aPcts.Count = 0" );
    pPcts->GetObject( pPcts->Count() - 1)->SetStatus();
}

WW8_CP WW8_WrPct::Fc2Cp( ULONG nFc ) const
{
    OSL_ENSURE( nFc >= (ULONG)nOldFc, "FilePos liegt vorm letzten Piece" );
    OSL_ENSURE( pPcts->Count(), "Fc2Cp noch kein Piece vorhanden" );

    nFc -= nOldFc;
    if( bIsUni )
        nFc /= 2;
    return nFc + pPcts->GetObject( pPcts->Count() - 1 )->GetStartCp();
}

//--------------------------------------------------------------------------
/*  */

WW8_WrtBookmarks::WW8_WrtBookmarks()
    : aSttCps( 0, 16 ), aEndCps( 0, 16 )
{
}

WW8_WrtBookmarks::~WW8_WrtBookmarks()
{
}

void WW8_WrtBookmarks::Append( WW8_CP nStartCp, const String& rNm,  const ::sw::mark::IMark* )
{
    USHORT nPos = GetPos( rNm );
    if( USHRT_MAX == nPos )
    {
        // new -> insert as start position
        nPos = aSttCps.Count();
        myIter aIter = maSwBkmkNms.end();
        // sort by startposition
        //      theory: write continuous -> then the new position is at end
        while( nPos && aSttCps[ nPos - 1 ] > ULONG( nStartCp ))
        {
            --nPos;
            --aIter;
        }

        aSttCps.Insert(nStartCp, nPos);
        aEndCps.Insert(nStartCp, nPos);
        aFieldMarks.Insert(BOOL(false), nPos);
        maSwBkmkNms.insert(aIter, rNm);
    }
    else
    {
        // old -> its the end position
        OSL_ENSURE( aEndCps[ nPos ] == aSttCps[ nPos ], "end position is valid" );

        //If this bookmark was around a field in writer, then we want to move
        //it to the field result in word. The end is therefore one cp
        //backwards from the 0x15 end mark that was inserted.
        if (aFieldMarks[nPos])
            --nStartCp;

        aEndCps.Replace( nStartCp, nPos );
    }
}


void WW8_WrtBookmarks::Write( WW8Export& rWrt )
{
    USHORT nCount = aSttCps.Count(), i;
    if( nCount )
    {
        SvULongs aEndSortTab( 255 < nCount ? 255 : nCount, 4 );
        // sort then endpositions
        for( i = 0; i < nCount; ++i )
        {
            ULONG nCP = aEndCps[ i ];
            USHORT nPos = i;
            while( nPos && aEndSortTab[ nPos - 1 ] > nCP )
                --nPos;
            aEndSortTab.Insert( nCP, nPos );
        }

        // we have some bookmarks found in the document -> write them
        // first the Bookmark Name Stringtable
        rWrt.WriteAsStringTable(maSwBkmkNms, rWrt.pFib->fcSttbfbkmk,
            rWrt.pFib->lcbSttbfbkmk);

        // second the Bookmark start positions as pcf of longs
        SvStream& rStrm = rWrt.bWrtWW8 ? *rWrt.pTableStrm : rWrt.Strm();
        rWrt.pFib->fcPlcfbkf = rStrm.Tell();
        for( i = 0; i < nCount; ++i )
            SwWW8Writer::WriteLong( rStrm, aSttCps[ i ] );
        SwWW8Writer::WriteLong(rStrm, rWrt.pFib->ccpText + rWrt.pFib->ccpTxbx);
        for( i = 0; i < nCount; ++i )
        {
            ULONG nEndCP = aEndCps[ i ];
            USHORT nPos = i;
            if( aEndSortTab[ nPos ] > nEndCP )
            {
                while( aEndSortTab[ --nPos ] != nEndCP )
                    ;
            }
            else if( aEndSortTab[ nPos ] < nEndCP )
                while( aEndSortTab[ ++nPos ] != nEndCP )
                    ;

            SwWW8Writer::WriteLong( rStrm, nPos );
        }
        rWrt.pFib->lcbPlcfbkf = rStrm.Tell() - rWrt.pFib->fcPlcfbkf;

        // third the Bookmark end positions
        rWrt.pFib->fcPlcfbkl = rStrm.Tell();
        for( i = 0; i < nCount; ++i )
            SwWW8Writer::WriteLong( rStrm, aEndSortTab[ i ] );
        SwWW8Writer::WriteLong(rStrm, rWrt.pFib->ccpText + rWrt.pFib->ccpTxbx);
        rWrt.pFib->lcbPlcfbkl = rStrm.Tell() - rWrt.pFib->fcPlcfbkl;
    }
}

USHORT WW8_WrtBookmarks::GetPos( const String& rNm )
{
    USHORT nRet = USHRT_MAX, n;
    for (n = 0; n < aSttCps.Count(); ++n)
        if (rNm == maSwBkmkNms[n])
        {
            nRet = n;
            break;
        }
    return nRet;
}

void WW8_WrtBookmarks::MoveFieldMarks(ULONG nFrom, ULONG nTo)
{
    for (USHORT nI=0;nI<aSttCps.Count();++nI)
    {
        if (aSttCps[nI] == nFrom)
        {
            aSttCps[nI] = nTo;
            if (aEndCps[nI] == nFrom)
            {
                aFieldMarks[nI] = true;
                aEndCps[nI] = nTo;
            }
        }
    }
}

void WW8Export::AppendBookmarks( const SwTxtNode& rNd,
    xub_StrLen nAktPos, xub_StrLen nLen )
{
    SvPtrarr aArr( 8, 8 );
    USHORT nCntnt;
    xub_StrLen nAktEnd = nAktPos + nLen;
    if( GetWriter().GetBookmarks( rNd, nAktPos, nAktEnd, aArr ))
    {
        ULONG nNd = rNd.GetIndex(), nSttCP = Fc2Cp( Strm().Tell() );
        for( USHORT n = 0; n < aArr.Count(); ++n )
        {
            ::sw::mark::IMark& rBkmk = *(::sw::mark::IMark*)aArr[ n ];
            if(dynamic_cast< ::sw::mark::IFieldmark *>(&rBkmk))
                continue;

            const SwPosition* pPos = &rBkmk.GetMarkPos();
            const SwPosition* pOPos = 0;
            if(rBkmk.IsExpanded())
                pOPos = &rBkmk.GetOtherMarkPos();
            if( pOPos && pOPos->nNode == pPos->nNode &&
                pOPos->nContent < pPos->nContent )
            {
                pPos = pOPos;
                pOPos = &rBkmk.GetMarkPos();
            }

            if( !pOPos || ( nNd == pPos->nNode.GetIndex() &&
                ( nCntnt = pPos->nContent.GetIndex() ) >= nAktPos &&
                nCntnt < nAktEnd ) )
            {
                ULONG nCp = nSttCP + pPos->nContent.GetIndex() - nAktPos;
                pBkmks->Append(nCp, BookmarkToWord(rBkmk.GetName()), &rBkmk);
            }
            if( pOPos && nNd == pOPos->nNode.GetIndex() &&
                ( nCntnt = pOPos->nContent.GetIndex() ) >= nAktPos &&
                nCntnt < nAktEnd )
            {
                ULONG nCp = nSttCP + pOPos->nContent.GetIndex() - nAktPos;
                pBkmks->Append(nCp, BookmarkToWord(rBkmk.GetName()), &rBkmk);
            }
        }
    }
}

void WW8Export::MoveFieldMarks(ULONG nFrom, ULONG nTo)
{
    pBkmks->MoveFieldMarks(nFrom, nTo);
}

void WW8Export::AppendBookmark( const rtl::OUString& rName, bool bSkip )
{
    ULONG nSttCP = Fc2Cp( Strm().Tell() ) + ( bSkip? 1: 0 );
    pBkmks->Append( nSttCP, rName );
}

void MSWordExportBase::AppendWordBookmark( const String& rName )
{
    AppendBookmark( BookmarkToWord( rName ) );
}


//--------------------------------------------------------------------------
/*  */

void WW8_WrtRedlineAuthor::Write( Writer& rWrt )
{
    WW8Export & rWW8Wrt = *(((SwWW8Writer&)rWrt).m_pExport);
    rWW8Wrt.WriteAsStringTable(maAuthors, rWW8Wrt.pFib->fcSttbfRMark,
        rWW8Wrt.pFib->lcbSttbfRMark, rWW8Wrt.bWrtWW8 ? 0 : 2);
}

USHORT WW8Export::AddRedlineAuthor( USHORT nId )
{
    if( !pRedlAuthors )
    {
        pRedlAuthors = new WW8_WrtRedlineAuthor;
        pRedlAuthors->AddName(CREATE_CONST_ASC("Unknown"));
    }
    return pRedlAuthors->AddName( SW_MOD()->GetRedlineAuthor( nId ) );
}

//--------------------------------------------------------------------------
/*  */

void WW8Export::WriteAsStringTable(const std::vector<String>& rStrings,
    INT32& rfcSttbf, INT32& rlcbSttbf, USHORT nExtraLen)
{
    USHORT n, nCount = static_cast< USHORT >(rStrings.size());
    if( nCount )
    {
        // we have some Redlines found in the document -> the
        // Author Name Stringtable
        SvStream& rStrm = bWrtWW8 ? *pTableStrm : Strm();
        rfcSttbf = rStrm.Tell();
        if( bWrtWW8 )
        {
            SwWW8Writer::WriteShort( rStrm, -1 );
            SwWW8Writer::WriteLong( rStrm, nCount );
            for( n = 0; n < nCount; ++n )
            {
                const String& rNm = rStrings[n];
                SwWW8Writer::WriteShort( rStrm, rNm.Len() );
                SwWW8Writer::WriteString16(rStrm, rNm, false);
                if( nExtraLen )
                    SwWW8Writer::FillCount(rStrm, nExtraLen);
            }
        }
        else
        {
            SwWW8Writer::WriteShort( rStrm, 0 );
            for( n = 0; n < nCount; ++n )
            {
                const String aNm(rStrings[n].Copy(0, 255));
                rStrm << (BYTE)aNm.Len();
                SwWW8Writer::WriteString8(rStrm, aNm, false,
                    RTL_TEXTENCODING_MS_1252);
                if (nExtraLen)
                    SwWW8Writer::FillCount(rStrm, nExtraLen);
            }
        }
        rlcbSttbf = rStrm.Tell() - rfcSttbf;
        if( !bWrtWW8 )
            SwWW8Writer::WriteShort( rStrm, rfcSttbf, (USHORT)rlcbSttbf );
    }
}

// WriteShort() traegt an FilePos nPos den Wert nVal ein und seekt auf die
// alte FilePos zurueck. Benutzt zum Nachtragen von Laengen.
void SwWW8Writer::WriteShort( SvStream& rStrm, ULONG nPos, INT16 nVal )
{
    ULONG nOldPos = rStrm.Tell();       // Pos merken
    rStrm.Seek( nPos );
    SwWW8Writer::WriteShort( rStrm, nVal );
    rStrm.Seek( nOldPos );
}

void SwWW8Writer::WriteLong( SvStream& rStrm, ULONG nPos, INT32 nVal )
{
    ULONG nOldPos = rStrm.Tell();       // Pos merken
    rStrm.Seek( nPos );
    SwWW8Writer::WriteLong( rStrm, nVal );
    rStrm.Seek( nOldPos );
}

void SwWW8Writer::InsUInt16(ww::bytes &rO, sal_uInt16 n)
{
    SVBT16 nL;
    ShortToSVBT16( n, nL );
    rO.push_back(nL[0]);
    rO.push_back(nL[1]);
}

void SwWW8Writer::InsUInt32(ww::bytes &rO, sal_uInt32 n)
{
    SVBT32 nL;
    UInt32ToSVBT32( n, nL );
    rO.push_back(nL[0]);
    rO.push_back(nL[1]);
    rO.push_back(nL[2]);
    rO.push_back(nL[3]);
}

void SwWW8Writer::InsAsString16(ww::bytes &rO, const String& rStr)
{
    const sal_Unicode* pStr = rStr.GetBuffer();
    for( xub_StrLen n = 0, nLen = rStr.Len(); n < nLen; ++n, ++pStr )
        SwWW8Writer::InsUInt16( rO, *pStr );
}

void SwWW8Writer::InsAsString8(ww::bytes &rO, const String& rStr,
        rtl_TextEncoding eCodeSet)
{
    ByteString sTmp(rStr, eCodeSet);
    const sal_Char *pStart = sTmp.GetBuffer();
    const sal_Char *pEnd = pStart + sTmp.Len();
    rO.reserve(rO.size() + sTmp.Len());

    std::copy(pStart, pEnd, std::inserter(rO, rO.end()));
}

#ifdef __WW8_NEEDS_COPY

void SwWW8Writer::InsUInt16( WW8Bytes& rO, UINT16 n )
{
    SVBT16 nL;
    ShortToSVBT16( n, nL );
    rO.Insert( nL, 2, rO.Count() );
}
void SwWW8Writer::InsUInt32( WW8Bytes& rO, UINT32 n )
{
    SVBT32 nL;
    UInt32ToSVBT32( n, nL );
    rO.Insert( nL, 4, rO.Count() );
}

#else

void SwWW8Writer::InsUInt16( WW8Bytes& rO, UINT16 n )
{
    rO.Insert( (BYTE*)&n, 2, rO.Count() );
}
void SwWW8Writer::InsUInt32( WW8Bytes& rO, UINT32 n )
{
    rO.Insert( (BYTE*)&n, 4, rO.Count() );
}

#endif // defined __WW8_NEEDS_COPY

void SwWW8Writer::InsAsString16( WW8Bytes& rO, const String& rStr )
{
    const sal_Unicode* pStr = rStr.GetBuffer();
    for( xub_StrLen n = 0, nLen = rStr.Len(); n < nLen; ++n, ++pStr )
        SwWW8Writer::InsUInt16( rO, *pStr );
}

void SwWW8Writer::InsAsString8( WW8Bytes& rO, const String& rStr,
                                rtl_TextEncoding eCodeSet )
{
    ByteString sTmp( rStr, eCodeSet );
    rO.Insert( (BYTE*)sTmp.GetBuffer(), sTmp.Len(), rO.Count() );
}

void SwWW8Writer::WriteString16(SvStream& rStrm, const String& rStr,
    bool bAddZero)
{
    ww::bytes aBytes;
    SwWW8Writer::InsAsString16(aBytes, rStr);
    if (bAddZero)
        SwWW8Writer::InsUInt16(aBytes, 0);
    //vectors are guaranteed to have contiguous memory, so we can do
    //this while migrating away from WW8Bytes. Meyers Effective STL, item 16
    if (!aBytes.empty())
        rStrm.Write(&aBytes[0], aBytes.size());
}

void SwWW8Writer::WriteString_xstz(SvStream& rStrm, const String& rStr, bool bAddZero)
{
    ww::bytes aBytes;
    SwWW8Writer::InsUInt16(aBytes, rStr.Len());
    SwWW8Writer::InsAsString16(aBytes, rStr);
    if (bAddZero)
        SwWW8Writer::InsUInt16(aBytes, 0);
    rStrm.Write(&aBytes[0], aBytes.size());
}


void SwWW8Writer::WriteString8(SvStream& rStrm, const String& rStr,
    bool bAddZero, rtl_TextEncoding eCodeSet)
{
    ww::bytes aBytes;
    SwWW8Writer::InsAsString8(aBytes, rStr, eCodeSet);
    if (bAddZero)
        aBytes.push_back(0);
    //vectors are guaranteed to have contiguous memory, so we can do
    ////this while migrating away from WW8Bytes. Meyers Effective STL, item 16
    if (!aBytes.empty())
        rStrm.Write(&aBytes[0], aBytes.size());
}

void WW8Export::WriteStringAsPara( const String& rTxt, USHORT nStyleId )
{
    if( rTxt.Len() )
        OutSwString( rTxt, 0, rTxt.Len(), IsUnicode(), RTL_TEXTENCODING_MS_1252 );
    WriteCR();              // CR danach

    WW8Bytes aArr( 10, 10 );
    SwWW8Writer::InsUInt16( aArr, nStyleId );
    if( bOutTable )
    {                                               // Tab-Attr
        // sprmPFInTable
        if( bWrtWW8 )
            SwWW8Writer::InsUInt16( aArr, NS_sprm::LN_PFInTable );
        else
            aArr.Insert( 24, aArr.Count() );
        aArr.Insert( 1, aArr.Count() );
    }

    ULONG nPos = Strm().Tell();
    pPapPlc->AppendFkpEntry( nPos, aArr.Count(), aArr.GetData() );
    pChpPlc->AppendFkpEntry( nPos );
}

void MSWordExportBase::WriteSpecialText( ULONG nStart, ULONG nEnd, BYTE nTTyp )
{
    BYTE nOldTyp = nTxtTyp;
    nTxtTyp = nTTyp;
    SwPaM* pOldPam = pCurPam;       //!! Einfaches Umsetzen des PaM ohne
    SwPaM* pOldEnd = pOrigPam;          // Wiederherstellen muesste es auch tun
    bool bOldPageDescs = bOutPageDescs;
    bOutPageDescs = false;
                                    // bOutKF wird in WriteKF1 gemerkt / gesetzt
    pCurPam = Writer::NewSwPaM( *pDoc, nStart, nEnd );

    // Tabelle in Sonderbereichen erkennen
    if ( ( nStart != pCurPam->GetMark()->nNode.GetIndex() ) &&
         pDoc->GetNodes()[ nStart ]->IsTableNode() )
    {
        pCurPam->GetMark()->nNode = nStart;
    }

    pOrigPam = pCurPam;
    pCurPam->Exchange();

    WriteText();

    bOutPageDescs = bOldPageDescs;
    delete pCurPam;                    // Pam wieder loeschen
    pCurPam = pOldPam;
    pOrigPam = pOldEnd;
    nTxtTyp = nOldTyp;
}

void WW8Export::OutSwString(const String& rStr, xub_StrLen nStt,
    xub_StrLen nLen, bool bUnicode, rtl_TextEncoding eChrSet)

{
#ifdef DEBUG
    ::std::clog << "<OutSwString>" << ::std::endl;
#endif

    if( nLen )
    {
        if ( bUnicode != pPiece->IsUnicode() )
            pPiece->AppendPc ( Strm().Tell(), bUnicode );

        if( nStt || nLen != rStr.Len() )
        {
            String sOut( rStr.Copy( nStt, nLen ) );

#ifdef DEBUG
            ::std::clog << ::rtl::OUStringToOString(sOut, RTL_TEXTENCODING_ASCII_US).getStr() << ::std::endl;
#endif

            if (bUnicode)
                SwWW8Writer::WriteString16(Strm(), sOut, false);
            else
                SwWW8Writer::WriteString8(Strm(), sOut, false, eChrSet);
        }
        else
        {
#ifdef DEBUG
            ::std::clog << ::rtl::OUStringToOString(rStr, RTL_TEXTENCODING_ASCII_US).getStr() << ::std::endl;
#endif

            if (bUnicode)
                SwWW8Writer::WriteString16(Strm(), rStr, false);
            else
                SwWW8Writer::WriteString8(Strm(), rStr, false, eChrSet);
        }
    }

#ifdef DEBUG
    ::std::clog << "</OutSwString>" << ::std::endl;
#endif
}

void WW8Export::WriteCR(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    if (pTableTextNodeInfoInner.get() != NULL && pTableTextNodeInfoInner->getDepth() == 1 && pTableTextNodeInfoInner->isEndOfCell())
        WriteChar('\007');
    else
        WriteChar( '\015' );

    pPiece->SetParaBreak();
}

void WW8Export::WriteChar( sal_Unicode c )
{
    if( pPiece->IsUnicode() )
        Strm() << c;
    else
        Strm() << (BYTE)c;
}

void MSWordExportBase::SaveData( ULONG nStt, ULONG nEnd )
{
    MSWordSaveData aData;

    // WW8Export only stuff - zeroed here not to issue warnings
    aData.pOOld = NULL;
    aData.mpTableAtOld = NULL;
    aData.mnTableStdAtLenOld = 0;

    // Common stuff
    aData.pOldPam = pCurPam;
    aData.pOldEnd = pOrigPam;
    aData.pOldFlyFmt = mpParentFrame;
    aData.pOldPageDesc = pAktPageDesc;

    aData.pOldFlyOffset = pFlyOffset;
    aData.eOldAnchorType = eNewAnchorType;

    aData.bOldOutTable = bOutTable;
    aData.bOldIsInTable = bIsInTable;
    aData.bOldFlyFrmAttrs = bOutFlyFrmAttrs;
    aData.bOldStartTOX = bStartTOX;
    aData.bOldInWriteTOX = bInWriteTOX;

    pCurPam = Writer::NewSwPaM( *pDoc, nStt, nEnd );

    // Recognize tables in special cases
    if ( nStt != pCurPam->GetMark()->nNode.GetIndex() &&
         pDoc->GetNodes()[ nStt ]->IsTableNode() )
    {
        pCurPam->GetMark()->nNode = nStt;
    }

    pOrigPam = pCurPam;
    pCurPam->Exchange();

    bOutTable = false;
    // Caution: bIsInTable should not be set here
    bOutFlyFrmAttrs = false;
    bStartTOX = false;
    bInWriteTOX = false;

    maSaveData.push( aData );
}

void MSWordExportBase::RestoreData()
{
    MSWordSaveData &rData = maSaveData.top();

    delete pCurPam;
    pCurPam = rData.pOldPam;
    pOrigPam = rData.pOldEnd;

    bOutTable = rData.bOldOutTable;
    bIsInTable = rData.bOldIsInTable;
    bOutFlyFrmAttrs = rData.bOldFlyFrmAttrs;
    bStartTOX = rData.bOldStartTOX;
    bInWriteTOX = rData.bOldInWriteTOX;

    mpParentFrame = rData.pOldFlyFmt;
    pAktPageDesc = rData.pOldPageDesc;

    eNewAnchorType = rData.eOldAnchorType;
    pFlyOffset = rData.pOldFlyOffset;

    maSaveData.pop();
}

void WW8Export::SaveData( ULONG nStt, ULONG nEnd )
{
    MSWordExportBase::SaveData( nStt, nEnd );

    MSWordSaveData &rData = maSaveData.top();

    if ( pO->Count() )
    {
        rData.pOOld = pO;
        pO = new WW8Bytes( 128, 128 );
    }
    else
        rData.pOOld = 0; // reuse pO

    rData.mpTableAtOld = mpTableAt;
    mpTableAt = NULL;
    rData.mnTableStdAtLenOld = mnTableStdAtLen;
    mnTableStdAtLen = 0;

    rData.bOldWriteAll = GetWriter().bWriteAll;
    GetWriter().bWriteAll = true;
}

void WW8Export::RestoreData()
{
    MSWordSaveData &rData = maSaveData.top();

    GetWriter().bWriteAll = rData.bOldWriteAll;

    OSL_ENSURE( !pO->Count(), "pO is not empty in WW8Export::RestoreData()" );
    if ( rData.pOOld )
    {
        delete pO;
        pO = rData.pOOld;
    }

    OSL_ENSURE( !mpTableAt || !mpTableAt->Count(), "mpTableAt is not empty in WW8Export::RestoreData()" );
    if ( mpTableAt )
        delete mpTableAt;
    mpTableAt = rData.mpTableAtOld;
    mnTableStdAtLen = rData.mnTableStdAtLenOld;

    MSWordExportBase::RestoreData();
}

void WW8AttributeOutput::TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    sal_uInt32 nDepth = pTableTextNodeInfoInner->getDepth();

    if ( nDepth > 0 )
    {
        /* Cell */
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFInTable );
        m_rWW8Export.pO->Insert( (BYTE)0x1, m_rWW8Export.pO->Count() );
        m_rWW8Export.InsUInt16( NS_sprm::LN_PTableDepth );
        m_rWW8Export.InsUInt32( nDepth );

        if ( nDepth > 1 && pTableTextNodeInfoInner->isEndOfCell() )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_PCell );
            m_rWW8Export.pO->Insert( (BYTE)0x1, m_rWW8Export.pO->Count() );
        }
    }
}

void WW8AttributeOutput::TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    sal_uInt32 nDepth = pTableTextNodeInfoInner->getDepth();

    if ( nDepth > 0 )
    {
        /* Row */
        if ( pTableTextNodeInfoInner->isEndOfLine() )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_PFInTable );
            m_rWW8Export.pO->Insert( (BYTE)0x1, m_rWW8Export.pO->Count() );

            if ( nDepth == 1 )
            {
                m_rWW8Export.InsUInt16( NS_sprm::LN_PFTtp );
                m_rWW8Export.pO->Insert( (BYTE)0x1, m_rWW8Export.pO->Count() );
            }

            m_rWW8Export.InsUInt16( NS_sprm::LN_PTableDepth );
            m_rWW8Export.InsUInt32( nDepth );

            if ( nDepth > 1 )
            {
                m_rWW8Export.InsUInt16( NS_sprm::LN_PCell );
                m_rWW8Export.pO->Insert( (BYTE)0x1, m_rWW8Export.pO->Count() );
                m_rWW8Export.InsUInt16( NS_sprm::LN_PRow );
                m_rWW8Export.pO->Insert( (BYTE)0x1, m_rWW8Export.pO->Count() );
            }

            TableDefinition( pTableTextNodeInfoInner );
            TableHeight( pTableTextNodeInfoInner );
            TableBackgrounds( pTableTextNodeInfoInner );
            TableDefaultBorders( pTableTextNodeInfoInner );
            TableCanSplit( pTableTextNodeInfoInner );
            TableBidi( pTableTextNodeInfoInner );
            TableVerticalCell( pTableTextNodeInfoInner );
            TableOrientation( pTableTextNodeInfoInner );
            TableSpacing( pTableTextNodeInfoInner );
        }
    }
}

static sal_uInt16 lcl_TCFlags(const SwTableBox * pBox, long nRowSpan)
{
    sal_uInt16 nFlags = 0;

    if (nRowSpan > 1)
        nFlags |= (3 << 5);
    else if (nRowSpan < 0)
        nFlags |= (1 << 5);

    if (pBox != NULL)
    {
        const SwFrmFmt * pFmt = pBox->GetFrmFmt();
        switch (pFmt->GetVertOrient().GetVertOrient())
        {
            case text::VertOrientation::CENTER:
                nFlags |= (1 << 7);
                break;
            case text::VertOrientation::BOTTOM:
                nFlags |= (2 << 7);
                break;
            default:
                break;
        }
    }

    return nFlags;
}

void WW8AttributeOutput::TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwTableBoxes & rTblBoxes = pTabLine->GetTabBoxes();

    sal_uInt8 nBoxes = rTblBoxes.Count();
    for ( sal_uInt8 n = 0; n < nBoxes; n++ )
    {
        const SwTableBox * pTabBox1 = rTblBoxes[n];
        const SwFrmFmt * pFrmFmt = pTabBox1->GetFrmFmt();

        if ( FRMDIR_VERT_TOP_RIGHT == m_rWW8Export.TrueFrameDirection( *pFrmFmt ) )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_TTextFlow );
            m_rWW8Export.pO->Insert( BYTE(n), m_rWW8Export.pO->Count() );        //start range
            m_rWW8Export.pO->Insert( BYTE(n + 1), m_rWW8Export.pO->Count() );    //end range
            m_rWW8Export.InsUInt16( 5 ); //Equals vertical writing
        }
    }
}

void WW8AttributeOutput::TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwFrmFmt * pLineFmt = pTabLine->GetFrmFmt();

    /*
     By default the row can be split in word, and now in writer we have a
     feature equivalent to this, Word stores 1 for fCantSplit if the row
     cannot be split, we set true if we can split it. An example is #i4569#
     */

    const SwFmtRowSplit& rSplittable = pLineFmt->GetRowSplit();
    BYTE nCantSplit = (!rSplittable.GetValue()) ? 1 : 0;
    if ( m_rWW8Export.bWrtWW8 )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_TFCantSplit );
        m_rWW8Export.pO->Insert( nCantSplit, m_rWW8Export.pO->Count() );
        m_rWW8Export.InsUInt16( NS_sprm::LN_TFCantSplit90 ); // also write fCantSplit90
    }
    else
    {
        m_rWW8Export.pO->Insert( 185, m_rWW8Export.pO->Count() );
    }
    m_rWW8Export.pO->Insert( nCantSplit, m_rWW8Export.pO->Count() );
}

void WW8AttributeOutput::TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();
    const SwFrmFmt * pFrmFmt = pTable->GetFrmFmt();

    if ( m_rWW8Export.bWrtWW8 )
    {
        if ( m_rWW8Export.TrueFrameDirection(*pFrmFmt) == FRMDIR_HORI_RIGHT_TOP )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_TFBiDi );
            m_rWW8Export.InsUInt16( 1 );
        }
    }
}

void WW8AttributeOutput::TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwFrmFmt * pLineFmt = pTabLine->GetFrmFmt();

    // Zeilenhoehe ausgeben   sprmTDyaRowHeight
    long nHeight = 0;
    const SwFmtFrmSize& rLSz = pLineFmt->GetFrmSize();
    if ( ATT_VAR_SIZE != rLSz.GetHeightSizeType() && rLSz.GetHeight() )
    {
        if ( ATT_MIN_SIZE == rLSz.GetHeightSizeType() )
            nHeight = rLSz.GetHeight();
        else
            nHeight = -rLSz.GetHeight();
    }

    if ( nHeight )
    {
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_TDyaRowHeight );
        else
            m_rWW8Export.pO->Insert( 189, m_rWW8Export.pO->Count() );
        m_rWW8Export.InsUInt16( (USHORT)nHeight );
    }

}

void WW8AttributeOutput::TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();

    const SwFrmFmt *pFmt = pTable->GetFrmFmt();
    OSL_ENSURE(pFmt,"Impossible");
    if (!pFmt)
        return;

    const SwFmtHoriOrient &rHori = pFmt->GetHoriOrient();
    const SwFmtVertOrient &rVert = pFmt->GetVertOrient();

    if (
        (text::RelOrientation::PRINT_AREA == rHori.GetRelationOrient() ||
         text::RelOrientation::FRAME == rHori.GetRelationOrient())
        &&
        (text::RelOrientation::PRINT_AREA == rVert.GetRelationOrient() ||
         text::RelOrientation::FRAME == rVert.GetRelationOrient())
        )
    {
        sal_Int16 eHOri = rHori.GetHoriOrient();
        switch (eHOri)
        {
            case text::HoriOrientation::CENTER:
            case text::HoriOrientation::RIGHT:
                if ( m_rWW8Export.bWrtWW8 )
                    m_rWW8Export.InsUInt16( NS_sprm::LN_TJc );
                else
                    m_rWW8Export.pO->Insert( 182, m_rWW8Export.pO->Count() );
                m_rWW8Export.InsUInt16( text::HoriOrientation::RIGHT == eHOri ? 2 : 1 );
                break;
            default:
                break;
        }
    }
}

void WW8AttributeOutput::TableSpacing(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();
    const SwTableFmt * pTableFmt = dynamic_cast<const SwTableFmt *>(pTable->GetRegisteredIn());

    if (pTableFmt != NULL)
    {
        const SvxULSpaceItem & rUL = pTableFmt->GetULSpace();

        if (rUL.GetUpper() > 0)
        {
            sal_uInt8 nPadding = 2;
            sal_uInt8 nPcVert = 0;
            sal_uInt8 nPcHorz = 0;

            sal_uInt8 nTPc = (nPadding << 4) | (nPcVert << 2) | nPcHorz;

            m_rWW8Export.InsUInt16(NS_sprm::LN_TPc);
            m_rWW8Export.pO->Insert( nTPc, m_rWW8Export.pO->Count() );

            m_rWW8Export.InsUInt16(NS_sprm::LN_TDyaAbs);
            m_rWW8Export.InsUInt16(rUL.GetUpper());

            m_rWW8Export.InsUInt16(NS_sprm::LN_TDyaFromText);
            m_rWW8Export.InsUInt16(rUL.GetUpper());
        }

        if (rUL.GetLower() > 0)
        {
            m_rWW8Export.InsUInt16(NS_sprm::LN_TDyaFromTextBottom);
            m_rWW8Export.InsUInt16(rUL.GetLower());
        }
    }
}

void WW8AttributeOutput::TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();

    if ( pTable->GetRowsToRepeat() > pTableTextNodeInfoInner->getRow() )
    {
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_TTableHeader );
        else
            m_rWW8Export.pO->Insert( 186, m_rWW8Export.pO->Count() );
        m_rWW8Export.pO->Insert( 1, m_rWW8Export.pO->Count() );
    }

    ww8::TableBoxVectorPtr pTableBoxes =
        pTableTextNodeInfoInner->getTableBoxesOfRow();
    // number of cell written
    sal_uInt32 nBoxes = pTableBoxes->size();

    // sprm header
    m_rWW8Export.InsUInt16( NS_sprm::LN_TDefTable );
    sal_uInt16 nSprmSize = 2 + (nBoxes + 1) * 2 + nBoxes * 20;
    m_rWW8Export.InsUInt16( nSprmSize ); // length

    // number of boxes
    m_rWW8Export.pO->Insert( static_cast<BYTE>(nBoxes), m_rWW8Export.pO->Count() );

    /* cellxs */
    /*
     ALWAYS relative when text::HoriOrientation::NONE (nPageSize + ( nPageSize / 10 )) < nTblSz,
     in that case the cell width's and table width's are not real. The table
     width is maxed and cells relative, so we need the frame (generally page)
     width that the table is in to work out the true widths.
     */
    //const bool bNewTableModel = pTbl->IsNewModel();
    const SwFrmFmt *pFmt = pTable->GetFrmFmt();
    OSL_ENSURE(pFmt,"Impossible");
    if (!pFmt)
        return;

    const SwFmtHoriOrient &rHori = pFmt->GetHoriOrient();
    const SwFmtVertOrient &rVert = pFmt->GetVertOrient();

    sal_uInt16 nTblOffset = 0;

    if (
        (text::RelOrientation::PRINT_AREA == rHori.GetRelationOrient() ||
         text::RelOrientation::FRAME == rHori.GetRelationOrient())
        &&
        (text::RelOrientation::PRINT_AREA == rVert.GetRelationOrient() ||
         text::RelOrientation::FRAME == rVert.GetRelationOrient())
        )
    {
        sal_Int16 eHOri = rHori.GetHoriOrient();
        switch ( eHOri )
        {
            case text::HoriOrientation::CENTER:
            case text::HoriOrientation::RIGHT:
                break;

            default:
                nTblOffset = rHori.GetPos();
                const SvxLRSpaceItem& rLRSp = pFmt->GetLRSpace();
                nTblOffset += rLRSp.GetLeft();
                break;
        }
    }

     m_rWW8Export.InsUInt16( nTblOffset );

    ww8::GridColsPtr pGridCols = GetGridCols( pTableTextNodeInfoInner );
    for ( ww8::GridCols::const_iterator it = pGridCols->begin(),
              end = pGridCols->end(); it != end; ++it )
     {
         m_rWW8Export.InsUInt16( static_cast<USHORT>( *it ) + nTblOffset );
     }

     /* TCs */
    ww8::RowSpansPtr pRowSpans = pTableTextNodeInfoInner->getRowSpansOfRow();
    ww8::RowSpans::const_iterator aItRowSpans = pRowSpans->begin();
    ww8::TableBoxVector::const_iterator aIt;
    ww8::TableBoxVector::const_iterator aItEnd = pTableBoxes->end();

#ifdef DEBUG
    size_t nRowSpans = pRowSpans->size();
    size_t nTableBoxes = pTableBoxes->size();
    (void) nRowSpans;
    (void) nTableBoxes;
#endif

    for( aIt = pTableBoxes->begin(); aIt != aItEnd; ++aIt, ++aItRowSpans)
    {
#ifdef DEBUG
        sal_uInt16 npOCount = m_rWW8Export.pO->Count();
#endif

        const SwTableBox * pTabBox1 = *aIt;
        const SwFrmFmt * pBoxFmt = NULL;
        if (pTabBox1 != NULL)
            pBoxFmt = pTabBox1->GetFrmFmt();

        if ( m_rWW8Export.bWrtWW8 )
        {
            sal_uInt16 nFlags =
                lcl_TCFlags(pTabBox1, *aItRowSpans);
             m_rWW8Export.InsUInt16( nFlags );
        }

        static BYTE aNullBytes[] = { 0x0, 0x0 };

        m_rWW8Export.pO->Insert( aNullBytes, 2, m_rWW8Export.pO->Count() );   // dummy
        if (pBoxFmt != NULL)
        {
            const SvxBoxItem & rBoxItem = pBoxFmt->GetBox();

            m_rWW8Export.Out_SwFmtTableBox( *m_rWW8Export.pO, &rBoxItem ); // 8/16 Byte
        }
        else
            m_rWW8Export.Out_SwFmtTableBox( *m_rWW8Export.pO, NULL); // 8/16 Byte

#ifdef DEBUG
        ::std::clog << "<tclength>" << m_rWW8Export.pO->Count() - npOCount << "</tclength>"
                    << ::std::endl;
#endif
    }
}

ww8::GridColsPtr AttributeOutputBase::GetGridCols( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    return pTableTextNodeInfoInner->getGridColsOfRow(*this);
}

void AttributeOutputBase::GetTablePageSize( ww8::WW8TableNodeInfoInner * pTableTextNodeInfoInner, sal_uInt32& rPageSize, bool& rRelBoxSize )
{
    sal_uInt32 nPageSize = 0;

    const SwNode *pTxtNd = pTableTextNodeInfoInner->getNode( );
    const SwTable *pTable = pTableTextNodeInfoInner->getTable( );

    const SwFrmFmt *pFmt = pTable->GetFrmFmt();
    OSL_ENSURE(pFmt,"Impossible");
    if (!pFmt)
        return;

    const SwFmtFrmSize &rSize = pFmt->GetFrmSize();
    int nWidthPercent = rSize.GetWidthPercent();
    bool bManualAligned = pFmt->GetHoriOrient().GetHoriOrient() == text::HoriOrientation::NONE;
    if ( (pFmt->GetHoriOrient().GetHoriOrient() == text::HoriOrientation::FULL) || bManualAligned )
        nWidthPercent = 100;
    bool bRelBoxSize = nWidthPercent != 0;
    unsigned long nTblSz = static_cast<unsigned long>(rSize.GetWidth());
    if (nTblSz > USHRT_MAX/2 && !bRelBoxSize)
    {
        OSL_ENSURE(bRelBoxSize, "huge table width but not relative, suspicious");
        bRelBoxSize = true;
    }

    if ( bRelBoxSize )
    {
        Point aPt;
        SwRect aRect( pFmt->FindLayoutRect( false, &aPt ) );
        if ( aRect.IsEmpty() )
        {
            // dann besorge mal die Seitenbreite ohne Raender !!
            const SwFrmFmt* pParentFmt =
                GetExport().mpParentFrame ?
                &(GetExport().mpParentFrame->GetFrmFmt()) :
                    const_cast<const SwDoc *>(GetExport().pDoc)->GetPageDesc(0).GetPageFmtOfNode(*pTxtNd, false);
            aRect = pParentFmt->FindLayoutRect(true);
            if ( 0 == ( nPageSize = aRect.Width() ) )
            {
                const SvxLRSpaceItem& rLR = pParentFmt->GetLRSpace();
                nPageSize = pParentFmt->GetFrmSize().GetWidth() - rLR.GetLeft()
                - rLR.GetRight();
            }
        }
        else
        {
            nPageSize = aRect.Width();
            if ( bManualAligned )
            {
                // #i37571# For manually aligned tables
                const SvxLRSpaceItem &rLR = pFmt->GetLRSpace();
                nPageSize -= (rLR.GetLeft() + rLR.GetRight());
            }

        }

        OSL_ENSURE(nWidthPercent, "Impossible");
        if (nWidthPercent)
        {
            nPageSize *= nWidthPercent;
            nPageSize /= 100;
        }
    }

    rPageSize = nPageSize;
    rRelBoxSize = bRelBoxSize;
}

void WW8AttributeOutput::TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrmFmt * pFrmFmt = pTabBox->GetFrmFmt();

    //Set Default, just taken from the first cell of the first
    //row
    static USHORT aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT,
        BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };

    for ( int i = 0; i < 4; ++i )
    {
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, 0xD634 );
        m_rWW8Export.pO->Insert( BYTE(6), m_rWW8Export.pO->Count() );
        m_rWW8Export.pO->Insert( BYTE(0), m_rWW8Export.pO->Count() );
        m_rWW8Export.pO->Insert( BYTE(1), m_rWW8Export.pO->Count() );
        m_rWW8Export.pO->Insert( BYTE(1 << i), m_rWW8Export.pO->Count() );
        m_rWW8Export.pO->Insert( BYTE(3), m_rWW8Export.pO->Count() );

        SwWW8Writer::InsUInt16( *m_rWW8Export.pO,
                pFrmFmt->GetBox().GetDistance( aBorders[i] ) );
    }
}

void WW8AttributeOutput::TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwTableBoxes & rTabBoxes = pTabLine->GetTabBoxes();

    sal_uInt8 nBoxes = rTabBoxes.Count();
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_TDefTableShd );
    else
        m_rWW8Export.pO->Insert( (BYTE)191, m_rWW8Export.pO->Count() );
    m_rWW8Export.pO->Insert( (BYTE)(nBoxes * 2), m_rWW8Export.pO->Count() );  // Len

    for ( sal_uInt8 n = 0; n < nBoxes; n++ )
    {
        const SwTableBox * pBox1 = rTabBoxes[n];
        const SwFrmFmt * pFrmFmt = pBox1->GetFrmFmt();
        const SfxPoolItem * pI = NULL;
        Color aColor;

        if ( SFX_ITEM_ON == pFrmFmt->GetAttrSet().GetItemState( RES_BACKGROUND, false, &pI ) )
        {
            aColor = dynamic_cast<const SvxBrushItem *>(pI)->GetColor();
        }
        else
            aColor = COL_AUTO;

        WW8_SHD aShd;
        m_rWW8Export.TransBrush( aColor, aShd );
        m_rWW8Export.InsUInt16( aShd.GetValue() );
    }

    if ( m_rWW8Export.bWrtWW8 )
    {
        sal_uInt32 aSprmIds[] = {NS_sprm::LN_TCellShd, NS_sprm::LN_TCellShadow};
        sal_uInt8 nBoxes0 = rTabBoxes.Count();
        if (nBoxes0 > 21)
            nBoxes0 = 21;

        for (sal_uInt32 m = 0; m < 2; m++)
        {
            m_rWW8Export.InsUInt16( aSprmIds[m] );
            m_rWW8Export.pO->Insert( static_cast<BYTE>(nBoxes0 * 10),
                                     m_rWW8Export.pO->Count() );

        for ( sal_uInt8 n = 0; n < nBoxes0; n++ )
        {
            const SwTableBox * pBox1 = rTabBoxes[n];
            const SwFrmFmt * pFrmFmt = pBox1->GetFrmFmt();
            const SfxPoolItem * pI = NULL;
            Color aColor;

                if ( SFX_ITEM_ON ==
                         pFrmFmt->GetAttrSet().
                         GetItemState( RES_BACKGROUND, false, &pI ) )
            {
                aColor = dynamic_cast<const SvxBrushItem *>(pI)->GetColor();
            }
            else
                aColor = COL_AUTO;

            WW8SHDLong aSHD;
            aSHD.setCvFore( 0xFF000000 );

            sal_uInt32 nBgColor = aColor.GetColor();
            if ( nBgColor == COL_AUTO )
                aSHD.setCvBack( 0xFF000000 );
            else
                aSHD.setCvBack( wwUtility::RGBToBGR( nBgColor ) );

            aSHD.Write( m_rWW8Export );
        }
        }
    }
}

void WW8Export::SectionBreaksAndFrames( const SwTxtNode& rNode )
{
    // output page/section breaks
    OutputSectionBreaks( rNode.GetpSwAttrSet(), rNode );

    // all textframes anchored as character for the winword 7- format
    if ( !bWrtWW8 && !bIsInTable )
        OutWW6FlyFrmsInCntnt( rNode );
}

#ifdef DEBUG
struct SwNodeHash
{
    size_t operator()(SwNode * pNode) const { return reinterpret_cast<size_t>(pNode); }
};

typedef ::boost::unordered_set<SwNode *, SwNodeHash> SwNodeHashSet;
typedef ::std::deque<SwNode *> SwNodeDeque;
#endif

void MSWordExportBase::WriteText()
{
    while( pCurPam->GetPoint()->nNode < pCurPam->GetMark()->nNode ||
           ( pCurPam->GetPoint()->nNode == pCurPam->GetMark()->nNode &&
             pCurPam->GetPoint()->nContent.GetIndex() <= pCurPam->GetMark()->nContent.GetIndex() ) )
    {
        SwNode * pNd = pCurPam->GetNode();

        if ( pNd->IsTxtNode() )
            SectionBreaksAndFrames( *pNd->GetTxtNode() );

        // output the various types of nodes
        if ( pNd->IsCntntNode() )
        {
            SwCntntNode* pCNd = (SwCntntNode*)pNd;

            const SwPageDesc* pTemp = pCNd->GetSwAttrSet().GetPageDesc().GetPageDesc();
            if ( pTemp )
                pAktPageDesc = pTemp;

            pCurPam->GetPoint()->nContent.Assign( pCNd, 0 );
            OutputContentNode( *pCNd );
        }
        else if ( pNd->IsTableNode() )
        {
            mpTableInfo->processSwTable( &pNd->GetTableNode()->GetTable() );
        }
        else if ( pNd->IsSectionNode() && TXT_MAINTEXT == nTxtTyp )
            OutputSectionNode( *pNd->GetSectionNode() );
        else if ( TXT_MAINTEXT == nTxtTyp && pNd->IsEndNode() &&
                  pNd->StartOfSectionNode()->IsSectionNode() )
        {
            const SwSection& rSect = pNd->StartOfSectionNode()->GetSectionNode()
                                        ->GetSection();
            if ( bStartTOX && TOX_CONTENT_SECTION == rSect.GetType() )
                bStartTOX = false;

            SwNodeIndex aIdx( *pNd, 1 );
            if ( aIdx.GetNode().IsEndNode() && aIdx.GetNode().StartOfSectionNode()->IsSectionNode() )
                ;
            else if ( aIdx.GetNode().IsSectionNode() )
                ;
            else if ( !bIsInTable ) //No sections in table
            {
                ReplaceCr( (char)0xc ); // Indikator fuer Page/Section-Break

                const SwSectionFmt* pParentFmt = rSect.GetFmt()->GetParent();
                if ( !pParentFmt )
                    pParentFmt = (SwSectionFmt*)0xFFFFFFFF;

                ULONG nRstLnNum;
                if ( aIdx.GetNode().IsCntntNode() )
                    nRstLnNum = ((SwCntntNode&)aIdx.GetNode()).GetSwAttrSet().
                                            GetLineNumber().GetStartValue();
                else
                    nRstLnNum = 0;

                AppendSection( pAktPageDesc, pParentFmt, nRstLnNum );
            }
        }
        else if ( pNd->IsStartNode() )
        {
            OutputStartNode( *pNd->GetStartNode() );
        }
        else if ( pNd->IsEndNode() )
        {
            OutputEndNode( *pNd->GetEndNode() );
        }

        if ( pNd == &pNd->GetNodes().GetEndOfContent() )
            break;

        SwNode * pCurrentNode = &pCurPam->GetPoint()->nNode.GetNode();
        const SwNode * pNextNode = mpTableInfo->getNextNode(pCurrentNode);

        if (pNextNode != NULL)
            pCurPam->GetPoint()->nNode = SwNodeIndex(*pNextNode);
        else
            pCurPam->GetPoint()->nNode++;

        ULONG nPos = pCurPam->GetPoint()->nNode.GetIndex();
        ::SetProgressState( nPos, pCurPam->GetDoc()->GetDocShell() );
    }

#ifdef DEBUG
    ::std::clog << "</WriteText>" << ::std::endl;
#endif
}

void WW8Export::WriteMainText()
{
#ifdef DEBUG
    ::std::clog << "<WriteMainText>" << ::std::endl;
#endif

    pFib->fcMin = Strm().Tell();

    pCurPam->GetPoint()->nNode = pDoc->GetNodes().GetEndOfContent().StartOfSectionNode()->GetIndex();

    WriteText();

    if( 0 == Strm().Tell() - pFib->fcMin )  // kein Text ?
        WriteCR();                  // dann CR ans Ende ( sonst mault WW )

    pFib->ccpText = Fc2Cp( Strm().Tell() );
    pFldMain->Finish( pFib->ccpText, 0 );

                    // ccpText beinhaltet Ftn- und KF-Texte
                    // deshalb wird pFib->ccpText evtl. noch geupdated
    // save the StyleId of the last paragraph. Because WW97 take the style
    // from the last CR, that will be write after footer/Header/fontnotes/
    // annotation usw.
    const SwTxtNode* pLastNd = pCurPam->GetMark()->nNode.GetNode().GetTxtNode();
    if( pLastNd )
        nLastFmtId = GetId( (SwTxtFmtColl&)pLastNd->GetAnyFmtColl() );

#ifdef DEBUG
    ::std::clog << "</WriteMainText>" << ::std::endl;
#endif
}

typedef ww8::WW8Sttb< ww8::WW8Struct >  WW8SttbAssoc;

void WW8Export::WriteFkpPlcUsw()
{
    if( !bWrtWW8 )
    {
        static const BYTE aSpec[2] =
        {
            117, 1
        };

        pChpPlc->AppendFkpEntry( Strm().Tell() );   // Sepx mit fSpecial
        pSepx->WriteSepx( Strm() );                 // Slcx.Sepx
        pGrf->Write();                              // Grafiken
        pChpPlc->AppendFkpEntry( Strm().Tell(), sizeof( aSpec ), aSpec );

        pChpPlc->WriteFkps();                   // Fkp.Chpx
        pPapPlc->WriteFkps();                   // Fkp.Papx
        pStyles->OutputStylesTable();           // Styles
        pFtn->WritePlc( *this );                // Footnote-Ref & Text Plc
        pEdn->WritePlc( *this );                // Endnote-Ref & Text Plc
        pAtn->WritePlc( *this );                // Annotation-Ref & Text Plc
        pSepx->WritePlcSed( *this );            // Slcx.PlcSed
        pSepx->WritePlcHdd( *this );            // Slcx.PlcHdd
        pChpPlc->WritePlc();                    // Plcx.Chpx
        pPapPlc->WritePlc();                    // Plcx.Papx
        maFontHelper.WriteFontTable(pTableStrm, *pFib); // FFNs
        if( pRedlAuthors )
            pRedlAuthors->Write( GetWriter() );       // sttbfRMark (RedlineAuthors)
        pFldMain->Write( *this );               // Fields ( Main Text )
        pFldHdFt->Write( *this );               // Fields ( Header/Footer )
        pFldFtn->Write( *this );                // Fields ( FootNotes )
        pFldEdn->Write( *this );                // Fields ( EndNotes )
        pFldAtn->Write( *this );                // Fields ( Annotations )
        pBkmks->Write( *this );                 // Bookmarks - sttbfBkmk/
                                                // plcfBkmkf/plcfBkmkl
        WriteDop( *this );                      // Document-Properties

    }
    else
    {
        // Grafiken in den Data-Tream
        pGrf->Write();                      // Grafiken

        // Ausgabe in WordDocument-Stream
        pChpPlc->WriteFkps();                   // Fkp.Chpx
        pPapPlc->WriteFkps();                   // Fkp.Papx
        pSepx->WriteSepx( Strm() );             // Sepx

        // Ausagbe in Table-Stream
        pStyles->OutputStylesTable();           // fuer WW8 StyleTab
        pFtn->WritePlc( *this );                // Footnote-Ref & Text Plc
        pEdn->WritePlc( *this );                // Endnote-Ref & Text Plc
        pTxtBxs->WritePlc( *this );             // Textbox Text Plc
        pHFTxtBxs->WritePlc( *this );           // Head/Foot-Textbox Text Plc
        pAtn->WritePlc( *this );                // Annotation-Ref & Text Plc

        pSepx->WritePlcSed( *this );            // Slcx.PlcSed
        pSepx->WritePlcHdd( *this );            // Slcx.PlcHdd

        pChpPlc->WritePlc();                    // Plcx.Chpx
        pPapPlc->WritePlc();                    // Plcx.Papx

        if( pRedlAuthors )
            pRedlAuthors->Write( GetWriter() );       // sttbfRMark (RedlineAuthors)
        pFldMain->Write( *this );               // Fields ( Main Text )
        pFldHdFt->Write( *this );               // Fields ( Header/Footer )
        pFldFtn->Write( *this );                // Fields ( FootNotes )
        pFldEdn->Write( *this );                // Fields ( EndNotes )
        pFldAtn->Write( *this );                // Fields ( Annotations )
        pFldTxtBxs->Write( *this );             // Fields ( Textboxes )
        pFldHFTxtBxs->Write( *this );           // Fields ( Head/Foot-Textboxes )

        if (pEscher || pDoc->ContainsMSVBasic())
        {
            /*
             Everytime MS 2000 creates an escher stream there is always
             an ObjectPool dir (even if empty). It turns out that if a copy of
             MS 2000 is used to open a document that contains escher graphics
             exported from StarOffice without this empty dir then *if* that
             copy of MS Office has never been used to open a MSOffice document
             that has escher graphics (and an ObjectPool dir of course) and
             that copy of office has not been used to draw escher graphics then
             our exported graphics do not appear. Once you do open a ms
             document with escher graphics or draw an escher graphic with that
             copy of word, then all documents from staroffice that contain
             escher work from then on. Tricky to track down, some sort of late
             binding trickery in MS where solely for first time initialization
             the existence of an ObjectPool dir is necessary for triggering
             some magic. cmc
            */
            /* Similiarly having msvbasic storage seems to also trigger creating this stream */
            GetWriter().GetStorage().OpenSotStorage(CREATE_CONST_ASC(SL::aObjectPool),
                STREAM_READWRITE | STREAM_SHARE_DENYALL);
        }

        // dggInfo - escher stream
        WriteEscher();

        pSdrObjs->WritePlc( *this );
        pHFSdrObjs->WritePlc( *this );
        // spamom - office drawing table
        // spahdr - header office drawing table

        pBkmks->Write( *this );                 // Bookmarks - sttbfBkmk/
                                                // plcfBkmkf/plcfBkmkl

        WriteNumbering();

        RestoreMacroCmds();

        pMagicTable->Write( *this );

        pPiece->WritePc( *this );               // Piece-Table
        maFontHelper.WriteFontTable(pTableStrm, *pFib); // FFNs

        //Convert OOo asian typography into MS typography structure
        ExportDopTypography(pDop->doptypography);

        WriteDop( *this );                      // Document-Properties

        // Write SttbfAssoc
        WW8SttbAssoc * pSttbfAssoc = dynamic_cast<WW8SttbAssoc *>
            (pDoc->getExternalData(::sw::STTBF_ASSOC).get());

        if ( pSttbfAssoc )                      // #i106057#
        // <--
        {
        ::std::vector<String> aStrings;

        ::ww8::StringVector_t & aSttbStrings = pSttbfAssoc->getStrings();
        ::ww8::StringVector_t::const_iterator aItEnd = aSttbStrings.end();
        for (::ww8::StringVector_t::const_iterator aIt = aSttbStrings.begin();
             aIt != aItEnd; aIt++)
        {
            String aStr(aIt->getStr());
            aStrings.push_back(aStr);
        }

        WriteAsStringTable(aStrings, pFib->fcSttbfAssoc,
                           pFib->lcbSttbfAssoc);
        }
    }
    Strm().Seek( 0 );

    // Reclaim stored FIB data from document.
    ::ww8::WW8FibData * pFibData = dynamic_cast<ww8::WW8FibData *>
          (pDoc->getExternalData(::sw::FIB).get());

    if ( pFibData )
    // <--
    {
    pFib->fReadOnlyRecommended =
        pFibData->getReadOnlyRecommended() ? 1 : 0;
    pFib->fWriteReservation =
        pFibData->getWriteReservation() ? 1 : 0;
    }

    pFib->Write( Strm() );  // FIB
}

void WW8Export::StoreDoc1()
{
    bool bNeedsFinalPara = false;
    // Start of Text ( Mangel ueber )
    SwWW8Writer::FillUntil( Strm(), pFib->fcMin );

    WriteMainText();                    // HauptText
    BYTE nSprmsLen;
    BYTE *pLastSprms = pPapPlc->CopyLastSprms(nSprmsLen);

    bNeedsFinalPara |= pFtn->WriteTxt( *this );         // Footnote-Text
    bNeedsFinalPara |= pSepx->WriteKFTxt( *this );          // K/F-Text
    bNeedsFinalPara |= pAtn->WriteTxt( *this );         // Annotation-Text
    bNeedsFinalPara |= pEdn->WriteTxt( *this );         // EndNote-Text

    // create the escher streams
    if( bWrtWW8 )
        CreateEscher();

    bNeedsFinalPara |= pTxtBxs->WriteTxt( *this );  //Textbox Text Plc
    bNeedsFinalPara |= pHFTxtBxs->WriteTxt( *this );//Head/Foot-Textbox Text Plc

    if (bNeedsFinalPara)
    {
        WriteCR();
        pPapPlc->AppendFkpEntry(Strm().Tell(), nSprmsLen, pLastSprms);
    }
    delete[] pLastSprms;

    pSepx->Finish( Fc2Cp( Strm().Tell() ));// Text + Ftn + HdFt als Section-Ende
    pMagicTable->Finish( Fc2Cp( Strm().Tell() ),0);

    pFib->fcMac = Strm().Tell();        // Ende aller Texte

    WriteFkpPlcUsw();                   // FKP, PLC, .....
}

void MSWordExportBase::AddLinkTarget(const String& rURL)
{
    if( !rURL.Len() || rURL.GetChar(0) != INET_MARK_TOKEN )
        return;

    String aURL( BookmarkToWriter( rURL.Copy( 1 ) ) );
    xub_StrLen nPos = aURL.SearchBackward( cMarkSeperator );

    if( nPos < 2 )
        return;

    String sCmp( aURL.Copy( nPos+1 ) );
    sCmp.EraseAllChars();
    if( !sCmp.Len() )
        return;

    sCmp.ToLowerAscii();

    if( sCmp.EqualsAscii( pMarkToOutline ) )
    {
        SwPosition aPos( *pCurPam->GetPoint() );
        String aOutline( BookmarkToWriter(aURL.Copy( 0, nPos )) );
        // If we can find the outline this bookmark refers to
        // save the name of the bookmark and the
        // node index number of where it points to
        if( pDoc->GotoOutline( aPos, aOutline ) )
        {
            ULONG nIdx = aPos.nNode.GetIndex();
            aBookmarkPair aImplicitBookmark;
            aImplicitBookmark.first = aOutline;
            aImplicitBookmark.second = nIdx;
            maImplicitBookmarks.push_back(aImplicitBookmark);
        }
    }
}

void MSWordExportBase::CollectOutlineBookmarks(const SwDoc &rDoc)
{
    const SwFmtINetFmt* pINetFmt;
    const SwTxtINetFmt* pTxtAttr;
    const SwTxtNode* pTxtNd;

    USHORT n, nMaxItems = rDoc.GetAttrPool().GetItemCount( RES_TXTATR_INETFMT );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pINetFmt = (SwFmtINetFmt*)rDoc.GetAttrPool().GetItem(
            RES_TXTATR_INETFMT, n ) ) &&
            0 != ( pTxtAttr = pINetFmt->GetTxtINetFmt()) &&
            0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) &&
            pTxtNd->GetNodes().IsDocNodes() )
        {
            AddLinkTarget( pINetFmt->GetValue() );
        }
    }

    const SwFmtURL *pURL;
    nMaxItems = rDoc.GetAttrPool().GetItemCount( RES_URL );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pURL = (SwFmtURL*)rDoc.GetAttrPool().GetItem(
            RES_URL, n ) ) )
        {
            AddLinkTarget( pURL->GetURL() );
            const ImageMap *pIMap = pURL->GetMap();
            if( pIMap )
            {
                for( USHORT i=0; i<pIMap->GetIMapObjectCount(); i++ )
                {
                    const IMapObject* pObj = pIMap->GetIMapObject( i );
                    if( pObj )
                    {
                        AddLinkTarget( pObj->GetURL() );
                    }
                }
            }
        }
    }
}

namespace
{
    const ULONG WW_BLOCKSIZE = 0x200;

    void EncryptRC4(msfilter::MSCodec_Std97& rCtx, SvStream &rIn, SvStream &rOut)
    {
        rIn.Seek(STREAM_SEEK_TO_END);
        ULONG nLen = rIn.Tell();
        rIn.Seek(0);

        sal_uInt8 in[WW_BLOCKSIZE];
        for (ULONG nI = 0, nBlock = 0; nI < nLen; nI += WW_BLOCKSIZE, ++nBlock)
        {
            ULONG nBS = (nLen - nI > WW_BLOCKSIZE) ? WW_BLOCKSIZE : nLen - nI;
            rIn.Read(in, nBS);
            rCtx.InitCipher(nBlock);
            rCtx.Encode(in, nBS, in, nBS);
            rOut.Write(in, nBS);
        }
    }
}

void MSWordExportBase::ExportDocument( bool bWriteAll )
{
    nCharFmtStart = ANZ_DEFAULT_STYLES;
    nFmtCollStart = nCharFmtStart + pDoc->GetCharFmts()->Count() - 1;

    bStyDef = bBreakBefore = bOutKF =
        bOutFlyFrmAttrs = bOutPageDescs = bOutTable = bOutFirstPage =
        bIsInTable = bOutGrf = bInWriteEscher = bStartTOX =
        bInWriteTOX = false;

    bFtnAtTxtEnd = bEndAtTxtEnd = true;

    mpParentFrame = 0;
    pFlyOffset = 0;
    eNewAnchorType = FLY_AT_PAGE;
    nTxtTyp = TXT_MAINTEXT;
    nStyleBeforeFly = nLastFmtId = 0;
    pStyAttr = 0;
    pCurrentStyle = NULL;
    pOutFmtNode = 0;
    pEscher = 0;
    pRedlAuthors = 0;
    if ( aTOXArr.Count() )
        aTOXArr.Remove( 0, aTOXArr.Count() );

    if ( !pOLEExp )
    {
        UINT32 nSvxMSDffOLEConvFlags = 0;
        const SvtFilterOptions* pOpt = SvtFilterOptions::Get();
        if ( pOpt->IsMath2MathType() )
            nSvxMSDffOLEConvFlags |= OLE_STARMATH_2_MATHTYPE;
        if ( pOpt->IsWriter2WinWord() )
            nSvxMSDffOLEConvFlags |= OLE_STARWRITER_2_WINWORD;
        if ( pOpt->IsCalc2Excel() )
            nSvxMSDffOLEConvFlags |= OLE_STARCALC_2_EXCEL;
        if ( pOpt->IsImpress2PowerPoint() )
            nSvxMSDffOLEConvFlags |= OLE_STARIMPRESS_2_POWERPOINT;

        pOLEExp = new SvxMSExportOLEObjects( nSvxMSDffOLEConvFlags );
    }

    if ( !pOleMap)
        pOleMap = new WW8OleMaps;

    if ( !pOCXExp )
        pOCXExp = new SwMSConvertControls( pDoc->GetDocShell(), pCurPam );

    // #i81405# - Collect anchored objects before changing the redline mode.
    maFrames = GetFrames( *pDoc, bWriteAll? NULL : pOrigPam );

    mnRedlineMode = pDoc->GetRedlineMode();
    if ( pDoc->GetRedlineTbl().Count() )
    {
        pDoc->SetRedlineMode( (RedlineMode_t)(mnRedlineMode | nsRedlineMode_t::REDLINE_SHOW_DELETE |
                                     nsRedlineMode_t::REDLINE_SHOW_INSERT) );
    }

    maFontHelper.InitFontTable( SupportsUnicode(), *pDoc );
    GatherChapterFields();

    CollectOutlineBookmarks(*pDoc);

    // make unique OrdNums (Z-Order) for all drawing-/fly Objects
    if ( pDoc->GetDrawModel() )
        pDoc->GetDrawModel()->GetPage( 0 )->RecalcObjOrdNums();

    ExportDocument_Impl();

    if ( mnRedlineMode != pDoc->GetRedlineMode() )
        pDoc->SetRedlineMode( (RedlineMode_t)(mnRedlineMode) );
}

String SwWW8Writer::GetPassword()
{
    String sUniPassword;
    if ( mpMedium )
    {
        SfxItemSet* pSet = mpMedium->GetItemSet();

        const SfxPoolItem* pPasswordItem = NULL;
        if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_PASSWORD, sal_True, &pPasswordItem ) )
            if( pPasswordItem != NULL )
                sUniPassword = ( (const SfxStringItem*)pPasswordItem )->GetValue();
    }

    return sUniPassword;
}

void WW8Export::ExportDocument_Impl()
{
    PrepareStorage();

    pFib = new WW8Fib( bWrtWW8 ? 8 : 6 );

    SvStorageStreamRef xWwStrm( GetWriter().GetStorage().OpenSotStream( aMainStg ) );
    SvStorageStreamRef xTableStrm( xWwStrm ), xDataStrm( xWwStrm );
    xWwStrm->SetBufferSize( 32768 );

    if( bWrtWW8 )
    {
        pFib->fWhichTblStm = 1;
        xTableStrm = GetWriter().GetStorage().OpenSotStream(CREATE_CONST_ASC(SL::a1Table),
            STREAM_STD_WRITE );
        xDataStrm = GetWriter().GetStorage().OpenSotStream(CREATE_CONST_ASC(SL::aData),
            STREAM_STD_WRITE );

        xDataStrm->SetBufferSize( 32768 );  // fuer Grafiken
        xTableStrm->SetBufferSize( 16384 ); // fuer die Font-/Style-Table, usw.

        xTableStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        xDataStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    }

    GetWriter().SetStream( & *xWwStrm );
    pTableStrm = &xTableStrm;
    pDataStrm = &xDataStrm;

    Strm().SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    String sUniPassword( GetWriter().GetPassword() );

    utl::TempFile aTempMain;
    aTempMain.EnableKillingFile();
    utl::TempFile aTempTable;
    aTempTable.EnableKillingFile();
    utl::TempFile aTempData;
    aTempData.EnableKillingFile();

    bool bEncrypt = false;

    xub_StrLen nLen = sUniPassword.Len();
    if ( nLen > 0 && nLen <= 15) // Password has been set
    {
        bEncrypt =true;

        GetWriter().SetStream(
            aTempMain.GetStream( STREAM_READWRITE | STREAM_SHARE_DENYWRITE ) );

        pTableStrm = aTempTable.GetStream( STREAM_READWRITE | STREAM_SHARE_DENYWRITE );

        pDataStrm = aTempData.GetStream( STREAM_READWRITE | STREAM_SHARE_DENYWRITE );

        sal_uInt8 aRC4EncryptionHeader[ 52 ] = {0};
        pTableStrm->Write( aRC4EncryptionHeader, 52 );
    }

    // Default: "Standard"
    pSepx = new WW8_WrPlcSepx( *this );                         // Sections/headers/footers

    pFtn = new WW8_WrPlcFtnEdn( TXT_FTN );                      // Footnotes
    pEdn = new WW8_WrPlcFtnEdn( TXT_EDN );                      // Endnotes
    pAtn = new WW8_WrPlcAnnotations;                                 // PostIts
    pTxtBxs = new WW8_WrPlcTxtBoxes( TXT_TXTBOX );
    pHFTxtBxs = new WW8_WrPlcTxtBoxes( TXT_HFTXTBOX );

    pSdrObjs = new MainTxtPlcDrawObj;   // Draw-/Fly-Objects for main text
    pHFSdrObjs = new HdFtPlcDrawObj;    // Draw-/Fly-Objects for header/footer

    pBkmks = new WW8_WrtBookmarks;                          // Bookmarks
    GetWriter().CreateBookmarkTbl();

    pPapPlc = new WW8_WrPlcPn( *this, PAP, pFib->fcMin );
    pChpPlc = new WW8_WrPlcPn( *this, CHP, pFib->fcMin );
    pO = new WW8Bytes( 128, 128 );
    pStyles = new MSWordStyles( *this );
    pFldMain = new WW8_WrPlcFld( 2, TXT_MAINTEXT );
    pFldHdFt = new WW8_WrPlcFld( 2, TXT_HDFT );
    pFldFtn = new WW8_WrPlcFld( 2, TXT_FTN );
    pFldEdn = new WW8_WrPlcFld( 2, TXT_EDN );
    pFldAtn = new WW8_WrPlcFld( 2, TXT_ATN );
    pFldTxtBxs = new WW8_WrPlcFld( 2, TXT_TXTBOX );
    pFldHFTxtBxs = new WW8_WrPlcFld( 2, TXT_HFTXTBOX );

    pMagicTable = new WW8_WrMagicTable;

    pGrf = new SwWW8WrGrf( *this );
    pPiece = new WW8_WrPct( pFib->fcMin, bWrtWW8 );
    pDop = new WW8Dop;


    pDop->fRevMarking = 0 != ( nsRedlineMode_t::REDLINE_ON & mnRedlineMode );
    pDop->fRMView = 0 != ( nsRedlineMode_t::REDLINE_SHOW_DELETE & mnRedlineMode );
    pDop->fRMPrint = pDop->fRMView;

    // set AutoHyphenation flag if found in default para style
    const SfxPoolItem* pItem;
    SwTxtFmtColl* pStdTxtFmtColl =
        pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false);
    if (pStdTxtFmtColl && SFX_ITEM_SET == pStdTxtFmtColl->GetItemState(
        RES_PARATR_HYPHENZONE, false, &pItem))
    {
        pDop->fAutoHyphen = ((const SvxHyphenZoneItem*)pItem)->IsHyphen();
    }

    StoreDoc1();

    if ( bEncrypt )
    {
        // Generate random number with a seed of time as salt.
        TimeValue aTime;
        osl_getSystemTime( &aTime );
        rtlRandomPool aRandomPool = rtl_random_createPool ();
        rtl_random_addBytes ( aRandomPool, &aTime, 8 );

        sal_uInt8 aDocId[ 16 ] = {0};
        rtl_random_getBytes( aRandomPool, aDocId, 16 );

        rtl_random_destroyPool( aRandomPool );

        sal_Unicode aPassword[16] = {0};
        for (xub_StrLen nChar = 0; nChar < nLen; ++nChar )
            aPassword[nChar] = sUniPassword.GetChar(nChar);

        msfilter::MSCodec_Std97 aCtx;
        aCtx.InitKey(aPassword, aDocId);

        SvStream *pStrmTemp, *pTableStrmTemp, *pDataStrmTemp;
        pStrmTemp = &xWwStrm;
        pTableStrmTemp = &xTableStrm;
        pDataStrmTemp = &xDataStrm;

        if ( pDataStrmTemp && pDataStrmTemp != pStrmTemp)
            EncryptRC4(aCtx, *pDataStrm, *pDataStrmTemp);

        EncryptRC4(aCtx, *pTableStrm, *pTableStrmTemp);

        // Write Unencrypted Header 52 bytes to the start of the table stream
        // EncryptionVersionInfo (4 bytes): A Version structure where Version.vMajor MUST be 0x0001, and Version.vMinor MUST be 0x0001.
        pTableStrmTemp->Seek( 0 );
        sal_uInt32 nEncType = 0x10001;
        *pTableStrmTemp << nEncType;

        sal_uInt8 pSaltData[16] = {0};
        sal_uInt8 pSaltDigest[16] = {0};
        aCtx.GetEncryptKey( aDocId, pSaltData, pSaltDigest );

        pTableStrmTemp->Write( aDocId, 16 );
        pTableStrmTemp->Write( pSaltData, 16 );
        pTableStrmTemp->Write( pSaltDigest, 16 );

        EncryptRC4(aCtx, GetWriter().Strm(), *pStrmTemp);

        // Write Unencrypted Fib 68 bytes to the start of the workdocument stream
        pFib->fEncrypted = 1; // fEncrypted indicates the document is encrypted.
        pFib->fObfuscated = 0; // Must be 0 for RC4.
        pFib->nHash = 0x34; // encrypt header bytes count of table stream.
        pFib->nKey = 0; // lkey2 must be 0 for RC4.

        pStrmTemp->Seek( 0 );
        pFib->WriteHeader( *pStrmTemp );
    }

    if (pUsedNumTbl)           // all used NumRules
    {
        // clear the part of the list array that was copied from the document
        // - it's an auto delete array, so the rest of the array which are
        // duplicated lists that were added during the export will be deleted.
        pUsedNumTbl->Remove(0, pUsedNumTbl->Count()-nUniqueList);
        delete pUsedNumTbl;
    }

    DELETEZ( pGrf );
    DELETEZ( pMagicTable );
    DELETEZ( pFldFtn );
    DELETEZ( pFldTxtBxs );
    DELETEZ( pFldHFTxtBxs );
    DELETEZ( pFldAtn );
    DELETEZ( pFldEdn );
    DELETEZ( pFldHdFt );
    DELETEZ( pFldMain );
    DELETEZ( pStyles );
    DELETEZ( pO );
    DELETEZ( pChpPlc );
    DELETEZ( pPapPlc );
    DELETEZ( pSepx );

    delete pRedlAuthors;
    delete pSdrObjs;
    delete pHFSdrObjs;
    delete pTxtBxs;
    delete pHFTxtBxs;
    delete pAtn;
    delete pEdn;
    delete pFtn;
    delete pBkmks;
    delete pPiece;
    delete pDop;
    delete pFib;
    GetWriter().SetStream( 0 );


    xWwStrm->SetBufferSize( 0 );
    if( bWrtWW8 )
    {
        xTableStrm->SetBufferSize( 0 );
        xDataStrm->SetBufferSize( 0 );
        if( 0 == pDataStrm->Seek( STREAM_SEEK_TO_END ))
        {
            xDataStrm.Clear();
            pDataStrm = 0;
            GetWriter().GetStorage().Remove(CREATE_CONST_ASC(SL::aData));
        }
    }
}


void WW8Export::PrepareStorage()
{
    ULONG nLen;
    const BYTE* pData;
    const char* pName;
    UINT32 nId1;

    if (bWrtWW8)
    {
        static const char aUserName[] = "Microsoft Word-Document";
        static const BYTE aCompObj[] =
        {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0x09, 0x02, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x46, 0x18, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x57, 0x6F, 0x72, 0x64, 0x2D, 0x44,
            0x6F, 0x6B, 0x75, 0x6D, 0x65, 0x6E, 0x74, 0x00,
            0x0A, 0x00, 0x00, 0x00, 0x4D, 0x53, 0x57, 0x6F,
            0x72, 0x64, 0x44, 0x6F, 0x63, 0x00, 0x10, 0x00,
            0x00, 0x00, 0x57, 0x6F, 0x72, 0x64, 0x2E, 0x44,
            0x6F, 0x63, 0x75, 0x6D, 0x65, 0x6E, 0x74, 0x2E,
            0x38, 0x00, 0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00
        };

        pName = aUserName;
        pData = aCompObj;
        nLen = sizeof( aCompObj );
        nId1 = 0x00020906L;
    }
    else
    {
        static const char aUserName[] = "Microsoft Word 6.0 Document";
        static const BYTE aCompObj[] =
        {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x09, 0x02, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x46, 0x1C, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x57, 0x6F, 0x72, 0x64, 0x20, 0x36,
            0x2E, 0x30, 0x2D, 0x44, 0x6F, 0x6B, 0x75, 0x6D,
            0x65, 0x6E, 0x74, 0x00, 0x0A, 0x00, 0x00, 0x00,
            0x4D, 0x53, 0x57, 0x6F, 0x72, 0x64, 0x44, 0x6F,
            0x63, 0x00, 0x10, 0x00, 0x00, 0x00, 0x57, 0x6F,
            0x72, 0x64, 0x2E, 0x44, 0x6F, 0x63, 0x75, 0x6D,
            0x65, 0x6E, 0x74, 0x2E, 0x36, 0x00, 0x00, 0x00,
            0x00, 0x00
        };

        pName = aUserName;
        pData = aCompObj;
        nLen = sizeof( aCompObj );
        nId1 = 0x00020900L;
    }

    SvGlobalName aGName( nId1, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x46 );
    GetWriter().GetStorage().SetClass( aGName, 0, String::CreateFromAscii( pName ));
    SvStorageStreamRef xStor( GetWriter().GetStorage().OpenSotStream(sCompObj) );
    xStor->Write( pData, nLen );

    SwDocShell* pDocShell = pDoc->GetDocShell ();
    DBG_ASSERT(pDocShell, "no SwDocShell");

    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        DBG_ASSERT(xDocProps.is(), "DocumentProperties is null");

        if (xDocProps.is())
        {
            if ( SvtFilterOptions::Get()->IsEnableWordPreview() )
            {
                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    pDocShell->GetPreviewMetaFile (sal_False);
                uno::Sequence<sal_uInt8> metaFile(
                    sfx2::convertMetaFile(pMetaFile.get()));
                sfx2::SaveOlePropertySet(xDocProps, &GetWriter().GetStorage(), &metaFile);
            }
            else
                sfx2::SaveOlePropertySet( xDocProps, &GetWriter().GetStorage() );
        }
    }
}

ULONG SwWW8Writer::WriteStorage()
{
    // #i34818# - update layout (if present), for SwWriteTable
    ViewShell* pViewShell = NULL;
    pDoc->GetEditShell( &pViewShell );
    if( pViewShell != NULL )
        pViewShell->CalcLayout();

    long nMaxNode = pDoc->GetNodes().Count();
    ::StartProgress( STR_STATSTR_W4WWRITE, 0, nMaxNode, pDoc->GetDocShell() );

    // Tabelle am Doc.-Anfang beachten
    {
        SwTableNode * pTNd = pCurPam->GetNode()->FindTableNode();
        if( pTNd && bWriteAll )
            // mit dem TabellenNode anfangen !!
            pCurPam->GetPoint()->nNode = *pTNd;
    }

    // Do the actual export
    {
        WW8Export aExport( this, pDoc, pCurPam, pOrigPam, m_bWrtWW8 );
        m_pExport = &aExport;
        aExport.ExportDocument( bWriteAll );
        m_pExport = NULL;
    }

    ::EndProgress( pDoc->GetDocShell() );
    return 0;
}

ULONG SwWW8Writer::WriteMedium( SfxMedium& )
{
    return WriteStorage();
}

ULONG SwWW8Writer::Write( SwPaM& rPaM, SfxMedium& rMed,
                          const String* pFileName )
{
    mpMedium = &rMed;
    ULONG nRet = StgWriter::Write( rPaM, rMed, pFileName );
    mpMedium = NULL;
    return nRet;
}

MSWordExportBase::MSWordExportBase( SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam )
    : aMainStg(sMainStream), pISet(0), pUsedNumTbl(0), mpTopNodeOfHdFtPage(0),
    pBmpPal(0), pKeyMap(0), pOLEExp(0), pOCXExp(0), pOleMap(0),
    mpTableInfo(new ww8::WW8TableInfo()), nUniqueList(0),
    mnHdFtIndex(0), pAktPageDesc(0), pPapPlc(0), pChpPlc(0), pChpIter(0),
    pStyles( NULL ),
    bHasHdr(false), bHasFtr(false), bSubstituteBullets(true),
    mbExportModeRTF( false ),
    pDoc( pDocument ),
    pCurPam( pCurrentPam ),
    pOrigPam( pOriginalPam )
{
}

MSWordExportBase::~MSWordExportBase()
{
    delete pBmpPal;
    if (pKeyMap)
        delete[] (NfKeywordTable*)pKeyMap;
    delete pOLEExp;
    delete pOCXExp;
    delete pOleMap;
}

WW8Export::WW8Export( SwWW8Writer *pWriter,
        SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam,
        bool bIsWW8 )
    : MSWordExportBase( pDocument, pCurrentPam, pOriginalPam ),
      pO( NULL ),
      mpTableAt( NULL ),
      mnTableStdAtLen( 0 ),
      pSepx( NULL ),
      bWrtWW8( bIsWW8 ),
      m_pWriter( pWriter ),
      m_pAttrOutput( new WW8AttributeOutput( *this ) )
{
}

WW8Export::~WW8Export()
{
    delete m_pAttrOutput, m_pAttrOutput = NULL;
}

AttributeOutputBase& WW8Export::AttrOutput() const
{
    return *m_pAttrOutput;
}

MSWordSections& WW8Export::Sections() const
{
    return *pSepx;
}

SwWW8Writer::SwWW8Writer(const String& rFltName, const String& rBaseURL)
    : StgWriter(),
      m_bWrtWW8( rFltName.EqualsAscii( FILTER_WW8 ) ),
      m_pExport( NULL ),
      mpMedium( 0 )
{
    SetBaseURL( rBaseURL );
}

SwWW8Writer::~SwWW8Writer()
{
}

extern "C" SAL_DLLPUBLIC_EXPORT ULONG SAL_CALL SaveOrDelMSVBAStorage_ww8( SfxObjectShell& rDoc, SotStorage& rStor, BOOL bSaveInto, const String& rStorageName )
{
    SvxImportMSVBasic aTmp( rDoc, rStor );
    return aTmp.SaveOrDelMSVBAStorage( bSaveInto, rStorageName );
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL ExportDOC( const String& rFltName, const String& rBaseURL, WriterRef& xRet )
{
    xRet = new SwWW8Writer( rFltName, rBaseURL );
}


extern "C" SAL_DLLPUBLIC_EXPORT ULONG SAL_CALL GetSaveWarningOfMSVBAStorage_ww8(  SfxObjectShell &rDocS )
{
    return SvxImportMSVBasic::GetSaveWarningOfMSVBAStorage( rDocS );
}

bool WW8_WrPlcFtnEdn::WriteTxt( WW8Export& rWrt )
{
    bool bRet = false;
    if (TXT_FTN == nTyp)
    {
        bRet = WriteGenericTxt( rWrt, TXT_FTN, rWrt.pFib->ccpFtn );
        rWrt.pFldFtn->Finish( rWrt.Fc2Cp( rWrt.Strm().Tell() ),
                            rWrt.pFib->ccpText );
    }
    else
    {
        bRet = WriteGenericTxt( rWrt, TXT_EDN, rWrt.pFib->ccpEdn );
        rWrt.pFldEdn->Finish( rWrt.Fc2Cp( rWrt.Strm().Tell() ),
                            rWrt.pFib->ccpText + rWrt.pFib->ccpFtn
                            + rWrt.pFib->ccpHdr + rWrt.pFib->ccpAtn );
    }
    return bRet;
}

void WW8_WrPlcFtnEdn::WritePlc( WW8Export& rWrt ) const
{
    if( TXT_FTN == nTyp )
    {
        WriteGenericPlc( rWrt, TXT_FTN, rWrt.pFib->fcPlcffndTxt,
            rWrt.pFib->lcbPlcffndTxt, rWrt.pFib->fcPlcffndRef,
            rWrt.pFib->lcbPlcffndRef );
    }
    else
    {
        WriteGenericPlc( rWrt, TXT_EDN, rWrt.pFib->fcPlcfendTxt,
            rWrt.pFib->lcbPlcfendTxt, rWrt.pFib->fcPlcfendRef,
            rWrt.pFib->lcbPlcfendRef );
    }
}


bool WW8_WrPlcAnnotations::WriteTxt( WW8Export& rWrt )
{
    bool bRet = WriteGenericTxt( rWrt, TXT_ATN, rWrt.pFib->ccpAtn );
    rWrt.pFldAtn->Finish( rWrt.Fc2Cp( rWrt.Strm().Tell() ),
                        rWrt.pFib->ccpText + rWrt.pFib->ccpFtn
                        + rWrt.pFib->ccpHdr );
    return bRet;
}

void WW8_WrPlcAnnotations::WritePlc( WW8Export& rWrt ) const
{
    WriteGenericPlc( rWrt, TXT_ATN, rWrt.pFib->fcPlcfandTxt,
        rWrt.pFib->lcbPlcfandTxt, rWrt.pFib->fcPlcfandRef,
        rWrt.pFib->lcbPlcfandRef );
}

void WW8_WrPlcTxtBoxes::WritePlc( WW8Export& rWrt ) const
{
    if( TXT_TXTBOX == nTyp )
    {
        WriteGenericPlc( rWrt, nTyp, rWrt.pFib->fcPlcftxbxBkd,
            rWrt.pFib->lcbPlcftxbxBkd, rWrt.pFib->fcPlcftxbxTxt,
            rWrt.pFib->lcbPlcftxbxTxt );
    }
    else
    {
        WriteGenericPlc( rWrt, nTyp, rWrt.pFib->fcPlcfHdrtxbxBkd,
            rWrt.pFib->lcbPlcfHdrtxbxBkd, rWrt.pFib->fcPlcfHdrtxbxTxt,
            rWrt.pFib->lcbPlcfHdrtxbxTxt );
    }
}

void WW8Export::RestoreMacroCmds()
{
    pFib->fcCmds = pTableStrm->Tell();

    uno::Reference < embed::XStorage > xSrcRoot(pDoc->GetDocShell()->GetStorage());
    try
    {
        uno::Reference < io::XStream > xSrcStream =
                xSrcRoot->openStreamElement( CREATE_CONST_ASC(SL::aMSMacroCmds), embed::ElementModes::READ );
        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xSrcStream );

        if ( pStream && SVSTREAM_OK == pStream->GetError())
        {
            pStream->Seek(STREAM_SEEK_TO_END);
            pFib->lcbCmds = pStream->Tell();
            pStream->Seek(0);

            sal_uInt8 *pBuffer = new sal_uInt8[pFib->lcbCmds];
            pStream->Read(pBuffer, pFib->lcbCmds);
            pTableStrm->Write(pBuffer, pFib->lcbCmds);
            delete[] pBuffer;

        }

        delete pStream;
    }
    catch ( uno::Exception& )
    {
    }

    // set len to FIB
    pFib->lcbCmds = pTableStrm->Tell() - pFib->fcCmds;
}

void WW8SHDLong::Write( WW8Export& rExport )
{
    rExport.InsUInt32( m_cvFore );
    rExport.InsUInt32( m_cvBack );
    rExport.InsUInt16( m_ipat );
}

void WW8Export::WriteFormData( const ::sw::mark::IFieldmark& rFieldmark )
{
    OSL_ENSURE( bWrtWW8, "No 95 export yet" );
    if ( !bWrtWW8 )
        return;

    const ::sw::mark::IFieldmark* pFieldmark = &rFieldmark;
    const ::sw::mark::ICheckboxFieldmark* pAsCheckbox = dynamic_cast< const ::sw::mark::ICheckboxFieldmark* >( pFieldmark );


    OSL_ENSURE(rFieldmark.GetFieldname().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ODF_FORMTEXT ) ) ||
                rFieldmark.GetFieldname().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ODF_FORMDROPDOWN ) ) ||
                rFieldmark.GetFieldname().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ODF_FORMCHECKBOX ) ), "Unknown field type!!!");
    if ( ! ( rFieldmark.GetFieldname().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ODF_FORMTEXT ) ) ||
                rFieldmark.GetFieldname().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ODF_FORMDROPDOWN ) ) ||
                rFieldmark.GetFieldname().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ODF_FORMCHECKBOX ) ) ) )
        return;

    int type = 0; // TextFieldmark
    if ( pAsCheckbox )
        type = 1;
    if ( rFieldmark.GetFieldname().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ODF_FORMDROPDOWN ) ) )
        type=2;

    ::sw::mark::IFieldmark::parameter_map_t::const_iterator pNameParameter = rFieldmark.GetParameters()->find(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("name")));
    ::rtl::OUString ffname;
    if(pNameParameter != rFieldmark.GetParameters()->end())
        pNameParameter->second >>= ffname;

    ULONG nDataStt = pDataStrm->Tell();
    pChpPlc->AppendFkpEntry(Strm().Tell());

    WriteChar(0x01);
    static BYTE aArr1[] =
    {
        0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation

        0x06, 0x08, 0x01,       // sprmCFData
        0x55, 0x08, 0x01,       // sprmCFSpec
        0x02, 0x08, 0x01        // sprmCFFldVanish
    };
    BYTE* pDataAdr = aArr1 + 2;
    Set_UInt32(pDataAdr, nDataStt);

    pChpPlc->AppendFkpEntry( Strm().Tell(), sizeof( aArr1 ), aArr1 );

    struct FFDataHeader
    {
        sal_uInt32 version;
        sal_uInt16 bits;
        sal_uInt16 cch;
        sal_uInt16 hps;
        FFDataHeader() : version( 0xFFFFFFFF ), bits(0), cch(0), hps(0) {}
    };


    FFDataHeader aFldHeader;
    aFldHeader.bits |= (type & 0x03);

    sal_Int32 ffres = 0; // rFieldmark.GetFFRes();
    if ( pAsCheckbox && pAsCheckbox->IsChecked() )
        ffres = 1;
    else if ( type == 2 )
    {
        ::sw::mark::IFieldmark::parameter_map_t::const_iterator pResParameter = rFieldmark.GetParameters()->find(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ODF_FORMDROPDOWN_RESULT)));
        if(pResParameter != rFieldmark.GetParameters()->end())
            pResParameter->second >>= ffres;
        else
            ffres = 0;
    }
    aFldHeader.bits |= ( (ffres<<2) & 0x7C );

    std::vector< ::rtl::OUString > aListItems;
    if (type==2)
    {
        aFldHeader.bits |= 0x8000; // ffhaslistbox
        const ::sw::mark::IFieldmark::parameter_map_t* const pParameters = rFieldmark.GetParameters();
        ::sw::mark::IFieldmark::parameter_map_t::const_iterator pListEntries = pParameters->find(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ODF_FORMDROPDOWN_LISTENTRY)));
        if(pListEntries != pParameters->end())
        {
            uno::Sequence< ::rtl::OUString > vListEntries;
            pListEntries->second >>= vListEntries;
            copy(::comphelper::stl_begin(vListEntries), ::comphelper::stl_end(vListEntries), back_inserter(aListItems));
        }
    }

    const ::rtl::OUString ffdeftext;
    const ::rtl::OUString ffformat;
    const ::rtl::OUString ffhelptext;
    const ::rtl::OUString ffstattext;
    const ::rtl::OUString ffentrymcr;
    const ::rtl::OUString ffexitmcr;


    const sal_uInt8 aFldData[] =
    {
        0x44,0,         // the start of "next" data
        0,0,0,0,0,0,0,0,0,0,                // PIC-Structure!  /10
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |              /16
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |              /16
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |              /16
        0,0,0,0,                            // /               /4
    };
   sal_uInt32 slen = sizeof(sal_uInt32)
        + sizeof(aFldData)
        + sizeof( aFldHeader.version ) + sizeof( aFldHeader.bits ) + sizeof( aFldHeader.cch ) + sizeof( aFldHeader.hps )
        + 2*ffname.getLength() + 4
        + 2*ffformat.getLength() + 4
        + 2*ffhelptext.getLength() + 4
        + 2*ffstattext.getLength() + 4
        + 2*ffentrymcr.getLength() + 4
        + 2*ffexitmcr.getLength() + 4;
    if ( type )
        slen += 2; // wDef
    else
        slen += 2*ffdeftext.getLength() + 4; //xstzTextDef
    if ( type==2 ) {
        slen += 2; // sttb ( fExtend )
        slen += 4; // for num of list items
        const int items = aListItems.size();
        for( int i = 0; i < items; i++ ) {
            rtl::OUString item = aListItems[i];
            slen += 2 * item.getLength() + 2;
        }
    }

    *pDataStrm << slen;

    int len = sizeof( aFldData );
    OSL_ENSURE( len == 0x44-sizeof(sal_uInt32), "SwWW8Writer::WriteFormData(..) - wrong aFldData length" );
    pDataStrm->Write( aFldData, len );

    *pDataStrm << aFldHeader.version << aFldHeader.bits << aFldHeader.cch << aFldHeader.hps;

    SwWW8Writer::WriteString_xstz( *pDataStrm, ffname, true ); // Form field name

    if ( !type )
        SwWW8Writer::WriteString_xstz( *pDataStrm, ffdeftext, true );
    if ( type )
        *pDataStrm << sal_uInt16(0);


    SwWW8Writer::WriteString_xstz( *pDataStrm, String( ffformat ), true );
    SwWW8Writer::WriteString_xstz( *pDataStrm, String( ffhelptext ), true );
    SwWW8Writer::WriteString_xstz( *pDataStrm, String( ffstattext ), true );
    SwWW8Writer::WriteString_xstz( *pDataStrm, String( ffentrymcr ), true );
    SwWW8Writer::WriteString_xstz( *pDataStrm, String( ffexitmcr ), true );
    if (type==2) {
        *pDataStrm<<(sal_uInt16)0xFFFF;
        const int items=aListItems.size();
        *pDataStrm<<(sal_uInt32)items;
        for(int i=0;i<items;i++) {
            rtl::OUString item=aListItems[i];
            SwWW8Writer::WriteString_xstz( *pDataStrm, item, false );
        }
    }
}

void WW8Export::WriteHyperlinkData( const sw::mark::IFieldmark& /*rFieldmark*/ )
{
    //@TODO implement me !!!
}

void WW8AttributeOutput::TableNodeInfoInner( ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner )
{
    SVBT16 nStyle;
    ShortToSVBT16( m_rWW8Export.nStyleBeforeFly, nStyle );

#ifdef DEBUG
    ::std::clog << "<OutWW8_TableNodeInfoInner>" << pNodeInfoInner->toString();
#endif

    m_rWW8Export.pO->Remove( 0, m_rWW8Export.pO->Count() );                       // leeren

    sal_uInt32 nShadowsBefore = pNodeInfoInner->getShadowsBefore();
    if (nShadowsBefore > 0)
    {
        ww8::WW8TableNodeInfoInner::Pointer_t
            pTmpNodeInfoInner(new ww8::WW8TableNodeInfoInner(NULL));

        pTmpNodeInfoInner->setDepth(pNodeInfoInner->getDepth());
        pTmpNodeInfoInner->setEndOfCell(true);

        for (sal_uInt32 n = 0; n < nShadowsBefore; ++n)
        {
            m_rWW8Export.WriteCR(pTmpNodeInfoInner);

            m_rWW8Export.pO->Insert( (BYTE*)&nStyle, 2,
                                     m_rWW8Export.pO->Count() );     // Style #
            TableInfoCell(pTmpNodeInfoInner);
            m_rWW8Export.pPapPlc->AppendFkpEntry
                ( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->Count(),
                  m_rWW8Export.pO->GetData() );

            m_rWW8Export.pO->Remove( 0, m_rWW8Export.pO->Count() );                       // leeren
        }
    }

    if (pNodeInfoInner->isEndOfCell())
    {
#ifdef DEBUG
        ::std::clog << "<endOfCell/>" << ::std::endl;
#endif
        m_rWW8Export.WriteCR(pNodeInfoInner);

        m_rWW8Export.pO->Insert( (BYTE*)&nStyle, 2, m_rWW8Export.pO->Count() );     // Style #
        TableInfoCell(pNodeInfoInner);
        m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->Count(),
                                m_rWW8Export.pO->GetData() );

        m_rWW8Export.pO->Remove( 0, m_rWW8Export.pO->Count() );                       // leeren
    }

    sal_uInt32 nShadowsAfter = pNodeInfoInner->getShadowsAfter();
    if (nShadowsAfter > 0)
    {
        ww8::WW8TableNodeInfoInner::Pointer_t
            pTmpNodeInfoInner(new ww8::WW8TableNodeInfoInner(NULL));

        pTmpNodeInfoInner->setDepth(pNodeInfoInner->getDepth());
        pTmpNodeInfoInner->setEndOfCell(true);

        for (sal_uInt32 n = 0; n < nShadowsAfter; ++n)
        {
            m_rWW8Export.WriteCR(pTmpNodeInfoInner);

            m_rWW8Export.pO->Insert( (BYTE*)&nStyle, 2, m_rWW8Export.pO->Count() );     // Style #
            TableInfoCell(pTmpNodeInfoInner);
            m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->Count(),
                                                  m_rWW8Export.pO->GetData() );

            m_rWW8Export.pO->Remove( 0, m_rWW8Export.pO->Count() );                       // leeren
        }
    }

    if (pNodeInfoInner->isEndOfLine())
    {
#ifdef DEBUG
        ::std::clog << "<endOfLine/>" << ::std::endl;
#endif
        TableRowEnd(pNodeInfoInner->getDepth());

        ShortToSVBT16(0, nStyle);
        m_rWW8Export.pO->Insert( (BYTE*)&nStyle, 2, m_rWW8Export.pO->Count() );     // Style #
        TableInfoRow(pNodeInfoInner);
        m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->Count(),
                                m_rWW8Export.pO->GetData() );

        m_rWW8Export.pO->Remove( 0, m_rWW8Export.pO->Count() );                       // leeren
    }
#ifdef DEBUG
    ::std::clog << "</OutWW8_TableNodeInfoInner>" << ::std::endl;
#endif
}

void MSWordExportBase::OutputStartNode( const SwStartNode & rNode)
{

    ww8::WW8TableNodeInfo::Pointer_t pNodeInfo =
        mpTableInfo->getTableNodeInfo( &rNode );

    if (pNodeInfo.get() != NULL)
    {
#ifdef DEBUG
        ::std::clog << pNodeInfo->toString() << ::std::endl;
#endif

        const ww8::WW8TableNodeInfo::Inners_t aInners = pNodeInfo->getInners();
        ww8::WW8TableNodeInfo::Inners_t::const_reverse_iterator aIt(aInners.rbegin());
        ww8::WW8TableNodeInfo::Inners_t::const_reverse_iterator aEnd(aInners.rend());
        while (aIt != aEnd)
        {
            ww8::WW8TableNodeInfoInner::Pointer_t pInner = aIt->second;

            AttrOutput().TableNodeInfoInner(pInner);
            ++aIt;
        }
    }
#ifdef DEBUG
    ::std::clog << "</OutWW8_SwStartNode>" << ::std::endl;
#endif
}

void MSWordExportBase::OutputEndNode( const SwEndNode &rNode )
{
#ifdef DEBUG
// whoever has need of the missing function should go and implement it!
// This piece of code always breaks builds...
//    ::std::clog << "<OutWW8_SwEndNode>" << dbg_out(&rNode) << ::std::endl;
#endif

    ww8::WW8TableNodeInfo::Pointer_t pNodeInfo = mpTableInfo->getTableNodeInfo( &rNode );

    if (pNodeInfo.get() != NULL)
     {
#ifdef DEBUG
        ::std::clog << pNodeInfo->toString() << ::std::endl;
#endif

        const ww8::WW8TableNodeInfo::Inners_t aInners = pNodeInfo->getInners();
        ww8::WW8TableNodeInfo::Inners_t::const_iterator aIt(aInners.begin());
        ww8::WW8TableNodeInfo::Inners_t::const_iterator aEnd(aInners.end());
        while (aIt != aEnd)
         {
            ww8::WW8TableNodeInfoInner::Pointer_t pInner = aIt->second;
            AttrOutput().TableNodeInfoInner(pInner);
            ++aIt;
         }
     }
#ifdef DEBUG
    ::std::clog << "</OutWW8_SwEndNode>" << ::std::endl;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
