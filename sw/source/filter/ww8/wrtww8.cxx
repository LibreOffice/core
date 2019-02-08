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

#include <memory>
#include <iostream>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <algorithm>
#include <map>
#include <set>
#include <hintids.hxx>
#include <string.h>
#include <osl/endian.h>
#include <sal/log.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>

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
#include <filter/msfilter/classids.hxx>
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
#include <../../core/inc/rootfrm.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentExternalData.hxx>
#include <viewopt.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <docstat.hxx>
#include <pagedesc.hxx>
#include <IMark.hxx>
#include <swtable.hxx>
#include "wrtww8.hxx"
#include "ww8par.hxx"
#include <fltini.hxx>
#include <swmodule.hxx>
#include <section.hxx>
#include <swfltopt.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <fmturl.hxx>
#include <fesh.hxx>
#include <vcl/imap.hxx>
#include <vcl/imapobj.hxx>
#include <tools/urlobj.hxx>
#include <mdiexp.hxx>
#include <strings.hrc>
#include <fmtline.hxx>
#include <fmtfsize.hxx>
#include "sprmids.hxx"

#include <comphelper/sequenceashashmap.hxx>
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "ww8attributeoutput.hxx"
#include <IDocumentMarkAccess.hxx>
#include <xmloff/odffields.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <dbgoutsw.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <svl/stritem.hxx>
#include <unotools/tempfile.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <rtl/random.h>
#include <vcl/svapp.hxx>
#include <sfx2/docfilt.hxx>
#include "WW8Sttbf.hxx"
#include <editeng/charrotateitem.hxx>
#include <svx/swframetypes.hxx>
#include "WW8FibData.hxx"
#include <numrule.hxx>
#include <fmtclds.hxx>
#include <rdfhelper.hxx>
#include <fmtclbl.hxx>
#include <iodetect.hxx>

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
    short nStartGrp;    // from here on grpprls
    short nOldStartGrp;
    sal_uInt8 nItemSize;
    sal_uInt8 nIMax;         // number of entry pairs
    sal_uInt8 nOldVarLen;
    bool bCombined;     // true : paste not allowed

    sal_uInt8 SearchSameSprm( sal_uInt16 nVarLen, const sal_uInt8* pSprms );

    WW8_WrFkp(const WW8_WrFkp&) = delete;
    WW8_WrFkp& operator=(const WW8_WrFkp&) = delete;

public:
    WW8_WrFkp(ePLCFT ePl, WW8_FC nStartFc);
    ~WW8_WrFkp();
    bool Append( WW8_FC nEndFc, sal_uInt16 nVarLen, const sal_uInt8* pSprms );
    void Combine();
    void Write( SvStream& rStrm, SwWW8WrGrf& rGrf );

    bool IsEqualPos(WW8_FC nEndFc) const
    {   return !bCombined && nIMax && nEndFc == reinterpret_cast<sal_Int32*>(pFkp)[nIMax]; }
    void MergeToNew( short& rVarLen, sal_uInt8 *& pNewSprms );
    bool IsEmptySprm() const
    {   return !bCombined && nIMax && !nOldVarLen;  }
    void SetNewEnd( WW8_FC nEnd )
    {   reinterpret_cast<sal_Int32*>(pFkp)[nIMax] = nEnd; }

    WW8_FC GetStartFc() const;
    WW8_FC GetEndFc() const;

    sal_uInt8 *CopyLastSprms(sal_uInt8 &rLen);
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

typedef std::map<OUString,long> BKMKNames;
typedef std::pair<bool,OUString> BKMK;
typedef std::pair<long,BKMK> BKMKCP;
typedef std::multimap<long,BKMKCP*> BKMKCPs;
typedef BKMKCPs::iterator CPItr;

class WW8_WrtBookmarks
{
private:
    /// Structure of one item inside this map: (startPos, (endPos, (a bool value?, bookmarkName)))
    BKMKCPs aSttCps;
    BKMKNames maSwBkmkNms;

    WW8_WrtBookmarks(WW8_WrtBookmarks const&) = delete;
    WW8_WrtBookmarks& operator=(WW8_WrtBookmarks const&) = delete;

public:
    WW8_WrtBookmarks();
    ~WW8_WrtBookmarks();
    //! Add a new bookmark to the list OR add an end position to an existing bookmark.
    void Append( WW8_CP nStartCp, const OUString& rNm );
    //! Write out bookmarks to file.
    void Write( WW8Export& rWrt );
    //! Move existing field marks from one position to another.
    void MoveFieldMarks(WW8_CP nFrom, WW8_CP nTo);
};

WW8_WrtBookmarks::WW8_WrtBookmarks()
{}

WW8_WrtBookmarks::~WW8_WrtBookmarks()
{
    for (auto& rEntry : aSttCps)
    {
        if (rEntry.second)
        {
            delete rEntry.second;
            rEntry.second = nullptr;
        }
    }
}

void WW8_WrtBookmarks::Append( WW8_CP nStartCp, const OUString& rNm)
{
    std::pair<BKMKNames::iterator, bool> aResult = maSwBkmkNms.insert(std::pair<OUString,long>(rNm,0L));
    if (aResult.second)
    {
        BKMK aBK(false,rNm);
        BKMKCP* pBKCP = new BKMKCP(static_cast<long>(nStartCp),aBK);
        aSttCps.insert(std::pair<long,BKMKCP*>(nStartCp,pBKCP));
        aResult.first->second = static_cast<long>(nStartCp);
    }
    else
    {
        std::pair<CPItr,CPItr> aRange = aSttCps.equal_range(aResult.first->second);
        for (CPItr aItr = aRange.first;aItr != aRange.second;++aItr)
        {
            if (aItr->second && aItr->second->second.second == rNm)
            {
                if (aItr->second->second.first)
                    nStartCp--;
                aItr->second->first = static_cast<long>(nStartCp);
                break;
            }
        }
    }
}

void WW8_WrtBookmarks::Write( WW8Export& rWrt)
{
    if (aSttCps.empty())
        return;
    long n;
    std::vector<OUString> aNames;
    SvMemoryStream aTempStrm1(65535,65535);
    SvMemoryStream aTempStrm2(65535,65535);

    BKMKCPs aEndCps;
    for (const auto& rEntry : aSttCps)
    {
        if (rEntry.second)
        {
            aEndCps.insert(std::pair<long,BKMKCP*>(rEntry.second->first, rEntry.second));
            aNames.push_back(rEntry.second->second.second);
            SwWW8Writer::WriteLong(aTempStrm1, rEntry.first);
        }
    }

    aTempStrm1.Seek(0);
    n = 0;
    for (const auto& rEntry : aEndCps)
    {
        if (rEntry.second)
        {
            rEntry.second->first = n;
            SwWW8Writer::WriteLong( aTempStrm2, rEntry.first);
        }
        ++n;
    }

    aTempStrm2.Seek(0);
    rWrt.WriteAsStringTable(aNames, rWrt.pFib->m_fcSttbfbkmk,rWrt.pFib->m_lcbSttbfbkmk);
    SvStream& rStrm = *rWrt.pTableStrm;
    rWrt.pFib->m_fcPlcfbkf = rStrm.Tell();
    rStrm.WriteStream( aTempStrm1 );
    SwWW8Writer::WriteLong(rStrm, rWrt.pFib->m_ccpText + rWrt.pFib->m_ccpTxbx);
    for (const auto& rEntry : aSttCps)
    {
        if (rEntry.second)
        {
            SwWW8Writer::WriteLong(rStrm, rEntry.second->first);
        }
    }
    rWrt.pFib->m_lcbPlcfbkf = rStrm.Tell() - rWrt.pFib->m_fcPlcfbkf;
    rWrt.pFib->m_fcPlcfbkl = rStrm.Tell();
    rStrm.WriteStream( aTempStrm2 );
    SwWW8Writer::WriteLong(rStrm, rWrt.pFib->m_ccpText + rWrt.pFib->m_ccpTxbx);
    rWrt.pFib->m_lcbPlcfbkl = rStrm.Tell() - rWrt.pFib->m_fcPlcfbkl;
}

void WW8_WrtBookmarks::MoveFieldMarks(WW8_CP nFrom, WW8_CP nTo)
{
    std::pair<CPItr,CPItr> aRange = aSttCps.equal_range(nFrom);
    CPItr aItr = aRange.first;
    while (aItr != aRange.second)
    {
        if (aItr->second)
        {
            if (aItr->second->first == static_cast<long>(nFrom))
            {
                aItr->second->second.first = true;
                aItr->second->first = nTo;
            }
            aSttCps.insert(std::pair<long,BKMKCP*>(nTo,aItr->second));
            aItr->second = nullptr;
            aRange = aSttCps.equal_range(nFrom);
            aItr = aRange.first;
            continue;
        }
        ++aItr;
    }
}

/// Handles export of smart tags.
class WW8_WrtFactoids
{
    std::vector<WW8_CP> m_aStartCPs;
    std::vector<WW8_CP> m_aEndCPs;
    std::vector< std::map<OUString, OUString> > m_aStatements;

    WW8_WrtFactoids(WW8_WrtFactoids const&) = delete;
    WW8_WrtFactoids& operator=(WW8_WrtFactoids const&) = delete;

public:
    WW8_WrtFactoids();
    void Append(WW8_CP nStartCp, WW8_CP nEndCp, const std::map<OUString, OUString>& rStatements);
    void Write(WW8Export& rWrt);
};

WW8_WrtFactoids::WW8_WrtFactoids()
{
}

void WW8_WrtFactoids::Append(WW8_CP nStartCp, WW8_CP nEndCp, const std::map<OUString, OUString>& rStatements)
{
    m_aStartCPs.push_back(nStartCp);
    m_aEndCPs.push_back(nEndCp);
    m_aStatements.push_back(rStatements);
}

void WW8_WrtFactoids::Write(WW8Export& rExport)
{
    if (m_aStartCPs.empty())
        return;

    // Smart tags are otherwise removed by Word on saving.
    rExport.pDop->fEmbedFactoids = true;

    SvStream& rStream = *rExport.pTableStrm;

    rExport.pFib->m_fcSttbfBkmkFactoid = rStream.Tell();
    // Write SttbfBkmkFactoid.
    rStream.WriteUInt16(0xffff); // fExtend
    rStream.WriteUInt16(m_aStartCPs.size()); // cData
    rStream.WriteUInt16(0); // cbExtra

    for (size_t i = 0; i < m_aStartCPs.size(); ++i)
    {
        rStream.WriteUInt16(6); // cchData
        // Write FACTOIDINFO.
        rStream.WriteUInt32(i); // dwId
        rStream.WriteUInt16(0); // fSubEntry
        rStream.WriteUInt16(0); // fto
        rStream.WriteUInt32(0); // pfpb
    }
    rExport.pFib->m_lcbSttbfBkmkFactoid = rStream.Tell() - rExport.pFib->m_fcSttbfBkmkFactoid;

    rExport.pFib->m_fcPlcfBkfFactoid = rStream.Tell();
    for (const WW8_CP& rCP : m_aStartCPs)
        rStream.WriteInt32(rCP);
    rStream.WriteInt32(rExport.pFib->m_ccpText + rExport.pFib->m_ccpTxbx);

    // Write FBKFD.
    for (size_t i = 0; i < m_aStartCPs.size(); ++i)
    {
        rStream.WriteInt16(i); // ibkl
        rStream.WriteInt16(0); // bkc
        rStream.WriteInt16(1); // cDepth, 1 as start and end is the same.
    }

    rExport.pFib->m_lcbPlcfBkfFactoid = rStream.Tell() - rExport.pFib->m_fcPlcfBkfFactoid;

    rExport.pFib->m_fcPlcfBklFactoid = rStream.Tell();
    for (const WW8_CP& rCP : m_aEndCPs)
        rStream.WriteInt32(rCP);
    rStream.WriteInt32(rExport.pFib->m_ccpText + rExport.pFib->m_ccpTxbx);

    // Write FBKLD.
    for (size_t i = 0; i < m_aEndCPs.size(); ++i)
    {
        rStream.WriteInt16(i); // ibkf
        rStream.WriteInt16(0); // cDepth, 0 as does not overlap with any other smart tag.
    }
    rExport.pFib->m_lcbPlcfBklFactoid = rStream.Tell() - rExport.pFib->m_fcPlcfBklFactoid;

    rExport.pFib->m_fcFactoidData = rStream.Tell();
    // Write SmartTagData.
    MSOFactoidType aFactoidType;
    aFactoidType.m_nId = 1;
    aFactoidType.m_aUri = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
    aFactoidType.m_aTag = "RDF";
    WW8SmartTagData aSmartTagData;
    aSmartTagData.m_aPropBagStore.m_aFactoidTypes.push_back(aFactoidType);

    std::set<OUString> aSet;
    for (const std::map<OUString, OUString>& rStatements : m_aStatements)
    {
        // Statements for a single text node.
        for (const auto& rPair : rStatements)
        {
            aSet.insert(rPair.first);
            aSet.insert(rPair.second);
        }
    }
    aSmartTagData.m_aPropBagStore.m_aStringTable.assign(aSet.begin(), aSet.end());
    for (const std::map<OUString, OUString>& rStatements : m_aStatements)
    {
        MSOPropertyBag aPropertyBag;
        aPropertyBag.m_nId = 1;
        for (const auto& rPair : rStatements)
        {
            MSOProperty aProperty;
            aProperty.m_nKey = std::distance(aSet.begin(), aSet.find(rPair.first));
            aProperty.m_nValue = std::distance(aSet.begin(), aSet.find(rPair.second));
            aPropertyBag.m_aProperties.push_back(aProperty);
        }
        aSmartTagData.m_aPropBags.push_back(aPropertyBag);
    }

    aSmartTagData.Write(rExport);
    rExport.pFib->m_lcbFactoidData = rStream.Tell() - rExport.pFib->m_fcFactoidData;
}

#define DEFAULT_STYLES_COUNT 16

// Names of the storage streams
#define sMainStream OUString("WordDocument")
#define sCompObj "\1CompObj"

