/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "scitems.hxx"

#include <comphelper/processfactory.hxx>
#include <svx/svdobj.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svl/intitem.hxx>
#include <svl/zformat.hxx>
#include <sot/storage.hxx>
#include <sfx2/objsh.hxx>
#include <rtl/ustring.hxx>

#include "cell.hxx"
#include "dociter.hxx"
#include "document.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
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
#include "tabprotection.hxx"
#include "postit.hxx"

#include "excdoc.hxx"
#include "namebuff.hxx"
#include "xeextlst.hxx"

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

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <oox/token/tokens.hxx>
#include <boost/shared_ptr.hpp>

using ::rtl::OString;
using namespace oox;

static String lcl_GetVbaTabName( SCTAB n )
{
    String  aRet( RTL_CONSTASCII_USTRINGPARAM( "__VBA__" ) );
    aRet += String::CreateFromInt32( static_cast<sal_uInt16>(n) );
    return aRet;
}


static void lcl_AddBookviews( XclExpRecordList<>& aRecList, ExcTable& self )
{
    aRecList.AppendNewRecord( new XclExpXmlStartElementRecord( XML_bookViews ) );
    aRecList.AppendNewRecord( new XclExpWindow1( self.GetRoot() ) );
    aRecList.AppendNewRecord( new XclExpXmlEndElementRecord( XML_bookViews ) );
}

static void lcl_AddCalcPr( XclExpRecordList<>& aRecList, ExcTable& self )
{
    ScDocument& rDoc = self.GetDoc();

    aRecList.AppendNewRecord( new XclExpXmlStartSingleElementRecord( XML_calcPr ) );
    // OOXTODO: calcCompleted, calcId, calcMode, calcOnSave,
    //          concurrentCalc, concurrentManualCount,
    //          forceFullCalc, fullCalcOnLoad, fullPrecision
    aRecList.AppendNewRecord( new XclCalccount( rDoc ) );
    aRecList.AppendNewRecord( new XclRefmode( rDoc ) );
    aRecList.AppendNewRecord( new XclIteration( rDoc ) );
    aRecList.AppendNewRecord( new XclDelta( rDoc ) );
    aRecList.AppendNewRecord( new XclExpBoolRecord(0x005F, true) ); // SAVERECALC
    aRecList.AppendNewRecord( new XclExpXmlEndSingleElementRecord() );  // XML_calcPr
}

static void lcl_AddWorkbookProtection( XclExpRecordList<>& aRecList, ExcTable& self )
{
    aRecList.AppendNewRecord( new XclExpXmlStartSingleElementRecord( XML_workbookProtection ) );

    const ScDocProtection* pProtect = self.GetDoc().GetDocProtection();
    if (pProtect && pProtect->isProtected())
    {
        aRecList.AppendNewRecord( new XclExpWindowProtection(pProtect->isOptionEnabled(ScDocProtection::WINDOWS)) );
        aRecList.AppendNewRecord( new XclExpProtection(pProtect->isOptionEnabled(ScDocProtection::STRUCTURE)) );
        aRecList.AppendNewRecord( new XclExpPassHash(pProtect->getPasswordHash(PASSHASH_XL)) );
    }

    aRecList.AppendNewRecord( new XclExpXmlEndSingleElementRecord() );   // XML_workbookProtection
}

static void lcl_AddScenariosAndFilters( XclExpRecordList<>& aRecList, const XclExpRoot& rRoot, SCTAB nScTab )
{
    // Scenarios
    aRecList.AppendNewRecord( new ExcEScenarioManager( rRoot, nScTab ) );
    // filter
    aRecList.AppendRecord( rRoot.GetFilterManager().CreateRecord( nScTab ) );
}


