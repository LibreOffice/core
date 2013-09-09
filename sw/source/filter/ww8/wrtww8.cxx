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


#include <iostream>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <unotools/ucbstreamhelper.hxx>

#include <algorithm>

#include <hintids.hxx>
#include <string.h>             // memcpy()
#include <osl/endian.h>
#include <docsh.hxx>

#include <unotools/fltrcfg.hxx>
#include <vcl/salbtype.hxx>
#include <sot/storage.hxx>
#include <svl/zformat.hxx>
#include <sfx2/docinf.hxx>
#include <editeng/tstpitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/langitem.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <swtypes.hxx>
#include <swrect.hxx>
#include <swtblfmt.hxx>
#include <txatbase.hxx>
#include <fmtcntnt.hxx>
#include <fmtpdsc.hxx>
#include <fmtrowsplt.hxx>
#include <frmatr.hxx>
#include <doc.hxx>
#include <viewopt.hxx>
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
#include <statstr.hrc>          // ResId for the status bar
#include <fmtline.hxx>
#include <fmtfsize.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/stlunosequence.hxx>
#include <comphelper/string.hxx>
#include <doctok/sprmids.hxx>

#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "ww8attributeoutput.hxx"

#include <IDocumentMarkAccess.hxx>
#include <xmloff/odffields.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include "dbgoutsw.hxx"

#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <svl/stritem.hxx>
#include <unotools/tempfile.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <osl/time.h>
#include <rtl/random.h>
#include <vcl/svapp.hxx>
#include "WW8Sttbf.hxx"
#include <editeng/charrotateitem.hxx>
#include "WW8FibData.hxx"
#include "numrule.hxx"//For i120928

using namespace css;
using namespace sw::util;
using namespace sw::types;

/** FKP - Formatted disK Page
*/
class WW8_WrFkp
{
    sal_uInt8* pFkp;         // Fkp total ( first and only FCs and Sprms )
    sal_uInt8* pOfs;         // pointer to the offset area, later copied to pFkp
    ePLCFT ePlc;
    short nStartGrp;    // ab hier grpprls
    short nOldStartGrp;
    sal_uInt8 nItemSize;
    sal_uInt8 nIMax;         // number of entry pairs
    sal_uInt8 nOldVarLen;
    bool bCombined;     // true : paste not allowed

    sal_uInt8 SearchSameSprm( sal_uInt16 nVarLen, const sal_uInt8* pSprms );
public:
    WW8_WrFkp(ePLCFT ePl, WW8_FC nStartFc, bool bWrtWW8);
    ~WW8_WrFkp();
    bool Append( WW8_FC nEndFc, sal_uInt16 nVarLen = 0, const sal_uInt8* pSprms = 0 );
    bool Combine();
    void Write( SvStream& rStrm, SwWW8WrGrf& rGrf );

    bool IsEqualPos(WW8_FC nEndFc) const
    {   return !bCombined && nIMax && nEndFc == ((sal_Int32*)pFkp)[nIMax]; }
    void MergeToNew( short& rVarLen, sal_uInt8 *& pNewSprms );
    bool IsEmptySprm() const
    {   return !bCombined && nIMax && !nOldVarLen;  }
    void SetNewEnd( WW8_FC nEnd )
    {   ((sal_Int32*)pFkp)[nIMax] = nEnd; }

#ifdef __WW8_NEEDS_COPY
    WW8_FC GetStartFc() const;
    WW8_FC GetEndFc() const;
#else
    WW8_FC GetStartFc() const { return ((sal_Int32*)pFkp)[0]; };
    WW8_FC GetEndFc() const { return ((sal_Int32*)pFkp)[nIMax]; };
#endif // defined __WW8_NEEDS_COPY

    sal_uInt8 *CopyLastSprms(sal_uInt8 &rLen, bool bVer8);
};


// class WW8_WrPc collects all piece entries for one piece
class WW8_WrPc
{
    WW8_CP nStartCp;                    // Starting character position of the text
    WW8_FC nStartFc;                    // Starting file position of the text
    sal_uInt16 nStatus;                     // End of paragraph inside the piece?

public:
    WW8_WrPc(WW8_FC nSFc, WW8_CP nSCp )
        : nStartCp( nSCp ), nStartFc( nSFc ), nStatus( 0x0040 )
    {}

    void SetStatus()                { nStatus = 0x0050; }
    sal_uInt16 GetStatus()  const       { return nStatus; }
    WW8_CP GetStartCp() const       { return nStartCp; }
    WW8_FC GetStartFc() const       { return nStartFc; }
};

class WW8_WrtBookmarks
{
private:
    //! Holds information about a single bookmark.
    struct BookmarkInfo {
        sal_uLong  startPos; //!< Starting character position.
        sal_uLong  endPos;   //!< Ending character position.
        bool   isField;  //!< True if the bookmark is in a field result.
        String name;     //!< Name of this bookmark.
        inline BookmarkInfo(sal_uLong start, sal_uLong end, bool isFld, const String& bkName) : startPos(start), endPos(end), isField(isFld), name(bkName) {};
        //! Operator < is defined purely for sorting.
        inline bool operator<(const BookmarkInfo &other) const { return startPos < other.startPos; }
    };
    std::vector<BookmarkInfo> aBookmarks;
    typedef std::vector<BookmarkInfo>::iterator BkmIter;

    //! Return the position in aBookmarks where the string rNm can be found.
    BkmIter GetPos( const String& rNm );

    //No copying
    WW8_WrtBookmarks(const WW8_WrtBookmarks&);
    WW8_WrtBookmarks& operator=(const WW8_WrtBookmarks&);
public:
    WW8_WrtBookmarks();
    ~WW8_WrtBookmarks();

    //! Add a new bookmark to the list OR add an end position to an existing bookmark.
    void Append( WW8_CP nStartCp, const String& rNm, const ::sw::mark::IMark* pBkmk=NULL );
    //! Write out bookmarks to file.
    void Write( WW8Export& rWrt );
    //! Move existing field marks from one position to another.
    void MoveFieldMarks(sal_uLong nFrom,sal_uLong nTo);

};

#define ANZ_DEFAULT_STYLES 16

// Names of the storage streams
#define sMainStream OUString("WordDocument")
#define sCompObj OUString("\1CompObj")

static void WriteDop( WW8Export& rWrt )
{
    WW8Dop& rDop = *rWrt.pDop;

    // i#78951#, store the value of unknown compatability options
    rDop.SetCompatabilityOptions( rWrt.pDoc->Getn32DummyCompatabilityOptions1());
    rDop.SetCompatabilityOptions2( rWrt.pDoc->Getn32DummyCompatabilityOptions2());

    rDop.fNoLeading = !rWrt.pDoc->get(IDocumentSettingAccess::ADD_EXT_LEADING);
    rDop.fUsePrinterMetrics = !rWrt.pDoc->get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE);

    // write default TabStop
    const SvxTabStopItem& rTabStop =
        DefaultItemGet<SvxTabStopItem>(*rWrt.pDoc, RES_PARATR_TABSTOP);
    rDop.dxaTab = (sal_uInt16)rTabStop[0].GetTabPos();

    // Zoom factor
    ViewShell *pViewShell(rWrt.pDoc->GetCurrentViewShell());
    if (pViewShell && pViewShell->GetViewOptions()->GetZoomType() == SVX_ZOOM_PERCENT)
        rDop.wScaleSaved = pViewShell->GetViewOptions()->GetZoom();

    // Werte aus der DocStatistik (werden aufjedenfall fuer die
    // DocStat-Felder benoetigt!)
    rDop.fWCFtnEdn = true; // because they are included in StarWriter

    const SwDocStat& rDStat = rWrt.pDoc->GetDocStat();
    rDop.cWords = rDStat.nWord;
    rDop.cCh = rDStat.nChar;
    rDop.cPg = static_cast< sal_Int16 >(rDStat.nPage);
    rDop.cParas = rDStat.nPara;
    rDop.cLines = rDStat.nPara;

    SwDocShell *pDocShell(rWrt.pDoc->GetDocShell());
    OSL_ENSURE(pDocShell, "no SwDocShell");
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
        OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");

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

    if (!xDocProps.is())
    {
        rDop.dttmCreated = rDop.dttmRevised = rDop.dttmLastPrint = 0x45FBAC69;
    }
    else
    {
        ::util::DateTime uDT = xDocProps->getCreationDate();
        Date aD(uDT.Day, uDT.Month, uDT.Year);
        Time aT(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.NanoSeconds);
        rDop.dttmCreated = sw::ms::DateTime2DTTM(DateTime(aD,aT));
        uDT = xDocProps->getModificationDate();
        Date aD2(uDT.Day, uDT.Month, uDT.Year);
        Time aT2(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.NanoSeconds);
        rDop.dttmRevised = sw::ms::DateTime2DTTM(DateTime(aD2,aT2));
        uDT = xDocProps->getPrintDate();
        Date aD3(uDT.Day, uDT.Month, uDT.Year);
        Time aT3(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.NanoSeconds);
        rDop.dttmLastPrint = sw::ms::DateTime2DTTM(DateTime(aD3,aT3));
    }

    // Also, the DocStat fields in headers, footers are not calculated correctly.
    // ( we do not have this fields! )

    // and also for the Headers and Footers
    rDop.cWordsFtnEnd   = rDStat.nWord;
    rDop.cChFtnEdn      = rDStat.nChar;
    rDop.cPgFtnEdn      = (sal_Int16)rDStat.nPage;
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

static int lcl_CmpBeginEndChars( const OUString& rSWStr,
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
    sal_uInt8 nUseReserved=0;
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

// It can only be found something with this method, if it is used within
// WW8_SwAttrIter::OutAttr() and WW8Export::OutputItemSet()
const SfxPoolItem* MSWordExportBase::HasItem( sal_uInt16 nWhich ) const
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
        OSL_ENSURE( !this, "Where is my ItemSet / pChpIter ?" );
        pItem = 0;
    }
    return pItem;
}

const SfxPoolItem& MSWordExportBase::GetItem(sal_uInt16 nWhich) const
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
        OSL_ENSURE( !this, "Where is my ItemSet / pChpIter ?" );
        pItem = 0;
    }
    return *pItem;
}

//------------------------------------------------------------------------------

