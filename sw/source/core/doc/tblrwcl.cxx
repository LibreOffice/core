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
#include <com/sun/star/text/HoriOrientation.hpp>
#include <hintids.hxx>

#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <tools/fract.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <docsh.hxx>
#include <fesh.hxx>
#include <tabfrm.hxx>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <fldbas.hxx>
#include <rowfrm.hxx>
#include <ddefld.hxx>
#include <hints.hxx>
#include <UndoTable.hxx>
#include <cellatr.hxx>
#include <mvsave.hxx>
#include <swtblfmt.hxx>
#include <swddetbl.hxx>
#include <poolfmt.hxx>
#include <tblrwcl.hxx>
#include <unochart.hxx>
#include <o3tl/numeric.hxx>
#include <calbck.hxx>
#include <docary.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;

#define COLFUZZY 20
#define ROWFUZZY 10

#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif

namespace {

// In order to set the Frame Formats for the Boxes, it's enough to look
// up the current one in the array. If it's already there return the new one.
struct CpyTabFrame
{
    SwFrameFormat* pFrameFormat;
    SwTableBoxFormat *pNewFrameFormat;

    explicit CpyTabFrame(SwFrameFormat* pCurrentFrameFormat) : pNewFrameFormat( nullptr )
    {   pFrameFormat = pCurrentFrameFormat; }

    bool operator==( const CpyTabFrame& rCpyTabFrame ) const
        { return pFrameFormat == rCpyTabFrame.pFrameFormat; }
    bool operator<( const CpyTabFrame& rCpyTabFrame ) const
        { return pFrameFormat < rCpyTabFrame.pFrameFormat; }
};

struct CR_SetBoxWidth
{
    SwShareBoxFormats aShareFormats;
    SwTableNode* pTableNd;
    SwTwips nDiff, nSide, nMaxSize, nLowerDiff;
    TableChgMode nMode;
    bool bBigger, bLeft;

    CR_SetBoxWidth( TableChgWidthHeightType eType, SwTwips nDif, SwTwips nSid,
                    SwTwips nMax, SwTableNode* pTNd )
        : pTableNd( pTNd ),
        nDiff( nDif ), nSide( nSid ), nMaxSize( nMax ), nLowerDiff( 0 )
    {
        bLeft = TableChgWidthHeightType::ColLeft == extractPosition( eType ) ||
                TableChgWidthHeightType::CellLeft == extractPosition( eType );
        bBigger = bool(eType & TableChgWidthHeightType::BiggerMode );
        nMode = pTableNd->GetTable().GetTableChgMode();
    }
    CR_SetBoxWidth( const CR_SetBoxWidth& rCpy )
        : pTableNd( rCpy.pTableNd ),
        nDiff( rCpy.nDiff ), nSide( rCpy.nSide ),
        nMaxSize( rCpy.nMaxSize ), nLowerDiff( 0 ),
        nMode( rCpy.nMode ),
        bBigger( rCpy.bBigger ), bLeft( rCpy.bLeft )
    {
    }

    void LoopClear()
    {
        nLowerDiff = 0;
    }
};

}

static bool lcl_SetSelBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                         SwTwips nDist, bool bCheck );
static bool lcl_SetOtherBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck );

typedef bool (*FN_lcl_SetBoxWidth)(SwTableLine*, CR_SetBoxWidth&, SwTwips, bool );

#ifdef DBG_UTIL

#define CHECKBOXWIDTH                                           \
    {                                                           \
        SwTwips nSize = GetFrameFormat()->GetFrameSize().GetWidth();   \
        for (size_t nTmp = 0; nTmp < m_aLines.size(); ++nTmp)   \
            ::CheckBoxWidth( *m_aLines[ nTmp ], nSize );         \
    }

#define CHECKTABLELAYOUT                                            \
    {                                                               \
        for ( size_t i = 0; i < GetTabLines().size(); ++i )        \
        {                                                           \
            SwFrameFormat* pFormat = GetTabLines()[i]->GetFrameFormat();  \
            SwIterator<SwRowFrame,SwFormat> aIter( *pFormat );              \
            for (SwRowFrame* pFrame=aIter.First(); pFrame; pFrame=aIter.Next())\
            {                                                       \
                if ( pFrame->GetTabLine() == GetTabLines()[i] )       \
                {                                               \
                    OSL_ENSURE( pFrame->GetUpper()->IsTabFrame(),       \
                                "Table layout does not match table structure" );       \
                }                                               \
            }                                                       \
        }                                                           \
    }

#else

#define CHECKBOXWIDTH
#define CHECKTABLELAYOUT

#endif // DBG_UTIL

namespace {

struct CR_SetLineHeight
{
    SwTableNode* pTableNd;
    SwTwips nMaxSpace, nMaxHeight;
    TableChgMode nMode;
    bool bBigger;

    CR_SetLineHeight( TableChgWidthHeightType eType, SwTableNode* pTNd )
        : pTableNd( pTNd ),
        nMaxSpace( 0 ), nMaxHeight( 0 )
    {
        bBigger = bool(eType & TableChgWidthHeightType::BiggerMode );
        nMode = pTableNd->GetTable().GetTableChgMode();
    }
    CR_SetLineHeight( const CR_SetLineHeight& rCpy )
        : pTableNd( rCpy.pTableNd ),
        nMaxSpace( rCpy.nMaxSpace ), nMaxHeight( rCpy.nMaxHeight ),
        nMode( rCpy.nMode ),
        bBigger( rCpy.bBigger )
    {}
};

}

static bool lcl_SetSelLineHeight( SwTableLine* pLine, const CR_SetLineHeight& rParam,
                         SwTwips nDist, bool bCheck );
static bool lcl_SetOtherLineHeight( SwTableLine* pLine, const CR_SetLineHeight& rParam,
                                SwTwips nDist, bool bCheck );

typedef bool (*FN_lcl_SetLineHeight)(SwTableLine*, CR_SetLineHeight&, SwTwips, bool );

typedef o3tl::sorted_vector<CpyTabFrame> CpyTabFrames;

namespace {

struct CpyPara
{
    std::shared_ptr< std::vector< std::vector< sal_uLong > > > pWidths;
    SwDoc& rDoc;
    SwTableNode* pTableNd;
    CpyTabFrames& rTabFrameArr;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    sal_uLong nOldSize, nNewSize;           // in order to correct the size attributes
    sal_uLong nMinLeft, nMaxRight;
    sal_uInt16 nCpyCnt, nInsPos;
    sal_uInt16 nLnIdx, nBoxIdx;
    sal_uInt8 nDelBorderFlag;
    bool bCpyContent;

    CpyPara( SwTableNode* pNd, sal_uInt16 nCopies, CpyTabFrames& rFrameArr )
        : rDoc( pNd->GetDoc() ), pTableNd( pNd ), rTabFrameArr(rFrameArr),
        pInsLine(nullptr), pInsBox(nullptr), nOldSize(0), nNewSize(0),
        nMinLeft(ULONG_MAX), nMaxRight(0),
        nCpyCnt(nCopies), nInsPos(0),
        nLnIdx(0), nBoxIdx(0),
        nDelBorderFlag(0), bCpyContent( true )
        {}
    CpyPara( const CpyPara& rPara, SwTableLine* pLine )
        : pWidths( rPara.pWidths ), rDoc(rPara.rDoc), pTableNd(rPara.pTableNd),
        rTabFrameArr(rPara.rTabFrameArr), pInsLine(pLine), pInsBox(rPara.pInsBox),
        nOldSize(0), nNewSize(rPara.nNewSize), nMinLeft( rPara.nMinLeft ),
        nMaxRight( rPara.nMaxRight ), nCpyCnt(rPara.nCpyCnt), nInsPos(0),
        nLnIdx( rPara.nLnIdx), nBoxIdx( rPara.nBoxIdx ),
        nDelBorderFlag( rPara.nDelBorderFlag ), bCpyContent( rPara.bCpyContent )
        {}
    CpyPara( const CpyPara& rPara, SwTableBox* pBox )
        : pWidths( rPara.pWidths ), rDoc(rPara.rDoc), pTableNd(rPara.pTableNd),
        rTabFrameArr(rPara.rTabFrameArr), pInsLine(rPara.pInsLine), pInsBox(pBox),
        nOldSize(rPara.nOldSize), nNewSize(rPara.nNewSize),
        nMinLeft( rPara.nMinLeft ), nMaxRight( rPara.nMaxRight ),
        nCpyCnt(rPara.nCpyCnt), nInsPos(0), nLnIdx(rPara.nLnIdx), nBoxIdx(rPara.nBoxIdx),
        nDelBorderFlag( rPara.nDelBorderFlag ), bCpyContent( rPara.bCpyContent )
        {}
};

}

static void lcl_CopyRow(FndLine_ & rFndLine, CpyPara *const pCpyPara);

static void lcl_CopyCol( FndBox_ & rFndBox, CpyPara *const pCpyPara)
{
    // Look up the Frame Format in the Frame Format Array
    SwTableBox* pBox = rFndBox.GetBox();
    CpyTabFrame aFindFrame(pBox->GetFrameFormat());

    if( pCpyPara->nCpyCnt )
    {
        sal_uInt16 nFndPos;
        CpyTabFrames::const_iterator itFind = pCpyPara->rTabFrameArr.lower_bound( aFindFrame );
        nFndPos = itFind - pCpyPara->rTabFrameArr.begin();
        if( itFind == pCpyPara->rTabFrameArr.end() || !(*itFind == aFindFrame) )
        {
            // For nested copying, also save the new Format as an old one.
            SwTableBoxFormat* pNewFormat = static_cast<SwTableBoxFormat*>(pBox->ClaimFrameFormat());

            // Find the selected Boxes in the Line:
            FndLine_ const* pCmpLine = nullptr;
            SwFormatFrameSize aFrameSz( pNewFormat->GetFrameSize() );

            bool bDiffCount = false;
            if( !pBox->GetTabLines().empty() )
            {
                pCmpLine = rFndBox.GetLines().front().get();
                if ( pCmpLine->GetBoxes().size() != pCmpLine->GetLine()->GetTabBoxes().size() )
                    bDiffCount = true;
            }

            if( bDiffCount )
            {
                // The first Line should be enough
                FndBoxes_t const& rFndBoxes = pCmpLine->GetBoxes();
                tools::Long nSz = 0;
                for( auto n = rFndBoxes.size(); n; )
                {
                    nSz += rFndBoxes[--n]->GetBox()->
                            GetFrameFormat()->GetFrameSize().GetWidth();
                }
                aFrameSz.SetWidth( aFrameSz.GetWidth() -
                                            nSz / ( pCpyPara->nCpyCnt + 1 ) );
                pNewFormat->SetFormatAttr( aFrameSz );
                aFrameSz.SetWidth( nSz / ( pCpyPara->nCpyCnt + 1 ) );

                // Create a new Format for the new Box, specifying its size.
                aFindFrame.pNewFrameFormat = reinterpret_cast<SwTableBoxFormat*>(pNewFormat->GetDoc()->
                                            MakeTableLineFormat());
                *aFindFrame.pNewFrameFormat = *pNewFormat;
                aFindFrame.pNewFrameFormat->SetFormatAttr( aFrameSz );
            }
            else
            {
                aFrameSz.SetWidth( aFrameSz.GetWidth() / ( pCpyPara->nCpyCnt + 1 ) );
                pNewFormat->SetFormatAttr( aFrameSz );

                aFindFrame.pNewFrameFormat = pNewFormat;
                pCpyPara->rTabFrameArr.insert( aFindFrame );
                aFindFrame.pFrameFormat = pNewFormat;
                pCpyPara->rTabFrameArr.insert( aFindFrame );
            }
        }
        else
        {
            aFindFrame = pCpyPara->rTabFrameArr[ nFndPos ];
            pBox->ChgFrameFormat( aFindFrame.pNewFrameFormat );
        }
    }
    else
    {
        CpyTabFrames::const_iterator itFind = pCpyPara->rTabFrameArr.find( aFindFrame );
        if( pCpyPara->nDelBorderFlag &&
            itFind != pCpyPara->rTabFrameArr.end() )
            aFindFrame = *itFind;
        else
            aFindFrame.pNewFrameFormat = static_cast<SwTableBoxFormat*>(pBox->GetFrameFormat());
    }

    if (!rFndBox.GetLines().empty())
    {
        pBox = new SwTableBox( aFindFrame.pNewFrameFormat,
                    rFndBox.GetLines().size(), pCpyPara->pInsLine );
        pCpyPara->pInsLine->GetTabBoxes().insert( pCpyPara->pInsLine->GetTabBoxes().begin() + pCpyPara->nInsPos++, pBox );
        CpyPara aPara( *pCpyPara, pBox );
        aPara.nDelBorderFlag &= 7;

        for (auto const& pFndLine : rFndBox.GetLines())
        {
            lcl_CopyRow(*pFndLine, &aPara);
        }
    }
    else
    {
        ::InsTableBox( pCpyPara->rDoc, pCpyPara->pTableNd, pCpyPara->pInsLine,
                    aFindFrame.pNewFrameFormat, pBox, pCpyPara->nInsPos++ );

        const FndBoxes_t& rFndBxs = rFndBox.GetUpper()->GetBoxes();
        if( 8 > pCpyPara->nDelBorderFlag
                ? pCpyPara->nDelBorderFlag != 0
                : &rFndBox == rFndBxs[rFndBxs.size() - 1].get())
        {
            const SvxBoxItem& rBoxItem = pBox->GetFrameFormat()->GetBox();
            if( 8 > pCpyPara->nDelBorderFlag
                    ? rBoxItem.GetTop()
                    : rBoxItem.GetRight() )
            {
                aFindFrame.pFrameFormat = pBox->GetFrameFormat();

                SvxBoxItem aNew( rBoxItem );
                if( 8 > pCpyPara->nDelBorderFlag )
                    aNew.SetLine( nullptr, SvxBoxItemLine::TOP );
                else
                    aNew.SetLine( nullptr, SvxBoxItemLine::RIGHT );

                if( 1 == pCpyPara->nDelBorderFlag ||
                    8 == pCpyPara->nDelBorderFlag )
                {
                    // For all Boxes that delete TopBorderLine, we copy after that
                    pBox = pCpyPara->pInsLine->GetTabBoxes()[
                                            pCpyPara->nInsPos - 1 ];
                }

                aFindFrame.pNewFrameFormat = static_cast<SwTableBoxFormat*>(pBox->GetFrameFormat());

                // Else we copy before that and the first Line keeps the TopLine
                // and we remove it at the original
                pBox->ClaimFrameFormat()->SetFormatAttr( aNew );

                if( !pCpyPara->nCpyCnt )
                    pCpyPara->rTabFrameArr.insert( aFindFrame );
            }
        }
    }
}

static void lcl_CopyRow(FndLine_& rFndLine, CpyPara *const pCpyPara)
{
    SwTableLine* pNewLine = new SwTableLine(
                            static_cast<SwTableLineFormat*>(rFndLine.GetLine()->GetFrameFormat()),
                        rFndLine.GetBoxes().size(), pCpyPara->pInsBox );
    if( pCpyPara->pInsBox )
    {
        SwTableLines& rLines = pCpyPara->pInsBox->GetTabLines();
        rLines.insert( rLines.begin() + pCpyPara->nInsPos++, pNewLine );
    }
    else
    {
        SwTableLines& rLines = pCpyPara->pTableNd->GetTable().GetTabLines();
        rLines.insert( rLines.begin() + pCpyPara->nInsPos++, pNewLine );
    }

    CpyPara aPara( *pCpyPara, pNewLine );
    for (auto const& it : rFndLine.GetBoxes())
    {
        lcl_CopyCol(*it, &aPara);
    }

    pCpyPara->nDelBorderFlag &= 0xf8;
}

static void lcl_InsCol( FndLine_* pFndLn, CpyPara& rCpyPara, sal_uInt16 nCpyCnt,
                bool bBehind )
{
    // Bug 29124: Not only copy in the BaseLines. If possible, we go down as far as possible
    FndBox_* pFBox;
    if( 1 == pFndLn->GetBoxes().size() &&
        !( pFBox = pFndLn->GetBoxes()[0].get() )->GetBox()->GetSttNd() )
    {
        // A Box with multiple Lines, so insert into these Lines
        for (auto &rpLine : pFBox->GetLines())
        {
            lcl_InsCol( rpLine.get(), rCpyPara, nCpyCnt, bBehind );
        }
    }
    else
    {
        rCpyPara.pInsLine = pFndLn->GetLine();
        SwTableBox* pBox = pFndLn->GetBoxes()[ bBehind ?
                    pFndLn->GetBoxes().size()-1 : 0 ]->GetBox();
        rCpyPara.nInsPos = pFndLn->GetLine()->GetBoxPos( pBox );
        if( bBehind )
            ++rCpyPara.nInsPos;

        for( sal_uInt16 n = 0; n < nCpyCnt; ++n )
        {
            if( n + 1 == nCpyCnt && bBehind )
                rCpyPara.nDelBorderFlag = 9;
            else
                rCpyPara.nDelBorderFlag = 8;
            for (auto const& it : pFndLn->GetBoxes())
            {
                lcl_CopyCol(*it, &rCpyPara);
            }
        }
    }
}