static void WriteDop( WW8Export& rWrt )
{
    WW8Dop& rDop = *rWrt.pDop;

    // i#78951#, store the value of unknown compatibility options
    rDop.SetCompatibilityOptions( rWrt.m_pDoc->getIDocumentSettingAccess().Getn32DummyCompatibilityOptions1());
    rDop.SetCompatibilityOptions2( rWrt.m_pDoc->getIDocumentSettingAccess().Getn32DummyCompatibilityOptions2());

    rDop.fNoLeading = !rWrt.m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::ADD_EXT_LEADING);
    rDop.fUsePrinterMetrics = !rWrt.m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::USE_VIRTUAL_DEVICE);

    // write default TabStop
    const SvxTabStopItem& rTabStop =
        DefaultItemGet<SvxTabStopItem>(*rWrt.m_pDoc, RES_PARATR_TABSTOP);
    rDop.dxaTab = static_cast<sal_uInt16>(rTabStop[0].GetTabPos());

    // Zoom factor and type
    SwViewShell *pViewShell(rWrt.m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell());
    if (pViewShell)
    {
        switch ( pViewShell->GetViewOptions()->GetZoomType() )
        {
            case SvxZoomType::WHOLEPAGE: rDop.zkSaved = 1; break;
            case SvxZoomType::PAGEWIDTH: rDop.zkSaved = 2; break;
            case SvxZoomType::OPTIMAL:   rDop.zkSaved = 3; break;
            default:                 rDop.zkSaved = 0;
                rDop.wScaleSaved = pViewShell->GetViewOptions()->GetZoom();
                break;
        }
    }

    // Values from the DocumentStatistics (are definitely needed
    // for the DocStat fields)
    rDop.fWCFootnoteEdn = true; // because they are included in StarWriter

    const SwDocStat& rDStat = rWrt.m_pDoc->getIDocumentStatistics().GetDocStat();
    rDop.cWords = rDStat.nWord;
    rDop.cCh = rDStat.nChar;
    rDop.cPg = static_cast< sal_Int16 >(rDStat.nPage);
    rDop.cParas = rDStat.nPara;
    rDop.cLines = rDStat.nPara;

    SwDocShell *pDocShell(rWrt.m_pDoc->GetDocShell());
    OSL_ENSURE(pDocShell, "no SwDocShell");
    uno::Reference<document::XDocumentProperties> xDocProps;
    uno::Reference<beans::XPropertySet> xProps;
    if ( pDocShell )
    {
        xProps.set(pDocShell->GetModel(), uno::UNO_QUERY);

        rDop.lKeyProtDoc = pDocShell->GetModifyPasswordHash();
    }

    if ((rWrt.pSepx && rWrt.pSepx->DocumentIsProtected()) ||
        rWrt.m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_FORM ) ||
        rDop.lKeyProtDoc != 0)
    {
        rDop.fProtEnabled =  true;
        // The password was ignored at import if forms protection was enabled,
        // so round-trip it since protection is still enabled.
        if ( rDop.lKeyProtDoc == 0 && xProps.is() )
        {
            comphelper::SequenceAsHashMap aPropMap( xProps->getPropertyValue("InteropGrabBag"));
            aPropMap.getValue("FormPasswordHash") >>= rDop.lKeyProtDoc;
        }
    }
    else
    {
        rDop.fProtEnabled = false;
    }

    if (!xDocProps.is())
    {
        rDop.dttmCreated = rDop.dttmRevised = rDop.dttmLastPrint = 0x45FBAC69;
    }
    else
    {
        ::util::DateTime uDT = xDocProps->getCreationDate();
        rDop.dttmCreated = sw::ms::DateTime2DTTM(DateTime(uDT));
        uDT = xDocProps->getModificationDate();
        rDop.dttmRevised = sw::ms::DateTime2DTTM(DateTime(uDT));
        uDT = xDocProps->getPrintDate();
        rDop.dttmLastPrint = sw::ms::DateTime2DTTM(DateTime(uDT));
    }

    // Also, the DocStat fields in headers, footers are not calculated correctly.
    // ( we do not have this fields! )

    // and also for the Headers and Footers
    rDop.cWordsFootnoteEnd   = rDStat.nWord;
    rDop.cChFootnoteEdn      = rDStat.nChar;
    rDop.cPgFootnoteEdn      = static_cast<sal_Int16>(rDStat.nPage);
    rDop.cParasFootnoteEdn   = rDStat.nPara;
    rDop.cLinesFootnoteEdn   = rDStat.nPara;

    rDop.fDontUseHTMLAutoSpacing = rWrt.m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::PARA_SPACE_MAX);

    rDop.fExpShRtn = !rWrt.m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK); // #i56856#

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

    const i18n::ForbiddenCharacters *pForbidden = nullptr;
    const i18n::ForbiddenCharacters *pUseMe = nullptr;
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
        if (nullptr != (pForbidden = m_pDoc->getIDocumentSettingAccess().getForbiddenCharacters(rTypo.GetConvertedLang(),
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
                                WW8DopTypography::GetJapanNotEndLevel1(),
                                WW8DopTypography::nMaxLeading * sizeof(sal_Unicode)
                            )
                        &&
                          !lcl_CmpBeginEndChars
                            (
                                pForbidden->beginLine,
                                WW8DopTypography::GetJapanNotBeginLevel1(),
                                WW8DopTypography::nMaxFollowing * sizeof(sal_Unicode)
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
    if (rTypo.iLevelOfKinsoku && pUseMe)
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

    const IDocumentSettingAccess& rIDocumentSettingAccess = GetWriter().getIDocumentSettingAccess();

    rTypo.fKerningPunct = sal_uInt16(rIDocumentSettingAccess.get(DocumentSettingId::KERN_ASIAN_PUNCTUATION));
    rTypo.iJustification = sal_uInt16(m_pDoc->getIDocumentSettingAccess().getCharacterCompressionType());
}

// It can only be found something with this method, if it is used within
// WW8_SwAttrIter::OutAttr() and WW8Export::OutputItemSet()
const SfxPoolItem* MSWordExportBase::HasItem( sal_uInt16 nWhich ) const
{
    const SfxPoolItem* pItem=nullptr;
    if (m_pISet)
    {
        // if write a EditEngine text, then the WhichIds are greater as
        // our own Ids. So the Id have to translate from our into the
        // EditEngine Range
        nWhich = sw::hack::GetSetWhichFromSwDocWhich(*m_pISet, *m_pDoc, nWhich);
        if (nWhich && SfxItemState::SET != m_pISet->GetItemState(nWhich, true, &pItem))
            pItem = nullptr;
    }
    else if( m_pChpIter )
        pItem = m_pChpIter->HasTextItem( nWhich );
    else
    {
        OSL_ENSURE( false, "Where is my ItemSet / pChpIter ?" );
        pItem = nullptr;
    }
    return pItem;
}

const SfxPoolItem& MSWordExportBase::GetItem(sal_uInt16 nWhich) const
{
    assert((m_pISet || m_pChpIter) && "Where is my ItemSet / pChpIter ?");
    if (m_pISet)
    {
        // if write a EditEngine text, then the WhichIds are greater as
        // our own Ids. So the Id have to translate from our into the
        // EditEngine Range
        nWhich = sw::hack::GetSetWhichFromSwDocWhich(*m_pISet, *m_pDoc, nWhich);
        OSL_ENSURE(nWhich != 0, "All broken, Impossible");
        return m_pISet->Get(nWhich);
    }
    return m_pChpIter->GetItem( nWhich );
}

WW8_WrPlc1::WW8_WrPlc1( sal_uInt16 nStructSz )
    : nStructSiz( nStructSz )
{
    nDataLen = 16 * nStructSz;
    pData.reset( new sal_uInt8[ nDataLen ] );
}

WW8_WrPlc1::~WW8_WrPlc1()
{
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
        memcpy( pNew, pData.get(), nDataLen );
        pData.reset(pNew);
        nDataLen *= 2;
    }
    memcpy( pData.get() + nInsPos, pNewData, nStructSiz );
}

void WW8_WrPlc1::Finish( sal_uLong nLastCp, sal_uLong nSttCp )
{
    if( !aPos.empty() )
    {
        aPos.push_back( nLastCp );
        if( nSttCp )
            for(WW8_CP & rCp : aPos)
                rCp -= nSttCp;
    }
}

void WW8_WrPlc1::Write( SvStream& rStrm )
{
    decltype(aPos)::size_type i;
    for( i = 0; i < aPos.size(); ++i )
        SwWW8Writer::WriteLong( rStrm, aPos[i] );
    if( i )
        rStrm.WriteBytes(pData.get(), (i-1) * nStructSiz);
}

// Class WW8_WrPlcField for fields

void WW8_WrPlcField::Write( WW8Export& rWrt )
{
    if( WW8_WrPlc1::Count() <= 1 )
        return;

    WW8_FC *pfc;
    sal_Int32 *plc;
    switch (nTextTyp)
    {
        case TXT_MAINTEXT:
            pfc = &rWrt.pFib->m_fcPlcffldMom;
            plc = &rWrt.pFib->m_lcbPlcffldMom;
            break;
        case TXT_HDFT:
            pfc = &rWrt.pFib->m_fcPlcffldHdr;
            plc = &rWrt.pFib->m_lcbPlcffldHdr;
            break;

        case TXT_FTN:
            pfc = &rWrt.pFib->m_fcPlcffldFootnote;
            plc = &rWrt.pFib->m_lcbPlcffldFootnote;
            break;

        case TXT_EDN:
            pfc = &rWrt.pFib->m_fcPlcffldEdn;
            plc = &rWrt.pFib->m_lcbPlcffldEdn;
            break;

        case TXT_ATN:
            pfc = &rWrt.pFib->m_fcPlcffldAtn;
            plc = &rWrt.pFib->m_lcbPlcffldAtn;
            break;

        case TXT_TXTBOX:
            pfc = &rWrt.pFib->m_fcPlcffldTxbx;
            plc = &rWrt.pFib->m_lcbPlcffldTxbx;
            break;

        case TXT_HFTXTBOX:
            pfc = &rWrt.pFib->m_fcPlcffldHdrTxbx;
            plc = &rWrt.pFib->m_lcbPlcffldHdrTxbx;
            break;

        default:
            pfc = plc = nullptr;
            break;
    }

    if( pfc && plc )
    {
        sal_uLong nFcStart = rWrt.pTableStrm->Tell();
        WW8_WrPlc1::Write( *rWrt.pTableStrm );
        *pfc = nFcStart;
        *plc = rWrt.pTableStrm->Tell() - nFcStart;
    }
}

void WW8_WrMagicTable::Write( WW8Export& rWrt )
{
    if( WW8_WrPlc1::Count() <= 1 )
        return;
    sal_uLong nFcStart = rWrt.pTableStrm->Tell();
    WW8_WrPlc1::Write( *rWrt.pTableStrm );
    rWrt.pFib->m_fcPlcfTch = nFcStart;
    rWrt.pFib->m_lcbPlcfTch = rWrt.pTableStrm->Tell() - nFcStart;
}

void WW8_WrMagicTable::Append( WW8_CP nCp, sal_uLong nData)
{
    /*
    Tell the undocumented table hack that everything between here and the last
    table position is non-table text, don't do it if the previous position is
    the same as this one, as that would be a region of 0 length
    */
    if ((!Count()) || (Prev() != nCp))
    {
        SVBT32 nLittle;
        UInt32ToSVBT32(nData,nLittle);
        WW8_WrPlc1::Append(nCp, nLittle);
    }
}

void SwWW8Writer::FillCount( SvStream& rStrm, sal_uLong nCount )
{
    static const sal_uInt32 aNulls[16] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 // 64 Byte
    };

    while (nCount > 64)
    {
        rStrm.WriteBytes(aNulls, 64); // in steps of 64-Byte
        nCount -= 64;
    }
    rStrm.WriteBytes(aNulls, nCount); // write the rest (0 .. 64 Bytes)
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
        OSL_ENSURE( nEndPos == nCurPos, "Wrong FillUntil()" );
#endif
    return rStrm.Tell();
}

WW8_WrPlcPn::WW8_WrPlcPn(WW8Export& rWr, ePLCFT ePl, WW8_FC nStartFc)
    : rWrt(rWr)
    , nFkpStartPage(0)
    , ePlc(ePl)
{
    m_Fkps.push_back(std::make_unique<WW8_WrFkp>(ePlc, nStartFc));
}

WW8_WrPlcPn::~WW8_WrPlcPn()
{
}

sal_uInt8 *WW8_WrPlcPn::CopyLastSprms(sal_uInt8 &rLen)
{
    WW8_WrFkp& rF = *m_Fkps.back();
    return rF.CopyLastSprms(rLen);
}

void WW8_WrPlcPn::AppendFkpEntry(WW8_FC nEndFc,short nVarLen,const sal_uInt8* pSprms)
{
    WW8_WrFkp* pF = m_Fkps.back().get();

    // big sprm? build the sprmPHugePapx
    sal_uInt8* pNewSprms = const_cast<sal_uInt8*>(pSprms);
    sal_uInt8 aHugePapx[ 8 ];
    if (PAP == ePlc && 488 <= nVarLen)
    {
        sal_uInt8* p = aHugePapx;
        *p++ = *pSprms++;           // set style Id
        *p++ = *pSprms++;
        nVarLen -= 2;

        long nDataPos = rWrt.pDataStrm->Tell();
        SwWW8Writer::WriteShort( *rWrt.pDataStrm, nVarLen );
        rWrt.pDataStrm->WriteBytes(pSprms, nVarLen);

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
        pF = new WW8_WrFkp(ePlc, pF->GetEndFc()); // Start new Fkp == end of old Fkp

        m_Fkps.push_back(std::unique_ptr<WW8_WrFkp>(pF));
        if( !pF->Append( nEndFc, nVarLen, pNewSprms ) )
        {
            OSL_ENSURE( false, "Unable to insert Sprm" );
        }
    }
    if( pNewSprms != pSprms )   //Merge to new has created a new block
        delete[] pNewSprms;
}

void WW8_WrPlcPn::WriteFkps()
{
    nFkpStartPage = static_cast<sal_uInt16>( SwWW8Writer::FillUntil( rWrt.Strm() ) >> 9 );

    for(const std::unique_ptr<WW8_WrFkp> & rp : m_Fkps)
    {
        rp->Write( rWrt.Strm(), *rWrt.m_pGrf );
    }

    if( CHP == ePlc )
    {
        rWrt.pFib->m_pnChpFirst = nFkpStartPage;
        rWrt.pFib->m_cpnBteChp = m_Fkps.size();
    }
    else
    {
        rWrt.pFib->m_pnPapFirst = nFkpStartPage;
        rWrt.pFib->m_cpnBtePap = m_Fkps.size();
    }
}

void WW8_WrPlcPn::WritePlc()
{
    sal_uLong nFcStart = rWrt.pTableStrm->Tell();
    decltype(m_Fkps)::size_type i;

    for (i = 0; i < m_Fkps.size(); ++i)
    {
        SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                m_Fkps[ i ]->GetStartFc() );
    }

    SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                                m_Fkps[ i - 1 ]->GetEndFc() );

    // for every FKP output the page
    for (i = 0; i < m_Fkps.size(); ++i)
    {
        SwWW8Writer::WriteLong( *rWrt.pTableStrm, i + nFkpStartPage );
    }

    if( CHP == ePlc )
    {
        rWrt.pFib->m_fcPlcfbteChpx = nFcStart;
        rWrt.pFib->m_lcbPlcfbteChpx = rWrt.pTableStrm->Tell() - nFcStart;
    }
    else
    {
        rWrt.pFib->m_fcPlcfbtePapx = nFcStart;
        rWrt.pFib->m_lcbPlcfbtePapx = rWrt.pTableStrm->Tell() - nFcStart;
    }
}

WW8_WrFkp::WW8_WrFkp(ePLCFT ePl, WW8_FC nStartFc)
    : ePlc(ePl), nStartGrp(511), nOldStartGrp(511),
    nItemSize( ( CHP == ePl ) ? 1 : 13 ),
    nIMax(0), nOldVarLen(0), bCombined(false)
{
    pFkp = reinterpret_cast<sal_uInt8*>(new sal_Int32[128]);           // 512 Byte
    pOfs = reinterpret_cast<sal_uInt8*>(new sal_Int32[128]);           // 512 Byte
    memset( pFkp, 0, 4 * 128 );
    memset( pOfs, 0, 4 * 128 );
    reinterpret_cast<sal_Int32*>(pFkp)[0] = nStartFc;         // 0th entry FC at nStartFc
}

WW8_WrFkp::~WW8_WrFkp()
{
    delete[] reinterpret_cast<sal_Int32 *>(pFkp);
    delete[] reinterpret_cast<sal_Int32 *>(pOfs);
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
            const sal_uInt8* p = pFkp + ( static_cast<sal_uInt16>(nStart) << 1 );
            if( ( CHP == ePlc
                    ? (*p++ == nVarLen)
                    : ((static_cast<sal_uInt16>(*p++) << 1 ) == (( nVarLen+1) & 0xfffe)) )
                && !memcmp( p, pSprms, nVarLen ) )
                    return nStart;                      // found it
        }
    }
    return 0;           // didn't found it
}

sal_uInt8 *WW8_WrFkp::CopyLastSprms(sal_uInt8 &rLen)
{
    rLen=0;
    sal_uInt8 *pStart=nullptr,*pRet=nullptr;

    if (!bCombined)
        pStart = pOfs;
    else
        pStart = pFkp + ( nIMax + 1 ) * 4;

    sal_uInt8 nStart = *(pStart + (nIMax-1) * nItemSize);

    const sal_uInt8* p = pFkp + ( static_cast<sal_uInt16>(nStart) << 1 );

    if (!*p)
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
    assert((!nVarLen || pSprms) && "Item pointer missing");

    OSL_ENSURE( nVarLen < ( ( ePlc == PAP ) ? 497U : 502U ), "Sprms too long !" );

    if( bCombined )
    {
        OSL_ENSURE( false, "Fkp::Append: Fkp is already combined" );
        return false;
    }
    sal_Int32 n = reinterpret_cast<sal_Int32*>(pFkp)[nIMax];        // last entry
    if( nEndFc <= n )
    {
        OSL_ENSURE( nEndFc >= n, "+Fkp: FC backwards" );
        OSL_ENSURE( !nVarLen || !pSprms || nEndFc != n,
                                    "+Fkp: used same FC multiple times" );
                        // same FC without Sprm is ignored without grumbling

        return true;    // ignore (do not create a new Fkp)
    }

    sal_uInt8 nOldP = nVarLen ? SearchSameSprm( nVarLen, pSprms ) : 0;
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

    if( static_cast<sal_uInt16>(nPos) <= ( nIMax + 2U ) * 4U + ( nIMax + 1U ) * nItemSize )
                                            // does it fits after the CPs and offsets?
        return false;                       // no

    reinterpret_cast<sal_Int32*>(pFkp)[nIMax + 1] = nEndFc;     // insert FC

    nOldVarLen = static_cast<sal_uInt8>(nVarLen);
    if( nVarLen && !nOldP )
    {               // insert it for real
        nOldStartGrp = nStartGrp;

        nStartGrp = nPos;
        pOfs[nIMax * nItemSize] = static_cast<sal_uInt8>( nStartGrp >> 1 );
                                            // insert (start-of-data >> 1)
        sal_uInt8 nCnt = static_cast< sal_uInt8 >(CHP == ePlc
                        ? ( nVarLen < 256 ) ? static_cast<sal_uInt8>(nVarLen) : 255
                        : ( ( nVarLen + 1 ) >> 1 ));

        pFkp[ nOffset ] = nCnt;                     // Enter data length
        memcpy( pFkp + nOffset + 1, pSprms, nVarLen );  // store Sprms
    }
    else
    {
        // do not enter for real ( no Sprms or recurrence )
        // start-of-data 0 ( no data ) or recurrence
        pOfs[nIMax * nItemSize] = nOldP;
    }
    nIMax++;
    return true;
}

void WW8_WrFkp::Combine()
{
    if( bCombined )
        return;
    if( nIMax )
        memcpy( pFkp + ( nIMax + 1 ) * 4, pOfs, nIMax * nItemSize );
    delete[] pOfs;
    pOfs = nullptr;
    pFkp[511] = nIMax;
    bCombined = true;

#if defined OSL_BIGENDIAN           // only the FCs will be rotated here
    sal_uInt16 i;                   // the Sprms must be rotated elsewhere

    sal_uInt32* p;
    for( i = 0, p = (sal_uInt32*)pFkp; i <= nIMax; i++, p++ )
        *p = OSL_SWAPDWORD( *p );
#endif // ifdef OSL_BIGENDIAN
}

void WW8_WrFkp::Write( SvStream& rStrm, SwWW8WrGrf& rGrf )
{
    Combine();                      // If not already combined

    sal_uInt8* p;               //  search magic for nPicLocFc
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
    rStrm.WriteBytes(pFkp, 512);
}