WW8_WrPlc1::WW8_WrPlc1( sal_uInt16 nStructSz )
    : nStructSiz( nStructSz )
{
    nDataLen = 16 * nStructSz;
    pData = new sal_uInt8[ nDataLen ];
}

WW8_WrPlc1::~WW8_WrPlc1()
{
    delete[] pData;
}

WW8_CP WW8_WrPlc1::Prev() const
{
    bool b = !aPos.empty();
    OSL_ENSURE(b,"Prev called on empty list");
    return b ? aPos.back() : 0;
}

void WW8_WrPlc1::Append( WW8_CP nCp, const void* pNewData )
{
    sal_uLong nInsPos = aPos.size() * nStructSiz;
    aPos.push_back( nCp );
    if( nDataLen < nInsPos + nStructSiz )
    {
        sal_uInt8* pNew = new sal_uInt8[ 2 * nDataLen ];
        memcpy( pNew, pData, nDataLen );
        delete[] pData;
        pData = pNew;
        nDataLen *= 2;
    }
    memcpy( pData + nInsPos, pNewData, nStructSiz );
}

void WW8_WrPlc1::Finish( sal_uLong nLastCp, sal_uLong nSttCp )
{
    if( !aPos.empty() )
    {
        aPos.push_back( nLastCp );
        if( nSttCp )
            for( sal_uInt32 n = 0; n < aPos.size(); ++n )
                aPos[ n ] -= nSttCp;
    }
}


void WW8_WrPlc1::Write( SvStream& rStrm )
{
    sal_uInt32 i;
    for( i = 0; i < aPos.size(); ++i )
        SwWW8Writer::WriteLong( rStrm, aPos[i] );
    if( i )
        rStrm.Write( pData, (i-1) * nStructSiz );
}


// Class WW8_WrPlcFld for fields

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
        sal_uLong nFcStart = rWrt.pTableStrm->Tell();
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
    sal_uLong nFcStart = rWrt.pTableStrm->Tell();
    WW8_WrPlc1::Write( *rWrt.pTableStrm );
    rWrt.pFib->fcPlcfTch = nFcStart;
    rWrt.pFib->lcbPlcfTch = rWrt.pTableStrm->Tell() - nFcStart;
    return true;
}

void WW8_WrMagicTable::Append( WW8_CP nCp, sal_uLong nData)
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

void SwWW8Writer::FillCount( SvStream& rStrm, sal_uLong nCount )
{
    static const sal_uInt32 aNulls[16] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 // 64 Byte
    };

    while (nCount > 64)
    {
        rStrm.Write( aNulls, 64 );          // in steps of 64-Byte
        nCount -= 64;
    }
    rStrm.Write( aNulls, nCount );          // write the rest ( 0 .. 64 Bytes )
}

sal_uLong SwWW8Writer::FillUntil( SvStream& rStrm, sal_uLong nEndPos )
{
    sal_uLong nCurPos = rStrm.Tell();
    if( !nEndPos )                          // nEndPos == 0 -> next Page
        nEndPos = (nCurPos + 0x1ff) & ~0x1ffUL;

    if( nEndPos > nCurPos )
        SwWW8Writer::FillCount( rStrm, nEndPos - nCurPos );
#if OSL_DEBUG_LEVEL > 0
    else
        OSL_ENSURE( nEndPos == nCurPos, "Falsches FillUntil()" );
#endif
    return rStrm.Tell();
}


//--------------------------------------------------------------------------

WW8_WrPlcPn::WW8_WrPlcPn( WW8Export& rWr, ePLCFT ePl, WW8_FC nStartFc )
    : rWrt(rWr), nFkpStartPage(0), ePlc(ePl), nMark(0)
{
    WW8_WrFkp* pF = new WW8_WrFkp( ePlc, nStartFc, rWrt.bWrtWW8 );
    aFkps.push_back( pF );
}

WW8_WrPlcPn::~WW8_WrPlcPn()
{
}

sal_uInt8 *WW8_WrPlcPn::CopyLastSprms(sal_uInt8 &rLen)
{
    WW8_WrFkp& rF = aFkps.back();
    return rF.CopyLastSprms(rLen, rWrt.bWrtWW8);
}

void WW8_WrPlcPn::AppendFkpEntry(WW8_FC nEndFc,short nVarLen,const sal_uInt8* pSprms)
{
    WW8_WrFkp* pF = &aFkps.back();

    // big sprm? build the sprmPHugePapx
    sal_uInt8* pNewSprms = (sal_uInt8*)pSprms;
    sal_uInt8 aHugePapx[ 8 ];
    if( rWrt.bWrtWW8 && PAP == ePlc && 488 < nVarLen )
    {
        sal_uInt8* p = aHugePapx;
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
        pF = new WW8_WrFkp( ePlc, pF->GetEndFc(), rWrt.bWrtWW8 ); // Start new Fkp == end of old Fkp

        aFkps.push_back( pF );
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
    nFkpStartPage = (sal_uInt16) ( SwWW8Writer::FillUntil( rWrt.Strm() ) >> 9 );

    for( sal_uInt16 i = 0; i < aFkps.size(); i++ )
        aFkps[ i ].Write( rWrt.Strm(), *rWrt.pGrf );

    if( CHP == ePlc )
    {
        rWrt.pFib->pnChpFirst = nFkpStartPage;
        rWrt.pFib->cpnBteChp = aFkps.size();
    }
    else
    {
        rWrt.pFib->pnPapFirst = nFkpStartPage;
        rWrt.pFib->cpnBtePap = aFkps.size();
    }
}

void WW8_WrPlcPn::WritePlc()
{
    sal_uLong nFcStart = rWrt.pTableStrm->Tell();
    sal_uInt16 i;

    for( i = 0; i < aFkps.size(); i++ )
        SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                aFkps[ i ].GetStartFc() );

    SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                aFkps[ i - 1 ].GetEndFc() );

    // fuer jedes FKP die Page ausgeben
    if( rWrt.bWrtWW8)                   // for WW97 Long output
        for ( i = 0; i < aFkps.size(); i++)
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, i + nFkpStartPage );
    else                            // for WW95 Short output
        for ( i = 0; i < aFkps.size(); i++)
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

WW8_WrFkp::WW8_WrFkp(ePLCFT ePl, WW8_FC nStartFc, bool bWrtWW8)
    : ePlc(ePl), nStartGrp(511), nOldStartGrp(511),
    nItemSize( ( CHP == ePl ) ? 1 : ( bWrtWW8 ? 13 : 7 )),
    nIMax(0), nOldVarLen(0), bCombined(false)
{
    pFkp = (sal_uInt8*)new sal_Int32[128];           // 512 Byte
    pOfs = (sal_uInt8*)new sal_Int32[128];           // 512 Byte
    memset( pFkp, 0, 4 * 128 );
    memset( pOfs, 0, 4 * 128 );
    ( (sal_Int32*)pFkp )[0] = nStartFc;         // 0th entry FC at nStartFc
}

WW8_WrFkp::~WW8_WrFkp()
{
    delete[] (sal_Int32 *)pFkp;
    delete[] (sal_Int32 *)pOfs;
}

sal_uInt8 WW8_WrFkp::SearchSameSprm( sal_uInt16 nVarLen, const sal_uInt8* pSprms )
{
    if( 3 < nVarLen )
    {
        // if the sprms contained picture-references then never equal!
        for( sal_uInt8 n = static_cast< sal_uInt8 >(nVarLen - 1); 3 < n; --n )
            if( pSprms[ n ] == GRF_MAGIC_3 &&
                pSprms[ n-1 ] == GRF_MAGIC_2 &&
                pSprms[ n-2 ] == GRF_MAGIC_1 )
                    return 0;
    }

    short i;
    for( i = 0; i < nIMax; i++ )
    {
        sal_uInt8 nStart = pOfs[i * nItemSize];
        if( nStart )
        {                               // has Sprms
            const sal_uInt8* p = pFkp + ( (sal_uInt16)nStart << 1 );
            if( ( CHP == ePlc
                    ? (*p++ == nVarLen)
                    : (((sal_uInt16)*p++ << 1 ) == (( nVarLen+1) & 0xfffe)) )
                && !memcmp( p, pSprms, nVarLen ) )
                    return nStart;                      // found it
        }
    }
    return 0;           // didn't found it
}

sal_uInt8 *WW8_WrFkp::CopyLastSprms(sal_uInt8 &rLen, bool bVer8)
{
    rLen=0;
    sal_uInt8 *pStart=0,*pRet=0;

    if (!bCombined)
        pStart = pOfs;
    else
        pStart = pFkp + ( nIMax + 1 ) * 4;

    sal_uInt8 nStart = *(pStart + (nIMax-1) * nItemSize);

    const sal_uInt8* p = pFkp + ( (sal_uInt16)nStart << 1 );

    if (!*p && bVer8)
        p++;

    if (*p)
    {
        rLen = *p++;
        if (PAP == ePlc)
            rLen *= 2;
        pRet = new sal_uInt8[rLen];
        memcpy(pRet,p,rLen);
    }
    return pRet;
}

