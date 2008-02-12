/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrtww8.cxx,v $
 *
 *  $Revision: 1.87 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 14:23:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#include <unotools/ucbstreamhelper.hxx>


#ifndef __SGI_STL_ALGORITHM
#include <algorithm>
#endif


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#include <string.h>             // memcpy()

#ifndef _OSL_ENDIAN_H_
#include <osl/endian.h>
#endif

#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif

#define _SVSTDARR_BOOLS
#include <svtools/svstdarr.hxx>

#include <svtools/fltrcfg.hxx>

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif
#include <sot/storage.hxx>
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif
#ifndef _SFXDOCINF_HXX
#include <sfx2/docinf.hxx>
#endif

#ifndef _SVX_TSPTITEM_HXX
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _MSOLEEXP_HXX
#include <svx/msoleexp.hxx>
#endif
#ifndef _MSOCXIMEX_HXX
#include <svx/msocximex.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _FMTCNTNT_HXX
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _WRTWW8_HXX
#include <wrtww8.hxx>
#endif
#ifndef _WW8PAR_HXX
#include <ww8par.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _SWFLTOPT_HXX
#include <swfltopt.hxx>
#endif
#ifndef _FMTINFMT_HXX
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTINET_HXX
#include <txtinet.hxx>
#endif
#ifndef _FMTURL_HXX
#include <fmturl.hxx>
#endif
#ifndef _FESH_HXX               // SwFEShell, #i34818
#include <fesh.hxx>
#endif
#ifndef _IMAP_HXX
#include <svtools/imap.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // Progress
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif

#ifndef SW_FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef SW_WRITERHELPER
#include "writerhelper.hxx"
#endif
#ifndef SW_WRITERWORDGLUE
#include "writerwordglue.hxx"
#endif

using namespace sw::util;
using namespace sw::types;

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
    SvBools aFieldBookmarks;       // If the bookmark is in a field result
    std::vector<String> maSwBkmkNms;    // Array of Sw - Bookmarknames
    typedef std::vector<String>::iterator myIter;

    USHORT GetPos( const String& rNm );

    //No copying
    WW8_WrtBookmarks(const WW8_WrtBookmarks&);
    WW8_WrtBookmarks& operator=(const WW8_WrtBookmarks&);
public:
    WW8_WrtBookmarks();
    ~WW8_WrtBookmarks();

    void Append( WW8_CP nStartCp, const String& rNm );
    void Write( SwWW8Writer& rWrt );
    void MoveFieldBookmarks(ULONG nFrom,ULONG nTo);

//  String GetWWBkmkName( const String& rName ) const;
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

static void WriteDop( SwWW8Writer& rWrt )
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

    const SfxDocumentInfo *pInf = rWrt.pDoc->GetDocumentInfo();
    ASSERT(pInf, "No document Info, very suspicious");
    if (!pInf)
        rDop.dttmCreated = rDop.dttmRevised = rDop.dttmLastPrint = 0x45FBAC69;
    else
    {
        rDop.dttmCreated = sw::ms::DateTime2DTTM(pInf->GetCreationDate());
        rDop.dttmRevised = sw::ms::DateTime2DTTM(pInf->GetModificationDate());
        rDop.dttmLastPrint = sw::ms::DateTime2DTTM(pInf->GetPrintDate());
    }

    rDop.fProtEnabled = rWrt.pSepx ? rWrt.pSepx->DocumentIsProtected() : 0;

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
void SwWW8Writer::ExportDopTypography(WW8DopTypography &rTypo)
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
    most and choose that, for now we choose the first and throw an ASSERT.
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

    ASSERT( nNoNeeded<=1, "Example of unexportable forbidden chars" );
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

    const IDocumentSettingAccess* pIDocumentSettingAccess = getIDocumentSettingAccess();

    rTypo.fKerningPunct = pIDocumentSettingAccess->get(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION);
    rTypo.iJustification = pDoc->getCharacterCompressionType();
}

// HasItem ist fuer die Zusammenfassung der Doppel-Attribute
// Underline / WordLineMode und Box / Shadow.
// Es kann nur etwas gefunden werden, wenn diese Methode innerhalb
// der aufgerufenen Methoden WW8_SwAttrIter::OutAttr() und
// SwWW8Writer::Out_SfxItemSet() benutzt wird.
const SfxPoolItem* SwWW8Writer::HasItem( USHORT nWhich ) const
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
        ASSERT( !this, "Wo ist mein ItemSet / pChpIter ?" );
        pItem = 0;
    }
    return pItem;
}

