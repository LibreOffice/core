/*************************************************************************
 *
 *  $RCSfile: excdoc.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: dr $ $Date: 2001-03-13 10:10:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <math.h>

#include "scitems.hxx"

#include <svx/svdobj.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/lrspitem.hxx>
#include <svx/ulspitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/zformat.hxx>
#include <so3/svstor.hxx>

#include "drwlayer.hxx"

#include "cell.hxx"
#include "dociter.hxx"
#include "document.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "progress.hxx"
#include "conditio.hxx"
#include "dpobject.hxx"
#include "attrib.hxx"
#include "scextopt.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "olinetab.hxx"

#include "excdoc.hxx"
#include "excupn.hxx"
#include "namebuff.hxx"

#include "xcl97dum.hxx"
#include "xcl97rec.hxx"
#include "xcl97esc.hxx"

#ifndef _SC_XCLEXPPIVOTTABLES_HXX
#include "XclExpPivotTables.hxx"
#endif
#ifndef _SC_XCLEXPCHANGETRACK_HXX
#include "XclExpChangeTrack.hxx"
#endif


NameBuffer*     ExcDocument::pTabNames = NULL;



static String lcl_GetVbaTabName( UINT16 n )
{
    String  aRet( RTL_CONSTASCII_STRINGPARAM( "__VBA__" ) );
    aRet += String::CreateFromInt32( n );
    return aRet;
}




ExcRecordListRefs::~ExcRecordListRefs()
{
}




ExcRecordListInst::~ExcRecordListInst()
{
    ExcRecord*  pDel = ( ExcRecord* ) List::First();
    while( pDel )
    {
        delete pDel;
        pDel = ( ExcRecord* ) List::Next();
    }
}




ExcRowBlock* ExcTable::pRowBlock = NULL;

ExcTable::ExcTable( RootData* pRD ) :
    ExcRoot( pRD ),
    nScTab( 0 ),
    nExcTab( EXC_TABBUF_INVALID )
{   }


ExcTable::ExcTable( RootData* pRD, UINT16 nScTable ) :
    ExcRoot( pRD ),
    nScTab( nScTable ),
    nExcTab( pRD->pTabBuffer->GetExcTable( nScTable ) )
{   }


ExcTable::~ExcTable()
{
    Clear();
}


void ExcTable::Clear( void )
{
}


void ExcTable::AddRow( ExcRow* pRow )
{
    DBG_ASSERT( pRowBlock, "ExcTable::AddRow() - missing RowBlock!" );
    ExcRowBlock* pNewRowBlock = pRowBlock->Append( pRow );
    if( pNewRowBlock )
    {
        pRowBlock = pNewRowBlock;
        Add( pRowBlock );
    }
}


void ExcTable::AddUsedRow( ExcRow*& rpRow )
{
    if( rpRow->IsDefault() )
        delete rpRow;
    else
        AddRow( rpRow );
}


void ExcTable::FillAsHeader( ExcRecordListRefs& rBSRecList )
{
    RootData&           rR          = *pExcRoot;
    ScDocument&         rDoc        = *rR.pDoc;
    ExcETabNumBuffer&   rTabBuffer  = *rR.pTabBuffer;

    if ( rR.eDateiTyp < Biff8 )
        Add( new ExcBofW );
    else
        Add( new ExcBofW8 );

    UINT16  nC;
    String  aTmpString;
    UINT16  nScTabCount     = rTabBuffer.GetScTabCount();
    UINT16  nExcTabCount    = rTabBuffer.GetExcTabCount();
    UINT16  nCodenames      = rR.nCodenames;

    ExcNameList*    pNameL      = rR.pNameList = new ExcNameList;
    ExcPalette2*    pPalette2   = rR.pPalette2 = new ExcPalette2( *rR.pColor );
    UsedFontList*   pFontRecs   = rR.pFontRecs = new UsedFontList( rR ) ;
    UsedFormList*   pFormRecs   = rR.pFormRecs = new UsedFormList;

    XclSstList*         pSstRecs            = NULL;
    XclExternsheetList* pExternsheetRecs    = NULL;     // change: ExternsheetList includes Supbooks
    if ( rR.eDateiTyp >= Biff8 )
    {
        rR.pSstRecs         = pSstRecs          = new XclSstList;
        rR.pExternsheetRecs = pExternsheetRecs  = new XclExternsheetList( &rR );
    }

    rR.pObjRecs = NULL;     // per sheet
    rR.pNoteRecs = NULL;        // per sheet

    pFontRecs->SetBaseIndex( 6 );   // 6 statt 5 wegen Ausfall von 4!
    pFormRecs->SetBaseIndex( 164 ); // siehe auch ValueFormBuffer::nNewFormats
    ExcFont::SetPalette( *pPalette2 );

    UsedAttrList*   pXFRecs = new UsedAttrList( &rR, *pPalette2, *pFontRecs, *pFormRecs );
    pXFRecs->SetBaseIndex( 21 );
    ExcCell::SetXFRecs( pXFRecs );


    ExcXf::SetPalette( *pPalette2 );

    if( rR.eDateiTyp < Biff8 )
        Add( new ExcDummy_00 );
    else
    {
        // first create style XFs
        SfxStyleSheetIterator   aStyleIter( rDoc.GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
        SfxStyleSheetBase*      pStyle = aStyleIter.First();
        ScPatternAttr*          pPatt;

        while( pStyle )
        {
            if( pStyle->IsUserDefined() )
            {
                pPatt = new ScPatternAttr( &pStyle->GetItemSet() );
                pXFRecs->Find( pPatt, TRUE );
            }
            pStyle = aStyleIter.Next();
        }

        Add( new ExcDummy8_00a );
        rR.pTabId = new XclExpChTrTabId( Max( nExcTabCount, nCodenames ) );
        Add( rR.pTabId );
        if( rR.bWriteVBAStorage )
        {
            Add( new XclObproj );
            const String*   p = rR.pExtDocOpt->GetCodename();
            if( p )
                Add( new XclCodename( *p ) );
        }
        Add( new ExcDummy8_00b );
    }

    // erst Namen- und Tabellen-Eintraege aufbauen
    DBG_ASSERT( rDoc.GetRangeName(), "-ExcTable::Header(): Keine Namen? Kann nicht angehen!" );

    String          aName;

    for( nC = 0 ; nC < nScTabCount ; nC++ )
        if( rTabBuffer.IsExportTable( nC ) )
        {
            rDoc.GetName( nC, aTmpString );
            *ExcDocument::pTabNames << aTmpString;
        }

    if ( rR.eDateiTyp < Biff8 )
    {
        // Externcount & Externsheet
        ExcExterncount* pExtCnt = new ExcExterncount( &rR, FALSE );

        Add( pExtCnt );

        ExcExternsheetList*     pExcExtShtList = new ExcExternsheetList;

        DBG_ASSERT( !rR.pExtSheetCntAndRecs, "*ExcTable::Header(): pExtSheetCntAndRecs already exist!" );

        rR.pExtSheetCntAndRecs = new ExcExternDup( *pExtCnt, *pExcExtShtList );

        Add( pExcExtShtList );

        for( nC = 0 ; nC < nScTabCount ; nC++ )
            if( rTabBuffer.IsExportTable( nC ) )
                pExcExtShtList->Add( new ExcExternsheet( &rR, nC ) );

        // Names
        Add( pNameL );

        ScRangeName&            rRangeNames = *rDoc.GetRangeName();
        UINT16                  j, nNames;
        nNames = rRangeNames.GetCount();
        for( j = 0 ; j < nNames ; j++ )
        {
            ScRangeData*        pData = ( ScRangeData * ) rRangeNames[ j ];
            DBG_ASSERT( pData, "-ExcTable::Header(): 1, 2, 3 - wer zaehlt hier falsch?" );

            if ( !rR.bBreakSharedFormula || !pData->HasType( RT_SHARED ) )
            {   // no SHARED_FORMULA_... names if not needed
                pData->SetExportIndex( rR.nRangeNameIndex++ );

                pNameL->Append( new ExcName( &rR, pData ) );
            }
        }
        ScDBCollection&         rDBAreas = *rDoc.GetDBCollection();
        nNames = rDBAreas.GetCount();
        for( j = 0 ; j < nNames ; j++ )
        {
            ScDBData*           pData = ( ScDBData * ) rDBAreas[ j ];
            DBG_ASSERT( pData, "-ExcTable::Header(): 1, 2, 3 - wer zaehlt hier falsch?" );

            pData->SetExportIndex( rR.nRangeNameIndex++ );
            pNameL->Append( new ExcName( &rR, pData ) );
        }

        Add( new ExcDummy_040 );
        Add( new Exc1904( rDoc ) );
        Add( new ExcDummy_041 );
        // Font
        Add( new ExcDummy_01 );
        Add( pFontRecs );
        // Format
        Add( new ExcDummy_Fm );
        Add( pFormRecs );
        // XF + Style
        Add( new ExcDummy_XF );
        Add( pXFRecs );
        // Style
        Add( new ExcDummy_Style );
        // Colors
        Add( pPalette2 );

        // Bundlesheet
        ExcBundlesheetBase* pBS;
        for( nC = 0 ; nC < nScTabCount ; nC++ )
            if( rTabBuffer.IsExportTable( nC ) )
            {
                pBS = new ExcBundlesheet( rR, nC );
                Add( pBS );
                rBSRecList.Append( pBS );
            }
    }
    else
    {
        Add( new ExcDummy8_040 );
        Add( new Exc1904( rDoc ) );
        Add( new ExcDummy8_041 );
        // Font
        Add( new ExcDummy8_01 );
        Add( pFontRecs );
        // Format
        Add( new ExcDummy8_Fm );
        Add( pFormRecs );
        // XF + Style
        Add( new ExcDummy8_XF );
        Add( pXFRecs );
        // Style
        Add( new ExcDummy8_Style );

        // Pivot Cache
        ScDPCollection*     pDPColl = rDoc.GetDPCollection();
        if( pDPColl )
        {
            XclPivotCacheList* pPCList = new XclPivotCacheList( &rR, *pDPColl );
            rR.pPivotCacheList = pPCList;

            for( const XclPivotCache* pCache = pPCList->First(); pCache; pCache = pPCList->Next() )
            {
                Add( new XclSxIdStm( *pCache ) );
                Add( new XclSxVs( *pCache ) );
                Add( new XclDConRef( pCache->GetRange(), pCache->GetWorkbook() ) );
            }
        }

        // Colors
        Add( pPalette2 );

        // Change tracking
        if( rDoc.GetChangeTrack() )
        {
            rR.pUserBViewList = new XclExpUserBViewList( *rDoc.GetChangeTrack() );
            Add( rR.pUserBViewList );
        }

        // Natural Language Formulas Flag
        Add( new ExcDummy8_UsesElfs );

        // Bundlesheet
        ExcBundlesheetBase* pBS;
        for( nC = 0 ; nC < nScTabCount ; nC++ )
            if( rTabBuffer.IsExportTable( nC ) )
            {
                pBS = new ExcBundlesheet8( rR, nC );
                Add( pBS );
                rBSRecList.Append( pBS );
            }

        for( UINT16 nAdd = 0; nC < nCodenames ; nC++, nAdd++ )
        {
            aTmpString = lcl_GetVbaTabName( nAdd );
            pBS = new ExcBundlesheet8( aTmpString );
            Add( pBS );
            rBSRecList.Append( pBS );
        }

        // COUNTRY always Germany
        Add( new ExcDummy8_Country );
        // SUPBOOKs, XCTs, CRNs, EXTERNSHEET
        Add( pExternsheetRecs );
        // NAMEs
        Add( pNameL );

        ScRangeName&        rRangeNames = *rDoc.GetRangeName();
        UINT16              j, nNames;
        nNames = rRangeNames.GetCount();
        for( j = 0 ; j < nNames ; j++ )
        {
            ScRangeData*    pData = ( ScRangeData * ) rRangeNames[ j ];
            DBG_ASSERT( pData, "-ExcTable::Header(): 1, 2, 3 - wer zaehlt hier falsch?" );

            if ( !rR.bBreakSharedFormula || !pData->HasType( RT_SHARED ) )
            {   // no SHARED_FORMULA_... names if not needed
                ExcName*        pExcNameRec = new ExcName( &rR, pData );

                if( pExcNameRec->IsDummy() )
                    pData->SetExportIndex( 0xFFFF );
                else
                    pData->SetExportIndex( rR.nRangeNameIndex++ );

                pNameL->Append( pExcNameRec );
            }
        }

        // print range and titles
        for( nC = 0 ; nC < nScTabCount ; nC++ )
        {
            pNameL->Append( new XclPrintRange( &rR, nC, rDoc ) );
            pNameL->Append( new XclTitleRange( &rR, nC, rDoc ) );
        }

        ScDBCollection&     rDBAreas = *rDoc.GetDBCollection();
        nNames = rDBAreas.GetCount();
        for( j = 0 ; j < nNames ; j++ )
        {
            ScDBData*       pData = ( ScDBData * ) rDBAreas[ j ];
            DBG_ASSERT( pData, "-ExcTable::Header(): 1, 2, 3 - wer zaehlt hier falsch?" );

            pData->SetExportIndex( rR.nRangeNameIndex++ );
            pNameL->Append( new ExcName( &rR, pData ) );
        }

        // MSODRAWINGGROUP per-document data
        Add( new XclMsodrawinggroup( rR, ESCHER_DggContainer ) );
        // SST, EXTSST
        Add( pSstRecs );
    }

    Add( new ExcEof );
}


void ExcTable::FillAsTable( void )
{
    RootData&           rR          = *pExcRoot;
    ScDocument&         rDoc        = *rR.pDoc;
    ExcETabNumBuffer&   rTabBuffer  = *rR.pTabBuffer;

    if( nScTab >= rTabBuffer.GetScTabCount() )
    {
        CodenameList*       pL = rR.pExtDocOpt->GetCodenames();
        if( pL )
        {
            const String*   p = pL->Next();
            if( p )
                NullTab( p );
        }

        return;
    }

    UINT16                  nLastCol, nLastRow,         // in Tabelle
                            nFirstCol, nFirstRow;
    UINT16                  nPrevRow = 0;
    UINT16                  nColMin;                    // fuer aktuelle Zeile
                                                        //  Row-Records
    const UINT16            nDefXF = 0x0F;
    UINT16                  nCol = 0;
    UINT16                  nRow = 0;

    UINT16                  nMaxFlagCol = rDoc.GetLastFlaggedCol( nScTab );
    UINT16                  nMaxFlagRow = rDoc.GetLastFlaggedRow( nScTab );;

    ExcRecord*              pAktExcCell;
    SvNumberFormatter&      rFormatter = *rR.pFormTable;
    const BiffTyp           eDateiTyp = rR.eDateiTyp;

    SfxStyleSheet*          pStSh = ( SfxStyleSheet* ) rDoc.GetStyleSheetPool()->Find(
                                        rDoc.GetPageStyle( nScTab ), SFX_STYLE_FAMILY_PAGE );
    rR.pStyleSheet = pStSh;

    SfxItemSet*             pStyleSheetItemSet = pStSh? &pStSh->GetItemSet() : NULL;
    rR.pStyleSheetItemSet = pStyleSheetItemSet;

    ExcRecordList*          pHlinks = new ExcRecordList;
    XclCellMergingList*     pCellMerging = new XclCellMergingList;
    XclExpTableOpManager    aTableOpList;
    XclExpTableOp*          pTableOpRec = NULL;

    ExcArrays               aArrayFormList;
    ExcArray*               pLastArray = NULL;

    ExcArrays               aShrdFmlaList;
    ExcShrdFmla*            pShrdFmla = NULL;

    DBG_ASSERT( (nScTab >= 0L) && (nScTab <= MAXTAB), "-ExcTable::Table(): nScTab - no ordinary table!" );
    DBG_ASSERT( (nExcTab >= 0L) && (nExcTab <= MAXTAB), "-ExcTable::Table(): nExcTab - no ordinary table!" );

    rDoc.GetTableArea( nScTab, nLastCol, nLastRow );

    if( nLastRow > rR.nRowMax )     // max. Zeilenzahl ueberschritten?
    {
        nLastRow = rR.nRowMax;
        rR.bCellCut = TRUE;
    }

    // find outline range
    ScOutlineTable*         pOLTable    = rDoc.GetOutlineTable( nScTab );
    ScOutlineArray*         pOLColArray = NULL;
    ScOutlineArray*         pOLRowArray = NULL;
    if( pOLTable )
    {
        UINT16              nStart, nEnd;
        UINT16              nMaxOLCol = 0;
        UINT16              nMaxOLRow = 0;

        pOLColArray = pOLTable->GetColArray();
        if( pOLColArray )
        {
            pOLColArray->GetRange( nStart, nEnd );
            nMaxOLCol = nEnd + 1;
        }
        nMaxFlagCol = Max( nMaxFlagCol, nMaxOLCol );

        pOLRowArray = pOLTable->GetRowArray();
        if( pOLRowArray )
        {
            pOLRowArray->GetRange( nStart, nEnd );
            nMaxOLRow = nEnd + 1;
        }
        nMaxFlagRow = Max( nMaxFlagRow, nMaxOLRow );
    }
    nMaxFlagCol = Min( nMaxFlagCol, (UINT16) MAXCOL );
    nMaxFlagRow = Min( Min( nMaxFlagRow, (UINT16) MAXROW ), rR.nRowMax );

    ExcEOutline aExcOLCol( pOLColArray );
    ExcEOutline aExcOLRow( pOLRowArray );


    ScUsedAreaIterator      aIterator( &rDoc, nScTab, 0, 0, nLastCol, nLastRow );
//  ScUsedAreaIterator      aIterator( &rDoc, nScTab, 0, 0, MAXCOL, MAXROW );
    const ScBaseCell*       pAktScCell;
    const ScPatternAttr*    pPatt;
    ExcBlankMulblank*       pLastBlank = NULL;
    ExcRKMulRK*             pLastRKMulRK = NULL;
    BOOL                    bIter;

    // jetz schon, um erste Zeile zu bekommen
    bIter = aIterator.GetNext();
    if( bIter )
    {
        nCol = aIterator.GetStartCol();
        nRow = aIterator.GetRow();

        if( nRow > rR.nRowMax )     // max. Zeilenzahl ueberschritten?
        {
            rR.bCellCut = TRUE;
            NullTab();
            return;
        }
        pAktScCell = aIterator.GetCell();
        pPatt = aIterator.GetPattern();
    }

    // Header und Default-Recs
    if( eDateiTyp < Biff8 )
    {
        Add( new ExcBof );
        // CALCMODE bis VCENTER
        Add( new ExcDummy_02 );
    }
    else
    {
        Add( new ExcBof8 );
        // CALCMODE bis VCENTER
        Add( new ExcDummy8_02 );
    }

    // GUTS (count & size of outline icons)
    Add( new ExcEGuts( pOLColArray, pOLRowArray ) );

    // HORIZONTALPAGEBREAKS & VERTICALPAGEBREAKS
    if( eDateiTyp < Biff8 )
    {
        Add( new XclExpPageBreaks( rR, nScTab, XclExpPageBreaks::pbHorizontal ) );
        Add( new XclExpPageBreaks( rR, nScTab, XclExpPageBreaks::pbVertical ) );
    }
    else
    {
        Add( new XclExpPageBreaks8( rR, nScTab, XclExpPageBreaks::pbHorizontal ) );
        Add( new XclExpPageBreaks8( rR, nScTab, XclExpPageBreaks::pbVertical ) );
    }

    const SvxLRSpaceItem&   rLRSpaceItem = ( const SvxLRSpaceItem& ) pStyleSheetItemSet->Get( ATTR_LRSPACE );
    Add( new ExcMargin( rLRSpaceItem.GetLeft(), IMPEXC_MARGINSIDE_LEFT ) );
    Add( new ExcMargin( rLRSpaceItem.GetRight(), IMPEXC_MARGINSIDE_RIGHT ) );

    const SvxULSpaceItem&   rULSpaceItem = ( const SvxULSpaceItem& ) pStyleSheetItemSet->Get( ATTR_ULSPACE );
    Add( new ExcMargin( rULSpaceItem.GetUpper(), IMPEXC_MARGINSIDE_TOP ) );
    Add( new ExcMargin( rULSpaceItem.GetLower(), IMPEXC_MARGINSIDE_BOTTOM ) );

    Add( new ExcPrintheaders( pStyleSheetItemSet ) );
    Add( new ExcPrintGridlines( pStyleSheetItemSet ) );
    Add( new ExcHcenter( pStyleSheetItemSet ) );
    Add( new ExcVcenter( pStyleSheetItemSet ) );

    Add( new ExcHeader( &rR, eDateiTyp >= Biff8 ) );
    Add( new ExcFooter( &rR, eDateiTyp >= Biff8 ) );
    Add( new ExcSetup( &rR ) );

    if( eDateiTyp >= Biff8 )
    {
        Add( new XclBGPic( rR ) );

        if( rDoc.IsTabProtected( nScTab ) )
            Add( new XclProtection() );
    }

    if ( eDateiTyp < Biff8 && rR.pExtSheetCntAndRecs )
        Add( new ExcExternDup( *rR.pExtSheetCntAndRecs ) );

    // Defcolwidth (Breite aus Excel-Dokument)
    ExcDefcolwidth*             pExcDefColWidth = new ExcDefcolwidth( 0x000a );
    Add( pExcDefColWidth );

    // COLINFO records
    ExcColinfo* pLastColInfo = new ExcColinfo( 0, nScTab, nDefXF, rR, aExcOLCol );
    ExcColinfo* pNewColInfo;

    Add( pLastColInfo );
    for( UINT16 iCol = 1; iCol <= MAXCOL; iCol++ )
    {
        pNewColInfo = new ExcColinfo( iCol, nScTab, nDefXF, rR, aExcOLCol );
        pLastColInfo->Expand( pNewColInfo );
        if( pNewColInfo )
        {
            pLastColInfo = pNewColInfo;
            Add( pLastColInfo );
        }
    }

    // Dimensions
    ExcDimensions*          pDimensions = new ExcDimensions( rR.eDateiTyp );
    Add( pDimensions );

    if ( rR.eDateiTyp >= Biff8 )
    {
        // Scenarios
        Add( new ExcEScenarioManager( rDoc, nScTab ) );
        // list holding OBJ records and creating MSODRAWING per-sheet data
        rR.pObjRecs = new XclObjList( rR );
        // AutoFilter
        Add( new ExcAutoFilterRecs( rR, nScTab ) );
        // list of NOTE records
        rR.pNoteRecs = new XclNoteList;
    }

    // NOTE
    const ScPostIt*         pNote = NULL;

    // rows & cols
    nFirstRow = nRow;
    nColMin = nFirstCol = nCol;
    pRowBlock = new ExcRowBlock;
    Add( pRowBlock );

    // at least one ROW rec
    if( !bIter )
        AddRow( new ExcRow( 0, nScTab, 0, 0, nDefXF, rDoc, aExcOLRow ) );

    while( bIter )
    {
        nCol = aIterator.GetStartCol();     // nur bei erstem Durchlauf doppelt!
        pAktScCell = aIterator.GetCell();
        pPatt = aIterator.GetPattern();

        // add ROW recs from empty rows
        while( nPrevRow < nRow )
        {
            ExcRow* pRow = new ExcRow( nPrevRow, nScTab, 0, 0, nDefXF, rDoc, aExcOLRow );
            AddUsedRow( pRow );
            nPrevRow++;
        }

        ScAddress   aScPos( nCol, nRow, nScTab );
        rR.sAddNoteText.Erase();

        if( pAktScCell )
        {// nicht-leere Zelle
            pLastBlank = NULL;
            pNote = pAktScCell->GetNotePtr();

            switch( pAktScCell->GetCellType() )
            {
                case CELLTYPE_NONE:
                    pLastRKMulRK = NULL;
                    break;
                case CELLTYPE_VALUE:
                {
                    double  fVal = ( ( ScValueCell * ) pAktScCell )->GetValue();
                    INT32   nRKValue;
                    if ( pPatt && (fVal == 0.0 || fVal == 1.0) &&
                            rFormatter.GetType(
                            ((const SfxUInt32Item&)pPatt->GetItem(
                            ATTR_VALUE_FORMAT )).GetValue() ) == NUMBERFORMAT_LOGICAL )
                    {
                        pLastRKMulRK = NULL;
                        pAktExcCell = new ExcBoolerr( aScPos, pPatt, UINT8(fVal), FALSE );
                    }
                    else if( XclExpHelper::GetRKFromDouble( fVal, nRKValue ) )
                    {
                        if( pLastRKMulRK )
                        {
                            ExcRKMulRK* pNewRK = pLastRKMulRK->Extend( aScPos, pPatt, nRKValue );
                            if( pNewRK )
                                pLastRKMulRK = pNewRK;

                            pAktExcCell = pNewRK;
                        }
                        else
                            pAktExcCell = pLastRKMulRK = new ExcRKMulRK( aScPos, pPatt, nRKValue );
                    }
                    else
                    {
                        pAktExcCell = new ExcNumber( aScPos, pPatt, fVal );
                        pLastRKMulRK = NULL;
                    }
                }
                break;
                case CELLTYPE_STRING:
                {
                    pLastRKMulRK = NULL;
                    String  aTemp;

                    ( ( ScStringCell* ) pAktScCell )->GetString( aTemp );

                    if ( rR.eDateiTyp < Biff8 )
                        pAktExcCell = new ExcLabel( aScPos, pPatt, aTemp, rR );
                    else
                        pAktExcCell = new ExcLabelSst( aScPos, pPatt, aTemp, rR );
                }
                break;
                case CELLTYPE_FORMULA:
                {
                    pLastRKMulRK = NULL;
                    ScFormulaCell*      pFormCell = ( ScFormulaCell * ) pAktScCell;
                    ULONG   nCellNumForm = ( pPatt ?
                        (( const SfxUInt32Item& ) pPatt->GetItem(
                        ATTR_VALUE_FORMAT )).GetValue() : 0 );
                    ULONG   nAltNumForm;
                    BOOL    bForceAltNumForm;
                    if( ( nCellNumForm % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 )
                    {
                        // #73420# Xcl doesn't know boolean number formats,
                        // we write "TRUE";"TRUE";"FALSE" or "WAHR";"WAHR";"FALSCH"
                        // or any other language dependent key words instead.
                        // Don't do it for automatic formula formats,
                        // because Xcl gets them right.
                        if( pFormCell->GetFormatType() == NUMBERFORMAT_LOGICAL )
                            nAltNumForm = NUMBERFORMAT_ENTRY_NOT_FOUND;
                        else
                            nAltNumForm = pFormCell->GetStandardFormat(
                                rFormatter, nCellNumForm );
                        bForceAltNumForm = FALSE;
                    }
                    else
                    {
                        // #73420# If number format set is boolean and
                        // automatic format is boolean don't write that ugly
                        // special format.
                        if( pFormCell->GetFormatType() == NUMBERFORMAT_LOGICAL
                                && rFormatter.GetType( nCellNumForm ) == NUMBERFORMAT_LOGICAL )
                        {
                            nAltNumForm = 0;
                            bForceAltNumForm = TRUE;
                        }
                        else
                        {
                            nAltNumForm = NUMBERFORMAT_ENTRY_NOT_FOUND;
                            bForceAltNumForm = FALSE;
                        }

                    }
                    ExcFormula* pFmlaCell = new ExcFormula(
                        &rR, aScPos, pPatt, nAltNumForm, bForceAltNumForm, *pFormCell->GetCode(),
                        &pLastArray, ( ScMatrixMode ) pFormCell->GetMatrixFlag(), &pShrdFmla, &aShrdFmlaList );
                    pAktExcCell = pFmlaCell;
                    pTableOpRec = aTableOpList.InsertCell( pFormCell->GetCode(), *pFmlaCell );
                }
                break;
                case CELLTYPE_EDIT:
                {
                    pLastRKMulRK = NULL;
                    if( rR.eDateiTyp < Biff8 )
                        pAktExcCell = new ExcRString( &rR, aScPos, pPatt, *((ScEditCell*) pAktScCell) );
                    else
                        pAktExcCell = new ExcLabelSst( aScPos, pPatt, *((ScEditCell*) pAktScCell), rR );

                    XclHlink*&      rpHlink = rR.pLastHlink;
                    if( rpHlink )
                    {
                        rpHlink->SetPosition( aScPos );
                        pHlinks->Append( rpHlink );
                        rpHlink = NULL;
                    }
                }
                break;
                case CELLTYPE_NOTE:
                {
                    pAktExcCell = NULL;
                    pLastRKMulRK = NULL;
                    DBG_ASSERT( pNote, "-ExcTable::Table(): Note-Cell ohne Note!" );
                }
                break;
#ifdef DBG_UTIL
                case CELLTYPE_DESTROYED:
                    pAktExcCell = NULL;
                    pLastRKMulRK = NULL;
                    break;
#endif
                default:
                    DBG_ERROR( "*ExcTable::Table(): Unbekannter Zelltyp" );
                    pAktExcCell = NULL;
                    pLastRKMulRK = NULL;
            }
        }
        else
        {// leere Zelle mit Attributierung
            pNote = NULL;
            if( pLastBlank && pLastBlank->GetLastCol() + 1 == aIterator.GetStartCol() )
            {
                pLastBlank->Add( pPatt,
                    aIterator.GetEndCol() - aIterator.GetStartCol() + 1 );

                pAktScCell = NULL;  // kein NEUER Record!
            }
            else
            {
                pLastBlank = new ExcBlankMulblank( aScPos, pPatt,
                    aIterator.GetEndCol() - aIterator.GetStartCol() + 1 );
                pAktExcCell = pLastBlank;
            }
        }

        if( pAktExcCell )
        {
            Add( pAktExcCell );
            pAktExcCell = NULL;

            if( pLastArray )
            {
                if( aArrayFormList.Insert( pLastArray ) )
                    Add( pLastArray );  // really new
                else
                    delete pLastArray;  // allready added

                pLastArray = NULL;
            }

            if( pShrdFmla )
            {
                aShrdFmlaList.Append( pShrdFmla );
                Add( pShrdFmla );
                pShrdFmla = NULL;
            }
        }

        if( pTableOpRec )
        {
            Add( pTableOpRec );
            pTableOpRec = NULL;
        }

        // notes
        String sNoteText;
        String sNoteAuthor;
        if( pNote )
        {
            sNoteText = pNote->GetText();
            sNoteAuthor = pNote->GetAuthor();
        }
        if( rR.sAddNoteText.Len() )
        {
            if( sNoteText.Len() )
                (sNoteText += (sal_Unicode) 0x0A) += (sal_Unicode) 0x0A;
            sNoteText += rR.sAddNoteText;
        }
        if( sNoteText.Len() || sNoteAuthor.Len() )
        {
            if ( rR.eDateiTyp < Biff8 )
                Add( new ExcNote( aScPos, sNoteText, rR ) );
            else
                rR.pNoteRecs->Add( new XclNote( rR, aScPos, sNoteText, sNoteAuthor ) );
        }

        // merged cells
        if( pPatt )
        {
            ScMergeAttr& rItem = (ScMergeAttr&) pPatt->GetItem( ATTR_MERGE );
            if( rItem.IsMerged() )
                for( UINT16 iCol = aIterator.GetStartCol(); iCol <= aIterator.GetEndCol(); iCol++ )
                    pCellMerging->Append( iCol, rItem.GetColMerge(),
                                            nRow, rItem.GetRowMerge() );
        }

        bIter = aIterator.GetNext();

        // new row number
        if( bIter )
        {
            nRow = aIterator.GetRow();
            if( nRow > rR.nRowMax )     // Excel row limit
                bIter = FALSE;
        }

        // new row -> add previous ROW rec
        if( !bIter || (nPrevRow < nRow) )
        {
            AddRow( new ExcRow( nPrevRow, nScTab, nColMin, nCol, nDefXF, rDoc, aExcOLRow ) );
            nPrevRow++;
            nColMin = aIterator.GetStartCol();
            nFirstCol = Min( nFirstCol, nColMin );
        }
    }

    // remaining rows with attributes
    while( nRow < nMaxFlagRow )
    {
        nRow++;
        ExcRow* pRow = new ExcRow( nRow, nScTab, 0, 0, nDefXF, rDoc, aExcOLRow );
        AddUsedRow( pRow );
    }

    // insert merged cells
    Add( pCellMerging );
    // update dimensions
    pDimensions->SetLimits( nFirstCol, nFirstRow, nLastCol, nLastRow );
    // update formula cells for multiple operations
    aTableOpList.UpdateCells();

    if( rR.eDateiTyp < Biff8 )
        Add( new ExcWindow2( nExcTab ) );
    else
    {
        ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
        if( pDrawLayer )
        {
            SdrPage* pPage = pDrawLayer->GetPage( nScTab );
            if( pPage )
                rR.pEscher->GetEx()->AddSdrPage( *pPage );
        }
        //! close Escher group shape and ESCHER_DgContainer
        //! opened by XclObjList ctor MSODRAWING
        rR.pObjRecs->EndSheet();
        // all MSODRAWING and OBJ stuff of this sheet goes here
        Add( rR.pObjRecs );
        // NOTE records
        Add( rR.pNoteRecs );

        // pivot tables
        ScDPCollection*     pDPColl = rDoc.GetDPCollection();
        XclPivotCacheList*  pPCList = rR.pPivotCacheList;
        if( pDPColl && pPCList )
        {
            for( USHORT nObjCnt = 0; nObjCnt < pDPColl->GetCount(); nObjCnt++ )
            {
                ScDPObject*             pDPObject   = (*pDPColl)[ nObjCnt ];
                const XclPivotCache*    pCache      = pPCList->Get( nObjCnt );

                if( pDPObject && pCache )
                {
                    const ScRange& rRange = pDPObject->GetOutRange();
                    if( rRange.aStart.Tab() == nScTab )
                        Add( new XclPivotTableRecs( *pCache, nObjCnt ) );
                }
            }
        }

        // WINDOW2
        Add( new ExcWindow28( nExcTab ) );
    }

    // Default-Recs am Ende
    Add( new ExcDummy_03 );

    if( rR.eDateiTyp >= Biff8 )
    {
        // conditional formats
        const ScConditionalFormatList*  pCondFormList = rDoc.GetCondFormList();
        if( pCondFormList )
        {
            UINT32                      nCondCnt = pCondFormList->Count();
            ScConditionalFormat* const* ppCondForm = pCondFormList->GetData();
            ScRangeList*                pRangeList = NULL;

            while( nCondCnt )
            {
                if( *ppCondForm )
                {
                    const ScConditionalFormat&  rCF = **ppCondForm;

                    if( pRangeList )
                        pRangeList->Clear();
                    else
                        pRangeList = new ScRangeList;

                    rDoc.FindConditionalFormat( rCF.GetKey(), *pRangeList );

                    if( pRangeList->Count() )
                    {
                        Add( new XclCondFormat( rCF, pRangeList, rR ) );
                        pRangeList = NULL;
                    }
                }

                ppCondForm++;
                nCondCnt--;
            }

            if( pRangeList )
                delete pRangeList;
        }

        if( rR.bWriteVBAStorage )
        {
            CodenameList*       pL = rR.pExtDocOpt->GetCodenames();
            if( pL )
            {
                const String* p = nExcTab ? pL->Next() : pL->First();
                if( p )
                    Add( new XclCodename( *p ) );
            }
        }
    }

    rR.pStyleSheet = NULL;
    rR.pStyleSheetItemSet = NULL;

    Add( pHlinks );

    // change tracking
    if( rR.pUserBViewList )
    {
        for( const XclExpUserBView* pBView = rR.pUserBViewList->First(); pBView; pBView = rR.pUserBViewList->Next() )
        {
            Add( new XclExpUsersViewBegin( pBView->GetGUID(), nExcTab ) );
            Add( new XclExpUsersViewEnd );
        }
    }

    // EOF
    Add( new ExcEof );
}


void ExcTable::NullTab( const String* pCodename )
{
    DBG_ASSERT( (nScTab >= 0L) && (nScTab <= MAXTAB), "-ExcTable::Table(): nScTab - no ordinary table!" );
    DBG_ASSERT( (nExcTab >= 0L) && (nExcTab <= MAXTAB), "-ExcTable::Table(): nExcTab - no ordinary table!" );

    RootData&       rR = *pExcRoot;

    if ( rR.eDateiTyp < Biff8 )
    {
        Add( new ExcBof );
        Add( new ExcWindow2( nExcTab ) );
    }
    else
    {
        Add( new ExcBof8 );

        if( pCodename )
            Add( new XclCodename( *pCodename ) );
        else
        {
            // create at least the MSODRAWING per-sheet data
            rR.pObjRecs = new XclObjList( rR );
            // all drawing obects
            ScDrawLayer*    pDrawLayer = pExcRoot->pDoc->GetDrawLayer();
            if ( pDrawLayer )
            {
                SdrPage*    pPage = pDrawLayer->GetPage( nScTab );
                if ( pPage )
                    rR.pEscher->GetEx()->AddSdrPage( *pPage );
            }
            //! close Escher group shape and ESCHER_DgContainer
            //! opened by XclObjList ctor MSODRAWING
            rR.pObjRecs->EndSheet();
            Add( rR.pObjRecs );
        }
        // WINDOW2

        Add( new ExcWindow28( nExcTab ) );
    }
    Add( new ExcEof );
}


void ExcTable::Write( XclExpStream& rStr )
{
    ExcRecord*          pAkt = aRecList.First();

    while( pAkt )
    {
        pAkt->Save( rStr );
        pAkt = aRecList.Next();
    }
}




void ExcDocument::Clear( void )
{
    ExcTable*           pDelTab = ( ExcTable* ) List::First();
    while( pDelTab )
    {
        delete pDelTab;
        pDelTab = ( ExcTable * ) List::Next();
    }
    List::Clear();
}


ExcDocument::ExcDocument( RootData* pRD ) :
    ExcRoot( pRD ),
    aHeader( pRD ),
    pExpChangeTrack( NULL )
{
    pTabNames = new NameBuffer( 0, 16 );

    pPrgrsBar = new ScProgress(
        NULL, ScGlobal::GetRscString(STR_SAVE_DOC),
        ( UINT32 ) pExcRoot->pDoc->GetCellCount() * 2 );
    ExcCell::SetPrgrsBar( *pPrgrsBar );
}


ExcDocument::~ExcDocument()
{
    Clear();

    DBG_ASSERT( ExcCell::_nRefCount == 0, "*ExcDocument::~ExcDocument(): Ein'n hab'n wir noch!" );

    delete pTabNames;
#ifdef DBG_UTIL
    pTabNames = NULL;
#endif

    delete pPrgrsBar;
    ExcCell::ClearPrgrsBar();

    if( pExpChangeTrack )
        delete pExpChangeTrack;
}


void ExcDocument::ReadDoc( void )
{
    CodenameList*   pL = pExcRoot->pExtDocOpt->GetCodenames();
    UINT16          nCodenames = pExcRoot->nCodenames = (UINT16) (pL ? pL->Count() : 0);

    aHeader.FillAsHeader( aBundleSheetRecList );

    UINT16          nTabs = Max( pExcRoot->pDoc->GetTableCount(), nCodenames );
    UINT16          nTabCnt;
    pExcRoot->pAktTab = &nTabCnt;

    for( nTabCnt = 0 ; nTabCnt < nTabs ; nTabCnt++ )
        Add( nTabCnt );

    pExcRoot->pAktTab = NULL;

    if ( pExcRoot->eDateiTyp >= Biff8 )
    {
        // complete temporary Escher stream
        pExcRoot->pEscher->GetEx()->EndDocument();

        // change tracking
        if ( pExcRoot->pDoc->GetChangeTrack() )
            pExpChangeTrack = new XclExpChangeTrack( pExcRoot );
    }
}


void ExcDocument::Add( UINT16 nScTab )
{
    if( pExcRoot->pTabBuffer->IsExportTable( nScTab ) )
    {
        ExcTable* pTab = new ExcTable( pExcRoot, nScTab );
        List::Insert( pTab, LIST_APPEND );
        pTab->FillAsTable();
    }
}


void ExcDocument::Write( SvStream& rSvStrm )
{
    if( List::Count() > 0 )
    {
        ULONG nMaxRecordLen;
        if ( pExcRoot->eDateiTyp >= Biff8 )
        {
            pExcRoot->pEscher->GetStrm().Seek(0);   // ready for take off
            nMaxRecordLen = EXC_MAXRECLEN_BIFF8;
        }
        else
            nMaxRecordLen = EXC_MAXRECLEN_BIFF5;

        pExcRoot->pPalette2->ReduceColors();

        XclExpStream        aXclStrm( rSvStrm, nMaxRecordLen );
        ExcTable*           pAktTab = ( ExcTable* ) List::First();
        ExcBundlesheetBase* pAktBS = ( ExcBundlesheetBase* ) aBundleSheetRecList.First();

        aHeader.Write( aXclStrm );

        while( pAktTab )
        {
            DBG_ASSERT( pAktBS, "-ExcDocument::Write(): BundleSheetRecs und Tabs passen nicht zusammen!" );
            pAktBS->SetStreamPos( aXclStrm.GetStreamPos() );
            pAktTab->Write( aXclStrm );
            pAktTab = ( ExcTable* ) List::Next();
            pAktBS = ( ExcBundlesheetBase* ) aBundleSheetRecList.Next();
        }

        DBG_ASSERT( !pAktBS, "+ExcDocument::Write(): mehr BundleSheetRecs als Tabs!" );

        // BundleSheetRecs anpassen
        pAktBS = ( ExcBundlesheetBase* ) aBundleSheetRecList.First();
        while( pAktBS )
        {
            pAktBS->UpdateStreamPos( aXclStrm );
            pAktBS = ( ExcBundlesheetBase* ) aBundleSheetRecList.Next();
        }

    }
    if( pExcRoot->pPivotCacheList )
        pExcRoot->pPivotCacheList->Write();
    if( pExpChangeTrack )
        pExpChangeTrack->Write();
}