static SwRowFrame* GetRowFrame( SwTableLine& rLine )
{
    SwIterator<SwRowFrame,SwFormat> aIter( *rLine.GetFrameFormat() );
    for( SwRowFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
        if( pFrame->GetTabLine() == &rLine )
            return pFrame;
    return nullptr;
}

bool SwTable::InsertCol( SwDoc& rDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( !rBoxes.empty() && nCnt, "No valid Box List" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    bool bRes = true;
    if( IsNewModel() )
        bRes = NewInsertCol( rDoc, rBoxes, nCnt, bBehind );
    else
    {
        // Find all Boxes/Lines
        FndBox_ aFndBox( nullptr, nullptr );
        {
            FndPara aPara( rBoxes, &aFndBox );
            ForEach_FndLineCopyCol( GetTabLines(), &aPara );
        }
        if( aFndBox.GetLines().empty() )
            return false;

        SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());    // Delete HTML Layout

        // Find Lines for the layout update
        aFndBox.SetTableLines( *this );
        aFndBox.DelFrames( *this );

        // TL_CHART2: nothing to be done since chart2 currently does not want to
        // get notified about new rows/cols.

        CpyTabFrames aTabFrameArr;
        CpyPara aCpyPara( pTableNd, nCnt, aTabFrameArr );

        for (auto & rpLine : aFndBox.GetLines())
        {
            lcl_InsCol( rpLine.get(), aCpyPara, nCnt, bBehind );
        }

        // clean up this Line's structure once again, generally all of them
        GCLines();

        // Update Layout
        aFndBox.MakeFrames( *this );

        CHECKBOXWIDTH;
        CHECKTABLELAYOUT;
        bRes = true;
    }

    SwChartDataProvider *pPCD = rDoc.getIDocumentChartDataProviderAccess().GetChartDataProvider();
    if (pPCD && nCnt)
        pPCD->AddRowCols( *this, rBoxes, nCnt, bBehind );
    rDoc.UpdateCharts( GetFrameFormat()->GetName() );

    rDoc.GetDocShell()->GetFEShell()->UpdateTableStyleFormatting();

    return bRes;
}

bool SwTable::InsertRow_( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                        sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( pDoc && !rBoxes.empty() && nCnt, "No valid Box List" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

     // Find all Boxes/Lines
    FndBox_ aFndBox( nullptr, nullptr );
    {
        FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());   // Delete HTML Layout

    FndBox_* pFndBox = &aFndBox;
    {
        FndLine_* pFndLine;
        while( 1 == pFndBox->GetLines().size() )
        {
            pFndLine = pFndBox->GetLines()[0].get();
            if( 1 != pFndLine->GetBoxes().size() )
                break;
            // Don't go down too far! One Line with Box needs to remain!
            FndBox_ *const pTmpBox = pFndLine->GetBoxes().front().get();
            if( !pTmpBox->GetLines().empty() )
                pFndBox = pTmpBox;
            else
                break;
        }
    }

    // Find Lines for the layout update
    const bool bLayout = !IsNewModel() &&
        nullptr != SwIterator<SwTabFrame,SwFormat>( *GetFrameFormat() ).First();

    if ( bLayout )
    {
        aFndBox.SetTableLines( *this );
        if( pFndBox != &aFndBox )
            aFndBox.DelFrames( *this );
        // TL_CHART2: nothing to be done since chart2 currently does not want to
        // get notified about new rows/cols.
    }

    CpyTabFrames aTabFrameArr;
    CpyPara aCpyPara( pTableNd, 0, aTabFrameArr );

    SwTableLine* pLine = pFndBox->GetLines()[ bBehind ?
                    pFndBox->GetLines().size()-1 : 0 ]->GetLine();
    if( &aFndBox == pFndBox )
        aCpyPara.nInsPos = GetTabLines().GetPos( pLine );
    else
    {
        aCpyPara.pInsBox = pFndBox->GetBox();
        aCpyPara.nInsPos = pFndBox->GetBox()->GetTabLines().GetPos( pLine );
    }

    if( bBehind )
    {
        ++aCpyPara.nInsPos;
        aCpyPara.nDelBorderFlag = 1;
    }
    else
        aCpyPara.nDelBorderFlag = 2;

    for( sal_uInt16 nCpyCnt = 0; nCpyCnt < nCnt; ++nCpyCnt )
    {
        if( bBehind )
            aCpyPara.nDelBorderFlag = 1;
        for (auto & rpFndLine : pFndBox->GetLines())
            lcl_CopyRow( *rpFndLine, &aCpyPara );
    }

    // clean up this Line's structure once again, generally all of them
    if( !pDoc->IsInReading() )
        GCLines();

    // Update Layout
    if ( bLayout )
    {
        if( pFndBox != &aFndBox )
            aFndBox.MakeFrames( *this );
        else
            aFndBox.MakeNewFrames( *this, nCnt, bBehind );
    }

    CHECKBOXWIDTH;
    CHECKTABLELAYOUT;

    SwChartDataProvider *pPCD = pDoc->getIDocumentChartDataProviderAccess().GetChartDataProvider();
    if (pPCD && nCnt)
        pPCD->AddRowCols( *this, rBoxes, nCnt, bBehind );
    pDoc->UpdateCharts( GetFrameFormat()->GetName() );

    pDoc->GetDocShell()->GetFEShell()->UpdateTableStyleFormatting();

    return true;
}

static void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const tools::Long nOffset,
                            bool bFirst, SwShareBoxFormats& rShareFormats );

static void lcl_LastBoxSetWidthLine( SwTableLines &rLines, const tools::Long nOffset,
                                bool bFirst, SwShareBoxFormats& rShareFormats )
{
    for ( auto pLine : rLines )
        ::lcl_LastBoxSetWidth( pLine->GetTabBoxes(), nOffset, bFirst, rShareFormats );
}

static void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const tools::Long nOffset,
                            bool bFirst, SwShareBoxFormats& rShareFormats )
{
    SwTableBox& rBox = *(bFirst ? rBoxes.front() : rBoxes.back());
    if( !rBox.GetSttNd() )
        ::lcl_LastBoxSetWidthLine( rBox.GetTabLines(), nOffset,
                                    bFirst, rShareFormats );

    // Adapt the Box
    const SwFrameFormat *pBoxFormat = rBox.GetFrameFormat();
    SwFormatFrameSize aNew( pBoxFormat->GetFrameSize() );
    aNew.SetWidth( aNew.GetWidth() + nOffset );
    SwFrameFormat *pFormat = rShareFormats.GetFormat( *pBoxFormat, aNew );
    if( pFormat )
        rBox.ChgFrameFormat( static_cast<SwTableBoxFormat*>(pFormat) );
    else
    {
        pFormat = rBox.ClaimFrameFormat();

        pFormat->LockModify();
        pFormat->SetFormatAttr( aNew );
        pFormat->UnlockModify();

        rShareFormats.AddFormat( *pBoxFormat, *pFormat );
    }
}

void DeleteBox_( SwTable& rTable, SwTableBox* pBox, SwUndo* pUndo,
                bool bCalcNewSize, const bool bCorrBorder,
                SwShareBoxFormats* pShareFormats )
{
    do {
        SwTwips nBoxSz = bCalcNewSize ?
                pBox->GetFrameFormat()->GetFrameSize().GetWidth() : 0;
        SwTableLine* pLine = pBox->GetUpper();
        SwTableBoxes& rTableBoxes = pLine->GetTabBoxes();
        sal_uInt16 nDelPos = pLine->GetBoxPos( pBox );
        SwTableBox* pUpperBox = pBox->GetUpper()->GetUpper();

        // Special treatment for the border:
        if( bCorrBorder && 1 < rTableBoxes.size() )
        {
            const SvxBoxItem& rBoxItem = pBox->GetFrameFormat()->GetBox();

            if( rBoxItem.GetLeft() || rBoxItem.GetRight() )
            {
                bool bChgd = false;

                // JP 02.04.97: 1st part for Bug 36271
                // First the left/right edges
                if( nDelPos + 1 < static_cast<sal_uInt16>(rTableBoxes.size()) )
                {
                    SwTableBox* pNxtBox = rTableBoxes[ nDelPos + 1 ];
                    const SvxBoxItem& rNxtBoxItem = pNxtBox->GetFrameFormat()->GetBox();

                    SwTableBox* pPrvBox = nDelPos ? rTableBoxes[ nDelPos - 1 ] : nullptr;

                    if( pNxtBox->GetSttNd() && !rNxtBoxItem.GetLeft() &&
                        ( !pPrvBox || !pPrvBox->GetFrameFormat()->GetBox().GetRight()) )
                    {
                        SvxBoxItem aTmp( rNxtBoxItem );
                        aTmp.SetLine( rBoxItem.GetLeft() ? rBoxItem.GetLeft()
                                                         : rBoxItem.GetRight(),
                                                            SvxBoxItemLine::LEFT );
                        if( pShareFormats )
                            pShareFormats->SetAttr( *pNxtBox, aTmp );
                        else
                            pNxtBox->ClaimFrameFormat()->SetFormatAttr( aTmp );
                        bChgd = true;
                    }
                }
                if( !bChgd && nDelPos )
                {
                    SwTableBox* pPrvBox = rTableBoxes[ nDelPos - 1 ];
                    const SvxBoxItem& rPrvBoxItem = pPrvBox->GetFrameFormat()->GetBox();

                    SwTableBox* pNxtBox = nDelPos + 1 < static_cast<sal_uInt16>(rTableBoxes.size())
                                            ? rTableBoxes[ nDelPos + 1 ] : nullptr;

                    if( pPrvBox->GetSttNd() && !rPrvBoxItem.GetRight() &&
                        ( !pNxtBox || !pNxtBox->GetFrameFormat()->GetBox().GetLeft()) )
                    {
                        SvxBoxItem aTmp( rPrvBoxItem );
                        aTmp.SetLine( rBoxItem.GetLeft() ? rBoxItem.GetLeft()
                                                         : rBoxItem.GetRight(),
                                                            SvxBoxItemLine::RIGHT );
                        if( pShareFormats )
                            pShareFormats->SetAttr( *pPrvBox, aTmp );
                        else
                            pPrvBox->ClaimFrameFormat()->SetFormatAttr( aTmp );
                    }
                }
            }
        }

        // Delete the Box first, then the Nodes!
        SwStartNode* pSttNd = const_cast<SwStartNode*>(pBox->GetSttNd());
        if( pShareFormats )
            pShareFormats->RemoveFormat( *rTableBoxes[ nDelPos ]->GetFrameFormat() );

        // Before deleting the 'Table Box' from memory - delete any redlines attached to it
        if ( rTable.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess().HasExtraRedlineTable() )
            rTable.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteTableCellRedline( rTable.GetFrameFormat()->GetDoc(), *(rTableBoxes[nDelPos]), true, RedlineType::Any );
        delete rTableBoxes[nDelPos];
        rTableBoxes.erase( rTableBoxes.begin() + nDelPos );

        if( pSttNd )
        {
            // Has the UndoObject been prepared to save the Section?
            if( pUndo && pUndo->IsDelBox() )
                static_cast<SwUndoTableNdsChg*>(pUndo)->SaveSection( pSttNd );
            else
                pSttNd->GetDoc().getIDocumentContentOperations().DeleteSection( pSttNd );
        }

        // Also delete the Line?
        if( !rTableBoxes.empty() )
        {
            // Then adapt the Frame-SSize
            bool bLastBox = nDelPos == rTableBoxes.size();
            if( bLastBox )
                --nDelPos;
            pBox = rTableBoxes[nDelPos];
            if( bCalcNewSize )
            {
                SwFormatFrameSize aNew( pBox->GetFrameFormat()->GetFrameSize() );
                aNew.SetWidth( aNew.GetWidth() + nBoxSz );
                if( pShareFormats )
                    pShareFormats->SetSize( *pBox, aNew );
                else
                    pBox->ClaimFrameFormat()->SetFormatAttr( aNew );

                if( !pBox->GetSttNd() )
                {
                    // We need to this recursively in all Lines in all Cells!
                    SwShareBoxFormats aShareFormats;
                    ::lcl_LastBoxSetWidthLine( pBox->GetTabLines(), nBoxSz,
                                                !bLastBox,
                                                pShareFormats ? *pShareFormats
                                                           : aShareFormats );
                }
            }
            break;      // Stop deleting
        }
        // Delete the Line from the Table/Box
        if( !pUpperBox )
        {
            // Also delete the Line from the Table
            nDelPos = rTable.GetTabLines().GetPos( pLine );
            if( pShareFormats )
                pShareFormats->RemoveFormat( *rTable.GetTabLines()[ nDelPos ]->GetFrameFormat() );

            SwTableLine* pTabLineToDelete = rTable.GetTabLines()[ nDelPos ];
            // Before deleting the 'Table Line' from memory - delete any redlines attached to it
            if ( rTable.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess().HasExtraRedlineTable() )
                rTable.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteTableRowRedline( rTable.GetFrameFormat()->GetDoc(), *pTabLineToDelete, true, RedlineType::Any );
            delete pTabLineToDelete;
            rTable.GetTabLines().erase( rTable.GetTabLines().begin() + nDelPos );
            break;      // we cannot delete more
        }

        // finally also delete the Line
        pBox = pUpperBox;
        nDelPos = pBox->GetTabLines().GetPos( pLine );
        if( pShareFormats )
            pShareFormats->RemoveFormat( *pBox->GetTabLines()[ nDelPos ]->GetFrameFormat() );

        SwTableLine* pTabLineToDelete = pBox->GetTabLines()[ nDelPos ];
        // Before deleting the 'Table Line' from memory - delete any redlines attached to it
        if ( rTable.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess().HasExtraRedlineTable() )
            rTable.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteTableRowRedline( rTable.GetFrameFormat()->GetDoc(), *pTabLineToDelete, true, RedlineType::Any );
        delete pTabLineToDelete;
        pBox->GetTabLines().erase( pBox->GetTabLines().begin() + nDelPos );
    } while( pBox->GetTabLines().empty() );
}

static SwTableBox*
lcl_FndNxtPrvDelBox( const SwTableLines& rTableLns,
                                SwTwips nBoxStt, SwTwips nBoxWidth,
                                sal_uInt16 nLinePos, bool bNxt,
                                SwSelBoxes* pAllDelBoxes, size_t *const pCurPos)
{
    SwTableBox* pFndBox = nullptr;
    do {
        if( bNxt )
            ++nLinePos;
        else
            --nLinePos;
        SwTableLine* pLine = rTableLns[ nLinePos ];
        SwTwips nFndBoxWidth = 0;
        SwTwips nFndWidth = nBoxStt + nBoxWidth;

        pFndBox = pLine->GetTabBoxes()[ 0 ];
        for( auto pBox : pLine->GetTabBoxes() )
        {
            if ( nFndWidth <= 0 )
            {
                break;
            }
            pFndBox = pBox;
            nFndBoxWidth = pFndBox->GetFrameFormat()->GetFrameSize().GetWidth();
            nFndWidth -= nFndBoxWidth;
        }

        // Find the first ContentBox
        while( !pFndBox->GetSttNd() )
        {
            const SwTableLines& rLowLns = pFndBox->GetTabLines();
            if( bNxt )
                pFndBox = rLowLns.front()->GetTabBoxes().front();
            else
                pFndBox = rLowLns.back()->GetTabBoxes().front();
        }

        if( std::abs( nFndWidth ) > COLFUZZY ||
            std::abs( nBoxWidth - nFndBoxWidth ) > COLFUZZY )
            pFndBox = nullptr;
        else if( pAllDelBoxes )
        {
            // If the predecessor will also be deleted, there's nothing to do
            SwSelBoxes::const_iterator aFndIt = pAllDelBoxes->find( pFndBox);
            if( aFndIt == pAllDelBoxes->end() )
                break;
            size_t const nFndPos = aFndIt - pAllDelBoxes->begin() ;

            // else, we keep on searching.
            // We do not need to recheck the Box, however
            pFndBox = nullptr;
            if( nFndPos <= *pCurPos )
                --*pCurPos;
            pAllDelBoxes->erase( pAllDelBoxes->begin() + nFndPos );
        }
    } while( bNxt ? ( nLinePos + 1 < static_cast<sal_uInt16>(rTableLns.size()) ) : nLinePos != 0 );
    return pFndBox;
}