ExcTable::ExcTable( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnScTab( SCTAB_GLOBAL ),
    nExcTab( EXC_NOTAB ),
    pTabNames( new NameBuffer( 0, 16 ) ),
    mxNoteList( new XclExpNoteList )
{
}


ExcTable::ExcTable( const XclExpRoot& rRoot, SCTAB nScTab ) :
    XclExpRoot( rRoot ),
    mnScTab( nScTab ),
    nExcTab( rRoot.GetTabInfo().GetXclTab( nScTab ) ),
    pTabNames( new NameBuffer( 0, 16 ) ),
    mxNoteList( new XclExpNoteList )
{
}


ExcTable::~ExcTable()
{
    delete pTabNames;
}


void ExcTable::Add( XclExpRecordBase* pRec )
{
    OSL_ENSURE( pRec, "-ExcTable::Add(): pRec ist NULL!" );
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
    rtl::OUString aTmpString;
    SCTAB  nScTabCount     = rTabInfo.GetScTabCount();
    sal_uInt16  nExcTabCount    = rTabInfo.GetXclTabCount();
    sal_uInt16  nCodenames      = static_cast< sal_uInt16 >( GetExtDocOptions().GetCodeNameCount() );

    SfxObjectShell* pShell = GetDocShell();
    sal_uInt16 nWriteProtHash = pShell ? pShell->GetModifyPasswordHash() : 0;
    bool bRecommendReadOnly = pShell && pShell->IsLoadReadonly();

    if( (nWriteProtHash > 0) || bRecommendReadOnly )
        Add( new XclExpEmptyRecord( EXC_ID_WRITEPROT ) );

    // TODO: correct codepage for BIFF5?
    sal_uInt16 nCodePage = XclTools::GetXclCodePage( (GetBiff() <= EXC_BIFF5) ? RTL_TEXTENCODING_MS_1252 : RTL_TEXTENCODING_UNICODE );

    if( GetBiff() <= EXC_BIFF5 )
    {
        Add( new XclExpEmptyRecord( EXC_ID_INTERFACEHDR ) );
        Add( new XclExpUInt16Record( EXC_ID_MMS, 0 ) );
        Add( new XclExpEmptyRecord( EXC_ID_TOOLBARHDR ) );
        Add( new XclExpEmptyRecord( EXC_ID_TOOLBAREND ) );
        Add( new XclExpEmptyRecord( EXC_ID_INTERFACEEND ) );
        Add( new ExcDummy_00 );
    }
    else
    {
        if( IsDocumentEncrypted() )
            Add( new XclExpFileEncryption( GetRoot() ) );
        Add( new XclExpInterfaceHdr( nCodePage ) );
        Add( new XclExpUInt16Record( EXC_ID_MMS, 0 ) );
        Add( new XclExpInterfaceEnd );
        Add( new XclExpWriteAccess );
    }

    Add( new XclExpFileSharing( GetRoot(), nWriteProtHash, bRecommendReadOnly ) );
    Add( new XclExpUInt16Record( EXC_ID_CODEPAGE, nCodePage ) );

    if( GetBiff() == EXC_BIFF8 )
    {
        Add( new XclExpBoolRecord( EXC_ID_DSF, false ) );
        Add( new XclExpEmptyRecord( EXC_ID_XL9FILE ) );
        rR.pTabId = new XclExpChTrTabId( Max( nExcTabCount, nCodenames ) );
        Add( rR.pTabId );
        if( HasVbaStorage() )
        {
            Add( new XclObproj );
            const String& rCodeName = GetExtDocOptions().GetDocSettings().maGlobCodeName;
            if( rCodeName.Len() )
                Add( new XclCodename( rCodeName ) );
        }
    }

    Add( new XclExpUInt16Record( EXC_ID_FNGROUPCOUNT, 14 ) );

    // erst Namen- und Tabellen-Eintraege aufbauen

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

    // document protection options
    if( GetOutput() == EXC_OUTPUT_BINARY )
    {
        lcl_AddWorkbookProtection( aRecList, *this );

        if( GetBiff() == EXC_BIFF8 )
        {
            Add( new XclExpProt4Rev );
            Add( new XclExpProt4RevPass );
        }

        lcl_AddBookviews( aRecList, *this );
    }

    Add( new XclExpXmlStartSingleElementRecord( XML_workbookPr ) );
    if ( GetBiff() == EXC_BIFF8 && GetOutput() != EXC_OUTPUT_BINARY )
    {
        Add( new XclExpBoolRecord(0x0040, false, XML_backupFile ) );    // BACKUP
        Add( new XclExpBoolRecord(0x008D, false, XML_showObjects ) );   // HIDEOBJ
    }

    if ( GetBiff() == EXC_BIFF8 )
    {
        Add( new XclExpBoolRecord(0x0040, false) ); // BACKUP
        Add( new XclExpBoolRecord(0x008D, false) ); // HIDEOBJ
    }

    if( GetBiff() <= EXC_BIFF5 )
    {
        Add( new ExcDummy_040 );
        Add( new Exc1904( rDoc ) );
        Add( new ExcDummy_041 );
    }
    else
    {
        // BIFF8
        Add( new Exc1904( rDoc ) );
        Add( new XclExpBoolRecord( 0x000E, !rDoc.GetDocOptions().IsCalcAsShown() ) );
        Add( new XclExpBoolRecord(0x01B7, false) ); // REFRESHALL
        Add( new XclExpBoolRecord(0x00DA, false) ); // BOOKBOOL
        // OOXTODO: The following /workbook/workbookPr attributes are mapped
        //          to various BIFF records that are not currently supported:
        //
        //          XML_allowRefreshQuery:          QSISTAG 802h: fEnableRefresh
        //          XML_autoCompressPictures:       COMPRESSPICTURES 89Bh: fAutoCompressPictures
        //          XML_checkCompatibility:         COMPAT12 88Ch: fNoCompatChk
        //          XML_codeName:                   "Calc"
        //          XML_defaultThemeVersion:        ???
        //          XML_filterPrivacy:              BOOKEXT 863h: fFilterPrivacy
        //          XML_hidePivotFieldList:         BOOKBOOL DAh: fHidePivotTableFList
        //          XML_promptedSolutions:          BOOKEXT 863h: fBuggedUserAboutSolution
        //          XML_publishItems:               NAMEPUBLISH 893h: fPublished
        //          XML_saveExternalLinkValues:     BOOKBOOL DAh: fNoSavSupp
        //          XML_showBorderUnselectedTables: BOOKBOOL DAh: fHideBorderUnsels
        //          XML_showInkAnnotation:          BOOKEXT 863h: fShowInkAnnotation
        //          XML_showPivotChart:             PIVOTCHARTBITS 859h: fGXHide??
        //          XML_updateLinks:                BOOKBOOL DAh: grbitUpdateLinks
    }
    Add( new XclExpXmlEndSingleElementRecord() );   // XML_workbookPr

    // Formatting: FONT, FORMAT, XF, STYLE, PALETTE
    if( GetOutput() != EXC_OUTPUT_BINARY )
    {
        aRecList.AppendNewRecord( new XclExpXmlStyleSheet( *this ) );
    }
    else
    {
        aRecList.AppendRecord( CreateRecord( EXC_ID_FONTLIST ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_FORMATLIST ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_XFLIST ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_PALETTE ) );
    }


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

        if( GetOutput() != EXC_OUTPUT_BINARY )
        {
            lcl_AddWorkbookProtection( aRecList, *this );
            lcl_AddBookviews( aRecList, *this );
        }

        // Bundlesheet
        aRecList.AppendNewRecord( new XclExpXmlStartElementRecord( XML_sheets ) );
        for( nC = 0 ; nC < nScTabCount ; nC++ )
            if( rTabInfo.IsExportTab( nC ) )
            {
                ExcBoundsheetList::RecordRefType xBoundsheet( new ExcBundlesheet8( rR, nC ) );
                aRecList.AppendRecord( xBoundsheet );
                rBoundsheetList.AppendRecord( xBoundsheet );
            }
        aRecList.AppendNewRecord( new XclExpXmlEndElementRecord( XML_sheets ) );

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

        if( GetOutput() != EXC_OUTPUT_BINARY )
            lcl_AddCalcPr( aRecList, *this );

        Add( new XclExpRecalcId );

        // MSODRAWINGGROUP per-document data
        aRecList.AppendRecord( GetObjectManager().CreateDrawingGroup() );
        // Shared string table: SST, EXTSST
        aRecList.AppendRecord( CreateRecord( EXC_ID_SST ) );

        Add( new XclExpBookExt );
    }

    Add( new ExcEof );
}