void WW8_WrFkp::MergeToNew( short& rVarLen, sal_uInt8 *& rpNewSprms )
{
    sal_uInt8 nStart = pOfs[ (nIMax-1) * nItemSize ];
    if( nStart )
    {   // has Sprms
        sal_uInt8* p = pFkp + ( static_cast<sal_uInt16>(nStart) << 1 );

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
        // if this Sprms don't used from others, remove it
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

WW8_FC WW8_WrFkp::GetStartFc() const
{
    // when bCombined, then the array beginning with pFkp is already byte-swapped
    // to LittleEndian, so to extract the start and end positions they must
    // be swapped back.
    if( bCombined )
        return SVBT32ToUInt32( pFkp );        // 0. Element
    return reinterpret_cast<sal_Int32*>(pFkp)[0];
}

WW8_FC WW8_WrFkp::GetEndFc() const
{
    if( bCombined )
        return SVBT32ToUInt32( &(pFkp[nIMax*4]) );    // nIMax-th SVBT32-Element
    return reinterpret_cast<sal_Int32*>(pFkp)[nIMax];
}

// Method for managing the piece table
WW8_WrPct::WW8_WrPct(WW8_FC nfcMin)
    : nOldFc(nfcMin)
{
    AppendPc(nOldFc);
}

WW8_WrPct::~WW8_WrPct()
{
}

// Fill the piece and create a new one
void WW8_WrPct::AppendPc(WW8_FC nStartFc)
{
    WW8_CP nStartCp = nStartFc - nOldFc;    // subtract the beginning of the text
    if ( !nStartCp && !m_Pcts.empty())
    {
        OSL_ENSURE(1 == m_Pcts.size(), "empty Piece!");
        m_Pcts.pop_back();
    }

    nOldFc = nStartFc;                      // remember StartFc as old

    nStartCp >>= 1;     // for Unicode: number of characters / 2

    if (!m_Pcts.empty())
    {
        nStartCp += m_Pcts.back()->GetStartCp();
    }

    m_Pcts.push_back(std::make_unique<WW8_WrPc>(nStartFc, nStartCp));
}

void WW8_WrPct::WritePc( WW8Export& rWrt )
{
    sal_uLong nPctStart;
    sal_uLong nOldPos, nEndPos;

    nPctStart = rWrt.pTableStrm->Tell();                    // Start piece table
    rWrt.pTableStrm->WriteChar( char(0x02) );                       // Status byte PCT
    nOldPos = nPctStart + 1;                                // remember Position
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );          // then the length

    for (auto const& it : m_Pcts) // ranges
    {
        SwWW8Writer::WriteLong( *rWrt.pTableStrm, it->GetStartCp() );
    }

    // calculate the last Pos
    sal_uLong nStartCp = rWrt.pFib->m_fcMac - nOldFc;
    nStartCp >>= 1;             // For Unicode: number of characters / 2
    nStartCp += m_Pcts.back()->GetStartCp();
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nStartCp );

    // piece references
    for (auto const& it : m_Pcts)
    {
        SwWW8Writer::WriteShort(*rWrt.pTableStrm, it->GetStatus());
        SwWW8Writer::WriteLong(*rWrt.pTableStrm, it->GetStartFc());
        SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0);          // PRM=0
    }

    // entries in the FIB
    rWrt.pFib->m_fcClx = nPctStart;
    nEndPos = rWrt.pTableStrm->Tell();
    rWrt.pFib->m_lcbClx = nEndPos - nPctStart;

    // and register the length as well
    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nOldPos,
                            nEndPos - nPctStart-5 );

}

void WW8_WrPct::SetParaBreak()
{
    OSL_ENSURE( !m_Pcts.empty(), "SetParaBreak : m_Pcts.empty()" );
    m_Pcts.back()->SetStatus();
}

WW8_CP WW8_WrPct::Fc2Cp( sal_uLong nFc ) const
{
    OSL_ENSURE( nFc >= static_cast<sal_uLong>(nOldFc), "FilePos lies in front of last piece" );
    OSL_ENSURE( ! m_Pcts.empty(), "Fc2Cp no piece available" );

    nFc -= nOldFc;
    nFc /= 2; // Unicode
    return nFc + m_Pcts.back()->GetStartCp();
}

void WW8Export::AppendBookmarks( const SwTextNode& rNd, sal_Int32 nCurrentPos, sal_Int32 nLen )
{
    std::vector< const ::sw::mark::IMark* > aArr;
    sal_uInt16 nContent;
    const sal_Int32 nCurrentEnd = nCurrentPos + nLen;
    if( GetWriter().GetBookmarks( rNd, nCurrentPos, nCurrentEnd, aArr ))
    {
        sal_uLong nNd = rNd.GetIndex(), nSttCP = Fc2Cp( Strm().Tell() );
        for(const ::sw::mark::IMark* p : aArr)
        {
            const ::sw::mark::IMark& rBkmk = *p;
            if(dynamic_cast< const ::sw::mark::IFieldmark *>(&rBkmk))
                continue;

            const SwPosition* pPos = &rBkmk.GetMarkPos();
            const SwPosition* pOPos = nullptr;
            if(rBkmk.IsExpanded())
                pOPos = &rBkmk.GetOtherMarkPos();
            if( pOPos && pOPos->nNode == pPos->nNode &&
                pOPos->nContent < pPos->nContent )
            {
                pPos = pOPos;
                pOPos = &rBkmk.GetMarkPos();
            }

            if( !pOPos || ( nNd == pPos->nNode.GetIndex() &&
                ( nContent = pPos->nContent.GetIndex() ) >= nCurrentPos &&
                nContent < nCurrentEnd ) )
            {
                sal_uLong nCp = nSttCP + pPos->nContent.GetIndex() - nCurrentPos;
                m_pBkmks->Append(nCp, BookmarkToWord(rBkmk.GetName()));
            }
            if( pOPos && nNd == pOPos->nNode.GetIndex() &&
                ( nContent = pOPos->nContent.GetIndex() ) >= nCurrentPos &&
                nContent < nCurrentEnd )
            {
                sal_uLong nCp = nSttCP + pOPos->nContent.GetIndex() - nCurrentPos;
                m_pBkmks->Append(nCp, BookmarkToWord(rBkmk.GetName()));
            }
        }
    }
}

void WW8Export::AppendAnnotationMarks(const SwTextNode& rNode, sal_Int32 nCurrentPos, sal_Int32 nLen)
{
    IMarkVector aMarks;
    if (GetAnnotationMarks(rNode, nCurrentPos, nCurrentPos + nLen, aMarks))
    {
        for (const sw::mark::IMark* pMark : aMarks)
        {
            const sal_Int32 nStart = pMark->GetMarkStart().nContent.GetIndex();
            if (nStart == nCurrentPos)
            {
                m_pAtn->AddRangeStartPosition(pMark->GetName(), Fc2Cp(Strm().Tell()));
            }
        }
    }
}

void WW8Export::AppendSmartTags(SwTextNode& rTextNode)
{
    std::map<OUString, OUString> aStatements = SwRDFHelper::getTextNodeStatements("urn:bails", rTextNode);
    if (!aStatements.empty())
    {
        WW8_CP nCP = Fc2Cp(Strm().Tell());
        m_pFactoids->Append(nCP, nCP, aStatements);
    }
}

void WW8Export::MoveFieldMarks(WW8_CP nFrom, WW8_CP nTo)
{
    m_pBkmks->MoveFieldMarks(nFrom, nTo);
}

void WW8Export::AppendBookmark( const OUString& rName )
{
    sal_uLong nSttCP = Fc2Cp( Strm().Tell() );
    m_pBkmks->Append( nSttCP, rName );
}

void WW8Export::AppendBookmarkEndWithCorrection( const OUString& rName )
{
    sal_uLong nEndCP = Fc2Cp( Strm().Tell() );
    m_pBkmks->Append( nEndCP - 1, rName );
}

boost::optional<SvxBrushItem> MSWordExportBase::getBackground()
{
    boost::optional<SvxBrushItem> oRet;
    const SwFrameFormat &rFormat = m_pDoc->GetPageDesc(0).GetMaster();
    SvxBrushItem aBrush(RES_BACKGROUND);
    SfxItemState eState = rFormat.GetBackgroundState(aBrush);

    if (SfxItemState::SET == eState)
    {
        // The 'color' is set for the first page style - take it and use it as the background color of the entire DOCX
        if (aBrush.GetColor() != COL_AUTO)
            oRet = aBrush;
    }
    return oRet;
}

