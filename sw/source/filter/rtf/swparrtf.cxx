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

#include <hintids.hxx>

#include <stack>

#include <tools/errinf.hxx>
#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <svl/itemiter.hxx>
#include <svtools/rtftoken.h>
#include <svl/intitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/hyznitem.hxx>
#include <fmtpdsc.hxx>
#include <fmtfld.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <txtftn.hxx>
#include <fmtclds.hxx>
#include <fmtftn.hxx>
#include <fmtfsize.hxx>
#include <fmtflcnt.hxx>
#include <fmtanchr.hxx>
#include <frmatr.hxx>
#include <docstat.hxx>
#include <swtable.hxx>
#include <shellio.hxx>
#include <swtypes.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <mdiexp.hxx>           // ...Percent()
#include <swparrtf.hxx>
#include <charfmt.hxx>
#include <pagedesc.hxx>
#include <ftninfo.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include <fltini.hxx>
#include <fchrfmt.hxx>
#include <paratr.hxx>
#include <section.hxx>
#include <fmtclbl.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>
#include <hfspacingitem.hxx>
#include <tox.hxx>
#include <swerror.h>
#include <cmdid.h>
#include <statstr.hrc>          // ResId fuer Statusleiste
#include <SwStyleNameMapper.hxx>
#include <tblsel.hxx>           // SwSelBoxes

#include <docsh.hxx>
#include <fmtlsplt.hxx> // SwLayoutSplit
#include <unotextrange.hxx>
#include <editeng/keepitem.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdorect.hxx>


#include <fmtsrnd.hxx>
#include <fmtfollowtextflow.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/opaqitem.hxx>
#include "svx/svdograf.hxx"
#include <svx/xflclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/svdoutl.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/paperinf.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/text/XTextRange.hpp>


using namespace ::com::sun::star;


// einige Hilfs-Funktionen
// char
inline const SvxFontHeightItem& GetSize(const SfxItemSet& rSet,sal_Bool bInP=sal_True)
    { return (const SvxFontHeightItem&)rSet.Get( RES_CHRATR_FONTSIZE,bInP); }
inline const SvxLRSpaceItem& GetLRSpace(const SfxItemSet& rSet,sal_Bool bInP=sal_True)
    { return (const SvxLRSpaceItem&)rSet.Get( RES_LR_SPACE,bInP); }


/// Glue class to call RtfImport as an internal filter, needed by copy&paste support.
class SwRTFReader : public Reader
{
    virtual sal_uLong Read( SwDoc &, const String& rBaseURL, SwPaM &,const String &);
};

sal_uLong SwRTFReader::Read( SwDoc &rDoc, const String& /*rBaseURL*/, SwPaM& rPam, const String &)
{
    if (!pStrm)
        return ERR_SWG_READ_ERROR;

    // We want to work in an empty paragraph.
    // Step 1: XTextRange will be updated when content is inserted, so we know
    // the end position.
    const uno::Reference<text::XTextRange> xInsertPosition =
        SwXTextRange::CreateXTextRange(rDoc, *rPam.GetPoint(), 0);
    SwNodeIndex *pSttNdIdx = new SwNodeIndex(rDoc.GetNodes());
    const SwPosition* pPos = rPam.GetPoint();

    // Step 2: Split once and remember the node that has been splitted.
    rDoc.SplitNode( *pPos, false );
    *pSttNdIdx = pPos->nNode.GetIndex()-1;

    // Step 3: Split again.
    rDoc.SplitNode( *pPos, false );

    // Step 4: Insert all content into the new node
    rPam.Move( fnMoveBackward );
    rDoc.SetTxtFmtColl
        ( rPam, rDoc.GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ) );

    SwDocShell *pDocShell(rDoc.GetDocShell());
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.RtfFilter"))), uno::UNO_QUERY_THROW);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XComponent> xDstDoc(pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    xImporter->setTargetDocument(xDstDoc);

    const uno::Reference<text::XTextRange> xInsertTextRange =
        SwXTextRange::CreateXTextRange(rDoc, *rPam.GetPoint(), 0);

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aDescriptor(3);
    aDescriptor[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream"));
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStrm));
    aDescriptor[0].Value <<= xStream;
    aDescriptor[1].Name = "IsNewDoc";
    aDescriptor[1].Value <<= sal_False;
    aDescriptor[2].Name = "TextInsertModeRange";
    aDescriptor[2].Value <<= xInsertTextRange;
    sal_uLong ret(0);
    try {
        xFilter->filter(aDescriptor);
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("sw.rtf", "SwRTFReader::Read(): exception: " << e.Message);
        ret = ERR_SWG_READ_ERROR;
    }

    // Clean up the fake paragraphs.
    SwUnoInternalPaM aPam(rDoc);
    ::sw::XTextRangeToSwPaM(aPam, xInsertPosition);
    if (pSttNdIdx->GetIndex())
    {
        // If we are in insert mode, join the splitted node that is in front
        // of the new content with the first new node. Or in other words:
        // Revert the first split node.
        SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
        SwNodeIndex aNxtIdx( *pSttNdIdx );
        if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ) &&
                pSttNdIdx->GetIndex() + 1 == aNxtIdx.GetIndex() )
        {
            // If the PaM points to the first new node, move the PaM to the
            // end of the previous node.
            if( aPam.GetPoint()->nNode == aNxtIdx )
            {
                aPam.GetPoint()->nNode = *pSttNdIdx;
                aPam.GetPoint()->nContent.Assign( pTxtNode,
                        pTxtNode->GetTxt().Len() );
            }
            // If the first new node isn't empty, convert  the node's text
            // attributes into hints. Otherwise, set the new node's
            // paragraph style at the previous (empty) node.
            SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
            if( pTxtNode->GetTxt().Len() )
                pDelNd->FmtToTxtAttr( pTxtNode );
            else
                pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
            pTxtNode->JoinNext();
        }
    }

    return ret;
}