bool WW8_WrFkp::Append( WW8_FC nEndFc, sal_uInt16 nVarLen, const sal_uInt8* pSprms )
{
    OSL_ENSURE( !nVarLen || pSprms, "Item pointer missing" );
    OSL_ENSURE( nVarLen < ( ( ePlc == PAP ) ? 497U : 502U ), "Sprms too long !" );

    if( bCombined )
    {
        OSL_ENSURE( !this, "Fkp::Append: Fkp is already combined" );
        return false;
    }
    sal_Int32 n = ((sal_Int32*)pFkp)[nIMax];        // last entry
    if( nEndFc <= n )
    {
        OSL_ENSURE( nEndFc >= n, "+Fkp: FC backwards" );
        OSL_ENSURE( !nVarLen || !pSprms || nEndFc != n,
                                    "+Fkp: selber FC mehrfach benutzt" );
                        // selber FC ohne Sprm wird ohne zu mosern ignoriert.

        return true;    // ignore (do not create a new Fkp)
    }

    sal_uInt8 nOldP = ( nVarLen ) ? SearchSameSprm( nVarLen, pSprms ) : 0;
                                            // Combine equal entries
    short nOffset=0, nPos = nStartGrp;
    if (nVarLen && !nOldP)
    {
        nPos = PAP == ePlc
                ? ( 13 == nItemSize     // HACK: PAP and bWrtWW8 !!
                     ? (nStartGrp & 0xFFFE ) - nVarLen - 1
                     : (nStartGrp - (((nVarLen + 1) & 0xFFFE)+1)) & 0xFFFE )
                : ((nStartGrp - nVarLen - 1) & 0xFFFE);
        if( nPos < 0 )
            return false;           // doesn't fit at all
        nOffset = nPos;             // save offset (can also be uneven!)
        nPos &= 0xFFFE;             // Pos for Sprms ( gerade Pos )
    }

    if( (sal_uInt16)nPos <= ( nIMax + 2U ) * 4U + ( nIMax + 1U ) * nItemSize )
                                            // does it fits behind the CPs and offsets?
        return false;                       // no

    ((sal_Int32*)pFkp)[nIMax + 1] = nEndFc;     // insert FC

    nOldVarLen = (sal_uInt8)nVarLen;
    if( nVarLen && !nOldP )
    {               // insert it for real
        nOldStartGrp = nStartGrp;

        nStartGrp = nPos;
        pOfs[nIMax * nItemSize] = (sal_uInt8)( nStartGrp >> 1 );
                                            // ( DatenAnfg >> 1 ) insert
        sal_uInt8 nCnt = static_cast< sal_uInt8 >(CHP == ePlc
                        ? ( nVarLen < 256 ) ? (sal_uInt8) nVarLen : 255
                        : ( ( nVarLen + 1 ) >> 1 ));

        pFkp[ nOffset ] = nCnt;                     // Enter data length
        memcpy( pFkp + nOffset + 1, pSprms, nVarLen );  // store Sprms
    }
    else
    {
        // do not enter for real ( no Sprms or recurrence )
        // DatenAnfg 0 ( no data ) or recurrence
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
    ((sal_uInt8*)pFkp)[511] = nIMax;
    bCombined = true;

#if defined OSL_BIGENDIAN           // only the FCs will be rotated here
    sal_uInt16 i;                   // the Sprms must be rotated elsewhere

    sal_uInt32* p;
    for( i = 0, p = (sal_uInt32*)pFkp; i <= nIMax; i++, p++ )
        *p = OSL_SWAPDWORD( *p );
#endif // ifdef OSL_BIGENDIAN

    return true;
}

void WW8_WrFkp::Write( SvStream& rStrm, SwWW8WrGrf& rGrf )
{
    Combine();                      // If not already combined

    sal_uInt8* p;               //  Suche Magic fuer nPicLocFc
    sal_uInt8* pEnd = pFkp + nStartGrp;
    for( p = pFkp + 511 - 4; p >= pEnd; p-- )
    {
        if( *p != GRF_MAGIC_1 )     // search for signature 0x12 0x34 0x56 0xXX
            continue;
        if( *(p+1) != GRF_MAGIC_2 )
            continue;
        if( *(p+2) != GRF_MAGIC_3 )
            continue;

        SVBT32 nPos;                // signature found
        UInt32ToSVBT32( rGrf.GetFPos(), nPos );   // FilePos the graphics
        memcpy( p, nPos, 4 );       // patch FilePos over the signature
    }
    rStrm.Write( pFkp, 512 );
}

void WW8_WrFkp::MergeToNew( short& rVarLen, sal_uInt8 *& rpNewSprms )
{
    sal_uInt8 nStart = pOfs[ (nIMax-1) * nItemSize ];
    if( nStart )
    {   // has Sprms
        sal_uInt8* p = pFkp + ( (sal_uInt16)nStart << 1 );

        // old and new equal? Then copy only one into the new sprms
        if( nOldVarLen == rVarLen && !memcmp( p+1, rpNewSprms, nOldVarLen ))
        {
            sal_uInt8* pNew = new sal_uInt8[ nOldVarLen ];
            memcpy( pNew, p+1, nOldVarLen );
            rpNewSprms = pNew;
        }
        else
        {
            sal_uInt8* pNew = new sal_uInt8[ nOldVarLen + rVarLen ];
            memcpy( pNew, p+1, nOldVarLen );
            memcpy( pNew + nOldVarLen, rpNewSprms, rVarLen );

            rpNewSprms = pNew;
            rVarLen = rVarLen + nOldVarLen;
        }
        --nIMax;
        // if this sprms dont used from others, remove it
        bool bFnd = false;
        for (sal_uInt16 n = 0; n < nIMax; ++n)
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
    return ((sal_Int32*)pFkp)[0];
}

WW8_FC WW8_WrFkp::GetEndFc() const
{
    if( bCombined )
        return SVBT32ToUInt32( &(pFkp[nIMax*4]) );    // nIMax-tes SVBT32-Element
    return ((sal_Int32*)pFkp)[nIMax];
}

#endif // defined __WW8_NEEDS_COPY


// Method for managing the piece table
WW8_WrPct::WW8_WrPct(WW8_FC nfcMin, bool bSaveUniCode)
    : nOldFc(nfcMin), bIsUni(bSaveUniCode)
{
    AppendPc( nOldFc, bIsUni );
}

WW8_WrPct::~WW8_WrPct()
{
}

// Fill the piece and create a new one
void WW8_WrPct::AppendPc(WW8_FC nStartFc, bool bIsUnicode)
{
    WW8_CP nStartCp = nStartFc - nOldFc;    // substract the beginning of the text
    if ( !nStartCp )
    {
        if ( !aPcts.empty() )
        {
            OSL_ENSURE( 1 == aPcts.size(), "Leeres Piece !!");
            aPcts.pop_back( );
        }
    }

    nOldFc = nStartFc;                      // remember StartFc as old

    if( bIsUni )
        nStartCp >>= 1;                 // for Unicode: number of characters / 2


    if ( !bIsUnicode )
    {
        nStartFc <<= 1;                 // Adress * 2
        nStartFc |= 0x40000000;         // second last bit for non-Unicode
    }

    if( !aPcts.empty() )
        nStartCp += aPcts.back().GetStartCp();

    WW8_WrPc* pPc = new WW8_WrPc( nStartFc, nStartCp );
    aPcts.push_back( pPc );

    bIsUni = bIsUnicode;
}


void WW8_WrPct::WritePc( WW8Export& rWrt )
{
    sal_uLong nPctStart;
    sal_uLong nOldPos, nEndPos;
    boost::ptr_vector<WW8_WrPc>::iterator aIter;

    nPctStart = rWrt.pTableStrm->Tell();                    // Start piece table
    *rWrt.pTableStrm << ( char )0x02;                       // Status byte PCT
    nOldPos = nPctStart + 1;                                // remember Position
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );          // then the length

    for( aIter = aPcts.begin(); aIter != aPcts.end(); ++aIter )     // ranges
        SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                aIter->GetStartCp() );


    // calculate the last Pos
    sal_uLong nStartCp = rWrt.pFib->fcMac - nOldFc;
    if( bIsUni )
        nStartCp >>= 1;             // For Unicode: number of characters / 2
    nStartCp += aPcts.back().GetStartCp();
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nStartCp );

    // piece references
    for ( aIter = aPcts.begin(); aIter != aPcts.end(); ++aIter )
    {
        SwWW8Writer::WriteShort( *rWrt.pTableStrm, aIter->GetStatus());
        SwWW8Writer::WriteLong( *rWrt.pTableStrm, aIter->GetStartFc());
        SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0);          // PRM=0
    }

    // entries in the FIB
    rWrt.pFib->fcClx = nPctStart;
    nEndPos = rWrt.pTableStrm->Tell();
    rWrt.pFib->lcbClx = nEndPos - nPctStart;

    // and register the length as well
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nOldPos,
                            nEndPos - nPctStart-5 );

}

void WW8_WrPct::SetParaBreak()
{
    OSL_ENSURE( !aPcts.empty(),"SetParaBreak : aPcts.empty()" );
    aPcts.back().SetStatus();
}

WW8_CP WW8_WrPct::Fc2Cp( sal_uLong nFc ) const
{
    OSL_ENSURE( nFc >= (sal_uLong)nOldFc, "FilePos lies in front of last piece" );
    OSL_ENSURE( ! aPcts.empty(), "Fc2Cp no piece available" );

    nFc -= nOldFc;
    if( bIsUni )
        nFc /= 2;
    return nFc + aPcts.back().GetStartCp();
}

//--------------------------------------------------------------------------

WW8_WrtBookmarks::WW8_WrtBookmarks()
{
}

WW8_WrtBookmarks::~WW8_WrtBookmarks()
{
}

void WW8_WrtBookmarks::Append( WW8_CP nStartCp, const String& rNm,  const ::sw::mark::IMark* )
{
    BkmIter bkIter = GetPos( rNm );
    if( bkIter == aBookmarks.end() )
    {
        // new bookmark -> insert with start==end
        aBookmarks.push_back( BookmarkInfo(nStartCp, nStartCp, false, rNm) );
    }
    else
    {
        // old bookmark -> this should be the end position
        OSL_ENSURE( bkIter->endPos == bkIter->startPos, "end position is valid" );

        //If this bookmark was around a field in writer, then we want to move
        //it to the field result in word. The end is therefore one cp
        //backwards from the 0x15 end mark that was inserted.
        if (bkIter->isField)
            --nStartCp;
        bkIter->endPos = nStartCp;
    }
}