// #i120928 collect all the graphics of bullets applied to paragraphs
int MSWordExportBase::CollectGrfsOfBullets()
{
    m_vecBulletPic.clear();

    if ( m_pDoc )
    {
        size_t nCountRule = m_pDoc->GetNumRuleTable().size();
        for (size_t n = 0; n < nCountRule; ++n)
        {
            const SwNumRule &rRule = *( m_pDoc->GetNumRuleTable().at(n) );
            sal_uInt16 nLevels = rRule.IsContinusNum() ? 1 : 9;
            for (sal_uInt16 nLvl = 0; nLvl < nLevels; ++nLvl)
            {
                const SwNumFormat &rFormat = rRule.Get(nLvl);
                if (SVX_NUM_BITMAP != rFormat.GetNumberingType())
                {
                    continue;
                }
                const Graphic *pGraf = rFormat.GetBrush()? rFormat.GetBrush()->GetGraphic():nullptr;
                if ( pGraf )
                {
                    bool bHas = false;
                    for (const Graphic* p : m_vecBulletPic)
                    {
                        if (p->GetChecksum() == pGraf->GetChecksum())
                        {
                            bHas = true;
                            break;
                        }
                    }
                    if (!bHas)
                    {
                        Size aSize(pGraf->GetPrefSize());
                        if (0 != aSize.Height() && 0 != aSize.Width())
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
        const MapMode aMapMode(MapUnit::MapTwip);
        const Graphic& rGraphic = *m_vecBulletPic[i];
        Size aSize(rGraphic.GetPrefSize());
        if (MapUnit::MapPixel == rGraphic.GetPrefMapMode().GetMapUnit())
            aSize = Application::GetDefaultDevice()->PixelToLogic(aSize, aMapMode);
        else
            aSize = OutputDevice::LogicToLogic(aSize,rGraphic.GetPrefMapMode(), aMapMode);

        if (0 != aSize.Height() && 0 != aSize.Width())
            AttrOutput().BulletDefinition(i, rGraphic, aSize);
    }
}

//Export Graphic of Bullets
void WW8Export::ExportGrfBullet(const SwTextNode& rNd)
{
    int nCount = CollectGrfsOfBullets();
    if (nCount > 0)
    {
        SwPosition aPos(rNd);
        OUString aPicBullets("_PictureBullets");
        AppendBookmark(aPicBullets);
        for (int i = 0; i < nCount; i++)
        {
            ww8::Frame aFrame(*(m_vecBulletPic[i]), aPos);
            OutGrfBullets(aFrame);
        }
        AppendBookmark(aPicBullets);
    }
}

static sal_uInt8 nAttrMagicIdx = 0;
void WW8Export::OutGrfBullets(const ww8::Frame & rFrame)
{
    if ( !m_pGrf || !m_pChpPlc || !pO )
        return;

    m_pGrf->Insert(rFrame);
    m_pChpPlc->AppendFkpEntry( Strm().Tell(), pO->size(), pO->data() );
    pO->clear();
    // if links...
    WriteChar( char(1) );

    sal_uInt8 aArr[ 22 ];
    sal_uInt8* pArr = aArr;

    // sprmCFSpec
    Set_UInt16( pArr, 0x855 );
    Set_UInt8( pArr, 1 );

    Set_UInt16( pArr, 0x083c );
    Set_UInt8( pArr, 0x81 );

    // sprmCPicLocation
    Set_UInt16( pArr, 0x6a03 );
    Set_UInt32( pArr, GRF_MAGIC_321 );

    //extern  nAttrMagicIdx;
    --pArr;
    Set_UInt8( pArr, nAttrMagicIdx++ );
    m_pChpPlc->AppendFkpEntry( Strm().Tell(), static_cast< short >(pArr - aArr), aArr );
}

int MSWordExportBase::GetGrfIndex(const SvxBrushItem& rBrush)
{
    int nIndex = -1;

    const Graphic* pGraphic = rBrush.GetGraphic();
    if (pGraphic)
    {
        for (size_t i = 0; i < m_vecBulletPic.size(); ++i)
        {
            if (m_vecBulletPic[i]->GetChecksum() == pGraphic->GetChecksum())
            {
                nIndex = i;
                break;
            }
        }
    }

    return nIndex;
}

void WW8_WrtRedlineAuthor::Write( Writer& rWrt )
{
    WW8Export & rWW8Wrt = *(static_cast<SwWW8Writer&>(rWrt).m_pExport);
    rWW8Wrt.WriteAsStringTable(maAuthors, rWW8Wrt.pFib->m_fcSttbfRMark,
        rWW8Wrt.pFib->m_lcbSttbfRMark);
}

sal_uInt16 WW8Export::AddRedlineAuthor( std::size_t nId )
{
    if( !m_pRedlAuthors )
    {
        m_pRedlAuthors = new WW8_WrtRedlineAuthor;
        m_pRedlAuthors->AddName("Unknown");
    }
    return m_pRedlAuthors->AddName( SW_MOD()->GetRedlineAuthor( nId ) );
}

void WW8Export::WriteAsStringTable(const std::vector<OUString>& rStrings,
    sal_Int32& rfcSttbf, sal_Int32& rlcbSttbf)
{
    sal_uInt16 n, nCount = static_cast< sal_uInt16 >(rStrings.size());
    if( nCount )
    {
        // we have some Redlines found in the document -> the
        // Author Name Stringtable
        SvStream& rStrm = *pTableStrm;
        rfcSttbf = rStrm.Tell();
        SwWW8Writer::WriteShort( rStrm, -1 );
        SwWW8Writer::WriteLong( rStrm, nCount );
        for( n = 0; n < nCount; ++n )
        {
            const OUString& rNm = rStrings[n];
            SwWW8Writer::WriteShort( rStrm, rNm.getLength() );
            SwWW8Writer::WriteString16(rStrm, rNm, false);
        }
        rlcbSttbf = rStrm.Tell() - rfcSttbf;
    }
}

// WriteShort() sets at FilePos nPos the value nVal and seeks to the old
// FilePos. Used to insert lengths after the fact.
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

void SwWW8Writer::InsAsString16(ww::bytes &rO, const OUString& rStr)
{
    const sal_Unicode* pStr = rStr.getStr();
    for (sal_Int32 n = 0, nLen = rStr.getLength(); n < nLen; ++n, ++pStr)
        SwWW8Writer::InsUInt16( rO, *pStr );
}

void SwWW8Writer::InsAsString8(ww::bytes &rO, const OUString& rStr,
        rtl_TextEncoding eCodeSet)
{
    OString sTmp(OUStringToOString(rStr, eCodeSet));
    const sal_Char *pStart = sTmp.getStr();
    const sal_Char *pEnd = pStart + sTmp.getLength();
    rO.reserve(rO.size() + sTmp.getLength());

    std::copy(pStart, pEnd, std::inserter(rO, rO.end()));
}

void SwWW8Writer::WriteString16(SvStream& rStrm, const OUString& rStr,
    bool bAddZero)
{
    ww::bytes aBytes;
    SwWW8Writer::InsAsString16(aBytes, rStr);
    if (bAddZero)
        SwWW8Writer::InsUInt16(aBytes, 0);
    //vectors are guaranteed to have contiguous memory, so we can do
    //this while migrating away from WW8Bytes. Meyers Effective STL, item 16
    if (!aBytes.empty())
        rStrm.WriteBytes(&aBytes[0], aBytes.size());
}

void SwWW8Writer::WriteString_xstz(SvStream& rStrm, const OUString& rStr, bool bAddZero)
{
    ww::bytes aBytes;
    SwWW8Writer::InsUInt16(aBytes, rStr.getLength());
    SwWW8Writer::InsAsString16(aBytes, rStr);
    if (bAddZero)
        SwWW8Writer::InsUInt16(aBytes, 0);
    rStrm.WriteBytes(&aBytes[0], aBytes.size());
}

void SwWW8Writer::WriteString8(SvStream& rStrm, const OUString& rStr,
    bool bAddZero, rtl_TextEncoding eCodeSet)
{
    ww::bytes aBytes;
    SwWW8Writer::InsAsString8(aBytes, rStr, eCodeSet);
    if (bAddZero)
        aBytes.push_back(0);
    //vectors are guaranteed to have contiguous memory, so we can do
    ////this while migrating away from WW8Bytes. Meyers Effective STL, item 16
    if (!aBytes.empty())
        rStrm.WriteBytes(&aBytes[0], aBytes.size());
}

void WW8Export::WriteStringAsPara( const OUString& rText )
{
    if( !rText.isEmpty() )
        OutSwString(rText, 0, rText.getLength());
    WriteCR();              // CR thereafter

    ww::bytes aArr;
    SwWW8Writer::InsUInt16( aArr, 0/*nStyleId*/ );
    if( m_bOutTable )
    {                                               // Tab-Attr
        // sprmPFInTable
        SwWW8Writer::InsUInt16( aArr, NS_sprm::sprmPFInTable );
        aArr.push_back( 1 );
    }

    sal_uLong nPos = Strm().Tell();
    m_pPapPlc->AppendFkpEntry( nPos, aArr.size(), aArr.data() );
    m_pChpPlc->AppendFkpEntry( nPos );
}

void MSWordExportBase::WriteSpecialText( sal_uLong nStart, sal_uLong nEnd, sal_uInt8 nTTyp )
{
    sal_uInt8 nOldTyp = m_nTextTyp;
    m_nTextTyp = nTTyp;
    SwPaM* pOldPam = m_pCurPam;       //!! Simply shifting the PaM without restoring should do the job too
    sal_uLong nOldStart = m_nCurStart;
    sal_uLong nOldEnd = m_nCurEnd;
    SwPaM* pOldEnd = m_pOrigPam;
    bool bOldPageDescs = m_bOutPageDescs;
    m_bOutPageDescs = false;
    if ( nTTyp == TXT_FTN || nTTyp == TXT_EDN )
        m_bAddFootnoteTab = true;   // enable one aesthetic tab for this footnote

    SetCurPam(nStart, nEnd);

    // clear linked textboxes since old ones can't be linked to frames in this section
    m_aLinkedTextboxesHelper.clear();

    // tdf#106261 Reset table infos, otherwise the depth of the cells will be
    // incorrect, in case the header/footer had table(s) and we try to export
    // the same table second time.
    ww8::WW8TableInfo::Pointer_t pOldTableInfo = m_pTableInfo;
    m_pTableInfo = std::make_shared<ww8::WW8TableInfo>();

    WriteText();

    m_pTableInfo = pOldTableInfo;

    m_bOutPageDescs = bOldPageDescs;
    delete m_pCurPam;                    // delete Pam
    m_pCurPam = pOldPam;
    m_nCurStart = nOldStart;
    m_nCurEnd = nOldEnd;
    m_pOrigPam = pOldEnd;
    m_nTextTyp = nOldTyp;
}

void WW8Export::OutSwString(const OUString& rStr, sal_Int32 nStt,
    sal_Int32 const nLen)

{
    SAL_INFO( "sw.ww8.level2", "<OutSwString>" );

    if( nLen )
    {
        if( nStt || nLen != rStr.getLength() )
        {
            OUString sOut( rStr.copy( nStt, nLen ) );

            SAL_INFO( "sw.ww8.level2", sOut );

            SwWW8Writer::WriteString16(Strm(), sOut, false);
        }
        else
        {
            SAL_INFO( "sw.ww8.level2", rStr );

            SwWW8Writer::WriteString16(Strm(), rStr, false);
        }
    }

    SAL_INFO( "sw.ww8.level2", "</OutSwString>" );
}

void WW8Export::WriteCR(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    if (pTableTextNodeInfoInner.get() != nullptr && pTableTextNodeInfoInner->getDepth() == 1 && pTableTextNodeInfoInner->isEndOfCell())
        WriteChar('\007');
    else
        WriteChar( '\015' );

    m_pPiece->SetParaBreak();
}

void WW8Export::WriteChar( sal_Unicode c )
{
    Strm().WriteUInt16( c );
}

void MSWordExportBase::SetCurPam(sal_uLong nStt, sal_uLong nEnd)
{
    m_nCurStart = nStt;
    m_nCurEnd = nEnd;
    m_pCurPam = Writer::NewSwPaM( *m_pDoc, nStt, nEnd );

    // Recognize tables in special cases
    if ( nStt != m_pCurPam->GetMark()->nNode.GetIndex() &&
         m_pDoc->GetNodes()[ nStt ]->IsTableNode() )
    {
        m_pCurPam->GetMark()->nNode = nStt;
    }

    m_pOrigPam = m_pCurPam;
    m_pCurPam->Exchange();
}

void MSWordExportBase::SaveData( sal_uLong nStt, sal_uLong nEnd )
{
    MSWordSaveData aData;

    // WW8Export only stuff - zeroed here not to issue warnings
    aData.pOOld = nullptr;

    // Common stuff
    aData.pOldPam = m_pCurPam;
    aData.pOldEnd = m_pOrigPam;
    aData.pOldFlyFormat = m_pParentFrame;
    aData.pOldPageDesc = m_pCurrentPageDesc;

    aData.pOldFlyOffset = m_pFlyOffset;
    aData.eOldAnchorType = m_eNewAnchorType;

    aData.bOldOutTable = m_bOutTable;
    aData.bOldFlyFrameAttrs = m_bOutFlyFrameAttrs;
    aData.bOldStartTOX = m_bStartTOX;
    aData.bOldInWriteTOX = m_bInWriteTOX;

    SetCurPam(nStt, nEnd);

    m_bOutTable = false;
    // Caution: bIsInTable should not be set here
    m_bOutFlyFrameAttrs = false;
    m_bStartTOX = false;
    m_bInWriteTOX = false;

    m_aSaveData.push( std::move(aData) );
}

void MSWordExportBase::RestoreData()
{
    MSWordSaveData &rData = m_aSaveData.top();

    delete m_pCurPam;
    m_pCurPam = rData.pOldPam;
    m_nCurStart = rData.nOldStart;
    m_nCurEnd = rData.nOldEnd;
    m_pOrigPam = rData.pOldEnd;

    m_bOutTable = rData.bOldOutTable;
    m_bOutFlyFrameAttrs = rData.bOldFlyFrameAttrs;
    m_bStartTOX = rData.bOldStartTOX;
    m_bInWriteTOX = rData.bOldInWriteTOX;

    m_pParentFrame = rData.pOldFlyFormat;
    m_pCurrentPageDesc = rData.pOldPageDesc;

    m_eNewAnchorType = rData.eOldAnchorType;
    m_pFlyOffset = rData.pOldFlyOffset;

    m_aSaveData.pop();
}

void WW8Export::SaveData( sal_uLong nStt, sal_uLong nEnd )
{
    MSWordExportBase::SaveData( nStt, nEnd );

    MSWordSaveData &rData = m_aSaveData.top();

    if ( !pO->empty() )
    {
        rData.pOOld = std::move(pO);
        pO.reset(new ww::bytes);
    }
    else
        rData.pOOld = nullptr; // reuse pO

    rData.bOldWriteAll = GetWriter().m_bWriteAll;
    GetWriter().m_bWriteAll = true;
}

void WW8Export::RestoreData()
{
    MSWordSaveData &rData = m_aSaveData.top();

    GetWriter().m_bWriteAll = rData.bOldWriteAll;

    OSL_ENSURE( pO->empty(), "pO is not empty in WW8Export::RestoreData()" );
    if ( rData.pOOld )
    {
        pO = std::move(rData.pOOld);
    }

    MSWordExportBase::RestoreData();
}

void WW8AttributeOutput::TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    sal_uInt32 nDepth = pTableTextNodeInfoInner->getDepth();

    if ( nDepth > 0 )
    {
        /* Cell */
        m_rWW8Export.InsUInt16( NS_sprm::sprmPFInTable );
        m_rWW8Export.pO->push_back( sal_uInt8(0x1) );
        m_rWW8Export.InsUInt16( NS_sprm::sprmPItap );
        m_rWW8Export.InsUInt32( nDepth );

        if ( nDepth > 1 && pTableTextNodeInfoInner->isEndOfCell() )
        {
            m_rWW8Export.InsUInt16( NS_sprm::sprmPFInnerTableCell );
            m_rWW8Export.pO->push_back( sal_uInt8(0x1) );
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
            m_rWW8Export.InsUInt16( NS_sprm::sprmPFInTable );
            m_rWW8Export.pO->push_back( sal_uInt8(0x1) );

            if ( nDepth == 1 )
            {
                m_rWW8Export.InsUInt16( NS_sprm::sprmPFTtp );
                m_rWW8Export.pO->push_back( sal_uInt8(0x1) );
            }

            m_rWW8Export.InsUInt16( NS_sprm::sprmPItap );
            m_rWW8Export.InsUInt32( nDepth );

            if ( nDepth > 1 )
            {
                m_rWW8Export.InsUInt16( NS_sprm::sprmPFInnerTableCell );
                m_rWW8Export.pO->push_back( sal_uInt8(0x1) );
                m_rWW8Export.InsUInt16( NS_sprm::sprmPFInnerTtp );
                m_rWW8Export.pO->push_back( sal_uInt8(0x1) );
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
            TableCellBorders( pTableTextNodeInfoInner );
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

    if (pBox != nullptr)
    {
        const SwFrameFormat * pFormat = pBox->GetFrameFormat();
        switch (pFormat->GetVertOrient().GetVertOrient())
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
            const SwContentNode * pCNd = pSttNd->GetNodes().GoNext( &aIdx );
            if( pCNd && pCNd->IsTextNode())
            {
                SfxItemSet aCoreSet(rDoc.GetAttrPool(), svl::Items<RES_CHRATR_ROTATE, RES_CHRATR_ROTATE>{});
                static_cast<const SwTextNode*>(pCNd)->GetParaAttr(aCoreSet,
                    0, static_cast<const SwTextNode*>(pCNd)->GetText().getLength());
                const SfxPoolItem * pRotItem;
                if ( SfxItemState::SET == aCoreSet.GetItemState(RES_CHRATR_ROTATE, true, &pRotItem))
                {
                    const SvxCharRotateItem * pRotate = static_cast<const SvxCharRotateItem*>(pRotItem);
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
    const SwTableBoxes & rTableBoxes = pTabLine->GetTabBoxes();

    sal_uInt8 nBoxes = rTableBoxes.size();
    for ( sal_uInt8 n = 0; n < nBoxes; n++ )
    {
        const SwTableBox * pTabBox1 = rTableBoxes[n];
        const SwFrameFormat * pFrameFormat = pTabBox1->GetFrameFormat();

        if ( SvxFrameDirection::Vertical_RL_TB == m_rWW8Export.TrueFrameDirection( *pFrameFormat ) )
        {
            m_rWW8Export.InsUInt16( NS_sprm::sprmTTextFlow );
            m_rWW8Export.pO->push_back( n );                   //start range
            m_rWW8Export.pO->push_back( sal_uInt8(n + 1) );    //end range
            m_rWW8Export.InsUInt16( 5 ); //Equals vertical writing
        }
    }
}

void WW8AttributeOutput::TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwFrameFormat * pLineFormat = pTabLine->GetFrameFormat();

    /*
     By default the row can be split in word, and now in writer we have a
     feature equivalent to this, Word stores 1 for fCantSplit if the row
     cannot be split, we set true if we can split it. An example is #i4569#
     */

    const SwFormatRowSplit& rSplittable = pLineFormat->GetRowSplit();
    sal_uInt8 nCantSplit = (!rSplittable.GetValue()) ? 1 : 0;
    m_rWW8Export.InsUInt16( NS_sprm::sprmTFCantSplit );
    m_rWW8Export.pO->push_back( nCantSplit );
    m_rWW8Export.InsUInt16( NS_sprm::sprmTFCantSplit90 ); // also write fCantSplit90
    m_rWW8Export.pO->push_back( nCantSplit );
}

void WW8AttributeOutput::TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();
    const SwFrameFormat * pFrameFormat = pTable->GetFrameFormat();

    if ( m_rWW8Export.TrueFrameDirection(*pFrameFormat) == SvxFrameDirection::Horizontal_RL_TB )
    {
        m_rWW8Export.InsUInt16( NS_sprm::sprmTFBiDi );
        m_rWW8Export.InsUInt16( 1 );
    }
}

void WW8AttributeOutput::TableRowRedline( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
}

void WW8AttributeOutput::TableCellRedline( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
}

void WW8AttributeOutput::TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwFrameFormat * pLineFormat = pTabLine->GetFrameFormat();

    // output line height   sprmTDyaRowHeight
    long nHeight = 0;
    const SwFormatFrameSize& rLSz = pLineFormat->GetFrameSize();
    if ( ATT_VAR_SIZE != rLSz.GetHeightSizeType() && rLSz.GetHeight() )
    {
        if ( ATT_MIN_SIZE == rLSz.GetHeightSizeType() )
            nHeight = rLSz.GetHeight();
        else
            nHeight = -rLSz.GetHeight();
    }

    if ( nHeight )
    {
        m_rWW8Export.InsUInt16( NS_sprm::sprmTDyaRowHeight );
        m_rWW8Export.InsUInt16( static_cast<sal_uInt16>(nHeight) );
    }

}

void WW8AttributeOutput::TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();

    const SwFrameFormat *pFormat = pTable->GetFrameFormat();
    if ( !pFormat )
    {
        SAL_WARN( "sw.ww8", "FrameFormat is nil" );
        return;
    }

    const SwFormatHoriOrient &rHori = pFormat->GetHoriOrient();
    const SwFormatVertOrient &rVert = pFormat->GetVertOrient();

    if (
        (text::RelOrientation::PRINT_AREA == rHori.GetRelationOrient() ||
         text::RelOrientation::FRAME == rHori.GetRelationOrient())
        &&
        (text::RelOrientation::PRINT_AREA == rVert.GetRelationOrient() ||
         text::RelOrientation::FRAME == rVert.GetRelationOrient())
        )
    {
        const bool bIsRTL = m_rWW8Export.TrueFrameDirection(*pFormat) == SvxFrameDirection::Horizontal_RL_TB;
        sal_Int16 eHOri = rHori.GetHoriOrient();
        switch (eHOri)
        {
            case text::HoriOrientation::CENTER:
                m_rWW8Export.InsUInt16( NS_sprm::sprmTJc ); //logical orientation required for MSO
                m_rWW8Export.InsUInt16( 1 );
                m_rWW8Export.InsUInt16( NS_sprm::sprmTJc90 ); //physical orientation required for LO
                m_rWW8Export.InsUInt16( 1 );
                break;
            case text::HoriOrientation::RIGHT:
                m_rWW8Export.InsUInt16( NS_sprm::sprmTJc90 ); //required for LO
                m_rWW8Export.InsUInt16( 2 );
                if ( !bIsRTL )
                {
                    m_rWW8Export.InsUInt16( NS_sprm::sprmTJc ); //required for MSO
                    m_rWW8Export.InsUInt16( 2 );
                }
                break;
            case text::HoriOrientation::LEFT:
                if ( bIsRTL )
                {
                    m_rWW8Export.InsUInt16( NS_sprm::sprmTJc ); //required for MSO
                    m_rWW8Export.InsUInt16( 2 );
                }
                break;
            case text::HoriOrientation::LEFT_AND_WIDTH:
                // Width can only be specified for the LOGICAL left, so in RTL, that is always PHYSICAL right
                if ( bIsRTL )
                {
                    m_rWW8Export.InsUInt16( NS_sprm::sprmTJc90 ); //required for LO
                    m_rWW8Export.InsUInt16( 2 );
                }
                break;
            default:
                break;
        }
    }
}

void WW8AttributeOutput::TableSpacing(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner)
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();
    const SwTableFormat* pTableFormat = pTable->GetFrameFormat();


    // Writing these SPRM's will make the table a floating one, so only write
    // them in case the table is already inside a frame.
    if (pTableFormat != nullptr && pTable->GetTableNode()->GetFlyFormat())
    {
        const SvxULSpaceItem & rUL = pTableFormat->GetULSpace();

        if (rUL.GetUpper() > 0)
        {
            sal_uInt8 const nPadding = 2;
            sal_uInt8 const nPcVert = 0;
            sal_uInt8 const nPcHorz = 0;

            sal_uInt8 const nTPc = (nPadding << 4) | (nPcVert << 2) | nPcHorz;

            m_rWW8Export.InsUInt16(NS_sprm::sprmTPc);
            m_rWW8Export.pO->push_back( nTPc );

            m_rWW8Export.InsUInt16(NS_sprm::sprmTDyaAbs);
            m_rWW8Export.InsUInt16(rUL.GetUpper());

            m_rWW8Export.InsUInt16(NS_sprm::sprmTDyaFromText);
            m_rWW8Export.InsUInt16(rUL.GetUpper());
        }

        if (rUL.GetLower() > 0)
        {
            m_rWW8Export.InsUInt16(NS_sprm::sprmTDyaFromTextBottom);
            m_rWW8Export.InsUInt16(rUL.GetLower());
        }
    }
}

void WW8AttributeOutput::TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable * pTable = pTableTextNodeInfoInner->getTable();

    if ( pTable->GetRowsToRepeat() > pTableTextNodeInfoInner->getRow() )
    {
        m_rWW8Export.InsUInt16( NS_sprm::sprmTTableHeader );
        m_rWW8Export.pO->push_back( 1 );
    }

    ww8::TableBoxVectorPtr pTableBoxes =
        pTableTextNodeInfoInner->getTableBoxesOfRow();
    // number of cell written
    sal_uInt32 nBoxes = pTableBoxes->size();
    assert(nBoxes <= ww8::MAXTABLECELLS);

    // sprm header
    m_rWW8Export.InsUInt16( NS_sprm::sprmTDefTable );
    sal_uInt16 nSprmSize = 2 + (nBoxes + 1) * 2 + nBoxes * 20;
    m_rWW8Export.InsUInt16( nSprmSize ); // length

    // number of boxes
    m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(nBoxes) );

    /* cells */
    /*
     ALWAYS relative when text::HoriOrientation::NONE (nPageSize + ( nPageSize / 10 )) < nTableSz,
     in that case the cell width's and table width's are not real. The table
     width is maxed and cells relative, so we need the frame (generally page)
     width that the table is in to work out the true widths.
     */
    //const bool bNewTableModel = pTable->IsNewModel();
    const SwFrameFormat *pFormat = pTable->GetFrameFormat();
    if ( !pFormat )
    {
        SAL_WARN( "sw.ww8", "FrameFormat is nil" );
        return;
    }

    const SwFormatHoriOrient &rHori = pFormat->GetHoriOrient();
    const SwFormatVertOrient &rVert = pFormat->GetVertOrient();

    SwTwips nTableOffset = 0;

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
                nTableOffset = rHori.GetPos();
                const SvxLRSpaceItem& rLRSp = pFormat->GetLRSpace();
                nTableOffset += rLRSp.GetLeft();

                // convert offset to be measured from right margin in right-to-left tables
                if ( nTableOffset && m_rWW8Export.TrueFrameDirection(*pFormat) == SvxFrameDirection::Horizontal_RL_TB )
                {
                    SwTwips nLeftPageMargin, nRightPageMargin;
                    const SwTwips nPageSize = m_rWW8Export.CurrentPageWidth(nLeftPageMargin, nRightPageMargin);
                    const SwTwips nTableWidth = pFormat->GetFrameSize().GetWidth();
                    nTableOffset = nPageSize - nLeftPageMargin - nRightPageMargin - nTableWidth - nTableOffset;
                }
                break;
        }
    }

    m_rWW8Export.InsInt16( nTableOffset );

    ww8::GridColsPtr pGridCols = GetGridCols( pTableTextNodeInfoInner );
    for ( const auto nCol : *pGridCols )
     {
         m_rWW8Export.InsUInt16( static_cast<sal_uInt16>(nCol) + nTableOffset );
     }

     /* TCs */
    ww8::RowSpansPtr pRowSpans = pTableTextNodeInfoInner->getRowSpansOfRow();
    ww8::RowSpans::const_iterator aItRowSpans = pRowSpans->begin();

    for (const SwTableBox * pTabBox1 : *pTableBoxes)
    {
        sal_uInt16 npOCount = m_rWW8Export.pO->size();

        const SwFrameFormat * pBoxFormat = nullptr;
        if (pTabBox1 != nullptr)
            pBoxFormat = pTabBox1->GetFrameFormat();

        sal_uInt16 nFlags =
            lcl_TCFlags(*m_rWW8Export.m_pDoc, pTabBox1, *aItRowSpans);
        m_rWW8Export.InsUInt16( nFlags );

        static sal_uInt8 aNullBytes[] = { 0x0, 0x0 };

        m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), aNullBytes, aNullBytes+2 );   // dummy
        if (pBoxFormat != nullptr)
        {
            const SvxBoxItem & rBoxItem = pBoxFormat->GetBox();

            WW8Export::Out_SwFormatTableBox( *m_rWW8Export.pO, &rBoxItem ); // 8/16 Byte
        }
        else
            WW8Export::Out_SwFormatTableBox( *m_rWW8Export.pO, nullptr); // 8/16 Byte

        SAL_INFO( "sw.ww8.level2", "<tclength>" << ( m_rWW8Export.pO->size() - npOCount ) << "</tclength>" );
        ++aItRowSpans;
    }

    int nWidthPercent = pFormat->GetFrameSize().GetWidthPercent();
    // Width is in fiftieths of a percent. For sprmTTableWidth, must be non-negative and 600% max
    if ( nWidthPercent > 0 && nWidthPercent <= 600 )
    {
        m_rWW8Export.InsUInt16( NS_sprm::sprmTTableWidth );
        m_rWW8Export.pO->push_back( sal_uInt8/*ftsPercent*/ (2) );
        m_rWW8Export.InsUInt16( static_cast<sal_uInt16>(nWidthPercent) * 50 );
    }
}