extern "C" SAL_DLLPUBLIC_EXPORT Reader* SAL_CALL ImportRTF()
{
    return new SwRTFReader();
}

bool HasHeader(const SwFrmFmt &rFmt)
{
    const SfxPoolItem *pHd;
    if (SFX_ITEM_SET == rFmt.GetItemState(RES_HEADER, false, &pHd))
        return ((const SwFmtHeader *)(pHd))->IsActive();
    return false;
}

bool HasFooter(const SwFrmFmt &rFmt)
{
    const SfxPoolItem *pFt;
    if (SFX_ITEM_SET == rFmt.GetItemState(RES_FOOTER, false, &pFt))
        return ((const SwFmtFooter *)(pFt))->IsActive();
    return false;
}

namespace sw{
    namespace util{

InsertedTableClient::InsertedTableClient(SwTableNode & rNode)
{
    rNode.Add(this);
}

SwTableNode * InsertedTableClient::GetTableNode()
{
    return dynamic_cast<SwTableNode *> (GetRegisteredInNonConst());
}

InsertedTablesManager::InsertedTablesManager(const SwDoc &rDoc)
    : mbHasRoot(rDoc.GetCurrentLayout())    //swmod 080218
{
}

void InsertedTablesManager::DelAndMakeTblFrms()
{
    if (!mbHasRoot)
        return;
    TblMapIter aEnd = maTables.end();
    for (TblMapIter aIter = maTables.begin(); aIter != aEnd; ++aIter)
    {
        // exitiert schon ein Layout, dann muss an dieser Tabelle die BoxFrames
        // neu erzeugt
        SwTableNode *pTable = aIter->first->GetTableNode();
        OSL_ENSURE(pTable, "Why no expected table");
        if (pTable)
        {
            SwFrmFmt * pFrmFmt = pTable->GetTable().GetFrmFmt();

            if (pFrmFmt != NULL)
            {
                SwNodeIndex *pIndex = aIter->second;
                pTable->DelFrms();
                pTable->MakeFrms(pIndex);
            }
        }
    }
}

void InsertedTablesManager::InsertTable(SwTableNode &rTableNode, SwPaM &rPaM)
{
    if (!mbHasRoot)
        return;
    //Associate this tablenode with this after position, replace an //old
    //node association if necessary

    InsertedTableClient * pClient = new InsertedTableClient(rTableNode);

    maTables.insert(TblMap::value_type(pClient, &(rPaM.GetPoint()->nNode)));
}
}
}

::basegfx::B2DPoint rotate(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rEnd)
{
    const ::basegfx::B2DVector aVector(rStart - rEnd);
    return ::basegfx::B2DPoint(aVector.getY() + rEnd.getX(), -aVector.getX() + rEnd.getY());
}


DocPageInformation::DocPageInformation()
    : maBox( RES_BOX ),
    mnPaperw(12240), mnPaperh(15840), mnMargl(1800), mnMargr(1800),
    mnMargt(1440), mnMargb(1440), mnGutter(0), mnPgnStart(1), mbFacingp(false),
    mbLandscape(false), mbRTLdoc(false)
{
}

