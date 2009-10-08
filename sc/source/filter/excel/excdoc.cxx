/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: excdoc.cxx,v $
 * $Revision: 1.69.60.2 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


//------------------------------------------------------------------------

#include "scitems.hxx"

#include <svx/svdobj.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/lrspitem.hxx>
#include <svx/ulspitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/zformat.hxx>
#include <sot/storage.hxx>
#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustring.hxx>

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
#include "docoptio.hxx"

#include "excdoc.hxx"
#include "namebuff.hxx"

#include "xcl97dum.hxx"
#include "xcl97rec.hxx"
#include "xcl97esc.hxx"
#include "xetable.hxx"
#include "xelink.hxx"
#include "xename.hxx"
#include "xepage.hxx"
#include "xeview.hxx"
#include "xecontent.hxx"
#include "xeescher.hxx"
#include "xepivot.hxx"
#include "XclExpChangeTrack.hxx"

#include <math.h>

static String lcl_GetVbaTabName( SCTAB n )
{
    String  aRet( RTL_CONSTASCII_USTRINGPARAM( "__VBA__" ) );
    aRet += String::CreateFromInt32( static_cast<sal_uInt16>(n) );
    return aRet;
}


ExcTable::ExcTable( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnScTab( SCTAB_GLOBAL ),
    nExcTab( EXC_NOTAB ),
    pTabNames( new NameBuffer( 0, 16 ) )
{
}


ExcTable::ExcTable( const XclExpRoot& rRoot, SCTAB nScTab ) :
    XclExpRoot( rRoot ),
    mnScTab( nScTab ),
    nExcTab( rRoot.GetTabInfo().GetXclTab( nScTab ) ),
    pTabNames( new NameBuffer( 0, 16 ) )
{
}


ExcTable::~ExcTable()
{
    delete pTabNames;
}


void ExcTable::Add( XclExpRecordBase* pRec )
{
    DBG_ASSERT( pRec, "-ExcTable::Add(): pRec ist NULL!" );
    aRecList.AppendNewRecord( pRec );
}


void ExcTable::FillAsHeader( ExcBoundsheetList& rBoundsheetList )
{
    InitializeGlobals();

    RootData& rR = GetOldRoot();
    ScDocument& rDoc = GetDoc();
    XclExpTabInfo& rTabInfo = GetTabInfo();

    if ( GetBiff() <= EXC_BIFF5 )
        Add( new ExcBofW );
    else
        Add( new ExcBofW8 );

    SCTAB   nC;
    String  aTmpString;
    SCTAB  nScTabCount     = rTabInfo.GetScTabCount();
    UINT16  nExcTabCount    = rTabInfo.GetXclTabCount();
    UINT16  nCodenames      = static_cast< UINT16 >( GetExtDocOptions().GetCodeNameCount() );

    rR.pObjRecs = NULL;             // per sheet

    if( GetBiff() <= EXC_BIFF5 )
        Add( new ExcDummy_00 );
    else
    {
        Add( new ExcDummy8_00a );
        rR.pTabId = new XclExpChTrTabId( Max( nExcTabCount, nCodenames ) );
        Add( rR.pTabId );
        if( HasVbaStorage() )
        {
            Add( new XclObproj );
            const String& rCodeName = GetExtDocOptions().GetDocSettings().maGlobCodeName;
            if( rCodeName.Len() )
                Add( new XclCodename( rCodeName ) );
        }
        Add( new ExcDummy8_00b );
    }

    // erst Namen- und Tabellen-Eintraege aufbauen
    String          aName;

    for( nC = 0 ; nC < nScTabCount ; nC++ )
        if( rTabInfo.IsExportTab( nC ) )
        {
            rDoc.GetName( nC, aTmpString );
            *pTabNames << aTmpString;
        }

    if ( GetBiff() <= EXC_BIFF5 )
    {
        // global link table: EXTERNCOUNT, EXTERNSHEET, NAME
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_NAME ) );
    }

    aRecList.AppendNewRecord( new XclExpWindowProtection( GetExtDocOptions().GetDocSettings().mbWinProtected ) );
    aRecList.AppendNewRecord( new XclExpDocProtection( rDoc.IsDocProtected() ) );
    aRecList.AppendNewRecord( new XclExpBoolRecord( EXC_ID_PASSWORD, false ) );

    if( GetBiff() == EXC_BIFF8 )
        Add( new ExcDummy8_040 );

    aRecList.AppendNewRecord( new XclExpWindow1( GetRoot() ) );

    if( GetBiff() <= EXC_BIFF5 )
    {
        Add( new ExcDummy_040 );
        Add( new Exc1904( rDoc ) );
        Add( new ExcDummy_041 );
    }
    else
    {
        Add( new Exc1904( rDoc ) );
        Add( new XclExpBoolRecord( 0x000E, !rDoc.GetDocOptions().IsCalcAsShown() ) );
        Add( new ExcDummy8_041 );
    }

    // Formatting: FONT, FORMAT, XF, STYLE, PALETTE
    aRecList.AppendRecord( CreateRecord( EXC_ID_FONTLIST ) );
    aRecList.AppendRecord( CreateRecord( EXC_ID_FORMATLIST ) );
    aRecList.AppendRecord( CreateRecord( EXC_ID_XFLIST ) );
    aRecList.AppendRecord( CreateRecord( EXC_ID_PALETTE ) );

    if( GetBiff() <= EXC_BIFF5 )
    {
        // Bundlesheet
        for( nC = 0 ; nC < nScTabCount ; nC++ )
            if( rTabInfo.IsExportTab( nC ) )
            {
                ExcBoundsheetList::RecordRefType xBoundsheet( new ExcBundlesheet( rR, nC ) );
                aRecList.AppendRecord( xBoundsheet );
                rBoundsheetList.AppendRecord( xBoundsheet );
            }
    }
    else
    {
        // Pivot Cache
        GetPivotTableManager().CreatePivotTables();
        aRecList.AppendRecord( GetPivotTableManager().CreatePivotCachesRecord() );

        // Change tracking
        if( rDoc.GetChangeTrack() )
        {
            rR.pUserBViewList = new XclExpUserBViewList( *rDoc.GetChangeTrack() );
            Add( rR.pUserBViewList );
        }

        // Natural Language Formulas Flag
        aRecList.AppendNewRecord( new XclExpBoolRecord( EXC_ID_USESELFS, GetDoc().GetDocOptions().IsLookUpColRowNames() ) );

        // Bundlesheet
        for( nC = 0 ; nC < nScTabCount ; nC++ )
            if( rTabInfo.IsExportTab( nC ) )
            {
                ExcBoundsheetList::RecordRefType xBoundsheet( new ExcBundlesheet8( rR, nC ) );
                aRecList.AppendRecord( xBoundsheet );
                rBoundsheetList.AppendRecord( xBoundsheet );
            }

        for( SCTAB nAdd = 0; nC < static_cast<SCTAB>(nCodenames) ; nC++, nAdd++ )
        {
            aTmpString = lcl_GetVbaTabName( nAdd );
            ExcBoundsheetList::RecordRefType xBoundsheet( new ExcBundlesheet8( aTmpString ) );
            aRecList.AppendRecord( xBoundsheet );
            rBoundsheetList.AppendRecord( xBoundsheet );
        }

        // COUNTRY - in BIFF8 in workbook globals
        Add( new XclExpCountry( GetRoot() ) );
        // link table: SUPBOOK, XCT, CRN, EXTERNNAME, EXTERNSHEET, NAME
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_NAME ) );

        // MSODRAWINGGROUP per-document data
        Add( new XclMsodrawinggroup( rR, ESCHER_DggContainer ) );
        // Shared string table: SST, EXTSST
        aRecList.AppendRecord( CreateRecord( EXC_ID_SST ) );
    }

    Add( new ExcEof );
}