static void
lcl_SaveUpperLowerBorder( SwTable& rTable, const SwTableBox& rBox,
                                SwShareBoxFormats& rShareFormats,
                                SwSelBoxes* pAllDelBoxes = nullptr,
                                size_t *const pCurPos = nullptr )
{
//JP 16.04.97:  2. part for Bug 36271
    const SwTableLine* pLine = rBox.GetUpper();
    const SwTableBoxes& rTableBoxes = pLine->GetTabBoxes();
    const SwTableBox* pUpperBox = &rBox;
    sal_uInt16 nDelPos = pLine->GetBoxPos( pUpperBox );
    pUpperBox = rBox.GetUpper()->GetUpper();
    const SvxBoxItem& rBoxItem = rBox.GetFrameFormat()->GetBox();

    // then the top/bottom edges
    if( !rBoxItem.GetTop() && !rBoxItem.GetBottom() )
        return;

    bool bChgd = false;
    const SwTableLines* pTableLns;
    if( pUpperBox )
        pTableLns = &pUpperBox->GetTabLines();
    else
        pTableLns = &rTable.GetTabLines();

    sal_uInt16 nLnPos = pTableLns->GetPos( pLine );

    // Calculate the attribute position of the top-be-deleted Box and then
    // search in the top/bottom Line of the respective counterparts.
    SwTwips nBoxStt = 0;
    for( sal_uInt16 n = 0; n < nDelPos; ++n )
        nBoxStt += rTableBoxes[ n ]->GetFrameFormat()->GetFrameSize().GetWidth();
    SwTwips nBoxWidth = rBox.GetFrameFormat()->GetFrameSize().GetWidth();

    SwTableBox *pPrvBox = nullptr, *pNxtBox = nullptr;
    if( nLnPos )        // Predecessor?
        pPrvBox = ::lcl_FndNxtPrvDelBox( *pTableLns, nBoxStt, nBoxWidth,
                            nLnPos, false, pAllDelBoxes, pCurPos );

    if( nLnPos + 1 < static_cast<sal_uInt16>(pTableLns->size()) )     // Successor?
        pNxtBox = ::lcl_FndNxtPrvDelBox( *pTableLns, nBoxStt, nBoxWidth,
                            nLnPos, true, pAllDelBoxes, pCurPos );

    if( pNxtBox && pNxtBox->GetSttNd() )
    {
        const SvxBoxItem& rNxtBoxItem = pNxtBox->GetFrameFormat()->GetBox();
        if( !rNxtBoxItem.GetTop() && ( !pPrvBox ||
            !pPrvBox->GetFrameFormat()->GetBox().GetBottom()) )
        {
            SvxBoxItem aTmp( rNxtBoxItem );
            aTmp.SetLine( rBoxItem.GetTop() ? rBoxItem.GetTop()
                                            : rBoxItem.GetBottom(),
                                            SvxBoxItemLine::TOP );
            rShareFormats.SetAttr( *pNxtBox, aTmp );
            bChgd = true;
        }
    }
    if( !(!bChgd && pPrvBox && pPrvBox->GetSttNd()) )
        return;

    const SvxBoxItem& rPrvBoxItem = pPrvBox->GetFrameFormat()->GetBox();
    if( !rPrvBoxItem.GetTop() && ( !pNxtBox ||
        !pNxtBox->GetFrameFormat()->GetBox().GetTop()) )
    {
        SvxBoxItem aTmp( rPrvBoxItem );
        aTmp.SetLine( rBoxItem.GetTop() ? rBoxItem.GetTop()
                                        : rBoxItem.GetBottom(),
                                        SvxBoxItemLine::BOTTOM );
        rShareFormats.SetAttr( *pPrvBox, aTmp );
    }

}

bool SwTable::DeleteSel(
    SwDoc*     pDoc
    ,
    const SwSelBoxes& rBoxes,
    const SwSelBoxes* pMerged, SwUndo* pUndo,
    const bool bDelMakeFrames, const bool bCorrBorder )
{
    OSL_ENSURE( pDoc, "No doc?" );
    SwTableNode* pTableNd = nullptr;
    if( !rBoxes.empty() )
    {
        pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
        if( !pTableNd )
            return false;
    }

    SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());    // Delete HTML Layout

    // Find Lines for the Layout update
    FndBox_ aFndBox( nullptr, nullptr );
    if ( bDelMakeFrames )
    {
        if( pMerged && !pMerged->empty() )
            aFndBox.SetTableLines( *pMerged, *this );
        else if( !rBoxes.empty() )
            aFndBox.SetTableLines( rBoxes, *this );
        aFndBox.DelFrames( *this );
    }

    SwShareBoxFormats aShareFormats;

    // First switch the Border, then delete
    if( bCorrBorder )
    {
        SwSelBoxes aBoxes( rBoxes );
        for (size_t n = 0; n < aBoxes.size(); ++n)
        {
            ::lcl_SaveUpperLowerBorder( *this, *rBoxes[ n ], aShareFormats,
                                        &aBoxes, &n );
        }
    }

    PrepareDelBoxes( rBoxes );

    SwChartDataProvider *pPCD = pDoc->getIDocumentChartDataProviderAccess().GetChartDataProvider();
    // Delete boxes from last to first
    for (size_t n = 0; n < rBoxes.size(); ++n)
    {
        size_t const nIdx = rBoxes.size() - 1 - n;

        // First adapt the data-sequence for chart if necessary
        // (needed to move the implementation cursor properly to its new
        // position which can't be done properly if the cell is already gone)
        if (pPCD && pTableNd)
            pPCD->DeleteBox( &pTableNd->GetTable(), *rBoxes[nIdx] );

        // ... then delete the boxes
        DeleteBox_( *this, rBoxes[nIdx], pUndo, true, bCorrBorder, &aShareFormats );
    }

    // then clean up the structure of all Lines
    GCLines();

    if( bDelMakeFrames && aFndBox.AreLinesToRestore( *this ) )
        aFndBox.MakeFrames( *this );

    // TL_CHART2: now inform chart that sth has changed
    pDoc->UpdateCharts( GetFrameFormat()->GetName() );

    CHECKTABLELAYOUT;
    CHECK_TABLE( *this );

    return true;
}

