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
#include <vector>

#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>

#include <tools/stream.hxx>
#include <svtools/rtftoken.h>

#include <editeng/eeitem.hxx>
#include <svx/svdetc.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/outlobj.hxx>

#include <cell.hxx>
#include <celltypes.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editdata.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/svxrtf.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

namespace sdr { namespace table {

struct RTFCellDefault
{
    SfxItemSet          maItemSet;
    sal_Int32           mnRowSpan;
    sal_Int32           mnColSpan;   // MergeCell if >1, merged cells if 0
    sal_Int32           mnCellX;

    explicit RTFCellDefault( SfxItemPool* pPool ) : maItemSet( *pPool ), mnRowSpan(1), mnColSpan(1), mnCellX(0) {}
};

typedef std::vector< std::shared_ptr< RTFCellDefault > > RTFCellDefaultVector;

struct RTFCellInfo
{
    SfxItemSet          maItemSet;
    sal_Int32           mnStartPara;
    sal_Int32           mnParaCount;
    sal_Int32           mnCellX;
    sal_Int32           mnRowSpan;
    std::shared_ptr< RTFCellInfo > mxVMergeCell;

    explicit RTFCellInfo( SfxItemPool& rPool ) : maItemSet(  rPool ), mnStartPara(0), mnParaCount(0), mnCellX(0), mnRowSpan(1), mxVMergeCell(nullptr) {}
};

typedef std::shared_ptr< RTFCellInfo > RTFCellInfoPtr;
typedef std::vector< RTFCellInfoPtr > RTFColumnVector;

typedef std::shared_ptr< RTFColumnVector > RTFColumnVectorPtr;

class SdrTableRTFParser
{
public:
    explicit SdrTableRTFParser( SdrTableObj& rTableObj );

    void Read( SvStream& rStream );

    void ProcToken( RtfImportInfo* pInfo );

    void NextRow();
    void NextColumn();
    void NewCellRow();

    void InsertCell( RtfImportInfo const * pInfo );
    void InsertColumnEdge( sal_Int32 nEdge );

    void FillTable();

    DECL_LINK( RTFImportHdl, RtfImportInfo&, void );

private:
    SdrTableObj&    mrTableObj;
    std::unique_ptr<SdrOutliner> mpOutliner;
    SfxItemPool&    mrItemPool;

    RTFCellDefaultVector maDefaultList;
    RTFCellDefaultVector::iterator maDefaultIterator;

    int             mnLastToken;
    bool            mbNewDef;

    sal_Int32       mnStartPara;

    sal_Int32       mnRowCnt;
    sal_Int32       mnLastEdge;
    sal_Int32       mnVMergeIdx;

    std::vector< sal_Int32 > maColumnEdges;
    std::vector< sal_Int32 >::iterator maLastEdge;
    std::vector< RTFColumnVectorPtr > maRows;

    std::unique_ptr<RTFCellDefault> mpInsDefault;
    RTFCellDefault* mpActDefault;
    RTFCellDefault* mpDefMerge;

    Reference< XTable > mxTable;

