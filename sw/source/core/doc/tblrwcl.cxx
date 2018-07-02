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

#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/protitem.hxx>
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
#include <cntfrm.hxx>
#include <docsh.hxx>
#include <fesh.hxx>
#include <tabfrm.hxx>
#include <frmtool.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <tblafmt.hxx>
#include <tblsel.hxx>
#include <fldbas.hxx>
#include <swundo.hxx>
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

// In order to set the Frame Formats for the Boxes, it's enough to look
// up the current one in the array. If it's already there return the new one.
struct CpyTabFrame
{
    union {
        SwFrameFormat* pFrameFormat; // for CopyCol
        SwTwips nSize;               // for DelCol
    } Value;
    SwTableBoxFormat *pNewFrameFormat;

    explicit CpyTabFrame(SwFrameFormat* pCurrentFrameFormat) : pNewFrameFormat( nullptr )
    {   Value.pFrameFormat = pCurrentFrameFormat; }

    bool operator==( const CpyTabFrame& rCpyTabFrame ) const
        { return  static_cast<sal_uLong>(Value.nSize) == static_cast<sal_uLong>(rCpyTabFrame.Value.nSize); }
    bool operator<( const CpyTabFrame& rCpyTabFrame ) const
        { return  static_cast<sal_uLong>(Value.nSize) < static_cast<sal_uLong>(rCpyTabFrame.Value.nSize); }
};

struct CR_SetBoxWidth
{
    SwSelBoxes m_Boxes;
    SwShareBoxFormats aShareFormats;
    SwTableNode* pTableNd;
    SwUndoTableNdsChg* pUndo;
    SwTwips nDiff, nSide, nMaxSize, nLowerDiff;
    TableChgMode nMode;
    sal_uInt16 nTableWidth, nRemainWidth, nBoxWidth;
    bool bBigger, bLeft, bSplittBox, bAnyBoxFnd;

    CR_SetBoxWidth( TableChgWidthHeightType eType, SwTwips nDif, SwTwips nSid, SwTwips nTableW,
                    SwTwips nMax, SwTableNode* pTNd )
        : pTableNd( pTNd ), pUndo( nullptr ),
        nDiff( nDif ), nSide( nSid ), nMaxSize( nMax ), nLowerDiff( 0 ),
        nTableWidth( static_cast<sal_uInt16>(nTableW) ), nRemainWidth( 0 ), nBoxWidth( 0 ),
        bSplittBox( false ), bAnyBoxFnd( false )
    {
        bLeft = TableChgWidthHeightType::ColLeft == extractPosition( eType ) ||
                TableChgWidthHeightType::CellLeft == extractPosition( eType );
        bBigger = bool(eType & TableChgWidthHeightType::BiggerMode );
        nMode = pTableNd->GetTable().GetTableChgMode();
    }
    CR_SetBoxWidth( const CR_SetBoxWidth& rCpy )
        : pTableNd( rCpy.pTableNd ),
        pUndo( rCpy.pUndo ),
        nDiff( rCpy.nDiff ), nSide( rCpy.nSide ),
        nMaxSize( rCpy.nMaxSize ), nLowerDiff( 0 ),
        nMode( rCpy.nMode ), nTableWidth( rCpy.nTableWidth ),
        nRemainWidth( rCpy.nRemainWidth ), nBoxWidth( rCpy.nBoxWidth ),
        bBigger( rCpy.bBigger ), bLeft( rCpy.bLeft ),
        bSplittBox( rCpy.bSplittBox ), bAnyBoxFnd( rCpy.bAnyBoxFnd )
    {
    }

    SwUndoTableNdsChg* CreateUndo( SwUndoId eUndoType )
    {
        return pUndo = new SwUndoTableNdsChg( eUndoType, m_Boxes, *pTableNd );
    }

    void LoopClear()
    {
        nLowerDiff = 0; nRemainWidth = 0;
    }
};

static bool lcl_SetSelBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                         SwTwips nDist, bool bCheck );
static bool lcl_SetOtherBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck );
static bool lcl_InsSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck );
static bool lcl_InsOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck );
static bool lcl_DelSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck );
static bool lcl_DelOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
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

struct CR_SetLineHeight
{
    SwSelBoxes m_Boxes;
    SwShareBoxFormats aShareFormats;
    SwTableNode* pTableNd;
    SwUndoTableNdsChg* pUndo;
    SwTwips nMaxSpace, nMaxHeight;
    TableChgMode nMode;
    bool bBigger, bTop;

    CR_SetLineHeight( TableChgWidthHeightType eType, SwTableNode* pTNd )
        : pTableNd( pTNd ), pUndo( nullptr ),
        nMaxSpace( 0 ), nMaxHeight( 0 )
    {
        bTop = TableChgWidthHeightType::CellTop == extractPosition( eType );
        bBigger = bool(eType & TableChgWidthHeightType::BiggerMode );
        if( eType & TableChgWidthHeightType::InsertDeleteMode )
            bBigger = !bBigger;
        nMode = pTableNd->GetTable().GetTableChgMode();
    }
    CR_SetLineHeight( const CR_SetLineHeight& rCpy )
        : pTableNd( rCpy.pTableNd ), pUndo( rCpy.pUndo ),
        nMaxSpace( rCpy.nMaxSpace ), nMaxHeight( rCpy.nMaxHeight ),
        nMode( rCpy.nMode ),
        bBigger( rCpy.bBigger ), bTop( rCpy.bTop )
    {}

    SwUndoTableNdsChg* CreateUndo( SwUndoId nUndoType )
    {
        return pUndo = new SwUndoTableNdsChg( nUndoType, m_Boxes, *pTableNd );
    }
};

static bool lcl_SetSelLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                         SwTwips nDist, bool bCheck );
static bool lcl_SetOtherLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, bool bCheck );
static bool lcl_InsDelSelLine( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, bool bCheck );

typedef bool (*FN_lcl_SetLineHeight)(SwTableLine*, CR_SetLineHeight&, SwTwips, bool );

typedef o3tl::sorted_vector<CpyTabFrame> CpyTabFrames;

struct CpyPara
{
    std::shared_ptr< std::vector< std::vector< sal_uLong > > > pWidths;
    SwDoc* pDoc;
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
        : pDoc( pNd->GetDoc() ), pTableNd( pNd ), rTabFrameArr(rFrameArr),
        pInsLine(nullptr), pInsBox(nullptr), nOldSize(0), nNewSize(0),
        nMinLeft(ULONG_MAX), nMaxRight(0),
        nCpyCnt(nCopies), nInsPos(0),
        nLnIdx(0), nBoxIdx(0),
        nDelBorderFlag(0), bCpyContent( true )
        {}
    CpyPara( const CpyPara& rPara, SwTableLine* pLine )
        : pWidths( rPara.pWidths ), pDoc(rPara.pDoc), pTableNd(rPara.pTableNd),
        rTabFrameArr(rPara.rTabFrameArr), pInsLine(pLine), pInsBox(rPara.pInsBox),
        nOldSize(0), nNewSize(rPara.nNewSize), nMinLeft( rPara.nMinLeft ),
        nMaxRight( rPara.nMaxRight ), nCpyCnt(rPara.nCpyCnt), nInsPos(0),
        nLnIdx( rPara.nLnIdx), nBoxIdx( rPara.nBoxIdx ),
        nDelBorderFlag( rPara.nDelBorderFlag ), bCpyContent( rPara.bCpyContent )
        {}
    CpyPara( const CpyPara& rPara, SwTableBox* pBox )
        : pWidths( rPara.pWidths ), pDoc(rPara.pDoc), pTableNd(rPara.pTableNd),
        rTabFrameArr(rPara.rTabFrameArr), pInsLine(rPara.pInsLine), pInsBox(pBox),
        nOldSize(rPara.nOldSize), nNewSize(rPara.nNewSize),
        nMinLeft( rPara.nMinLeft ), nMaxRight( rPara.nMaxRight ),
        nCpyCnt(rPara.nCpyCnt), nInsPos(0), nLnIdx(rPara.nLnIdx), nBoxIdx(rPara.nBoxIdx),
        nDelBorderFlag( rPara.nDelBorderFlag ), bCpyContent( rPara.bCpyContent )
        {}
};

static void lcl_CopyRow(FndLine_ & rFndLine, CpyPara *const pCpyPara);