bool SwTable::OldSplitRow( SwDoc& rDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt,
                        bool bSameHeight )
{
    OSL_ENSURE( !rBoxes.empty() && nCnt, "No valid values" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    // TL_CHART2: splitting/merging of a number of cells or rows will usually make
    // the table too complex to be handled with chart.
    // Thus we tell the charts to use their own data provider and forget about this table
    rDoc.getIDocumentChartDataProviderAccess().CreateChartInternalDataProviders( this );

    SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());    // Delete HTML Layout

    // If the rows should get the same (min) height, we first have
    // to store the old row heights before deleting the frames
    std::unique_ptr<tools::Long[]> pRowHeights;
    if ( bSameHeight )
    {
        pRowHeights.reset(new tools::Long[ rBoxes.size() ]);
        for (size_t n = 0; n < rBoxes.size(); ++n)
        {
            SwTableBox* pSelBox = rBoxes[n];
            const SwRowFrame* pRow = GetRowFrame( *pSelBox->GetUpper() );
            OSL_ENSURE( pRow, "Where is the SwTableLine's Frame?" );
            SwRectFnSet aRectFnSet(pRow);
            pRowHeights[ n ] = aRectFnSet.GetHeight(pRow->getFrameArea());
        }
    }

    // Find Lines for the Layout update
    FndBox_ aFndBox( nullptr, nullptr );
    aFndBox.SetTableLines( rBoxes, *this );
    aFndBox.DelFrames( *this );

    for (size_t n = 0; n < rBoxes.size(); ++n)
    {
        SwTableBox* pSelBox = rBoxes[n];
        OSL_ENSURE( pSelBox, "Box is not within the Table" );

        // Insert nCnt new Lines into the Box
        SwTableLine* pInsLine = pSelBox->GetUpper();
        SwTableBoxFormat* pFrameFormat = static_cast<SwTableBoxFormat*>(pSelBox->GetFrameFormat());

        // Respect the Line's height, reset if needed
        SwFormatFrameSize aFSz( pInsLine->GetFrameFormat()->GetFrameSize() );
        if ( bSameHeight && SwFrameSize::Variable == aFSz.GetHeightSizeType() )
            aFSz.SetHeightSizeType( SwFrameSize::Minimum );

        bool bChgLineSz = 0 != aFSz.GetHeight() || bSameHeight;
        if ( bChgLineSz )
            aFSz.SetHeight( ( bSameHeight ? pRowHeights[ n ] : aFSz.GetHeight() ) /
                             (nCnt + 1) );

        SwTableBox* pNewBox = new SwTableBox( pFrameFormat, nCnt, pInsLine );
        sal_uInt16 nBoxPos = pInsLine->GetBoxPos( pSelBox );
        pInsLine->GetTabBoxes()[nBoxPos] = pNewBox; // overwrite old one

        // Delete background/border attribute
        SwTableBox* pLastBox = pSelBox;         // To distribute the TextNodes!
        // If Areas are contained in the Box, it stays as is
        // !! If this is changed we need to adapt the Undo, too !!!
        bool bMoveNodes = true;
        {
            sal_uLong nSttNd = pLastBox->GetSttIdx() + 1,
                    nEndNd = pLastBox->GetSttNd()->EndOfSectionIndex();
            while( nSttNd < nEndNd )
                if( !rDoc.GetNodes()[ nSttNd++ ]->IsTextNode() )
                {
                    bMoveNodes = false;
                    break;
                }
        }

        SwTableBoxFormat* pCpyBoxFrameFormat = static_cast<SwTableBoxFormat*>(pSelBox->GetFrameFormat());
        bool bChkBorder = nullptr != pCpyBoxFrameFormat->GetBox().GetTop();
        if( bChkBorder )
            pCpyBoxFrameFormat = static_cast<SwTableBoxFormat*>(pSelBox->ClaimFrameFormat());

        for( sal_uInt16 i = 0; i <= nCnt; ++i )
        {
            // Create a new Line in the new Box
            SwTableLine* pNewLine = new SwTableLine(
                    static_cast<SwTableLineFormat*>(pInsLine->GetFrameFormat()), 1, pNewBox );
            if( bChgLineSz )
            {
                pNewLine->ClaimFrameFormat()->SetFormatAttr( aFSz );
            }

            pNewBox->GetTabLines().insert( pNewBox->GetTabLines().begin() + i, pNewLine );
            // then a new Box in the Line
            if( !i )        // hang up the original Box
            {
                pSelBox->SetUpper( pNewLine );
                pNewLine->GetTabBoxes().insert( pNewLine->GetTabBoxes().begin(), pSelBox );
            }
            else
            {
                ::InsTableBox( rDoc, pTableNd, pNewLine, pCpyBoxFrameFormat,
                                pLastBox, 0 );

                if( bChkBorder )
                {
                    pCpyBoxFrameFormat = static_cast<SwTableBoxFormat*>(pNewLine->GetTabBoxes()[ 0 ]->ClaimFrameFormat());
                    SvxBoxItem aTmp( pCpyBoxFrameFormat->GetBox() );
                    aTmp.SetLine( nullptr, SvxBoxItemLine::TOP );
                    pCpyBoxFrameFormat->SetFormatAttr( aTmp );
                    bChkBorder = false;
                }

                if( bMoveNodes )
                {
                    const SwNode* pEndNd = pLastBox->GetSttNd()->EndOfSectionNode();
                    if( pLastBox->GetSttIdx()+2 != pEndNd->GetIndex() )
                    {
                        // Move TextNodes
                        SwNodeRange aRg( *pLastBox->GetSttNd(), +2, *pEndNd );
                        pLastBox = pNewLine->GetTabBoxes()[0];  // reset
                        SwNodeIndex aInsPos( *pLastBox->GetSttNd(), 1 );
                        rDoc.GetNodes().MoveNodes(aRg, rDoc.GetNodes(), aInsPos, false);
                        rDoc.GetNodes().Delete( aInsPos ); // delete the empty one
                    }
                }
            }
        }
        // In Boxes with Lines, we can only have Size/Fillorder
        pFrameFormat = static_cast<SwTableBoxFormat*>(pNewBox->ClaimFrameFormat());
        pFrameFormat->ResetFormatAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
        pFrameFormat->ResetFormatAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
    }

    pRowHeights.reset();

    GCLines();

    aFndBox.MakeFrames( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT
    return true;
}

bool SwTable::SplitCol(SwDoc& rDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt)
{
    OSL_ENSURE( !rBoxes.empty() && nCnt, "No valid values" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    // TL_CHART2: splitting/merging of a number of cells or rows will usually make
    // the table too complex to be handled with chart.
    // Thus we tell the charts to use their own data provider and forget about this table
    rDoc.getIDocumentChartDataProviderAccess().CreateChartInternalDataProviders( this );

    SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());    // Delete HTML Layout
    SwSelBoxes aSelBoxes(rBoxes);
    ExpandSelection( aSelBoxes );

    // Find Lines for the Layout update
    FndBox_ aFndBox( nullptr, nullptr );
    aFndBox.SetTableLines( aSelBoxes, *this );
    aFndBox.DelFrames( *this );

    CpyTabFrames aFrameArr;
    std::vector<SwTableBoxFormat*> aLastBoxArr;
    for (size_t n = 0; n < aSelBoxes.size(); ++n)
    {
        SwTableBox* pSelBox = aSelBoxes[n];
        OSL_ENSURE( pSelBox, "Box is not in the table" );

        // We don't want to split small table cells into very very small cells
        if( pSelBox->GetFrameFormat()->GetFrameSize().GetWidth()/( nCnt + 1 ) < 10 )
            continue;

        // Then split the nCnt Box up into nCnt Boxes
        SwTableLine* pInsLine = pSelBox->GetUpper();
        sal_uInt16 nBoxPos = pInsLine->GetBoxPos( pSelBox );

        // Find the Frame Format in the Frame Format Array
        SwTableBoxFormat* pLastBoxFormat;
        CpyTabFrame aFindFrame( static_cast<SwTableBoxFormat*>(pSelBox->GetFrameFormat()) );
        CpyTabFrames::const_iterator itFind = aFrameArr.lower_bound( aFindFrame );
        const size_t nFndPos = itFind - aFrameArr.begin();
        if( itFind == aFrameArr.end() || !(*itFind == aFindFrame) )
        {
            // Change the FrameFormat
            aFindFrame.pNewFrameFormat = static_cast<SwTableBoxFormat*>(pSelBox->ClaimFrameFormat());
            SwTwips nBoxSz = aFindFrame.pNewFrameFormat->GetFrameSize().GetWidth();
            SwTwips nNewBoxSz = nBoxSz / ( nCnt + 1 );
            aFindFrame.pNewFrameFormat->SetFormatAttr( SwFormatFrameSize( SwFrameSize::Variable,
                                                        nNewBoxSz, 0 ) );
            aFrameArr.insert( aFindFrame );

            pLastBoxFormat = aFindFrame.pNewFrameFormat;
            if( nBoxSz != ( nNewBoxSz * (nCnt + 1)))
            {
                // We have a remainder, so we need to define an own Format
                // for the last Box.
                pLastBoxFormat = new SwTableBoxFormat( *aFindFrame.pNewFrameFormat );
                pLastBoxFormat->SetFormatAttr( SwFormatFrameSize( SwFrameSize::Variable,
                                nBoxSz - ( nNewBoxSz * nCnt ), 0 ) );
            }
            aLastBoxArr.insert( aLastBoxArr.begin() + nFndPos, pLastBoxFormat );
        }
        else
        {
            aFindFrame = aFrameArr[ nFndPos ];
            pSelBox->ChgFrameFormat( aFindFrame.pNewFrameFormat );
            pLastBoxFormat = aLastBoxArr[ nFndPos ];
        }

        // Insert the Boxes at the Position
        for( sal_uInt16 i = 1; i < nCnt; ++i )
            ::InsTableBox( rDoc, pTableNd, pInsLine, aFindFrame.pNewFrameFormat,
                        pSelBox, nBoxPos + i ); // insert after

        ::InsTableBox( rDoc, pTableNd, pInsLine, pLastBoxFormat,
                    pSelBox, nBoxPos + nCnt );  // insert after

        // Special treatment for the Border:
        const SvxBoxItem& aSelBoxItem = aFindFrame.pNewFrameFormat->GetBox();
        if( aSelBoxItem.GetRight() )
        {
            pInsLine->GetTabBoxes()[ nBoxPos + nCnt ]->ClaimFrameFormat();

            SvxBoxItem aTmp( aSelBoxItem );
            aTmp.SetLine( nullptr, SvxBoxItemLine::RIGHT );
            aFindFrame.pNewFrameFormat->SetFormatAttr( aTmp );

            // Remove the Format from the "cache"
            for( auto i = aFrameArr.size(); i; )
            {
                const CpyTabFrame& rCTF = aFrameArr[ --i ];
                if( rCTF.pNewFrameFormat == aFindFrame.pNewFrameFormat ||
                    rCTF.pFrameFormat == aFindFrame.pNewFrameFormat )
                {
                    aFrameArr.erase( aFrameArr.begin() + i );
                    aLastBoxArr.erase( aLastBoxArr.begin() + i );
                }
            }
        }
    }

    // Update Layout
    aFndBox.MakeFrames( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT
    return true;
}

/*
 * >> MERGE <<
 * Algorithm:
 *    If we only have one Line in the FndBox_, take this Line and test
 *    the Box count:
 * If we have more than one Box, we merge on Box level, meaning
 *      the new Box will be as wide as the old ones.
 * All Lines that are above/under the Area, are inserted into
 *        the Box as Line + Box.
 * All Lines that come before/after the Area, are inserted into
 *          the Boxes Left/Right.
 *
 * >> MERGE <<
 */
static void lcl_CpyLines( sal_uInt16 nStt, sal_uInt16 nEnd,
                                SwTableLines& rLines,
                                SwTableBox* pInsBox,
                                sal_uInt16 nPos = USHRT_MAX )
{
    for( sal_uInt16 n = nStt; n < nEnd; ++n )
        rLines[n]->SetUpper( pInsBox );
    if( USHRT_MAX == nPos )
        nPos = pInsBox->GetTabLines().size();
    pInsBox->GetTabLines().insert( pInsBox->GetTabLines().begin() + nPos,
                             rLines.begin() + nStt, rLines.begin() + nEnd );
    rLines.erase( rLines.begin() + nStt, rLines.begin() + nEnd );
}

static void lcl_CpyBoxes( sal_uInt16 nStt, sal_uInt16 nEnd,
                                SwTableBoxes& rBoxes,
                                SwTableLine* pInsLine )
{
    for( sal_uInt16 n = nStt; n < nEnd; ++n )
        rBoxes[n]->SetUpper( pInsLine );
    sal_uInt16 nPos = pInsLine->GetTabBoxes().size();
    pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin() + nPos,
                              rBoxes.begin() + nStt, rBoxes.begin() + nEnd );
    rBoxes.erase( rBoxes.begin() + nStt, rBoxes.begin() + nEnd );
}

static void lcl_CalcWidth( SwTableBox* pBox )
{
    // Assertion: Every Line in the Box is as large
    SwFrameFormat* pFormat = pBox->ClaimFrameFormat();
    OSL_ENSURE( pBox->GetTabLines().size(), "Box does not have any Lines" );

    SwTableLine* pLine = pBox->GetTabLines()[0];
    OSL_ENSURE( pLine, "Box is not within a Line" );

    tools::Long nWidth = 0;
    for( auto pTabBox : pLine->GetTabBoxes() )
        nWidth += pTabBox->GetFrameFormat()->GetFrameSize().GetWidth();

    pFormat->SetFormatAttr( SwFormatFrameSize( SwFrameSize::Variable, nWidth, 0 ));

    // Boxes with Lines can only have Size/Fillorder
    pFormat->ResetFormatAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
    pFormat->ResetFormatAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
}

namespace {

struct InsULPara
{
    SwTableNode* pTableNd;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    bool bUL_LR : 1;        // Upper-Lower(true) or Left-Right(false) ?
    bool bUL : 1;           // Upper-Left(true) or Lower-Right(false) ?

    SwTableBox* pLeftBox;

    InsULPara( SwTableNode* pTNd,
                SwTableBox* pLeft,
                SwTableLine* pLine )
        : pTableNd( pTNd ), pInsLine( pLine ), pInsBox( nullptr ),
        pLeftBox( pLeft )
        {   bUL_LR = true; bUL = true; }

    void SetLeft( SwTableBox* pBox )
        { bUL_LR = false;   bUL = true; if( pBox ) pInsBox = pBox; }
    void SetRight( SwTableBox* pBox )
        { bUL_LR = false;   bUL = false; if( pBox ) pInsBox = pBox; }
    void SetLower( SwTableLine* pLine )
        { bUL_LR = true;    bUL = false; if( pLine ) pInsLine = pLine; }
};

}

static void lcl_Merge_MoveLine(FndLine_ & rFndLine, InsULPara *const pULPara);

static void lcl_Merge_MoveBox(FndBox_ & rFndBox, InsULPara *const pULPara)
{
    SwTableBoxes* pBoxes;

    sal_uInt16 nStt = 0, nEnd = rFndBox.GetLines().size();
    sal_uInt16 nInsPos = USHRT_MAX;
    if( !pULPara->bUL_LR )  // Left/Right
    {
        sal_uInt16 nPos;
        SwTableBox* pFndTableBox = rFndBox.GetBox();
        pBoxes = &pFndTableBox->GetUpper()->GetTabBoxes();
        if( pULPara->bUL )  // Left ?
        {
            // if there are Boxes before it, move them
            nPos = pFndTableBox->GetUpper()->GetBoxPos( pFndTableBox );
            if( 0 != nPos )
                lcl_CpyBoxes( 0, nPos, *pBoxes, pULPara->pInsLine );
        }
        else                // Right
        {
            // if there are Boxes behind it, move them
            nPos = pFndTableBox->GetUpper()->GetBoxPos( pFndTableBox );
            if( nPos +1 < static_cast<sal_uInt16>(pBoxes->size()) )
            {
                nInsPos = pULPara->pInsLine->GetTabBoxes().size();
                lcl_CpyBoxes( nPos+1, pBoxes->size(),
                                    *pBoxes, pULPara->pInsLine );
            }
        }
    }
    // Upper/Lower and still deeper?
    else if (!rFndBox.GetLines().empty())
    {
        // Only search the Line from which we need to move
        nStt = pULPara->bUL ? 0 : rFndBox.GetLines().size()-1;
        nEnd = nStt+1;
    }

    pBoxes = &pULPara->pInsLine->GetTabBoxes();

    // Is there still a level to step down to?
    if (rFndBox.GetBox()->GetTabLines().empty())
        return;

    SwTableBox* pBox = new SwTableBox(
            static_cast<SwTableBoxFormat*>(rFndBox.GetBox()->GetFrameFormat()),
            0, pULPara->pInsLine );
    InsULPara aPara( *pULPara );
    aPara.pInsBox = pBox;
    for (FndLines_t::iterator it = rFndBox.GetLines().begin() + nStt;
         it != rFndBox.GetLines().begin() + nEnd; ++it )
    {
        lcl_Merge_MoveLine(**it, &aPara);
    }
    if( !pBox->GetTabLines().empty() )
    {
        if( USHRT_MAX == nInsPos )
            nInsPos = pBoxes->size();
        pBoxes->insert( pBoxes->begin() + nInsPos, pBox );
        lcl_CalcWidth( pBox );      // calculate the Box's width
    }
    else
        delete pBox;
}

static void lcl_Merge_MoveLine(FndLine_& rFndLine, InsULPara *const pULPara)
{
    SwTableLines* pLines;

    sal_uInt16 nStt = 0, nEnd = rFndLine.GetBoxes().size();
    sal_uInt16 nInsPos = USHRT_MAX;
    if( pULPara->bUL_LR )   // UpperLower ?
    {
        sal_uInt16 nPos;
        SwTableLine* pFndLn = rFndLine.GetLine();
        pLines = pFndLn->GetUpper() ?
                        &pFndLn->GetUpper()->GetTabLines() :
                        &pULPara->pTableNd->GetTable().GetTabLines();

        SwTableBox* pLBx = rFndLine.GetBoxes().front()->GetBox();
        SwTableBox* pRBx = rFndLine.GetBoxes().back()->GetBox();
        sal_uInt16 nLeft = pFndLn->GetBoxPos( pLBx );
        sal_uInt16 nRight = pFndLn->GetBoxPos( pRBx );

        if( !nLeft || nRight == pFndLn->GetTabBoxes().size() )
        {
            if( pULPara->bUL )  // Upper ?
            {
                // If there are Lines before it, move them
                nPos = pLines->GetPos( pFndLn );
                if( 0 != nPos )
                    lcl_CpyLines( 0, nPos, *pLines, pULPara->pInsBox );
            }
            else
                // If there are Lines after it, move them
                if( (nPos = pLines->GetPos( pFndLn )) + 1 < static_cast<sal_uInt16>(pLines->size()) )
                {
                    nInsPos = pULPara->pInsBox->GetTabLines().size();
                    lcl_CpyLines( nPos+1, pLines->size(), *pLines,
                                        pULPara->pInsBox );
                }
        }
        else
        {
            // There are still Boxes on the left side, so put the Left-
            // and Merge-Box into one Box and Line, insert before/after
            // a Line with a Box, into which the upper/lower Lines are
            // inserted
            SwTableLine* pInsLine = pULPara->pLeftBox->GetUpper();
            SwTableBox* pLMBox = new SwTableBox(
                static_cast<SwTableBoxFormat*>(pULPara->pLeftBox->GetFrameFormat()), 0, pInsLine );
            SwTableLine* pLMLn = new SwTableLine(
                        static_cast<SwTableLineFormat*>(pInsLine->GetFrameFormat()), 2, pLMBox );
            pLMLn->ClaimFrameFormat()->ResetFormatAttr( RES_FRM_SIZE );

            pLMBox->GetTabLines().insert( pLMBox->GetTabLines().begin(), pLMLn );

            lcl_CpyBoxes( 0, 2, pInsLine->GetTabBoxes(), pLMLn );

            pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin(), pLMBox );

            if( pULPara->bUL )  // Upper ?
            {
                // If there are Lines before it, move them
                nPos = pLines->GetPos( pFndLn );
                if( 0 != nPos )
                    lcl_CpyLines( 0, nPos, *pLines, pLMBox, 0 );
            }
            else
                // If there are Lines after it, move them
                if( (nPos = pLines->GetPos( pFndLn )) + 1 < static_cast<sal_uInt16>(pLines->size()) )
                    lcl_CpyLines( nPos+1, pLines->size(), *pLines,
                                        pLMBox );
            lcl_CalcWidth( pLMBox );        // calculate the Box's width
        }
    }
    // Left/Right
    else
    {
        // Find only the Line from which we need to move
        nStt = pULPara->bUL ? 0 : rFndLine.GetBoxes().size()-1;
        nEnd = nStt+1;
    }
    pLines = &pULPara->pInsBox->GetTabLines();

    SwTableLine* pNewLine = new SwTableLine(
        static_cast<SwTableLineFormat*>(rFndLine.GetLine()->GetFrameFormat()), 0, pULPara->pInsBox );
    InsULPara aPara( *pULPara );       // copying
    aPara.pInsLine = pNewLine;
    FndBoxes_t & rLineBoxes = rFndLine.GetBoxes();
    for (FndBoxes_t::iterator it = rLineBoxes.begin() + nStt;
         it != rLineBoxes.begin() + nEnd; ++it)
    {
        lcl_Merge_MoveBox(**it, &aPara);
    }

    if( !pNewLine->GetTabBoxes().empty() )
    {
        if( USHRT_MAX == nInsPos )
            nInsPos = pLines->size();
        pLines->insert( pLines->begin() + nInsPos, pNewLine );
    }
    else
        delete pNewLine;
}

static void lcl_BoxSetHeadCondColl( const SwTableBox* pBox );

bool SwTable::OldMerge( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                        SwTableBox* pMergeBox, SwUndoTableMerge* pUndo )
{
    OSL_ENSURE( !rBoxes.empty() && pMergeBox, "no valid values" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    // Find all Boxes/Lines
    FndBox_ aFndBox( nullptr, nullptr );
    {
        FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    // TL_CHART2: splitting/merging of a number of cells or rows will usually make
    // the table too complex to be handled with chart.
    // Thus we tell the charts to use their own data provider and forget about this table
    pDoc->getIDocumentChartDataProviderAccess().CreateChartInternalDataProviders( this );

    SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());    // Delete HTML Layout

    if( pUndo )
        pUndo->SetSelBoxes( rBoxes );

    // Find Lines for the Layout update
    aFndBox.SetTableLines( *this );
    aFndBox.DelFrames( *this );

    FndBox_* pFndBox = &aFndBox;
    while( 1 == pFndBox->GetLines().size() &&
            1 == pFndBox->GetLines().front()->GetBoxes().size() )
    {
        pFndBox = pFndBox->GetLines().front()->GetBoxes().front().get();
    }

    SwTableLine* pInsLine = new SwTableLine(
                static_cast<SwTableLineFormat*>(pFndBox->GetLines().front()->GetLine()->GetFrameFormat()), 0,
                !pFndBox->GetUpper() ? nullptr : pFndBox->GetBox() );
    pInsLine->ClaimFrameFormat()->ResetFormatAttr( RES_FRM_SIZE );

    // Add the new Line
    SwTableLines* pLines =  pFndBox->GetUpper() ?
                  &pFndBox->GetBox()->GetTabLines() :  &GetTabLines();

    SwTableLine* pNewLine = pFndBox->GetLines().front()->GetLine();
    sal_uInt16 nInsPos = pLines->GetPos( pNewLine );
    pLines->insert( pLines->begin() + nInsPos, pInsLine );

    SwTableBox* pLeftBox = new SwTableBox( static_cast<SwTableBoxFormat*>(pMergeBox->GetFrameFormat()), 0, pInsLine );
    SwTableBox* pRightBox = new SwTableBox( static_cast<SwTableBoxFormat*>(pMergeBox->GetFrameFormat()), 0, pInsLine );
    pMergeBox->SetUpper( pInsLine );
    pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin(), pLeftBox );
    pLeftBox->ClaimFrameFormat();
    pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin() + 1, pMergeBox);
    pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin() + 2, pRightBox );
    pRightBox->ClaimFrameFormat();

    // This contains all Lines that are above the selected Area,
    // thus they form a Upper/Lower Line
    InsULPara aPara( pTableNd, pLeftBox, pInsLine );

    // Move the overlapping upper/lower Lines of the selected Area
    for (auto & it : pFndBox->GetLines().front()->GetBoxes())
    {
        lcl_Merge_MoveBox(*it, &aPara);
    }
    aPara.SetLower( pInsLine );
    const auto nEnd = pFndBox->GetLines().size()-1;
    for (auto & it : pFndBox->GetLines()[nEnd]->GetBoxes())
    {
        lcl_Merge_MoveBox(*it, &aPara);
    }

    // Move the Boxes extending into the selected Area from left/right
    aPara.SetLeft( pLeftBox );
    for (auto & rpFndLine : pFndBox->GetLines())
    {
        lcl_Merge_MoveLine( *rpFndLine, &aPara );
    }

    aPara.SetRight( pRightBox );
    for (auto & rpFndLine : pFndBox->GetLines())
    {
        lcl_Merge_MoveLine( *rpFndLine, &aPara );
    }

    if( pLeftBox->GetTabLines().empty() )
        DeleteBox_( *this, pLeftBox, nullptr, false, false );
    else
    {
        lcl_CalcWidth( pLeftBox );      // calculate the Box's width
        if( pUndo && pLeftBox->GetSttNd() )
            pUndo->AddNewBox( pLeftBox->GetSttIdx() );
    }
    if( pRightBox->GetTabLines().empty() )
        DeleteBox_( *this, pRightBox, nullptr, false, false );
    else
    {
        lcl_CalcWidth( pRightBox );     // calculate the Box's width
        if( pUndo && pRightBox->GetSttNd() )
            pUndo->AddNewBox( pRightBox->GetSttIdx() );
    }

    DeleteSel( pDoc, rBoxes, nullptr, nullptr, false, false );

    // Clean up this Line's structure once again, generally all of them
    GCLines();

    for( const auto& rpBox : GetTabLines()[0]->GetTabBoxes() )
        lcl_BoxSetHeadCondColl(rpBox);

    aFndBox.MakeFrames( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT

    return true;
}

