/*************************************************************************
 *
 *  $RCSfile: excdoc.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:31:40 $
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
#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustring>

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
#include "unonames.hxx"
#include "convuno.hxx"
#include "patattr.hxx"

#include "excdoc.hxx"
#include "excupn.hxx"
#include "namebuff.hxx"

#include "xcl97dum.hxx"
#include "xcl97rec.hxx"
#include "xcl97esc.hxx"

#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif
#ifndef SC_XESTYLE_HXX
#include "xestyle.hxx"
#endif
#ifndef SC_XEPAGE_HXX
#include "xepage.hxx"
#endif
#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif

#ifndef SC_XCLEXPPIVOTTABLES_HXX
#include "XclExpPivotTables.hxx"
#endif
#ifndef SC_XCLEXPCHANGETRACK_HXX
#include "XclExpChangeTrack.hxx"
#endif


NameBuffer*     ExcDocument::pTabNames = NULL;



static String lcl_GetVbaTabName( UINT16 n )
{
    String  aRet( RTL_CONSTASCII_USTRINGPARAM( "__VBA__" ) );
    aRet += String::CreateFromInt32( n );
    return aRet;
}




ExcRecordListRefs::~ExcRecordListRefs()
{
}




DefRowXFs::DefRowXFs( void )
{
    nLastList = 0;
    nLastRow = 0;
}


BOOL DefRowXFs::ChangeXF( sal_uInt16 nRow, sal_uInt32& rnXFId )
{
    XclExpDefRowXFVec::const_iterator aBegin = maXFList.begin(), aIter = aBegin, aEnd = maXFList.end();
    if( nRow > nLastRow )
        aIter += nLastList;

    for( ; aIter != aEnd; ++aIter )
    {
        if( nRow == aIter->mnRow )
        {
            rnXFId = aIter->mnXFId;

            nLastList = aIter - aBegin;
            nLastRow = aIter->mnRow;

            return TRUE;
        }
    }
    return FALSE;
}




ExcRowBlock* ExcTable::pRowBlock = NULL;

ExcTable::ExcTable( RootData* pRD ) :
    ExcRoot( pRD ),
    nScTab( 0 ),
    nExcTab( EXC_TABBUF_INVALID ),
    pDefRowXFs( NULL )
{   }


ExcTable::ExcTable( RootData* pRD, UINT16 nScTable ) :
    ExcRoot( pRD ),
    nScTab( nScTable ),
    nExcTab( pRD->pER->GetTabIdBuffer().GetXclTab( nScTable ) ),
    pDefRowXFs( NULL )
{   }


ExcTable::~ExcTable()
{
    Clear();
}


void ExcTable::Clear( void )
{
    if( pDefRowXFs )
    {
        delete pDefRowXFs;
        pDefRowXFs = NULL;
    }
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


void ExcTable::SetDefRowXF( sal_uInt16 nRow, sal_uInt32 nXFId )
{
    if( !pDefRowXFs )
        pDefRowXFs = new DefRowXFs;

    pDefRowXFs->Append( nRow, nXFId );
}


void ExcTable::FillAsHeader( ExcRecordListRefs& rBSRecList )
{
    RootData&           rR              = *pExcRoot;
    const XclExpRoot&   rRoot           = *rR.pER;
    ScDocument&         rDoc            = *rR.pDoc;
    XclExpTabIdBuffer&  rTabBuffer      = rRoot.GetTabIdBuffer();

    if ( rR.eDateiTyp < Biff8 )
        Add( new ExcBofW );
    else
        Add( new ExcBofW8 );

    UINT16  nC;
    String  aTmpString;
    UINT16  nScTabCount     = rTabBuffer.GetScTabCount();
    UINT16  nExcTabCount    = rTabBuffer.GetXclTabCount();
    UINT16  nCodenames      = rTabBuffer.GetCodenameCount();

    ExcNameList*    pNameList   = rR.pNameList  = new ExcNameList( rR );

    rR.pObjRecs = NULL;             // per sheet

    if( rR.eDateiTyp < Biff8 )
        Add( new ExcDummy_00 );
    else
    {
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
        Add( pNameList );

        Add( new XclExpWindowProtection(false) );
        Add( new XclExpDocProtection(rDoc.IsDocProtected() == TRUE) );
        Add( new ExcDummy_040 );
        Add( new Exc1904( rDoc ) );
        Add( new ExcDummy_041 );

        // Formatting: FONT, FORMAT, XF, STYLE, PALETTE
        Add( new XclExpRefRecord( rRoot.GetFontBuffer() ) );
        Add( new XclExpRefRecord( rRoot.GetNumFmtBuffer() ) );
        Add( new XclExpRefRecord( rRoot.GetXFBuffer() ) );
        Add( new XclExpRefRecord( rRoot.GetPalette() ) );

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
        if(rDoc.GetExtDocOptions())
            Add( new XclExpWindowProtection(rDoc.GetExtDocOptions()->IsWinProtected()) );
        else
            Add( new XclExpWindowProtection(false));
        Add( new XclExpDocProtection(rDoc.IsDocProtected() == TRUE) );
        Add( new ExcDummy8_040 );
        Add( new ExcWindow18( rR ) );
        Add( new Exc1904( rDoc ) );
        Add( new ExcDummy8_041 );

        // Formatting: FONT, FORMAT, XF, STYLE, PALETTE
        Add( new XclExpRefRecord( rRoot.GetFontBuffer() ) );
        Add( new XclExpRefRecord( rRoot.GetNumFmtBuffer() ) );
        Add( new XclExpRefRecord( rRoot.GetXFBuffer() ) );
        Add( new XclExpRefRecord( rRoot.GetPalette() ) );

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

        // COUNTRY - in BIFF8 in workbook globals
        Add( new XclExpCountry( rRoot ) );
        // SUPBOOK, XCT, CRN, EXTERNNAME, EXTERNSHEET
        Add( new XclExpRefRecord( rRoot.GetLinkManager() ) );
        // NAME
        Add( pNameList );

        // MSODRAWINGGROUP per-document data
        Add( new XclMsodrawinggroup( rR, ESCHER_DggContainer ) );
        // SST, EXTSST
        Add( new XclExpRefRecord( rRoot.GetSst() ) );
    }

    Add( new ExcEof );
}


void ExcTable::FillAsTable( void )
{
    RootData&           rR          = *pExcRoot;
    const XclExpRoot&   rRoot       = *rR.pER;
    ScDocument&         rDoc        = rRoot.GetDoc();
    XclExpTabIdBuffer&  rTabBuffer  = rRoot.GetTabIdBuffer();
    XclExpXFBuffer&     rXFBuffer   = rRoot.GetXFBuffer();

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
    UINT16                  nCol = 0;
    UINT16                  nRow = 0;

    UINT16                  nMaxFlagCol = rDoc.GetLastFlaggedCol( nScTab );
    UINT16                  nMaxFlagRow = rDoc.GetLastFlaggedRow( nScTab );

    ExcCell*                pAktExcCell;
    SvNumberFormatter&      rFormatter = *rR.pDoc->GetFormatTable();
    const BiffTyp           eDateiTyp = rR.eDateiTyp;

    XclExpHyperlinkList*    pHlinks = new XclExpHyperlinkList;
    XclExpTableOpManager    aTableOpList;
    XclExpTableOp*          pTableOpRec = NULL;

    ExcArrays               aArrayFormList;
    ExcArray*               pLastArray = NULL;

    ExcArrays               aShrdFmlaList;
    ExcShrdFmla*            pShrdFmla = NULL;

    XclExpDval*             pRecDval = NULL;        // data validation

    XclExpNoteList*         pNoteList = NULL;       // cell notes

    ExcFmlaResultStr*       pFormulaResult = NULL;

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

    DBG_ASSERT( !rR.pCellMerging, "ExcTable::FillAsTable - old merging list found" );
    rR.pCellMerging = new XclExpCellMerging;


//    ScUsedAreaIterator      aIterator( &rDoc, nScTab, 0, 0, nLastCol, nLastRow );
    ScUsedAreaIterator      aIterator( &rDoc, nScTab, 0, 0, MAXCOL, nLastRow );
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

    // WSBOOL needs data from page settings, create it here, add it later
    ::std::auto_ptr< XclExpPageSettings > pPageSett( new XclExpPageSettings( rRoot ) );
    bool bFitToPages = pPageSett->GetPageData().mbFitToPages;

    if( eDateiTyp < Biff8 )
    {
        Add( new ExcBof );
        Add( new ExcDummy_02a );
        // GUTS (count & size of outline icons)
        Add( new ExcEGuts( pOLColArray, pOLRowArray ) );
        Add( new ExcDummy_02c );
        // COUNTRY - in BIFF5/7 in every worksheet
        Add( new XclExpCountry( rRoot ) );
        Add( new XclExpWsbool( bFitToPages ) );
    }
    else
    {
        Add( new ExcBof8 );
        Add( new XclCalccount( rDoc ) );
        Add( new XclRefmode() );
        Add( new XclIteration( rDoc ) );
        Add( new XclDelta( rDoc ) );
        Add( new ExcDummy8_02 );
        // GUTS (count & size of outline icons)
        Add( new ExcEGuts( pOLColArray, pOLRowArray ) );
        Add( new XclExpWsbool( bFitToPages ) );
    }

    Add( pPageSett.release() );

    if( rDoc.IsTabProtected( nScTab ) )
        Add( new XclProtection() );

    if ( eDateiTyp < Biff8 && rR.pExtSheetCntAndRecs )
        Add( new ExcExternDup( *rR.pExtSheetCntAndRecs ) );

    Add( new XclExpUInt16Record( EXC_ID_DEFCOLWIDTH, 10 ) );

    // COLINFO records for all columns
    XclExpColinfo* pColinfo = NULL;
    for( sal_uInt16 nScCol = 0; nScCol <= MAXCOL; ++nScCol )
    {
        sal_uInt32 nColXFId = rXFBuffer.Insert( rDoc.GetPattern( nScCol, MAXROW, nScTab ) );
        if( !pColinfo || !pColinfo->Expand( nScCol, nScTab, nColXFId, aExcOLCol ) )
        {
            pColinfo = new XclExpColinfo( rRoot, nScCol, nScTab, nColXFId, aExcOLCol );
            Add( pColinfo );
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
        rR.pObjRecs = new XclObjList( rRoot );
        // AutoFilter
        Add( new ExcAutoFilterRecs( rR, nScTab ) );
        // list of NOTE records
        pNoteList = new XclExpNoteList;
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
        AddRow( new ExcRow( 0, nScTab, 0, 0, EXC_XF_DEFAULTCELL, rDoc, aExcOLRow, *this ) );

    while( bIter )
    {
        nCol = aIterator.GetStartCol();     // nur bei erstem Durchlauf doppelt!
        pAktScCell = aIterator.GetCell();
        pPatt = aIterator.GetPattern();

        pAktExcCell = NULL;
        pNote = NULL;
        pTableOpRec = NULL;

        // add ROW recs from empty rows
        while( nPrevRow < nRow )
        {
            ExcRow* pRow = new ExcRow( nPrevRow, nScTab, 0, 0, EXC_XF_DEFAULTCELL, rDoc, aExcOLRow, *this );
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
                        pAktExcCell = new ExcBoolerr( aScPos, pPatt, rR, UINT8(fVal), FALSE );
                    }
                    else if( XclTools::GetRKFromDouble( nRKValue, fVal ) )
                    {
                        if( pLastRKMulRK )
                        {
                            ExcRKMulRK* pNewRK = pLastRKMulRK->Extend( aScPos, pPatt, rR, nRKValue );
                            if( pNewRK )
                                pLastRKMulRK = pNewRK;

                            pAktExcCell = pNewRK;
                        }
                        else
                            pAktExcCell = pLastRKMulRK = new ExcRKMulRK( aScPos, pPatt, rR, nRKValue );
                    }
                    else
                    {
                        pAktExcCell = new ExcNumber( aScPos, pPatt, rR, fVal );
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
                        pAktExcCell = new ExcLabel( aScPos, pPatt, rR, aTemp );
                    else
                        pAktExcCell = new XclExpLabelSst( *rR.pER, aScPos, aTemp, pPatt );
                }
                break;
                case CELLTYPE_FORMULA:
                {
                    pLastRKMulRK = NULL;
                    ScFormulaCell* pFormCell = (ScFormulaCell*) pAktScCell;

                    // current cell number format
                    sal_uInt32 nCellNumFmt = pPatt ?
                        static_cast< const SfxUInt32Item& >( pPatt->GetItem( ATTR_VALUE_FORMAT ) ).GetValue() :
                        rR.pER->GetNumFmtBuffer().GetStandardFormat();
                    // alternative number format passed to XF buffer
                    sal_uInt32 nFmlaNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND;

                    /*  #73420# Xcl doesn't know boolean number formats, we write
                        "TRUE";"TRUE";"FALSE" (language dependent). Don't do it for
                        automatic formula formats, because Xcl gets them right. */
                    /*  #i8640# Don't set text format, if we have string results */
                    if( ((nCellNumFmt % SV_COUNTRY_LANGUAGE_OFFSET) == 0) &&
                            (pFormCell->GetFormatType() != NUMBERFORMAT_LOGICAL) &&
                            (pFormCell->GetFormatType() != NUMBERFORMAT_TEXT) )
                        nFmlaNumFmt = pFormCell->GetStandardFormat( rFormatter, nCellNumFmt );
                    /*  #73420# If cell number format is Boolean and automatic formula
                        format is Boolean don't write that ugly special format. */
                    else if( (pFormCell->GetFormatType() == NUMBERFORMAT_LOGICAL) &&
                            (rFormatter.GetType( nCellNumFmt ) == NUMBERFORMAT_LOGICAL) )
                        nFmlaNumFmt = rR.pER->GetNumFmtBuffer().GetStandardFormat();

                    ExcFormula* pFmlaCell = new ExcFormula(
                        aScPos, pPatt, rR, nFmlaNumFmt, *pFormCell->GetCode(),
                        &pLastArray, ( ScMatrixMode ) pFormCell->GetMatrixFlag(), &pShrdFmla, &aShrdFmlaList, pFormCell, &pFormulaResult);
                    pAktExcCell = pFmlaCell;
                    pTableOpRec = aTableOpList.InsertCell( pFormCell->GetCode(), *pFmlaCell );
                }
                break;
                case CELLTYPE_EDIT:
                {
                    pLastRKMulRK = NULL;
                    ScEditCell& rEditCell = *static_cast< ScEditCell* >( const_cast< ScBaseCell* >( pAktScCell ) );
                    if( rR.eDateiTyp < Biff8 )
                        pAktExcCell = new ExcRString( aScPos, pPatt, rR, rEditCell );
                    else
                        pAktExcCell = new XclExpLabelSst( *rR.pER, aScPos, rEditCell, pPatt );

                    XclExpHyperlink*& rpHlink = rR.pLastHlink;
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
                    pAktScCell = NULL;  // #i11733# empty note cell is empty cell
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

        // #i11733# not "else" - pAktScCell may be set to NULL above (empty note cell)
        if( !pAktScCell )
        {// leere Zelle mit Attributierung
            UINT16  nColCnt = aIterator.GetEndCol() - aIterator.GetStartCol() + 1;

            if( pLastBlank && pLastBlank->GetLastCol() + 1 == aIterator.GetStartCol() )
            {
                pLastBlank->Add( aScPos, pPatt, rR, nColCnt, *this );
                pAktExcCell = NULL;    // kein NEUER Record!
            }
            else
            {
                pLastBlank = new ExcBlankMulblank( aScPos, pPatt, rR, nColCnt, *this );
                pAktExcCell = pLastBlank;
            }
        }

        if( pAktExcCell )
        {
            Add( pAktExcCell );

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
            Add( pTableOpRec );

        if(pFormulaResult)
        {
            Add( pFormulaResult );
            pFormulaResult = NULL;
        }


        // notes
        if( rRoot.GetBiff() < xlBiff8 )
        {
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
                Add( new ExcNote( aScPos, sNoteText, rR ) );
        }
        else
        {
            if( pNote || rR.sAddNoteText.Len() )
                pNoteList->Append( new XclExpNote( rRoot, aScPos, pNote, rR.sAddNoteText ) );
        }

        if( pPatt && (rR.eDateiTyp >= Biff8) )
        {
            // merged cells
            ScMergeAttr& rItem = (ScMergeAttr&) pPatt->GetItem( ATTR_MERGE );
            if( rItem.IsMerged() )
            {
                sal_uInt32 nXFId = (pAktExcCell ? pAktExcCell->GetXFId() :
                            (pLastBlank ? pLastBlank->GetXFId() :
                            (pLastRKMulRK ? pLastRKMulRK->GetXFId() :
                            XclExpXFBuffer::GetXFIdFromIndex( EXC_XF_DEFAULTCELL ))));
                for( UINT16 iCol = aIterator.GetStartCol(); iCol <= aIterator.GetEndCol(); iCol++ )
                    rR.pCellMerging->Append( iCol, rItem.GetColMerge(), nRow, rItem.GetRowMerge(), nXFId );
            }

            // data validation
            const SfxPoolItem* pItem;
            if( pPatt->GetItemSet().GetItemState( ATTR_VALIDDATA, FALSE, &pItem ) == SFX_ITEM_SET )
            {
                sal_uInt32 nHandle = ((const SfxUInt32Item*)pItem)->GetValue();
                if( !pRecDval )
                    pRecDval = new XclExpDval( *rR.pER );
                ScRange aRange( aScPos );
                aRange.aEnd.SetCol( aIterator.GetEndCol() );
                pRecDval->InsertCellRange( aRange, nHandle );
            }
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
            AddRow( new ExcRow( nPrevRow, nScTab, nColMin, nCol, EXC_XF_DEFAULTCELL, rDoc, aExcOLRow, *this ) );

            nPrevRow++;
            nColMin = aIterator.GetStartCol();
            nFirstCol = Min( nFirstCol, nColMin );
            pLastBlank = NULL;
            pLastRKMulRK = NULL;
        }
    }

    // remaining rows with attributes
    while( nRow < nMaxFlagRow )
    {
        nRow++;
        ExcRow* pRow = new ExcRow( nRow, nScTab, 0, 0, EXC_XF_DEFAULTCELL, rDoc, aExcOLRow, *this );
        AddUsedRow( pRow );
    }

    // insert merged cells
    Add( rR.pCellMerging );
    rR.pCellMerging = NULL;
    // label ranges
    if( rR.eDateiTyp >= Biff8 )
        Add( new XclExpLabelranges( *rR.pER ) );
    // insert data validation
    if( pRecDval )
        Add( pRecDval );
    pRecDval = NULL;
    // update dimensions
    pDimensions->SetLimits( nFirstCol, nFirstRow, nLastCol, nLastRow );
    // update formula cells for multiple operations
    aTableOpList.UpdateCells();

    if( rR.eDateiTyp < Biff8 )
    {
        Add( new ExcWindow2( nExcTab ) );
        Add( new ExcSelection( 0, 0, 3 ) );
    }
    else
    {
        rR.pEscher->AddSdrPage( rR );
        //! close Escher group shape and ESCHER_DgContainer
        //! opened by XclObjList ctor MSODRAWING
        rR.pObjRecs->EndSheet();
        // all MSODRAWING and OBJ stuff of this sheet goes here
        Add( rR.pObjRecs );
        // NOTE records
        Add( pNoteList );

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
        Add( new ExcWindow28( *rR.pER, nScTab ) );
    }

    if( rR.eDateiTyp >= Biff8 )
    {
        // web queries
        Add( new XclExpWebQueryBuffer( *rR.pER ) );

        // conditional formats
        Add( new XclExpCondFormatBuffer( *rR.pER ) );

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
    const XclExpRoot& rRoot = *rR.pER;

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
            rR.pObjRecs = new XclObjList( rRoot );
            // all drawing obects
            rR.pEscher->AddSdrPage( rR );
            //! close Escher group shape and ESCHER_DgContainer
            //! opened by XclObjList ctor MSODRAWING
            rR.pObjRecs->EndSheet();
            Add( rR.pObjRecs );
        }
        // WINDOW2

        Add( new ExcWindow28( rRoot , nScTab ) );
    }
    Add( new ExcEof );
}


