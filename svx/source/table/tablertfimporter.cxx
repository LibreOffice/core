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


#include <vector>
#include <boost/shared_ptr.hpp>

#include <com/sun/star/table/XTable.hpp>

#include <tools/stream.hxx>
#include <svtools/rtftoken.h>

#include <editeng/eeitem.hxx>
#include <svx/svdetc.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/outlobj.hxx>

#include "cell.hxx"
#include "celltypes.hxx"
#include "svx/svdotable.hxx"
#include "svx/svdoutl.hxx"
#include "editeng/editeng.hxx"
#include "editeng/editdata.hxx"
#include "svx/svdmodel.hxx"
#include "editeng/svxrtf.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

namespace sdr { namespace table {

struct RTFCellDefault
{
    SfxItemSet          maItemSet;
    sal_Int32           mnCol;
    sal_uInt16              mnTwips;         // right border of the cell
    sal_Int32           mnColSpan;   // MergeCell if >1, merged cells if 0

    RTFCellDefault( SfxItemPool* pPool ) : maItemSet( *pPool ), mnCol(0), mnTwips(0 ), mnColSpan(1) {}
};

typedef std::vector< boost::shared_ptr< RTFCellDefault > > RTFCellDefaultVector;

struct RTFCellInfo
{
    SfxItemSet          maItemSet;
    sal_Int32           mnStartPara;
    sal_Int32           mnParaCount;
    sal_Int32           mnColSpan;

    RTFCellInfo( SfxItemPool& rPool ) : maItemSet(  rPool ), mnStartPara(0), mnParaCount(0), mnColSpan(0) {}
};

typedef boost::shared_ptr< RTFCellInfo > RTFCellInfoPtr;
typedef std::vector< RTFCellInfoPtr > RTFColumnVector;

typedef boost::shared_ptr< RTFColumnVector > RTFColumnVectorPtr;

typedef std::vector< RTFColumnVectorPtr > RTFRowVector;

class SdrTableRTFParser
{
public:
    SdrTableRTFParser( SdrTableObj& rTableObj );
    ~SdrTableRTFParser();

    void Read( SvStream& rStream );

    void ProcToken( ImportInfo* pInfo );

    void NextRow();
    void NextColumn();
    void NewCellRow();

    void InsertCell( ImportInfo* pInfo );

    void FillTable();

    DECL_LINK( RTFImportHdl, ImportInfo* );

private:
    SdrTableObj&    mrTableObj;
    SdrOutliner*    mpOutliner;
    SfxItemPool&    mrItemPool;

    RTFCellDefaultVector maDefaultList;
    RTFCellDefaultVector::iterator maDefaultIterator;

    int             mnLastToken;
    bool            mbNewDef;

    sal_Int32       mnStartPara;

    sal_Int32       mnColCnt;
    sal_Int32       mnRowCnt;
    sal_Int32       mnColMax;

    std::vector< sal_Int32 > maColumnEdges;

    RTFRowVector    maRows;

    RTFCellDefault* mpInsDefault;
    RTFCellDefault* mpActDefault;
    RTFCellDefault* mpDefMerge;