static void lcl_CheckRowSpan( SwTable &rTable )
{
    const tools::Long nLineCount = static_cast<tools::Long>(rTable.GetTabLines().size());
    tools::Long nMaxSpan = nLineCount;
    tools::Long nMinSpan = 1;
    while( nMaxSpan )
    {
        SwTableLine* pLine = rTable.GetTabLines()[ nLineCount - nMaxSpan ];
        for( auto pBox : pLine->GetTabBoxes() )
        {
            sal_Int32 nRowSpan = pBox->getRowSpan();
            if( nRowSpan > nMaxSpan )
                pBox->setRowSpan( nMaxSpan );
            else if( nRowSpan < nMinSpan )
                pBox->setRowSpan( nMinSpan > 0 ? nMaxSpan : nMinSpan );
        }
        --nMaxSpan;
        nMinSpan = -nMaxSpan;
    }
}

static sal_uInt16 lcl_GetBoxOffset( const FndBox_& rBox )
{
    // Find the first Box
    const FndBox_* pFirstBox = &rBox;
    while (!pFirstBox->GetLines().empty())
    {
        pFirstBox = pFirstBox->GetLines().front()->GetBoxes().front().get();
    }

    sal_uInt16 nRet = 0;
    // Calculate the position relative to above via the Lines
    const SwTableBox* pBox = pFirstBox->GetBox();
    do {
        const SwTableBoxes& rBoxes = pBox->GetUpper()->GetTabBoxes();
        for( auto pCmp : rBoxes )
        {
            if (pBox==pCmp)
                break;
            nRet = nRet + static_cast<sal_uInt16>(pCmp->GetFrameFormat()->GetFrameSize().GetWidth());
        }
        pBox = pBox->GetUpper()->GetUpper();
    } while( pBox );
    return nRet;
}

static sal_uInt16 lcl_GetLineWidth( const FndLine_& rLine )
{
    sal_uInt16 nRet = 0;
    for( auto n = rLine.GetBoxes().size(); n; )
    {
        nRet = nRet + static_cast<sal_uInt16>(rLine.GetBoxes()[--n]->GetBox()
                            ->GetFrameFormat()->GetFrameSize().GetWidth());
    }
    return nRet;
}

static void lcl_CalcNewWidths(const FndLines_t& rFndLines, CpyPara& rPara)
{
    rPara.pWidths.reset();
    const size_t nLineCount = rFndLines.size();
    if( nLineCount )
    {
        rPara.pWidths = std::make_shared< std::vector< std::vector< sal_uLong > > >
                        ( nLineCount );
        // First we collect information about the left/right borders of all
        // selected cells
        for( size_t nLine = 0; nLine < nLineCount; ++nLine )
        {
            std::vector< sal_uLong > &rWidth = (*rPara.pWidths)[ nLine ];
            const FndLine_ *pFndLine = rFndLines[ nLine ].get();
            if( pFndLine && !pFndLine->GetBoxes().empty() )
            {
                const SwTableLine *pLine = pFndLine->GetLine();
                if( pLine && !pLine->GetTabBoxes().empty() )
                {
                    size_t nBoxCount = pLine->GetTabBoxes().size();
                    sal_uLong nPos = 0;
                    // The first selected box...
                    const SwTableBox *const pSel =
                        pFndLine->GetBoxes().front()->GetBox();
                    size_t nBox = 0;
                    // Sum up the width of all boxes before the first selected box
                    while( nBox < nBoxCount )
                    {
                        SwTableBox* pBox = pLine->GetTabBoxes()[nBox++];
                        if( pBox != pSel )
                            nPos += pBox->GetFrameFormat()->GetFrameSize().GetWidth();
                        else
                            break;
                    }
                    // nPos is now the left border of the first selected box
                    if( rPara.nMinLeft > nPos )
                        rPara.nMinLeft = nPos;
                    nBoxCount = pFndLine->GetBoxes().size();
                    rWidth = std::vector< sal_uLong >( nBoxCount+2 );
                    rWidth[ 0 ] = nPos;
                    // Add now the widths of all selected boxes and store
                    // the positions in the vector
                    for( nBox = 0; nBox < nBoxCount; )
                    {
                        nPos += pFndLine->GetBoxes()[nBox]
                            ->GetBox()->GetFrameFormat()->GetFrameSize().GetWidth();
                        rWidth[ ++nBox ] = nPos;
                    }
                    // nPos: The right border of the last selected box
                    if( rPara.nMaxRight < nPos )
                        rPara.nMaxRight = nPos;
                    if( nPos <= rWidth[ 0 ] )
                        rWidth.clear();
                }
            }
        }
    }
    // Second step: calculate the new widths for the copied cells
    sal_uLong nSelSize = rPara.nMaxRight - rPara.nMinLeft;
    if( !nSelSize )
        return;

    for( size_t nLine = 0; nLine < nLineCount; ++nLine )
    {
        std::vector< sal_uLong > &rWidth = (*rPara.pWidths)[ nLine ];
        const size_t nCount = rWidth.size();
        if( nCount > 2 )
        {
            rWidth[ nCount - 1 ] = rPara.nMaxRight;
            sal_uLong nLastPos = 0;
            for( size_t nBox = 0; nBox < nCount; ++nBox )
            {
                sal_uInt64 nNextPos = rWidth[ nBox ];
                nNextPos -= rPara.nMinLeft;
                nNextPos *= rPara.nNewSize;
                nNextPos /= nSelSize;
                rWidth[ nBox ] = static_cast<sal_uLong>(nNextPos - nLastPos);
                nLastPos = static_cast<sal_uLong>(nNextPos);
            }
        }
    }
}

static void
lcl_CopyLineToDoc(FndLine_ const& rpFndLn, CpyPara *const pCpyPara);

