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

#include "xiroot.hxx"
#include "addincol.hxx"
#include "document.hxx"
#include "scextopt.hxx"
#include "xltracer.hxx"
#include "xihelper.hxx"
#include "xiformula.hxx"
#include "xilink.hxx"
#include "xiname.hxx"
#include "xistyle.hxx"
#include "xicontent.hxx"
#include "xiescher.hxx"
#include "xipivot.hxx"
#include "xipage.hxx"
#include "xiview.hxx"

#include "root.hxx"
#include "excimp8.hxx"
#include "documentimport.hxx"

// Global data ================================================================

XclImpRootData::XclImpRootData( XclBiff eBiff, SfxMedium& rMedium,
        SotStorageRef xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc ) :
    XclRootData( eBiff, rMedium, xRootStrg, rDoc, eTextEnc, false ),
    mxDocImport(new ScDocumentImport(rDoc)),
    mbHasCodePage( false ),
    mbHasBasic( false )
{
}

XclImpRootData::~XclImpRootData()
{
}

// ----------------------------------------------------------------------------

XclImpRoot::XclImpRoot( XclImpRootData& rImpRootData ) :
    XclRoot( rImpRootData ),
    mrImpData( rImpRootData )
{
    mrImpData.mxAddrConv.reset( new XclImpAddressConverter( GetRoot() ) );
    mrImpData.mxFmlaComp.reset( new XclImpFormulaCompiler( GetRoot() ) );
    mrImpData.mxPalette.reset( new XclImpPalette( GetRoot() ) );
    mrImpData.mxFontBfr.reset( new XclImpFontBuffer( GetRoot() ) );
    mrImpData.mxNumFmtBfr.reset( new XclImpNumFmtBuffer( GetRoot() ) );
    mrImpData.mpXFBfr.reset( new XclImpXFBuffer( GetRoot() ) );
    mrImpData.mxXFRangeBfr.reset( new XclImpXFRangeBuffer( GetRoot() ) );
    mrImpData.mxTabInfo.reset( new XclImpTabInfo );
    mrImpData.mxNameMgr.reset( new XclImpNameManager( GetRoot() ) );
    mrImpData.mxObjMgr.reset( new XclImpObjectManager( GetRoot() ) );

    if( GetBiff() == EXC_BIFF8 )
    {
        mrImpData.mxLinkMgr.reset( new XclImpLinkManager( GetRoot() ) );
        mrImpData.mxSst.reset( new XclImpSst( GetRoot() ) );
        mrImpData.mxCondFmtMgr.reset( new XclImpCondFormatManager( GetRoot() ) );
        mrImpData.mxValidMgr.reset( new XclImpValidationManager( GetRoot() ) );
        // TODO still in old RootData (deleted by RootData)
        GetOldRoot().pAutoFilterBuffer = new XclImpAutoFilterBuffer;
        mrImpData.mxWebQueryBfr.reset( new XclImpWebQueryBuffer( GetRoot() ) );
        mrImpData.mxPTableMgr.reset( new XclImpPivotTableManager( GetRoot() ) );
        mrImpData.mxTabProtect.reset( new XclImpSheetProtectBuffer( GetRoot() ) );
        mrImpData.mxDocProtect.reset( new XclImpDocProtectBuffer( GetRoot() ) );
    }

    mrImpData.mxPageSett.reset( new XclImpPageSettings( GetRoot() ) );
    mrImpData.mxDocViewSett.reset( new XclImpDocViewSettings( GetRoot() ) );
    mrImpData.mxTabViewSett.reset( new XclImpTabViewSettings( GetRoot() ) );
}

void XclImpRoot::SetCodePage( sal_uInt16 nCodePage )
{
    SetTextEncoding( XclTools::GetTextEncoding( nCodePage ) );
    mrImpData.mbHasCodePage = true;
}

void XclImpRoot::SetAppFontEncoding( rtl_TextEncoding eAppFontEnc )
{
    if( !mrImpData.mbHasCodePage )
        SetTextEncoding( eAppFontEnc );
}

void XclImpRoot::InitializeTable( SCTAB nScTab )
{
    if( GetBiff() <= EXC_BIFF4 )
    {
        GetPalette().Initialize();
        GetFontBuffer().Initialize();
        GetNumFmtBuffer().Initialize();
        GetXFBuffer().Initialize();
    }
    GetXFRangeBuffer().Initialize();
    GetPageSettings().Initialize();
    GetTabViewSettings().Initialize();
    // delete the automatically generated codename
    GetDoc().SetCodeName( nScTab, OUString() );
}

void XclImpRoot::FinalizeTable()
{
    GetXFRangeBuffer().Finalize();
    GetOldRoot().pColRowBuff->Convert( GetCurrScTab() );
    GetPageSettings().Finalize();
    GetTabViewSettings().Finalize();
}

XclImpAddressConverter& XclImpRoot::GetAddressConverter() const
{
    return *mrImpData.mxAddrConv;
}

XclImpFormulaCompiler& XclImpRoot::GetFormulaCompiler() const
{
    return *mrImpData.mxFmlaComp;
}

ExcelToSc& XclImpRoot::GetOldFmlaConverter() const
{
    // TODO still in old RootData
    return *GetOldRoot().pFmlaConverter;
}

XclImpSst& XclImpRoot::GetSst() const
{
    OSL_ENSURE( mrImpData.mxSst, "XclImpRoot::GetSst - invalid call, wrong BIFF" );
    return *mrImpData.mxSst;
}