void WW8_WrtBookmarks::Write( WW8Export& rWrt )
{
    if (!aBookmarks.empty())
    {
        //Make sure the bookmarks are sorted in order of start position.
        std::sort(aBookmarks.begin(), aBookmarks.end());

        // First write the Bookmark Name Stringtable
        std::vector<OUString> aNames;
        aNames.reserve(aBookmarks.size());
        for (BkmIter bIt = aBookmarks.begin(); bIt < aBookmarks.end(); ++bIt)
            aNames.push_back(bIt->name);
        rWrt.WriteAsStringTable(aNames, rWrt.pFib->fcSttbfbkmk, rWrt.pFib->lcbSttbfbkmk);

        // Second write the Bookmark start positions as pcf of longs
        SvStream& rStrm = rWrt.bWrtWW8 ? *rWrt.pTableStrm : rWrt.Strm();
        rWrt.pFib->fcPlcfbkf = rStrm.Tell();
        for (BkmIter bIt = aBookmarks.begin(); bIt < aBookmarks.end(); ++bIt)
            SwWW8Writer::WriteLong( rStrm, bIt->startPos );
        SwWW8Writer::WriteLong(rStrm, rWrt.pFib->ccpText + rWrt.pFib->ccpTxbx);

        //Lastly, need to write out the end positions (sorted by end position). But
        //before that we need a lookup table (sorted by start position) to link
        //start and end positions.
        //   Start by sorting the end positions.
        std::vector<sal_uLong> aEndSortTab;
        aEndSortTab.reserve(aBookmarks.size());
        for (BkmIter bIt = aBookmarks.begin(); bIt < aBookmarks.end(); ++bIt)
            aEndSortTab.push_back(bIt->endPos);
        std::sort(aEndSortTab.begin(), aEndSortTab.end());

        //Now write out the lookups.
        //Note that in most cases, the positions in both vectors will be very close.
        for( sal_uLong i = 0; i < aBookmarks.size(); ++i )
        {
            sal_uLong nEndCP = aBookmarks[ i ].endPos;
            sal_uInt16 nPos = i;
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

        // Finally, the actual Bookmark end positions.
        rWrt.pFib->fcPlcfbkl = rStrm.Tell();
        for(sal_uLong i = 0; i < aEndSortTab.size(); ++i )
            SwWW8Writer::WriteLong( rStrm, aEndSortTab[ i ] );
        SwWW8Writer::WriteLong(rStrm, rWrt.pFib->ccpText + rWrt.pFib->ccpTxbx);
        rWrt.pFib->lcbPlcfbkl = rStrm.Tell() - rWrt.pFib->fcPlcfbkl;
    }
}

WW8_WrtBookmarks::BkmIter WW8_WrtBookmarks::GetPos( const String& rNm )
{
    for (BkmIter bIt = aBookmarks.begin(); bIt < aBookmarks.end(); ++bIt) {
        if (rNm == bIt->name)
            return bIt;
    }
    return aBookmarks.end();
}

void WW8_WrtBookmarks::MoveFieldMarks(sal_uLong nFrom, sal_uLong nTo)
{
    for (BkmIter i = aBookmarks.begin(); i < aBookmarks.end(); ++i)
    {
        if (i->startPos == nFrom)
        {
            i->startPos = nTo;
            if (i->endPos == nFrom)
            {
                i->isField = true;
                i->endPos = nTo;
            }
        }
    }
}

void WW8Export::AppendBookmarks( const SwTxtNode& rNd,
    xub_StrLen nAktPos, xub_StrLen nLen )
{
    std::vector< const ::sw::mark::IMark* > aArr;
    sal_uInt16 nCntnt;
    xub_StrLen nAktEnd = nAktPos + nLen;
    if( GetWriter().GetBookmarks( rNd, nAktPos, nAktEnd, aArr ))
    {
        sal_uLong nNd = rNd.GetIndex(), nSttCP = Fc2Cp( Strm().Tell() );
        for( sal_uInt16 n = 0; n < aArr.size(); ++n )
        {
            const ::sw::mark::IMark& rBkmk = *(aArr[ n ]);
            if(dynamic_cast< const ::sw::mark::IFieldmark *>(&rBkmk))
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
                sal_uLong nCp = nSttCP + pPos->nContent.GetIndex() - nAktPos;
                pBkmks->Append(nCp, BookmarkToWord(rBkmk.GetName()), &rBkmk);
            }
            if( pOPos && nNd == pOPos->nNode.GetIndex() &&
                ( nCntnt = pOPos->nContent.GetIndex() ) >= nAktPos &&
                nCntnt < nAktEnd )
            {
                sal_uLong nCp = nSttCP + pOPos->nContent.GetIndex() - nAktPos;
                pBkmks->Append(nCp, BookmarkToWord(rBkmk.GetName()), &rBkmk);
            }
        }
    }
}

void WW8Export::MoveFieldMarks(sal_uLong nFrom, sal_uLong nTo)
{
    pBkmks->MoveFieldMarks(nFrom, nTo);
}

void WW8Export::AppendBookmark( const OUString& rName, bool bSkip )
{
    sal_uLong nSttCP = Fc2Cp( Strm().Tell() ) + ( bSkip? 1: 0 );
    pBkmks->Append( nSttCP, rName );
}

// #i120928 collect all the graphics of bullets applied to paragraphs
int MSWordExportBase::CollectGrfsOfBullets()
{
    m_vecBulletPic.clear();

    if ( pDoc )
    {
        int nCountRule = pDoc->GetNumRuleTbl().size();
        for (int n = 0; n < nCountRule; ++n)
        {
            const SwNumRule &rRule = *( pDoc->GetNumRuleTbl().at(n) );
            sal_uInt16 nLevels = rRule.IsContinusNum() ? 1 : 9;
            for (sal_uInt16 nLvl = 0; nLvl < nLevels; ++nLvl)
            {
                const SwNumFmt &rFmt = rRule.Get(nLvl);
                if (SVX_NUM_BITMAP != rFmt.GetNumberingType())
                {
                    continue;
                }
                const Graphic *pGraf = rFmt.GetBrush()? rFmt.GetBrush()->GetGraphic():0;
                if ( pGraf )
                {
                    bool bHas = false;
                    for (unsigned i = 0; i < m_vecBulletPic.size(); ++i)
                    {
                        if (m_vecBulletPic[i]->GetChecksum() == pGraf->GetChecksum())
                        {
                            bHas = true;
                            break;
                        }
                    }
                    if (!bHas)
                    {
                        m_vecBulletPic.push_back(pGraf);
                    }
                }
            }
        }
    }

    return m_vecBulletPic.size();
}

void MSWordExportBase::BulletDefinitions()
{
    for (size_t i = 0; i < m_vecBulletPic.size(); ++i)
    {
        const MapMode aMapMode(MAP_TWIP);
        const Graphic& rGraphic = *m_vecBulletPic[i];
        Size aSize(rGraphic.GetPrefSize());
        if (MAP_PIXEL == rGraphic.GetPrefMapMode().GetMapUnit())
            aSize = Application::GetDefaultDevice()->PixelToLogic(aSize, aMapMode);
        else
            aSize = OutputDevice::LogicToLogic(aSize,rGraphic.GetPrefMapMode(), aMapMode);
        AttrOutput().BulletDefinition(i, rGraphic, aSize);
    }
}

//Export Graphic of Bullets
void WW8Export::ExportGrfBullet(const SwTxtNode& rNd)
{
    int nCount = CollectGrfsOfBullets();
    if (nCount > 0)
    {
        SwPosition aPos(rNd);
        OUString aPicBullets("_PictureBullets");
        AppendBookmark(aPicBullets);
        for (int i = 0; i < nCount; i++)
        {
            sw::Frame aFrame(*(m_vecBulletPic[i]), aPos);
            OutGrfBullets(aFrame);
        }
        AppendBookmark(aPicBullets);
    }
}

static sal_uInt8 nAttrMagicIdx = 0;
void WW8Export::OutGrfBullets(const sw::Frame & rFrame)
{
    if ( !pGrf || !pChpPlc || !pO )
        return;

    pGrf->Insert(rFrame);
    pChpPlc->AppendFkpEntry( Strm().Tell(), pO->size(), pO->data() );
    pO->clear();
    //if links...
    WriteChar( (char)1 );

    sal_uInt8 aArr[ 22 ];
    sal_uInt8* pArr = aArr;

    // sprmCFSpec
    if( bWrtWW8 )
        Set_UInt16( pArr, 0x855 );
    else
        Set_UInt8( pArr, 117 );
    Set_UInt8( pArr, 1 );

    Set_UInt16( pArr, 0x083c );
    Set_UInt8( pArr, 0x81 );

    // sprmCPicLocation
    if( bWrtWW8 )
        Set_UInt16( pArr, 0x6a03 );
    else
    {
        Set_UInt8( pArr, 68 );
        Set_UInt8( pArr, 4 );
    }
    Set_UInt32( pArr, GRF_MAGIC_321 );

    //extern  nAttrMagicIdx;
    --pArr;
    Set_UInt8( pArr, nAttrMagicIdx++ );
    pChpPlc->AppendFkpEntry( Strm().Tell(), static_cast< short >(pArr - aArr), aArr );
}

int MSWordExportBase::GetGrfIndex(const SvxBrushItem& rBrush)
{
    int nIndex = -1;
    if ( rBrush.GetGraphic() )
    {
        for (unsigned i = 0; i < m_vecBulletPic.size(); ++i)
        {
            if (m_vecBulletPic[i]->GetChecksum() == rBrush.GetGraphic()->GetChecksum())
            {
                nIndex = i;
                break;
            }
        }
    }

    return nIndex;
}

void MSWordExportBase::AppendWordBookmark( const String& rName )
{
    AppendBookmark( BookmarkToWord( rName ) );
}


//--------------------------------------------------------------------------

void WW8_WrtRedlineAuthor::Write( Writer& rWrt )
{
    WW8Export & rWW8Wrt = *(((SwWW8Writer&)rWrt).m_pExport);
    rWW8Wrt.WriteAsStringTable(maAuthors, rWW8Wrt.pFib->fcSttbfRMark,
        rWW8Wrt.pFib->lcbSttbfRMark, rWW8Wrt.bWrtWW8 ? 0 : 2);
}

sal_uInt16 WW8Export::AddRedlineAuthor( sal_uInt16 nId )
{
    if( !pRedlAuthors )
    {
        pRedlAuthors = new WW8_WrtRedlineAuthor;
        pRedlAuthors->AddName(OUString("Unknown"));
    }
    return pRedlAuthors->AddName( SW_MOD()->GetRedlineAuthor( nId ) );
}

//--------------------------------------------------------------------------

void WW8Export::WriteAsStringTable(const std::vector<OUString>& rStrings,
    sal_Int32& rfcSttbf, sal_Int32& rlcbSttbf, sal_uInt16 nExtraLen)
{
    sal_uInt16 n, nCount = static_cast< sal_uInt16 >(rStrings.size());
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
                const OUString &rString = rStrings[n];
                const String aNm(rString.copy(0, std::min<sal_Int32>(rString.getLength(), 255)));
                rStrm << (sal_uInt8)aNm.Len();
                SwWW8Writer::WriteString8(rStrm, aNm, false,
                    RTL_TEXTENCODING_MS_1252);
                if (nExtraLen)
                    SwWW8Writer::FillCount(rStrm, nExtraLen);
            }
        }
        rlcbSttbf = rStrm.Tell() - rfcSttbf;
        if( !bWrtWW8 )
            SwWW8Writer::WriteShort( rStrm, rfcSttbf, (sal_uInt16)rlcbSttbf );
    }
}