static void lcl_CopyBoxToDoc(FndBox_ const& rFndBox, CpyPara *const pCpyPara)
{
    // Calculation of new size
    sal_uLong nRealSize;
    sal_uLong nDummy1 = 0;
    sal_uLong nDummy2 = 0;
    if( pCpyPara->pTableNd->GetTable().IsNewModel() )
    {
        if( pCpyPara->nBoxIdx == 1 )
            nDummy1 = (*pCpyPara->pWidths)[pCpyPara->nLnIdx][0];
        nRealSize = (*pCpyPara->pWidths)[pCpyPara->nLnIdx][pCpyPara->nBoxIdx++];
        if( pCpyPara->nBoxIdx == (*pCpyPara->pWidths)[pCpyPara->nLnIdx].size()-1 )
            nDummy2 = (*pCpyPara->pWidths)[pCpyPara->nLnIdx][pCpyPara->nBoxIdx];
    }
    else
    {
        nRealSize = pCpyPara->nNewSize;
        nRealSize *= rFndBox.GetBox()->GetFrameFormat()->GetFrameSize().GetWidth();
        if (pCpyPara->nOldSize == 0)
            throw o3tl::divide_by_zero();
        nRealSize /= pCpyPara->nOldSize;
    }

    sal_uLong nSize;
    bool bDummy = nDummy1 > 0;
    if( bDummy )
        nSize = nDummy1;
    else
    {
        nSize = nRealSize;
        nRealSize = 0;
    }
    do
    {
        // Find the Frame Format in the list of all Frame Formats
        CpyTabFrame aFindFrame(static_cast<SwTableBoxFormat*>(rFndBox.GetBox()->GetFrameFormat()));

        std::shared_ptr<SwFormatFrameSize> aFrameSz(std::make_shared<SwFormatFrameSize>());
        CpyTabFrames::const_iterator itFind = pCpyPara->rTabFrameArr.lower_bound( aFindFrame );
        const CpyTabFrames::size_type nFndPos = itFind - pCpyPara->rTabFrameArr.begin();

        // It *is* sometimes cool to have multiple tests/if's and assignments
        // in a single statement, and it is technically possible. But it is definitely
        // not simply readable - where from my POV reading code is done 1000 times
        // more often than writing it. Thus I dismantled the expression in smaller
        // chunks to keep it handy/understandable/changeable (hopefully without error)
        // The original for reference:
        // if( itFind == pCpyPara->rTabFrameArr.end() || !(*itFind == aFindFrame) ||
        //     ( aFrameSz = ( aFindFrame = pCpyPara->rTabFrameArr[ nFndPos ]).pNewFrameFormat->
        //         GetFrameSize()).GetWidth() != static_cast<SwTwips>(nSize) )

        bool DoCopyIt(itFind == pCpyPara->rTabFrameArr.end());

        if(!DoCopyIt)
        {
            DoCopyIt = !(*itFind == aFindFrame);
        }

        if(!DoCopyIt)
        {
            aFindFrame = pCpyPara->rTabFrameArr[ nFndPos ];
            aFrameSz.reset(aFindFrame.pNewFrameFormat->GetFrameSize().Clone());
            DoCopyIt = aFrameSz->GetWidth() != static_cast<SwTwips>(nSize);
        }

        if(DoCopyIt)
        {
            // It doesn't exist yet, so copy it
            aFindFrame.pNewFrameFormat = pCpyPara->rDoc.MakeTableBoxFormat();
            aFindFrame.pNewFrameFormat->CopyAttrs( *rFndBox.GetBox()->GetFrameFormat() );
            if( !pCpyPara->bCpyContent )
                aFindFrame.pNewFrameFormat->ResetFormatAttr(  RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
            aFrameSz->SetWidth( nSize );
            aFindFrame.pNewFrameFormat->SetFormatAttr( *aFrameSz );
            pCpyPara->rTabFrameArr.insert( aFindFrame );
        }

        SwTableBox* pBox;
        if (!rFndBox.GetLines().empty())
        {
            pBox = new SwTableBox( aFindFrame.pNewFrameFormat,
                        rFndBox.GetLines().size(), pCpyPara->pInsLine );
            pCpyPara->pInsLine->GetTabBoxes().insert( pCpyPara->pInsLine->GetTabBoxes().begin() + pCpyPara->nInsPos++, pBox );
            CpyPara aPara( *pCpyPara, pBox );
            aPara.nNewSize = nSize;     // get the size
            for (auto const& rpFndLine : rFndBox.GetLines())
            {
                lcl_CopyLineToDoc( *rpFndLine, &aPara );
            }
        }
        else
        {
            // Create an empty Box
            pCpyPara->rDoc.GetNodes().InsBoxen( pCpyPara->pTableNd, pCpyPara->pInsLine,
                            aFindFrame.pNewFrameFormat,
                            pCpyPara->rDoc.GetDfltTextFormatColl(),
                            nullptr, pCpyPara->nInsPos );
            pBox = pCpyPara->pInsLine->GetTabBoxes()[ pCpyPara->nInsPos ];
            if( bDummy )
                pBox->setDummyFlag( true );
            else if( pCpyPara->bCpyContent )
            {
                // Copy the content into this empty Box
                pBox->setRowSpan(rFndBox.GetBox()->getRowSpan());

                // We can also copy formulas and values, if we copy the content
                {
                    SfxItemSet aBoxAttrSet( pCpyPara->rDoc.GetAttrPool(),
                                            svl::Items<RES_BOXATR_FORMAT, RES_BOXATR_VALUE>{} );
                    aBoxAttrSet.Put(rFndBox.GetBox()->GetFrameFormat()->GetAttrSet());
                    if( aBoxAttrSet.Count() )
                    {
                        const SfxPoolItem* pItem;
                        SvNumberFormatter* pN = pCpyPara->rDoc.GetNumberFormatter( false );
                        if( pN && pN->HasMergeFormatTable() && SfxItemState::SET == aBoxAttrSet.
                            GetItemState( RES_BOXATR_FORMAT, false, &pItem ) )
                        {
                            sal_uLong nOldIdx = static_cast<const SwTableBoxNumFormat*>(pItem)->GetValue();
                            sal_uLong nNewIdx = pN->GetMergeFormatIndex( nOldIdx );
                            if( nNewIdx != nOldIdx )
                                aBoxAttrSet.Put( SwTableBoxNumFormat( nNewIdx ));
                        }
                        pBox->ClaimFrameFormat()->SetFormatAttr( aBoxAttrSet );
                    }
                }
                SwDoc* pFromDoc = rFndBox.GetBox()->GetFrameFormat()->GetDoc();
                SwNodeRange aCpyRg( *rFndBox.GetBox()->GetSttNd(), 1,
                        *rFndBox.GetBox()->GetSttNd()->EndOfSectionNode() );
                SwNodeIndex aInsIdx( *pBox->GetSttNd(), 1 );

                pFromDoc->GetDocumentContentOperationsManager().CopyWithFlyInFly(aCpyRg, aInsIdx, nullptr, false);
                // Delete the initial TextNode
                pCpyPara->rDoc.GetNodes().Delete( aInsIdx );
            }
            ++pCpyPara->nInsPos;
        }
        if( nRealSize )
        {
            bDummy = false;
            nSize = nRealSize;
            nRealSize = 0;
        }
        else
        {
            bDummy = true;
            nSize = nDummy2;
            nDummy2 = 0;
        }
    }
    while( nSize );
}

static void
lcl_CopyLineToDoc(const FndLine_& rFndLine, CpyPara *const pCpyPara)
{
    // Find the Frame Format in the list of all Frame Formats
    CpyTabFrame aFindFrame( rFndLine.GetLine()->GetFrameFormat() );
    CpyTabFrames::const_iterator itFind = pCpyPara->rTabFrameArr.find( aFindFrame );
    if( itFind == pCpyPara->rTabFrameArr.end() )
    {
        // It doesn't exist yet, so copy it
        aFindFrame.pNewFrameFormat = reinterpret_cast<SwTableBoxFormat*>(pCpyPara->rDoc.MakeTableLineFormat());
        aFindFrame.pNewFrameFormat->CopyAttrs( *rFndLine.GetLine()->GetFrameFormat() );
        pCpyPara->rTabFrameArr.insert( aFindFrame );
    }
    else
        aFindFrame = *itFind;

    SwTableLine* pNewLine = new SwTableLine( reinterpret_cast<SwTableLineFormat*>(aFindFrame.pNewFrameFormat),
                        rFndLine.GetBoxes().size(), pCpyPara->pInsBox );
    if( pCpyPara->pInsBox )
    {
        SwTableLines& rLines = pCpyPara->pInsBox->GetTabLines();
        rLines.insert( rLines.begin() + pCpyPara->nInsPos++, pNewLine );
    }
    else
    {
        SwTableLines& rLines = pCpyPara->pTableNd->GetTable().GetTabLines();
        rLines.insert( rLines.begin() + pCpyPara->nInsPos++, pNewLine);
    }

    CpyPara aPara( *pCpyPara, pNewLine );

    if( pCpyPara->pTableNd->GetTable().IsNewModel() )
    {
        aPara.nOldSize = 0; // will not be used
        aPara.nBoxIdx = 1;
    }
    else if( rFndLine.GetBoxes().size() ==
                    rFndLine.GetLine()->GetTabBoxes().size() )
    {
        // Get the Parent's size
        const SwFrameFormat* pFormat;

        if( rFndLine.GetLine()->GetUpper() )
            pFormat = rFndLine.GetLine()->GetUpper()->GetFrameFormat();
        else
            pFormat = pCpyPara->pTableNd->GetTable().GetFrameFormat();
        aPara.nOldSize = pFormat->GetFrameSize().GetWidth();
    }
    else
        // Calculate it
        for (auto &rpBox : rFndLine.GetBoxes())
        {
            aPara.nOldSize += rpBox->GetBox()->GetFrameFormat()->GetFrameSize().GetWidth();
        }

    const FndBoxes_t& rBoxes = rFndLine.GetBoxes();
    for (auto const& it : rBoxes)
    {
        lcl_CopyBoxToDoc(*it, &aPara);
    }
    if( pCpyPara->pTableNd->GetTable().IsNewModel() )
        ++pCpyPara->nLnIdx;
}

void SwTable::CopyHeadlineIntoTable( SwTableNode& rTableNd )
{
    // Find all Boxes/Lines
    SwSelBoxes aSelBoxes;
    SwTableBox* pBox = GetTabSortBoxes()[ 0 ];
    pBox = GetTableBox( pBox->GetSttNd()->StartOfSectionNode()->GetIndex() + 1 );
    SelLineFromBox( pBox, aSelBoxes );

    FndBox_ aFndBox( nullptr, nullptr );
    {
        FndPara aPara( aSelBoxes, &aFndBox );
        ForEach_FndLineCopyCol( GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return;

    {
        // Convert Table formulas to their relative representation
        SwTableFormulaUpdate aMsgHint( this );
        aMsgHint.m_eFlags = TBL_RELBOXNAME;
        GetFrameFormat()->GetDoc()->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
    }

    CpyTabFrames aCpyFormat;
    CpyPara aPara( &rTableNd, 1, aCpyFormat );
    aPara.nNewSize = aPara.nOldSize = rTableNd.GetTable().GetFrameFormat()->GetFrameSize().GetWidth();
    // Copy
    if( IsNewModel() )
        lcl_CalcNewWidths( aFndBox.GetLines(), aPara );
    for (const auto & rpFndLine : aFndBox.GetLines())
    {
         lcl_CopyLineToDoc( *rpFndLine, &aPara );
    }
    if( rTableNd.GetTable().IsNewModel() )
    {   // The copied line must not contain any row span attributes > 1
        SwTableLine* pLine = rTableNd.GetTable().GetTabLines()[0];
        OSL_ENSURE( !pLine->GetTabBoxes().empty(), "Empty Table Line" );
        for( auto pTableBox : pLine->GetTabBoxes() )
        {
            OSL_ENSURE( pTableBox, "Missing Table Box" );
            pTableBox->setRowSpan( 1 );
        }
    }
}

bool SwTable::MakeCopy( SwDoc& rInsDoc, const SwPosition& rPos,
                        const SwSelBoxes& rSelBoxes,
                        bool bCpyName ) const
{
    // Find all Boxes/Lines
    FndBox_ aFndBox( nullptr, nullptr );
    {
        FndPara aPara( rSelBoxes, &aFndBox );
        ForEach_FndLineCopyCol( const_cast<SwTableLines&>(GetTabLines()), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    // First copy the PoolTemplates for the Table, so that the Tables are
    // actually copied and have valid values.
    SwDoc* pSrcDoc = GetFrameFormat()->GetDoc();
    if( pSrcDoc != &rInsDoc )
    {
        rInsDoc.CopyTextColl( *pSrcDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TABLE ) );
        rInsDoc.CopyTextColl( *pSrcDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TABLE_HDLN ) );
    }

    SwTable* pNewTable = const_cast<SwTable*>(rInsDoc.InsertTable(
            SwInsertTableOptions( SwInsertTableFlags::HeadlineNoBorder, 1 ),
            rPos, 1, 1, GetFrameFormat()->GetHoriOrient().GetHoriOrient(),
            nullptr, nullptr, false, IsNewModel() ));
    if( !pNewTable )
        return false;

    SwNodeIndex aIdx( rPos.nNode, -1 );
    SwTableNode* pTableNd = aIdx.GetNode().FindTableNode();
    ++aIdx;
    OSL_ENSURE( pTableNd, "Where is the TableNode now?" );

    pTableNd->GetTable().SetRowsToRepeat( GetRowsToRepeat() );

    pNewTable->SetTableStyleName(pTableNd->GetTable().GetTableStyleName());

    if( auto pSwDDETable = dynamic_cast<const SwDDETable*>(this) )
    {
        // A DDE-Table is being copied
        // Does the new Document actually have it's FieldType?
        SwFieldType* pFieldType = rInsDoc.getIDocumentFieldsAccess().InsertFieldType(
                                    *pSwDDETable->GetDDEFieldType() );
        OSL_ENSURE( pFieldType, "unknown FieldType" );

        // Change the Table Pointer at the Node
        pNewTable = new SwDDETable( *pNewTable,
                                 static_cast<SwDDEFieldType*>(pFieldType) );
        pTableNd->SetNewTable( std::unique_ptr<SwTable>(pNewTable), false );
    }

    pNewTable->GetFrameFormat()->CopyAttrs( *GetFrameFormat() );
    pNewTable->SetTableChgMode( GetTableChgMode() );

    // Destroy the already created Frames
    pTableNd->DelFrames();

    {
        // Convert the Table formulas to their relative representation
        SwTableFormulaUpdate aMsgHint( this );
        aMsgHint.m_eFlags = TBL_RELBOXNAME;
        pSrcDoc->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
    }

    SwTableNumFormatMerge aTNFM(*pSrcDoc, rInsDoc);

    // Also copy Names or enforce a new unique one
    if( bCpyName )
        pNewTable->GetFrameFormat()->SetName( GetFrameFormat()->GetName() );

    CpyTabFrames aCpyFormat;
    CpyPara aPara( pTableNd, 1, aCpyFormat );
    aPara.nNewSize = aPara.nOldSize = GetFrameFormat()->GetFrameSize().GetWidth();

    if( IsNewModel() )
        lcl_CalcNewWidths( aFndBox.GetLines(), aPara );
    // Copy
    for (const auto & rpFndLine : aFndBox.GetLines())
    {
         lcl_CopyLineToDoc( *rpFndLine, &aPara );
    }

    // Set the "right" margin above/below
    {
        FndLine_* pFndLn = aFndBox.GetLines().front().get();
        SwTableLine* pLn = pFndLn->GetLine();
        const SwTableLine* pTmp = pLn;
        sal_uInt16 nLnPos = GetTabLines().GetPos( pTmp );
        if( USHRT_MAX != nLnPos && nLnPos )
        {
            // There is a Line before it
            SwCollectTableLineBoxes aLnPara( false, SplitTable_HeadlineOption::BorderCopy );

            pLn = GetTabLines()[ nLnPos - 1 ];
            for( const auto& rpBox : pLn->GetTabBoxes() )
                sw_Box_CollectBox( rpBox, &aLnPara );

            if( aLnPara.Resize( lcl_GetBoxOffset( aFndBox ),
                                lcl_GetLineWidth( *pFndLn )) )
            {
                aLnPara.SetValues( true );
                pLn = pNewTable->GetTabLines()[ 0 ];
                for( const auto& rpBox : pLn->GetTabBoxes() )
                    sw_BoxSetSplitBoxFormats(rpBox, &aLnPara );
            }
        }

        pFndLn = aFndBox.GetLines().back().get();
        pLn = pFndLn->GetLine();
        pTmp = pLn;
        nLnPos = GetTabLines().GetPos( pTmp );
        if( nLnPos < GetTabLines().size() - 1 )
        {
            // There is a Line following it
            SwCollectTableLineBoxes aLnPara( true, SplitTable_HeadlineOption::BorderCopy );

            pLn = GetTabLines()[ nLnPos + 1 ];
            for( const auto& rpBox : pLn->GetTabBoxes() )
                sw_Box_CollectBox( rpBox, &aLnPara );

            if( aLnPara.Resize( lcl_GetBoxOffset( aFndBox ),
                                lcl_GetLineWidth( *pFndLn )) )
            {
                aLnPara.SetValues( false );
                pLn = pNewTable->GetTabLines().back();
                for( const auto& rpBox : pLn->GetTabBoxes() )
                    sw_BoxSetSplitBoxFormats(rpBox, &aLnPara );
            }
        }
    }

    // We need to delete the initial Box
    DeleteBox_( *pNewTable, pNewTable->GetTabLines().back()->GetTabBoxes()[0],
                nullptr, false, false );

    if( pNewTable->IsNewModel() )
        lcl_CheckRowSpan( *pNewTable );
    // Clean up
    pNewTable->GCLines();

    pTableNd->MakeOwnFrames( &aIdx );  // re-generate the Frames

    CHECKTABLELAYOUT

    return true;
}

// Find the next Box with content from this Line
SwTableBox* SwTableLine::FindNextBox( const SwTable& rTable,
                     const SwTableBox* pSrchBox, bool bOvrTableLns ) const
{
    const SwTableLine* pLine = this;            // for M800
    SwTableBox* pBox;
    sal_uInt16 nFndPos;
    if( !GetTabBoxes().empty() && pSrchBox )
    {
        nFndPos = GetBoxPos( pSrchBox );
        if( USHRT_MAX != nFndPos &&
            nFndPos + 1 != static_cast<sal_uInt16>(GetTabBoxes().size()) )
        {
            pBox = GetTabBoxes()[ nFndPos + 1 ];
            while( !pBox->GetTabLines().empty() )
                pBox = pBox->GetTabLines().front()->GetTabBoxes()[0];
            return pBox;
        }
    }

    if( GetUpper() )
    {
        nFndPos = GetUpper()->GetTabLines().GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nFndPos, "Line is not in the Table" );
        // Is there another Line?
        if( nFndPos+1 >= static_cast<sal_uInt16>(GetUpper()->GetTabLines().size()) )
            return GetUpper()->GetUpper()->FindNextBox( rTable, GetUpper(), bOvrTableLns );
        pLine = GetUpper()->GetTabLines()[nFndPos+1];
    }
    else if( bOvrTableLns )       // Over a Table's the "BaseLines"??
    {
        // Search for the next Line in the Table
        nFndPos = rTable.GetTabLines().GetPos( pLine );
        if( nFndPos + 1 >= static_cast<sal_uInt16>(rTable.GetTabLines().size()) )
            return nullptr;           // there are no more Boxes

        pLine = rTable.GetTabLines()[ nFndPos+1 ];
    }
    else
        return nullptr;

    if( !pLine->GetTabBoxes().empty() )
    {
        pBox = pLine->GetTabBoxes().front();
        while( !pBox->GetTabLines().empty() )
            pBox = pBox->GetTabLines().front()->GetTabBoxes().front();
        return pBox;
    }
    return pLine->FindNextBox( rTable, nullptr, bOvrTableLns );
}

// Find the previous Box from this Line
SwTableBox* SwTableLine::FindPreviousBox( const SwTable& rTable,
                         const SwTableBox* pSrchBox, bool bOvrTableLns ) const
{
    const SwTableLine* pLine = this;            // for M800
    SwTableBox* pBox;
    sal_uInt16 nFndPos;
    if( !GetTabBoxes().empty() && pSrchBox )
    {
        nFndPos = GetBoxPos( pSrchBox );
        if( USHRT_MAX != nFndPos && nFndPos )
        {
            pBox = GetTabBoxes()[ nFndPos - 1 ];
            while( !pBox->GetTabLines().empty() )
            {
                pLine = pBox->GetTabLines().back();
                pBox = pLine->GetTabBoxes().back();
            }
            return pBox;
        }
    }

    if( GetUpper() )
    {
        nFndPos = GetUpper()->GetTabLines().GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nFndPos, "Line is not in the Table" );
        // Is there another Line?
        if( !nFndPos )
            return GetUpper()->GetUpper()->FindPreviousBox( rTable, GetUpper(), bOvrTableLns );
        pLine = GetUpper()->GetTabLines()[nFndPos-1];
    }
    else if( bOvrTableLns )       // Over a Table's the "BaseLines"??
    {
        // Search for the next Line in the Table
        nFndPos = rTable.GetTabLines().GetPos( pLine );
        if( !nFndPos )
            return nullptr;           // there are no more Boxes

        pLine = rTable.GetTabLines()[ nFndPos-1 ];
    }
    else
        return nullptr;

    if( !pLine->GetTabBoxes().empty() )
    {
        pBox = pLine->GetTabBoxes().back();
        while( !pBox->GetTabLines().empty() )
        {
            pLine = pBox->GetTabLines().back();
            pBox = pLine->GetTabBoxes().back();
        }
        return pBox;
    }
    return pLine->FindPreviousBox( rTable, nullptr, bOvrTableLns );
}

// Find the next Box with content from this Line
SwTableBox* SwTableBox::FindNextBox( const SwTable& rTable,
                         const SwTableBox* pSrchBox, bool bOvrTableLns ) const
{
    if( !pSrchBox  && GetTabLines().empty() )
        return const_cast<SwTableBox*>(this);
    return GetUpper()->FindNextBox( rTable, pSrchBox ? pSrchBox : this,
                                        bOvrTableLns );

}

// Find the next Box with content from this Line
SwTableBox* SwTableBox::FindPreviousBox( const SwTable& rTable,
                         const SwTableBox* pSrchBox ) const
{
    if( !pSrchBox && GetTabLines().empty() )
        return const_cast<SwTableBox*>(this);
    return GetUpper()->FindPreviousBox( rTable, pSrchBox ? pSrchBox : this );
}

static void lcl_BoxSetHeadCondColl( const SwTableBox* pBox )
{
    // We need to adapt the paragraphs with conditional templates in the HeadLine
    const SwStartNode* pSttNd = pBox->GetSttNd();
    if( pSttNd )
        pSttNd->CheckSectionCondColl();
    else
        for( const SwTableLine* pLine : pBox->GetTabLines() )
            sw_LineSetHeadCondColl( pLine );
}

void sw_LineSetHeadCondColl( const SwTableLine* pLine )
{
    for( const SwTableBox* pBox : pLine->GetTabBoxes() )
        lcl_BoxSetHeadCondColl(pBox);
}

static SwTwips lcl_GetDistance( SwTableBox* pBox, bool bLeft )
{
    bool bFirst = true;
    SwTwips nRet = 0;
    SwTableLine* pLine;
    while( pBox )
    {
        pLine = pBox->GetUpper();
        if( !pLine )
            break;
        sal_uInt16 nStt = 0, nPos = pLine->GetBoxPos( pBox );

        if( bFirst && !bLeft )
            ++nPos;
        bFirst = false;

        while( nStt < nPos )
            nRet += pLine->GetTabBoxes()[ nStt++ ]->GetFrameFormat()
                            ->GetFrameSize().GetWidth();
        pBox = pLine->GetUpper();
    }
    return nRet;
}

static bool lcl_SetSelBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                         SwTwips nDist, bool bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( auto pBox : rBoxes )
    {
        SwFrameFormat* pFormat = pBox->GetFrameFormat();
        const SwFormatFrameSize& rSz = pFormat->GetFrameSize();
        SwTwips nWidth = rSz.GetWidth();
        bool bGreaterBox = false;

        if( bCheck )
        {
            for( auto pLn : pBox->GetTabLines() )
                if( !::lcl_SetSelBoxWidth( pLn, rParam, nDist, true ))
                    return false;

            // Collect all "ContentBoxes"
            bGreaterBox = (TableChgMode::FixedWidthChangeAbs != rParam.nMode)
                       && ((nDist + (rParam.bLeft ? 0 : nWidth)) >= rParam.nSide);
            if (bGreaterBox
                || (!rParam.bBigger
                    && (std::abs(nDist + ((rParam.nMode != TableChgMode::FixedWidthChangeAbs && rParam.bLeft) ? 0 : nWidth) - rParam.nSide) < COLFUZZY)))
            {
                SwTwips nLowerDiff;
                if( bGreaterBox && TableChgMode::FixedWidthChangeProp == rParam.nMode )
                {
                    // The "other Boxes" have been adapted, so change by this value
                    nLowerDiff = (nDist + ( rParam.bLeft ? 0 : nWidth ) ) - rParam.nSide;
                    nLowerDiff *= rParam.nDiff;
                    nLowerDiff /= rParam.nMaxSize;
                    nLowerDiff = rParam.nDiff - nLowerDiff;
                }
                else
                    nLowerDiff = rParam.nDiff;

                if( nWidth < nLowerDiff || nWidth - nLowerDiff < MINLAY )
                    return false;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( auto pLn : pBox->GetTabLines() )
            {
                rParam.nLowerDiff = 0;
                lcl_SetSelBoxWidth( pLn, rParam, nDist, false );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff ||
                 (bGreaterBox = !nOldLower && TableChgMode::FixedWidthChangeAbs != rParam.nMode &&
                    ( nDist + ( rParam.bLeft ? 0 : nWidth ) ) >= rParam.nSide) ||
                ( std::abs( nDist + ( (rParam.nMode != TableChgMode::FixedWidthChangeAbs && rParam.bLeft) ? 0 : nWidth )
                            - rParam.nSide ) < COLFUZZY ))
            {
                // This column contains the Cursor - so decrease/increase
                SwFormatFrameSize aNew( rSz );

                if( !nLowerDiff )
                {
                    if( bGreaterBox && TableChgMode::FixedWidthChangeProp == rParam.nMode )
                    {
                        // The "other Boxes" have been adapted, so change by this value
                        nLowerDiff = (nDist + ( rParam.bLeft ? 0 : nWidth ) ) - rParam.nSide;
                        nLowerDiff *= rParam.nDiff;
                        nLowerDiff /= rParam.nMaxSize;
                        nLowerDiff = rParam.nDiff - nLowerDiff;
                    }
                    else
                        nLowerDiff = rParam.nDiff;
                }

                rParam.nLowerDiff += nLowerDiff;

                if( rParam.bBigger )
                    aNew.SetWidth( nWidth + nLowerDiff );
                else
                    aNew.SetWidth( nWidth - nLowerDiff );
                rParam.aShareFormats.SetSize( *pBox, aNew );
                break;
            }
        }

        if( rParam.bLeft && rParam.nMode != TableChgMode::FixedWidthChangeAbs && nDist >= rParam.nSide )
            break;

        nDist += nWidth;

        // If it gets bigger, then that's it
        if( ( TableChgMode::FixedWidthChangeAbs == rParam.nMode || !rParam.bLeft ) &&
                nDist >= rParam.nSide )
            break;
    }
    return true;
}

static bool lcl_SetOtherBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( auto pBox : rBoxes )
    {
        SwFrameFormat* pFormat = pBox->GetFrameFormat();
        const SwFormatFrameSize& rSz = pFormat->GetFrameSize();
        SwTwips nWidth = rSz.GetWidth();

        if( bCheck )
        {
            for( auto pLn : pBox->GetTabLines() )
                if( !::lcl_SetOtherBoxWidth( pLn, rParam, nDist, true ))
                    return false;

            if( rParam.bBigger && ( TableChgMode::FixedWidthChangeAbs == rParam.nMode
                    ? std::abs( nDist - rParam.nSide ) < COLFUZZY
                    : ( rParam.bLeft ? nDist < rParam.nSide - COLFUZZY
                                     : nDist >= rParam.nSide - COLFUZZY )) )
            {
                SwTwips nDiff;
                if( TableChgMode::FixedWidthChangeProp == rParam.nMode )        // Table fixed, proportional
                {
                    // calculate relative
                    nDiff = nWidth;
                    nDiff *= rParam.nDiff;
                    nDiff /= rParam.nMaxSize;
                }
                else
                    nDiff = rParam.nDiff;

                if( nWidth < nDiff || nWidth - nDiff < MINLAY )
                    return false;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( auto pLn : pBox->GetTabLines() )
            {
                rParam.nLowerDiff = 0;
                lcl_SetOtherBoxWidth( pLn, rParam, nDist, false );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff ||
                ( TableChgMode::FixedWidthChangeAbs == rParam.nMode
                        ? std::abs( nDist - rParam.nSide ) < COLFUZZY
                        : ( rParam.bLeft ? nDist < rParam.nSide - COLFUZZY
                                         : nDist >= rParam.nSide - COLFUZZY)
                 ) )
            {
                SwFormatFrameSize aNew( rSz );

                if( !nLowerDiff )
                {
                    if( TableChgMode::FixedWidthChangeProp == rParam.nMode )        // Table fixed, proportional
                    {
                        // calculate relative
                        nLowerDiff = nWidth;
                        nLowerDiff *= rParam.nDiff;
                        nLowerDiff /= rParam.nMaxSize;
                    }
                    else
                        nLowerDiff = rParam.nDiff;
                }

                rParam.nLowerDiff += nLowerDiff;

                if( rParam.bBigger )
                    aNew.SetWidth( nWidth - nLowerDiff );
                else
                    aNew.SetWidth( nWidth + nLowerDiff );

                rParam.aShareFormats.SetSize( *pBox, aNew );
            }
        }

        nDist += nWidth;
        if( ( TableChgMode::FixedWidthChangeAbs == rParam.nMode || rParam.bLeft ) &&
            nDist > rParam.nSide )
            break;
    }
    return true;
}

static void lcl_AjustLines( SwTableLine* pLine, CR_SetBoxWidth& rParam )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( auto pBox : rBoxes )
    {
        SwFormatFrameSize aSz( pBox->GetFrameFormat()->GetFrameSize() );
        SwTwips nWidth = aSz.GetWidth();
        nWidth *= rParam.nDiff;
        nWidth /= rParam.nMaxSize;
        aSz.SetWidth( nWidth );
        rParam.aShareFormats.SetSize( *pBox, aSz );

        for( auto pLn : pBox->GetTabLines() )
            ::lcl_AjustLines( pLn, rParam );
    }
}