void ExcTable::FillAsTable( SCTAB nCodeNameIdx )
{
    InitializeTable( mnScTab );

    RootData& rR = GetOldRoot();
    XclBiff eBiff = GetBiff();
    ScDocument& rDoc = GetDoc();

    OSL_ENSURE( (mnScTab >= 0L) && (mnScTab <= MAXTAB), "-ExcTable::Table(): mnScTab - no ordinary table!" );
    OSL_ENSURE( nExcTab <= static_cast<sal_uInt16>(MAXTAB), "-ExcTable::Table(): nExcTab - no ordinary table!" );

    // create a new OBJ list for this sheet (may be used by notes, autofilter, data validation)
    if( eBiff == EXC_BIFF8 )
        GetObjectManager().StartSheet();

    // cell table: DEFROWHEIGHT, DEFCOLWIDTH, COLINFO, DIMENSIONS, ROW, cell records
    mxCellTable.reset( new XclExpCellTable( GetRoot() ) );

    //export cell notes
    ScNotes::iterator itr = rDoc.GetNotes(mnScTab)->begin();
    ScNotes::iterator itrEnd = rDoc.GetNotes(mnScTab)->end();
    for (; itr != itrEnd; ++itr)
    {
        // notes
        const ScPostIt* pScNote = itr->second;
        ScAddress aScPos( itr->first.first, itr->first.second, mnScTab );
        mxNoteList->AppendNewRecord( new XclExpNote( GetRoot(), aScPos, pScNote, rtl::OUString() ) );
    }

    if( GetOutput() != EXC_OUTPUT_BINARY )
    {
        FillAsXmlTable( nCodeNameIdx );
        return;
    }


    // WSBOOL needs data from page settings, create it here, add it later
    boost::shared_ptr< XclExpPageSettings > xPageSett( new XclExpPageSettings( GetRoot() ) );
    bool bFitToPages = xPageSett->GetPageData().mbFitToPages;

    if( eBiff <= EXC_BIFF5 )
    {
        Add( new ExcBof );
        Add( new ExcDummy_02a );
    }
    else
    {
        Add( new ExcBof8 );
        lcl_AddCalcPr( aRecList, *this );
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

    const ScTableProtection* pTabProtect = rDoc.GetTabProtection(mnScTab);
    if (pTabProtect && pTabProtect->isProtected())
    {
        Add( new XclExpProtection(true) );
        Add( new XclExpBoolRecord(0x00DD, pTabProtect->isOptionEnabled(ScTableProtection::SCENARIOS)) );
        Add( new XclExpBoolRecord(0x0063, pTabProtect->isOptionEnabled(ScTableProtection::OBJECTS)) );
        Add( new XclExpPassHash(pTabProtect->getPasswordHash(PASSHASH_XL)) );
    }

    // local link table: EXTERNCOUNT, EXTERNSHEET
    if( eBiff <= EXC_BIFF5 )
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );

    if ( eBiff == EXC_BIFF8 )
        lcl_AddScenariosAndFilters( aRecList, GetRoot(), mnScTab );

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
        // all MSODRAWING and OBJ stuff of this sheet goes here
        aRecList.AppendRecord( GetObjectManager().ProcessDrawing( GetSdrPage( mnScTab ) ) );
        // pivot tables
        aRecList.AppendRecord( GetPivotTableManager().CreatePivotTablesRecord( mnScTab ) );
    }

    // list of NOTE records, generated by the cell table
    aRecList.AppendRecord( mxNoteList );

    // sheet view settings: WINDOW2, SCL, PANE, SELECTION
    aRecList.AppendNewRecord( new XclExpTabViewSettings( GetRoot(), mnScTab ) );

    if( eBiff == EXC_BIFF8 )
    {
        // sheet protection options
        Add( new XclExpSheetProtectOptions( GetRoot(), mnScTab ) );

        // web queries
        Add( new XclExpWebQueryBuffer( GetRoot() ) );

        // conditional formats
        Add( new XclExpCondFormatBuffer( GetRoot(), XclExtLstRef() ) );

        if( HasVbaStorage() )
            if( nCodeNameIdx < GetExtDocOptions().GetCodeNameCount() )
                Add( new XclCodename( GetExtDocOptions().GetCodeName( nCodeNameIdx ) ) );
    }

    // list of HLINK records, generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_HLINK ) );

    // change tracking
    if( rR.pUserBViewList )
    {
        XclExpUserBViewList::const_iterator iter;
        for ( iter = rR.pUserBViewList->begin(); iter != rR.pUserBViewList->end(); ++iter)
        {
            Add( new XclExpUsersViewBegin( (*iter)->GetGUID(), nExcTab ) );
            Add( new XclExpUsersViewEnd );
        }
    }

    // EOF
    Add( new ExcEof );
}

