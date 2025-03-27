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

#include <sfx2/objsh.hxx>
#include <rtl/ustring.hxx>

#include <document.hxx>
#include <docsh.hxx>
#include <scextopt.hxx>
#include <docoptio.hxx>
#include <tabprotection.hxx>
#include <postit.hxx>
#include <root.hxx>
#include <connectionsbuffer.hxx>
#include <connectionsfragment.hxx>

#include <excdoc.hxx>
#include <xeextlst.hxx>
#include <biffhelper.hxx>

#include <xcl97rec.hxx>
#include <xetable.hxx>
#include <xelink.hxx>
#include <xepage.hxx>
#include <xeview.hxx>
#include <xecontent.hxx>
#include <xeescher.hxx>
#include <xepivot.hxx>
#include <export/SparklineExt.hxx>
#include <XclExpChangeTrack.hxx>
#include <xepivotxml.hxx>
#include <xedbdata.hxx>
#include <xlcontent.hxx>
#include <xlname.hxx>
#include <xllink.hxx>
#include <xltools.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <o3tl/safeint.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/relationship.hxx>
#include <oox/export/ThemeExport.hxx>
#include <docmodel/theme/Theme.hxx>
#include <svx/svdpage.hxx>
#include <memory>

using namespace oox;

static OUString lcl_GetVbaTabName( SCTAB n )
{
    OUString aRet = "__VBA__"  + OUString::number( static_cast<sal_uInt16>(n) );
    return aRet;
}

static void lcl_AddBookviews( XclExpRecordList<>& aRecList, const ExcTable& self )
{
    aRecList.AppendNewRecord( new XclExpXmlStartElementRecord( XML_bookViews ) );
    aRecList.AppendNewRecord( new XclExpWindow1( self.GetRoot() ) );
    aRecList.AppendNewRecord( new XclExpXmlEndElementRecord( XML_bookViews ) );
}

static void lcl_AddCalcPr( XclExpRecordList<>& aRecList, const ExcTable& self )
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
    aRecList.AppendNewRecord( new XclExpBoolRecord(oox::xls::BIFF_ID_SAVERECALC, true) );
    aRecList.AppendNewRecord( new XclExpXmlEndSingleElementRecord() );  // XML_calcPr
}

static void lcl_AddWorkbookProtection( XclExpRecordList<>& aRecList, const ExcTable& self )
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
    mxNoteList( new XclExpNoteList )
{
}

ExcTable::ExcTable( const XclExpRoot& rRoot, SCTAB nScTab ) :
    XclExpRoot( rRoot ),
    mnScTab( nScTab ),
    nExcTab( rRoot.GetTabInfo().GetXclTab( nScTab ) ),
    mxNoteList( new XclExpNoteList )
{
}

ExcTable::~ExcTable()
{
}

void ExcTable::Add( XclExpRecordBase* pRec )
{
    OSL_ENSURE( pRec, "-ExcTable::Add(): pRec is NULL!" );
    aRecList.AppendNewRecord( pRec );
}

void ExcTable::FillAsHeaderBinary( ExcBoundsheetList& rBoundsheetList )
{
    InitializeGlobals();

    RootData& rR = GetOldRoot();
    ScDocument& rDoc = GetDoc();
    XclExpTabInfo& rTabInfo = GetTabInfo();

    if ( GetBiff() <= EXC_BIFF5 )
        Add( new ExcBofW );
    else
        Add( new ExcBofW8 );

    sal_uInt16  nExcTabCount    = rTabInfo.GetXclTabCount();
    sal_uInt16  nCodenames      = static_cast< sal_uInt16 >( GetExtDocOptions().GetCodeNameCount() );

    ScDocShell* pShell = GetDocShell();
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
        rR.pTabId = new XclExpChTrTabId( std::max( nExcTabCount, nCodenames ) );
        Add( rR.pTabId );
        if( HasVbaStorage() )
        {
            Add( new XclObproj );
            const OUString& rCodeName = GetExtDocOptions().GetDocSettings().maGlobCodeName;
            if( !rCodeName.isEmpty() )
                Add( new XclCodename( rCodeName ) );
        }
    }

    Add( new XclExpUInt16Record( EXC_ID_FNGROUPCOUNT, 14 ) );

    if ( GetBiff() <= EXC_BIFF5 )
    {
        // global link table: EXTERNCOUNT, EXTERNSHEET, NAME
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_NAME ) );
    }

    // document protection options
    lcl_AddWorkbookProtection( aRecList, *this );

    if( GetBiff() == EXC_BIFF8 )
    {
        Add( new XclExpProt4Rev );
        Add( new XclExpProt4RevPass );
    }

    lcl_AddBookviews( aRecList, *this );

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
    }

    // Formatting: FONT, FORMAT, XF, STYLE, PALETTE
    aRecList.AppendRecord( CreateRecord( EXC_ID_FONTLIST ) );
    aRecList.AppendRecord( CreateRecord( EXC_ID_FORMATLIST ) );
    aRecList.AppendRecord( CreateRecord( EXC_ID_XFLIST ) );
    aRecList.AppendRecord( CreateRecord( EXC_ID_PALETTE ) );

    SCTAB   nC;
    SCTAB  nScTabCount     = rTabInfo.GetScTabCount();
    if( GetBiff() <= EXC_BIFF5 )
    {
        // Bundlesheet
        for( nC = 0 ; nC < nScTabCount ; nC++ )
            if( rTabInfo.IsExportTab( nC ) )
            {
                ExcBoundsheetList::RecordRefType xBoundsheet = new ExcBundlesheet( rR, nC );
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
                ExcBoundsheetList::RecordRefType xBoundsheet = new ExcBundlesheet8( rR, nC );
                aRecList.AppendRecord( xBoundsheet );
                rBoundsheetList.AppendRecord( xBoundsheet );
            }

        OUString aTmpString;
        for( SCTAB nAdd = 0; nC < static_cast<SCTAB>(nCodenames) ; nC++, nAdd++ )
        {
            aTmpString = lcl_GetVbaTabName( nAdd );
            ExcBoundsheetList::RecordRefType xBoundsheet = new ExcBundlesheet8( aTmpString );
            aRecList.AppendRecord( xBoundsheet );
            rBoundsheetList.AppendRecord( xBoundsheet );
        }

        // COUNTRY - in BIFF8 in workbook globals
        Add( new XclExpCountry( GetRoot() ) );

        // link table: SUPBOOK, XCT, CRN, EXTERNNAME, EXTERNSHEET, NAME
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_NAME ) );

        Add( new XclExpRecalcId );

        // MSODRAWINGGROUP per-document data
        aRecList.AppendRecord( GetObjectManager().CreateDrawingGroup() );
        // Shared string table: SST, EXTSST
        aRecList.AppendRecord( CreateRecord( EXC_ID_SST ) );

        Add( new XclExpBookExt );
    }

    Add( new ExcEof );
}