#ifdef DBG_UTIL
void CheckBoxWidth( const SwTableLine& rLine, SwTwips nSize )
{
    const SwTableBoxes& rBoxes = rLine.GetTabBoxes();

    SwTwips nCurrentSize = 0;
    // See if the tables have a correct width
    for (const SwTableBox* pBox : rBoxes)
    {
        const SwTwips nBoxW = pBox->GetFrameFormat()->GetFrameSize().GetWidth();
        nCurrentSize += nBoxW;

        for( auto pLn : pBox->GetTabLines() )
            CheckBoxWidth( *pLn, nBoxW );
    }

    if (sal::static_int_cast< tools::ULong >(std::abs(nCurrentSize - nSize)) >
        (COLFUZZY * rBoxes.size()))
    {
        OSL_FAIL( "Line's Boxes are too small or too large" );
    }
}
#endif

bool SwTable::SetColWidth( SwTableBox& rCurrentBox, TableChgWidthHeightType eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, std::unique_ptr<SwUndo>* ppUndo )
{
    SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());    // Delete HTML Layout

    const SwFormatFrameSize& rSz = GetFrameFormat()->GetFrameSize();
    const SvxLRSpaceItem& rLR = GetFrameFormat()->GetLRSpace();

    std::unique_ptr<FndBox_> xFndBox;                // for insertion/deletion
    bool bBigger,
        bRet = false,
        bLeft = TableChgWidthHeightType::ColLeft == extractPosition( eType ) ||
                TableChgWidthHeightType::CellLeft == extractPosition( eType );

    // Get the current Box's edge
    // Only needed for manipulating the width
    const SwTwips nDist = ::lcl_GetDistance( &rCurrentBox, bLeft );
    SwTwips nDistStt = 0;
    CR_SetBoxWidth aParam( eType, nRelDiff, nDist,
                            bLeft ? nDist : rSz.GetWidth() - nDist,
                            const_cast<SwTableNode*>(rCurrentBox.GetSttNd()->FindTableNode()) );
    bBigger = aParam.bBigger;

    FN_lcl_SetBoxWidth fnSelBox, fnOtherBox;
    fnSelBox = lcl_SetSelBoxWidth;
    fnOtherBox = lcl_SetOtherBoxWidth;

    switch( extractPosition(eType) )
    {
    case TableChgWidthHeightType::ColRight:
    case TableChgWidthHeightType::ColLeft:
        if( TableChgMode::VarWidthChangeAbs == m_eTableChgMode )
        {
            // First test if we have room at all
            bool bChgLRSpace = true;
            if( bBigger )
            {
                if( GetFrameFormat()->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE) &&
                    !rSz.GetWidthPercent() )
                {
                    // silence -Wsign-compare on Android with the static cast
                    bRet = rSz.GetWidth() < static_cast<unsigned short>(USHRT_MAX) - nRelDiff;
                    bChgLRSpace = bLeft ? rLR.GetLeft() >= nAbsDiff
                                        : rLR.GetRight() >= nAbsDiff;
                }
                else
                    bRet = bLeft ? rLR.GetLeft() >= nAbsDiff
                                 : rLR.GetRight() >= nAbsDiff;

                if( !bRet )
                {
                    // Then call itself recursively; only with another mode (proportional)
                    TableChgMode eOld = m_eTableChgMode;
                    m_eTableChgMode = TableChgMode::FixedWidthChangeProp;

                    bRet = SetColWidth( rCurrentBox, eType, nAbsDiff, nRelDiff,
                                        ppUndo );
                    m_eTableChgMode = eOld;
                    return bRet;
                }
            }
            else
            {
                bRet = true;
                for( auto const & n: m_aLines )
                {
                    aParam.LoopClear();
                    if( !(*fnSelBox)( n, aParam, nDistStt, true ))
                    {
                        bRet = false;
                        break;
                    }
                }
            }

            if( bRet )
            {
                if( ppUndo )
                    ppUndo->reset(new SwUndoAttrTable( *aParam.pTableNd, true ));

                tools::Long nFrameWidth = LONG_MAX;
                LockModify();
                SwFormatFrameSize aSz( rSz );
                SvxLRSpaceItem aLR( rLR );
                if( bBigger )
                {
                    // If the Table does not have any room to grow, we need to create some!
                    // silence -Wsign-compare on Android with the static cast
                    if( aSz.GetWidth() + nRelDiff > static_cast<unsigned short>(USHRT_MAX) )
                    {
                        // Break down to USHRT_MAX / 2
                        CR_SetBoxWidth aTmpPara( TableChgWidthHeightType::ColLeft, aSz.GetWidth() / 2,
                                        0, aSz.GetWidth(), aParam.pTableNd );
                        for( size_t nLn = 0; nLn < m_aLines.size(); ++nLn )
                            ::lcl_AjustLines( m_aLines[ nLn ], aTmpPara );
                        aSz.SetWidth( aSz.GetWidth() / 2 );
                        aParam.nDiff = nRelDiff /= 2;
                        aParam.nSide /= 2;
                        aParam.nMaxSize /= 2;
                    }

                    if( bLeft )
                        aLR.SetLeft( sal_uInt16( aLR.GetLeft() - nAbsDiff ) );
                    else
                        aLR.SetRight( sal_uInt16( aLR.GetRight() - nAbsDiff ) );
                }
                else if( bLeft )
                    aLR.SetLeft( sal_uInt16( aLR.GetLeft() + nAbsDiff ) );
                else
                    aLR.SetRight( sal_uInt16( aLR.GetRight() + nAbsDiff ) );

                if( bChgLRSpace )
                    GetFrameFormat()->SetFormatAttr( aLR );
                const SwFormatHoriOrient& rHOri = GetFrameFormat()->GetHoriOrient();
                if( text::HoriOrientation::FULL == rHOri.GetHoriOrient() ||
                    (text::HoriOrientation::LEFT == rHOri.GetHoriOrient() && aLR.GetLeft()) ||
                    (text::HoriOrientation::RIGHT == rHOri.GetHoriOrient() && aLR.GetRight()))
                {
                    SwFormatHoriOrient aHOri( rHOri );
                    aHOri.SetHoriOrient( text::HoriOrientation::NONE );
                    GetFrameFormat()->SetFormatAttr( aHOri );

                    // If the Table happens to contain relative values (USHORT_MAX),
                    // we need to convert them to absolute ones now.
                    // Bug 61494
                    if( GetFrameFormat()->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE) &&
                        !rSz.GetWidthPercent() )
                    {
                        SwTabFrame* pTabFrame = SwIterator<SwTabFrame,SwFormat>( *GetFrameFormat() ).First();
                        if( pTabFrame &&
                            pTabFrame->getFramePrintArea().Width() != rSz.GetWidth() )
                        {
                            nFrameWidth = pTabFrame->getFramePrintArea().Width();
                            if( bBigger )
                                nFrameWidth += nAbsDiff;
                            else
                                nFrameWidth -= nAbsDiff;
                        }
                    }
                }

                if( bBigger )
                    aSz.SetWidth( aSz.GetWidth() + nRelDiff );
                else
                    aSz.SetWidth( aSz.GetWidth() - nRelDiff );

                if( rSz.GetWidthPercent() )
                    aSz.SetWidthPercent( static_cast<sal_uInt8>(( aSz.GetWidth() * 100 ) /
                        ( aSz.GetWidth() + aLR.GetRight() + aLR.GetLeft())));

                GetFrameFormat()->SetFormatAttr( aSz );

                UnlockModify();

                for( sal_uInt16 n = m_aLines.size(); n; )
                {
                    --n;
                    aParam.LoopClear();
                    (*fnSelBox)( m_aLines[ n ], aParam, nDistStt, false );
                }

                // If the Table happens to contain relative values (USHORT_MAX),
                // we need to convert them to absolute ones now.
                // Bug 61494
                if( LONG_MAX != nFrameWidth )
                {
                    SwFormatFrameSize aAbsSz( aSz );
                    aAbsSz.SetWidth( nFrameWidth );
                    GetFrameFormat()->SetFormatAttr( aAbsSz );
                }
            }
        }
        else if( bLeft ? nDist != 0 : std::abs( rSz.GetWidth() - nDist ) > COLFUZZY )
        {
            bRet = true;
            if( bLeft && TableChgMode::FixedWidthChangeAbs == m_eTableChgMode )
                aParam.bBigger = !bBigger;

            // First test if we have room at all
            if( aParam.bBigger )
            {
                for( auto const & n: m_aLines )
                {
                    aParam.LoopClear();
                    if( !(*fnOtherBox)( n, aParam, 0, true ))
                    {
                        bRet = false;
                        break;
                    }
                }
            }
            else
            {
                for( auto const & n: m_aLines )
                {
                    aParam.LoopClear();
                    if( !(*fnSelBox)( n, aParam, nDistStt, true ))
                    {
                        bRet = false;
                        break;
                    }
                }
            }

            // If true, set it
            if( bRet )
            {
                CR_SetBoxWidth aParam1( aParam );
                if( ppUndo )
                    ppUndo->reset(new SwUndoAttrTable( *aParam.pTableNd, true ));

                if( TableChgMode::FixedWidthChangeAbs != m_eTableChgMode && bLeft )
                {
                    for( sal_uInt16 n = m_aLines.size(); n; )
                    {
                        --n;
                        aParam.LoopClear();
                        aParam1.LoopClear();
                        (*fnSelBox)( m_aLines[ n ], aParam, nDistStt, false );
                        (*fnOtherBox)( m_aLines[ n ], aParam1, nDistStt, false );
                    }
                }
                else
                {
                    for( sal_uInt16 n = m_aLines.size(); n; )
                    {
                        --n;
                        aParam.LoopClear();
                        aParam1.LoopClear();
                        (*fnOtherBox)( m_aLines[ n ], aParam1, nDistStt, false );
                        (*fnSelBox)( m_aLines[ n ], aParam, nDistStt, false );
                    }
                }
            }
        }
        break;

    case TableChgWidthHeightType::CellRight:
    case TableChgWidthHeightType::CellLeft:
        if( TableChgMode::VarWidthChangeAbs == m_eTableChgMode )
        {
            // Then call itself recursively; only with another mode (proportional)
            TableChgMode eOld = m_eTableChgMode;
            m_eTableChgMode = TableChgMode::FixedWidthChangeAbs;

            bRet = SetColWidth( rCurrentBox, eType, nAbsDiff, nRelDiff,
                                ppUndo );
            m_eTableChgMode = eOld;
            return bRet;
        }
        else if( bLeft ? nDist != 0 : (rSz.GetWidth() - nDist) > COLFUZZY )
        {
            if( bLeft && TableChgMode::FixedWidthChangeAbs == m_eTableChgMode )
                aParam.bBigger = !bBigger;

            // First, see if there is enough room at all
            SwTableBox* pBox = &rCurrentBox;
            SwTableLine* pLine = rCurrentBox.GetUpper();
            while( pLine->GetUpper() )
            {
                const SwTableBoxes::size_type nPos = pLine->GetBoxPos( pBox );
                if( bLeft ? nPos != 0 : nPos + 1 != pLine->GetTabBoxes().size() )
                    break;

                pBox = pLine->GetUpper();
                pLine = pBox->GetUpper();
            }

            if( pLine->GetUpper() )
            {
                // We need to correct the distance once again!
                aParam.nSide -= ::lcl_GetDistance( pLine->GetUpper(), true );

                if( bLeft )
                    aParam.nMaxSize = aParam.nSide;
                else
                    aParam.nMaxSize = pLine->GetUpper()->GetFrameFormat()->
                                    GetFrameSize().GetWidth() - aParam.nSide;
            }

            // First, see if there is enough room at all
            FN_lcl_SetBoxWidth fnTmp = aParam.bBigger ? fnOtherBox : fnSelBox;
            bRet = (*fnTmp)( pLine, aParam, nDistStt, true );

            // If true, set it
            if( bRet )
            {
                CR_SetBoxWidth aParam1( aParam );
                if( ppUndo )
                    ppUndo->reset(new SwUndoAttrTable( *aParam.pTableNd, true ));

                if( TableChgMode::FixedWidthChangeAbs != m_eTableChgMode && bLeft )
                {
                    (*fnSelBox)( pLine, aParam, nDistStt, false );
                    (*fnOtherBox)( pLine, aParam1, nDistStt, false );
                }
                else
                {
                    (*fnOtherBox)( pLine, aParam1, nDistStt, false );
                    (*fnSelBox)( pLine, aParam, nDistStt, false );
                }
            }
        }
        break;
        default: break;
    }

    if( xFndBox )
    {
        // Clean up the structure of all Lines
        GCLines();

        // Update Layout
        if( !bBigger || xFndBox->AreLinesToRestore( *this ) )
            xFndBox->MakeFrames( *this );

        // TL_CHART2: it is currently unclear if sth has to be done here.
        // The function name hints that nothing needs to be done, on the other
        // hand there is a case where sth gets deleted.  :-(

        xFndBox.reset();
    }

