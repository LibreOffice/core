/*************************************************************************
 *
 *  $RCSfile: excdoc.cxx,v $
 *
 *  $Revision: 1.57 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:00:04 $
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
#include <sot/storage.hxx>
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
#include "namebuff.hxx"

#include "xcl97dum.hxx"
#include "xcl97rec.hxx"
#include "xcl97esc.hxx"

#ifndef SC_XETABLE_HXX
#include "xetable.hxx"
#endif
#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif
#ifndef SC_XENAME_HXX
#include "xename.hxx"
#endif
#ifndef SC_XEPAGE_HXX
#include "xepage.hxx"
#endif
#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif
#ifndef SC_XEESCHER_HXX
#include "xeescher.hxx"
#endif
#ifndef SC_XEPIVOT_HXX
#include "xepivot.hxx"
#endif

#ifndef SC_XCLEXPCHANGETRACK_HXX
#include "XclExpChangeTrack.hxx"
#endif


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

    RootData& rR = *mpRD;
    ScDocument& rDoc = GetDoc();
    XclExpTabInfo& rTabInfo = GetTabInfo();

    if ( rR.eDateiTyp < Biff8 )
        Add( new ExcBofW );
    else
        Add( new ExcBofW8 );

    SCTAB   nC;
    String  aTmpString;
    SCTAB  nScTabCount     = rTabInfo.GetScTabCount();
    UINT16  nExcTabCount    = rTabInfo.GetXclTabCount();
    UINT16  nCodenames      = rTabInfo.GetXclCodenameCount();

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
            const String*   p = GetExtDocOptions().GetCodename();
            if( p )
                Add( new XclCodename( *p ) );
        }
        Add( new ExcDummy8_00b );
    }

    // erst Namen- und Tabellen-Eintraege aufbauen
    DBG_ASSERT( rDoc.GetRangeName(), "-ExcTable::Header(): Keine Namen? Kann nicht angehen!" );

    String          aName;

    for( nC = 0 ; nC < nScTabCount ; nC++ )
        if( rTabInfo.IsExportTab( nC ) )
        {
            rDoc.GetName( nC, aTmpString );
            *pTabNames << aTmpString;
        }

    if ( rR.eDateiTyp < Biff8 )
    {
        // global link table: EXTERNCOUNT, EXTERNSHEET, NAME
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_NAME ) );

        Add( new XclExpWindowProtection(false) );
        Add( new XclExpDocProtection(rDoc.IsDocProtected() == TRUE) );
        Add( new XclExpBoolRecord( 0x0013, false ) );   // PASSWORD
        Add( new XclExpWindow1( GetRoot() ) );
        Add( new ExcDummy_040 );
        Add( new Exc1904( rDoc ) );
        Add( new ExcDummy_041 );

        // Formatting: FONT, FORMAT, XF, STYLE, PALETTE
        aRecList.AppendRecord( CreateRecord( EXC_ID_FONT ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_FORMAT ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_XF ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_PALETTE ) );

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
        if(rDoc.GetExtDocOptions())
            Add( new XclExpWindowProtection(rDoc.GetExtDocOptions()->IsWinProtected()) );
        else
            Add( new XclExpWindowProtection(false));
        Add( new XclExpDocProtection(rDoc.IsDocProtected() == TRUE) );
        Add( new ExcDummy8_040 );
        Add( new XclExpWindow1( GetRoot() ) );
        Add( new Exc1904( rDoc ) );
        Add( new ExcDummy8_041 );

        // Formatting: FONT, FORMAT, XF, STYLE, PALETTE
        aRecList.AppendRecord( CreateRecord( EXC_ID_FONT ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_FORMAT ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_XF ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_PALETTE ) );

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
        Add( new ExcDummy8_UsesElfs );

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


void ExcTable::FillAsTable()
{
    InitializeTable( mnScTab );

    RootData& rR = *mpRD;
    XclBiff eBiff = GetBiff();
    ScDocument& rDoc = GetDoc();
    XclExpTabInfo& rTabInfo = GetTabInfo();
    XclExpXFBuffer& rXFBuffer = GetXFBuffer();

    if( mnScTab >= rTabInfo.GetScTabCount() )
    {
        if( CodenameList* pL = GetExtDocOptions().GetCodenames() )
            if( const String* p = pL->Next() )
                NullTab( p );
        return;
    }

    SvNumberFormatter&      rFormatter = *rR.pDoc->GetFormatTable();
    const BiffTyp           eDateiTyp = rR.eDateiTyp;

    DBG_ASSERT( (mnScTab >= 0L) && (mnScTab <= MAXTAB), "-ExcTable::Table(): mnScTab - no ordinary table!" );
    DBG_ASSERT( nExcTab <= static_cast<sal_uInt16>(MAXTAB), "-ExcTable::Table(): nExcTab - no ordinary table!" );

    if ( eBiff >= xlBiff8 )
        // list holding OBJ records and creating MSODRAWING per-sheet data
        rR.pObjRecs = new XclObjList( GetRoot() );

    // cell table: DEFROWHEIGHT, DEFCOLWIDTH, COLINFO, DIMENSIONS, ROW, cell records
    mxCellTable.reset( new XclExpCellTable( GetRoot() ) );


    // WSBOOL needs data from page settings, create it here, add it later
    ScfRef< XclExpPageSettings > xPageSett( new XclExpPageSettings( GetRoot() ) );
    bool bFitToPages = xPageSett->GetPageData().mbFitToPages;

    if( eBiff <= xlBiff7 )
    {
        Add( new ExcBof );
        Add( new ExcDummy_02a );
    }
    else
    {
        Add( new ExcBof8 );
        Add( new XclCalccount( rDoc ) );
        Add( new XclRefmode() );
        Add( new XclIteration( rDoc ) );
        Add( new XclDelta( rDoc ) );
        Add( new ExcDummy8_02 );
    }

    // GUTS (count & size of outline icons)
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_GUTS ) );
    // DEFROWHEIGHT, created by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_DEFROWHEIGHT ) );

    // COUNTRY - in BIFF5/7 in every worksheet
    if( eBiff <= xlBiff7 )
        Add( new XclExpCountry( GetRoot() ) );

    Add( new XclExpWsbool( bFitToPages ) );

    // page settings (SETUP and various other records)
    aRecList.AppendRecord( xPageSett );

    if( rDoc.IsTabProtected( mnScTab ) )
        Add( new XclProtection() );

    // local link table: EXTERNCOUNT, EXTERNSHEET
    if( eBiff <= xlBiff7 )
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );

    if ( eBiff >= xlBiff8 )
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
    if( eBiff >= xlBiff8 )
        Add( new XclExpLabelranges( GetRoot() ) );
    // data validation (DVAL and list of DV records), generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_DVAL ) );

    if( eBiff <= xlBiff7 )
    {
        Add( new XclExpWindow2( GetRoot(), mnScTab ) );
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

        // pivot tables
        aRecList.AppendRecord( GetPivotTableManager().CreatePivotTablesRecord( mnScTab ) );

        // WINDOW2
        Add( new XclExpWindow28( GetRoot(), mnScTab ) );
    }

    // list of NOTE records, generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_NOTE ) );

    if( eBiff >= xlBiff8 )
    {
        // web queries
        Add( new XclExpWebQueryBuffer( GetRoot() ) );

        // conditional formats
        Add( new XclExpCondFormatBuffer( GetRoot() ) );

        if( rR.bWriteVBAStorage )
            if( CodenameList* pL = GetExtDocOptions().GetCodenames() )
                if( const String* p = nExcTab ? pL->Next() : pL->First() )
                    Add( new XclCodename( *p ) );
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


void ExcTable::NullTab( const String* pCodename )
{
    DBG_ASSERT( (mnScTab >= 0L) && (mnScTab <= MAXTAB), "-ExcTable::Table(): mnScTab - no ordinary table!" );
    DBG_ASSERT( nExcTab <= static_cast<sal_uInt16>(MAXTAB), "-ExcTable::Table(): nExcTab - no ordinary table!" );

    RootData& rR = *mpRD;

    if ( rR.eDateiTyp < Biff8 )
    {
        Add( new ExcBof );
        Add( new XclExpWindow2( GetRoot(), mnScTab ) );
    }
    else
    {
        Add( new ExcBof8 );

        if( pCodename )
            Add( new XclCodename( *pCodename ) );
        else
        {
            // create at least the MSODRAWING per-sheet data
            rR.pObjRecs = new XclObjList( GetRoot() );
            // all drawing obects
            rR.pEscher->AddSdrPage( rR );
            //! close Escher group shape and ESCHER_DgContainer
            //! opened by XclObjList ctor MSODRAWING
            rR.pObjRecs->EndSheet();
            Add( rR.pObjRecs );
        }
        // WINDOW2
        Add( new XclExpWindow28( GetRoot(), mnScTab ) );
    }
    Add( new ExcEof );
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

    SCTAB nScTabCount = ::std::max< SCTAB >(
        GetTabInfo().GetScTabCount(), static_cast<SCTAB>(GetTabInfo().GetXclCodenameCount()) );

    for( SCTAB nScTab = 0; nScTab < nScTabCount; ++nScTab )
    {
        if( GetTabInfo().IsExportTab( nScTab ) )
        {
            ExcTableList::RecordRefType xTab( new ExcTable( GetRoot(), nScTab ) );
            maTableList.AppendRecord( xTab );
            xTab->FillAsTable();
        }
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
        InitializeSave();

        if ( GetBiff() >= xlBiff8 )
            mpRD->pEscher->GetStrm().Seek(0);   // ready for take off

        XclExpStream aXclStrm( rSvStrm, GetRoot() );

        aHeader.Write( aXclStrm );

        DBG_ASSERT( maTableList.Size() == maBoundsheetList.Size(),
            "ExcDocument::Write - different number of sheets and BOUNDSHEET records" );

        for( size_t nTab = 0, nTabCount = maTableList.Size(); nTab < nTabCount; ++nTab )
        {
            // set current stream position in BOUNDSHEET record
            ExcBoundsheetRef xBoundsheet = maBoundsheetList.GetRecord( nTab );
            if( xBoundsheet.get() )
                xBoundsheet->SetStreamPos( aXclStrm.GetStreamPos() );
            // write the table
            maTableList.GetRecord( nTab )->Write( aXclStrm );
        }

        // write the table stream positions into the BOUNDSHEET records
        for( size_t nBSheet = 0, nBSheetCount = maBoundsheetList.Size(); nBSheet < nBSheetCount; ++nBSheet )
            maBoundsheetList.GetRecord( nBSheet )->UpdateStreamPos( aXclStrm );
    }
    if( pExpChangeTrack )
        pExpChangeTrack->Write();
}

