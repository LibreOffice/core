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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
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

// Global data ================================================================

XclImpRootData::XclImpRootData( XclBiff eBiff, SfxMedium& rMedium,
        SotStorageRef xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc ) :
    XclRootData( eBiff, rMedium, xRootStrg, rDoc, eTextEnc, false ),
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
    GetDoc().SetCodeName( nScTab, String::EmptyString() );
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
    DBG_ASSERT( mrImpData.mxSst, "XclImpRoot::GetSst - invalid call, wrong BIFF" );
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
    DBG_ASSERT( mrImpData.mxLinkMgr, "XclImpRoot::GetLinkManager - invalid call, wrong BIFF" );
    return *mrImpData.mxLinkMgr;
}

XclImpObjectManager& XclImpRoot::GetObjectManager() const
{
    return *mrImpData.mxObjMgr;
}

XclImpSheetDrawing& XclImpRoot::GetCurrSheetDrawing() const
{
    DBG_ASSERT( !IsInGlobals(), "XclImpRoot::GetCurrSheetDrawing - must not be called from workbook globals" );
    return mrImpData.mxObjMgr->GetSheetDrawing( GetCurrScTab() );
}

XclImpCondFormatManager& XclImpRoot::GetCondFormatManager() const
{
    DBG_ASSERT( mrImpData.mxCondFmtMgr, "XclImpRoot::GetCondFormatManager - invalid call, wrong BIFF" );
    return *mrImpData.mxCondFmtMgr;
}

XclImpValidationManager& XclImpRoot::GetValidationManager() const
{
    DBG_ASSERT( mrImpData.mxValidMgr, "XclImpRoot::GetValidationManager - invalid call, wrong BIFF" );
    return *mrImpData.mxValidMgr;
}

XclImpAutoFilterBuffer& XclImpRoot::GetFilterManager() const
{
    // TODO still in old RootData
    DBG_ASSERT( GetOldRoot().pAutoFilterBuffer, "XclImpRoot::GetFilterManager - invalid call, wrong BIFF" );
    return *GetOldRoot().pAutoFilterBuffer;
}

XclImpWebQueryBuffer& XclImpRoot::GetWebQueryBuffer() const
{
    DBG_ASSERT( mrImpData.mxWebQueryBfr, "XclImpRoot::GetWebQueryBuffer - invalid call, wrong BIFF" );
    return *mrImpData.mxWebQueryBfr;
}

XclImpPivotTableManager& XclImpRoot::GetPivotTableManager() const
{
    DBG_ASSERT( mrImpData.mxPTableMgr, "XclImpRoot::GetPivotTableManager - invalid call, wrong BIFF" );
    return *mrImpData.mxPTableMgr;
}

XclImpSheetProtectBuffer& XclImpRoot::GetSheetProtectBuffer() const
{
    DBG_ASSERT( mrImpData.mxTabProtect, "XclImpRoot::GetSheetProtectBuffer - invalid call, wrong BIFF" );
    return *mrImpData.mxTabProtect;
}

XclImpDocProtectBuffer& XclImpRoot::GetDocProtectBuffer() const
{
    DBG_ASSERT( mrImpData.mxDocProtect, "XclImpRoot::GetDocProtectBuffer - invalid call, wrong BIFF" );
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
    String aScName;
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

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