#if defined DBG_UTIL
    if( bRet )
    {
        CHECKBOXWIDTH
        CHECKTABLELAYOUT
    }
#endif

    return bRet;
}

static void SetLineHeight( SwTableLine& rLine, SwTwips nOldHeight, SwTwips nNewHeight,
                    bool bMinSize )
{
    SwLayoutFrame* pLineFrame = GetRowFrame( rLine );
    OSL_ENSURE( pLineFrame, "Where is the Frame from the SwTableLine?" );

    SwFrameFormat* pFormat = rLine.ClaimFrameFormat();

    SwTwips nMyNewH, nMyOldH = pLineFrame->getFrameArea().Height();
    if( !nOldHeight )                       // the BaseLine and absolute
        nMyNewH = nMyOldH + nNewHeight;
    else
    {
        // Calculate as exactly as possible
        Fraction aTmp( nMyOldH );
        aTmp *= Fraction( nNewHeight, nOldHeight );
        aTmp += Fraction( 1, 2 );       // round up if needed
        nMyNewH = tools::Long(aTmp);
    }

    SwFrameSize eSize = SwFrameSize::Minimum;
    if( !bMinSize &&
        ( nMyOldH - nMyNewH ) > ( CalcRowRstHeight( pLineFrame ) + ROWFUZZY ))
        eSize = SwFrameSize::Fixed;

    pFormat->SetFormatAttr( SwFormatFrameSize( eSize, 0, nMyNewH ) );

    // First adapt all internal ones
    for( auto pBox : rLine.GetTabBoxes() )
    {
        for( auto pLine : pBox->GetTabLines() )
            SetLineHeight( *pLine, nMyOldH, nMyNewH, bMinSize );
    }
}

static bool lcl_SetSelLineHeight( SwTableLine* pLine, const CR_SetLineHeight& rParam,
                             SwTwips nDist, bool bCheck )
{
    bool bRet = true;
    if( !bCheck )
    {
        // Set line height
        SetLineHeight( *pLine, 0, rParam.bBigger ? nDist : -nDist,
                        rParam.bBigger );
    }
    else if( !rParam.bBigger )
    {
        // Calculate the new relative size by means of the old one
        SwLayoutFrame* pLineFrame = GetRowFrame( *pLine );
        OSL_ENSURE( pLineFrame, "Where is the Frame from the SwTableLine?" );
        SwTwips nRstHeight = CalcRowRstHeight( pLineFrame );
        if( (nRstHeight + ROWFUZZY) < nDist )
            bRet = false;
    }
    return bRet;
}

static bool lcl_SetOtherLineHeight( SwTableLine* pLine, const CR_SetLineHeight& rParam,
                                SwTwips nDist, bool bCheck )
{
    bool bRet = true;
    if( bCheck )
    {
        if( rParam.bBigger )
        {
            // Calculate the new relative size by means of the old one
            SwLayoutFrame* pLineFrame = GetRowFrame( *pLine );
            OSL_ENSURE( pLineFrame, "Where is the Frame from the SwTableLine?" );

            if( TableChgMode::FixedWidthChangeProp == rParam.nMode )
            {
                nDist *= pLineFrame->getFrameArea().Height();
                nDist /= rParam.nMaxHeight;
            }
            bRet = nDist <= CalcRowRstHeight( pLineFrame );
        }
    }
    else
    {
        // Set line height
        // pLine is the following/preceding, thus adjust it
        if( TableChgMode::FixedWidthChangeProp == rParam.nMode )
        {
            SwLayoutFrame* pLineFrame = GetRowFrame( *pLine );
            OSL_ENSURE( pLineFrame, "Where is the Frame from the SwTableLine??" );

            // Calculate the new relative size by means of the old one
            // If the selected Box get bigger, adjust via the max space else
            // via the max height.
            if( (true) /*!rParam.bBigger*/ )
            {
                nDist *= pLineFrame->getFrameArea().Height();
                nDist /= rParam.nMaxHeight;
            }
            else
            {
                // Calculate the new relative size by means of the old one
                nDist *= CalcRowRstHeight( pLineFrame );
                nDist /= rParam.nMaxSpace;
            }
        }
        SetLineHeight( *pLine, 0, rParam.bBigger ? -nDist : nDist,
                        !rParam.bBigger );
    }
    return bRet;
}

bool SwTable::SetRowHeight( SwTableBox& rCurrentBox, TableChgWidthHeightType eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, std::unique_ptr<SwUndo>* ppUndo )
{
    SwTableLine* pLine = rCurrentBox.GetUpper();

    SwTableLine* pBaseLine = pLine;
    while( pBaseLine->GetUpper() )
        pBaseLine = pBaseLine->GetUpper()->GetUpper();

    std::unique_ptr<FndBox_> xFndBox;                // for insertion/deletion
    bool bBigger,
        bRet = false,
        bTop = TableChgWidthHeightType::CellTop == extractPosition( eType );
    sal_uInt16 nBaseLinePos = GetTabLines().GetPos( pBaseLine );

    CR_SetLineHeight aParam( eType,
                        const_cast<SwTableNode*>(rCurrentBox.GetSttNd()->FindTableNode()) );
    bBigger = aParam.bBigger;

    SwTableLines* pLines = &m_aLines;

    // How do we get to the height?
    switch( extractPosition(eType) )
    {
    case TableChgWidthHeightType::CellTop:
    case TableChgWidthHeightType::CellBottom:
        if( pLine == pBaseLine )
            break;  // it doesn't work then!

        // Is a nested Line (Box!)
        pLines = &pLine->GetUpper()->GetTabLines();
        nBaseLinePos = pLines->GetPos( pLine );
        [[fallthrough]];

    case TableChgWidthHeightType::RowBottom:
        {
            if( TableChgMode::VarWidthChangeAbs == m_eTableChgMode )
            {
                // First test if we have room at all
                if( bBigger )
                    bRet = true;
                else
                    bRet = lcl_SetSelLineHeight( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, true );

                if( bRet )
                {
                    if( ppUndo )
                        ppUndo->reset(new SwUndoAttrTable( *aParam.pTableNd, true ));

                    lcl_SetSelLineHeight( (*pLines)[ nBaseLinePos ], aParam,
                                    nAbsDiff, false );
                }
            }
            else
            {
                bRet = true;
                SwTableLines::size_type nStt;
                SwTableLines::size_type nEnd;
                if( bTop )
                {
                    nStt = 0;
                    nEnd = nBaseLinePos;
                }
                else
                {
                    nStt = nBaseLinePos + 1;
                    nEnd = pLines->size();
                }

                // Get the current Lines' height
                if( TableChgMode::FixedWidthChangeProp == m_eTableChgMode )
                {
                    for( auto n = nStt; n < nEnd; ++n )
                    {
                        SwLayoutFrame* pLineFrame = GetRowFrame( *(*pLines)[ n ] );
                        OSL_ENSURE( pLineFrame, "Where is the Frame from the SwTableLine??" );
                        aParam.nMaxSpace += CalcRowRstHeight( pLineFrame );
                        aParam.nMaxHeight += pLineFrame->getFrameArea().Height();
                    }
                    if( bBigger && aParam.nMaxSpace < nAbsDiff )
                        bRet = false;
                }
                else
                {
                    if( bTop ? nEnd != 0 : nStt < nEnd  )
                    {
                        if( bTop )
                            nStt = nEnd - 1;
                        else
                            nEnd = nStt + 1;
                    }
                    else
                        bRet = false;
                }

                if( bRet )
                {
                    if( bBigger )
                    {
                        for( auto n = nStt; n < nEnd; ++n )
                        {
                            if( !lcl_SetOtherLineHeight( (*pLines)[ n ], aParam,
                                                    nAbsDiff, true ))
                            {
                                bRet = false;
                                break;
                            }
                        }
                    }
                    else
                        bRet = lcl_SetSelLineHeight( (*pLines)[ nBaseLinePos ], aParam,
                                                nAbsDiff, true );
                }

                if( bRet )
                {
                    // Adjust
                    if( ppUndo )
                        ppUndo->reset(new SwUndoAttrTable( *aParam.pTableNd, true ));

                    CR_SetLineHeight aParam1( aParam );

                    if( bTop )
                    {
                        lcl_SetSelLineHeight( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, false );
                        for( auto n = nStt; n < nEnd; ++n )
                            lcl_SetOtherLineHeight( (*pLines)[ n ], aParam1,
                                            nAbsDiff, false );
                    }
                    else
                    {
                        for( auto n = nStt; n < nEnd; ++n )
                            lcl_SetOtherLineHeight( (*pLines)[ n ], aParam1,
                                            nAbsDiff, false );
                        lcl_SetSelLineHeight( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, false );
                    }
                }
                else
                {
                    // Then call itself recursively; only with another mode (proportional)
                    TableChgMode eOld = m_eTableChgMode;
                    m_eTableChgMode = TableChgMode::VarWidthChangeAbs;

                    bRet = SetRowHeight( rCurrentBox, eType, nAbsDiff,
                                        nRelDiff, ppUndo );

                    m_eTableChgMode = eOld;
                    xFndBox.reset();
                }
            }
        }
        break;
        default: break;
    }

    if( xFndBox )
    {
        // then clean up the structure of all Lines
        GCLines();

        // Update Layout
        if( bBigger || xFndBox->AreLinesToRestore( *this ) )
            xFndBox->MakeFrames( *this );

        // TL_CHART2: it is currently unclear if sth has to be done here.

        xFndBox.reset();
    }

    CHECKTABLELAYOUT

    return bRet;
}

SwFrameFormat* SwShareBoxFormat::GetFormat( tools::Long nWidth ) const
{
    SwFrameFormat *pRet = nullptr, *pTmp;
    for( auto n = m_aNewFormats.size(); n; )
        if( ( pTmp = m_aNewFormats[ --n ])->GetFrameSize().GetWidth()
                == nWidth )
        {
            pRet = pTmp;
            break;
        }
    return pRet;
}

SwFrameFormat* SwShareBoxFormat::GetFormat( const SfxPoolItem& rItem ) const
{
    const SfxPoolItem* pItem;
    sal_uInt16 nWhich = rItem.Which();
    SwFrameFormat *pRet = nullptr, *pTmp;
    const SfxPoolItem& rFrameSz = m_pOldFormat->GetFormatAttr( RES_FRM_SIZE, false );
    for( auto n = m_aNewFormats.size(); n; )
        if( SfxItemState::SET == ( pTmp = m_aNewFormats[ --n ])->
            GetItemState( nWhich, false, &pItem ) && *pItem == rItem &&
            pTmp->GetFormatAttr( RES_FRM_SIZE, false ) == rFrameSz )
        {
            pRet = pTmp;
            break;
        }
    return pRet;
}

void SwShareBoxFormat::AddFormat( SwFrameFormat& rNew )
{
    m_aNewFormats.push_back( &rNew );
}

bool SwShareBoxFormat::RemoveFormat( const SwFrameFormat& rFormat )
{
    // returns true, if we can delete
    if( m_pOldFormat == &rFormat )
        return true;

    std::vector<SwFrameFormat*>::iterator it = std::find( m_aNewFormats.begin(), m_aNewFormats.end(), &rFormat );
    if( m_aNewFormats.end() != it )
        m_aNewFormats.erase( it );
    return m_aNewFormats.empty();
}

SwShareBoxFormats::~SwShareBoxFormats()
{
}

SwFrameFormat* SwShareBoxFormats::GetFormat( const SwFrameFormat& rFormat, tools::Long nWidth ) const
{
    sal_uInt16 nPos;
    return Seek_Entry( rFormat, &nPos )
                    ? m_ShareArr[ nPos ]->GetFormat(nWidth)
                    : nullptr;
}
SwFrameFormat* SwShareBoxFormats::GetFormat( const SwFrameFormat& rFormat,
                                     const SfxPoolItem& rItem ) const
{
    sal_uInt16 nPos;
    return Seek_Entry( rFormat, &nPos )
                    ? m_ShareArr[ nPos ]->GetFormat(rItem)
                    : nullptr;
}

void SwShareBoxFormats::AddFormat( const SwFrameFormat& rOld, SwFrameFormat& rNew )
{
    sal_uInt16 nPos;
    SwShareBoxFormat* pEntry;
    if( !Seek_Entry( rOld, &nPos ))
    {
        pEntry = new SwShareBoxFormat( rOld );
        m_ShareArr.insert(m_ShareArr.begin() + nPos, std::unique_ptr<SwShareBoxFormat>(pEntry));
    }
    else
        pEntry = m_ShareArr[ nPos ].get();

    pEntry->AddFormat( rNew );
}

void SwShareBoxFormats::ChangeFrameFormat( SwTableBox* pBox, SwTableLine* pLn,
                                    SwFrameFormat& rFormat )
{
    SwClient aCl;
    SwFrameFormat* pOld = nullptr;
    if( pBox )
    {
        pOld = pBox->GetFrameFormat();
        pOld->Add( &aCl );
        pBox->ChgFrameFormat( static_cast<SwTableBoxFormat*>(&rFormat) );
    }
    else if( pLn )
    {
        pOld = pLn->GetFrameFormat();
        pOld->Add( &aCl );
        pLn->ChgFrameFormat( static_cast<SwTableLineFormat*>(&rFormat) );
    }
    if( pOld && pOld->HasOnlyOneListener() )
    {
        RemoveFormat( *pOld );
        delete pOld;
    }
}

void SwShareBoxFormats::SetSize( SwTableBox& rBox, const SwFormatFrameSize& rSz )
{
    SwFrameFormat *pBoxFormat = rBox.GetFrameFormat(),
             *pRet = GetFormat( *pBoxFormat, rSz.GetWidth() );
    if( pRet )
        ChangeFrameFormat( &rBox, nullptr, *pRet );
    else
    {
        pRet = rBox.ClaimFrameFormat();
        pRet->SetFormatAttr( rSz );
        AddFormat( *pBoxFormat, *pRet );
    }
}

void SwShareBoxFormats::SetAttr( SwTableBox& rBox, const SfxPoolItem& rItem )
{
    SwFrameFormat *pBoxFormat = rBox.GetFrameFormat(),
             *pRet = GetFormat( *pBoxFormat, rItem );
    if( pRet )
        ChangeFrameFormat( &rBox, nullptr, *pRet );
    else
    {
        pRet = rBox.ClaimFrameFormat();
        pRet->SetFormatAttr( rItem );
        AddFormat( *pBoxFormat, *pRet );
    }
}

void SwShareBoxFormats::SetAttr( SwTableLine& rLine, const SfxPoolItem& rItem )
{
    SwFrameFormat *pLineFormat = rLine.GetFrameFormat(),
             *pRet = GetFormat( *pLineFormat, rItem );
    if( pRet )
        ChangeFrameFormat( nullptr, &rLine, *pRet );
    else
    {
        pRet = rLine.ClaimFrameFormat();
        pRet->SetFormatAttr( rItem );
        AddFormat( *pLineFormat, *pRet );
    }
}

void SwShareBoxFormats::RemoveFormat( const SwFrameFormat& rFormat )
{
    for (auto i = m_ShareArr.size(); i; )
    {
        if (m_ShareArr[ --i ]->RemoveFormat(rFormat))
        {
            m_ShareArr.erase( m_ShareArr.begin() + i );
        }
    }
}

bool SwShareBoxFormats::Seek_Entry( const SwFrameFormat& rFormat, sal_uInt16* pPos ) const
{
    sal_uIntPtr nIdx = reinterpret_cast<sal_uIntPtr>(&rFormat);
    auto nO = m_ShareArr.size();
    decltype(nO) nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            const auto nM = nU + ( nO - nU ) / 2;
            sal_uIntPtr nFormat = reinterpret_cast<sal_uIntPtr>(&m_ShareArr[ nM ]->GetOldFormat());
            if( nFormat == nIdx )
            {
                if( pPos )
                    *pPos = nM;
                return true;
            }
            else if( nFormat < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pPos )
                    *pPos = nU;
                return false;
            }
            else
                nO = nM - 1;
        }
    }
    if( pPos )
        *pPos = nU;
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
