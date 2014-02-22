/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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

#include "formulacell.hxx"
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

using namespace oox;

static OUString lcl_GetVbaTabName( SCTAB n )
{
    OUString aRet = "__VBA__"  + OUString::number( static_cast<sal_uInt16>(n) );
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
    
    
    
    aRecList.AppendNewRecord( new XclCalccount( rDoc ) );
    aRecList.AppendNewRecord( new XclRefmode( rDoc ) );
    aRecList.AppendNewRecord( new XclIteration( rDoc ) );
    aRecList.AppendNewRecord( new XclDelta( rDoc ) );
    aRecList.AppendNewRecord( new XclExpBoolRecord(0x005F, true) ); 
    aRecList.AppendNewRecord( new XclExpXmlEndSingleElementRecord() );  
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

    aRecList.AppendNewRecord( new XclExpXmlEndSingleElementRecord() );   
}

static void lcl_AddScenariosAndFilters( XclExpRecordList<>& aRecList, const XclExpRoot& rRoot, SCTAB nScTab )
{
    
    aRecList.AppendNewRecord( new ExcEScenarioManager( rRoot, nScTab ) );
    
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
    OSL_ENSURE( pRec, "-ExcTable::Add(): pRec is NULL!" );
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
    OUString aTmpString;
    SCTAB  nScTabCount     = rTabInfo.GetScTabCount();
    sal_uInt16  nExcTabCount    = rTabInfo.GetXclTabCount();
    sal_uInt16  nCodenames      = static_cast< sal_uInt16 >( GetExtDocOptions().GetCodeNameCount() );

    SfxObjectShell* pShell = GetDocShell();
    sal_uInt16 nWriteProtHash = pShell ? pShell->GetModifyPasswordHash() : 0;
    bool bRecommendReadOnly = pShell && pShell->IsLoadReadonly();

    if( (nWriteProtHash > 0) || bRecommendReadOnly )
        Add( new XclExpEmptyRecord( EXC_ID_WRITEPROT ) );

    
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

    

    for( nC = 0 ; nC < nScTabCount ; nC++ )
        if( rTabInfo.IsExportTab( nC ) )
        {
            rDoc.GetName( nC, aTmpString );
            *pTabNames << aTmpString;
        }

    if ( GetBiff() <= EXC_BIFF5 )
    {
        
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_NAME ) );
    }

    
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
        Add( new XclExpBoolRecord(0x0040, false, XML_backupFile ) );    
        Add( new XclExpBoolRecord(0x008D, false, XML_showObjects ) );   
    }

    if ( GetBiff() == EXC_BIFF8 )
    {
        Add( new XclExpBoolRecord(0x0040, false) ); 
        Add( new XclExpBoolRecord(0x008D, false) ); 
    }

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
        Add( new XclExpBoolRecord(0x01B7, false) ); 
        Add( new XclExpBoolRecord(0x00DA, false) ); 
        
        
        //
        
        
        
        
        
        
        
        
        
        
        
        
        
        
    }
    Add( new XclExpXmlEndSingleElementRecord() );   

    
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
        
        GetPivotTableManager().CreatePivotTables();
        aRecList.AppendRecord( GetPivotTableManager().CreatePivotCachesRecord() );

        
        if( rDoc.GetChangeTrack() )
        {
            rR.pUserBViewList = new XclExpUserBViewList( *rDoc.GetChangeTrack() );
            Add( rR.pUserBViewList );
        }

        
        aRecList.AppendNewRecord( new XclExpBoolRecord( EXC_ID_USESELFS, GetDoc().GetDocOptions().IsLookUpColRowNames() ) );

        if( GetOutput() != EXC_OUTPUT_BINARY )
        {
            lcl_AddWorkbookProtection( aRecList, *this );
            lcl_AddBookviews( aRecList, *this );
        }

        
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

        
        Add( new XclExpCountry( GetRoot() ) );
        
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );
        aRecList.AppendRecord( CreateRecord( EXC_ID_NAME ) );

        if( GetOutput() != EXC_OUTPUT_BINARY )
            lcl_AddCalcPr( aRecList, *this );

        Add( new XclExpRecalcId );

        
        aRecList.AppendRecord( GetObjectManager().CreateDrawingGroup() );
        
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

    
    if( eBiff == EXC_BIFF8 )
        GetObjectManager().StartSheet();

    
    mxCellTable.reset( new XclExpCellTable( GetRoot() ) );

    
    std::vector<sc::NoteEntry> aNotes;
    rDoc.GetAllNoteEntries(aNotes);
    std::vector<sc::NoteEntry>::const_iterator it = aNotes.begin(), itEnd = aNotes.end();
    for (; it != itEnd; ++it)
    {
        if (it->maPos.Tab() != mnScTab)
            continue;

        mxNoteList->AppendNewRecord(new XclExpNote(GetRoot(), it->maPos, it->mpNote, OUString()));
    }

    if( GetOutput() != EXC_OUTPUT_BINARY )
    {
        FillAsXmlTable( nCodeNameIdx );
        return;
    }


    
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

    
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_GUTS ) );
    
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID2_DEFROWHEIGHT ) );

    
    if( eBiff <= EXC_BIFF5 )
        Add( new XclExpCountry( GetRoot() ) );

    Add( new XclExpWsbool( bFitToPages ) );

    
    aRecList.AppendRecord( xPageSett );

    const ScTableProtection* pTabProtect = rDoc.GetTabProtection(mnScTab);
    if (pTabProtect && pTabProtect->isProtected())
    {
        Add( new XclExpProtection(true) );
        Add( new XclExpBoolRecord(0x00DD, pTabProtect->isOptionEnabled(ScTableProtection::SCENARIOS)) );
        Add( new XclExpBoolRecord(0x0063, pTabProtect->isOptionEnabled(ScTableProtection::OBJECTS)) );
        Add( new XclExpPassHash(pTabProtect->getPasswordHash(PASSHASH_XL)) );
    }

    
    if( eBiff <= EXC_BIFF5 )
        aRecList.AppendRecord( CreateRecord( EXC_ID_EXTERNSHEET ) );

    if ( eBiff == EXC_BIFF8 )
        lcl_AddScenariosAndFilters( aRecList, GetRoot(), mnScTab );

    
    aRecList.AppendRecord( mxCellTable );

    
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_MERGEDCELLS ) );
    
    if( eBiff == EXC_BIFF8 )
        Add( new XclExpLabelranges( GetRoot() ) );
    
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_DVAL ) );

    if( eBiff == EXC_BIFF8 )
    {
        
        aRecList.AppendRecord( GetObjectManager().ProcessDrawing( GetSdrPage( mnScTab ) ) );
        
        aRecList.AppendRecord( GetPivotTableManager().CreatePivotTablesRecord( mnScTab ) );
    }

    
    aRecList.AppendRecord( mxNoteList );

    
    aRecList.AppendNewRecord( new XclExpTabViewSettings( GetRoot(), mnScTab ) );

    if( eBiff == EXC_BIFF8 )
    {
        
        Add( new XclExpSheetProtectOptions( GetRoot(), mnScTab ) );

        
        Add( new XclExpWebQueryBuffer( GetRoot() ) );

        
        Add( new XclExpCondFormatBuffer( GetRoot(), XclExtLstRef() ) );

        if( HasVbaStorage() )
            if( nCodeNameIdx < GetExtDocOptions().GetCodeNameCount() )
                Add( new XclCodename( GetExtDocOptions().GetCodeName( nCodeNameIdx ) ) );
    }

    
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_HLINK ) );

    
    if( rR.pUserBViewList )
    {
        XclExpUserBViewList::const_iterator iter;
        for ( iter = rR.pUserBViewList->begin(); iter != rR.pUserBViewList->end(); ++iter)
        {
            Add( new XclExpUsersViewBegin( (*iter)->GetGUID(), nExcTab ) );
            Add( new XclExpUsersViewEnd );
        }
    }

    
    Add( new ExcEof );
}