const SfxPoolItem& SwWW8Writer::GetItem(USHORT nWhich) const
{
    const SfxPoolItem* pItem;
    if (pISet)
    {
        // if write a EditEngine text, then the WhichIds are greater as
        // ourer own Ids. So the Id have to translate from our into the
        // EditEngine Range
        nWhich = sw::hack::GetSetWhichFromSwDocWhich(*pISet, *pDoc, nWhich);
        ASSERT(nWhich != 0, "All broken, Impossible");
        pItem = &pISet->Get(nWhich, true);
    }
    else if( pChpIter )
        pItem = &pChpIter->GetItem( nWhich );
    else
    {
        ASSERT( !this, "Wo ist mein ItemSet / pChpIter ?" );
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
    ASSERT(nLen,"Prev called on empty list");
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


bool WW8_WrPlcFld::Write(SwWW8Writer& rWrt)
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

bool WW8_WrMagicTable::Write(SwWW8Writer& rWrt)
{
    if( WW8_WrPlc1::Count() <= 1 )
        return false;
    ULONG nFcStart = rWrt.pTableStrm->Tell();
    WW8_WrPlc1::Write( *rWrt.pTableStrm );
    rWrt.pFib->fcMagicTable = nFcStart;
    rWrt.pFib->lcbMagicTable = rWrt.pTableStrm->Tell() - nFcStart;
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
        nEndPos = nCurPos + 0x1ff & ~0x1ffUL;

    if( nEndPos > nCurPos )
        SwWW8Writer::FillCount( rStrm, nEndPos - nCurPos );
#ifndef PRODUCT
    else
        ASSERT( nEndPos == nCurPos, "Falsches FillUntil()" );
#endif
    return rStrm.Tell();
}


//--------------------------------------------------------------------------
/*  */

WW8_WrPlcPn::WW8_WrPlcPn( SwWW8Writer& rWr, ePLCFT ePl, WW8_FC nStartFc )
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
            ASSERT( !this, "Sprm liess sich nicht einfuegen" );
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
    ASSERT( !nVarLen || pSprms, "Item-Pointer fehlt" );
    ASSERT( nVarLen < ( ( ePlc == PAP ) ? 497U : 502U ), "Sprms zu lang !" );

    if( bCombined )
    {
        ASSERT( !this, "Fkp::Append: Fkp is already combined" );
        return false;
    }
    INT32 n = ((INT32*)pFkp)[nIMax];        // letzter Eintrag
    if( nEndFc <= n )
    {
        ASSERT( nEndFc >= n, "+Fkp: FC rueckwaerts" );
        ASSERT( !nVarLen || !pSprms || nEndFc != n,
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
    DELETEZ( pOfs );
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
            ASSERT( 1 == pPcts->Count(), "Leeres Piece !!");
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


void WW8_WrPct::WritePc( SwWW8Writer& rWrt )
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
    ASSERT( pPcts->Count(),"SetParaBreak : aPcts.Count = 0" );
    pPcts->GetObject( pPcts->Count() - 1)->SetStatus();
}

WW8_CP WW8_WrPct::Fc2Cp( ULONG nFc ) const
{
    ASSERT( nFc >= (ULONG)nOldFc, "FilePos liegt vorm letzten Piece" );
    ASSERT( pPcts->Count(), "Fc2Cp noch kein Piece vorhanden" );

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

void WW8_WrtBookmarks::Append( WW8_CP nStartCp, const String& rNm )
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
        aFieldBookmarks.Insert(BOOL(false), nPos);
        maSwBkmkNms.insert(aIter, rNm);
    }
    else
    {
        // old -> its the end position
        ASSERT( aEndCps[ nPos ] == aSttCps[ nPos ], "end position is valid" );

        //If this bookmark was around a field in writer, then we want to move
        //it to the field result in word. The end is therefore one cp
        //backwards from the 0x15 end mark that was inserted.
        if (aFieldBookmarks[nPos])
            --nStartCp;

        aEndCps.Replace( nStartCp, nPos );
    }
}


void WW8_WrtBookmarks::Write( SwWW8Writer& rWrt )
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

void WW8_WrtBookmarks::MoveFieldBookmarks(ULONG nFrom, ULONG nTo)
{
    for (USHORT nI=0;nI<aSttCps.Count();++nI)
    {
        if (aSttCps[nI] == nFrom)
        {
            aSttCps[nI] = nTo;
            if (aEndCps[nI] == nFrom)
            {
                aFieldBookmarks[nI] = true;
                aEndCps[nI] = nTo;
            }
        }
    }
}

void SwWW8Writer::AppendBookmarks( const SwTxtNode& rNd,
                                    xub_StrLen nAktPos, xub_StrLen nLen )
{
    SvPtrarr aArr( 8, 8 );
    USHORT nCntnt;
    xub_StrLen nAktEnd = nAktPos + nLen;
    if( GetBookmarks( rNd, nAktPos, nAktEnd, aArr ))
    {
        ULONG nNd = rNd.GetIndex(), nSttCP = Fc2Cp( Strm().Tell() );
        for( USHORT n = 0; n < aArr.Count(); ++n )
        {
            const SwBookmark& rBkmk = *(SwBookmark*)aArr[ n ];
            const SwPosition* pPos = &rBkmk.GetPos(),
                            * pOPos = rBkmk.GetOtherPos();
            if( pOPos && pOPos->nNode == pPos->nNode &&
                pOPos->nContent < pPos->nContent )
            {
                pOPos = pPos;
                pPos = rBkmk.GetOtherPos();
            }

            if( !pOPos || ( nNd == pPos->nNode.GetIndex() &&
                ( nCntnt = pPos->nContent.GetIndex() ) >= nAktPos &&
                nCntnt < nAktEnd ) )
            {
                ULONG nCp = nSttCP + pPos->nContent.GetIndex() - nAktPos;
                pBkmks->Append(nCp, BookmarkToWord(rBkmk.GetName()));
            }
            if( pOPos && nNd == pOPos->nNode.GetIndex() &&
                ( nCntnt = pOPos->nContent.GetIndex() ) >= nAktPos &&
                nCntnt < nAktEnd )
            {
                ULONG nCp = nSttCP + pOPos->nContent.GetIndex() - nAktPos;
                pBkmks->Append(nCp, BookmarkToWord(rBkmk.GetName()));
            }
        }
    }
}

void SwWW8Writer::MoveFieldBookmarks(ULONG nFrom, ULONG nTo)
{
    pBkmks->MoveFieldBookmarks(nFrom, nTo);
}

void SwWW8Writer::AppendBookmark( const String& rName, USHORT nOffset )
{
    ULONG nSttCP = Fc2Cp( Strm().Tell() ) + nOffset;
    pBkmks->Append( nSttCP, rName );
}


//--------------------------------------------------------------------------
/*  */

void WW8_WrtRedlineAuthor::Write( Writer& rWrt )
{
    SwWW8Writer & rWW8Wrt = (SwWW8Writer&)rWrt;
    rWW8Wrt.WriteAsStringTable(maAuthors, rWW8Wrt.pFib->fcSttbfRMark,
        rWW8Wrt.pFib->lcbSttbfRMark, rWW8Wrt.bWrtWW8 ? 0 : 2);
}

USHORT SwWW8Writer::AddRedlineAuthor( USHORT nId )
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

void SwWW8Writer::WriteAsStringTable(const std::vector<String>& rStrings,
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

void SwWW8Writer::WriteStringAsPara( const String& rTxt, USHORT nStyleId )
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
            SwWW8Writer::InsUInt16( aArr, 0x2416 );
        else
            aArr.Insert( 24, aArr.Count() );
        aArr.Insert( 1, aArr.Count() );
    }

    ULONG nPos = Strm().Tell();
    pPapPlc->AppendFkpEntry( nPos, aArr.Count(), aArr.GetData() );
    pChpPlc->AppendFkpEntry( nPos );
}

void SwWW8Writer::WriteSpecialText( ULONG nStart, ULONG nEnd, BYTE nTTyp )
{
    BYTE nOldTyp = nTxtTyp;
    nTxtTyp = nTTyp;
    SwPaM* pOldPam = pCurPam;       //!! Einfaches Umsetzen des PaM ohne
    SwPaM* pOldEnd = pOrigPam;          // Wiederherstellen muesste es auch tun
    bool bOldPageDescs = bOutPageDescs;
    bOutPageDescs = false;
                                    // bOutKF wird in WriteKF1 gemerkt / gesetzt
    pCurPam = NewSwPaM( *pDoc, nStart, nEnd );

    // Tabelle in Sonderbereichen erkennen
    if(    (nStart != pCurPam->GetMark()->nNode.GetIndex())
        && pDoc->GetNodes()[ nStart ]->IsTableNode() )
        pCurPam->GetMark()->nNode = nStart;

    pOrigPam = pCurPam;
    pCurPam->Exchange();

    WriteText();

    bOutPageDescs = bOldPageDescs;
    delete pCurPam;                    // Pam wieder loeschen
    pCurPam = pOldPam;
    pOrigPam = pOldEnd;
    nTxtTyp = nOldTyp;
}

void SwWW8Writer::OutSwString(const String& rStr, xub_StrLen nStt,
    xub_StrLen nLen, bool bUnicode, rtl_TextEncoding eChrSet)

{
    if( nLen )
    {
        if ( bUnicode != pPiece->IsUnicode() )
            pPiece->AppendPc ( Strm().Tell(), bUnicode );

        if( nStt || nLen != rStr.Len() )
        {
            String sOut( rStr.Copy( nStt, nLen ) );
            if (bUnicode)
                SwWW8Writer::WriteString16(Strm(), sOut, false);
            else
                SwWW8Writer::WriteString8(Strm(), sOut, false, eChrSet);
        }
        else if (bUnicode)
            SwWW8Writer::WriteString16(Strm(), rStr, false);
        else
            SwWW8Writer::WriteString8(Strm(), rStr, false, eChrSet);
    }
}

void SwWW8Writer::WriteCR()
{
    WriteChar( '\015' );
    pPiece->SetParaBreak();
}

void SwWW8Writer::WriteChar( sal_Unicode c )
{
    if( pPiece->IsUnicode() )
        Strm() << c;
    else
        Strm() << (BYTE)c;
}

/*  */
//---------------------------------------------------------------------------
//       Hilfsroutinen fuer Flys
//---------------------------------------------------------------------------
// Struktur speichert die aktuellen Daten des Writers zwischen, um
// einen anderen Dokument-Teil auszugeben, wie z.B. Header/Footer
WW8SaveData::WW8SaveData( SwWW8Writer& rWriter, ULONG nStt, ULONG nEnd )
    : rWrt( rWriter ),
    pOldPam( rWrt.pCurPam ), pOldEnd( rWrt.GetEndPaM() ),
    pOldFlyFmt(rWrt.mpParentFrame), pOldPageDesc( rWrt.pAktPageDesc )

{
    pOldFlyOffset = rWrt.pFlyOffset;
    eOldAnchorType = rWrt.eNewAnchorType;
    if( rWrt.pO->Count() )
    {
        pOOld = rWrt.pO;
        rWrt.pO = new WW8Bytes( 128, 128 );
    }
    else
        pOOld = 0;
    bOldWriteAll = rWrt.bWriteAll;
    bOldOutTable = rWrt.bOutTable;
    bOldIsInTable= rWrt.bIsInTable;
    bOldFlyFrmAttrs = rWrt.bOutFlyFrmAttrs;
    bOldStartTOX = rWrt.bStartTOX;
    bOldInWriteTOX = rWrt.bInWriteTOX;

    rWrt.pCurPam = rWrt.NewSwPaM( *rWrt.pDoc, nStt, nEnd );

    // Tabelle in Sonderbereichen erkennen
    if( nStt != rWrt.pCurPam->GetMark()->nNode.GetIndex() &&
        rWrt.pDoc->GetNodes()[ nStt ]->IsTableNode() )
        rWrt.pCurPam->GetMark()->nNode = nStt;

    rWrt.SetEndPaM( rWrt.pCurPam );
    rWrt.pCurPam->Exchange( );
    rWrt.bWriteAll = true;
    rWrt.bOutTable = false;
    // Vorsicht:  rWrt.bIsInTable  darf hier NICHT veraendert werden!
    rWrt.bOutFlyFrmAttrs = false;
//  rWrt.pAttrSet = 0;
    rWrt.bStartTOX = false;
    rWrt.bInWriteTOX = false;
}


WW8SaveData::~WW8SaveData()
{
    delete rWrt.pCurPam;                    // Pam wieder loeschen
    rWrt.pCurPam = pOldPam;
    rWrt.SetEndPaM( pOldEnd );
    rWrt.bWriteAll = bOldWriteAll;
    rWrt.bOutTable = bOldOutTable;
    rWrt.bIsInTable= bOldIsInTable;
    rWrt.bOutFlyFrmAttrs = bOldFlyFrmAttrs;
    rWrt.bStartTOX = bOldStartTOX;
    rWrt.bInWriteTOX = bOldInWriteTOX;
    rWrt.mpParentFrame = pOldFlyFmt;
    rWrt.pAktPageDesc = pOldPageDesc;
    ASSERT( !rWrt.pO->Count(), " pO ist am Ende von WW8SaveData nicht leer" );
    if( pOOld )
    {
        delete rWrt.pO;
        rWrt.pO = pOOld;
    }
    rWrt.eNewAnchorType = eOldAnchorType;
    rWrt.pFlyOffset = pOldFlyOffset;
}


void SwWW8Writer::WriteText()
{
    while( pCurPam->GetPoint()->nNode < pCurPam->GetMark()->nNode ||
            (pCurPam->GetPoint()->nNode == pCurPam->GetMark()->nNode &&
            pCurPam->GetPoint()->nContent.GetIndex() <= pCurPam->GetMark()->nContent.GetIndex()) )
    {
        SwNode * pNd = pCurPam->GetNode();

        if (pNd->IsTxtNode()) // blitzschnelle Methode
        {
            const SwTxtNode* pTxtNode = pNd->GetTxtNode();
            Out_SfxBreakItems(pTxtNode->GetpSwAttrSet(), *pTxtNode);
            // all textframes anchored as character for the winword 7- format
            if (!bWrtWW8 && !bIsInTable)
                OutWW8FlyFrmsInCntnt(*pTxtNode);
        }

        if( pNd->IsCntntNode() )
        {
            SwCntntNode* pCNd = (SwCntntNode*)pNd;

            const SwPageDesc* pTemp =
                pCNd->GetSwAttrSet().GetPageDesc().GetPageDesc();
            if (pTemp)
                pAktPageDesc = pTemp;

            pCurPam->GetPoint()->nContent.Assign( pCNd, 0 );
            Out( aWW8NodeFnTab, *pCNd, *this );
        }
        else if( pNd->IsTableNode() && !bIsInTable )
            OutWW8_SwTblNode( *this, *(SwTableNode*)pNd );
        else if( pNd->IsSectionNode() && TXT_MAINTEXT == nTxtTyp )
            OutWW8_SwSectionNode( *this, *pNd->GetSectionNode() );
        else if( TXT_MAINTEXT == nTxtTyp && pNd->IsEndNode() &&
                 pNd->StartOfSectionNode()->IsSectionNode() )
        {
            const SwSection& rSect = pNd->StartOfSectionNode()->GetSectionNode()
                                        ->GetSection();
            if( bStartTOX && TOX_CONTENT_SECTION == rSect.GetType() )
                bStartTOX = false;

            SwNodeIndex aIdx( *pNd, 1 );
            if(    aIdx.GetNode().IsEndNode()
                && aIdx.GetNode().StartOfSectionNode()->IsSectionNode() )
                ;
            else
            if(    aIdx.GetNode().IsSectionNode() )
                ;
            else if (!bIsInTable) //No sections in table
            {
                ReplaceCr( (char)0xc ); // Indikator fuer Page/Section-Break

                const SwSectionFmt* pParentFmt = rSect.GetFmt()->GetParent();
                if( !pParentFmt )
                    pParentFmt = (SwSectionFmt*)0xFFFFFFFF;

                ULONG nRstLnNum;
                if( aIdx.GetNode().IsCntntNode() )
                    nRstLnNum = ((SwCntntNode&)aIdx.GetNode()).GetSwAttrSet().
                                            GetLineNumber().GetStartValue();
                else
                    nRstLnNum = 0;

                pSepx->AppendSep( Fc2Cp( Strm().Tell() ),
                                    pAktPageDesc, pParentFmt, nRstLnNum );
            }
        }
        else if( pNd == &pNd->GetNodes().GetEndOfContent() )
            break;

        ULONG nPos = pCurPam->GetPoint()->nNode++;  // Bewegen
        ::SetProgressState( nPos, pCurPam->GetDoc()->GetDocShell() );   // Wie weit ?
    }
}

void SwWW8Writer::WriteMainText()
{
    pFib->fcMin = Strm().Tell();

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
}

void SwWW8Writer::WriteFkpPlcUsw()
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
        pStyles->OutStyleTab();                 // Styles
        pFtn->WritePlc( *this );                // Footnote-Ref & Text Plc
        pEdn->WritePlc( *this );                // Endnote-Ref & Text Plc
        pAtn->WritePlc( *this );                // Annotation-Ref & Text Plc
        pSepx->WritePlcSed( *this );            // Slcx.PlcSed
        pSepx->WritePlcHdd( *this );            // Slcx.PlcHdd
        pChpPlc->WritePlc();                    // Plcx.Chpx
        pPapPlc->WritePlc();                    // Plcx.Papx
        maFontHelper.WriteFontTable(pTableStrm, *pFib); // FFNs
        if( pRedlAuthors )
            pRedlAuthors->Write( *this );       // sttbfRMark (RedlineAuthors)
        pFldMain->Write( *this );               // Fields ( Main Text )
        pFldHdFt->Write( *this );               // Fields ( Header/Footer )
        pFldFtn->Write( *this );                // Fields ( FootNotes )
        pFldEdn->Write( *this );                // Fields ( EndNotes )
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
        pStyles->OutStyleTab();                 // fuer WW8 StyleTab
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
            pRedlAuthors->Write( *this );       // sttbfRMark (RedlineAuthors)
        pFldMain->Write( *this );               // Fields ( Main Text )
        pFldHdFt->Write( *this );               // Fields ( Header/Footer )
        pFldFtn->Write( *this );                // Fields ( FootNotes )
        pFldEdn->Write( *this );                // Fields ( EndNotes )
        pFldTxtBxs->Write( *this );             // Fields ( Textboxes )
        pFldHFTxtBxs->Write( *this );           // Fields ( Head/Foot-Textboxes )

        if (pEscher || pDoc->ContainsMSVBasic())
        {
            /*
             #82587# Everytime MS 2000 creates an escher stream there is always
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
            /*
            #10570# Similiarly having msvbasic storage seems to also trigger
            creating this stream
            */
            GetStorage().OpenSotStorage(CREATE_CONST_ASC(SL::aObjectPool),
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

        OutListTab();                           // listformats  - LSTF
        OutOverrideListTab();                   //   - "" -     - LFO
        OutListNamesTab();                      //   - "" -     - ListNames

        RestoreMacroCmds();

        pMagicTable->Write( *this );

        pPiece->WritePc( *this );               // Piece-Table
        maFontHelper.WriteFontTable(pTableStrm, *pFib); // FFNs

        //Convert OOo asian typography into MS typography structure
        ExportDopTypography(pDop->doptypography);

        WriteDop( *this );                      // Document-Properties
    }
    Strm().Seek( 0 );

    pFib->Write( Strm() );  // FIB
}

void SwWW8Writer::StoreDoc1()
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

void SwWW8Writer::AddLinkTarget(const String& rURL)
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
            aPair aImplicitBookmark;
            aImplicitBookmark.first = aOutline;
            aImplicitBookmark.second = nIdx;
            maImplicitBookmarks.push_back(aImplicitBookmark);
        }
    }
}