// WriteShort() traegt an FilePos nPos den Wert nVal ein und seekt auf die
// alte FilePos zurueck. Benutzt zum Nachtragen von Laengen.
void SwWW8Writer::WriteShort( SvStream& rStrm, sal_uLong nPos, sal_Int16 nVal )
{
    sal_uLong nOldPos = rStrm.Tell();       // remember Pos
    rStrm.Seek( nPos );
    SwWW8Writer::WriteShort( rStrm, nVal );
    rStrm.Seek( nOldPos );
}

void SwWW8Writer::WriteLong( SvStream& rStrm, sal_uLong nPos, sal_Int32 nVal )
{
    sal_uLong nOldPos = rStrm.Tell();       // remember Pos
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
    OString sTmp(OUStringToOString(rStr, eCodeSet));
    const sal_Char *pStart = sTmp.getStr();
    const sal_Char *pEnd = pStart + sTmp.getLength();
    rO.reserve(rO.size() + sTmp.getLength());

    std::copy(pStart, pEnd, std::inserter(rO, rO.end()));
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

void WW8Export::WriteStringAsPara( const String& rTxt, sal_uInt16 nStyleId )
{
    if( rTxt.Len() )
        OutSwString( rTxt, 0, rTxt.Len(), IsUnicode(), RTL_TEXTENCODING_MS_1252 );
    WriteCR();              // CR thereafter

    ww::bytes aArr;
    SwWW8Writer::InsUInt16( aArr, nStyleId );
    if( bOutTable )
    {                                               // Tab-Attr
        // sprmPFInTable
        if( bWrtWW8 )
            SwWW8Writer::InsUInt16( aArr, NS_sprm::LN_PFInTable );
        else
            aArr.push_back( 24 );
        aArr.push_back( 1 );
    }

    sal_uLong nPos = Strm().Tell();
    pPapPlc->AppendFkpEntry( nPos, aArr.size(), aArr.data() );
    pChpPlc->AppendFkpEntry( nPos );
}

void MSWordExportBase::WriteSpecialText( sal_uLong nStart, sal_uLong nEnd, sal_uInt8 nTTyp )
{
    sal_uInt8 nOldTyp = nTxtTyp;
    nTxtTyp = nTTyp;
    SwPaM* pOldPam = pCurPam;       //!! Simply shifting the PaM without restoring should do the job too
    SwPaM* pOldEnd = pOrigPam;
    bool bOldPageDescs = bOutPageDescs;
    bOutPageDescs = false;
                                    // bOutKF was setted / stored in WriteKF1
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
    delete pCurPam;                    // delete Pam
    pCurPam = pOldPam;
    pOrigPam = pOldEnd;
    nTxtTyp = nOldTyp;
}

void WW8Export::OutSwString(const String& rStr, xub_StrLen nStt,
    xub_StrLen nLen, bool bUnicode, rtl_TextEncoding eChrSet)

{
    SAL_INFO( "sw.ww8.level2", "<OutSwString>" );

    if( nLen )
    {
        if ( bUnicode != pPiece->IsUnicode() )
            pPiece->AppendPc ( Strm().Tell(), bUnicode );

        if( nStt || nLen != rStr.Len() )
        {
            String sOut( rStr.Copy( nStt, nLen ) );

            SAL_INFO( "sw.ww8.level2", sOut );

            if (bUnicode)
                SwWW8Writer::WriteString16(Strm(), sOut, false);
            else
                SwWW8Writer::WriteString8(Strm(), sOut, false, eChrSet);
        }
        else
        {
            SAL_INFO( "sw.ww8.level2", rStr );

            if (bUnicode)
                SwWW8Writer::WriteString16(Strm(), rStr, false);
            else
                SwWW8Writer::WriteString8(Strm(), rStr, false, eChrSet);
        }
    }

    SAL_INFO( "sw.ww8.level2", "</OutSwString>" );
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
        Strm() << (sal_uInt8)c;
}

void MSWordExportBase::SaveData( sal_uLong nStt, sal_uLong nEnd )
{
    MSWordSaveData aData;

    // WW8Export only stuff - zeroed here not to issue warnings
    aData.pOOld = NULL;

    // Common stuff
    aData.pOldPam = pCurPam;
    aData.pOldEnd = pOrigPam;
    aData.pOldFlyFmt = mpParentFrame;
    aData.pOldPageDesc = pAktPageDesc;

    aData.pOldFlyOffset = pFlyOffset;
    aData.eOldAnchorType = eNewAnchorType;

    aData.bOldOutTable = bOutTable;
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
    bOutFlyFrmAttrs = rData.bOldFlyFrmAttrs;
    bStartTOX = rData.bOldStartTOX;
    bInWriteTOX = rData.bOldInWriteTOX;

    mpParentFrame = rData.pOldFlyFmt;
    pAktPageDesc = rData.pOldPageDesc;

    eNewAnchorType = rData.eOldAnchorType;
    pFlyOffset = rData.pOldFlyOffset;

    maSaveData.pop();
}

void WW8Export::SaveData( sal_uLong nStt, sal_uLong nEnd )
{
    MSWordExportBase::SaveData( nStt, nEnd );

    MSWordSaveData &rData = maSaveData.top();

    if ( !pO->empty() )
    {
        rData.pOOld = pO;
        pO = new ww::bytes();
    }
    else
        rData.pOOld = 0; // reuse pO

    rData.bOldWriteAll = GetWriter().bWriteAll;
    GetWriter().bWriteAll = true;
}

void WW8Export::RestoreData()
{
    MSWordSaveData &rData = maSaveData.top();

    GetWriter().bWriteAll = rData.bOldWriteAll;

    OSL_ENSURE( pO->empty(), "pO is not empty in WW8Export::RestoreData()" );
    if ( rData.pOOld )
    {
        delete pO;
        pO = rData.pOOld;
    }

    MSWordExportBase::RestoreData();
}

void WW8AttributeOutput::TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    sal_uInt32 nDepth = pTableTextNodeInfoInner->getDepth();

    if ( nDepth > 0 )
    {
        /* Cell */
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFInTable );
        m_rWW8Export.pO->push_back( (sal_uInt8)0x1 );
        m_rWW8Export.InsUInt16( NS_sprm::LN_PTableDepth );
        m_rWW8Export.InsUInt32( nDepth );

        if ( nDepth > 1 && pTableTextNodeInfoInner->isEndOfCell() )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_PCell );
            m_rWW8Export.pO->push_back( (sal_uInt8)0x1 );
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
            m_rWW8Export.pO->push_back( (sal_uInt8)0x1 );

            if ( nDepth == 1 )
            {
                m_rWW8Export.InsUInt16( NS_sprm::LN_PFTtp );
                m_rWW8Export.pO->push_back( (sal_uInt8)0x1 );
            }

            m_rWW8Export.InsUInt16( NS_sprm::LN_PTableDepth );
            m_rWW8Export.InsUInt32( nDepth );

            if ( nDepth > 1 )
            {
                m_rWW8Export.InsUInt16( NS_sprm::LN_PCell );
                m_rWW8Export.pO->push_back( (sal_uInt8)0x1 );
                m_rWW8Export.InsUInt16( NS_sprm::LN_PRow );
                m_rWW8Export.pO->push_back( (sal_uInt8)0x1 );
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

static sal_uInt16 lcl_TCFlags(SwDoc &rDoc, const SwTableBox * pBox, sal_Int32 nRowSpan)
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
        const SwStartNode * pSttNd = pBox->GetSttNd();
        if(pSttNd)
        {
            SwNodeIndex aIdx( *pSttNd );
            const SwCntntNode * pCNd = pSttNd->GetNodes().GoNext( &aIdx );
            if( pCNd && pCNd->IsTxtNode())
            {
                SfxItemSet aCoreSet(rDoc.GetAttrPool(), RES_CHRATR_ROTATE, RES_CHRATR_ROTATE);
                ((SwTxtNode*)pCNd)->GetAttr( aCoreSet, 0, !((SwTxtNode*)pCNd)->GetTxt().isEmpty());
                const SvxCharRotateItem * pRotate = NULL;
                const SfxPoolItem * pRotItem;
                if ( SFX_ITEM_SET == aCoreSet.GetItemState(RES_CHRATR_ROTATE, sal_True, &pRotItem))
                {
                    pRotate = (SvxCharRotateItem*)pRotItem;
                    if(pRotate && pRotate->GetValue() == 900)
                    {
                        nFlags = nFlags | 0x0004 | 0x0008;
                    }
                    else if(pRotate && pRotate->GetValue() == 2700 )
                    {
                        nFlags = nFlags | 0x0004 | 0x0010;
                    }
                }
            }
        }
    }

    return nFlags;
}