ww8::GridColsPtr AttributeOutputBase::GetGridCols( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner )
{
    return pTableTextNodeInfoInner->getGridColsOfRow(*this);
}

ww8::WidthsPtr AttributeOutputBase::GetColumnWidths( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner )
{
    // Get the column widths based on ALL the rows, not just the current row
    return pTableTextNodeInfoInner->getGridColsOfRow(*this, true);
}

void AttributeOutputBase::GetTablePageSize( ww8::WW8TableNodeInfoInner const * pTableTextNodeInfoInner, long& rPageSize, bool& rRelBoxSize )
{
    long nPageSize = 0;

    const SwNode *pTextNd = pTableTextNodeInfoInner->getNode( );
    const SwTable *pTable = pTableTextNodeInfoInner->getTable( );

    const SwFrameFormat *pFormat = pTable->GetFrameFormat();
    if ( !pFormat )
    {
        SAL_WARN( "sw.ww8", "FrameFormat is nil" );
        return;
    }

    const SwFormatFrameSize &rSize = pFormat->GetFrameSize();
    int nWidthPercent = rSize.GetWidthPercent();
    bool bManualAligned = pFormat->GetHoriOrient().GetHoriOrient() == text::HoriOrientation::NONE;
    if ( (pFormat->GetHoriOrient().GetHoriOrient() == text::HoriOrientation::FULL) || bManualAligned )
        nWidthPercent = 100;
    bool bRelBoxSize = nWidthPercent != 0;
    unsigned long nTableSz = static_cast<unsigned long>(rSize.GetWidth());
    if (nTableSz > USHRT_MAX/2 && !bRelBoxSize)
    {
        OSL_ENSURE(bRelBoxSize, "huge table width but not relative, suspicious");
        bRelBoxSize = true;
    }

    if ( bRelBoxSize )
    {
        Point aPt;
        SwRect aRect( pFormat->FindLayoutRect( false, &aPt ) );
        if ( aRect.IsEmpty() )
        {
            // Then fetch the page width without margins!
            const SwFrameFormat* pParentFormat =
                GetExport().m_pParentFrame ?
                &(GetExport().m_pParentFrame->GetFrameFormat()) :
                    GetExport().m_pDoc->GetPageDesc(0).GetPageFormatOfNode(*pTextNd, false);
            aRect = pParentFormat->FindLayoutRect(true);
            if ( 0 == ( nPageSize = aRect.Width() ) )
            {
                const SvxLRSpaceItem& rLR = pParentFormat->GetLRSpace();
                nPageSize = pParentFormat->GetFrameSize().GetWidth() - rLR.GetLeft()
                - rLR.GetRight();
            }
        }
        else
        {
            nPageSize = aRect.Width();
            if ( bManualAligned )
            {
                // #i37571# For manually aligned tables
                const SvxLRSpaceItem &rLR = pFormat->GetLRSpace();
                nPageSize -= (rLR.GetLeft() + rLR.GetRight());
            }

        }

        if ( nWidthPercent )
        {
            nPageSize *= nWidthPercent;
            nPageSize /= 100;
        }
        else
            SAL_WARN( "sw.ww8", "nWidthPercent is zero" );
    }
    else
    {
        // As the table width is not relative, the TablePageSize equals its width
        nPageSize = nTableSz;
    }

    rPageSize = nPageSize;
    rRelBoxSize = bRelBoxSize;
}

void WW8AttributeOutput::TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwFrameFormat * pFrameFormat = pTabBox->GetFrameFormat();

    //Set Default, just taken from the first cell of the first
    //row
    static const SvxBoxItemLine aBorders[] =
    {
        SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT,
        SvxBoxItemLine::BOTTOM, SvxBoxItemLine::RIGHT
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
                pFrameFormat->GetBox().GetDistance( aBorders[i] ) );
    }
}

void WW8AttributeOutput::TableCellBorders(
    ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner )
{
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwTableBoxes & rTabBoxes = pTabLine->GetTabBoxes();
    sal_uInt8 nBoxes = std::min<size_t>(rTabBoxes.size(), 255);
    const SvxBoxItem * pLastBox = nullptr;
    sal_uInt8 nSeqStart = 0; // start of sequence of cells with same borders

    // Detect sequences of cells which have the same borders, and output
    // a border description for each such cell range.
    for ( unsigned n = 0; n <= nBoxes; ++n )
    {
        const SvxBoxItem * pBox = (n == nBoxes) ? nullptr :
            &rTabBoxes[n]->GetFrameFormat()->GetBox();
        if( !pLastBox )
            pLastBox = pBox;
        else if( !pBox || *pLastBox != *pBox )
        {
            // This cell has different borders than the previous cell,
            // so output the borders for the preceding cell range.
            m_rWW8Export.Out_CellRangeBorders(pLastBox, nSeqStart, n);
            nSeqStart = n;
            pLastBox = pBox;
        }
    }
}

void WW8AttributeOutput::TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    const SwTable * pTab = pTableTextNodeInfoInner->getTable();
    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwTableBoxes & rTabBoxes = pTabLine->GetTabBoxes();

    sal_uInt8 nBoxes = rTabBoxes.size();
    m_rWW8Export.InsUInt16( NS_sprm::sprmTDefTableShd80 );
    m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(nBoxes * 2) );  // Len

    Color aRowColor = COL_AUTO;
    const SvxBrushItem *pTableColorProp = pTab->GetFrameFormat()->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    if ( pTableColorProp )
        aRowColor = pTableColorProp->GetColor();

    const SvxBrushItem *pRowColorProp = pTabLine->GetFrameFormat()->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    if ( pRowColorProp && pRowColorProp->GetColor() != COL_AUTO )
        aRowColor = pRowColorProp->GetColor();

    for ( sal_uInt8 n = 0; n < nBoxes; n++ )
    {
        const SwTableBox * pBox1 = rTabBoxes[n];
        const SwFrameFormat * pFrameFormat = pBox1->GetFrameFormat();
        Color aColor = aRowColor;

        const SvxBrushItem *pCellColorProp = pFrameFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
        if ( pCellColorProp && pCellColorProp->GetColor() != COL_AUTO )
            aColor = pCellColorProp->GetColor();

        WW8_SHD aShd;
        WW8Export::TransBrush( aColor, aShd );
        m_rWW8Export.InsUInt16( aShd.GetValue() );
    }

    sal_uInt32 const aSprmIds[] { NS_sprm::sprmTDefTableShd,
                                  NS_sprm::sprmTDefTableShdRaw };
    sal_uInt8 nBoxes0 = rTabBoxes.size();
    if (nBoxes0 > 21)
        nBoxes0 = 21;

    for (sal_uInt32 m : aSprmIds)
    {
        m_rWW8Export.InsUInt16( m );
        m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(nBoxes0 * 10) );

        for ( sal_uInt8 n = 0; n < nBoxes0; n++ )
        {
            const SwTableBox * pBox1 = rTabBoxes[n];
            const SwFrameFormat * pFrameFormat = pBox1->GetFrameFormat();
            Color aColor = aRowColor;

            const SvxBrushItem *pCellColorProp = pFrameFormat->GetAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
            if ( pCellColorProp && pCellColorProp->GetColor() != COL_AUTO )
                aColor = pCellColorProp->GetColor();

            WW8SHDLong aSHD;
            aSHD.setCvFore( 0xFF000000 );

            if ( aColor == COL_AUTO )
                aSHD.setCvBack( 0xFF000000 );
            else
                aSHD.setCvBack( wwUtility::RGBToBGR( aColor ) );

            aSHD.Write( m_rWW8Export );
        }
    }
}

void WW8Export::SectionBreaksAndFrames( const SwTextNode& rNode )
{
    // output page/section breaks
    OutputSectionBreaks( rNode.GetpSwAttrSet(), rNode );
}

class TrackContentToExport
{
private:
    SwPaM *m_pCurPam;
    sal_uLong m_nStart, m_nEnd;
public:
    TrackContentToExport(SwPaM *pCurPam, sal_uLong nCurStart, sal_uLong nCurEnd)
        : m_pCurPam(pCurPam)
        , m_nStart(nCurStart)
        , m_nEnd(nCurEnd)
    {
    }

    bool contentRemainsToExport(ww8::WW8TableInfo *pTableInfo)
    {
        bool bSimpleContentRemains = m_pCurPam->GetPoint()->nNode < m_pCurPam->GetMark()->nNode ||
            (m_pCurPam->GetPoint()->nNode == m_pCurPam->GetMark()->nNode &&
              m_pCurPam->GetPoint()->nContent.GetIndex() <= m_pCurPam->GetMark()->nContent.GetIndex());
        if (bSimpleContentRemains)
            return true;

        if (!pTableInfo)
            return false;

        //An old-school table where one cell may points back to a previous node as the next cell
        //so if this node is the last node in the range, we may need to jump back to a previously
        //skipped cell to output it in a sane sequence. See ooo47778-3.sxw for one of these
        //horrors. So if we are at the end of the selection, but this end point is a table
        //cell whose next cell is in the selection allow jumping back to it
        const SwNode* pCurrentNode = &m_pCurPam->GetPoint()->nNode.GetNode();
        const SwNode* pNextNode = pTableInfo->getNextNode(pCurrentNode);

        if (pNextNode && pCurrentNode != pNextNode)
        {
            return pNextNode->GetIndex() >= m_nStart &&
                   pNextNode->GetIndex() < m_nEnd;
        }

        return false;
    }
};

void MSWordExportBase::WriteText()
{
    TrackContentToExport aContentTracking(m_pCurPam, m_nCurStart, m_nCurEnd);
    while (aContentTracking.contentRemainsToExport(m_pTableInfo.get()))
    {
        SwNode& rNd = m_pCurPam->GetNode();

        // no section breaks exported for Endnotes
        if ( rNd.IsTextNode() && m_nTextTyp != TXT_EDN && m_nTextTyp != TXT_FTN )
        {
            SwSoftPageBreakList breakList;
            // if paragraph need to be split than handle section break somewhere
            // else.
            if( !NeedTextNodeSplit( *rNd.GetTextNode(), breakList) )
                SectionBreaksAndFrames( *rNd.GetTextNode() );
        }


        // output the various types of nodes
        if ( rNd.IsContentNode() )
        {
            SwContentNode* pCNd = static_cast<SwContentNode*>(&rNd);

            const SwPageDesc* pTemp = rNd.FindPageDesc();
            if ( pTemp )
                m_pCurrentPageDesc = pTemp;

            m_pCurPam->GetPoint()->nContent.Assign( pCNd, 0 );
            OutputContentNode( *pCNd );
        }
        else if ( rNd.IsTableNode() )
        {
            m_pTableInfo->processSwTable( &rNd.GetTableNode()->GetTable() );
        }
        else if ( rNd.IsSectionNode() && TXT_MAINTEXT == m_nTextTyp )
            OutputSectionNode( *rNd.GetSectionNode() );
        else if ( TXT_MAINTEXT == m_nTextTyp && rNd.IsEndNode() &&
                  rNd.StartOfSectionNode()->IsSectionNode() )
        {
            const SwSection& rSect = rNd.StartOfSectionNode()->GetSectionNode()
                                        ->GetSection();
            if ( m_bStartTOX && TOX_CONTENT_SECTION == rSect.GetType() )
                m_bStartTOX = false;

            SwNodeIndex aIdx( rNd, 1 );
            if ( aIdx.GetNode().IsEndNode() && aIdx.GetNode().StartOfSectionNode()->IsSectionNode() )
                ;
            else if ( aIdx.GetNode().IsSectionNode() )
                ;
            else if ( !IsInTable()
                && (rSect.GetType() != TOX_CONTENT_SECTION && rSect.GetType() != TOX_HEADER_SECTION )) //No sections in table
            {
                //#120140# Do not need to insert a page/section break after a section end. Check this case first
                bool bNeedExportBreakHere = true;
                if ( aIdx.GetNode().IsTextNode() )
                {
                    SwTextNode *pTempNext = aIdx.GetNode().GetTextNode();
                    if ( pTempNext )
                    {
                        const SfxPoolItem * pTempItem = nullptr;
                        if (pTempNext->GetpSwAttrSet() && SfxItemState::SET == pTempNext->GetpSwAttrSet()->GetItemState(RES_PAGEDESC, false, &pTempItem)
                            && pTempItem && static_cast<const SwFormatPageDesc*>(pTempItem)->GetRegisteredIn())
                        {
                            //Next node has a new page style which means this node is a section end. Do not insert another page/section break here
                            bNeedExportBreakHere = false;
                        }
                    }
                }
                else
                {
                    /* Do not export Section Break in case DOCX containing MultiColumn and
                     * aIdx.GetNode().IsTextNode() is False i.e. Text node is NULL.
                     */
                    const SwFrameFormat* pPgFormat = rSect.GetFormat();
                    const SwFormatCol& rCol = pPgFormat->GetCol();
                    sal_uInt16 nColumnCount = rCol.GetNumCols();
                    const SwFormatNoBalancedColumns& rNoBalanced = pPgFormat->GetBalancedColumns();
                    // Prevent the additional section break only for non-balanced columns.
                    if (nColumnCount > 1 && rNoBalanced.GetValue())
                    {
                        bNeedExportBreakHere = false;
                    }
                    // No need to create a "fake" section if this is the end of the document,
                    // except to emulate balanced columns.
                    else if ( nColumnCount < 2 && aIdx == m_pDoc->GetNodes().GetEndOfContent() )
                        bNeedExportBreakHere = false;
                }

                if (bNeedExportBreakHere)  //#120140# End of check
                {
                    ReplaceCr( char(0xc) ); // indicator for Page/Section-Break

                    const SwSectionFormat* pParentFormat = rSect.GetFormat()->GetParent();
                    if ( !pParentFormat )
                        pParentFormat = reinterpret_cast<SwSectionFormat*>(sal_IntPtr(-1));

                    sal_uLong nRstLnNum;
                    if ( aIdx.GetNode().IsContentNode() )
                        nRstLnNum = static_cast<SwContentNode&>(aIdx.GetNode()).GetSwAttrSet().
                                                GetLineNumber().GetStartValue();
                    else
                        nRstLnNum = 0;

                    AppendSection( m_pCurrentPageDesc, pParentFormat, nRstLnNum );
                }
                else
                {
                    OutputEndNode( *rNd.GetEndNode() );
                }
            }
        }
        else if ( rNd.IsStartNode() )
        {
            OutputStartNode( *rNd.GetStartNode() );
        }
        else if ( rNd.IsEndNode() )
        {
            OutputEndNode( *rNd.GetEndNode() );
        }

        if ( &rNd == &rNd.GetNodes().GetEndOfContent() )
            break;

        const SwNode * pCurrentNode = &m_pCurPam->GetPoint()->nNode.GetNode();
        const SwNode * pNextNode = m_pTableInfo->getNextNode(pCurrentNode);

        if (pCurrentNode == pNextNode)
        {
            SAL_WARN("sw.ww8", "loop in TableInfo");
            pNextNode = nullptr;
        }

        if (pNextNode != nullptr)
            m_pCurPam->GetPoint()->nNode.Assign(*pNextNode);
        else
            ++m_pCurPam->GetPoint()->nNode;

        sal_uLong nPos = m_pCurPam->GetPoint()->nNode.GetIndex();
        ::SetProgressState( nPos, m_pCurPam->GetDoc()->GetDocShell() );
    }

    SAL_INFO( "sw.ww8.level2", "</WriteText>" );
}