SectPageInformation::SectPageInformation(const DocPageInformation &rDoc)
    : maBox(rDoc.maBox), mpTitlePageHdFt(0), mpPageHdFt(0),
    mnPgwsxn(rDoc.mnPaperw), mnPghsxn(rDoc.mnPaperh), mnMarglsxn(rDoc.mnMargl),
    mnMargrsxn(rDoc.mnMargr), mnMargtsxn(rDoc.mnMargt),
    mnMargbsxn(rDoc.mnMargb), mnGutterxsn(rDoc.mnGutter), mnHeadery(720),
    mnFootery(720), mnPgnStarts(rDoc.mnPgnStart), mnCols(1), mnColsx(720),
    mnStextflow(rDoc.mbRTLdoc ? 3 : 0), mnBkc(2), mbLndscpsxn(rDoc.mbLandscape),
    mbTitlepg(false), mbFacpgsxn(rDoc.mbFacingp), mbRTLsection(rDoc.mbRTLdoc),
    mbPgnrestart(false), mbTitlePageHdFtUsed(false), mbPageHdFtUsed(false)
{
};

SectPageInformation::SectPageInformation(const SectPageInformation &rSect)
    : maColumns(rSect.maColumns), maBox(rSect.maBox),
    maNumType(rSect.maNumType), mpTitlePageHdFt(rSect.mpTitlePageHdFt),
    mpPageHdFt(rSect.mpPageHdFt), mnPgwsxn(rSect.mnPgwsxn),
    mnPghsxn(rSect.mnPghsxn), mnMarglsxn(rSect.mnMarglsxn),
    mnMargrsxn(rSect.mnMargrsxn), mnMargtsxn(rSect.mnMargtsxn),
    mnMargbsxn(rSect.mnMargbsxn), mnGutterxsn(rSect.mnGutterxsn),
    mnHeadery(rSect.mnHeadery), mnFootery(rSect.mnFootery),
    mnPgnStarts(rSect.mnPgnStarts), mnCols(rSect.mnCols),
    mnColsx(rSect.mnColsx), mnStextflow(rSect.mnStextflow), mnBkc(rSect.mnBkc),
    mbLndscpsxn(rSect.mbLndscpsxn), mbTitlepg(rSect.mbTitlepg),
    mbFacpgsxn(rSect.mbFacpgsxn), mbRTLsection(rSect.mbRTLsection),
    mbPgnrestart(rSect.mbPgnrestart),
    mbTitlePageHdFtUsed(rSect.mbTitlePageHdFtUsed),
    mbPageHdFtUsed(rSect.mbPageHdFtUsed)
{
};

rtfSection::rtfSection(const SwPosition &rPos,
    const SectPageInformation &rPageInfo)
    : maStart(rPos.nNode), maPageInfo(rPageInfo), mpSection(0), mpTitlePage(0),
    mpPage(0)
{
}

BookmarkPosition::BookmarkPosition(const SwPaM &rPaM)
    : maMkNode(rPaM.GetMark()->nNode),
    mnMkCntnt(rPaM.GetMark()->nContent.GetIndex())
{
}

BookmarkPosition::BookmarkPosition(const BookmarkPosition &rEntry)
    : maMkNode(rEntry.maMkNode), mnMkCntnt(rEntry.mnMkCntnt)
{
}

bool BookmarkPosition::operator==(const BookmarkPosition rhs)
{
    return(maMkNode.GetIndex() == rhs.maMkNode.GetIndex() && mnMkCntnt == rhs.mnMkCntnt);
}

sal_uLong SwNodeIdx::GetIdx() const
{
    return aIdx.GetIndex();
}

SvxNodeIdx* SwNodeIdx::Clone() const
{
    return new SwNodeIdx( aIdx );
}

SvxPosition* SwxPosition::Clone() const
{
    return new SwxPosition( pPam );
}

SvxNodeIdx* SwxPosition::MakeNodeIdx() const
{
    return new SwNodeIdx( pPam->GetPoint()->nNode );
}

sal_uLong   SwxPosition::GetNodeIdx() const
{
    return pPam->GetPoint()->nNode.GetIndex();
}

xub_StrLen SwxPosition::GetCntIdx() const
{
    return pPam->GetPoint()->nContent.GetIndex();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