void WW8AttributeOutput::TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwTableBoxes & rTblBoxes = pTabLine->GetTabBoxes();

    sal_uInt8 nBoxes = rTblBoxes.size();
    for ( sal_uInt8 n = 0; n < nBoxes; n++ )
    {
        const SwTableBox * pTabBox1 = rTblBoxes[n];
        const SwFrmFmt * pFrmFmt = pTabBox1->GetFrmFmt();

        if ( FRMDIR_VERT_TOP_RIGHT == m_rWW8Export.TrueFrameDirection( *pFrmFmt ) )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_TTextFlow );
            m_rWW8Export.pO->push_back( sal_uInt8(n) );        //start range
            m_rWW8Export.pO->push_back( sal_uInt8(n + 1) );    //end range
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
    sal_uInt8 nCantSplit = (!rSplittable.GetValue()) ? 1 : 0;
    if ( m_rWW8Export.bWrtWW8 )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_TFCantSplit );
        m_rWW8Export.pO->push_back( nCantSplit );
        m_rWW8Export.InsUInt16( NS_sprm::LN_TFCantSplit90 ); // also write fCantSplit90
    }
    else
    {
        m_rWW8Export.pO->push_back( 185 );
    }
    m_rWW8Export.pO->push_back( nCantSplit );
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
            m_rWW8Export.pO->push_back( 189 );
        m_rWW8Export.InsUInt16( (sal_uInt16)nHeight );
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
                    m_rWW8Export.pO->push_back( 182 );
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
    const SwTableFmt * pTableFmt = pTable->GetTableFmt();

    // Writing these SPRM's will make the table a floating one, so only write
    // them in case the table is already inside a frame.
    if (pTableFmt != NULL && pTable->GetTableNode()->GetFlyFmt())
    {
        const SvxULSpaceItem & rUL = pTableFmt->GetULSpace();

        if (rUL.GetUpper() > 0)
        {
            sal_uInt8 nPadding = 2;
            sal_uInt8 nPcVert = 0;
            sal_uInt8 nPcHorz = 0;

            sal_uInt8 nTPc = (nPadding << 4) | (nPcVert << 2) | nPcHorz;

            m_rWW8Export.InsUInt16(NS_sprm::LN_TPc);
            m_rWW8Export.pO->push_back( nTPc );

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
            m_rWW8Export.pO->push_back( 186 );
        m_rWW8Export.pO->push_back( 1 );
    }

    ww8::TableBoxVectorPtr pTableBoxes =
        pTableTextNodeInfoInner->getTableBoxesOfRow();
    // number of cell written
    sal_uInt32 nBoxes = pTableBoxes->size();
    if (nBoxes > ww8::MAXTABLECELLS)
        nBoxes = ww8::MAXTABLECELLS;

    // sprm header
    m_rWW8Export.InsUInt16( NS_sprm::LN_TDefTable );
    sal_uInt16 nSprmSize = 2 + (nBoxes + 1) * 2 + nBoxes * 20;
    m_rWW8Export.InsUInt16( nSprmSize ); // length

    // number of boxes
    m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(nBoxes) );

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
         m_rWW8Export.InsUInt16( static_cast<sal_uInt16>( *it ) + nTblOffset );
     }

     /* TCs */
    ww8::RowSpansPtr pRowSpans = pTableTextNodeInfoInner->getRowSpansOfRow();
    ww8::RowSpans::const_iterator aItRowSpans = pRowSpans->begin();
    ww8::TableBoxVector::const_iterator aIt;
    ww8::TableBoxVector::const_iterator aItEnd = pTableBoxes->end();

#if OSL_DEBUG_LEVEL > 1
    size_t nRowSpans = pRowSpans->size();
    size_t nTableBoxes = pTableBoxes->size();
    (void) nRowSpans;
    (void) nTableBoxes;
#endif

    for( aIt = pTableBoxes->begin(); aIt != aItEnd; ++aIt, ++aItRowSpans)
    {
        sal_uInt16 npOCount = m_rWW8Export.pO->size();

        const SwTableBox * pTabBox1 = *aIt;
        const SwFrmFmt * pBoxFmt = NULL;
        if (pTabBox1 != NULL)
            pBoxFmt = pTabBox1->GetFrmFmt();

        if ( m_rWW8Export.bWrtWW8 )
        {
            sal_uInt16 nFlags =
                lcl_TCFlags(*m_rWW8Export.pDoc, pTabBox1, *aItRowSpans);
             m_rWW8Export.InsUInt16( nFlags );
        }

        static sal_uInt8 aNullBytes[] = { 0x0, 0x0 };

        m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), aNullBytes, aNullBytes+2 );   // dummy
        if (pBoxFmt != NULL)
        {
            const SvxBoxItem & rBoxItem = pBoxFmt->GetBox();

            m_rWW8Export.Out_SwFmtTableBox( *m_rWW8Export.pO, &rBoxItem ); // 8/16 Byte
        }
        else
            m_rWW8Export.Out_SwFmtTableBox( *m_rWW8Export.pO, NULL); // 8/16 Byte

        SAL_INFO( "sw.ww8.level2", "<tclength>" << ( m_rWW8Export.pO->size() - npOCount ) << "</tclength>" );
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
                    GetExport().pDoc->GetPageDesc(0).GetPageFmtOfNode(*pTxtNd, false);
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
    static const sal_uInt16 aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT,
        BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };

    for ( int i = 0; i < 4; ++i )
    {
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, 0xD634 );
        m_rWW8Export.pO->push_back( sal_uInt8(6) );
        m_rWW8Export.pO->push_back( sal_uInt8(0) );
        m_rWW8Export.pO->push_back( sal_uInt8(1) );
        m_rWW8Export.pO->push_back( sal_uInt8(1 << i) );
        m_rWW8Export.pO->push_back( sal_uInt8(3) );

        SwWW8Writer::InsUInt16( *m_rWW8Export.pO,
                pFrmFmt->GetBox().GetDistance( aBorders[i] ) );
    }
}

void WW8AttributeOutput::TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwTableBoxes & rTabBoxes = pTabLine->GetTabBoxes();

    sal_uInt8 nBoxes = rTabBoxes.size();
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_TDefTableShd );
    else
        m_rWW8Export.pO->push_back( (sal_uInt8)191 );
    m_rWW8Export.pO->push_back( (sal_uInt8)(nBoxes * 2) );  // Len

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
        sal_uInt8 nBoxes0 = rTabBoxes.size();
        if (nBoxes0 > 21)
            nBoxes0 = 21;

        for (sal_uInt32 m = 0; m < 2; m++)
        {
            m_rWW8Export.InsUInt16( aSprmIds[m] );
            m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(nBoxes0 * 10) );

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
    if ( !bWrtWW8 && !IsInTable() )
        OutWW6FlyFrmsInCntnt( rNode );
}

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
            else if ( !IsInTable() ) //No sections in table
            {
                //#120140# Do not need to insert a page/section break after a section end. Check this case first
                sal_Bool bNeedExportBreakHere = sal_True;
                if ( aIdx.GetNode().IsTxtNode() )
                {
                    SwTxtNode *pTempNext = aIdx.GetNode().GetTxtNode();
                    if ( pTempNext )
                    {
                        const SfxPoolItem * pTempItem = NULL;
                        if (pTempNext->GetpSwAttrSet() && SFX_ITEM_SET == pTempNext->GetpSwAttrSet()->GetItemState(RES_PAGEDESC, false, &pTempItem)
                            && pTempItem && ((SwFmtPageDesc*)pTempItem)->GetRegisteredIn())
                        {
                            //Next node has a new page style which means this node is a section end. Do not insert another page/section break here
                            bNeedExportBreakHere = sal_False;
                        }
                    }
                }
                if (bNeedExportBreakHere)  //#120140# End of check
                {
                    ReplaceCr( (char)0xc ); // indicator for Page/Section-Break

                    const SwSectionFmt* pParentFmt = rSect.GetFmt()->GetParent();
                    if ( !pParentFmt )
                        pParentFmt = (SwSectionFmt*)0xFFFFFFFF;

                    sal_uLong nRstLnNum;
                    if ( aIdx.GetNode().IsCntntNode() )
                        nRstLnNum = ((SwCntntNode&)aIdx.GetNode()).GetSwAttrSet().
                                                GetLineNumber().GetStartValue();
                    else
                        nRstLnNum = 0;

                    AppendSection( pAktPageDesc, pParentFmt, nRstLnNum );
                }
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

        sal_uLong nPos = pCurPam->GetPoint()->nNode.GetIndex();
        ::SetProgressState( nPos, pCurPam->GetDoc()->GetDocShell() );
    }

    SAL_INFO( "sw.ww8.level2", "</WriteText>" );
}

void WW8Export::WriteMainText()
{
    SAL_INFO( "sw.ww8.level2", "<WriteMainText>" );

    pFib->fcMin = Strm().Tell();

    pCurPam->GetPoint()->nNode = pDoc->GetNodes().GetEndOfContent().StartOfSectionNode()->GetIndex();

    WriteText();

    if( 0 == Strm().Tell() - pFib->fcMin )  // no text ?
        WriteCR();                  // then CR at the end ( otherwise WW will complain )

    pFib->ccpText = Fc2Cp( Strm().Tell() );
    pFldMain->Finish( pFib->ccpText, 0 );

                    // ccpText includes Footnote and KF-text
                    // therefore pFib->ccpText may get updated as well
    // save the StyleId of the last paragraph. Because WW97 take the style
    // from the last CR, that will be writen after footer/Header/footnotes/
    // annotation usw.
    const SwTxtNode* pLastNd = pCurPam->GetMark()->nNode.GetNode().GetTxtNode();
    if( pLastNd )
        nLastFmtId = GetId( (SwTxtFmtColl&)pLastNd->GetAnyFmtColl() );

    SAL_INFO( "sw.ww8.level2", "</WriteMainText>" );
}

bool MSWordExportBase::IsInTable() const
{
    bool bResult = false;

    if (pCurPam != NULL)
    {
        SwNode * pNode = pCurPam->GetNode();

        if (pNode != NULL && mpTableInfo.get() != NULL)
        {
            ww8::WW8TableNodeInfo::Pointer_t pTableNodeInfo = mpTableInfo->getTableNodeInfo(pNode);

            if (pTableNodeInfo.get() != NULL && pTableNodeInfo->getDepth() > 0)
            {
                bResult = true;
            }
        }
    }

    return bResult;
}

typedef ww8::WW8Sttb< ww8::WW8Struct >  WW8SttbAssoc;