void WW8Export::WriteMainText()
{
    SAL_INFO( "sw.ww8.level2", "<WriteMainText>" );

    pFib->m_fcMin = Strm().Tell();

    m_pCurPam->GetPoint()->nNode = m_pDoc->GetNodes().GetEndOfContent().StartOfSectionNode()->GetIndex();

    WriteText();

    if( 0 == Strm().Tell() - pFib->m_fcMin )  // no text ?
        WriteCR();                  // then CR at the end ( otherwise WW will complain )

    pFib->m_ccpText = Fc2Cp( Strm().Tell() );
    m_pFieldMain->Finish( pFib->m_ccpText, 0 );

                    // ccpText includes Footnote and KF-text
                    // therefore pFib->ccpText may get updated as well
    // save the StyleId of the last paragraph. Because WW97 take the style
    // from the last CR, that will be written after footer/Header/footnotes/
    // annotation etc.
    const SwTextNode* pLastNd = m_pCurPam->GetMark()->nNode.GetNode().GetTextNode();
    if( pLastNd )
        m_nLastFormatId = GetId( static_cast<SwTextFormatColl&>(pLastNd->GetAnyFormatColl()) );

    SAL_INFO( "sw.ww8.level2", "</WriteMainText>" );
}

bool MSWordExportBase::IsInTable() const
{
    bool bResult = false;

    if (m_pCurPam != nullptr)
    {
        SwNode& rNode = m_pCurPam->GetNode();

        if (m_pTableInfo.get() != nullptr)
        {
            ww8::WW8TableNodeInfo::Pointer_t pTableNodeInfo = m_pTableInfo->getTableNodeInfo(&rNode);

            if (pTableNodeInfo.get() != nullptr && pTableNodeInfo->getDepth() > 0)
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
    // Graphics in the data stream
    m_pGrf->Write();                          // Graphics

    // output into WordDocument stream
    m_pChpPlc->WriteFkps();                   // Fkp.Chpx
    m_pPapPlc->WriteFkps();                   // Fkp.Papx
    pSepx->WriteSepx( Strm() );             // Sepx

    // output into Table stream
    m_pStyles->OutputStylesTable();           // for WW8 StyleTab
    pFootnote->WritePlc( *this );                // Footnote-Ref & Text Plc
    pEdn->WritePlc( *this );                // Endnote-Ref & Text Plc
    m_pTextBxs->WritePlc( *this );             // Textbox Text Plc
    m_pHFTextBxs->WritePlc( *this );           // Head/Foot-Textbox Text Plc
    m_pAtn->WritePlc( *this );                // Annotation-Ref & Text Plc

    pSepx->WritePlcSed( *this );            // Slcx.PlcSed
    pSepx->WritePlcHdd( *this );            // Slcx.PlcHdd

    m_pChpPlc->WritePlc();                    // Plcx.Chpx
    m_pPapPlc->WritePlc();                    // Plcx.Papx

    if( m_pRedlAuthors )
        m_pRedlAuthors->Write( GetWriter() );       // sttbfRMark (RedlineAuthors)
    m_pFieldMain->Write( *this );               // Fields ( Main Text )
    m_pFieldHdFt->Write( *this );               // Fields ( Header/Footer )
    m_pFieldFootnote->Write( *this );                // Fields ( FootNotes )
    m_pFieldEdn->Write( *this );                // Fields ( EndNotes )
    m_pFieldAtn->Write( *this );                // Fields ( Annotations )
    m_pFieldTextBxs->Write( *this );             // Fields ( Textboxes )
    m_pFieldHFTextBxs->Write( *this );           // Fields ( Head/Foot-Textboxes )

    if (m_pEscher || m_pDoc->ContainsMSVBasic())
    {
        /*
         Every time MS 2000 creates an escher stream there is always
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
         some magic.
        */
        // avoid memory leak #i120098#, the unnamed obj will be released in destructor.
        xEscherStg = GetWriter().GetStorage().OpenSotStorage(SL::aObjectPool);
    }

    // dggInfo - escher stream
    WriteEscher();

    m_pSdrObjs->WritePlc( *this );
    m_pHFSdrObjs->WritePlc( *this );
    // spamom - office drawing table
    // spahdr - header office drawing table

    m_pBkmks->Write( *this );                 // Bookmarks - sttbfBkmk/
                                            // plcfBkmkf/plcfBkmkl
    m_pFactoids->Write(*this);

    WriteNumbering();

    RestoreMacroCmds();

    m_pMagicTable->Write( *this );

    m_pPiece->WritePc( *this );               // Piece-Table
    m_aFontHelper.WriteFontTable(pTableStrm, *pFib); // FFNs

    //Convert OOo asian typography into MS typography structure
    ExportDopTypography(pDop->doptypography);

    WriteDop( *this );                      // Document-Properties

    // Write SttbfAssoc
    WW8SttbAssoc * pSttbfAssoc = dynamic_cast<WW8SttbAssoc *>
        (m_pDoc->getIDocumentExternalData().getExternalData(::sw::tExternalDataType::STTBF_ASSOC).get());

    if ( pSttbfAssoc )                      // #i106057#
    {
        std::vector<OUString> aStrings(pSttbfAssoc->getStrings());
        WriteAsStringTable(aStrings, pFib->m_fcSttbfAssoc,
                           pFib->m_lcbSttbfAssoc);
    }

    Strm().Seek( 0 );

    // Reclaim stored FIB data from document.
    ::ww8::WW8FibData * pFibData = dynamic_cast<ww8::WW8FibData *>
          (m_pDoc->getIDocumentExternalData().getExternalData(::sw::tExternalDataType::FIB).get());

    if ( pFibData )
    {
    pFib->m_fReadOnlyRecommended =
        pFibData->getReadOnlyRecommended();
    pFib->m_fWriteReservation =
        pFibData->getWriteReservation();
    }

    pFib->Write( Strm() );  // FIB
}

void WW8Export::StoreDoc1()
{
    bool bNeedsFinalPara = false;
    // Start of Text ( overwrite )
    SwWW8Writer::FillUntil( Strm(), pFib->m_fcMin );

    WriteMainText();                    // main text
    sal_uInt8 nSprmsLen;
    sal_uInt8 *pLastSprms = m_pPapPlc->CopyLastSprms(nSprmsLen);

    bNeedsFinalPara |= pFootnote->WriteText( *this );         // Footnote-Text
    bNeedsFinalPara |= pSepx->WriteKFText( *this );          // K/F-Text
    bNeedsFinalPara |= m_pAtn->WriteText( *this );         // Annotation-Text
    bNeedsFinalPara |= pEdn->WriteText( *this );         // EndNote-Text

    // create the escher streams
    CreateEscher();

    bNeedsFinalPara |= m_pTextBxs->WriteText( *this );  //Textbox Text Plc
    bNeedsFinalPara |= m_pHFTextBxs->WriteText( *this );//Head/Foot-Textbox Text Plc

    if (bNeedsFinalPara)
    {
        WriteCR();
        m_pPapPlc->AppendFkpEntry(Strm().Tell(), nSprmsLen, pLastSprms);
    }
    delete[] pLastSprms;

    pSepx->Finish( Fc2Cp( Strm().Tell() ));// Text + Footnote + HdFt as section end
    m_pMagicTable->Finish( Fc2Cp( Strm().Tell() ),0);

    pFib->m_fcMac = Strm().Tell();        // End of all texts

    WriteFkpPlcUsw();                   // FKP, PLC, .....
}

void MSWordExportBase::AddLinkTarget(const OUString& rURL)
{
    if( rURL.isEmpty() || rURL[0] != '#' )
        return;

    OUString aURL( BookmarkToWriter( rURL.copy( 1 ) ) );
    sal_Int32 nPos = aURL.lastIndexOf( cMarkSeparator );

    if( nPos < 2 )
        return;

    OUString sCmp = aURL.copy(nPos+1).replaceAll(" ", "");
    if( sCmp.isEmpty() )
        return;

    sCmp = sCmp.toAsciiLowerCase();

    if( sCmp == "outline" )
    {
        SwPosition aPos( *m_pCurPam->GetPoint() );
        OUString aOutline( BookmarkToWriter(aURL.copy( 0, nPos )) );
        // If we can find the outline this bookmark refers to
        // save the name of the bookmark and the
        // node index number of where it points to
        if( m_pDoc->GotoOutline( aPos, aOutline ) )
        {
            sal_uLong nIdx = aPos.nNode.GetIndex();
            aBookmarkPair aImplicitBookmark;
            aImplicitBookmark.first = aOutline;
            aImplicitBookmark.second = nIdx;
            m_aImplicitBookmarks.push_back(aImplicitBookmark);
        }
    }
}

void MSWordExportBase::CollectOutlineBookmarks(const SwDoc &rDoc)
{
    sal_uInt32 nMaxItems = rDoc.GetAttrPool().GetItemCount2(RES_TXTATR_INETFMT);
    for (sal_uInt32 n = 0; n < nMaxItems; ++n)
    {
        const SwFormatINetFormat* pINetFormat = rDoc.GetAttrPool().GetItem2(RES_TXTATR_INETFMT, n);
        if (!pINetFormat)
            continue;

        const SwTextINetFormat* pTextAttr = pINetFormat->GetTextINetFormat();
        if (!pTextAttr)
            continue;

        const SwTextNode* pTextNd = pTextAttr->GetpTextNode();
        if (!pTextNd)
            continue;

        if (!pTextNd->GetNodes().IsDocNodes())
            continue;

        AddLinkTarget( pINetFormat->GetValue() );
    }

    nMaxItems = rDoc.GetAttrPool().GetItemCount2( RES_URL );
    for (sal_uInt32 n = 0; n < nMaxItems; ++n)
    {
        const SwFormatURL *pURL = rDoc.GetAttrPool().GetItem2(RES_URL, n);
        if (!pURL)
            continue;

        AddLinkTarget(pURL->GetURL());
        const ImageMap *pIMap = pURL->GetMap();
        if (!pIMap)
            continue;

        for (size_t i=0; i < pIMap->GetIMapObjectCount(); ++i)
        {
            const IMapObject* pObj = pIMap->GetIMapObject(i);
            if (!pObj)
                continue;
            AddLinkTarget( pObj->GetURL() );
        }
    }
}

namespace
{
    const sal_uLong WW_BLOCKSIZE = 0x200;

    ErrCode EncryptRC4(msfilter::MSCodec_Std97& rCtx, SvStream &rIn, SvStream &rOut)
    {
        sal_uLong nLen = rIn.TellEnd();
        rIn.Seek(0);

        sal_uInt8 in[WW_BLOCKSIZE];
        for (std::size_t nI = 0, nBlock = 0; nI < nLen; nI += WW_BLOCKSIZE, ++nBlock)
        {
            std::size_t nBS = std::min(nLen - nI, WW_BLOCKSIZE);
            nBS = rIn.ReadBytes(in, nBS);
            if (!rCtx.InitCipher(nBlock)) {
                return ERRCODE_IO_NOTSUPPORTED;
            }
            rCtx.Encode(in, nBS, in, nBS);
            rOut.WriteBytes(in, nBS);
        }
        return ERRCODE_NONE;
    }
}

ErrCode MSWordExportBase::ExportDocument( bool bWriteAll )
{
    m_nCharFormatStart = DEFAULT_STYLES_COUNT;
    m_nFormatCollStart = m_nCharFormatStart + m_pDoc->GetCharFormats()->size() - 1;

    m_bStyDef = m_bBreakBefore = m_bOutKF =
        m_bOutFlyFrameAttrs = m_bOutPageDescs = m_bOutTable = m_bOutFirstPage =
        m_bOutGrf = m_bInWriteEscher = m_bStartTOX =
        m_bInWriteTOX = false;

    m_bFootnoteAtTextEnd = m_bEndAtTextEnd = true;

    m_pParentFrame = nullptr;
    m_pFlyOffset = nullptr;
    m_eNewAnchorType = RndStdIds::FLY_AT_PAGE;
    m_nTextTyp = TXT_MAINTEXT;
    m_nStyleBeforeFly = m_nLastFormatId = 0;
    m_pStyAttr = nullptr;
    m_pCurrentStyle = nullptr;
    m_pOutFormatNode = nullptr;
    m_pEscher = nullptr;
    m_pRedlAuthors = nullptr;
    m_aTOXArr.clear();

    if ( !m_pOLEExp )
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

        m_pOLEExp.reset(new SvxMSExportOLEObjects( nSvxMSDffOLEConvFlags ));
    }

    if ( !m_pOCXExp && m_pDoc->GetDocShell() )
        m_pOCXExp.reset(new SwMSConvertControls( m_pDoc->GetDocShell(), m_pCurPam ));

    // #i81405# - Collect anchored objects before changing the redline mode.
    m_aFrames = GetFrames( *m_pDoc, bWriteAll? nullptr : m_pOrigPam );

    m_nOrigRedlineFlags = m_pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
    if ( !m_pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty() )
    {
        //restored to original state by SwWriter::Write
        m_pDoc->getIDocumentRedlineAccess().SetRedlineFlags(m_nOrigRedlineFlags |
                                                         RedlineFlags::ShowDelete |
                                                         RedlineFlags::ShowInsert);
    }

    // fix the SwPositions in m_aFrames after SetRedlineFlags
    UpdateFramePositions(m_aFrames);

    m_aFontHelper.InitFontTable(*m_pDoc);
    GatherChapterFields();

    CollectOutlineBookmarks(*m_pDoc);

    // make unique OrdNums (Z-Order) for all drawing-/fly Objects
    if ( m_pDoc->getIDocumentDrawModelAccess().GetDrawModel() )
        m_pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 )->RecalcObjOrdNums();

    ErrCode err = ExportDocument_Impl();

    m_aFrames.clear();

    // park m_pCurPam in a "safe place" now that document is fully exported
    // before toggling redline mode to avoid ~SwIndexReg assert e.g. export
    // ooo103014-1.odt to .doc
    // park m_pOrigPam as well, as needed for exporting abi9915-1.odt to doc
    m_pOrigPam->DeleteMark();
    *m_pOrigPam->GetPoint() = SwPosition(m_pDoc->GetNodes().GetEndOfContent());
    *m_pCurPam = *m_pOrigPam;

    m_pDoc->getIDocumentRedlineAccess().SetRedlineFlags(m_nOrigRedlineFlags);

    return err;
}