static void lcl_CopyCol( FndBox_ & rFndBox, CpyPara *const pCpyPara)
{
    // Look up the Frame Format in the Frame Format Array
    SwTableBox* pBox = rFndBox.GetBox();
    CpyTabFrame aFindFrame(pBox->GetFrameFormat());

    sal_uInt16 nFndPos;
    if( pCpyPara->nCpyCnt )
    {
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
            if( pBox->GetTabLines().size() )
            {
                pCmpLine = rFndBox.GetLines().front().get();
                if ( pCmpLine->GetBoxes().size() != pCmpLine->GetLine()->GetTabBoxes().size() )
                    bDiffCount = true;
            }

            if( bDiffCount )
            {
                // The first Line should be enough
                FndBoxes_t const& rFndBoxes = pCmpLine->GetBoxes();
                long nSz = 0;
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
                aFindFrame.Value.pFrameFormat = pNewFormat;
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
        pCpyPara->pInsLine->GetTabBoxes().emplace( pCpyPara->pInsLine->GetTabBoxes().begin() + pCpyPara->nInsPos++, pBox );
        CpyPara aPara( *pCpyPara, pBox );
        aPara.nDelBorderFlag &= 7;

        for (auto const& pFndLine : rFndBox.GetLines())
        {
            lcl_CopyRow(*pFndLine, &aPara);
        }
    }
    else
    {
        ::InsTableBox( pCpyPara->pDoc, pCpyPara->pTableNd, pCpyPara->pInsLine,
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
                aFindFrame.Value.pFrameFormat = pBox->GetFrameFormat();

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
                                            pCpyPara->nInsPos - 1 ].get();
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

SwRowFrame* GetRowFrame( SwTableLine& rLine )
{
    SwIterator<SwRowFrame,SwFormat> aIter( *rLine.GetFrameFormat() );
    for( SwRowFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
        if( pFrame->GetTabLine() == &rLine )
            return pFrame;
    return nullptr;
}

bool SwTable::InsertCol( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( !rBoxes.empty() && nCnt, "No valid Box List" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    bool bRes = true;
    if( IsNewModel() )
        bRes = NewInsertCol( pDoc, rBoxes, nCnt, bBehind );
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

    SwChartDataProvider *pPCD = pDoc->getIDocumentChartDataProviderAccess().GetChartDataProvider();
    if (pPCD && nCnt)
        pPCD->AddRowCols( *this, rBoxes, nCnt, bBehind );
    pDoc->UpdateCharts( GetFrameFormat()->GetName() );

    pDoc->GetDocShell()->GetFEShell()->UpdateTableStyleFormatting();

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
        while( 1 == pFndBox->GetLines().size() &&
            1 == (pFndLine = pFndBox->GetLines()[0].get())->GetBoxes().size())
        {
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

    pDoc->GetDocShell()->GetFEShell()->UpdateTableStyleFormatting( pTableNd );

    return true;
}

static void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const long nOffset,
                            bool bFirst, SwShareBoxFormats& rShareFormats );

static void lcl_LastBoxSetWidthLine( SwTableLines &rLines, const long nOffset,
                                bool bFirst, SwShareBoxFormats& rShareFormats )
{
    for ( auto pLine : rLines )
        ::lcl_LastBoxSetWidth( pLine->GetTabBoxes(), nOffset, bFirst, rShareFormats );
}

static void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const long nOffset,
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
                    SwTableBox* pNxtBox = rTableBoxes[ nDelPos + 1 ].get();
                    const SvxBoxItem& rNxtBoxItem = pNxtBox->GetFrameFormat()->GetBox();

                    SwTableBox* pPrvBox = nDelPos ? rTableBoxes[ nDelPos - 1 ].get() : nullptr;

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
                    SwTableBox* pPrvBox = rTableBoxes[ nDelPos - 1 ].get();
                    const SvxBoxItem& rPrvBoxItem = pPrvBox->GetFrameFormat()->GetBox();

                    SwTableBox* pNxtBox = nDelPos + 1 < static_cast<sal_uInt16>(rTableBoxes.size())
                                            ? rTableBoxes[ nDelPos + 1 ].get() : nullptr;

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
            rTable.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteTableCellRedline( rTable.GetFrameFormat()->GetDoc(), *(rTableBoxes[nDelPos]), true, USHRT_MAX );
        rTableBoxes.erase( rTableBoxes.begin() + nDelPos );

        if( pSttNd )
        {
            // Has the UndoObject been prepared to save the Section?
            if( pUndo && pUndo->IsDelBox() )
                static_cast<SwUndoTableNdsChg*>(pUndo)->SaveSection( pSttNd );
            else
                pSttNd->GetDoc()->getIDocumentContentOperations().DeleteSection( pSttNd );
        }

        // Also delete the Line?
        if( !rTableBoxes.empty() )
        {
            // Then adapt the Frame-SSize
            bool bLastBox = nDelPos == rTableBoxes.size();
            if( bLastBox )
                --nDelPos;
            pBox = rTableBoxes[nDelPos].get();
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
                rTable.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteTableRowRedline( rTable.GetFrameFormat()->GetDoc(), *pTabLineToDelete, true, USHRT_MAX );
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
            rTable.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteTableRowRedline( rTable.GetFrameFormat()->GetDoc(), *pTabLineToDelete, true, USHRT_MAX );
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

        pFndBox = pLine->GetTabBoxes()[ 0 ].get();
        for( std::unique_ptr<SwTableBox> const & pBox : pLine->GetTabBoxes() )
        {
            if ( nFndWidth <= 0 )
            {
                break;
            }
            pFndBox = pBox.get();
            nFndBoxWidth = pFndBox->GetFrameFormat()->GetFrameSize().GetWidth();
            nFndWidth -= nFndBoxWidth;
        }

        // Find the first ContentBox
        while( !pFndBox->GetSttNd() )
        {
            const SwTableLines& rLowLns = pFndBox->GetTabLines();
            if( bNxt )
                pFndBox = rLowLns.front()->GetTabBoxes().front().get();
            else
                pFndBox = rLowLns.back()->GetTabBoxes().front().get();
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
    if( !bChgd && pPrvBox && pPrvBox->GetSttNd() )
    {
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

bool SwTable::OldSplitRow( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt,
                        bool bSameHeight )
{
    OSL_ENSURE( pDoc && !rBoxes.empty() && nCnt, "No valid values" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    // TL_CHART2: splitting/merging of a number of cells or rows will usually make
    // the table too complex to be handled with chart.
    // Thus we tell the charts to use their own data provider and forget about this table
    pDoc->getIDocumentChartDataProviderAccess().CreateChartInternalDataProviders( this );

    SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());    // Delete HTML Layout

    // If the rows should get the same (min) height, we first have
    // to store the old row heights before deleting the frames
    long* pRowHeights = nullptr;
    if ( bSameHeight )
    {
        pRowHeights = new long[ rBoxes.size() ];
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
        if ( bSameHeight && ATT_VAR_SIZE == aFSz.GetHeightSizeType() )
            aFSz.SetHeightSizeType( ATT_MIN_SIZE );

        bool bChgLineSz = 0 != aFSz.GetHeight() || bSameHeight;
        if ( bChgLineSz )
            aFSz.SetHeight( ( bSameHeight ? pRowHeights[ n ] : aFSz.GetHeight() ) /
                             (nCnt + 1) );

        SwTableBox* pNewBox = new SwTableBox( pFrameFormat, nCnt, pInsLine );
        sal_uInt16 nBoxPos = pInsLine->GetBoxPos( pSelBox );
        pInsLine->GetTabBoxes()[nBoxPos].reset( pNewBox ); // overwrite old one

        // Delete background/border attribute
        SwTableBox* pLastBox = pSelBox;         // To distribute the TextNodes!
        // If Areas are contained in the Box, it stays as is
        // !! If this is changed we need to adapt the Undo, too !!!
        bool bMoveNodes = true;
        {
            sal_uLong nSttNd = pLastBox->GetSttIdx() + 1,
                    nEndNd = pLastBox->GetSttNd()->EndOfSectionIndex();
            while( nSttNd < nEndNd )
                if( !pDoc->GetNodes()[ nSttNd++ ]->IsTextNode() )
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
                pNewLine->GetTabBoxes().emplace( pNewLine->GetTabBoxes().begin(), pSelBox );
            }
            else
            {
                ::InsTableBox( pDoc, pTableNd, pNewLine, pCpyBoxFrameFormat,
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
                        pLastBox = pNewLine->GetTabBoxes()[0].get();  // reset
                        SwNodeIndex aInsPos( *pLastBox->GetSttNd(), 1 );
                        pDoc->GetNodes().MoveNodes(aRg, pDoc->GetNodes(), aInsPos, false);
                        pDoc->GetNodes().Delete( aInsPos ); // delete the empty one
                    }
                }
            }
        }
        // In Boxes with Lines, we can only have Size/Fillorder
        pFrameFormat = static_cast<SwTableBoxFormat*>(pNewBox->ClaimFrameFormat());
        pFrameFormat->ResetFormatAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
        pFrameFormat->ResetFormatAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
    }

    delete[] pRowHeights;

    GCLines();

    aFndBox.MakeFrames( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT
    return true;
}

bool SwTable::SplitCol( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt )
{
    OSL_ENSURE( pDoc && !rBoxes.empty() && nCnt, "No valid values" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    // TL_CHART2: splitting/merging of a number of cells or rows will usually make
    // the table too complex to be handled with chart.
    // Thus we tell the charts to use their own data provider and forget about this table
    pDoc->getIDocumentChartDataProviderAccess().CreateChartInternalDataProviders( this );

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
            aFindFrame.pNewFrameFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE,
                                                        nNewBoxSz, 0 ) );
            aFrameArr.insert( aFindFrame );

            pLastBoxFormat = aFindFrame.pNewFrameFormat;
            if( nBoxSz != ( nNewBoxSz * (nCnt + 1)))
            {
                // We have a remainder, so we need to define an own Format
                // for the last Box.
                pLastBoxFormat = new SwTableBoxFormat( *aFindFrame.pNewFrameFormat );
                pLastBoxFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE,
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
            ::InsTableBox( pDoc, pTableNd, pInsLine, aFindFrame.pNewFrameFormat,
                        pSelBox, nBoxPos + i ); // insert after

        ::InsTableBox( pDoc, pTableNd, pInsLine, pLastBoxFormat,
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
                    rCTF.Value.pFrameFormat == aFindFrame.pNewFrameFormat )
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
                              std::make_move_iterator(rBoxes.begin() + nStt),
                              std::make_move_iterator(rBoxes.begin() + nEnd ));
    rBoxes.erase( rBoxes.begin() + nStt, rBoxes.begin() + nEnd );
}

static void lcl_CalcWidth( SwTableBox* pBox )
{
    // Assertion: Every Line in the Box is as large
    SwFrameFormat* pFormat = pBox->ClaimFrameFormat();
    OSL_ENSURE( pBox->GetTabLines().size(), "Box does not have any Lines" );

    SwTableLine* pLine = pBox->GetTabLines()[0];
    OSL_ENSURE( pLine, "Box is not within a Line" );

    long nWidth = 0;
    for( std::unique_ptr<SwTableBox> const & pTabBox : pLine->GetTabBoxes() )
        nWidth += pTabBox->GetFrameFormat()->GetFrameSize().GetWidth();

    pFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, nWidth, 0 ));

    // Boxes with Lines can only have Size/Fillorder
    pFormat->ResetFormatAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
    pFormat->ResetFormatAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
}

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
            if( 0 != ( nPos = pFndTableBox->GetUpper()->GetBoxPos( pFndTableBox ) ) )
                lcl_CpyBoxes( 0, nPos, *pBoxes, pULPara->pInsLine );
        }
        else                // Right
            // if there are Boxes behind it, move them
            if( (nPos = pFndTableBox->GetUpper()->GetBoxPos( pFndTableBox )) +1 < static_cast<sal_uInt16>(pBoxes->size()) )
            {
                nInsPos = pULPara->pInsLine->GetTabBoxes().size();
                lcl_CpyBoxes( nPos+1, pBoxes->size(),
                                    *pBoxes, pULPara->pInsLine );
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
    if (rFndBox.GetBox()->GetTabLines().size())
    {
        std::unique_ptr<SwTableBox> pBox( new SwTableBox(
                static_cast<SwTableBoxFormat*>(rFndBox.GetBox()->GetFrameFormat()),
                0, pULPara->pInsLine ) );
        InsULPara aPara( *pULPara );
        aPara.pInsBox = pBox.get();
        for (FndLines_t::iterator it = rFndBox.GetLines().begin() + nStt;
             it != rFndBox.GetLines().begin() + nEnd; ++it )
        {
            lcl_Merge_MoveLine(**it, &aPara);
        }
        if( pBox->GetTabLines().size() )
        {
            if( USHRT_MAX == nInsPos )
                nInsPos = pBoxes->size();
            auto pBoxTemp = pBox.get();
            pBoxes->insert( pBoxes->begin() + nInsPos, std::move(pBox) );
            lcl_CalcWidth( pBoxTemp );      // calculate the Box's width
        }
    }
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
                if( 0 != ( nPos = pLines->GetPos( pFndLn )) )
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

            pInsLine->GetTabBoxes().emplace( pInsLine->GetTabBoxes().begin(), pLMBox );

            if( pULPara->bUL )  // Upper ?
            {
                // If there are Lines before it, move them
                if( 0 != ( nPos = pLines->GetPos( pFndLn )) )
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
    pInsLine->GetTabBoxes().emplace( pInsLine->GetTabBoxes().begin(), pLeftBox );
    pLeftBox->ClaimFrameFormat();
    pInsLine->GetTabBoxes().emplace( pInsLine->GetTabBoxes().begin() + 1, pMergeBox);
    pInsLine->GetTabBoxes().emplace( pInsLine->GetTabBoxes().begin() + 2, pRightBox );
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

    for( std::unique_ptr<SwTableBox> const & pBox : GetTabLines()[0]->GetTabBoxes() )
        lcl_BoxSetHeadCondColl(pBox.get());

    aFndBox.MakeFrames( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT

    return true;
}

static void lcl_CheckRowSpan( SwTable &rTable )
{
    const long nLineCount = static_cast<long>(rTable.GetTabLines().size());
    long nMaxSpan = nLineCount;
    long nMinSpan = 1;
    while( nMaxSpan )
    {
        SwTableLine* pLine = rTable.GetTabLines()[ nLineCount - nMaxSpan ];
        for( std::unique_ptr<SwTableBox> const & pBox : pLine->GetTabBoxes() )
        {
            long nRowSpan = pBox->getRowSpan();
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
        for( std::unique_ptr<SwTableBox> const & pCmp : rBoxes )
        {
            if (pBox==pCmp.get())
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
            std::vector< sal_uLong > &rWidth = (*rPara.pWidths.get())[ nLine ];
            const FndLine_ *pFndLine = rFndLines[ nLine ].get();
            if( pFndLine && pFndLine->GetBoxes().size() )
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
                        SwTableBox* pBox = pLine->GetTabBoxes()[nBox++].get();
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
    if( nSelSize )
    {
        for( size_t nLine = 0; nLine < nLineCount; ++nLine )
        {
            std::vector< sal_uLong > &rWidth = (*rPara.pWidths.get())[ nLine ];
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
            nDummy1 = (*pCpyPara->pWidths.get())[pCpyPara->nLnIdx][0];
        nRealSize = (*pCpyPara->pWidths.get())[pCpyPara->nLnIdx][pCpyPara->nBoxIdx++];
        if( pCpyPara->nBoxIdx == (*pCpyPara->pWidths.get())[pCpyPara->nLnIdx].size()-1 )
            nDummy2 = (*pCpyPara->pWidths.get())[pCpyPara->nLnIdx][pCpyPara->nBoxIdx];
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

        SwFormatFrameSize aFrameSz;
        CpyTabFrames::const_iterator itFind = pCpyPara->rTabFrameArr.lower_bound( aFindFrame );
        const CpyTabFrames::size_type nFndPos = itFind - pCpyPara->rTabFrameArr.begin();
        if( itFind == pCpyPara->rTabFrameArr.end() || !(*itFind == aFindFrame) ||
            ( aFrameSz = ( aFindFrame = pCpyPara->rTabFrameArr[ nFndPos ]).pNewFrameFormat->
                GetFrameSize()).GetWidth() != static_cast<SwTwips>(nSize) )
        {
            // It doesn't exist yet, so copy it
            aFindFrame.pNewFrameFormat = pCpyPara->pDoc->MakeTableBoxFormat();
            aFindFrame.pNewFrameFormat->CopyAttrs( *rFndBox.GetBox()->GetFrameFormat() );
            if( !pCpyPara->bCpyContent )
                aFindFrame.pNewFrameFormat->ResetFormatAttr(  RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
            aFrameSz.SetWidth( nSize );
            aFindFrame.pNewFrameFormat->SetFormatAttr( aFrameSz );
            pCpyPara->rTabFrameArr.insert( aFindFrame );
        }

        SwTableBox* pBox;
        if (!rFndBox.GetLines().empty())
        {
            pBox = new SwTableBox( aFindFrame.pNewFrameFormat,
                        rFndBox.GetLines().size(), pCpyPara->pInsLine );
            pCpyPara->pInsLine->GetTabBoxes().emplace( pCpyPara->pInsLine->GetTabBoxes().begin() + pCpyPara->nInsPos++, pBox );
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
            pCpyPara->pDoc->GetNodes().InsBoxen( pCpyPara->pTableNd, pCpyPara->pInsLine,
                            aFindFrame.pNewFrameFormat,
                            pCpyPara->pDoc->GetDfltTextFormatColl(),
                            nullptr, pCpyPara->nInsPos );
            pBox = pCpyPara->pInsLine->GetTabBoxes()[ pCpyPara->nInsPos ].get();
            if( bDummy )
                pBox->setDummyFlag( true );
            else if( pCpyPara->bCpyContent )
            {
                // Copy the content into this empty Box
                pBox->setRowSpan(rFndBox.GetBox()->getRowSpan());

                // We can also copy formulas and values, if we copy the content
                {
                    SfxItemSet aBoxAttrSet( pCpyPara->pDoc->GetAttrPool(),
                                            svl::Items<RES_BOXATR_FORMAT, RES_BOXATR_VALUE>{} );
                    aBoxAttrSet.Put(rFndBox.GetBox()->GetFrameFormat()->GetAttrSet());
                    if( aBoxAttrSet.Count() )
                    {
                        const SfxPoolItem* pItem;
                        SvNumberFormatter* pN = pCpyPara->pDoc->GetNumberFormatter( false );
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

                pFromDoc->GetDocumentContentOperationsManager().CopyWithFlyInFly( aCpyRg, 0, aInsIdx, nullptr, false );
                // Delete the initial TextNode
                pCpyPara->pDoc->GetNodes().Delete( aInsIdx );
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
        aFindFrame.pNewFrameFormat = reinterpret_cast<SwTableBoxFormat*>(pCpyPara->pDoc->MakeTableLineFormat());
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
    for (auto & rpFndLine : aFndBox.GetLines())
    {
         lcl_CopyLineToDoc( *rpFndLine, &aPara );
    }
    if( rTableNd.GetTable().IsNewModel() )
    {   // The copied line must not contain any row span attributes > 1
        SwTableLine* pLine = rTableNd.GetTable().GetTabLines()[0];
        OSL_ENSURE( !pLine->GetTabBoxes().empty(), "Empty Table Line" );
        for( std::unique_ptr<SwTableBox> const & pTableBox : pLine->GetTabBoxes() )
        {
            OSL_ENSURE( pTableBox, "Missing Table Box" );
            pTableBox->setRowSpan( 1 );
        }
    }
}

bool SwTable::MakeCopy( SwDoc* pInsDoc, const SwPosition& rPos,
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
    if( pSrcDoc != pInsDoc )
    {
        pInsDoc->CopyTextColl( *pSrcDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TABLE ) );
        pInsDoc->CopyTextColl( *pSrcDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TABLE_HDLN ) );
    }

    SwTable* pNewTable = const_cast<SwTable*>(pInsDoc->InsertTable(
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
        SwFieldType* pFieldType = pInsDoc->getIDocumentFieldsAccess().InsertFieldType(
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
        // Conver the Table formulas to their relative representation
        SwTableFormulaUpdate aMsgHint( this );
        aMsgHint.m_eFlags = TBL_RELBOXNAME;
        pSrcDoc->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
    }

    SwTableNumFormatMerge aTNFM( *pSrcDoc, *pInsDoc );

    // Also copy Names or enforce a new unique one
    if( bCpyName )
        pNewTable->GetFrameFormat()->SetName( GetFrameFormat()->GetName() );

    CpyTabFrames aCpyFormat;
    CpyPara aPara( pTableNd, 1, aCpyFormat );
    aPara.nNewSize = aPara.nOldSize = GetFrameFormat()->GetFrameSize().GetWidth();

    if( IsNewModel() )
        lcl_CalcNewWidths( aFndBox.GetLines(), aPara );
    // Copy
    for (auto & rpFndLine : aFndBox.GetLines())
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
            for( std::unique_ptr<SwTableBox> const & pBox : pLn->GetTabBoxes() )
                sw_Box_CollectBox( pBox.get(), &aLnPara );

            if( aLnPara.Resize( lcl_GetBoxOffset( aFndBox ),
                                lcl_GetLineWidth( *pFndLn )) )
            {
                aLnPara.SetValues( true );
                pLn = pNewTable->GetTabLines()[ 0 ];
                for( std::unique_ptr<SwTableBox> const & pBox : pLn->GetTabBoxes() )
                    sw_BoxSetSplitBoxFormats(pBox.get(), &aLnPara );
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
            for( std::unique_ptr<SwTableBox> const & pBox : pLn->GetTabBoxes() )
                sw_Box_CollectBox( pBox.get(), &aLnPara );

            if( aLnPara.Resize( lcl_GetBoxOffset( aFndBox ),
                                lcl_GetLineWidth( *pFndLn )) )
            {
                aLnPara.SetValues( false );
                pLn = pNewTable->GetTabLines().back();
                for( std::unique_ptr<SwTableBox> const & pBox : pLn->GetTabBoxes() )
                    sw_BoxSetSplitBoxFormats(pBox.get(), &aLnPara );
            }
        }
    }

    // We need to delete the initial Box
    DeleteBox_( *pNewTable, pNewTable->GetTabLines().back()->GetTabBoxes()[0].get(),
                nullptr, false, false );

    if( pNewTable->IsNewModel() )
        lcl_CheckRowSpan( *pNewTable );
    // Clean up
    pNewTable->GCLines();

    pTableNd->MakeFrames( &aIdx );  // re-generate the Frames

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
    if( !GetTabBoxes().empty() && pSrchBox &&
        USHRT_MAX != ( nFndPos = GetBoxPos( pSrchBox )) &&
        nFndPos + 1 != static_cast<sal_uInt16>(GetTabBoxes().size()) )
    {
        pBox = GetTabBoxes()[ nFndPos + 1 ].get();
        while( !pBox->GetTabLines().empty() )
            pBox = pBox->GetTabLines().front()->GetTabBoxes()[0].get();
        return pBox;
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
        pBox = pLine->GetTabBoxes().front().get();
        while( !pBox->GetTabLines().empty() )
            pBox = pBox->GetTabLines().front()->GetTabBoxes().front().get();
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
    if( !GetTabBoxes().empty() && pSrchBox &&
        USHRT_MAX != ( nFndPos = GetBoxPos( pSrchBox )) &&
        nFndPos )
    {
        pBox = GetTabBoxes()[ nFndPos - 1 ].get();
        while( !pBox->GetTabLines().empty() )
        {
            pLine = pBox->GetTabLines().back();
            pBox = pLine->GetTabBoxes().back().get();
        }
        return pBox;
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
        pBox = pLine->GetTabBoxes().back().get();
        while( !pBox->GetTabLines().empty() )
        {
            pLine = pBox->GetTabLines().back();
            pBox = pLine->GetTabBoxes().back().get();
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
    for( std::unique_ptr<SwTableBox> const & pBox : pLine->GetTabBoxes() )
        lcl_BoxSetHeadCondColl(pBox.get());
}

static SwTwips lcl_GetDistance( SwTableBox* pBox, bool bLeft )
{
    bool bFirst = true;
    SwTwips nRet = 0;
    SwTableLine* pLine;
    while( pBox && nullptr != ( pLine = pBox->GetUpper() ) )
    {
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
    for( std::unique_ptr<SwTableBox> const & pBox : rBoxes )
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
                rParam.bAnyBoxFnd = true;
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
    for( std::unique_ptr<SwTableBox> const & pBox : rBoxes )
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
                rParam.bAnyBoxFnd = true;
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

static bool lcl_InsSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                            SwTwips nDist, bool bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( size_t n = 0; n < rBoxes.size(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ].get();
        SwTableBoxFormat* pFormat = static_cast<SwTableBoxFormat*>(pBox->GetFrameFormat());
        const SwFormatFrameSize& rSz = pFormat->GetFrameSize();
        SwTwips nWidth = rSz.GetWidth();

        int nCmp {0};

        if( bCheck )
        {
            for( size_t i = 0; i < pBox->GetTabLines().size(); ++i )
                if( !::lcl_InsSelBox( pBox->GetTabLines()[ i ], rParam,
                                            nDist, true ))
                    return false;

            // Collect all "ContentBoxes"
            if( std::abs( nDist + ( rParam.bLeft ? 0 : nWidth )
                    - rParam.nSide ) < COLFUZZY )
                nCmp = 1;
            else if( nDist + ( rParam.bLeft ? 0 : nWidth/2 ) > rParam.nSide )
                nCmp = 2;

            if( nCmp )
            {
                rParam.bAnyBoxFnd = true;
                if( pFormat->GetProtect().IsContentProtected() )
                    return false;

                if( rParam.bSplittBox &&
                    nWidth - rParam.nDiff <= COLFUZZY +
                        ( 567 / 2 /*leave room for at least 0.5 cm*/) )
                    return false;

                if( pBox->GetSttNd() )
                {
                    rParam.m_Boxes.insert(pBox);
                }

                break;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( auto pLn : pBox->GetTabLines() )
            {
                rParam.nLowerDiff = 0;
                lcl_InsSelBox( pLn, rParam, nDist, false );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff )
                nCmp = 1;
            else if( std::abs( nDist + ( rParam.bLeft ? 0 : nWidth )
                                - rParam.nSide ) < COLFUZZY )
                nCmp = 2;
            else if( nDist + nWidth / 2 > rParam.nSide )
                nCmp = 3;

            if( nCmp )
            {
                // This column contains the Cursor - so decrease/increase
                if( 1 == nCmp )
                {
                    if( !rParam.bSplittBox )
                    {
                        // the current Box on
                        SwFormatFrameSize aNew( rSz );
                        aNew.SetWidth( nWidth + rParam.nDiff );
                        rParam.aShareFormats.SetSize( *pBox, aNew );
                    }
                }
                else
                {
                    OSL_ENSURE( pBox->GetSttNd(), "This must be an EndBox!");

                    if( !rParam.bLeft && 3 != nCmp )
                        ++n;

                    ::InsTableBox( pFormat->GetDoc(), rParam.pTableNd,
                                        pLine, pFormat, pBox, n );

                    SwTableBox* pNewBox = rBoxes[ n ].get();
                    SwFormatFrameSize aNew( rSz );
                    aNew.SetWidth( rParam.nDiff );
                    rParam.aShareFormats.SetSize( *pNewBox, aNew );

                    // Special case: There is no space in the other Boxes, but in the Cell
                    if( rParam.bSplittBox )
                    {
                        // the current Box on
                        SwFormatFrameSize aNewSize( rSz );
                        aNewSize.SetWidth( nWidth - rParam.nDiff );
                        rParam.aShareFormats.SetSize( *pBox, aNewSize );
                    }

                    // Special treatment for the Border
                    // The right one needs to be removed
                    {
                        const SvxBoxItem& rBoxItem = pBox->GetFrameFormat()->GetBox();
                        if( rBoxItem.GetRight() )
                        {
                            SvxBoxItem aTmp( rBoxItem );
                            aTmp.SetLine( nullptr, SvxBoxItemLine::RIGHT );
                            rParam.aShareFormats.SetAttr( rParam.bLeft
                                                            ? *pNewBox
                                                            : *pBox, aTmp );
                        }
                    }
                }

                rParam.nLowerDiff = rParam.nDiff;
                break;
            }
        }

        if( rParam.bLeft && rParam.nMode != TableChgMode::FixedWidthChangeAbs && nDist >= rParam.nSide )
            break;

        nDist += nWidth;
    }
    return true;
}

static bool lcl_InsOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck )
{
    // Special case: There is no space in the other Boxes, but in the cell
    if( rParam.bSplittBox )
        return true;

    SwTableBoxes& rBoxes = pLine->GetTabBoxes();

    // Table fixed, proportional
    if( !rParam.nRemainWidth && TableChgMode::FixedWidthChangeProp == rParam.nMode )
    {
        // Find the right width to which the relative width adjustment
        // corresponds to
        SwTwips nTmpDist = nDist;
        for( std::unique_ptr<SwTableBox> const & pBox : rBoxes )
        {
            SwTwips nWidth = pBox->GetFrameFormat()->GetFrameSize().GetWidth();
            if( (nTmpDist + nWidth / 2 ) > rParam.nSide )
            {
                rParam.nRemainWidth = rParam.bLeft
                                        ? sal_uInt16(nTmpDist)
                                        : sal_uInt16(rParam.nTableWidth - nTmpDist);
                break;
            }
            nTmpDist += nWidth;
        }
    }

    for( SwTableBoxes::size_type n = 0; n < rBoxes.size(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ].get();
        SwFrameFormat* pFormat = pBox->GetFrameFormat();
        const SwFormatFrameSize& rSz = pFormat->GetFrameSize();
        SwTwips nWidth = rSz.GetWidth();

        if( bCheck )
        {
            for( auto pLn : pBox->GetTabLines() )
                if( !::lcl_InsOtherBox( pLn, rParam, nDist, true ))
                    return false;

            if(
                rParam.bLeft ? ((nDist + nWidth / 2 ) <= rParam.nSide &&
                                (TableChgMode::FixedWidthChangeAbs != rParam.nMode ||
                                (n < rBoxes.size() &&
                                  (nDist + nWidth + rBoxes[ n+1 ]->
                                   GetFrameFormat()->GetFrameSize().GetWidth() / 2)
                                  > rParam.nSide) ))
                             : (nDist + nWidth / 2 ) > rParam.nSide
                )
            {
                rParam.bAnyBoxFnd = true;
                SwTwips nDiff;
                if( TableChgMode::FixedWidthChangeProp == rParam.nMode )        // Table fixed, proportional
                {
                    // calculate relatively
                    nDiff = nWidth;
                    nDiff *= rParam.nDiff;
                    nDiff /= rParam.nRemainWidth;

                    if( nWidth < nDiff || nWidth - nDiff < MINLAY )
                        return false;
                }
                else
                {
                    nDiff = rParam.nDiff;

                    // See if the left or right Box is big enough to give up space.
                    // We're inserting a Box before or after.
                    SwTwips nTmpWidth = nWidth;
                    if( rParam.bLeft && pBox->GetUpper()->GetUpper() )
                    {
                        const SwTableBox* pTmpBox = pBox;
                        sal_uInt16 nBoxPos = n;
                        while( !nBoxPos && pTmpBox->GetUpper()->GetUpper() )
                        {
                            pTmpBox = pTmpBox->GetUpper()->GetUpper();
                            nBoxPos = pTmpBox->GetUpper()->GetBoxPos( pTmpBox );
                        }
                        nTmpWidth = pTmpBox->GetFrameFormat()->GetFrameSize().GetWidth();
                    }

                    if( nTmpWidth < nDiff || nTmpWidth - nDiff < MINLAY )
                        return false;
                    break;
                }
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( auto pLn : pBox->GetTabLines() )
            {
                rParam.nLowerDiff = 0;
                lcl_InsOtherBox( pLn, rParam, nDist, false );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff ||
                (rParam.bLeft ? ((nDist + nWidth / 2 ) <= rParam.nSide &&
                                (TableChgMode::FixedWidthChangeAbs != rParam.nMode ||
                                (n < rBoxes.size() &&
                                  (nDist + nWidth + rBoxes[ n+1 ]->
                                   GetFrameFormat()->GetFrameSize().GetWidth() / 2)
                                  > rParam.nSide) ))
                              : (nDist + nWidth / 2 ) > rParam.nSide ))
            {
                if( !nLowerDiff )
                {
                    if( TableChgMode::FixedWidthChangeProp == rParam.nMode )        // Table fixed, proportional
                    {
                        // Calculate relatively
                        nLowerDiff = nWidth;
                        nLowerDiff *= rParam.nDiff;
                        nLowerDiff /= rParam.nRemainWidth;
                    }
                    else
                        nLowerDiff = rParam.nDiff;
                }

                SwFormatFrameSize aNew( rSz );
                rParam.nLowerDiff += nLowerDiff;

                if( rParam.bBigger )
                    aNew.SetWidth( nWidth - nLowerDiff );
                else
                    aNew.SetWidth( nWidth + nLowerDiff );
                rParam.aShareFormats.SetSize( *pBox, aNew );

                if( TableChgMode::FixedWidthChangeAbs == rParam.nMode )
                    break;
            }
        }

        nDist += nWidth;
    }
    return true;
}

// The position comparison's result
//  SwComparePosition::Before,             // Box comes before
//  SwComparePosition::Behind,             // Box comes after
//  SwComparePosition::Inside,             // Box is completely within start/end
//  SwComparePosition::Outside,            // Box overlaps start/end completely
//  SwComparePosition::Equal,              // Box and start/end are the same
//  SwComparePosition::OverlapBefore,      // Box overlapps the start
//  SwComparePosition::OverlapBehind       // Box overlapps the end
SwComparePosition CheckBoxInRange( sal_uInt16 nStt, sal_uInt16 nEnd,
                                    sal_uInt16 nBoxStt, sal_uInt16 nBoxEnd )
{
    // Still treat COLFUZZY!
    SwComparePosition nRet;
    if( nBoxStt + COLFUZZY < nStt )
    {
        if( nBoxEnd > nStt + COLFUZZY )
        {
            if( nBoxEnd >= nEnd + COLFUZZY )
                nRet = SwComparePosition::Outside;
            else
                nRet = SwComparePosition::OverlapBefore;
        }
        else
            nRet = SwComparePosition::Before;
    }
    else if( nEnd > nBoxStt + COLFUZZY )
    {
        if( nEnd + COLFUZZY >= nBoxEnd )
        {
            if( COLFUZZY > std::abs( long(nEnd) - long(nBoxEnd) ) &&
                COLFUZZY > std::abs( long(nStt) - long(nBoxStt) ) )
                nRet = SwComparePosition::Equal;
            else
                nRet = SwComparePosition::Inside;
        }
        else
            nRet = SwComparePosition::OverlapBehind;
    }
    else
        nRet = SwComparePosition::Behind;

    return nRet;
}

static void lcl_DelSelBox_CorrLowers( SwTableLine& rLine, CR_SetBoxWidth& rParam,
                                SwTwips nWidth )
{
    // 1. step: Calculate own width
    SwTableBoxes& rBoxes = rLine.GetTabBoxes();
    SwTwips nBoxWidth = 0;

    for( auto n = rBoxes.size(); n; )
        nBoxWidth += rBoxes[ --n ]->GetFrameFormat()->GetFrameSize().GetWidth();

    if( COLFUZZY < std::abs( nWidth - nBoxWidth ))
    {
        // Thus, they need to be adjusted
        for( auto n = rBoxes.size(); n; )
        {
            SwTableBox* pBox = rBoxes[ --n ].get();
            SwFormatFrameSize aNew( pBox->GetFrameFormat()->GetFrameSize() );
            long nDiff = aNew.GetWidth();
            nDiff *= nWidth;
            nDiff /= nBoxWidth;
            aNew.SetWidth( nDiff );

            rParam.aShareFormats.SetSize( *pBox, aNew );

            if( !pBox->GetSttNd() )
            {
                // Has Lower itself, so also adjust that
                for( auto i = pBox->GetTabLines().size(); i; )
                    ::lcl_DelSelBox_CorrLowers( *pBox->GetTabLines()[ --i ],
                                                rParam, nDiff  );
            }
        }
    }
}

static void lcl_ChgBoxSize( SwTableBox& rBox, CR_SetBoxWidth& rParam,
                    const SwFormatFrameSize& rOldSz,
                    sal_uInt16& rDelWidth, SwTwips nDist )
{
    long nDiff = 0;
    bool bSetSize = false;

    switch( rParam.nMode )
    {
    case TableChgMode::FixedWidthChangeAbs:     // Fixed width table, change neighbor
        nDiff = rDelWidth + rParam.nLowerDiff;
        bSetSize = true;
        break;

    case TableChgMode::FixedWidthChangeProp:    // Fixed width table, change all neighbors
        if( !rParam.nRemainWidth )
        {
            // Calculate
            if( rParam.bLeft )
                rParam.nRemainWidth = sal_uInt16(nDist);
            else
                rParam.nRemainWidth = sal_uInt16(rParam.nTableWidth - nDist);
        }

        // Calculate relatively
        nDiff = rOldSz.GetWidth();
        nDiff *= rDelWidth + rParam.nLowerDiff;
        nDiff /= rParam.nRemainWidth;

        bSetSize = true;
        break;

    case TableChgMode::VarWidthChangeAbs:     // Variable table, change all neighbors
        if( COLFUZZY < std::abs( rParam.nBoxWidth -
                            ( rDelWidth + rParam.nLowerDiff )))
        {
            nDiff = rDelWidth + rParam.nLowerDiff - rParam.nBoxWidth;
            if( 0 < nDiff )
                rDelWidth = rDelWidth - sal_uInt16(nDiff);
            else
                rDelWidth = rDelWidth + sal_uInt16(-nDiff);
            bSetSize = true;
        }
        break;
    }

    if( bSetSize )
    {
        SwFormatFrameSize aNew( rOldSz );
        aNew.SetWidth( aNew.GetWidth() + nDiff );
        rParam.aShareFormats.SetSize( rBox, aNew );

        // Change the Lower once again
        for( auto i = rBox.GetTabLines().size(); i; )
            ::lcl_DelSelBox_CorrLowers( *rBox.GetTabLines()[ --i ], rParam,
                                            aNew.GetWidth() );
    }
}

static bool lcl_DeleteBox_Recursive( CR_SetBoxWidth& rParam, SwTableBox& rBox,
                            bool bCheck )
{
    bool bRet = true;
    if( rBox.GetSttNd() )
    {
        if( bCheck )
        {
            rParam.bAnyBoxFnd = true;
            if( rBox.GetFrameFormat()->GetProtect().IsContentProtected() )
                bRet = false;
            else
            {
                SwTableBox* pBox = &rBox;
                rParam.m_Boxes.insert(pBox);
            }
        }
        else
            ::DeleteBox_( rParam.pTableNd->GetTable(), &rBox,
                            rParam.pUndo, false, true, &rParam.aShareFormats );
    }
    else
    {
        // We need to delete these sequentially via the ContentBoxes
        for( auto i = rBox.GetTabLines().size(); i; )
        {
            SwTableLine& rLine = *rBox.GetTabLines()[ --i ];
            for( auto n = rLine.GetTabBoxes().size(); n; )
            {
                if (!::lcl_DeleteBox_Recursive( rParam,
                                *rLine.GetTabBoxes()[ --n ], bCheck ))
                {
                    return false;
                }
            }
        }
    }
    return bRet;
}

static bool lcl_DelSelBox( SwTableLine* pTabLine, CR_SetBoxWidth& rParam,
                    SwTwips nDist, bool bCheck )
{
    SwTableBoxes& rBoxes = pTabLine->GetTabBoxes();
    SwTableBoxes::size_type n;
    SwTableBoxes::size_type nCntEnd;

    sal_uInt16 nBoxChkStt, nBoxChkEnd, nDelWidth = 0;
    if( rParam.bLeft )
    {
        n = rBoxes.size();
        nCntEnd = 0;
        nBoxChkStt = static_cast<sal_uInt16>(rParam.nSide);
        nBoxChkEnd = static_cast<sal_uInt16>(rParam.nSide + rParam.nBoxWidth);
    }
    else
    {
        n = 0;
        nCntEnd = rBoxes.size();
        nBoxChkStt = static_cast<sal_uInt16>(rParam.nSide - rParam.nBoxWidth);
        nBoxChkEnd = static_cast<sal_uInt16>(rParam.nSide);
    }

    while( n != nCntEnd )
    {
        SwTableBox* pBox;
        if( rParam.bLeft )
            pBox = rBoxes[ --n ].get();
        else
            pBox = rBoxes[ n++ ].get();

        SwFrameFormat* pFormat = pBox->GetFrameFormat();
        const SwFormatFrameSize& rSz = pFormat->GetFrameSize();
        long nWidth = rSz.GetWidth();
        bool bDelBox = false, bChgLowers = false;

        // Test the Box width and react accordingly
        SwComparePosition ePosType = ::CheckBoxInRange(
                            nBoxChkStt, nBoxChkEnd,
                            sal_uInt16(rParam.bLeft ? nDist - nWidth : nDist),
                            sal_uInt16(rParam.bLeft ? nDist : nDist + nWidth));

        switch( ePosType )
        {
        case SwComparePosition::Before:
            if( bCheck )
            {
                if( rParam.bLeft )
                    return true;
            }
            else if( rParam.bLeft )
            {
                ::lcl_ChgBoxSize( *pBox, rParam, rSz, nDelWidth, nDist );
                if( TableChgMode::FixedWidthChangeAbs == rParam.nMode )
                    n = nCntEnd;
            }
            break;

        case SwComparePosition::Behind:
            if( bCheck )
            {
                if( !rParam.bLeft )
                    return true;
            }
            else if( !rParam.bLeft )
            {
                ::lcl_ChgBoxSize( *pBox, rParam, rSz, nDelWidth, nDist );
                if( TableChgMode::FixedWidthChangeAbs == rParam.nMode )
                    n = nCntEnd;
            }
            break;

        case SwComparePosition::Outside:           // Box fully overlaps start/end
        case SwComparePosition::Inside:            // Box is completely within start/end
        case SwComparePosition::Equal:             // Box and start/end are the same
            bDelBox = true;
            break;

        case SwComparePosition::OverlapBefore:     // Box overlaps the start
            if( nBoxChkStt <= ( nDist + (rParam.bLeft ? - nWidth / 2
                                                      : nWidth / 2 )))
            {
                if( !pBox->GetSttNd() )
                    bChgLowers = true;
                else
                    bDelBox = true;
            }
            else if( !bCheck && rParam.bLeft )
            {
                if( !pBox->GetSttNd() )
                    bChgLowers = true;
                else
                {
                    ::lcl_ChgBoxSize( *pBox, rParam, rSz, nDelWidth, nDist );
                    if( TableChgMode::FixedWidthChangeAbs == rParam.nMode )
                        n = nCntEnd;
                }
            }
            break;

        case SwComparePosition::OverlapBehind:     // Box overlaps the end
            // JP 10.02.99:
            // Delete generally or (like in OVERLAP_BEFORE) only delete the one who reaches up to the half into the delete Box?
            if( !pBox->GetSttNd() )
                bChgLowers = true;
            else
                bDelBox = true;
            break;
        default: break;
        }

        if( bDelBox )
        {
            nDelWidth = nDelWidth + sal_uInt16(nWidth);
            if( bCheck )
            {
                // The last/first Box can only be deleted for the variable Table,
                // if it's as large as the change in the Table.
                if( (( TableChgMode::VarWidthChangeAbs != rParam.nMode ||
                        nDelWidth != rParam.nBoxWidth ) &&
                     COLFUZZY > std::abs( rParam.bLeft
                                    ? nWidth - nDist
                                    : (nDist + nWidth - rParam.nTableWidth )))
                    || !::lcl_DeleteBox_Recursive(rParam, *pBox, bCheck))
                {
                    return false;
                }

                if( pFormat->GetProtect().IsContentProtected() )
                    return false;
            }
            else
            {
                ::lcl_DeleteBox_Recursive(rParam, *pBox, bCheck);

                if( !rParam.bLeft )
                {
                    --n;
                    --nCntEnd;
                }
            }
        }
        else if( bChgLowers )
        {
            bool bFirst = true, bCorrLowers = false;
            long nLowerDiff = 0;
            long nOldLower = rParam.nLowerDiff;
            sal_uInt16 nOldRemain = rParam.nRemainWidth;

            for( auto i = pBox->GetTabLines().size(); i; )
            {
                rParam.nLowerDiff = nDelWidth + nOldLower;
                rParam.nRemainWidth = nOldRemain;
                SwTableLine* pLine = pBox->GetTabLines()[ --i ];
                if( !::lcl_DelSelBox( pLine, rParam, nDist, bCheck ))
                    return false;

                // Do the Box and its Lines still exist?
                if( n < rBoxes.size() &&
                    pBox == rBoxes[ rParam.bLeft ? n : n-1 ].get() &&
                    i < pBox->GetTabLines().size() &&
                    pLine == pBox->GetTabLines()[ i ] )
                {
                    if( !bFirst && !bCorrLowers &&
                        COLFUZZY < std::abs( nLowerDiff - rParam.nLowerDiff ) )
                        bCorrLowers = true;

                    // The largest deletion width counts, but only if we don't
                    // delete the whole Line
                    if( nLowerDiff < rParam.nLowerDiff )
                        nLowerDiff = rParam.nLowerDiff;

                    bFirst = false;
                }
            }
            rParam.nLowerDiff = nOldLower;
            rParam.nRemainWidth = nOldRemain;

            // Did we delete all Boxes? Then the deletion width = the Box width, of course
            if( !nLowerDiff )
                nLowerDiff = nWidth;

            // Adjust deletion width!
            nDelWidth = nDelWidth + sal_uInt16(nLowerDiff);

            if( !bCheck )
            {
                // Has the Box already been removed?
                if( n > rBoxes.size() ||
                    pBox != rBoxes[ ( rParam.bLeft ? n : n-1 ) ].get() )
                {
                    // Then change the loop variable when deleting to the right
                    if( !rParam.bLeft )
                    {
                        --n;
                        --nCntEnd;
                    }
                }
                else
                {
                    // Or else we need to adapt the Box's size
                    SwFormatFrameSize aNew( rSz );
                    bool bCorrRel = false;

                    if( TableChgMode::VarWidthChangeAbs != rParam.nMode )
                    {
                        switch( ePosType )
                        {
                        case SwComparePosition::OverlapBefore:    // Box overlaps the start
                            if( TableChgMode::FixedWidthChangeProp == rParam.nMode )
                                bCorrRel = rParam.bLeft;
                            else if( rParam.bLeft ) // TableChgMode::FixedWidthChangeAbs
                            {
                                nLowerDiff = nLowerDiff - nDelWidth;
                                bCorrLowers = true;
                                n = nCntEnd;
                            }
                            break;

                        case SwComparePosition::OverlapBehind:    // Box overlaps the end
                            if( TableChgMode::FixedWidthChangeProp == rParam.nMode )
                                bCorrRel = !rParam.bLeft;
                            else if( !rParam.bLeft )    // TableChgMode::FixedWidthChangeAbs
                            {
                                nLowerDiff = nLowerDiff - nDelWidth;
                                bCorrLowers = true;
                                n = nCntEnd;
                            }
                            break;

                        default:
                            OSL_ENSURE( !pBox, "we should never reach this!" );
                            break;
                        }
                    }

                    if( bCorrRel )
                    {
                        if( !rParam.nRemainWidth )
                        {
                            // Calculate
                            if( rParam.bLeft )
                                rParam.nRemainWidth = sal_uInt16(nDist - nLowerDiff);
                            else
                                rParam.nRemainWidth = sal_uInt16(rParam.nTableWidth - nDist
                                                                - nLowerDiff );
                        }

                        long nDiff = aNew.GetWidth() - nLowerDiff;
                        nDiff *= nDelWidth + rParam.nLowerDiff;
                        nDiff /= rParam.nRemainWidth;

                        aNew.SetWidth( aNew.GetWidth() - nLowerDiff + nDiff );
                    }
                    else
                        aNew.SetWidth( aNew.GetWidth() - nLowerDiff );
                    rParam.aShareFormats.SetSize( *pBox, aNew );

                    if( bCorrLowers )
                    {
                        // Adapt the Lower once again
                        for( auto i = pBox->GetTabLines().size(); i; )
                            ::lcl_DelSelBox_CorrLowers( *pBox->
                                GetTabLines()[ --i ], rParam, aNew.GetWidth() );
                    }
                }
            }
        }

        if( rParam.bLeft )
            nDist -= nWidth;
        else
            nDist += nWidth;
    }
    rParam.nLowerDiff = nDelWidth;
    return true;
}

// Dummy function for the method SetColWidth
static bool lcl_DelOtherBox( SwTableLine* , CR_SetBoxWidth& , SwTwips , bool )
{
    return true;
}

static void lcl_AjustLines( SwTableLine* pLine, CR_SetBoxWidth& rParam )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( std::unique_ptr<SwTableBox> const & pBox : rBoxes )
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
    for (std::unique_ptr<SwTableBox> const & pBox : rBoxes)
    {
        const SwTwips nBoxW = pBox->GetFrameFormat()->GetFrameSize().GetWidth();
        nCurrentSize += nBoxW;

        for( auto pLn : pBox->GetTabLines() )
            CheckBoxWidth( *pLn, nBoxW );
    }

    if (sal::static_int_cast< unsigned long >(std::abs(nCurrentSize - nSize)) >
        (COLFUZZY * rBoxes.size()))
    {
        OSL_FAIL( "Line's Boxes are too small or too large" );
    }
}
#endif

static FndBox_* lcl_SaveInsDelData( CR_SetBoxWidth& rParam, SwUndo** ppUndo,
                                SwTableSortBoxes& rTmpLst, SwTwips nDistStt )
{
    // Find all Boxes/Lines
    SwTable& rTable = rParam.pTableNd->GetTable();

    if (rParam.m_Boxes.empty())
    {
        // Get the Boxes
        if( rParam.bBigger )
            for( auto pLn : rTable.GetTabLines() )
                ::lcl_DelSelBox( pLn, rParam, nDistStt, true );
        else
            for( auto pLn : rTable.GetTabLines() )
                ::lcl_InsSelBox( pLn, rParam, nDistStt, true );
    }

    // Prevent deleting the whole Table
    if (rParam.bBigger
        && rParam.m_Boxes.size() == rTable.GetTabSortBoxes().size())
    {
        return nullptr;
    }

    FndBox_* pFndBox = new FndBox_( nullptr, nullptr );
    if( rParam.bBigger )
        pFndBox->SetTableLines( rParam.m_Boxes, rTable );
    else
    {
        FndPara aPara(rParam.m_Boxes, pFndBox);
        ForEach_FndLineCopyCol( rTable.GetTabLines(), &aPara );
        OSL_ENSURE( pFndBox->GetLines().size(), "Where are the Boxes" );
        pFndBox->SetTableLines( rTable );

        if( ppUndo )
            rTmpLst.insert( rTable.GetTabSortBoxes() );
    }

    // Find Lines for the Layout update
    pFndBox->DelFrames( rTable );

    // TL_CHART2: this function get called from SetColWidth exclusively,
    // thus it is currently speculated that nothing needs to be done here.
    // Note: that SetColWidth is currently not completely understood though :-(

    return pFndBox;
}

bool SwTable::SetColWidth( SwTableBox& rCurrentBox, TableChgWidthHeightType eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, SwUndo** ppUndo )
{
    SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());    // Delete HTML Layout

    const SwFormatFrameSize& rSz = GetFrameFormat()->GetFrameSize();
    const SvxLRSpaceItem& rLR = GetFrameFormat()->GetLRSpace();

    std::unique_ptr<FndBox_> xFndBox;                // for insertion/deletion
    SwTableSortBoxes aTmpLst;       // for Undo
    bool bBigger,
        bRet = false,
        bLeft = TableChgWidthHeightType::ColLeft == extractPosition( eType ) ||
                TableChgWidthHeightType::CellLeft == extractPosition( eType ),
        bInsDel = bool(eType & TableChgWidthHeightType::InsertDeleteMode );
    sal_uLong nBoxIdx = rCurrentBox.GetSttIdx();

    // Get the current Box's edge
    // Only needed for manipulating the width
    const SwTwips nDist = ::lcl_GetDistance( &rCurrentBox, bLeft );
    SwTwips nDistStt = 0;
    CR_SetBoxWidth aParam( eType, nRelDiff, nDist, rSz.GetWidth(),
                            bLeft ? nDist : rSz.GetWidth() - nDist,
                            const_cast<SwTableNode*>(rCurrentBox.GetSttNd()->FindTableNode()) );
    bBigger = aParam.bBigger;

    FN_lcl_SetBoxWidth fnSelBox, fnOtherBox;
    if( bInsDel )
    {
        if( bBigger )
        {
            fnSelBox = lcl_DelSelBox;
            fnOtherBox = lcl_DelOtherBox;
            aParam.nBoxWidth = static_cast<sal_uInt16>(rCurrentBox.GetFrameFormat()->GetFrameSize().GetWidth());
            if( bLeft )
                nDistStt = rSz.GetWidth();
        }
        else
        {
            fnSelBox = lcl_InsSelBox;
            fnOtherBox = lcl_InsOtherBox;
        }
    }
    else
    {
        fnSelBox = lcl_SetSelBoxWidth;
        fnOtherBox = lcl_SetOtherBoxWidth;
    }

    switch( extractPosition(eType) )
    {
    case TableChgWidthHeightType::ColRight:
    case TableChgWidthHeightType::ColLeft:
        if( TableChgMode::VarWidthChangeAbs == m_eTableChgMode )
        {
            if( bInsDel )
                bBigger = !bBigger;

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

                if( !bRet && bInsDel &&
                    // Is the room on the other side?
                    ( bLeft ? rLR.GetRight() >= nAbsDiff
                            : rLR.GetLeft() >= nAbsDiff ))
                {
                    bRet = true; bLeft = !bLeft;
                }

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
                if( bInsDel )
                {
                    xFndBox.reset(::lcl_SaveInsDelData( aParam, ppUndo,
                                                    aTmpLst, nDistStt));
                    if (aParam.bBigger &&
                        aParam.m_Boxes.size() == m_TabSortContentBoxes.size())
                    {
                        // This whole Table is to be deleted!
                        GetFrameFormat()->GetDoc()->DeleteRowCol(aParam.m_Boxes);
                        return false;
                    }

                    if( ppUndo )
                        *ppUndo = aParam.CreateUndo(
                                        aParam.bBigger ? SwUndoId::COL_DELETE
                                                       : SwUndoId::TABLE_INSCOL );
                }
                else if( ppUndo )
                    *ppUndo = new SwUndoAttrTable( *aParam.pTableNd, true );

                long nFrameWidth = LONG_MAX;
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
                                        0, aSz.GetWidth(), aSz.GetWidth(), aParam.pTableNd );
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
                aParam.nTableWidth = sal_uInt16( aSz.GetWidth() );

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
        else if( bInsDel ||
                ( bLeft ? nDist != 0 : std::abs( rSz.GetWidth() - nDist ) > COLFUZZY ) )
        {
            bRet = true;
            if( bLeft && TableChgMode::FixedWidthChangeAbs == m_eTableChgMode && !bInsDel )
                aParam.bBigger = !bBigger;

            // First test if we have room at all
            if( bInsDel )
            {
                if( aParam.bBigger )
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
                else
                {
                    bRet = bLeft ? nDist != 0 : ( rSz.GetWidth() - nDist ) > COLFUZZY;
                    if( bRet )
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
                        if( bRet && !aParam.bAnyBoxFnd )
                            bRet = false;
                    }

                    if( !bRet && rCurrentBox.GetFrameFormat()->GetFrameSize().GetWidth()
                        - nRelDiff > COLFUZZY +
                            ( 567 / 2 /*leave room for at least 0.5 cm*/) )
                    {
                        // Consume the space from the current Cell
                        aParam.bSplittBox = true;
                        // We also need to test this!
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
                }
            }
            else if( aParam.bBigger )
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
                if( bInsDel )
                {
                    aParam1.bBigger = !aParam.bBigger;
                    xFndBox.reset(::lcl_SaveInsDelData(aParam, ppUndo, aTmpLst, nDistStt));
                    if( ppUndo )
                        *ppUndo = aParam.CreateUndo(
                                        aParam.bBigger ? SwUndoId::TABLE_DELBOX
                                                       : SwUndoId::TABLE_INSCOL );
                }
                else if( ppUndo )
                    *ppUndo = new SwUndoAttrTable( *aParam.pTableNd, true );

                if( bInsDel
                    ? ( TableChgMode::FixedWidthChangeAbs == m_eTableChgMode ? (bBigger && bLeft) : bLeft )
                    : ( TableChgMode::FixedWidthChangeAbs != m_eTableChgMode && bLeft ) )
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
        else if( bInsDel || ( bLeft ? nDist != 0
                                    : (rSz.GetWidth() - nDist) > COLFUZZY ))
        {
            if( bLeft && TableChgMode::FixedWidthChangeAbs == m_eTableChgMode && !bInsDel )
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
            if( bInsDel )
            {
                bRet = bLeft ? nDist != 0 : ( rSz.GetWidth() - nDist ) > COLFUZZY;
                if( bRet && !aParam.bBigger )
                {
                    bRet = (*fnOtherBox)( pLine, aParam, 0, true );
                    if( bRet && !aParam.bAnyBoxFnd )
                        bRet = false;
                }

                if( !bRet && !aParam.bBigger && rCurrentBox.GetFrameFormat()->
                    GetFrameSize().GetWidth() - nRelDiff > COLFUZZY +
                        ( 567 / 2 /*leave room for at least 0.5 cm*/) )
                {
                    // Consume the room from the current Cell
                    aParam.bSplittBox = true;
                    bRet = true;
                }
            }
            else
            {
                FN_lcl_SetBoxWidth fnTmp = aParam.bBigger ? fnOtherBox : fnSelBox;
                bRet = (*fnTmp)( pLine, aParam, nDistStt, true );
            }

            // If true, set it
            if( bRet )
            {
                CR_SetBoxWidth aParam1( aParam );
                if( bInsDel )
                {
                    aParam1.bBigger = !aParam.bBigger;
                    xFndBox.reset(::lcl_SaveInsDelData(aParam, ppUndo, aTmpLst, nDistStt));
                    if( ppUndo )
                        *ppUndo = aParam.CreateUndo(
                                        aParam.bBigger ? SwUndoId::TABLE_DELBOX
                                                       : SwUndoId::TABLE_INSCOL );
                }
                else if( ppUndo )
                    *ppUndo = new SwUndoAttrTable( *aParam.pTableNd, true );

                if( bInsDel
                    ? ( TableChgMode::FixedWidthChangeAbs == m_eTableChgMode ? (bBigger && bLeft) : bLeft )
                    : ( TableChgMode::FixedWidthChangeAbs != m_eTableChgMode && bLeft ) )
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

        if (ppUndo && *ppUndo && aParam.pUndo)
        {
            aParam.pUndo->SetColWidthParam( nBoxIdx, static_cast<sal_uInt16>(m_eTableChgMode), eType,
                                            nAbsDiff, nRelDiff );
            if( !aParam.bBigger )
                aParam.pUndo->SaveNewBoxes( *aParam.pTableNd, aTmpLst );
        }
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

static FndBox_* lcl_SaveInsDelData( CR_SetLineHeight& rParam, SwUndo** ppUndo,
                                SwTableSortBoxes& rTmpLst )
{
    // Find all Boxes/Lines
    SwTable& rTable = rParam.pTableNd->GetTable();

    OSL_ENSURE( !rParam.m_Boxes.empty(), "We can't go on without Boxes!" );

    // Prevent deleting the whole Table
    if (!rParam.bBigger
        && rParam.m_Boxes.size() == rTable.GetTabSortBoxes().size())
    {
        return nullptr;
    }

    FndBox_* pFndBox = new FndBox_( nullptr, nullptr );
    if( !rParam.bBigger )
        pFndBox->SetTableLines( rParam.m_Boxes, rTable );
    else
    {
        FndPara aPara(rParam.m_Boxes, pFndBox);
        ForEach_FndLineCopyCol( rTable.GetTabLines(), &aPara );
        OSL_ENSURE( pFndBox->GetLines().size(), "Where are the Boxes?" );
        pFndBox->SetTableLines( rTable );

        if( ppUndo )
            rTmpLst.insert( rTable.GetTabSortBoxes() );
    }

    // Find Lines for the Layout update
    pFndBox->DelFrames( rTable );

    // TL_CHART2: it is currently unclear if sth has to be done here.

    return pFndBox;
}

void SetLineHeight( SwTableLine& rLine, SwTwips nOldHeight, SwTwips nNewHeight,
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
        nMyNewH = long(aTmp);
    }

    SwFrameSize eSize = ATT_MIN_SIZE;
    if( !bMinSize &&
        ( nMyOldH - nMyNewH ) > ( CalcRowRstHeight( pLineFrame ) + ROWFUZZY ))
        eSize = ATT_FIX_SIZE;

    pFormat->SetFormatAttr( SwFormatFrameSize( eSize, 0, nMyNewH ) );

    // First adapt all internal ones
    for( std::unique_ptr<SwTableBox> const & pBox : rLine.GetTabBoxes() )
    {
        for( auto pLine : pBox->GetTabLines() )
            SetLineHeight( *pLine, nMyOldH, nMyNewH, bMinSize );
    }
}

static bool lcl_SetSelLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
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

static bool lcl_SetOtherLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
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

static bool lcl_InsDelSelLine( SwTableLine* pLine, CR_SetLineHeight& rParam,
                            SwTwips nDist, bool bCheck )
{
    if( !bCheck )
    {
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        SwDoc* pDoc = pLine->GetFrameFormat()->GetDoc();
        if( !rParam.bBigger )
        {
            for (size_t n = rBoxes.size(); n; )
            {
                ::lcl_SaveUpperLowerBorder( rParam.pTableNd->GetTable(),
                                                    *rBoxes[ --n ],
                                                    rParam.aShareFormats );
            }
            for (size_t n = rBoxes.size(); n; )
            {
                ::DeleteBox_( rParam.pTableNd->GetTable(),
                                    rBoxes[ --n ].get(), rParam.pUndo, false,
                                    false, &rParam.aShareFormats );
            }
        }
        else
        {
            // Insert Line
            SwTableLine* pNewLine = new SwTableLine( static_cast<SwTableLineFormat*>(pLine->GetFrameFormat()),
                                        rBoxes.size(), pLine->GetUpper() );
            SwTableLines* pLines;
            if( pLine->GetUpper() )
                pLines = &pLine->GetUpper()->GetTabLines();
            else
                pLines = &rParam.pTableNd->GetTable().GetTabLines();
            sal_uInt16 nPos = pLines->GetPos( pLine );
            if( !rParam.bTop )
                ++nPos;
            pLines->insert( pLines->begin() + nPos, pNewLine );

            SwFrameFormat* pNewFormat = pNewLine->ClaimFrameFormat();
            pNewFormat->SetFormatAttr( SwFormatFrameSize( ATT_MIN_SIZE, 0, nDist ) );

            // And once again calculate the Box count
            SwTableBoxes& rNewBoxes = pNewLine->GetTabBoxes();
            for( SwTableBoxes::size_type n = 0; n < rBoxes.size(); ++n )
            {
                SwTwips nWidth = 0;
                SwTableBox* pOld = rBoxes[ n ].get();
                if( !pOld->GetSttNd() )
                {
                    // Not a normal content Box, so fall back to the 1st next Box
                    nWidth = pOld->GetFrameFormat()->GetFrameSize().GetWidth();
                    while( !pOld->GetSttNd() )
                        pOld = pOld->GetTabLines()[ 0 ]->GetTabBoxes()[ 0 ].get();
                }
                ::InsTableBox( pDoc, rParam.pTableNd, pNewLine,
                                    static_cast<SwTableBoxFormat*>(pOld->GetFrameFormat()), pOld, n );

                // Special treatment for the border:
                // The top one needs to be removed
                const SvxBoxItem& rBoxItem = pOld->GetFrameFormat()->GetBox();
                if( rBoxItem.GetTop() )
                {
                    SvxBoxItem aTmp( rBoxItem );
                    aTmp.SetLine( nullptr, SvxBoxItemLine::TOP );
                    rParam.aShareFormats.SetAttr( rParam.bTop
                                                ? *pOld
                                                : *rNewBoxes[ n ], aTmp );
                }

                if( nWidth )
                    rParam.aShareFormats.SetAttr( *rNewBoxes[ n ],
                                SwFormatFrameSize( ATT_FIX_SIZE, nWidth, 0 ) );
            }
        }
    }
    else
    {
        // Collect Boxes!
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for( auto n = rBoxes.size(); n; )
        {
            SwTableBox* pBox = rBoxes[ --n ].get();
            if( pBox->GetFrameFormat()->GetProtect().IsContentProtected() )
                return false;

            if( pBox->GetSttNd() )
            {
                rParam.m_Boxes.insert(pBox);
            }
            else
            {
                for( auto i = pBox->GetTabLines().size(); i; )
                    lcl_InsDelSelLine( pBox->GetTabLines()[ --i ],
                                        rParam, 0, true );
            }
        }
    }
    return true;
}

bool SwTable::SetRowHeight( SwTableBox& rCurrentBox, TableChgWidthHeightType eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff,SwUndo** ppUndo )
{
    SwTableLine* pLine = rCurrentBox.GetUpper();

    SwTableLine* pBaseLine = pLine;
    while( pBaseLine->GetUpper() )
        pBaseLine = pBaseLine->GetUpper()->GetUpper();

    std::unique_ptr<FndBox_> xFndBox;                // for insertion/deletion
    SwTableSortBoxes aTmpLst;       // for Undo
    bool bBigger,
        bRet = false,
        bTop = TableChgWidthHeightType::CellTop == extractPosition( eType ),
        bInsDel = bool(eType & TableChgWidthHeightType::InsertDeleteMode );
    sal_uInt16 nBaseLinePos = GetTabLines().GetPos( pBaseLine );
    sal_uLong nBoxIdx = rCurrentBox.GetSttIdx();

    CR_SetLineHeight aParam( eType,
                        const_cast<SwTableNode*>(rCurrentBox.GetSttNd()->FindTableNode()) );
    bBigger = aParam.bBigger;

    FN_lcl_SetLineHeight fnSelLine, fnOtherLine = lcl_SetOtherLineHeight;
    if( bInsDel )
        fnSelLine = lcl_InsDelSelLine;
    else
        fnSelLine = lcl_SetSelLineHeight;

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
        pBaseLine = pLine;
        SAL_FALLTHROUGH;

    case TableChgWidthHeightType::RowBottom:
        {
            if( bInsDel && !bBigger )       // By how much does it get higher?
            {
                nAbsDiff = GetRowFrame( *pBaseLine )->getFrameArea().Height();
            }

            if( TableChgMode::VarWidthChangeAbs == m_eTableChgMode )
            {
                // First test if we have room at all
                if( bBigger )
                    bRet = true;
                else
                    bRet = (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, true );

                if( bRet )
                {
                    if( bInsDel )
                    {
                        if (aParam.m_Boxes.empty())
                        {
                            ::lcl_InsDelSelLine( (*pLines)[ nBaseLinePos ],
                                                    aParam, 0, true );
                        }

                        xFndBox.reset(::lcl_SaveInsDelData( aParam, ppUndo, aTmpLst ));

                        // delete complete table when last row is deleted
                        if( !bBigger &&
                            aParam.m_Boxes.size() == m_TabSortContentBoxes.size())
                        {
                            GetFrameFormat()->GetDoc()->DeleteRowCol(aParam.m_Boxes);
                            return false;
                        }

                        if( ppUndo )
                            *ppUndo = aParam.CreateUndo(
                                        bBigger ? SwUndoId::TABLE_INSROW
                                                : SwUndoId::ROW_DELETE );
                    }
                    else if( ppUndo )
                        *ppUndo = new SwUndoAttrTable( *aParam.pTableNd, true );

                    (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
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
                            if( !(*fnOtherLine)( (*pLines)[ n ], aParam,
                                                    nAbsDiff, true ))
                            {
                                bRet = false;
                                break;
                            }
                        }
                    }
                    else
                        bRet = (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                                nAbsDiff, true );
                }

                if( bRet )
                {
                    // Adjust
                    if( bInsDel )
                    {
                        if (aParam.m_Boxes.empty())
                        {
                            ::lcl_InsDelSelLine( (*pLines)[ nBaseLinePos ],
                                                    aParam, 0, true );
                        }
                        xFndBox.reset(::lcl_SaveInsDelData( aParam, ppUndo, aTmpLst ));
                        if( ppUndo )
                            *ppUndo = aParam.CreateUndo(
                                        bBigger ? SwUndoId::TABLE_INSROW
                                                : SwUndoId::ROW_DELETE );
                    }
                    else if( ppUndo )
                        *ppUndo = new SwUndoAttrTable( *aParam.pTableNd, true );

                    CR_SetLineHeight aParam1( aParam );

                    if( bTop )
                    {
                        (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, false );
                        for( auto n = nStt; n < nEnd; ++n )
                            (*fnOtherLine)( (*pLines)[ n ], aParam1,
                                            nAbsDiff, false );
                    }
                    else
                    {
                        for( auto n = nStt; n < nEnd; ++n )
                            (*fnOtherLine)( (*pLines)[ n ], aParam1,
                                            nAbsDiff, false );
                        (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
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

        if (ppUndo && *ppUndo && aParam.pUndo)
        {
            aParam.pUndo->SetColWidthParam( nBoxIdx, static_cast<sal_uInt16>(m_eTableChgMode), eType,
                                            nAbsDiff, nRelDiff );
            if( bBigger )
                aParam.pUndo->SaveNewBoxes( *aParam.pTableNd, aTmpLst );
        }
    }

    CHECKTABLELAYOUT

    return bRet;
}

SwFrameFormat* SwShareBoxFormat::GetFormat( long nWidth ) const
{
    SwFrameFormat *pRet = nullptr, *pTmp;
    for( auto n = aNewFormats.size(); n; )
        if( ( pTmp = aNewFormats[ --n ])->GetFrameSize().GetWidth()
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
    const SfxPoolItem& rFrameSz = pOldFormat->GetFormatAttr( RES_FRM_SIZE, false );
    for( auto n = aNewFormats.size(); n; )
        if( SfxItemState::SET == ( pTmp = aNewFormats[ --n ])->
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
    aNewFormats.push_back( &rNew );
}

bool SwShareBoxFormat::RemoveFormat( const SwFrameFormat& rFormat )
{
    // returns true, if we can delete
    if( pOldFormat == &rFormat )
        return true;

    std::vector<SwFrameFormat*>::iterator it = std::find( aNewFormats.begin(), aNewFormats.end(), &rFormat );
    if( aNewFormats.end() != it )
        aNewFormats.erase( it );
    return aNewFormats.empty();
}

SwShareBoxFormats::~SwShareBoxFormats()
{
}

SwFrameFormat* SwShareBoxFormats::GetFormat( const SwFrameFormat& rFormat, long nWidth ) const
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