void ExcTable::FillAsHeaderXml( ExcBoundsheetList& rBoundsheetList )
{
    InitializeGlobals();

    RootData& rR = GetOldRoot();
    ScDocument& rDoc = GetDoc();
    XclExpTabInfo& rTabInfo = GetTabInfo();

    sal_uInt16  nExcTabCount    = rTabInfo.GetXclTabCount();
    sal_uInt16  nCodenames      = static_cast< sal_uInt16 >( GetExtDocOptions().GetCodeNameCount() );

    rR.pTabId = new XclExpChTrTabId( std::max( nExcTabCount, nCodenames ) );
    Add( rR.pTabId );

    Add( new XclExpXmlStartSingleElementRecord( XML_workbookPr ) );
    Add( new XclExpBoolRecord(0x0040, false, XML_backupFile ) );    // BACKUP
    Add( new XclExpBoolRecord(0x008D, false, XML_showObjects ) );   // HIDEOBJ

    Add( new Exc1904( rDoc ) );
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
    Add( new XclExpXmlEndSingleElementRecord() );   // XML_workbookPr

    // Formatting: FONT, FORMAT, XF, STYLE, PALETTE
    aRecList.AppendNewRecord( new XclExpXmlStyleSheet( *this ) );

    // Change tracking
    if( rDoc.GetChangeTrack() )
    {
        rR.pUserBViewList = new XclExpUserBViewList( *rDoc.GetChangeTrack() );
        Add( rR.pUserBViewList );
    }

    lcl_AddWorkbookProtection( aRecList, *this );
    lcl_AddBookviews( aRecList, *this );

    // Bundlesheet
    SCTAB nC;
    SCTAB nScTabCount = rTabInfo.GetScTabCount();
    aRecList.AppendNewRecord( new XclExpXmlStartElementRecord( XML_sheets ) );
    for( nC = 0 ; nC < nScTabCount ; nC++ )
        if( rTabInfo.IsExportTab( nC ) )
        {
            ExcBoundsheetList::RecordRefType xBoundsheet = new ExcBundlesheet8( rR, nC );
            aRecList.AppendRecord( xBoundsheet );
            rBoundsheetList.AppendRecord( xBoundsheet );
        }
    aRecList.AppendNewRecord( new XclExpXmlEndElementRecord( XML_sheets ) );

    OUString aTmpString;
    for( SCTAB nAdd = 0; nC < static_cast<SCTAB>(nCodenames) ; nC++, nAdd++ )
    {
        aTmpString = lcl_GetVbaTabName( nAdd );
        ExcBoundsheetList::RecordRefType xBoundsheet = new ExcBundlesheet8( aTmpString );
        aRecList.AppendRecord( xBoundsheet );
        rBoundsheetList.AppendRecord( xBoundsheet );
    }

    // link table: SUPBOOK, XCT, CRN, EXTERNNAME, EXTERNSHEET, NAME
    aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );
    aRecList.AppendRecord( CreateRecord( EXC_ID_NAME ) );

    lcl_AddCalcPr( aRecList, *this );

    // MSODRAWINGGROUP per-document data
    aRecList.AppendRecord( GetObjectManager().CreateDrawingGroup() );
    // Shared string table: SST, EXTSST
    aRecList.AppendRecord( CreateRecord( EXC_ID_SST ) );
}