bool SwWW8Writer::InitStd97CodecUpdateMedium( ::msfilter::MSCodec_Std97& rCodec )
{
    uno::Sequence< beans::NamedValue > aEncryptionData;

    if ( mpMedium )
    {
        const SfxUnoAnyItem* pEncryptionDataItem = SfxItemSet::GetItem<SfxUnoAnyItem>(mpMedium->GetItemSet(), SID_ENCRYPTIONDATA, false);
        if ( pEncryptionDataItem && ( pEncryptionDataItem->GetValue() >>= aEncryptionData ) && !rCodec.InitCodec( aEncryptionData ) )
        {
            OSL_ENSURE( false, "Unexpected EncryptionData!" );
            aEncryptionData.realloc( 0 );
        }

        if ( !aEncryptionData.getLength() )
        {
            // try to generate the encryption data based on password
            const SfxStringItem* pPasswordItem = SfxItemSet::GetItem<SfxStringItem>(mpMedium->GetItemSet(), SID_PASSWORD, false);
            if ( pPasswordItem && !pPasswordItem->GetValue().isEmpty() && pPasswordItem->GetValue().getLength() <= 15 )
            {
                // Generate random number with a seed of time as salt.
                rtlRandomPool aRandomPool = rtl_random_createPool ();
                sal_uInt8 pDocId[ 16 ];
                rtl_random_getBytes( aRandomPool, pDocId, 16 );

                rtl_random_destroyPool( aRandomPool );

                sal_uInt16 aPassword[16];
                memset( aPassword, 0, sizeof( aPassword ) );

                const OUString& sPassword(pPasswordItem->GetValue());
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

    // nonempty encryption data means here that the codec was successfully initialized
    return ( aEncryptionData.getLength() != 0 );
}

ErrCode WW8Export::ExportDocument_Impl()
{
    PrepareStorage();

    pFib.reset(new WW8Fib(8, m_bDot));

    tools::SvRef<SotStorageStream> xWwStrm( GetWriter().GetStorage().OpenSotStream( m_aMainStg ) );
    tools::SvRef<SotStorageStream> xTableStrm( xWwStrm ), xDataStrm( xWwStrm );
    xWwStrm->SetBufferSize( 32768 );

    pFib->m_fWhichTableStm = true;
    xTableStrm = GetWriter().GetStorage().OpenSotStream(SL::a1Table, StreamMode::STD_WRITE);
    xDataStrm = GetWriter().GetStorage().OpenSotStream(SL::aData, StreamMode::STD_WRITE);

    xDataStrm->SetBufferSize( 32768 );  // for graphics
    xTableStrm->SetBufferSize( 16384 ); // for the Font-/Style-Table, etc.

    xTableStrm->SetEndian( SvStreamEndian::LITTLE );
    xDataStrm->SetEndian( SvStreamEndian::LITTLE );

    GetWriter().SetStream( xWwStrm.get() );
    pTableStrm = xTableStrm.get();
    pDataStrm = xDataStrm.get();

    Strm().SetEndian( SvStreamEndian::LITTLE );

    utl::TempFile aTempMain;
    aTempMain.EnableKillingFile();
    utl::TempFile aTempTable;
    aTempTable.EnableKillingFile();
    utl::TempFile aTempData;
    aTempData.EnableKillingFile();

    msfilter::MSCodec_Std97 aCtx;
    bool bEncrypt = GetWriter().InitStd97CodecUpdateMedium(aCtx);
    if ( bEncrypt )
    {
        GetWriter().SetStream(
            aTempMain.GetStream( StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE ) );

        pTableStrm = aTempTable.GetStream( StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE );

        pDataStrm = aTempData.GetStream( StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE );

        sal_uInt8 const aRC4EncryptionHeader[ 52 ] = {0};
        pTableStrm->WriteBytes(aRC4EncryptionHeader, 52);
    }

    // Default: "Standard"
    pSepx.reset(new WW8_WrPlcSepx( *this ));                         // Sections/headers/footers

    pFootnote.reset(new WW8_WrPlcFootnoteEdn( TXT_FTN ));                      // Footnotes
    pEdn.reset(new WW8_WrPlcFootnoteEdn( TXT_EDN ));                      // Endnotes
    m_pAtn = new WW8_WrPlcAnnotations;                                 // PostIts
    m_pFactoids.reset(new WW8_WrtFactoids); // Smart tags.
    m_pTextBxs = new WW8_WrPlcTextBoxes( TXT_TXTBOX );
    m_pHFTextBxs = new WW8_WrPlcTextBoxes( TXT_HFTXTBOX );

    m_pSdrObjs = new MainTextPlcDrawObj;   // Draw-/Fly-Objects for main text
    m_pHFSdrObjs = new HdFtPlcDrawObj;    // Draw-/Fly-Objects for header/footer

    m_pBkmks = new WW8_WrtBookmarks;                          // Bookmarks
    GetWriter().CreateBookmarkTable();

    m_pPapPlc.reset(new WW8_WrPlcPn( *this, PAP, pFib->m_fcMin ));
    m_pChpPlc.reset(new WW8_WrPlcPn( *this, CHP, pFib->m_fcMin ));
    pO.reset(new ww::bytes);
    m_pStyles.reset(new MSWordStyles( *this ));
    m_pFieldMain.reset(new WW8_WrPlcField( 2, TXT_MAINTEXT ));
    m_pFieldHdFt.reset(new WW8_WrPlcField( 2, TXT_HDFT ));
    m_pFieldFootnote.reset(new WW8_WrPlcField( 2, TXT_FTN ));
    m_pFieldEdn.reset(new WW8_WrPlcField( 2, TXT_EDN ));
    m_pFieldAtn.reset(new WW8_WrPlcField( 2, TXT_ATN ));
    m_pFieldTextBxs.reset(new WW8_WrPlcField( 2, TXT_TXTBOX ));
    m_pFieldHFTextBxs.reset(new WW8_WrPlcField( 2, TXT_HFTXTBOX ));

    m_pMagicTable.reset(new WW8_WrMagicTable);

    m_pGrf.reset(new SwWW8WrGrf( *this ));
    m_pPiece = new WW8_WrPct( pFib->m_fcMin );
    pDop.reset(new WW8Dop);

    pDop->fRevMarking = bool( RedlineFlags::On & m_nOrigRedlineFlags );
    SwRootFrame const*const pLayout(m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    pDop->fRMView = pLayout == nullptr || !pLayout->IsHideRedlines();
    pDop->fRMPrint = pDop->fRMView;

    // set AutoHyphenation flag if found in default para style
    const SfxPoolItem* pItem;
    SwTextFormatColl* pStdTextFormatColl =
        m_pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD, false);
    if (pStdTextFormatColl && SfxItemState::SET == pStdTextFormatColl->GetItemState(
        RES_PARATR_HYPHENZONE, false, &pItem))
    {
        pDop->fAutoHyphen = static_cast<const SvxHyphenZoneItem*>(pItem)->IsHyphen();
    }

    StoreDoc1();

    ErrCode err = ERRCODE_NONE;
    if ( bEncrypt )
    {
        SvStream *pStrmTemp, *pTableStrmTemp, *pDataStrmTemp;
        pStrmTemp = xWwStrm.get();
        pTableStrmTemp = xTableStrm.get();
        pDataStrmTemp = xDataStrm.get();

        if ( pDataStrmTemp && pDataStrmTemp != pStrmTemp) {
            err = EncryptRC4(aCtx, *pDataStrm, *pDataStrmTemp);
            if (err != ERRCODE_NONE) {
                goto done;
            }
        }

        err = EncryptRC4(aCtx, *pTableStrm, *pTableStrmTemp);
        if (err != ERRCODE_NONE) {
            goto done;
        }

        // Write Unencrypted Header 52 bytes to the start of the table stream
        // EncryptionVersionInfo (4 bytes): A Version structure where Version.vMajor MUST be 0x0001, and Version.vMinor MUST be 0x0001.
        pTableStrmTemp->Seek( 0 );
        pTableStrmTemp->WriteUInt32( 0x10001 ); // nEncType

        sal_uInt8 pDocId[16];
        aCtx.GetDocId( pDocId );

        sal_uInt8 pSaltData[16];
        sal_uInt8 pSaltDigest[16];
        aCtx.GetEncryptKey( pDocId, pSaltData, pSaltDigest );

        pTableStrmTemp->WriteBytes(pDocId, 16);
        pTableStrmTemp->WriteBytes(pSaltData, 16);
        pTableStrmTemp->WriteBytes(pSaltDigest, 16);

        err = EncryptRC4(aCtx, GetWriter().Strm(), *pStrmTemp);
        if (err != ERRCODE_NONE) {
            goto done;
        }

        // Write Unencrypted Fib 68 bytes to the start of the workdocument stream
        pFib->m_fEncrypted = true; // fEncrypted indicates the document is encrypted.
        pFib->m_fObfuscated = false; // Must be 0 for RC4.
        pFib->m_nHash = 0x34; // encrypt header bytes count of table stream.
        pFib->m_nKey = 0; // lkey2 must be 0 for RC4.

        pStrmTemp->Seek( 0 );
        pFib->WriteHeader( *pStrmTemp );
    done:;
    }

    m_pGrf.reset();
    m_pMagicTable.reset();;
    m_pFieldFootnote.reset();;
    m_pFieldTextBxs.reset();;
    m_pFieldHFTextBxs.reset();;
    m_pFieldAtn.reset();;
    m_pFieldEdn.reset();;
    m_pFieldHdFt.reset();;
    m_pFieldMain.reset();;
    m_pStyles.reset();;
    pO.reset();
    m_pChpPlc.reset();;
    m_pPapPlc.reset();;
    pSepx.reset();

    delete m_pRedlAuthors;
    delete m_pSdrObjs;
    delete m_pHFSdrObjs;
    delete m_pTextBxs;
    delete m_pHFTextBxs;
    delete m_pAtn;
    pEdn.reset();
    pFootnote.reset();
    delete m_pBkmks;
    delete m_pPiece;
    pDop.reset();
    pFib.reset();
    GetWriter().SetStream( nullptr );

    xWwStrm->SetBufferSize( 0 );
    xTableStrm->SetBufferSize( 0 );
    xDataStrm->SetBufferSize( 0 );
    if( 0 == pDataStrm->Seek( STREAM_SEEK_TO_END ))
    {
        xDataStrm.clear();
        pDataStrm = nullptr;
        GetWriter().GetStorage().Remove(SL::aData);
    }

    return err;
}

void WW8Export::PrepareStorage()
{
    static const sal_uInt8 pData[] =
    {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0x09, 0x02, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x46,

        0x18, 0x00, 0x00, 0x00,
        'M', 'i', 'c', 'r', 'o', 's', 'o', 'f',
        't', ' ', 'W', 'o', 'r', 'd', '-', 'D',
        'o', 'k', 'u', 'm', 'e', 'n', 't', 0x0,

        0x0A, 0x00, 0x00, 0x00,
        'M', 'S', 'W', 'o', 'r', 'd', 'D', 'o',
        'c', 0x0,

        0x10, 0x00, 0x00, 0x00,
        'W', 'o', 'r', 'd', '.', 'D', 'o', 'c',
        'u', 'm', 'e', 'n', 't', '.', '8', 0x0,

        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    SvGlobalName aGName(MSO_WW8_CLASSID);
    GetWriter().GetStorage().SetClass(
        aGName, SotClipboardFormatId::NONE, "Microsoft Word-Document");
    tools::SvRef<SotStorageStream> xStor( GetWriter().GetStorage().OpenSotStream(sCompObj) );
    xStor->WriteBytes(pData, sizeof(pData));

    SwDocShell* pDocShell = m_pDoc->GetDocShell ();
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
                std::shared_ptr<GDIMetaFile> xMetaFile =
                    pDocShell->GetPreviewMetaFile();
                uno::Sequence<sal_Int8> metaFile(
                    sfx2::convertMetaFile(xMetaFile.get()));
                sfx2::SaveOlePropertySet(xDocProps, &GetWriter().GetStorage(), &metaFile);
            }
            else
                sfx2::SaveOlePropertySet( xDocProps, &GetWriter().GetStorage() );
        }
    }
}

ErrCode SwWW8Writer::WriteStorage()
{
    // #i34818# - update layout (if present), for SwWriteTable
    SwViewShell* pViewShell = m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    if( pViewShell != nullptr )
        pViewShell->CalcLayout();

    long nMaxNode = m_pDoc->GetNodes().Count();
    ::StartProgress( STR_STATSTR_W4WWRITE, 0, nMaxNode, m_pDoc->GetDocShell() );

    // Respect table at the beginning of the document
    {
        SwTableNode* pTNd = m_pCurrentPam->GetNode().FindTableNode();
        if( pTNd && m_bWriteAll )
            // start with the table node !!
            m_pCurrentPam->GetPoint()->nNode = *pTNd;
    }

    // Do the actual export
    ErrCode err = ERRCODE_NONE;
    {
        bool bDot = mpMedium->GetFilter()->GetName().endsWith("Vorlage");
        WW8Export aExport(this, m_pDoc, m_pCurrentPam, m_pOrigPam, bDot);
        m_pExport = &aExport;
        err = aExport.ExportDocument( m_bWriteAll );
        m_pExport = nullptr;
    }

    ::EndProgress( m_pDoc->GetDocShell() );
    return err;
}

ErrCode SwWW8Writer::WriteMedium( SfxMedium& )
{
    return WriteStorage();
}

ErrCode SwWW8Writer::Write( SwPaM& rPaM, SfxMedium& rMed,
                          const OUString* pFileName )
{
    mpMedium = &rMed;
    ErrCode nRet = StgWriter::Write( rPaM, rMed, pFileName );
    mpMedium = nullptr;
    return nRet;
}

MSWordExportBase::MSWordExportBase( SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam )
    : m_aMainStg(sMainStream)
    , m_pISet(nullptr)
    , m_pPiece(nullptr)
    , m_pTopNodeOfHdFtPage(nullptr)
    , m_pBkmks(nullptr)
    , m_pRedlAuthors(nullptr)
    , m_pTableInfo(new ww8::WW8TableInfo())
    , m_nCharFormatStart(0)
    , m_nFormatCollStart(0)
    , m_nStyleBeforeFly(0)
    , m_nLastFormatId(0)
    , m_nUniqueList(0)
    , m_nHdFtIndex(0)
    , m_nOrigRedlineFlags(RedlineFlags::NONE)
    , m_pCurrentPageDesc(nullptr)
    , m_bPrevTextNodeIsEmpty(false)
    , m_bFirstTOCNodeWithSection(false)
    , m_pChpIter(nullptr)
    , m_pAtn(nullptr)
    , m_pTextBxs(nullptr)
    , m_pHFTextBxs(nullptr)
    , m_pParentFrame(nullptr)
    , m_pFlyOffset(nullptr)
    , m_eNewAnchorType(RndStdIds::FLY_AS_CHAR)
    , m_pStyAttr(nullptr)
    , m_pOutFormatNode(nullptr)
    , m_pCurrentStyle(nullptr)
    , m_pSdrObjs(nullptr)
    , m_pHFSdrObjs(nullptr)
    , m_pEscher(nullptr)
    , m_nTextTyp(0)
    , m_bStyDef(false)
    , m_bBreakBefore(false)
    , m_bOutKF(false)
    , m_bOutFlyFrameAttrs(false)
    , m_bOutPageDescs(false)
    , m_bOutFirstPage(false)
    , m_bOutTable(false)
    , m_bOutGrf(false)
    , m_bInWriteEscher(false)
    , m_bStartTOX(false)
    , m_bInWriteTOX(false)
    , m_bFootnoteAtTextEnd(false)
    , m_bEndAtTextEnd(false)
    , m_bHasHdr(false)
    , m_bHasFtr(false)
    , m_bSubstituteBullets(true)
    , m_bTabInTOC(false)
    , m_bHideTabLeaderAndPageNumbers(false)
    , m_bExportModeRTF(false)
    , m_bFontSizeWritten(false)
    , m_bAddFootnoteTab(false)
    , m_pDoc(pDocument)
    , m_nCurStart(pCurrentPam->GetPoint()->nNode.GetIndex())
    , m_nCurEnd(pCurrentPam->GetMark()->nNode.GetIndex())
    , m_pCurPam(pCurrentPam)
    , m_pOrigPam(pOriginalPam)
{
}

MSWordExportBase::~MSWordExportBase()
{
    if (m_pUsedNumTable)           // all used NumRules
    {
        // clear the part of the list array that was copied from the document
        // - it's an auto delete array, so the rest of the array which are
        // duplicated lists that were added during the export will be deleted.
        m_pUsedNumTable->erase(m_pUsedNumTable->begin(), m_pUsedNumTable->begin() + m_pUsedNumTable->size() - m_nUniqueList);
        m_pUsedNumTable.reset();
    }
    m_pOLEExp.reset();
    m_pOCXExp.reset();
}

WW8Export::WW8Export( SwWW8Writer *pWriter,
        SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam,
        bool bDot )
    : MSWordExportBase( pDocument, pCurrentPam, pOriginalPam )
    , pTableStrm(nullptr)
    , pDataStrm(nullptr)
    , m_bDot(bDot)
    , m_pWriter(pWriter)
    , m_pAttrOutput(new WW8AttributeOutput(*this))
{
}

WW8Export::~WW8Export()
{
}

AttributeOutputBase& WW8Export::AttrOutput() const
{
    return *m_pAttrOutput;
}

MSWordSections& WW8Export::Sections() const
{
    return *pSepx;
}

SwWW8Writer::SwWW8Writer(const OUString& rFltName, const OUString& rBaseURL)
    : StgWriter(),
      m_pExport( nullptr ),
      mpMedium( nullptr )
{
    assert(rFltName == FILTER_WW8); // WW6/7 export was removed
    (void)rFltName;
    SetBaseURL( rBaseURL );
}

SwWW8Writer::~SwWW8Writer()
{
}

extern "C" SAL_DLLPUBLIC_EXPORT sal_uInt32 SaveOrDelMSVBAStorage_ww8( SfxObjectShell& rDoc, SotStorage& rStor, sal_Bool bSaveInto, const OUString& rStorageName )
{
    SvxImportMSVBasic aTmp( rDoc, rStor );
    return sal_uInt32(aTmp.SaveOrDelMSVBAStorage( bSaveInto, rStorageName ));
}

extern "C" SAL_DLLPUBLIC_EXPORT void ExportDOC( const OUString& rFltName, const OUString& rBaseURL, WriterRef& xRet )
{
    xRet = new SwWW8Writer( rFltName, rBaseURL );
}

extern "C" SAL_DLLPUBLIC_EXPORT sal_uInt32 GetSaveWarningOfMSVBAStorage_ww8(  SfxObjectShell &rDocS )
{
    return sal_uInt32(SvxImportMSVBasic::GetSaveWarningOfMSVBAStorage( rDocS ));
}

bool WW8_WrPlcFootnoteEdn::WriteText( WW8Export& rWrt )
{
    bool bRet = false;
    if (TXT_FTN == nTyp)
    {
        bRet = WriteGenericText( rWrt, TXT_FTN, rWrt.pFib->m_ccpFootnote );
        rWrt.m_pFieldFootnote->Finish( rWrt.Fc2Cp( rWrt.Strm().Tell() ),
                            rWrt.pFib->m_ccpText );
    }
    else
    {
        bRet = WriteGenericText( rWrt, TXT_EDN, rWrt.pFib->m_ccpEdn );
        rWrt.m_pFieldEdn->Finish( rWrt.Fc2Cp( rWrt.Strm().Tell() ),
                            rWrt.pFib->m_ccpText + rWrt.pFib->m_ccpFootnote
                            + rWrt.pFib->m_ccpHdr + rWrt.pFib->m_ccpAtn );
    }
    return bRet;
}

void WW8_WrPlcFootnoteEdn::WritePlc( WW8Export& rWrt ) const
{
    if( TXT_FTN == nTyp )
    {
        WriteGenericPlc( rWrt, TXT_FTN, rWrt.pFib->m_fcPlcffndText,
            rWrt.pFib->m_lcbPlcffndText, rWrt.pFib->m_fcPlcffndRef,
            rWrt.pFib->m_lcbPlcffndRef );
    }
    else
    {
        WriteGenericPlc( rWrt, TXT_EDN, rWrt.pFib->m_fcPlcfendText,
            rWrt.pFib->m_lcbPlcfendText, rWrt.pFib->m_fcPlcfendRef,
            rWrt.pFib->m_lcbPlcfendRef );
    }
}

bool WW8_WrPlcAnnotations::WriteText( WW8Export& rWrt )
{
    bool bRet = WriteGenericText( rWrt, TXT_ATN, rWrt.pFib->m_ccpAtn );
    rWrt.m_pFieldAtn->Finish( rWrt.Fc2Cp( rWrt.Strm().Tell() ),
                        rWrt.pFib->m_ccpText + rWrt.pFib->m_ccpFootnote
                        + rWrt.pFib->m_ccpHdr );
    return bRet;
}

void WW8_WrPlcAnnotations::WritePlc( WW8Export& rWrt ) const
{
    WriteGenericPlc( rWrt, TXT_ATN, rWrt.pFib->m_fcPlcfandText,
        rWrt.pFib->m_lcbPlcfandText, rWrt.pFib->m_fcPlcfandRef,
        rWrt.pFib->m_lcbPlcfandRef );
}

void WW8_WrPlcTextBoxes::WritePlc( WW8Export& rWrt ) const
{
    if( TXT_TXTBOX == nTyp )
    {
        WriteGenericPlc( rWrt, nTyp, rWrt.pFib->m_fcPlcftxbxBkd,
            rWrt.pFib->m_lcbPlcftxbxBkd, rWrt.pFib->m_fcPlcftxbxText,
            rWrt.pFib->m_lcbPlcftxbxText );
    }
    else
    {
        WriteGenericPlc( rWrt, nTyp, rWrt.pFib->m_fcPlcfHdrtxbxBkd,
            rWrt.pFib->m_lcbPlcfHdrtxbxBkd, rWrt.pFib->m_fcPlcfHdrtxbxText,
            rWrt.pFib->m_lcbPlcfHdrtxbxText );
    }
}

void WW8Export::RestoreMacroCmds()
{
    pFib->m_fcCmds = pTableStrm->Tell();

    uno::Reference < embed::XStorage > xSrcRoot(m_pDoc->GetDocShell()->GetStorage());
    try
    {
        uno::Reference < io::XStream > xSrcStream =
                xSrcRoot->openStreamElement( SL::aMSMacroCmds, embed::ElementModes::READ );
        std::unique_ptr<SvStream> pStream = ::utl::UcbStreamHelper::CreateStream( xSrcStream );

        if ( pStream && ERRCODE_NONE == pStream->GetError())
        {
            pFib->m_lcbCmds = pStream->TellEnd();
            pStream->Seek(0);

            std::unique_ptr<sal_uInt8[]> pBuffer( new sal_uInt8[pFib->m_lcbCmds] );
            bool bReadOk = checkRead(*pStream, pBuffer.get(), pFib->m_lcbCmds);
            if (bReadOk)
                pTableStrm->WriteBytes(pBuffer.get(), pFib->m_lcbCmds);
        }
    }
    catch ( const uno::Exception& )
    {
    }

    // set len to FIB
    pFib->m_lcbCmds = pTableStrm->Tell() - pFib->m_fcCmds;
}

void WW8SHDLong::Write( WW8Export& rExport )
{
    rExport.InsUInt32( m_cvFore );
    rExport.InsUInt32( m_cvBack );
    rExport.InsUInt16( 0 ); // ipat
}

void WW8Export::WriteFormData( const ::sw::mark::IFieldmark& rFieldmark )
{
    const ::sw::mark::IFieldmark* pFieldmark = &rFieldmark;
    const ::sw::mark::ICheckboxFieldmark* pAsCheckbox = dynamic_cast< const ::sw::mark::ICheckboxFieldmark* >( pFieldmark );

    if ( ! ( rFieldmark.GetFieldname() == ODF_FORMTEXT ||
                rFieldmark.GetFieldname() == ODF_FORMDROPDOWN ||
                rFieldmark.GetFieldname() == ODF_FORMCHECKBOX ) )
    {
        SAL_WARN( "sw.ww8", "unknown field type" );
        return;
    }

    int type = 0; // TextFieldmark
    if ( pAsCheckbox )
        type = 1;
    if ( rFieldmark.GetFieldname() == ODF_FORMDROPDOWN )
        type=2;

    ::sw::mark::IFieldmark::parameter_map_t::const_iterator pParameter = rFieldmark.GetParameters()->find("name");
    OUString ffname;
    if ( pParameter != rFieldmark.GetParameters()->end() )
    {
        OUString aName;
        pParameter->second >>= aName;
        assert( aName.getLength() < 21 && "jluth seeing if following documentation will cause problems." );
        const sal_Int32 nLen = std::min( sal_Int32(20), aName.getLength() );
        ffname = aName.copy(0, nLen);
    }

    sal_uLong nDataStt = pDataStrm->Tell();
    m_pChpPlc->AppendFkpEntry(Strm().Tell());

    WriteChar(0x01);
    static sal_uInt8 aArr1[] =
    {
        0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation

        0x06, 0x08, 0x01,       // sprmCFData
        0x55, 0x08, 0x01,       // sprmCFSpec
        0x02, 0x08, 0x01        // sprmCFFieldVanish
    };
    sal_uInt8* pDataAdr = aArr1 + 2;
    Set_UInt32(pDataAdr, nDataStt);

    m_pChpPlc->AppendFkpEntry( Strm().Tell(), sizeof( aArr1 ), aArr1 );

    struct FFDataHeader
    {
        sal_uInt32 version;
        sal_uInt16 bits;
        sal_uInt16 cch;
        sal_uInt16 hps;
        FFDataHeader() : version( 0xFFFFFFFF ), bits(0), cch(0), hps(0) {}
    };

    FFDataHeader aFieldHeader;
    aFieldHeader.bits |= (type & 0x03);

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
    aFieldHeader.bits |= ( (ffres<<2) & 0x7C );

    OUString ffdeftext;
    OUString ffformat;
    OUString ffhelptext = rFieldmark.GetFieldHelptext();
    if ( ffhelptext.getLength() > 255 )
        ffhelptext = ffhelptext.copy(0, 255);
    OUString ffstattext;
    OUString ffentrymcr;
    OUString ffexitmcr;
    if (type == 0) // iTypeText
    {
        sal_uInt16 nType = 0;
        pParameter = rFieldmark.GetParameters()->find("Type");
        if ( pParameter != rFieldmark.GetParameters()->end() )
        {
            OUString aType;
            pParameter->second >>= aType;
            if ( aType == "number" )            nType = 1;
            else if ( aType == "date" )         nType = 2;
            else if ( aType == "currentTime" )  nType = 3;
            else if ( aType == "currentDate" )  nType = 4;
            else if ( aType == "calculated" )   nType = 5;
            aFieldHeader.bits |= nType<<11; // FFDataBits-F  00111000 00000000
        }

        if ( nType < 3 || nType == 5 )  // not currentTime or currentDate
        {
            pParameter = rFieldmark.GetParameters()->find("Content");
            if ( pParameter != rFieldmark.GetParameters()->end() )
            {
                OUString aDefaultText;
                pParameter->second >>= aDefaultText;
                assert( aDefaultText.getLength() < 256 && "jluth seeing if following documentation will cause problems." );
                const sal_Int32 nLen = std::min( sal_Int32(255), aDefaultText.getLength() );
                ffdeftext = aDefaultText.copy (0, nLen);
            }
        }

        pParameter = rFieldmark.GetParameters()->find("MaxLength");
        if ( pParameter != rFieldmark.GetParameters()->end() )
        {
            sal_uInt16 nLength = 0;
            pParameter->second >>= nLength;
            assert( nLength < 32768 && "jluth seeing if following documentation will cause problems." );
            nLength = std::min( sal_uInt16(32767), nLength );
            aFieldHeader.cch = nLength;
        }

        pParameter = rFieldmark.GetParameters()->find("Format");
        if ( pParameter != rFieldmark.GetParameters()->end() )
        {
            OUString aFormat;
            pParameter->second >>= aFormat;
            const sal_Int32 nLen = std::min( sal_Int32(64), aFormat.getLength() );
            assert( nLen < 65 && "jluth seeing if following documentation will cause problems." );
            ffformat = aFormat.copy(0, nLen);
        }
    }

    pParameter = rFieldmark.GetParameters()->find("Help"); //help
    if ( ffhelptext.isEmpty() && pParameter != rFieldmark.GetParameters()->end() )
    {
        OUString aHelpText;
        pParameter->second >>= aHelpText;
        const sal_Int32 nLen = std::min( sal_Int32(255), aHelpText.getLength() );
        ffhelptext = aHelpText.copy (0, nLen);
    }
    if ( !ffhelptext.isEmpty() )
        aFieldHeader.bits |= 0x1<<7;

    pParameter = rFieldmark.GetParameters()->find("Description"); // doc tooltip
    if ( pParameter == rFieldmark.GetParameters()->end() )
        pParameter = rFieldmark.GetParameters()->find("Hint"); //docx tooltip
    if ( pParameter != rFieldmark.GetParameters()->end() )
    {
        OUString aStatusText;
        pParameter->second >>= aStatusText;
        const sal_Int32 nLen = std::min( sal_Int32(138), aStatusText.getLength() );
        ffstattext = aStatusText.copy (0, nLen);
    }
    if ( !ffstattext.isEmpty() )
        aFieldHeader.bits |= 0x1<<8;

    pParameter = rFieldmark.GetParameters()->find("EntryMacro");
    if ( pParameter != rFieldmark.GetParameters()->end() )
    {
        OUString aEntryMacro;
        pParameter->second >>= aEntryMacro;
        assert( aEntryMacro.getLength() < 33 && "jluth seeing if following documentation will cause problems." );
        const sal_Int32 nLen = std::min( sal_Int32(32), aEntryMacro.getLength() );
        ffentrymcr = aEntryMacro.copy (0, nLen);
    }

    pParameter = rFieldmark.GetParameters()->find("ExitMacro");
    if ( pParameter != rFieldmark.GetParameters()->end() )
    {
        OUString aExitMacro;
        pParameter->second >>= aExitMacro;
        assert( aExitMacro.getLength() < 33 && "jluth seeing if following documentation will cause problems." );
        const sal_Int32 nLen = std::min( sal_Int32(32), aExitMacro.getLength() );
        ffexitmcr = aExitMacro.copy (0, nLen);
    }

    std::vector< OUString > aListItems;
    if (type==2)
    {
        aFieldHeader.bits |= 0x8000; // ffhaslistbox
        const ::sw::mark::IFieldmark::parameter_map_t* const pParameters = rFieldmark.GetParameters();
        ::sw::mark::IFieldmark::parameter_map_t::const_iterator pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
        if(pListEntries != pParameters->end())
        {
            uno::Sequence< OUString > vListEntries;
            pListEntries->second >>= vListEntries;
            copy(vListEntries.begin(), vListEntries.end(), back_inserter(aListItems));
        }
    }

    const sal_uInt8 aFieldData[] =
    {
        0x44,0,         // the start of "next" data
        0,0,0,0,0,0,0,0,0,0,                // PIC-Structure!  /10
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |              /16
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |              /16
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |              /16
        0,0,0,0,                            // /               /4
    };
    sal_uInt32 slen = sizeof(sal_uInt32)
        + sizeof(aFieldData)
        + sizeof( aFieldHeader.version ) + sizeof( aFieldHeader.bits ) + sizeof( aFieldHeader.cch ) + sizeof( aFieldHeader.hps )
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

    pDataStrm->WriteUInt32( slen );

    int len = sizeof( aFieldData );
    OSL_ENSURE( len == 0x44-sizeof(sal_uInt32), "SwWW8Writer::WriteFormData(..) - wrong aFieldData length" );
    pDataStrm->WriteBytes( aFieldData, len );

    pDataStrm->WriteUInt32( aFieldHeader.version ).WriteUInt16( aFieldHeader.bits ).WriteUInt16( aFieldHeader.cch ).WriteUInt16( aFieldHeader.hps );

    SwWW8Writer::WriteString_xstz( *pDataStrm, ffname, true ); // Form field name

    if ( !type )
        SwWW8Writer::WriteString_xstz( *pDataStrm, ffdeftext, true );
    if ( type )
        pDataStrm->WriteUInt16( 0 );

    SwWW8Writer::WriteString_xstz( *pDataStrm, ffformat, true );
    SwWW8Writer::WriteString_xstz( *pDataStrm, ffhelptext, true );
    SwWW8Writer::WriteString_xstz( *pDataStrm, ffstattext, true );
    SwWW8Writer::WriteString_xstz( *pDataStrm, ffentrymcr, true );
    SwWW8Writer::WriteString_xstz( *pDataStrm, ffexitmcr, true );
    if (type==2) {
        pDataStrm->WriteUInt16( 0xFFFF );
        const int items=aListItems.size();
        pDataStrm->WriteUInt32( items );
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
    ShortToSVBT16( m_rWW8Export.m_nStyleBeforeFly, nStyle );

#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", "<OutWW8_TableNodeInfoInner>" << pNodeInfoInner->toString());
#endif

    m_rWW8Export.pO->clear();

    sal_uInt32 nShadowsBefore = pNodeInfoInner->getShadowsBefore();
    if (nShadowsBefore > 0)
    {
        ww8::WW8TableNodeInfoInner::Pointer_t
            pTmpNodeInfoInner(new ww8::WW8TableNodeInfoInner(nullptr));

        pTmpNodeInfoInner->setDepth(pNodeInfoInner->getDepth());
        pTmpNodeInfoInner->setEndOfCell(true);

        for (sal_uInt32 n = 0; n < nShadowsBefore; ++n)
        {
            m_rWW8Export.WriteCR(pTmpNodeInfoInner);

            m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), nStyle, nStyle+2 );     // Style #
            TableInfoCell(pTmpNodeInfoInner);
            m_rWW8Export.m_pPapPlc->AppendFkpEntry
                ( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );

            m_rWW8Export.pO->clear();
        }
    }

    if (pNodeInfoInner->isEndOfCell())
    {
        SAL_INFO( "sw.ww8", "<endOfCell/>" );

        m_rWW8Export.WriteCR(pNodeInfoInner);

        m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), nStyle, nStyle+2 );     // Style #
        TableInfoCell(pNodeInfoInner);
        m_rWW8Export.m_pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );

        m_rWW8Export.pO->clear();
    }

    sal_uInt32 nShadowsAfter = pNodeInfoInner->getShadowsAfter();
    if (nShadowsAfter > 0)
    {
        ww8::WW8TableNodeInfoInner::Pointer_t
            pTmpNodeInfoInner(new ww8::WW8TableNodeInfoInner(nullptr));

        pTmpNodeInfoInner->setDepth(pNodeInfoInner->getDepth());
        pTmpNodeInfoInner->setEndOfCell(true);

        for (sal_uInt32 n = 0; n < nShadowsAfter; ++n)
        {
            m_rWW8Export.WriteCR(pTmpNodeInfoInner);

            m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), nStyle, nStyle+2 );     // Style #
            TableInfoCell(pTmpNodeInfoInner);
            m_rWW8Export.m_pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );

            m_rWW8Export.pO->clear();
        }
    }

    if (pNodeInfoInner->isEndOfLine())
    {
        SAL_INFO( "sw.ww8", "<endOfLine/>" );

        TableRowEnd(pNodeInfoInner->getDepth());

        ShortToSVBT16(0, nStyle);
        m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), nStyle, nStyle+2 );     // Style #
        TableInfoRow(pNodeInfoInner);
        m_rWW8Export.m_pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );

        m_rWW8Export.pO->clear();
    }
    SAL_INFO( "sw.ww8", "</OutWW8_TableNodeInfoInner>" );
}