BOOL ExcTable::ModifyToDefaultRowXF( sal_uInt16 nRow, sal_uInt32& rnXFId )
{
    if( pDefRowXFs )
        return pDefRowXFs->ChangeXF( nRow, rnXFId );
    return FALSE;
}


void ExcTable::Write( XclExpStream& rStr )
{
    aRecList.Save( rStr );
}




ExcDocument::ExcDocument( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    aHeader( rRoot.mpRD ),
    pExpChangeTrack( NULL )
{
    pTabNames = new NameBuffer( 0, 16 );

    pPrgrsBar = new ScProgress(
        GetDocShell(), ScGlobal::GetRscString(STR_SAVE_DOC),
        ( UINT32 ) GetDoc().GetCellCount() * 2 );
    ExcCell::SetPrgrsBar( *pPrgrsBar );
}


ExcDocument::~ExcDocument()
{
    maTableList.Clear();    //! for the following assertion
    DBG_ASSERT( ExcCell::_nRefCount == 0, "*ExcDocument::~ExcDocument(): Ein'n hab'n wir noch!" );

    delete pTabNames;
#ifdef DBG_UTIL
    pTabNames = NULL;
#endif

    delete pPrgrsBar;
    ExcCell::ClearPrgrsBar();

    delete pExpChangeTrack;
}


void ExcDocument::ReadDoc( void )
{
    aHeader.FillAsHeader( aBundleSheetRecList );

    sal_uInt16 nScTabCount = GetTabIdBuffer().GetMaxScTabCount();
    while( GetScTab() < nScTabCount )
    {
        if( GetTabIdBuffer().IsExportTable( GetScTab() ) )
        {
            ExcTable* pTab = new ExcTable( mpRD, GetScTab() );
            maTableList.Append( pTab );
            pTab->FillAsTable();
        }
        IncScTab();
    }

    if ( GetBiff() >= xlBiff8 )
    {
        // complete temporary Escher stream
        mpRD->pEscher->GetEx()->EndDocument();

        // change tracking
        if ( GetDoc().GetChangeTrack() )
            pExpChangeTrack = new XclExpChangeTrack( mpRD );
    }
}


void ExcDocument::Write( SvStream& rSvStrm )
{
    if( !maTableList.Empty() )
    {
        if ( GetBiff() >= xlBiff8 )
            mpRD->pEscher->GetStrm().Seek(0);   // ready for take off

        GetPalette().Reduce();
        GetXFBuffer().Reduce();

        XclExpStream        aXclStrm( rSvStrm, GetRoot() );
        ExcTable*           pTab = maTableList.First();
        ExcBundlesheetBase* pAktBS = ( ExcBundlesheetBase* ) aBundleSheetRecList.First();

        aHeader.Write( aXclStrm );

        while( pTab )
        {
            DBG_ASSERT( pAktBS, "-ExcDocument::Write(): BundleSheetRecs und Tabs passen nicht zusammen!" );
            pAktBS->SetStreamPos( aXclStrm.GetStreamPos() );
            pTab->Write( aXclStrm );
            pTab = maTableList.Next();
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
    if( mpRD->pPivotCacheList )
        mpRD->pPivotCacheList->Write();
    if( pExpChangeTrack )
        pExpChangeTrack->Write();
}