void SwWW8Writer::AddBookmark(String sBkmkName)
{
    pBkmks->Append(Fc2Cp(Strm().Tell()), BookmarkToWord(sBkmkName));
}

void SwWW8Writer::CollectOutlineBookmarks(const SwDoc &rDoc)
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

ULONG SwWW8Writer::StoreDoc()
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
    eNewAnchorType = FLY_PAGE;
    nTxtTyp = TXT_MAINTEXT;
    // --> OD 2007-04-19 #i43447# - removed
//    nFlyWidth = nFlyHeight = 0;
    // <--
    nStyleBeforeFly = nLastFmtId = 0;
    pStyAttr = 0;
    pOutFmtNode = 0;
    pEscher = 0;
    pRedlAuthors = 0;
    if( aTOXArr.Count() )
        aTOXArr.Remove( 0, aTOXArr.Count() );

    if( !pOLEExp )
    {
        UINT32 nSvxMSDffOLEConvFlags = 0;
        const SvtFilterOptions* pOpt = SvtFilterOptions::Get();
        if( pOpt->IsMath2MathType() )
            nSvxMSDffOLEConvFlags |= OLE_STARMATH_2_MATHTYPE;
        if( pOpt->IsWriter2WinWord() )
            nSvxMSDffOLEConvFlags |= OLE_STARWRITER_2_WINWORD;
        if( pOpt->IsCalc2Excel() )
            nSvxMSDffOLEConvFlags |= OLE_STARCALC_2_EXCEL;
        if( pOpt->IsImpress2PowerPoint() )
            nSvxMSDffOLEConvFlags |= OLE_STARIMPRESS_2_POWERPOINT;

        pOLEExp = new SvxMSExportOLEObjects( nSvxMSDffOLEConvFlags );
    }

    if( !pOleMap)
        pOleMap = new WW8OleMaps;

    if( !pOCXExp )
        pOCXExp = new SwMSConvertControls(pDoc->GetDocShell(),pCurPam);

    PrepareStorage();

    // --> OD 2007-10-08 #i81405#
    // Collect anchored objects before changing the redline mode.
    if (bWrtWW8)
        maFrames = GetAllFrames(*pDoc, bWriteAll ? 0 : pOrigPam);
    else
        maFrames = GetNonDrawingFrames(*pDoc, bWriteAll ? 0 : pOrigPam);
    // <--

    USHORT nRedlineMode = pDoc->GetRedlineMode();
    if (pDoc->GetRedlineTbl().Count())
    {
        pDoc->SetRedlineMode((RedlineMode_t)(nRedlineMode | nsRedlineMode_t::REDLINE_SHOW_DELETE |
                                     nsRedlineMode_t::REDLINE_SHOW_INSERT));
    }

    maFontHelper.InitFontTable(bWrtWW8, *pDoc);
    GatherChapterFields();

    pFib = new WW8Fib( bWrtWW8 ? 8 : 6 );

    SvStream* pOldStrm = pStrm;         // JP 19.05.99: wozu das ???
    SvStorageStreamRef xWwStrm( pStg->OpenSotStream( aMainStg ) );
    SvStorageStreamRef xTableStrm( xWwStrm ), xDataStrm( xWwStrm );
    xWwStrm->SetBufferSize( 32768 );

    if( bWrtWW8 )
    {
        pFib->fWhichTblStm = 1;
        xTableStrm = pStg->OpenSotStream(CREATE_CONST_ASC(SL::a1Table),
            STREAM_STD_WRITE );
        xDataStrm = pStg->OpenSotStream(CREATE_CONST_ASC(SL::aData),
            STREAM_STD_WRITE );

        xDataStrm->SetBufferSize( 32768 );  // fuer Grafiken
        xTableStrm->SetBufferSize( 16384 ); // fuer die Font-/Style-Table, usw.

        xTableStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        xDataStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    }

    pStrm = &xWwStrm;
    pTableStrm = &xTableStrm;
    pDataStrm = &xDataStrm;

    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    const SwSectionFmt *pFmt=0;
    // Default: "Standard"
    pAktPageDesc = &const_cast<const SwDoc *>(pDoc)->GetPageDesc( 0 );
    pSepx = new WW8_WrPlcSepx;      // Sepx
    {
        const SfxPoolItem* pI;
        const SwNode* pNd = pCurPam->GetCntntNode();
        const SfxItemSet* pSet = pNd ? &((SwCntntNode*)pNd)->GetSwAttrSet()
                                     : 0;

        ULONG nRstLnNum =  pSet ? ((SwFmtLineNumber&)pSet->Get(
                                            RES_LINENUMBER )).GetStartValue()
                                : 0;

        const SwTableNode* pTblNd = pCurPam->GetNode()->FindTableNode();
        const SwSectionNode* pSectNd;
        if( pTblNd )
        {
            pSet = &pTblNd->GetTable().GetFrmFmt()->GetAttrSet();
            pNd = pTblNd;
        }
        else if( 0 != ( pSectNd = pNd->FindSectionNode() ) )
        {
            if( TOX_HEADER_SECTION == pSectNd->GetSection().GetType() &&
                pSectNd->StartOfSectionNode()->IsSectionNode() )
                pSectNd = pSectNd->StartOfSectionNode()->GetSectionNode();

            if( TOX_CONTENT_SECTION == pSectNd->GetSection().GetType() )
            {
                pNd = pSectNd;
                pCurPam->GetPoint()->nNode = *pNd;
            }

            if (CONTENT_SECTION == pSectNd->GetSection().GetType())
                pFmt = pSectNd->GetSection().GetFmt();
        }

        // Hole evtl. Pagedesc des 1. Nodes
        if( pSet && SFX_ITEM_ON == pSet->GetItemState(RES_PAGEDESC, true, &pI)
            && ((SwFmtPageDesc*)pI)->GetPageDesc() )
            pSepx->AppendSep( 0, *(SwFmtPageDesc*)pI, *pNd, pFmt, nRstLnNum );
        else
            pSepx->AppendSep( 0, pAktPageDesc, pFmt, nRstLnNum );
    }

    pFtn = new WW8_WrPlcFtnEdn( TXT_FTN );                      // Ftn
    pEdn = new WW8_WrPlcFtnEdn( TXT_EDN );                      // Edn
    pAtn = new WW8_WrPlcPostIt;                                 // PostIts
    pTxtBxs = new WW8_WrPlcTxtBoxes( TXT_TXTBOX );
    pHFTxtBxs = new WW8_WrPlcTxtBoxes( TXT_HFTXTBOX );

    pSdrObjs = new MainTxtPlcDrawObj;   // Draw-/Fly-Objects for main text
    pHFSdrObjs = new HdFtPlcDrawObj;    // Draw-/Fly-Objects for header/footer

    pBkmks = new WW8_WrtBookmarks;                          // Bookmarks
    CreateBookmarkTbl();

    pPapPlc = new WW8_WrPlcPn( *this, PAP, pFib->fcMin );
    pChpPlc = new WW8_WrPlcPn( *this, CHP, pFib->fcMin );
    pO = new WW8Bytes( 128, 128 );
    pStyles = new WW8WrtStyle( *this );
    pFldMain = new WW8_WrPlcFld( 2, TXT_MAINTEXT );
    pFldHdFt = new WW8_WrPlcFld( 2, TXT_HDFT );
    pFldFtn = new WW8_WrPlcFld( 2, TXT_FTN );
    pFldEdn = new WW8_WrPlcFld( 2, TXT_EDN );
    pFldTxtBxs = new WW8_WrPlcFld( 2, TXT_TXTBOX );
    pFldHFTxtBxs = new WW8_WrPlcFld( 2, TXT_HFTXTBOX );

    pMagicTable = new WW8_WrMagicTable;

    pGrf = new SwWW8WrGrf( *this );
    pPiece = new WW8_WrPct( pFib->fcMin, bWrtWW8 );
    pDop = new WW8Dop;


    pDop->fRevMarking = 0 != (nsRedlineMode_t::REDLINE_ON & nRedlineMode);
    pDop->fRMView = 0 != ( nsRedlineMode_t::REDLINE_SHOW_DELETE & nRedlineMode );
    pDop->fRMPrint = pDop->fRMView;

    CollectOutlineBookmarks(*pDoc);

    // set AutoHyphenation flag if found in default para style
    const SfxPoolItem* pItem;
    SwTxtFmtColl* pStdTxtFmtColl =
        pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false);
    if (pStdTxtFmtColl && SFX_ITEM_SET == pStdTxtFmtColl->GetItemState(
        RES_PARATR_HYPHENZONE, false, &pItem))
    {
        pDop->fAutoHyphen = ((const SvxHyphenZoneItem*)pItem)->IsHyphen();
    }

    // make unique OrdNums (Z-Order) for all drawing-/fly Objects
    if (pDoc->GetDrawModel())
        pDoc->GetDrawModel()->GetPage( 0 )->RecalcObjOrdNums();

    StoreDoc1();

    if (nRedlineMode != pDoc->GetRedlineMode())
      pDoc->SetRedlineMode((RedlineMode_t)(nRedlineMode));

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
    pStrm = pOldStrm;


    xWwStrm->SetBufferSize( 0 );
    if( bWrtWW8 )
    {
        xTableStrm->SetBufferSize( 0 );
        xDataStrm->SetBufferSize( 0 );
        if( 0 == pDataStrm->Seek( STREAM_SEEK_TO_END ))
        {
            xDataStrm.Clear();
            pDataStrm = 0;
            pStg->Remove(CREATE_CONST_ASC(SL::aData));
        }
    }

    return 0;
}