XclImpPalette& XclImpRoot::GetPalette() const
{
    return *mrImpData.mxPalette;
}

XclImpFontBuffer& XclImpRoot::GetFontBuffer() const
{
    return *mrImpData.mxFontBfr;
}

XclImpNumFmtBuffer& XclImpRoot::GetNumFmtBuffer() const
{
    return *mrImpData.mxNumFmtBfr;
}

XclImpXFBuffer& XclImpRoot::GetXFBuffer() const
{
    return *mrImpData.mpXFBfr;
}

XclImpXFRangeBuffer& XclImpRoot::GetXFRangeBuffer() const
{
    return *mrImpData.mxXFRangeBfr;
}

_ScRangeListTabs& XclImpRoot::GetPrintAreaBuffer() const
{
    // TODO still in old RootData
    return *GetOldRoot().pPrintRanges;
}

_ScRangeListTabs& XclImpRoot::GetTitleAreaBuffer() const
{
    // TODO still in old RootData
    return *GetOldRoot().pPrintTitles;
}

XclImpTabInfo& XclImpRoot::GetTabInfo() const
{
    return *mrImpData.mxTabInfo;
}

XclImpNameManager& XclImpRoot::GetNameManager() const
{
    return *mrImpData.mxNameMgr;
}

XclImpLinkManager& XclImpRoot::GetLinkManager() const
{
    OSL_ENSURE( mrImpData.mxLinkMgr, "XclImpRoot::GetLinkManager - invalid call, wrong BIFF" );
    return *mrImpData.mxLinkMgr;
}

XclImpObjectManager& XclImpRoot::GetObjectManager() const
{
    return *mrImpData.mxObjMgr;
}

XclImpSheetDrawing& XclImpRoot::GetCurrSheetDrawing() const
{
    OSL_ENSURE( !IsInGlobals(), "XclImpRoot::GetCurrSheetDrawing - must not be called from workbook globals" );
    return mrImpData.mxObjMgr->GetSheetDrawing( GetCurrScTab() );
}

XclImpCondFormatManager& XclImpRoot::GetCondFormatManager() const
{
    OSL_ENSURE( mrImpData.mxCondFmtMgr, "XclImpRoot::GetCondFormatManager - invalid call, wrong BIFF" );
    return *mrImpData.mxCondFmtMgr;
}

XclImpValidationManager& XclImpRoot::GetValidationManager() const
{
    OSL_ENSURE( mrImpData.mxValidMgr, "XclImpRoot::GetValidationManager - invalid call, wrong BIFF" );
    return *mrImpData.mxValidMgr;
}

XclImpAutoFilterBuffer& XclImpRoot::GetFilterManager() const
{
    // TODO still in old RootData
    OSL_ENSURE( GetOldRoot().pAutoFilterBuffer, "XclImpRoot::GetFilterManager - invalid call, wrong BIFF" );
    return *GetOldRoot().pAutoFilterBuffer;
}

XclImpWebQueryBuffer& XclImpRoot::GetWebQueryBuffer() const
{
    OSL_ENSURE( mrImpData.mxWebQueryBfr, "XclImpRoot::GetWebQueryBuffer - invalid call, wrong BIFF" );
    return *mrImpData.mxWebQueryBfr;
}

XclImpPivotTableManager& XclImpRoot::GetPivotTableManager() const
{
    OSL_ENSURE( mrImpData.mxPTableMgr, "XclImpRoot::GetPivotTableManager - invalid call, wrong BIFF" );
    return *mrImpData.mxPTableMgr;
}

XclImpSheetProtectBuffer& XclImpRoot::GetSheetProtectBuffer() const
{
    OSL_ENSURE( mrImpData.mxTabProtect, "XclImpRoot::GetSheetProtectBuffer - invalid call, wrong BIFF" );
    return *mrImpData.mxTabProtect;
}

XclImpDocProtectBuffer& XclImpRoot::GetDocProtectBuffer() const
{
    OSL_ENSURE( mrImpData.mxDocProtect, "XclImpRoot::GetDocProtectBuffer - invalid call, wrong BIFF" );
    return *mrImpData.mxDocProtect;
}

XclImpPageSettings& XclImpRoot::GetPageSettings() const
{
    return *mrImpData.mxPageSett;
}

XclImpDocViewSettings& XclImpRoot::GetDocViewSettings() const
{
    return *mrImpData.mxDocViewSett;
}

XclImpTabViewSettings& XclImpRoot::GetTabViewSettings() const
{
    return *mrImpData.mxTabViewSett;
}

String XclImpRoot::GetScAddInName( const String& rXclName ) const
{
    OUString aScName;
    if( ScGlobal::GetAddInCollection()->GetCalcName( rXclName, aScName ) )
        return aScName;
    return rXclName;
}

void XclImpRoot::ReadCodeName( XclImpStream& rStrm, bool bGlobals )
{
    if( mrImpData.mbHasBasic && (GetBiff() == EXC_BIFF8) )
    {
        String aName = rStrm.ReadUniString();
        if( aName.Len() > 0 )
        {
            if( bGlobals )
            {
                GetExtDocOptions().GetDocSettings().maGlobCodeName = aName;
                GetDoc().SetCodeName( aName );
            }
            else
            {
                GetExtDocOptions().SetCodeName( GetCurrScTab(), aName );
                GetDoc().SetCodeName( GetCurrScTab(), aName );
            }
        }
    }
}

ScDocumentImport& XclImpRoot::GetDocImport()
{
    return *mrImpData.mxDocImport;
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