void ExcTable::FillAsTableBinary( SCTAB nCodeNameIdx )
{
    InitializeTable( mnScTab );

    RootData& rR = GetOldRoot();
    XclBiff eBiff = GetBiff();
    ScDocument& rDoc = GetDoc();

    OSL_ENSURE( (mnScTab >= 0) && (mnScTab <= MAXTAB), "-ExcTable::Table(): mnScTab - no ordinary table!" );
    OSL_ENSURE( nExcTab <= o3tl::make_unsigned(MAXTAB), "-ExcTable::Table(): nExcTab - no ordinary table!" );

    // create a new OBJ list for this sheet (may be used by notes, autofilter, data validation)
    if( eBiff == EXC_BIFF8 )
        GetObjectManager().StartSheet();

    // cell table: DEFROWHEIGHT, DEFCOLWIDTH, COLINFO, DIMENSIONS, ROW, cell records
    mxCellTable = new XclExpCellTable( GetRoot() );

    //export cell notes
    std::vector<sc::NoteEntry> aNotes;
    rDoc.GetAllNoteEntries(aNotes);
    for (const auto& rNote : aNotes)
    {
        if (rNote.maPos.Tab() != mnScTab)
            continue;

        mxNoteList->AppendNewRecord(new XclExpNote(GetRoot(), rNote.maPos, rNote.mpNote, u""));
    }

    // WSBOOL needs data from page settings, create it here, add it later
    rtl::Reference<XclExpPageSettings> xPageSett = new XclExpPageSettings( GetRoot() );
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
        Add( new XclExpBoolRecord(oox::xls::BIFF_ID_SCENPROTECT, pTabProtect->isOptionEnabled(ScTableProtection::SCENARIOS)) );
        if (pTabProtect->isOptionEnabled(ScTableProtection::OBJECTS))
            Add( new XclExpBoolRecord(oox::xls::BIFF_ID_OBJECTPROTECT, true ));
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

        // enhanced protections if there are
        if (pTabProtect)
        {
            const ::std::vector<ScEnhancedProtection>& rProts( pTabProtect->getEnhancedProtection());
            for (const auto& rProt : rProts)
            {
                Add( new XclExpSheetEnhancedProtection( GetRoot(), rProt));
            }
        }

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
        for ( iter = rR.pUserBViewList->cbegin(); iter != rR.pUserBViewList->cend(); ++iter)
        {
            Add( new XclExpUsersViewBegin( (*iter).GetGUID(), nExcTab ) );
            Add( new XclExpUsersViewEnd );
        }
    }

    // EOF
    Add( new ExcEof );
}

void ExcTable::FillAsTableXml()
{
    InitializeTable( mnScTab );

    ScDocument& rDoc = GetDoc();

    OSL_ENSURE( (mnScTab >= 0) && (mnScTab <= MAXTAB), "-ExcTable::Table(): mnScTab - no ordinary table!" );
    OSL_ENSURE( nExcTab <= o3tl::make_unsigned(MAXTAB), "-ExcTable::Table(): nExcTab - no ordinary table!" );

    // create a new OBJ list for this sheet (may be used by notes, autofilter, data validation)
    GetObjectManager().StartSheet();

    // cell table: DEFROWHEIGHT, DEFCOLWIDTH, COLINFO, DIMENSIONS, ROW, cell records
    mxCellTable = new XclExpCellTable( GetRoot() );

    //export cell notes
    std::vector<sc::NoteEntry> aNotes;
    rDoc.GetAllNoteEntries(aNotes);
    for (const auto& rNote : aNotes)
    {
        if (rNote.maPos.Tab() != mnScTab)
            continue;

        mxNoteList->AppendNewRecord(new XclExpNote(GetRoot(), rNote.maPos, rNote.mpNote, u""));
    }

    // WSBOOL needs data from page settings, create it here, add it later
    rtl::Reference<XclExpPageSettings> xPageSett = new XclExpPageSettings( GetRoot() );
    XclExtLstRef xExtLst = new XclExtLst( GetRoot() );
    bool bFitToPages = xPageSett->GetPageData().mbFitToPages;

    bool bSummaryBelow = GetRoot().GetDoc().GetTotalsRowBelow(mnScTab);
    Color aTabColor = GetRoot().GetDoc().GetTabBgColor(mnScTab);
    Add(new XclExpXmlSheetPr(bFitToPages, mnScTab, aTabColor, bSummaryBelow, &GetFilterManager()));

    // GUTS (count & size of outline icons)
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_GUTS ) );
    // DEFROWHEIGHT, created by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID2_DEFROWHEIGHT ) );

    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID3_DIMENSIONS ) );

    // sheet view settings: WINDOW2, SCL, PANE, SELECTION
    aRecList.AppendNewRecord( new XclExpTabViewSettings( GetRoot(), mnScTab ) );

    // cell table: DEFCOLWIDTH, COLINFO, DIMENSIONS, ROW, cell records
    aRecList.AppendRecord( mxCellTable );

    // list of NOTE records, generated by the cell table
    // not in the worksheet file
    if( mxNoteList != nullptr && !mxNoteList->IsEmpty() )
        aRecList.AppendNewRecord( new XclExpComments( mnScTab, *mxNoteList ) );

    const ScTableProtection* pTabProtect = rDoc.GetTabProtection(mnScTab);
    if (pTabProtect && pTabProtect->isProtected())
        Add( new XclExpSheetProtection(true, mnScTab) );

    lcl_AddScenariosAndFilters( aRecList, GetRoot(), mnScTab );

    // MERGEDCELLS record, generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_MERGEDCELLS ) );

    // conditional formats
    Add( new XclExpCondFormatBuffer( GetRoot(), xExtLst ) );

    Add(new xcl::exp::SparklineBuffer(GetRoot(), xExtLst));

    // data validation (DVAL and list of DV records), generated by the cell table
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_DVAL ) );

    // list of HLINK records, generated by the cell table
    XclExpRecordRef xHyperlinks = mxCellTable->CreateRecord( EXC_ID_HLINK );
    XclExpHyperlinkList* pHyperlinkList = dynamic_cast<XclExpHyperlinkList*>(xHyperlinks.get());
    if( pHyperlinkList != nullptr && !pHyperlinkList->IsEmpty() )
    {
        aRecList.AppendNewRecord( new XclExpXmlStartElementRecord( XML_hyperlinks ) );
        aRecList.AppendRecord( xHyperlinks );
        aRecList.AppendNewRecord( new XclExpXmlEndElementRecord( XML_hyperlinks ) );
    }

    aRecList.AppendRecord( xPageSett );

    // all MSODRAWING and OBJ stuff of this sheet goes here
    aRecList.AppendRecord( GetObjectManager().ProcessDrawing( GetSdrPage( mnScTab ) ) );

    XclExpImgData* pImgData = xPageSett->getGraphicExport();
    if (pImgData)
        aRecList.AppendRecord(pImgData);

    // <tableParts> after <drawing> and before <extLst>
    aRecList.AppendRecord( GetTablesManager().GetTablesBySheet( mnScTab));

    aRecList.AppendRecord( xExtLst );
}