void ExcTable::FillAsTable( size_t nCodeNameIdx )
{
    InitializeTable( mnScTab );

    RootData& rR = GetOldRoot();
    XclBiff eBiff = GetBiff();
    ScDocument& rDoc = GetDoc();

    DBG_ASSERT( (mnScTab >= 0L) && (mnScTab <= MAXTAB), "-ExcTable::Table(): mnScTab - no ordinary table!" );
    DBG_ASSERT( nExcTab <= static_cast<sal_uInt16>(MAXTAB), "-ExcTable::Table(): nExcTab - no ordinary table!" );

    if ( eBiff == EXC_BIFF8 )
        // list holding OBJ records and creating MSODRAWING per-sheet data
        rR.pObjRecs = new XclObjList( GetRoot() );

    // cell table: DEFROWHEIGHT, DEFCOLWIDTH, COLINFO, DIMENSIONS, ROW, cell records
    mxCellTable.reset( new XclExpCellTable( GetRoot() ) );


    // WSBOOL needs data from page settings, create it here, add it later
    ScfRef< XclExpPageSettings > xPageSett( new XclExpPageSettings( GetRoot() ) );
    bool bFitToPages = xPageSett->GetPageData().mbFitToPages;

    if( eBiff <= EXC_BIFF5 )
    {
        Add( new ExcBof );
        Add( new ExcDummy_02a );
    }
    else
    {
        Add( new ExcBof8 );
        Add( new XclCalccount( rDoc ) );
        Add( new XclRefmode( rDoc ) );
        Add( new XclIteration( rDoc ) );
        Add( new XclDelta( rDoc ) );
        Add( new ExcDummy8_02 );
    }

    // GUTS (count & size of outline icons)
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_GUTS ) );
    // DEFROWHEIGHT, created by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID2_DEFROWHEIGHT ) );

    // COUNTRY - in BIFF5/7 in every worksheet
    if( eBiff <= EXC_BIFF5 )
        Add( new XclExpCountry( GetRoot() ) );

    Add( new XclExpWsbool( bFitToPages ) );

    // page settings (SETUP and various other records)
    aRecList.AppendRecord( xPageSett );

    if( rDoc.IsTabProtected( mnScTab ) )
        Add( new XclProtection() );

    // local link table: EXTERNCOUNT, EXTERNSHEET
    if( eBiff <= EXC_BIFF5 )
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );

    if ( eBiff == EXC_BIFF8 )
    {
        // Scenarios
        Add( new ExcEScenarioManager( rDoc, mnScTab ) );
        // filter
        aRecList.AppendRecord( GetFilterManager().CreateRecord( mnScTab ) );
    }

    // cell table: DEFCOLWIDTH, COLINFO, DIMENSIONS, ROW, cell records
    aRecList.AppendRecord( mxCellTable );

    // MERGEDCELLS record, generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_MERGEDCELLS ) );
    // label ranges
    if( eBiff == EXC_BIFF8 )
        Add( new XclExpLabelranges( GetRoot() ) );
    // data validation (DVAL and list of DV records), generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_DVAL ) );

    if( eBiff == EXC_BIFF8 )
    {
        rR.pEscher->AddSdrPage();
        //! close Escher group shape and ESCHER_DgContainer
        //! opened by XclObjList ctor MSODRAWING
        rR.pObjRecs->EndSheet();
        // all MSODRAWING and OBJ stuff of this sheet goes here
        Add( rR.pObjRecs );

        // pivot tables
        aRecList.AppendRecord( GetPivotTableManager().CreatePivotTablesRecord( mnScTab ) );
    }

    // list of NOTE records, generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_NOTE ) );

    // sheet view settings: WINDOW2, SCL, PANE, SELECTION
    aRecList.AppendNewRecord( new XclExpTabViewSettings( GetRoot(), mnScTab ) );

    if( eBiff == EXC_BIFF8 )
    {
        // web queries
        Add( new XclExpWebQueryBuffer( GetRoot() ) );

        // conditional formats
        Add( new XclExpCondFormatBuffer( GetRoot() ) );

        if( HasVbaStorage() )
            if( nCodeNameIdx < GetExtDocOptions().GetCodeNameCount() )
                Add( new XclCodename( GetExtDocOptions().GetCodeName( nCodeNameIdx ) ) );
    }

    // list of HLINK records, generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_HLINK ) );

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


