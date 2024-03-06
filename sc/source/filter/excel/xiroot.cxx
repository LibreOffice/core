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

#include <xiroot.hxx>
#include <addincol.hxx>
#include <colrowst.hxx>
#include <document.hxx>
#include <formel.hxx>
#include <scextopt.hxx>
#include <xihelper.hxx>
#include <xiformula.hxx>
#include <xilink.hxx>
#include <xiname.hxx>
#include <xistyle.hxx>
#include <xicontent.hxx>
#include <xiescher.hxx>
#include <xipivot.hxx>
#include <xipage.hxx>
#include <xiview.hxx>

#include <root.hxx>
#include <excimp8.hxx>
#include <documentimport.hxx>
#include <sot/storage.hxx>

// Global data ================================================================

XclImpRootData::XclImpRootData( XclBiff eBiff, SfxMedium& rMedium,
        const rtl::Reference<SotStorage>& xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc ) :
    XclRootData( eBiff, rMedium, xRootStrg, rDoc, eTextEnc, false ),
    mxDocImport(std::make_shared<ScDocumentImport>(rDoc)),
    mbHasCodePage( false ),
    mbHasBasic( false )
{
}

XclImpRootData::~XclImpRootData()
{
}

XclImpRoot::XclImpRoot( XclImpRootData& rImpRootData ) :
    XclRoot( rImpRootData ),
    mrImpData( rImpRootData )
{
    mrImpData.mxAddrConv = std::make_shared<XclImpAddressConverter>( GetRoot() );
    mrImpData.mxFmlaComp = std::make_shared<XclImpFormulaCompiler>( GetRoot() );
    mrImpData.mxPalette = std::make_shared<XclImpPalette>( GetRoot() );
    mrImpData.mxFontBfr = std::make_shared<XclImpFontBuffer>( GetRoot() );
    mrImpData.mxNumFmtBfr = std::make_shared<XclImpNumFmtBuffer>( GetRoot() );
    mrImpData.mpXFBfr = std::make_shared<XclImpXFBuffer>( GetRoot() );
    mrImpData.mxXFRangeBfr = std::make_shared<XclImpXFRangeBuffer>( GetRoot() );
    mrImpData.mxTabInfo = std::make_shared<XclImpTabInfo>();
    mrImpData.mxNameMgr = std::make_shared<XclImpNameManager>( GetRoot() );
    mrImpData.mxObjMgr = std::make_shared<XclImpObjectManager>( GetRoot() );

    if( GetBiff() == EXC_BIFF8 )
    {
        mrImpData.mxLinkMgr = std::make_shared<XclImpLinkManager>( GetRoot() );
        mrImpData.mxSst = std::make_shared<XclImpSst>( GetRoot() );
        mrImpData.mxCondFmtMgr = std::make_shared<XclImpCondFormatManager>( GetRoot() );
        mrImpData.mxValidMgr = std::make_shared<XclImpValidationManager>( GetRoot() );
        // TODO still in old RootData (deleted by RootData)
        GetOldRoot().pAutoFilterBuffer.reset( new XclImpAutoFilterBuffer );
        mrImpData.mxWebQueryBfr = std::make_shared<XclImpWebQueryBuffer>( GetRoot() );
        mrImpData.mxPTableMgr = std::make_shared<XclImpPivotTableManager>( GetRoot() );
        mrImpData.mxTabProtect = std::make_shared<XclImpSheetProtectBuffer>( GetRoot() );
        mrImpData.mxDocProtect = std::make_shared<XclImpDocProtectBuffer>( GetRoot() );
    }

    mrImpData.mxPageSett = std::make_shared<XclImpPageSettings>( GetRoot() );
    mrImpData.mxDocViewSett = std::make_shared<XclImpDocViewSettings>( GetRoot() );
    mrImpData.mxTabViewSett = std::make_shared<XclImpTabViewSettings>( GetRoot() );
    mrImpData.mpPrintRanges = std::make_unique<ScRangeListTabs>( GetRoot() );
    mrImpData.mpPrintTitles = std::make_unique<ScRangeListTabs>( GetRoot() );
}

void XclImpRoot::SetCodePage( sal_uInt16 nCodePage )
{
    SetTextEncoding( XclTools::GetTextEncoding( nCodePage ) );
    mrImpData.mbHasCodePage = true;
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
    assert(mrImpData.mxSst && "XclImpRoot::GetSst - invalid call, wrong BIFF");
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

ScRangeListTabs& XclImpRoot::GetPrintAreaBuffer() const
{
    return *mrImpData.mpPrintRanges;
}

ScRangeListTabs& XclImpRoot::GetTitleAreaBuffer() const
{
    return *mrImpData.mpPrintTitles;
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
    assert(mrImpData.mxLinkMgr && "XclImpRoot::GetLinkManager - invalid call, wrong BIFF");
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
    assert(mrImpData.mxCondFmtMgr && "XclImpRoot::GetCondFormatManager - invalid call, wrong BIFF");
    return *mrImpData.mxCondFmtMgr;
}

XclImpValidationManager& XclImpRoot::GetValidationManager() const
{
    assert(mrImpData.mxValidMgr && "XclImpRoot::GetValidationManager - invalid call, wrong BIFF");
    return *mrImpData.mxValidMgr;
}

XclImpAutoFilterBuffer& XclImpRoot::GetFilterManager() const
{
    // TODO still in old RootData
    assert(GetOldRoot().pAutoFilterBuffer && "XclImpRoot::GetFilterManager - invalid call, wrong BIFF");
    return *GetOldRoot().pAutoFilterBuffer;
}

XclImpWebQueryBuffer& XclImpRoot::GetWebQueryBuffer() const
{
    assert(mrImpData.mxWebQueryBfr && "XclImpRoot::GetWebQueryBuffer - invalid call, wrong BIFF");
    return *mrImpData.mxWebQueryBfr;
}

XclImpPivotTableManager& XclImpRoot::GetPivotTableManager() const
{
    assert(mrImpData.mxPTableMgr && "XclImpRoot::GetPivotTableManager - invalid call, wrong BIFF");
    return *mrImpData.mxPTableMgr;
}

XclImpSheetProtectBuffer& XclImpRoot::GetSheetProtectBuffer() const
{
    assert(mrImpData.mxTabProtect && "XclImpRoot::GetSheetProtectBuffer - invalid call, wrong BIFF");
    return *mrImpData.mxTabProtect;
}

XclImpDocProtectBuffer& XclImpRoot::GetDocProtectBuffer() const
{
    assert(mrImpData.mxDocProtect && "XclImpRoot::GetDocProtectBuffer - invalid call, wrong BIFF");
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

OUString XclImpRoot::GetScAddInName( const OUString& rXclName )
{
    OUString aScName;
    if( ScGlobal::GetAddInCollection()->GetCalcName( rXclName, aScName ) )
        return aScName;
    return rXclName;
}

void XclImpRoot::ReadCodeName( XclImpStream& rStrm, bool bGlobals )
{
    if( !(mrImpData.mbHasBasic && (GetBiff() == EXC_BIFF8)) )
        return;

    OUString aName = rStrm.ReadUniString();
    if( aName.isEmpty() )
        return;

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

ScDocumentImport& XclImpRoot::GetDocImport()
{
    return *mrImpData.mxDocImport;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