    RTFColumnVectorPtr mxLastRow;
    // Copy assignment is forbidden and not implemented.
    SdrTableRTFParser (const SdrTableRTFParser &) = delete;
    SdrTableRTFParser & operator= (const SdrTableRTFParser &) = delete;
};

SdrTableRTFParser::SdrTableRTFParser( SdrTableObj& rTableObj )
: mrTableObj( rTableObj )
, mpOutliner( SdrMakeOutliner( OutlinerMode::TextObject, rTableObj.getSdrModelFromSdrObject() ) )
, mrItemPool( rTableObj.getSdrModelFromSdrObject().GetItemPool() )
, mnLastToken( 0 )
, mbNewDef( false )
, mnStartPara( 0 )
, mnRowCnt( 0 )
, mnLastEdge( 0 )
, mnVMergeIdx ( 0 )
, mpActDefault( nullptr )
, mpDefMerge( nullptr )
, mxTable( rTableObj.getTable() )
, mxLastRow( nullptr )
{
    mpOutliner->SetUpdateMode(true);
    mpOutliner->SetStyleSheet( 0, mrTableObj.GetStyleSheet() );
    mpInsDefault.reset( new RTFCellDefault( &mrItemPool ) );
}

void SdrTableRTFParser::Read( SvStream& rStream )
{
    EditEngine& rEdit = const_cast< EditEngine& >( mpOutliner->GetEditEngine() );

    Link<RtfImportInfo&,void> aOldLink( rEdit.GetRtfImportHdl() );
    rEdit.SetRtfImportHdl( LINK( this, SdrTableRTFParser, RTFImportHdl ) );
    mpOutliner->Read( rStream, OUString(), EETextFormat::Rtf );
    rEdit.SetRtfImportHdl( aOldLink );

    FillTable();
}

IMPL_LINK( SdrTableRTFParser, RTFImportHdl, RtfImportInfo&, rInfo, void )
{
    switch ( rInfo.eState )
    {
        case RtfImportState::NextToken:
            ProcToken( &rInfo );
            break;
        case RtfImportState::UnknownAttr:
            ProcToken( &rInfo );
            break;
        case RtfImportState::Start:
        {
            SvxRTFParser* pParser = static_cast<SvxRTFParser*>(rInfo.pParser);
            pParser->SetAttrPool( &mrItemPool );
            RTFPardAttrMapIds& rMap = pParser->GetPardMap();
            rMap.nBox = SDRATTR_TABLE_BORDER;
        }
            break;
        case RtfImportState::End:
            if ( rInfo.aSelection.nEndPos )
            {
                mpActDefault = nullptr;
                rInfo.nToken = RTF_PAR;
                rInfo.aSelection.nEndPara++;
                ProcToken( &rInfo );
            }
            break;
        case RtfImportState::SetAttr:
        case RtfImportState::InsertText:
        case RtfImportState::InsertPara:
            break;
        default:
            SAL_WARN( "svx.table","unknown ImportInfo.eState");
    }
}

void SdrTableRTFParser::NextRow()
{
    mxLastRow = maRows.back();
    mnVMergeIdx = 0;
    ++mnRowCnt;
}

void SdrTableRTFParser::InsertCell( RtfImportInfo const * pInfo )
{

    RTFCellInfoPtr xCellInfo( new RTFCellInfo(mrItemPool) );

    xCellInfo->mnStartPara = mnStartPara;
    xCellInfo->mnParaCount = pInfo->aSelection.nEndPara - 1 - mnStartPara;
    xCellInfo->mnCellX = mpActDefault->mnCellX;
    xCellInfo->mnRowSpan = mpActDefault->mnRowSpan;


    if ( mxLastRow != nullptr )
    {
        sal_Int32 nSize = mxLastRow->size();
        while( mnVMergeIdx < nSize &&
             (*mxLastRow)[mnVMergeIdx]->mnCellX < xCellInfo->mnCellX )
            ++mnVMergeIdx;

        if ( xCellInfo->mnRowSpan == 0 && mnVMergeIdx < nSize )
        {
            RTFCellInfoPtr xLastCell( (*mxLastRow)[mnVMergeIdx] );
            if (xLastCell->mnRowSpan)
                xCellInfo->mxVMergeCell = xLastCell;
            else
                xCellInfo->mxVMergeCell = xLastCell->mxVMergeCell;
        }
    }

    if( !maRows.empty() )
    {
        RTFColumnVectorPtr xColumn( maRows.back() );
        if ( xCellInfo->mxVMergeCell )
        {
            if ( xColumn->size()==0 ||
                    xColumn->back()->mxVMergeCell != xCellInfo->mxVMergeCell )
                xCellInfo->mxVMergeCell->mnRowSpan++;
        }

        xColumn->push_back( xCellInfo );
    }

    mnStartPara = pInfo->aSelection.nEndPara - 1;
}

void SdrTableRTFParser::InsertColumnEdge( sal_Int32 nEdge )
{
    auto aNextEdge = std::lower_bound( maLastEdge, maColumnEdges.end(), nEdge );

    if ( aNextEdge == maColumnEdges.end() || nEdge != *aNextEdge )
    {
        maLastEdge = maColumnEdges.insert( aNextEdge , nEdge );
        mnLastEdge = nEdge;
    }
}

void SdrTableRTFParser::FillTable()
{
    try
    {
        sal_Int32 nColCount = mxTable->getColumnCount();
        Reference< XTableColumns > xCols( mxTable->getColumns(), UNO_QUERY_THROW );
        sal_Int32 nColMax = maColumnEdges.size();
        if( nColCount < nColMax )
        {
            xCols->insertByIndex( nColCount, nColMax - nColCount );
            nColCount = mxTable->getColumnCount();
        }

        const OUString sWidth("Width");
        sal_Int32 nCol, nLastEdge = 0;
        for( nCol = 0; nCol < nColCount; nCol++ )
        {
            Reference< XPropertySet > xSet( xCols->getByIndex( nCol ), UNO_QUERY_THROW );
            sal_Int32 nWidth = maColumnEdges[nCol] - nLastEdge;

            xSet->setPropertyValue( sWidth, Any( nWidth ) );
            nLastEdge += nWidth;
        }

        const sal_Int32 nRowCount = mxTable->getRowCount();
        if( nRowCount < mnRowCnt )
        {
            Reference< XTableRows > xRows( mxTable->getRows(), UNO_QUERY_THROW );
            xRows->insertByIndex( nRowCount, mnRowCnt - nRowCount );
        }

        for( sal_Int32 nRow = 0; nRow < static_cast<sal_Int32>(maRows.size()); nRow++ )
        {
            RTFColumnVectorPtr xColumn( maRows[nRow] );
            nCol = 0;
            auto aEdge = maColumnEdges.begin();
            for( sal_Int32 nIdx = 0; nCol < nColMax && nIdx < static_cast<sal_Int32>(xColumn->size()); nIdx++ )
            {
                RTFCellInfoPtr xCellInfo( (*xColumn)[nIdx] );

                CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                if( xCell.is() && xCellInfo.get() )
                {
                    const SfxPoolItem *pPoolItem = nullptr;
                    if( xCellInfo->maItemSet.GetItemState(SDRATTR_TABLE_BORDER,false,&pPoolItem)==SfxItemState::SET)
                        xCell->SetMergedItem( *pPoolItem );

                    std::unique_ptr<OutlinerParaObject> pTextObject(mpOutliner->CreateParaObject( xCellInfo->mnStartPara, xCellInfo->mnParaCount ));
                    if( pTextObject )
                    {
                        SdrOutliner& rOutliner=mrTableObj.ImpGetDrawOutliner();
                        rOutliner.SetUpdateMode(true);
                        rOutliner.SetText( *pTextObject );
                        mrTableObj.NbcSetOutlinerParaObjectForText( rOutliner.CreateParaObject(), xCell.get() );
                    }

                    sal_Int32 nLastRow = nRow;
                    if ( xCellInfo->mnRowSpan )
                        nLastRow += xCellInfo->mnRowSpan - 1;

                    aEdge = std::lower_bound( aEdge, maColumnEdges.end(), xCellInfo->mnCellX );
                    sal_Int32 nLastCol = nCol;
                    if ( aEdge != maColumnEdges.end() )
                    {
                        nLastCol = std::distance( maColumnEdges.begin(), aEdge);
                        ++aEdge;
                    }

                    if ( nLastCol > nCol || nLastRow > nRow )
                    {
                         Reference< XMergeableCellRange > xRange( mxTable->createCursorByRange( mxTable->getCellRangeByPosition( nCol, nRow, nLastCol, nLastRow ) ), UNO_QUERY_THROW );
                         if( xRange->isMergeable() )
                             xRange->merge();
                    }
                    nCol = nLastCol + 1;
                }
            }
        }

        tools::Rectangle aRect( mrTableObj.GetSnapRect() );
        aRect.SetRight( aRect.Left() + nLastEdge );
        mrTableObj.NbcSetSnapRect( aRect );

    }
    catch( Exception& )
    {
        OSL_FAIL("sdr::table::SdrTableRTFParser::InsertCell(), exception caught!" );
    }
}

void SdrTableRTFParser::NewCellRow()
{
    if( mbNewDef )
    {
        mbNewDef = false;

        maRows.push_back( std::make_shared<std::vector<std::shared_ptr<RTFCellInfo>>>( ) );
    }
    mpDefMerge = nullptr;
    maDefaultIterator = maDefaultList.begin();

    NextColumn();

    DBG_ASSERT( mpActDefault, "NewCellRow: pActDefault==0" );
}

void SdrTableRTFParser::NextColumn()
{
    if( maDefaultIterator != maDefaultList.end() )
        mpActDefault = (*maDefaultIterator++).get();
    else
        mpActDefault = nullptr;
}

long TwipsToHundMM( long nIn )
{
    long nRet = OutputDevice::LogicToLogic( nIn, MapUnit::MapTwip, MapUnit::Map100thMM );
    return nRet;
}

void SdrTableRTFParser::ProcToken( RtfImportInfo* pInfo )
{
    switch ( pInfo->nToken )
    {
        case RTF_TROWD:         // denotes table row default, before RTF_CELLX
        {
            maDefaultList.clear();
            mpDefMerge = nullptr;
            mnLastToken = pInfo->nToken;
            maLastEdge = maColumnEdges.begin();
            mnLastEdge = 0;
        }
        break;
        case RTF_CLMGF:         // The first cell of cells to be merged
        {
            mpDefMerge = mpInsDefault.get();
            mnLastToken = pInfo->nToken;
        }
        break;
        case RTF_CLMRG:         // A cell to be merged with the preceding cell
        {
            if ( !mpDefMerge )
                mpDefMerge = maDefaultList.back().get();
            DBG_ASSERT( mpDefMerge, "RTF_CLMRG: pDefMerge==0" );
            if( mpDefMerge )
                mpDefMerge->mnColSpan++;
            mpInsDefault->mnColSpan = 0;
            mnLastToken = pInfo->nToken;
        }
        break;
        case RTF_CLVMGF:
        {
            mnLastToken = pInfo->nToken;
        }
        break;
        case RTF_CLVMRG:
        {
            mpInsDefault->mnRowSpan = 0;
            mnLastToken = pInfo->nToken;
        }
        break;
        case RTF_CELLX:         // closes cell default
        {
            mbNewDef = true;
            std::shared_ptr<RTFCellDefault> pDefault( mpInsDefault.release() );
            maDefaultList.push_back( pDefault );


            const sal_Int32 nSize = TwipsToHundMM( pInfo->nTokenValue );
            if ( nSize > mnLastEdge )
                InsertColumnEdge( nSize );

            pDefault->mnCellX = nSize;
            // Record cellx in the first merged cell.
            if ( mpDefMerge && pDefault->mnColSpan == 0 )
                mpDefMerge->mnCellX = nSize;

            mpInsDefault.reset( new RTFCellDefault( &mrItemPool ) );

            mnLastToken = pInfo->nToken;
        }
        break;
        case RTF_INTBL:         // before the first RTF_CELL
        {
            if ( mnLastToken != RTF_INTBL && mnLastToken != RTF_CELL && mnLastToken != RTF_PAR )
            {
                NewCellRow();
                mnLastToken = pInfo->nToken;
            }
        }
        break;
        case RTF_CELL:          // denotes the end of a cell.
        {
            DBG_ASSERT( mpActDefault, "RTF_CELL: pActDefault==0" );
            if ( mbNewDef || !mpActDefault )
                NewCellRow();
            if ( !mpActDefault )
                mpActDefault = mpInsDefault.get();
            if ( mpActDefault->mnColSpan > 0 )
            {
                InsertCell(pInfo);
            }
            NextColumn();
            mnLastToken = pInfo->nToken;
        }
        break;
        case RTF_ROW:           // means the end of a row
        {
            NextRow();
            mnLastToken = pInfo->nToken;
        }
        break;
        case RTF_PAR:           // Paragraph
            mnLastToken = pInfo->nToken;
            break;
        default:
        {   // do not set nLastToken
            switch ( pInfo->nToken & ~(0xff | RTF_TABLEDEF) )
            {
                case RTF_SHADINGDEF:
//                  ((SvxRTFParser*)pInfo->pParser)->ReadBackgroundAttr(pInfo->nToken, mpInsDefault->maItemSet, sal_True );
                break;
                case RTF_BRDRDEF:
                    static_cast<SvxRTFParser*>(pInfo->pParser)->ReadBorderAttr(pInfo->nToken, mpInsDefault->maItemSet, true );
                break;
            }
        }
    }
}

void SdrTableObj::ImportAsRTF( SvStream& rStream, SdrTableObj& rObj )
{
    SdrTableRTFParser aParser( rObj );
    aParser.Read( rStream );
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