void ExcTable::FillAsXmlTable( SCTAB nCodeNameIdx )
{
    RootData& rR = GetOldRoot();

    
    boost::shared_ptr< XclExpPageSettings > xPageSett( new XclExpPageSettings( GetRoot() ) );
    XclExtLstRef xExtLst( new XclExtLst( GetRoot() ) );
    bool bFitToPages = xPageSett->GetPageData().mbFitToPages;

    Add( new ExcBof8 );

    Add( new XclExpWsbool( bFitToPages, mnScTab, &GetFilterManager() ) );

    
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_GUTS ) );
    
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID2_DEFROWHEIGHT ) );

    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID3_DIMENSIONS ) );

    
    aRecList.AppendNewRecord( new XclExpTabViewSettings( GetRoot(), mnScTab ) );

    
    aRecList.AppendRecord( mxCellTable );

    
    
    Add( new XclExpLabelranges( GetRoot() ) );

    







    
    
    aRecList.AppendRecord( GetPivotTableManager().CreatePivotTablesRecord( mnScTab ) );

    
    
    if( mxNoteList != 0 && !mxNoteList->IsEmpty() )
        aRecList.AppendNewRecord( new XclExpComments( mnScTab, *mxNoteList ) );

    
    
    Add( new XclExpWebQueryBuffer( GetRoot() ) );

    ScDocument& rDoc = GetDoc();
    const ScTableProtection* pTabProtect = rDoc.GetTabProtection(mnScTab);
    if (pTabProtect && pTabProtect->isProtected())
        Add( new XclExpSheetProtection(true, mnScTab) );

    lcl_AddScenariosAndFilters( aRecList, GetRoot(), mnScTab );

    
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_MERGEDCELLS ) );

    
    Add( new XclExpCondFormatBuffer( GetRoot(), xExtLst ) );

    if( HasVbaStorage() )
        if( nCodeNameIdx < GetExtDocOptions().GetCodeNameCount() )
            Add( new XclCodename( GetExtDocOptions().GetCodeName( nCodeNameIdx ) ) );

    
    aRecList.AppendRecord( mxCellTable->CreateRecord( EXC_ID_DVAL ) );

    
    XclExpRecordRef xHyperlinks = mxCellTable->CreateRecord( EXC_ID_HLINK );
    XclExpHyperlinkList* xHyperlinkList = dynamic_cast<XclExpHyperlinkList*>(xHyperlinks.get());
    if( xHyperlinkList != NULL && !xHyperlinkList->IsEmpty() )
    {
        aRecList.AppendNewRecord( new XclExpXmlStartElementRecord( XML_hyperlinks ) );
        aRecList.AppendRecord( xHyperlinks );
        aRecList.AppendNewRecord( new XclExpXmlEndElementRecord( XML_hyperlinks ) );
    }

    aRecList.AppendRecord( xPageSett );

    
    if( rR.pUserBViewList )
    {
        XclExpUserBViewList::const_iterator iter;
        for ( iter = rR.pUserBViewList->begin(); iter != rR.pUserBViewList->end(); ++iter )
        {
            Add( new XclExpUsersViewBegin( (*iter)->GetGUID(), nExcTab ) );
            Add( new XclExpUsersViewEnd );
        }
    }

    
    aRecList.AppendRecord( GetObjectManager().ProcessDrawing( GetSdrPage( mnScTab ) ) );

    aRecList.AppendRecord( xExtLst );

    
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
        
        OUString sSheetName = XclXmlUtils::GetStreamName( "xl/", "worksheets/sheet", mnScTab+1 );

        sax_fastparser::FSHelperPtr pWorksheet = rStrm.GetStreamForPath( sSheetName );

        rStrm.PushStream( pWorksheet );

        pWorksheet->startElement( XML_worksheet,
                XML_xmlns, "http:
                FSNS( XML_xmlns, XML_r ), "http:
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
    maTableList.RemoveAllRecords();    
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
        
        GetObjectManager().EndDocument();

        
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
            
            ExcBoundsheetRef xBoundsheet = maBoundsheetList.GetRecord( nTab );
            if( xBoundsheet.get() )
                xBoundsheet->SetStreamPos( aXclStrm.GetSvStreamPos() );
            
            maTableList.GetRecord( nTab )->Write( aXclStrm );
        }

        
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
            XML_xmlns, "http:
            FSNS(XML_xmlns, XML_r), "http:
            FSEND );
    rWorkbook->singleElement( XML_fileVersion,
            XML_appName, "Calc",
            
            
            
            
            FSEND );

    if( !maTableList.IsEmpty() )
    {
        InitializeSave();

        aHeader.WriteXml( rStrm );

        for( size_t nTab = 0, nTabCount = maTableList.GetSize(); nTab < nTabCount; ++nTab )
        {
            
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