void MSWordExportBase::OutputStartNode( const SwStartNode & rNode)
{

    ww8::WW8TableNodeInfo::Pointer_t pNodeInfo =
        m_pTableInfo->getTableNodeInfo( &rNode );

    if (pNodeInfo.get() != nullptr)
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

    ww8::WW8TableNodeInfo::Pointer_t pNodeInfo = m_pTableInfo->getTableNodeInfo( &rNode );

    if (pNodeInfo.get() != nullptr)
    {
#ifdef DBG_UTIL
        SAL_INFO( "sw.ww8", pNodeInfo->toString());
#endif
        const ww8::WW8TableNodeInfo::Inners_t aInners = pNodeInfo->getInners();
        for (const auto& rEntry : aInners)
        {
            ww8::WW8TableNodeInfoInner::Pointer_t pInner = rEntry.second;
            AttrOutput().TableNodeInfoInner(pInner);
        }
    }
    SAL_INFO( "sw.ww8", "</OutWW8_SwEndNode>" );
}

const NfKeywordTable & MSWordExportBase::GetNfKeywordTable()
{
    if (m_pKeyMap == nullptr)
    {
        m_pKeyMap.reset(new NfKeywordTable);
        NfKeywordTable & rKeywordTable = *m_pKeyMap;
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

    return *m_pKeyMap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