void ExcTable::FillAsEmptyTable( size_t nCodeNameIdx )
{
    InitializeTable( mnScTab );

    if( HasVbaStorage() && (nCodeNameIdx < GetExtDocOptions().GetCodeNameCount()) )
    {
        if( GetBiff() <= EXC_BIFF5 )
        {
            Add( new ExcBof );
        }
        else
        {
            Add( new ExcBof8 );
            Add( new XclCodename( GetExtDocOptions().GetCodeName( nCodeNameIdx ) ) );
        }
        // sheet view settings: WINDOW2, SCL, PANE, SELECTION
        aRecList.AppendNewRecord( new XclExpTabViewSettings( GetRoot(), mnScTab ) );
        Add( new ExcEof );
    }
}


void ExcTable::Write( XclExpStream& rStr )
{
    SetCurrScTab( mnScTab );
    if( mxCellTable.get() )
        mxCellTable->Finalize();
    aRecList.Save( rStr );
}




ExcDocument::ExcDocument( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    aHeader( rRoot ),
    pExpChangeTrack( NULL )
{
}


ExcDocument::~ExcDocument()
{
    maTableList.RemoveAllRecords();    //! for the following assertion
    delete pExpChangeTrack;
}


void ExcDocument::ReadDoc( void )
{
    InitializeConvert();

    aHeader.FillAsHeader( maBoundsheetList );

    SCTAB nScTab = 0, nScTabCount = GetTabInfo().GetScTabCount();
    size_t nCodeNameIdx = 0, nCodeNameCount = GetExtDocOptions().GetCodeNameCount();

    for( ; nScTab < nScTabCount; ++nScTab )
    {
        if( GetTabInfo().IsExportTab( nScTab ) )
        {
            ExcTableList::RecordRefType xTab( new ExcTable( GetRoot(), nScTab ) );
            maTableList.AppendRecord( xTab );
            xTab->FillAsTable( nCodeNameIdx );
            ++nCodeNameIdx;
        }
    }
    for( ; nCodeNameIdx < nCodeNameCount; ++nScTab, ++nCodeNameIdx )
    {
        ExcTableList::RecordRefType xTab( new ExcTable( GetRoot(), nScTab ) );
        maTableList.AppendRecord( xTab );
        xTab->FillAsEmptyTable( nCodeNameIdx );
    }

    if ( GetBiff() == EXC_BIFF8 )
    {
        // complete temporary Escher stream
        GetOldRoot().pEscher->GetEx()->EndDocument();

        // change tracking
        if ( GetDoc().GetChangeTrack() )
            pExpChangeTrack = new XclExpChangeTrack( GetRoot() );
    }
}


void ExcDocument::Write( SvStream& rSvStrm )
{
    if( !maTableList.IsEmpty() )
    {
        InitializeSave();

        if ( GetBiff() == EXC_BIFF8 )
            GetOldRoot().pEscher->GetStrm().Seek(0);   // ready for take off

        XclExpStream aXclStrm( rSvStrm, GetRoot() );

        aHeader.Write( aXclStrm );

        DBG_ASSERT( maTableList.GetSize() == maBoundsheetList.GetSize(),
            "ExcDocument::Write - different number of sheets and BOUNDSHEET records" );

        for( size_t nTab = 0, nTabCount = maTableList.GetSize(); nTab < nTabCount; ++nTab )
        {
            // set current stream position in BOUNDSHEET record
            ExcBoundsheetRef xBoundsheet = maBoundsheetList.GetRecord( nTab );
            if( xBoundsheet.get() )
                xBoundsheet->SetStreamPos( aXclStrm.GetSvStreamPos() );
            // write the table
            maTableList.GetRecord( nTab )->Write( aXclStrm );
        }

        // write the table stream positions into the BOUNDSHEET records
        for( size_t nBSheet = 0, nBSheetCount = maBoundsheetList.GetSize(); nBSheet < nBSheetCount; ++nBSheet )
            maBoundsheetList.GetRecord( nBSheet )->UpdateStreamPos( aXclStrm );
    }
    if( pExpChangeTrack )
        pExpChangeTrack->Write();
}