    Reference< XTable > mxTable;
};

SdrTableRTFParser::SdrTableRTFParser( SdrTableObj& rTableObj )
: mrTableObj( rTableObj )
, mpOutliner( SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, rTableObj.GetModel() ) )
, mrItemPool( rTableObj.GetModel()->GetItemPool() )
, mnLastToken( 0 )
, mbNewDef( false )
, mnStartPara( 0 )
, mnColCnt( 0 )
, mnRowCnt( 0 )
, mnColMax( 0 )
, mpActDefault( 0 )
, mpDefMerge( 0 )
, mxTable( rTableObj.getTable() )
{
    mpOutliner->SetUpdateMode(sal_True);
    mpOutliner->SetStyleSheet( 0, mrTableObj.GetStyleSheet() );
    mpInsDefault = new RTFCellDefault( &mrItemPool );
}

SdrTableRTFParser::~SdrTableRTFParser()
{
    delete mpOutliner;
    delete mpInsDefault;
}

void SdrTableRTFParser::Read( SvStream& rStream )
{
    EditEngine& rEdit = const_cast< EditEngine& >( mpOutliner->GetEditEngine() );

    Link aOldLink( rEdit.GetImportHdl() );
    rEdit.SetImportHdl( LINK( this, SdrTableRTFParser, RTFImportHdl ) );
    mpOutliner->Read( rStream, OUString(), EE_FORMAT_RTF );
    rEdit.SetImportHdl( aOldLink );

    FillTable();
}

IMPL_LINK( SdrTableRTFParser, RTFImportHdl, ImportInfo*, pInfo )
{
    switch ( pInfo->eState )
    {
        case RTFIMP_NEXTTOKEN:
            ProcToken( pInfo );
            break;
        case RTFIMP_UNKNOWNATTR:
            ProcToken( pInfo );
            break;
        case RTFIMP_START:
        {
            SvxRTFParser* pParser = (SvxRTFParser*) pInfo->pParser;
            pParser->SetAttrPool( &mrItemPool );
            RTFPardAttrMapIds& rMap = pParser->GetPardMap();
            rMap.nBox = SDRATTR_TABLE_BORDER;
        }
            break;
        case RTFIMP_END:
            if ( pInfo->aSelection.nEndPos )
            {
                mpActDefault = NULL;
                pInfo->nToken = RTF_PAR;
                pInfo->aSelection.nEndPara++;
                ProcToken( pInfo );
            }
            break;
        case RTFIMP_SETATTR:
            break;
        case RTFIMP_INSERTTEXT:
            break;
        case RTFIMP_INSERTPARA:
            break;
        default:
            SAL_WARN( "svx.table","unknown ImportInfo.eState");
    }
    return 0;
}

void SdrTableRTFParser::NextRow()
{
    ++mnRowCnt;
}

void SdrTableRTFParser::InsertCell( ImportInfo* pInfo )
{
    sal_Int32 nCol = mpActDefault->mnCol;

    RTFCellInfoPtr xCellInfo( new RTFCellInfo(mrItemPool) );

    xCellInfo->mnStartPara = mnStartPara;
    xCellInfo->mnParaCount = pInfo->aSelection.nEndPara - 1 - mnStartPara;

    if( !maRows.empty() )
    {
        RTFColumnVectorPtr xColumn( maRows.back() );

        if( xColumn->size() <= (size_t)nCol )
            xColumn->resize( nCol+1 );

        (*xColumn)[nCol] = xCellInfo;
    }

    mnStartPara = pInfo->aSelection.nEndPara - 1;
}

void SdrTableRTFParser::FillTable()
{
    try
    {
        sal_Int32 nColCount = mxTable->getColumnCount();
        Reference< XTableColumns > xCols( mxTable->getColumns(), UNO_QUERY_THROW );

        if( nColCount < mnColMax )
        {
            xCols->insertByIndex( nColCount, mnColMax - nColCount );
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

        for( sal_Int32 nRow = 0; nRow < (sal_Int32)maRows.size(); nRow++ )
        {
            RTFColumnVectorPtr xColumn( maRows[nRow] );
            for( nCol = 0; nCol < (sal_Int32)xColumn->size(); nCol++ )
            {
                RTFCellInfoPtr xCellInfo( (*xColumn)[nCol] );

                CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );
                if( xCell.is() && xCellInfo.get() )
                {
                    const SfxPoolItem *pPoolItem = 0;
                    if( xCellInfo->maItemSet.GetItemState(SDRATTR_TABLE_BORDER,sal_False,&pPoolItem)==SFX_ITEM_SET)
                        xCell->SetMergedItem( *pPoolItem );

                    OutlinerParaObject* pTextObject = mpOutliner->CreateParaObject( xCellInfo->mnStartPara, xCellInfo->mnParaCount );
                    if( pTextObject )
                    {
                        SdrOutliner& rOutliner=mrTableObj.ImpGetDrawOutliner();
                        rOutliner.SetUpdateMode(sal_True);
                        rOutliner.SetText( *pTextObject );
                        mrTableObj.NbcSetOutlinerParaObjectForText( rOutliner.CreateParaObject(), xCell.get() );
                        delete pTextObject;
                    }
                }
            }
        }

        Rectangle aRect( mrTableObj.GetSnapRect() );
        aRect.Right() = aRect.Left() + nLastEdge;
        mrTableObj.NbcSetSnapRect( aRect );

    }
    catch( Exception& e )
    {
        (void)e;
        OSL_FAIL("sdr::table::SdrTableRTFParser::InsertCell(), exception caught!" );
    }
}

void SdrTableRTFParser::NewCellRow()
{
    if( mbNewDef )
    {
        mbNewDef = sal_False;

        maRows.push_back( RTFColumnVectorPtr( new RTFColumnVector() ) );
    }
    mpDefMerge = NULL;
    maDefaultIterator = maDefaultList.begin();

    NextColumn();

    DBG_ASSERT( mpActDefault, "NewCellRow: pActDefault==0" );
}

void SdrTableRTFParser::NextColumn()
{
    if( maDefaultIterator != maDefaultList.end() )
        mpActDefault = (*maDefaultIterator++).get();
    else
        mpActDefault = 0;
}

long TwipsToHundMM( long nIn )
{
    long nRet = OutputDevice::LogicToLogic( nIn, MAP_TWIP, MAP_100TH_MM );
    return nRet;
}

void SdrTableRTFParser::ProcToken( ImportInfo* pInfo )
{
    switch ( pInfo->nToken )
    {
        case RTF_TROWD:         // denotes table row defauls, before RTF_CELLX
        {
            mnColCnt = 0;
            maDefaultList.clear();
            mpDefMerge = NULL;
            mnLastToken = pInfo->nToken;
        }
        break;
        case RTF_CLMGF:         // The first cell of cells to be merged
        {
            mpDefMerge = mpInsDefault;
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
        case RTF_CELLX:         // closes cell default
        {
            mbNewDef = sal_True;
            mpInsDefault->mnCol = mnColCnt;
            maDefaultList.push_back( boost::shared_ptr< RTFCellDefault >( mpInsDefault ) );

            if( (sal_Int32)maColumnEdges.size() <= mnColCnt )
                maColumnEdges.resize( mnColCnt + 1 );

            const sal_Int32 nSize = TwipsToHundMM( pInfo->nTokenValue );
            maColumnEdges[mnColCnt] = std::max( maColumnEdges[mnColCnt], nSize );

            mpInsDefault = new RTFCellDefault( &mrItemPool );
            if ( ++mnColCnt > mnColMax )
                mnColMax = mnColCnt;
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
                mpActDefault = mpInsDefault;
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
                    ((SvxRTFParser*)pInfo->pParser)->ReadBorderAttr(pInfo->nToken, mpInsDefault->maItemSet, sal_True );
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