void ExcTable::FillAsEmptyTable( SCTAB nCodeNameIdx )
{
    InitializeTable( mnScTab );

    if( !(HasVbaStorage() && (nCodeNameIdx < GetExtDocOptions().GetCodeNameCount())) )
        return;

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

void ExcTable::Write( XclExpStream& rStrm )
{
    SetCurrScTab( mnScTab );
    if( mxCellTable )
        mxCellTable->Finalize(true);
    aRecList.Save( rStrm );
}

void ExcTable::WriteXml( XclExpXmlStream& rStrm )
{
    if (!GetTabInfo().IsExportTab(mnScTab))
    {
        // header export.
        SetCurrScTab(mnScTab);
        if (mxCellTable)
            mxCellTable->Finalize(false);
        aRecList.SaveXml(rStrm);

        return;
    }

    // worksheet export
    OUString sSheetName = XclXmlUtils::GetStreamName( "xl/", "worksheets/sheet", mnScTab+1 );

    sax_fastparser::FSHelperPtr pWorksheet = rStrm.GetStreamForPath( sSheetName );

    rStrm.PushStream( pWorksheet );

    pWorksheet->startElement( XML_worksheet,
        XML_xmlns, rStrm.getNamespaceURL(OOX_NS(xls)),
        FSNS(XML_xmlns, XML_r), rStrm.getNamespaceURL(OOX_NS(officeRel)),
        FSNS(XML_xmlns, XML_xdr), "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing", // rStrm.getNamespaceURL(OOX_NS(xm)) -> "http://schemas.microsoft.com/office/excel/2006/main",
        FSNS(XML_xmlns, XML_x14), rStrm.getNamespaceURL(OOX_NS(xls14Lst)),
        FSNS(XML_xmlns, XML_xr2), rStrm.getNamespaceURL(OOX_NS(xr2)),
        FSNS(XML_xmlns, XML_mc), rStrm.getNamespaceURL(OOX_NS(mce)));

    SetCurrScTab( mnScTab );
    if (mxCellTable)
        mxCellTable->Finalize(false);
    aRecList.SaveXml( rStrm );

    XclExpXmlPivotTables* pPT = GetXmlPivotTableManager().GetTablesBySheet(mnScTab);
    if (pPT)
        pPT->SaveXml(rStrm);

    rStrm.GetCurrentStream()->endElement( XML_worksheet );
    rStrm.PopStream();
}

ExcDocument::ExcDocument( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    aHeader( rRoot )
{
}

ExcDocument::~ExcDocument()
{
    maTableList.RemoveAllRecords();    // for the following assertion!
}

void ExcDocument::ReadDoc()
{
    InitializeConvert();

    if (GetOutput() == EXC_OUTPUT_BINARY)
        aHeader.FillAsHeaderBinary(maBoundsheetList);
    else
    {
        aHeader.FillAsHeaderXml(maBoundsheetList);
        GetXmlPivotTableManager().Initialize();
        GetTablesManager().Initialize();    // Move outside conditions if we wanted to support BIFF.
    }

    SCTAB nScTab = 0, nScTabCount = GetTabInfo().GetScTabCount();
    SCTAB nCodeNameIdx = 0, nCodeNameCount = GetExtDocOptions().GetCodeNameCount();

    for( ; nScTab < nScTabCount; ++nScTab )
    {
        if( GetTabInfo().IsExportTab( nScTab ) )
        {
            ExcTableList::RecordRefType xTab = new ExcTable( GetRoot(), nScTab );
            maTableList.AppendRecord( xTab );
            if (GetOutput() == EXC_OUTPUT_BINARY)
                xTab->FillAsTableBinary(nCodeNameIdx);
            else
                xTab->FillAsTableXml();

            ++nCodeNameIdx;
        }
    }
    for( ; nCodeNameIdx < nCodeNameCount; ++nScTab, ++nCodeNameIdx )
    {
        ExcTableList::RecordRefType xTab = new ExcTable( GetRoot(), nScTab );
        maTableList.AppendRecord( xTab );
        xTab->FillAsEmptyTable( nCodeNameIdx );
    }

    if ( GetBiff() == EXC_BIFF8 )
    {
        // complete temporary Escher stream
        GetObjectManager().EndDocument();

        // change tracking
        if ( GetDoc().GetChangeTrack() )
            m_xExpChangeTrack.reset(new XclExpChangeTrack( GetRoot() ));
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
            if( xBoundsheet )
                xBoundsheet->SetStreamPos( aXclStrm.GetSvStreamPos() );
            // write the table
            maTableList.GetRecord( nTab )->Write( aXclStrm );
        }

        // write the table stream positions into the BOUNDSHEET records
        for( size_t nBSheet = 0, nBSheetCount = maBoundsheetList.GetSize(); nBSheet < nBSheetCount; ++nBSheet )
            maBoundsheetList.GetRecord( nBSheet )->UpdateStreamPos( aXclStrm );
    }
    if( m_xExpChangeTrack )
        m_xExpChangeTrack->Write();
}

void ExcDocument::WriteXml( XclExpXmlStream& rStrm )
{
    ScDocShell* pDocShell = GetDocShell();

    using namespace ::com::sun::star;
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS( static_cast<cppu::OWeakObject*>(pDocShell->GetModel()), uno::UNO_QUERY_THROW );
    uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();

    OUString sUserName = GetUserName();
    sal_uInt32 nWriteProtHash = pDocShell->GetModifyPasswordHash();
    bool bHasPasswordHash = nWriteProtHash && !sUserName.isEmpty();
    const uno::Sequence<beans::PropertyValue> aInfo = pDocShell->GetModifyPasswordInfo();
    OUString sAlgorithm, sSalt, sHash;
    sal_Int32 nCount = 0;
    for (const auto& prop : aInfo)
    {
        if (prop.Name == "algorithm-name")
            prop.Value >>= sAlgorithm;
        else if (prop.Name == "salt")
            prop.Value >>= sSalt;
        else if (prop.Name == "iteration-count")
            prop.Value >>= nCount;
        else if (prop.Name == "hash")
            prop.Value >>= sHash;
    }
    bool bHasPasswordInfo
        = sAlgorithm != "PBKDF2" && !sSalt.isEmpty() && !sHash.isEmpty() && !sUserName.isEmpty();
    rStrm.exportDocumentProperties(xDocProps, pDocShell->IsSecurityOptOpenReadOnly()
                                                  && !bHasPasswordHash && !bHasPasswordInfo);
    rStrm.exportCustomFragments();

    sax_fastparser::FSHelperPtr& rWorkbook = rStrm.GetCurrentStream();
    rWorkbook->startElement( XML_workbook,
            XML_xmlns, rStrm.getNamespaceURL(OOX_NS(xls)),
            FSNS(XML_xmlns, XML_r), rStrm.getNamespaceURL(OOX_NS(officeRel)) );
    rWorkbook->singleElement( XML_fileVersion,
            XML_appName, "Calc"
            // OOXTODO: XML_codeName
            // OOXTODO: XML_lastEdited
            // OOXTODO: XML_lowestEdited
            // OOXTODO: XML_rupBuild
    );

    if (bHasPasswordHash)
        rWorkbook->singleElement(XML_fileSharing,
                XML_userName, sUserName,
                XML_reservationPassword, OString::number(nWriteProtHash, 16));
    else if (bHasPasswordInfo)
        rWorkbook->singleElement(XML_fileSharing,
                XML_userName, sUserName,
                XML_algorithmName, sAlgorithm,
                XML_hashValue, sHash,
                XML_saltValue, sSalt,
                XML_spinCount, OString::number(nCount));

    if( !maTableList.IsEmpty() )
    {
        InitializeSave();

        auto* pDrawLayer = GetDoc().GetDrawLayer();
        if (pDrawLayer)
        {
            std::shared_ptr<model::Theme> pTheme = pDrawLayer->getTheme();
            if (pTheme)
            {
                OUString sThemeRelationshipPath = u"theme/theme1.xml"_ustr;
                OUString sThemeDocumentPath = "xl/" + sThemeRelationshipPath;

                oox::ThemeExport aThemeExport(&rStrm, oox::drawingml::DOCUMENT_XLSX);
                aThemeExport.write(sThemeDocumentPath, *pTheme);

                // xl/_rels/workbook.xml.rels
                // TODO: check if Theme import & export work correctly
                rStrm.addRelation(rStrm.GetCurrentStream()->getOutputStream(),
                                  oox::getRelationship(Relationship::THEME),
                                  sThemeRelationshipPath);
            }
        }

        aHeader.WriteXml( rStrm );

        for( size_t nTab = 0, nTabCount = maTableList.GetSize(); nTab < nTabCount; ++nTab )
        {
            // write the table
            maTableList.GetRecord( nTab )->WriteXml( rStrm );
        }
    }

    if( m_xExpChangeTrack )
        m_xExpChangeTrack->WriteXml( rStrm );

    XclExpXmlPivotCaches& rCaches = GetXmlPivotTableManager().GetCaches();
    if (rCaches.HasCaches())
        rCaches.SaveXml(rStrm);

    const ScCalcConfig& rCalcConfig = GetDoc().GetCalcConfig();
    formula::FormulaGrammar::AddressConvention eConv = rCalcConfig.meStringRefAddressSyntax;

    // don't save "unspecified" string ref syntax ... query formula grammar
    // and save that instead
    if( eConv == formula::FormulaGrammar::CONV_UNSPECIFIED)
    {
        eConv = GetDoc().GetAddressConvention();
    }

    ScDocument& rDoc = GetDoc();
    bool connectionXml = rDoc.hasConnectionXml();

    if (connectionXml)
    {
        // save xl/connections.xml reference into [Content_Types].xml
        sax_fastparser::FSHelperPtr aConnectionsXml = rStrm.CreateOutputStream(
            "xl/connections.xml", u"connections.xml", rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.connections+xml",
            oox::getRelationship(Relationship::CONNECTIONS));
        rStrm.PushStream(aConnectionsXml);

        /*
            <connections
            xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"
            xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"
            xmlns:xr16="http://schemas.microsoft.com/office/spreadsheetml/2017/revision16"
            mc:Ignorable="xr16">
        */

        // write into xl/connections.xml
        // export <connections>
        aConnectionsXml->startElement(
            XML_connections, XML_xmlns, rStrm.getNamespaceURL(OOX_NS(xls)), FSNS(XML_xmlns, XML_mc),
            rStrm.getNamespaceURL(OOX_NS(mce)), FSNS(XML_xmlns, XML_xr16),
            rStrm.getNamespaceURL(OOX_NS(xr16)), FSNS(XML_mc, XML_Ignorable), "xr16");

        // get a list of <connection> element and export it with its child elements
        ConnectionVector vConnVector = rDoc.getConnectionVector();
        for (const auto& rConnection : vConnVector)
        {
            const oox::xls::ConnectionModel& rModel = rConnection->getModel();

            if (rModel.mnId == -1 || rModel.mnRefreshedVersion == -1)
                continue; // incorrect values, skip

            rtl::Reference<sax_fastparser::FastAttributeList> pAttrList
                = sax_fastparser::FastSerializerHelper::createAttrList();

            pAttrList->add(XML_id, OUString::number(rModel.mnId));

            if (!rModel.maXr16Uid.isEmpty())
                pAttrList->add(FSNS(XML_xr16, XML_uid), rModel.maXr16Uid);
            if (!rModel.maSourceFile.isEmpty())
                pAttrList->add(XML_sourceFile, rModel.maSourceFile);
            if (!rModel.maSourceConnFile.isEmpty())
                pAttrList->add(XML_odcFile, rModel.maSourceConnFile);

            pAttrList->add(XML_keepAlive, ToPsz10(rModel.mbKeepAlive));
            pAttrList->add(XML_interval, OUString::number(rModel.mnInterval));

            if (!rModel.maName.isEmpty())
                pAttrList->add(XML_name, rModel.maName);
            if (!rModel.maDescription.isEmpty())
                pAttrList->add(XML_description, rModel.maDescription);
            if (OUString::number(rModel.mnType).length > 0)
                pAttrList->add(XML_type, OUString::number(rModel.mnType));

            pAttrList->add(XML_reconnectionMethod, OUString::number(rModel.mnReconnectMethod));
            pAttrList->add(XML_refreshedVersion, OUString::number(rModel.mnRefreshedVersion));
            pAttrList->add(XML_minRefreshableVersion,
                           OUString::number(rModel.mnMinRefreshableVersion));
            pAttrList->add(XML_savePassword, ToPsz10(rModel.mbSavePassword));
            pAttrList->add(XML_new, ToPsz10(rModel.mbNew));
            pAttrList->add(XML_deleted, ToPsz10(rModel.mbDeleted));
            pAttrList->add(XML_onlyUseConnectionFile, ToPsz10(rModel.mbOnlyUseConnFile));
            pAttrList->add(XML_background, ToPsz10(rModel.mbBackground));
            pAttrList->add(XML_refreshOnLoad, ToPsz10(rModel.mbRefreshOnLoad));
            pAttrList->add(XML_saveData, ToPsz10(rModel.mbSaveData));

            // import credentials="" attribute of <connection> element
            if (rModel.mnCredentials != -1)
            {
                sal_Int32 nToken = rModel.mnCredentials;
                OUString nValue;

                switch (nToken)
                {
                    case XML_none:
                        nValue = "none";
                        break;
                    case XML_stored:
                        nValue = "stored";
                        break;
                    case XML_prompt:
                        nValue = "prompt";
                        break;
                    default:
                        nValue = "integrated";
                        break;
                }

                pAttrList->add(XML_credentials, nValue);
            }

            if (!rModel.maSsoId.isEmpty())
                pAttrList->add(XML_singleSignOnId, rModel.maSsoId);

            // export <connection> with attributes
            rStrm.GetCurrentStream()->startElement(XML_connection, pAttrList);

            /*
                start export child elements of <connection>

                <xsd:sequence>
                    <xsd:element name="dbPr" minOccurs="0" maxOccurs="1" type="CT_DbPr"/>
                    <xsd:element name="olapPr" minOccurs="0" maxOccurs="1" type="CT_OlapPr"/>
                    <xsd:element name="webPr" minOccurs="0" maxOccurs="1" type="CT_WebPr"/>
                    <xsd:element name="textPr" minOccurs="0" maxOccurs="1" type="CT_TextPr"/>
                    <xsd:element name="parameters" minOccurs="0" maxOccurs="1" type="CT_Parameters"/>
                    <xsd:element name="extLst" minOccurs="0" maxOccurs="1" type="CT_ExtensionList"/>
                </xsd:sequence>
            */

            { // export <dbPr>

                rtl::Reference<sax_fastparser::FastAttributeList> pAttrListDbPr
                    = sax_fastparser::FastSerializerHelper::createAttrList();

                css::uno::Sequence<css::uno::Any> aSeqs = rConnection->getDbPrSequenceAny();
                addElemensToAttrList(pAttrListDbPr, aSeqs);

                rStrm.GetCurrentStream()->singleElement(XML_dbPr, pAttrListDbPr);
            }

            { // export <olapPr>

                rtl::Reference<sax_fastparser::FastAttributeList> pAttrListOlapPr
                    = sax_fastparser::FastSerializerHelper::createAttrList();

                css::uno::Sequence<css::uno::Any> aSeqs = rConnection->getOlapPrSequenceAny();
                addElemensToAttrList(pAttrListOlapPr, aSeqs);

                // this prints empty <olapPr/> even if aSeqs is empty, TODO: check if aSeqs is empty
                rStrm.GetCurrentStream()->singleElement(XML_olapPr, pAttrListOlapPr);
            }

            { // export <webPr> and its child elements
                rtl::Reference<sax_fastparser::FastAttributeList> pAttrListWebPr
                    = sax_fastparser::FastSerializerHelper::createAttrList();

                if (rModel.mxWebPr)
                {
                    pAttrListWebPr->add(XML_xml, ToPsz10(rModel.mxWebPr->mbXml));
                    pAttrListWebPr->add(XML_sourceData, ToPsz10(rModel.mxWebPr->mbSourceData));
                    pAttrListWebPr->add(XML_parsePre, ToPsz10(rModel.mxWebPr->mbParsePre));
                    pAttrListWebPr->add(XML_consecutive, ToPsz10(rModel.mxWebPr->mbConsecutive));
                    pAttrListWebPr->add(XML_firstRow, ToPsz10(rModel.mxWebPr->mbFirstRow));
                    pAttrListWebPr->add(XML_xl97, ToPsz10(rModel.mxWebPr->mbXl97Created));
                    pAttrListWebPr->add(XML_textDates, ToPsz10(rModel.mxWebPr->mbTextDates));
                    pAttrListWebPr->add(XML_xl2000, ToPsz10(rModel.mxWebPr->mbXl2000Refreshed));
                    pAttrListWebPr->add(XML_htmlTables, ToPsz10(rModel.mxWebPr->mbHtmlTables));

                    if (!rModel.mxWebPr->maUrl.isEmpty())
                        pAttrListWebPr->add(XML_url, rModel.mxWebPr->maUrl);
                    if (!rModel.mxWebPr->maPostMethod.isEmpty())
                        pAttrListWebPr->add(XML_post, rModel.mxWebPr->maPostMethod);
                    if (!rModel.mxWebPr->maEditPage.isEmpty())
                        pAttrListWebPr->add(XML_editPage, rModel.mxWebPr->maEditPage);

                    // import htmlFormat="" attribute of <webPr> element
                    if (rModel.mxWebPr->mnHtmlFormat != -1)
                    {
                        sal_Int32 nToken = rModel.mxWebPr->mnHtmlFormat;
                        OUString nValue;

                        switch (nToken)
                        {
                            case XML_all:
                                nValue = "all";
                                break;
                            case XML_rtf:
                                nValue = "rtf";
                                break;
                            default:
                                nValue = "none";
                                break;
                        }

                        pAttrListWebPr->add(XML_htmlFormat, nValue);
                    }

                    // export <webPr> with attributes
                    rStrm.GetCurrentStream()->startElement(XML_webPr, pAttrListWebPr);

                    { // export <tables> and its child elements

                        rtl::Reference<sax_fastparser::FastAttributeList> pAttrListTables
                            = sax_fastparser::FastSerializerHelper::createAttrList();

                        // <tables count="<xsd:unsignedInt>">
                        if (rModel.mxWebPr->mnCount >= 0)
                        {
                            pAttrListTables->add(XML_count,
                                                 OUString::number(rModel.mxWebPr->mnCount));

                            // export <tables> with attributes
                            rStrm.GetCurrentStream()->startElement(XML_tables, pAttrListTables);

                            { // export <m>, <s> and <x> elements
                                for (auto& tableElement : rModel.mxWebPr->maTables)
                                {
                                    OUString sElement;
                                    tableElement >>= sElement;
                                    OUString token = sElement.getToken(0, ',');
                                    OUString attributeValue = sElement.getToken(1, ',');

                                    if (token == "s")
                                        rStrm.GetCurrentStream()->singleElement(XML_s, XML_v,
                                                                                attributeValue);
                                    else if (token == "x")
                                        rStrm.GetCurrentStream()->singleElement(XML_x, XML_v,
                                                                                attributeValue);
                                    else
                                        rStrm.GetCurrentStream()->singleElement(XML_m);
                                }
                            }

                            // put </tables>
                            rStrm.GetCurrentStream()->endElement(XML_tables);
                        }
                    }

                    // put </webPr>
                    rStrm.GetCurrentStream()->endElement(XML_webPr);
                }
            }

            { // export <textPr>

                rtl::Reference<sax_fastparser::FastAttributeList> pAttrListTextPr
                    = sax_fastparser::FastSerializerHelper::createAttrList();

                if (rModel.mxTextPr)
                {
                    addElemensToAttrList(pAttrListTextPr, rModel.mxTextPr->maTextPrSequenceAny);

                    rStrm.GetCurrentStream()->startElement(XML_textPr, pAttrListTextPr);

                    { // export <textFields>
                        rtl::Reference<sax_fastparser::FastAttributeList> pAttrListTextFields
                            = sax_fastparser::FastSerializerHelper::createAttrList();

                        addElemensToAttrList(pAttrListTextFields,
                                             rModel.mxTextPr->maTextFieldsSequenceAny);

                        rStrm.GetCurrentStream()->startElement(XML_textFields, pAttrListTextFields);

                        { // export <textField>

                            for (auto& textFieldAttr : rModel.mxTextPr->vTextField)
                            {
                                rtl::Reference<sax_fastparser::FastAttributeList> pAttrListTextField
                                    = sax_fastparser::FastSerializerHelper::createAttrList();

                                addElemensToAttrList(pAttrListTextField, textFieldAttr);

                                rStrm.GetCurrentStream()->singleElement(XML_textField,
                                                                        pAttrListTextField);
                            }
                        }

                        // put </textFields>
                        rStrm.GetCurrentStream()->endElement(XML_textFields);
                    }

                    // put </textPr>
                    rStrm.GetCurrentStream()->endElement(XML_textPr);
                }
            }

            { // export <parameters>

                rtl::Reference<sax_fastparser::FastAttributeList> pAttrListParameters
                    = sax_fastparser::FastSerializerHelper::createAttrList();

                if (rModel.mxParameters && rModel.mxParameters->mnCount > -1)
                {
                    pAttrListParameters->add(XML_count,
                                             OUString::number(rModel.mxParameters->mnCount));

                    rStrm.GetCurrentStream()->startElement(XML_parameters, pAttrListParameters);

                    // export <parameter>
                    for (auto& parameterAttr : rModel.mxParameters->vParameter)
                    {
                        rtl::Reference<sax_fastparser::FastAttributeList> pAttrListParameter
                            = sax_fastparser::FastSerializerHelper::createAttrList();

                        addElemensToAttrList(pAttrListParameter, parameterAttr);

                        rStrm.GetCurrentStream()->singleElement(XML_parameter, pAttrListParameter);
                    }

                    // put </parameters>
                    rStrm.GetCurrentStream()->endElement(XML_parameters);
                }
            }

            { // export <extLst>
                if (rModel.mxExtensionList)
                {
                    // put <extLst>, it has no attributes
                    rStrm.GetCurrentStream()->startElement(XML_extLst);

                    // export uri attribute of <ext> element
                    for (auto& uriValue : rModel.mxExtensionList->vExtension)
                    {
                        // export <ext> with uri attribute.
                        rStrm.GetCurrentStream()->startElement(XML_ext, XML_uri, uriValue);

                    /*
                        TODO: export child elements of <ext>. We should export "any element in any namespace", which seems challenging.

                        <extLst>
                            <ext>
                                (Any element in any namespace)
                            </ext>
                        </extLst>
                    */

                        // put </ext>
                        rStrm.GetCurrentStream()->endElement(XML_ext);
                    }

                    // put </extLst>
                    rStrm.GetCurrentStream()->endElement(XML_extLst);
                }
            }

            // put </connection>
            rStrm.GetCurrentStream()->endElement(XML_connection);
        }

        // put </connections>
        aConnectionsXml->endElement(XML_connections);
        rStrm.PopStream();
    }

    if (rDoc.hasCustomXml())
    {
        // export customXml/item1.xml into xl/_rels/workbook.xml.rels
        OUString sCustomXmlPath = OUString::Concat("../") + rDoc.getCustomXmlItems();

        // FIXME: what if there are customXml/item2.xml, customXml/item3.xml etc?
        // we need to save all customXml/item*.xml paths into ScDocument from XmlFilterBase::importCustomFragments
        // then we should get them with rDoc here.
        rStrm.addRelation(rStrm.GetCurrentStream()->getOutputStream(),
                          oox::getRelationship(Relationship::CUSTOMXML), sCustomXmlPath);
    }

    // write if it has been read|imported or explicitly changed
    // or if ref syntax isn't what would be native for our file format
    // i.e. ExcelA1 in this case
    if ( rCalcConfig.mbHasStringRefSyntax ||
         (eConv != formula::FormulaGrammar::CONV_XL_A1) )
    {
        XclExtLstRef xExtLst = new XclExtLst( GetRoot()  );
        xExtLst->AddRecord( new XclExpExtCalcPr( GetRoot(), eConv )  );
        xExtLst->SaveXml(rStrm);
    }

    rWorkbook->endElement( XML_workbook );
    rWorkbook.reset();
}

void ExcDocument::addElemensToAttrList(const rtl::Reference<sax_fastparser::FastAttributeList>& pAttrList,
                                       css::uno::Sequence<css::uno::Any>& aSeqs)
{
    css::uno::Sequence<css::xml::FastAttribute> aFastSeq;
    css::uno::Sequence<css::xml::Attribute> aUnkSeq;

    // TODO: check if aSeqs is empty or not
    for (const auto& a : aSeqs)
    {
        if (a >>= aFastSeq)
        {
            for (const auto& rAttr : aFastSeq)
                pAttrList->add(rAttr.Token, rAttr.Value);
        }
        else if (a >>= aUnkSeq)
        {
            for (const auto& rAttr : aUnkSeq)
                pAttrList->addUnknown(rAttr.NamespaceURL,
                                      OUStringToOString(rAttr.Name, RTL_TEXTENCODING_UTF8),
                                      OUStringToOString(rAttr.Value, RTL_TEXTENCODING_UTF8));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