void ExcTable::FillAsXmlTable( SCTAB nCodeNameIdx )
{
    RootData& rR = GetOldRoot();

    // WSBOOL needs data from page settings, create it here, add it later
    boost::shared_ptr< XclExpPageSettings > xPageSett( new XclExpPageSettings( GetRoot() ) );
    XclExtLstRef xExtLst( new XclExtLst( GetRoot() ) );
    bool bFitToPages = xPageSett->GetPageData().mbFitToPages;

    Add( new ExcBof8 );

    Add( new XclExpWsbool( bFitToPages, mnScTab, &GetFilterManager() ) );

    // GUTS (count & size of outline icons)
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_GUTS ) );
    // DEFROWHEIGHT, created by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID2_DEFROWHEIGHT ) );

    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID3_DIMENSIONS ) );

    // sheet view settings: WINDOW2, SCL, PANE, SELECTION
    aRecList.AppendNewRecord( new XclExpTabViewSettings( GetRoot(), mnScTab ) );

    // cell table: DEFCOLWIDTH, COLINFO, DIMENSIONS, ROW, cell records
    aRecList.AppendRecord( mxCellTable );

    // label ranges
    Add( new XclExpLabelranges( GetRoot() ) );

    // DFF not needed in MSOOXML export
//    GetObjectManager().AddSdrPage();
//    //! close Escher group shape and ESCHER_DgContainer
//    //! opened by XclExpObjList ctor MSODRAWING
//    rR.pObjRecs->EndSheet();
//    // all MSODRAWING and OBJ stuff of this sheet goes here
//    Add( rR.pObjRecs );

    // pivot tables
    aRecList.AppendRecord( GetPivotTableManager().CreatePivotTablesRecord( mnScTab ) );

    // list of NOTE records, generated by the cell table
    if( mxNoteList != NULL && !mxNoteList->IsEmpty() )
        aRecList.AppendNewRecord( new XclExpComments( mnScTab, *mxNoteList ) );

    // web queries
    Add( new XclExpWebQueryBuffer( GetRoot() ) );

    lcl_AddScenariosAndFilters( aRecList, GetRoot(), mnScTab );

    // MERGEDCELLS record, generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_MERGEDCELLS ) );

    // conditional formats
    Add( new XclExpCondFormatBuffer( GetRoot(), xExtLst ) );

    if( HasVbaStorage() )
        if( nCodeNameIdx < GetExtDocOptions().GetCodeNameCount() )
            Add( new XclCodename( GetExtDocOptions().GetCodeName( nCodeNameIdx ) ) );

    // data validation (DVAL and list of DV records), generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_DVAL ) );

    // list of HLINK records, generated by the cell table
    XclExpRecordRef xHyperlinks = mxCellTable->CreateRecord( EXC_ID_HLINK );
    XclExpHyperlinkList* xHyperlinkList = dynamic_cast<XclExpHyperlinkList*>(xHyperlinks.get());
    if( xHyperlinkList != NULL && !xHyperlinkList->IsEmpty() )
    {
        aRecList.AppendNewRecord( new XclExpXmlStartElementRecord( XML_hyperlinks ) );
        aRecList.AppendRecord( xHyperlinks );
        aRecList.AppendNewRecord( new XclExpXmlEndElementRecord( XML_hyperlinks ) );
    }

    aRecList.AppendRecord( xPageSett );

    // change tracking
    if( rR.pUserBViewList )
    {
        XclExpUserBViewList::const_iterator iter;
        for ( iter = rR.pUserBViewList->begin(); iter != rR.pUserBViewList->end(); ++iter )
        {
            Add( new XclExpUsersViewBegin( (*iter)->GetGUID(), nExcTab ) );
            Add( new XclExpUsersViewEnd );
        }
    }

    // all MSODRAWING and OBJ stuff of this sheet goes here
    aRecList.AppendRecord( GetObjectManager().ProcessDrawing( GetSdrPage( mnScTab ) ) );

    aRecList.AppendRecord( xExtLst );

    // EOF
    Add( new ExcEof );
}