void SwWW8Writer::PrepareStorage()
{
    ULONG nLen;
    const BYTE* pData;
    const char* pName;
    UINT32 nId1;
    const SwDocShell* pDocShell;
    SfxDocumentInfo* pInfo;


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
    pStg->SetClass( aGName, 0, String::CreateFromAscii( pName ));
    SvStorageStreamRef xStor( pStg->OpenSotStream(sCompObj) );
    xStor->Write( pData, nLen );

    pInfo = pDoc->GetDocumentInfo ();
    pDocShell = pDoc->GetDocShell ();

    GDIMetaFile *pMetaFile=0;
    if (pDocShell)
        pMetaFile = pDocShell->GetPreviewMetaFile (sal_False);

    pInfo->SavePropertySet( pStg, pMetaFile );   // DocInfo
    delete pMetaFile;
}

ULONG SwWW8Writer::WriteStorage()
{
    // #i34818# #120099# - update layout (if present), for SwWriteTable
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

    ULONG nRet = StoreDoc();

    ::EndProgress( pDoc->GetDocShell() );
    bWrtWW8 = false;        // sicherheitshalber: Default fuer's naechste Mal
    return nRet;
}

ULONG SwWW8Writer::WriteMedium( SfxMedium& )
{
    return WriteStorage();
}