void WW8Export::WriteFkpPlcUsw()
{
    if( !bWrtWW8 )
    {
        static const sal_uInt8 aSpec[2] =
        {
            117, 1
        };

        pChpPlc->AppendFkpEntry( Strm().Tell() );   // Sepx with fSpecial
        pSepx->WriteSepx( Strm() );                 // Slcx.Sepx
        pGrf->Write();                              // Graphics
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
        // Graphics in the data stream
        pGrf->Write();                          // Graphics

        // Ausgabe in WordDocument-Stream
        pChpPlc->WriteFkps();                   // Fkp.Chpx
        pPapPlc->WriteFkps();                   // Fkp.Papx
        pSepx->WriteSepx( Strm() );             // Sepx

        // Ausagbe in Table-Stream
        pStyles->OutputStylesTable();           // for WW8 StyleTab
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
            // avoid memory leak #i120098#, the unnamed obj will be released in destructor.
            xEscherStg = GetWriter().GetStorage().OpenSotStorage(OUString(SL::aObjectPool),
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
        {
        ::std::vector<OUString> aStrings;

        ::ww8::StringVector_t & aSttbStrings = pSttbfAssoc->getStrings();
        ::ww8::StringVector_t::const_iterator aItEnd = aSttbStrings.end();
        for (::ww8::StringVector_t::const_iterator aIt = aSttbStrings.begin();
             aIt != aItEnd; ++aIt)
        {
            aStrings.push_back(aIt->getStr());
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

    WriteMainText();                    // main text
    sal_uInt8 nSprmsLen;
    sal_uInt8 *pLastSprms = pPapPlc->CopyLastSprms(nSprmsLen);

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

    pFib->fcMac = Strm().Tell();        // End of all texts

    WriteFkpPlcUsw();                   // FKP, PLC, .....
}

void MSWordExportBase::AddLinkTarget(const String& rURL)
{
    if( !rURL.Len() || rURL.GetChar(0) != INET_MARK_TOKEN )
        return;

    String aURL( BookmarkToWriter( rURL.Copy( 1 ) ) );
    xub_StrLen nPos = aURL.SearchBackward( cMarkSeparator );

    if( nPos < 2 )
        return;

    OUString sCmp(comphelper::string::remove(aURL.Copy(nPos+1), ' '));
    if( sCmp.isEmpty() )
        return;

    sCmp = sCmp.toAsciiLowerCase();

    if( sCmp == "outline" )
    {
        SwPosition aPos( *pCurPam->GetPoint() );
        String aOutline( BookmarkToWriter(aURL.Copy( 0, nPos )) );
        // If we can find the outline this bookmark refers to
        // save the name of the bookmark and the
        // node index number of where it points to
        if( pDoc->GotoOutline( aPos, aOutline ) )
        {
            sal_uLong nIdx = aPos.nNode.GetIndex();
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

    sal_uInt32 n, nMaxItems = rDoc.GetAttrPool().GetItemCount2( RES_TXTATR_INETFMT );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pINetFmt = (SwFmtINetFmt*)rDoc.GetAttrPool().GetItem2(
            RES_TXTATR_INETFMT, n ) ) &&
            0 != ( pTxtAttr = pINetFmt->GetTxtINetFmt()) &&
            0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) &&
            pTxtNd->GetNodes().IsDocNodes() )
        {
            AddLinkTarget( pINetFmt->GetValue() );
        }
    }

    const SwFmtURL *pURL;
    nMaxItems = rDoc.GetAttrPool().GetItemCount2( RES_URL );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pURL = (SwFmtURL*)rDoc.GetAttrPool().GetItem2(
            RES_URL, n ) ) )
        {
            AddLinkTarget( pURL->GetURL() );
            const ImageMap *pIMap = pURL->GetMap();
            if( pIMap )
            {
                for( sal_uInt16 i=0; i<pIMap->GetIMapObjectCount(); i++ )
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
    const sal_uLong WW_BLOCKSIZE = 0x200;

    void EncryptRC4(msfilter::MSCodec_Std97& rCtx, SvStream &rIn, SvStream &rOut)
    {
        rIn.Seek(STREAM_SEEK_TO_END);
        sal_uLong nLen = rIn.Tell();
        rIn.Seek(0);

        sal_uInt8 in[WW_BLOCKSIZE];
        for (sal_Size nI = 0, nBlock = 0; nI < nLen; nI += WW_BLOCKSIZE, ++nBlock)
        {
            sal_Size nBS = (nLen - nI > WW_BLOCKSIZE) ? WW_BLOCKSIZE : nLen - nI;
            nBS = rIn.Read(in, nBS);
            rCtx.InitCipher(nBlock);
            rCtx.Encode(in, nBS, in, nBS);
            rOut.Write(in, nBS);
        }
    }
}

void MSWordExportBase::ExportDocument( bool bWriteAll )
{
    nCharFmtStart = ANZ_DEFAULT_STYLES;
    nFmtCollStart = nCharFmtStart + pDoc->GetCharFmts()->size() - 1;

    bStyDef = bBreakBefore = bOutKF =
        bOutFlyFrmAttrs = bOutPageDescs = bOutTable = bOutFirstPage =
        bOutGrf = bInWriteEscher = bStartTOX =
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
    aTOXArr.clear();

    if ( !pOLEExp )
    {
        sal_uInt32 nSvxMSDffOLEConvFlags = 0;
        const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
        if ( rOpt.IsMath2MathType() )
            nSvxMSDffOLEConvFlags |= OLE_STARMATH_2_MATHTYPE;
        if ( rOpt.IsWriter2WinWord() )
            nSvxMSDffOLEConvFlags |= OLE_STARWRITER_2_WINWORD;
        if ( rOpt.IsCalc2Excel() )
            nSvxMSDffOLEConvFlags |= OLE_STARCALC_2_EXCEL;
        if ( rOpt.IsImpress2PowerPoint() )
            nSvxMSDffOLEConvFlags |= OLE_STARIMPRESS_2_POWERPOINT;

        pOLEExp = new SvxMSExportOLEObjects( nSvxMSDffOLEConvFlags );
    }

    if ( !pOCXExp && pDoc->GetDocShell() )
        pOCXExp = new SwMSConvertControls( pDoc->GetDocShell(), pCurPam );

    // #i81405# - Collect anchored objects before changing the redline mode.
    maFrames = GetFrames( *pDoc, bWriteAll? NULL : pOrigPam );

    mnRedlineMode = pDoc->GetRedlineMode();
    if ( !pDoc->GetRedlineTbl().empty() )
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

bool SwWW8Writer::InitStd97CodecUpdateMedium( ::msfilter::MSCodec_Std97& rCodec )
{
    uno::Sequence< beans::NamedValue > aEncryptionData;

    if ( mpMedium )
    {
        SFX_ITEMSET_ARG( mpMedium->GetItemSet(), pEncryptionDataItem, SfxUnoAnyItem, SID_ENCRYPTIONDATA, sal_False );
        if ( pEncryptionDataItem && ( pEncryptionDataItem->GetValue() >>= aEncryptionData ) && !rCodec.InitCodec( aEncryptionData ) )
        {
            OSL_ENSURE( false, "Unexpected EncryptionData!" );
            aEncryptionData.realloc( 0 );
        }

        if ( !aEncryptionData.getLength() )
        {
            // try to generate the encryption data based on password
            SFX_ITEMSET_ARG( mpMedium->GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD, sal_False );
            if ( pPasswordItem && !pPasswordItem->GetValue().isEmpty() && pPasswordItem->GetValue().getLength() <= 15 )
            {
                // Generate random number with a seed of time as salt.
                TimeValue aTime;
                osl_getSystemTime( &aTime );
                rtlRandomPool aRandomPool = rtl_random_createPool ();
                rtl_random_addBytes ( aRandomPool, &aTime, 8 );

                sal_uInt8 pDocId[ 16 ];
                rtl_random_getBytes( aRandomPool, pDocId, 16 );

                rtl_random_destroyPool( aRandomPool );

                sal_Unicode aPassword[16];
                memset( aPassword, 0, sizeof( aPassword ) );

                OUString sPassword(pPasswordItem->GetValue());
                for ( sal_Int32 nChar = 0; nChar < sPassword.getLength(); ++nChar )
                    aPassword[nChar] = sPassword[nChar];

                rCodec.InitKey( aPassword, pDocId );
                aEncryptionData = rCodec.GetEncryptionData();

                mpMedium->GetItemSet()->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );
            }
        }

        if ( aEncryptionData.getLength() )
            mpMedium->GetItemSet()->ClearItem( SID_PASSWORD );
    }

    // nonempty encryption data means hier that the codec was successfully initialized
    return ( aEncryptionData.getLength() != 0 );
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
        xTableStrm = GetWriter().GetStorage().OpenSotStream(OUString(SL::a1Table),
            STREAM_STD_WRITE );
        xDataStrm = GetWriter().GetStorage().OpenSotStream(OUString(SL::aData),
            STREAM_STD_WRITE );

        xDataStrm->SetBufferSize( 32768 );  // for graphics
        xTableStrm->SetBufferSize( 16384 ); // for the Font-/Style-Table, etc.

        xTableStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        xDataStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    }

    GetWriter().SetStream( & *xWwStrm );
    pTableStrm = &xTableStrm;
    pDataStrm = &xDataStrm;

    Strm().SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    utl::TempFile aTempMain;
    aTempMain.EnableKillingFile();
    utl::TempFile aTempTable;
    aTempTable.EnableKillingFile();
    utl::TempFile aTempData;
    aTempData.EnableKillingFile();

    msfilter::MSCodec_Std97 aCtx;
    bool bEncrypt = m_pWriter ? m_pWriter->InitStd97CodecUpdateMedium( aCtx ) : false;
    if ( bEncrypt )
    {
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
    pO = new ww::bytes();
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

        sal_uInt8 pDocId[16];
        aCtx.GetDocId( pDocId );

        sal_uInt8 pSaltData[16];
        sal_uInt8 pSaltDigest[16];
        aCtx.GetEncryptKey( pDocId, pSaltData, pSaltDigest );

        pTableStrmTemp->Write( pDocId, 16 );
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
        pUsedNumTbl->erase(pUsedNumTbl->begin(), pUsedNumTbl->begin() + pUsedNumTbl->size() - nUniqueList);
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
            GetWriter().GetStorage().Remove(OUString(SL::aData));
        }
    }
}