void ExcTable::FillAsEmptyTable( SCTAB nCodeNameIdx )
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


void ExcTable::Write( XclExpStream& rStrm )
{
    SetCurrScTab( mnScTab );
    if( mxCellTable.get() )
        mxCellTable->Finalize();
    aRecList.Save( rStrm );
}


void ExcTable::WriteXml( XclExpXmlStream& rStrm )
{
    if (GetTabInfo().IsExportTab( mnScTab ) )
    {
        // worksheet export
        String sSheetName = XclXmlUtils::GetStreamName( "xl/", "worksheets/sheet", mnScTab+1 );

        sax_fastparser::FSHelperPtr pWorksheet = rStrm.GetStreamForPath( sSheetName );

        rStrm.PushStream( pWorksheet );

        pWorksheet->startElement( XML_worksheet,
                XML_xmlns, "http://schemas.openxmlformats.org/spreadsheetml/2006/main",
                FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
                FSEND );
    }

    SetCurrScTab( mnScTab );
    if( mxCellTable.get() )
        mxCellTable->Finalize();
    aRecList.SaveXml( rStrm );

    if (GetTabInfo().IsExportTab( mnScTab ) )
    {
        rStrm.GetCurrentStream()->endElement( XML_worksheet );
        rStrm.PopStream();
    }
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
    SCTAB nCodeNameIdx = 0, nCodeNameCount = GetExtDocOptions().GetCodeNameCount();

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
        GetObjectManager().EndDocument();

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

        XclExpStream aXclStrm( rSvStrm, GetRoot() );

        aHeader.Write( aXclStrm );

        OSL_ENSURE( maTableList.GetSize() == maBoundsheetList.GetSize(),
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

void ExcDocument::WriteXml( XclExpXmlStream& rStrm )
{
    SfxObjectShell* pDocShell = GetDocShell();

    using namespace ::com::sun::star;
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS( pDocShell->GetModel(), uno::UNO_QUERY_THROW );
    uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();

    rStrm.exportDocumentProperties( xDocProps );

    sax_fastparser::FSHelperPtr& rWorkbook = rStrm.GetCurrentStream();
    rWorkbook->startElement( XML_workbook,
            XML_xmlns, "http://schemas.openxmlformats.org/spreadsheetml/2006/main",
            FSNS(XML_xmlns, XML_r), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
            FSEND );
    rWorkbook->singleElement( XML_fileVersion,
            XML_appName, "Calc",
            // OOXTODO: XML_codeName
            // OOXTODO: XML_lastEdited
            // OOXTODO: XML_lowestEdited
            // OOXTODO: XML_rupBuild
            FSEND );

    if( !maTableList.IsEmpty() )
    {
        InitializeSave();

        aHeader.WriteXml( rStrm );

        for( size_t nTab = 0, nTabCount = maTableList.GetSize(); nTab < nTabCount; ++nTab )
        {
            // write the table
            maTableList.GetRecord( nTab )->WriteXml( rStrm );
        }
    }

    if( pExpChangeTrack )
        pExpChangeTrack->WriteXml( rStrm );

    rWorkbook->endElement( XML_workbook );
    rWorkbook.reset();

    rStrm.commitStorage();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