SwWW8Writer::SwWW8Writer(const String& rFltName, const String& rBaseURL)
    : aMainStg(sMainStream), pISet(0), pUsedNumTbl(0), mpTopNodeOfHdFtPage(0),
    pBmpPal(0), pKeyMap(0), pOLEExp(0), pOCXExp(0), pOleMap(0), nUniqueList(0),
    mnHdFtIndex(0), pAktPageDesc(0), pPapPlc(0), pChpPlc(0), pChpIter(0), pO(0),
    bHasHdr(false), bHasFtr(false)
{
    SetBaseURL( rBaseURL );
    bWrtWW8 = rFltName.EqualsAscii(FILTER_WW8);
}

SwWW8Writer::~SwWW8Writer()
{
    delete pBmpPal;
    if (pKeyMap)
        delete[] (NfKeywordTable*)pKeyMap;
    delete pOLEExp;
    delete pOCXExp;
    delete pOleMap;
}

void GetWW8Writer( const String& rFltName, const String& rBaseURL, WriterRef& xRet )
{
    xRet = new SwWW8Writer( rFltName, rBaseURL );
}

bool WW8_WrPlcFtnEdn::WriteTxt( SwWW8Writer& rWrt )
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

void WW8_WrPlcFtnEdn::WritePlc( SwWW8Writer& rWrt ) const
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


bool WW8_WrPlcPostIt::WriteTxt(SwWW8Writer& rWrt)
{
    return WriteGenericTxt( rWrt, TXT_ATN, rWrt.pFib->ccpAtn );
}

void WW8_WrPlcPostIt::WritePlc( SwWW8Writer& rWrt ) const
{
    WriteGenericPlc( rWrt, TXT_ATN, rWrt.pFib->fcPlcfandTxt,
        rWrt.pFib->lcbPlcfandTxt, rWrt.pFib->fcPlcfandRef,
        rWrt.pFib->lcbPlcfandRef );
}

void WW8_WrPlcTxtBoxes::WritePlc( SwWW8Writer& rWrt ) const
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

void SwWW8Writer::RestoreMacroCmds()
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


/* vi:set tabstop=4 shiftwidth=4 expandtab: */