void WW8Export::PrepareStorage()
{
    sal_uLong nLen;
    const sal_uInt8* pData;
    const char* pName;
    sal_uInt32 nId1;

    if (bWrtWW8)
    {
        static const char aUserName[] = "Microsoft Word-Document";
        static const sal_uInt8 aCompObj[] =
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
        static const sal_uInt8 aCompObj[] =
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
    GetWriter().GetStorage().SetClass( aGName, 0, OUString::createFromAscii( pName ));
    SvStorageStreamRef xStor( GetWriter().GetStorage().OpenSotStream(sCompObj) );
    xStor->Write( pData, nLen );

    SwDocShell* pDocShell = pDoc->GetDocShell ();
    OSL_ENSURE(pDocShell, "no SwDocShell");

    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");

        if (xDocProps.is())
        {
            if ( SvtFilterOptions::Get().IsEnableWordPreview() )
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

sal_uLong SwWW8Writer::WriteStorage()
{
    // #i34818# - update layout (if present), for SwWriteTable
    ViewShell* pViewShell = NULL;
    pDoc->GetEditShell( &pViewShell );
    if( pViewShell != NULL )
        pViewShell->CalcLayout();

    long nMaxNode = pDoc->GetNodes().Count();
    ::StartProgress( STR_STATSTR_W4WWRITE, 0, nMaxNode, pDoc->GetDocShell() );

    // Respect table at the beginning of the document
    {
        SwTableNode * pTNd = pCurPam->GetNode()->FindTableNode();
        if( pTNd && bWriteAll )
            // start with the table node !!
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

sal_uLong SwWW8Writer::WriteMedium( SfxMedium& )
{
    return WriteStorage();
}

sal_uLong SwWW8Writer::Write( SwPaM& rPaM, SfxMedium& rMed,
                          const OUString* pFileName )
{
    mpMedium = &rMed;
    sal_uLong nRet = StgWriter::Write( rPaM, rMed, pFileName );
    mpMedium = NULL;
    return nRet;
}

MSWordExportBase::MSWordExportBase( SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam )
    : aMainStg(sMainStream), pISet(0), pUsedNumTbl(0), mpTopNodeOfHdFtPage(0),
    pBmpPal(0), pOLEExp(0), pOCXExp(0),
    mpTableInfo(new ww8::WW8TableInfo()), nUniqueList(0),
    mnHdFtIndex(0), pAktPageDesc(0), pPapPlc(0), pChpPlc(0), pChpIter(0),
    pStyles( NULL ),
    bHasHdr(false), bHasFtr(false), bSubstituteBullets(true),
    mbExportModeRTF( false ),
    mbOutOutlineOnly( false ),
    pDoc( pDocument ),
    pCurPam( pCurrentPam ),
    pOrigPam( pOriginalPam )
{
}

MSWordExportBase::~MSWordExportBase()
{
    delete pBmpPal;
    delete pOLEExp;
    delete pOCXExp;
}

WW8Export::WW8Export( SwWW8Writer *pWriter,
        SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam,
        bool bIsWW8 )
    : MSWordExportBase( pDocument, pCurrentPam, pOriginalPam ),
      pO( NULL ),
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

extern "C" SAL_DLLPUBLIC_EXPORT sal_uLong SAL_CALL SaveOrDelMSVBAStorage_ww8( SfxObjectShell& rDoc, SotStorage& rStor, sal_Bool bSaveInto, const String& rStorageName )
{
    SvxImportMSVBasic aTmp( rDoc, rStor );
    return aTmp.SaveOrDelMSVBAStorage( bSaveInto, rStorageName );
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL ExportDOC( const String& rFltName, const String& rBaseURL, WriterRef& xRet )
{
    xRet = new SwWW8Writer( rFltName, rBaseURL );
}


extern "C" SAL_DLLPUBLIC_EXPORT sal_uLong SAL_CALL GetSaveWarningOfMSVBAStorage_ww8(  SfxObjectShell &rDocS )
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
                xSrcRoot->openStreamElement( OUString(SL::aMSMacroCmds), embed::ElementModes::READ );
        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xSrcStream );

        if ( pStream && SVSTREAM_OK == pStream->GetError())
        {
            pStream->Seek(STREAM_SEEK_TO_END);
            pFib->lcbCmds = pStream->Tell();
            pStream->Seek(0);

            sal_uInt8 *pBuffer = new sal_uInt8[pFib->lcbCmds];
            bool bReadOk = checkRead(*pStream, pBuffer, pFib->lcbCmds);
            if (bReadOk)
                pTableStrm->Write(pBuffer, pFib->lcbCmds);
            delete[] pBuffer;

        }

        delete pStream;
    }
    catch ( const uno::Exception& )
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


    OSL_ENSURE(rFieldmark.GetFieldname() == ODF_FORMTEXT ||
                rFieldmark.GetFieldname() == ODF_FORMDROPDOWN ||
                rFieldmark.GetFieldname() == ODF_FORMCHECKBOX, "Unknown field type!!!");
    if ( ! ( rFieldmark.GetFieldname() == ODF_FORMTEXT ||
                rFieldmark.GetFieldname() == ODF_FORMDROPDOWN ||
                rFieldmark.GetFieldname() == ODF_FORMCHECKBOX ) )
        return;

    int type = 0; // TextFieldmark
    if ( pAsCheckbox )
        type = 1;
    if ( rFieldmark.GetFieldname() == ODF_FORMDROPDOWN )
        type=2;

    ::sw::mark::IFieldmark::parameter_map_t::const_iterator pNameParameter = rFieldmark.GetParameters()->find("name");
    OUString ffname;
    if(pNameParameter != rFieldmark.GetParameters()->end())
        pNameParameter->second >>= ffname;

    sal_uLong nDataStt = pDataStrm->Tell();
    pChpPlc->AppendFkpEntry(Strm().Tell());

    WriteChar(0x01);
    static sal_uInt8 aArr1[] =
    {
        0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation

        0x06, 0x08, 0x01,       // sprmCFData
        0x55, 0x08, 0x01,       // sprmCFSpec
        0x02, 0x08, 0x01        // sprmCFFldVanish
    };
    sal_uInt8* pDataAdr = aArr1 + 2;
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
        ::sw::mark::IFieldmark::parameter_map_t::const_iterator pResParameter = rFieldmark.GetParameters()->find(ODF_FORMDROPDOWN_RESULT);
        if(pResParameter != rFieldmark.GetParameters()->end())
            pResParameter->second >>= ffres;
        else
            ffres = 0;
    }
    aFldHeader.bits |= ( (ffres<<2) & 0x7C );

    std::vector< OUString > aListItems;
    if (type==2)
    {
        aFldHeader.bits |= 0x8000; // ffhaslistbox
        const ::sw::mark::IFieldmark::parameter_map_t* const pParameters = rFieldmark.GetParameters();
        ::sw::mark::IFieldmark::parameter_map_t::const_iterator pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
        if(pListEntries != pParameters->end())
        {
            uno::Sequence< OUString > vListEntries;
            pListEntries->second >>= vListEntries;
            copy(::comphelper::stl_begin(vListEntries), ::comphelper::stl_end(vListEntries), back_inserter(aListItems));
        }
    }

    const OUString ffdeftext;
    const OUString ffformat;
    const OUString ffhelptext;
    const OUString ffstattext;
    const OUString ffentrymcr;
    const OUString ffexitmcr;


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
            OUString item = aListItems[i];
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
            OUString item=aListItems[i];
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

#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", "<OutWW8_TableNodeInfoInner>" << pNodeInfoInner->toString());
#endif

    m_rWW8Export.pO->clear();

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

            m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), (sal_uInt8*)&nStyle, (sal_uInt8*)&nStyle+2 );     // Style #
            TableInfoCell(pTmpNodeInfoInner);
            m_rWW8Export.pPapPlc->AppendFkpEntry
                ( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );

            m_rWW8Export.pO->clear();
        }
    }

    if (pNodeInfoInner->isEndOfCell())
    {
        SAL_INFO( "sw.ww8", "<endOfCell/>" );

        m_rWW8Export.WriteCR(pNodeInfoInner);

        m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), (sal_uInt8*)&nStyle, (sal_uInt8*)&nStyle+2 );     // Style #
        TableInfoCell(pNodeInfoInner);
        m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );

        m_rWW8Export.pO->clear();
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

            m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), (sal_uInt8*)&nStyle, (sal_uInt8*)&nStyle+2 );     // Style #
            TableInfoCell(pTmpNodeInfoInner);
            m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );

            m_rWW8Export.pO->clear();
        }
    }

    if (pNodeInfoInner->isEndOfLine())
    {
        SAL_INFO( "sw.ww8", "<endOfLine/>" );

        TableRowEnd(pNodeInfoInner->getDepth());

        ShortToSVBT16(0, nStyle);
        m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), (sal_uInt8*)&nStyle, (sal_uInt8*)&nStyle+2 );     // Style #
        TableInfoRow(pNodeInfoInner);
        m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );

        m_rWW8Export.pO->clear();
    }
    SAL_INFO( "sw.ww8", "</OutWW8_TableNodeInfoInner>" );
}

void MSWordExportBase::OutputStartNode( const SwStartNode & rNode)
{

    ww8::WW8TableNodeInfo::Pointer_t pNodeInfo =
        mpTableInfo->getTableNodeInfo( &rNode );

    if (pNodeInfo.get() != NULL)
    {
#ifdef DBG_UTIL
        SAL_INFO( "sw.ww8", pNodeInfo->toString());
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
    SAL_INFO( "sw.ww8", "</OutWW8_SwStartNode>" );
}

void MSWordExportBase::OutputEndNode( const SwEndNode &rNode )
{
#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", "<OutWW8_SwEndNode>" << dbg_out(&rNode));
#endif

    ww8::WW8TableNodeInfo::Pointer_t pNodeInfo = mpTableInfo->getTableNodeInfo( &rNode );

    if (pNodeInfo.get() != NULL)
    {
#ifdef DBG_UTIL
        SAL_INFO( "sw.ww8", pNodeInfo->toString());
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
    SAL_INFO( "sw.ww8", "</OutWW8_SwEndNode>" );
}

const NfKeywordTable & MSWordExportBase::GetNfKeywordTable()
{
    if (pKeyMap.get() == NULL)
    {
        pKeyMap.reset(new NfKeywordTable);
        NfKeywordTable & rKeywordTable = *pKeyMap;
        rKeywordTable[NF_KEY_D] = "d";
        rKeywordTable[NF_KEY_DD] = "dd";
        rKeywordTable[NF_KEY_DDD] = "ddd";
        rKeywordTable[NF_KEY_DDDD] = "dddd";
        rKeywordTable[NF_KEY_M] = "M";
        rKeywordTable[NF_KEY_MM] = "MM";
        rKeywordTable[NF_KEY_MMM] = "MMM";
        rKeywordTable[NF_KEY_MMMM] = "MMMM";
        rKeywordTable[NF_KEY_NN] = "ddd";
        rKeywordTable[NF_KEY_NNN] = "dddd";
        rKeywordTable[NF_KEY_NNNN] = "dddd";
        rKeywordTable[NF_KEY_YY] = "yy";
        rKeywordTable[NF_KEY_YYYY] = "yyyy";
        rKeywordTable[NF_KEY_H] = "H";
        rKeywordTable[NF_KEY_HH] = "HH";
        rKeywordTable[NF_KEY_MI] = "m";
        rKeywordTable[NF_KEY_MMI] = "mm";
        rKeywordTable[NF_KEY_S] = "s";
        rKeywordTable[NF_KEY_SS] = "ss";
        rKeywordTable[NF_KEY_AMPM] = "AM/PM";
    }

    return *pKeyMap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
