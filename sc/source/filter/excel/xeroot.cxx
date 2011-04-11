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

#include <rtl/random.h>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <unotools/saveopt.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include "xecontent.hxx"
#include "xltracer.hxx"
#include "xeescher.hxx"
#include "xeformula.hxx"
#include "xehelper.hxx"
#include "xelink.hxx"
#include "xename.hxx"
#include "xepivot.hxx"
#include "xestyle.hxx"
#include "xeroot.hxx"

#include "excrecds.hxx"  // for filter manager
#include "tabprotection.hxx"
#include "document.hxx"
#include "scextopt.hxx"

using namespace ::com::sun::star;

// Global data ================================================================

XclExpRootData::XclExpRootData( XclBiff eBiff, SfxMedium& rMedium,
        SotStorageRef xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc ) :
    XclRootData( eBiff, rMedium, xRootStrg, rDoc, eTextEnc, true )
{
    SvtSaveOptions aSaveOpt;
    mbRelUrl = mrMedium.IsRemote() ? aSaveOpt.IsSaveRelINet() : aSaveOpt.IsSaveRelFSys();
}

XclExpRootData::~XclExpRootData()
{
}

// ----------------------------------------------------------------------------

XclExpRoot::XclExpRoot( XclExpRootData& rExpRootData ) :
    XclRoot( rExpRootData ),
    mrExpData( rExpRootData )
{
}

XclExpTabInfo& XclExpRoot::GetTabInfo() const
{
    DBG_ASSERT( mrExpData.mxTabInfo, "XclExpRoot::GetTabInfo - missing object (wrong BIFF?)" );
    return *mrExpData.mxTabInfo;
}

XclExpAddressConverter& XclExpRoot::GetAddressConverter() const
{
    DBG_ASSERT( mrExpData.mxAddrConv, "XclExpRoot::GetAddressConverter - missing object (wrong BIFF?)" );
    return *mrExpData.mxAddrConv;
}

XclExpFormulaCompiler& XclExpRoot::GetFormulaCompiler() const
{
    DBG_ASSERT( mrExpData.mxFmlaComp, "XclExpRoot::GetFormulaCompiler - missing object (wrong BIFF?)" );
    return *mrExpData.mxFmlaComp;
}

XclExpProgressBar& XclExpRoot::GetProgressBar() const
{
    DBG_ASSERT( mrExpData.mxProgress, "XclExpRoot::GetProgressBar - missing object (wrong BIFF?)" );
    return *mrExpData.mxProgress;
}

XclExpSst& XclExpRoot::GetSst() const
{
    DBG_ASSERT( mrExpData.mxSst, "XclExpRoot::GetSst - missing object (wrong BIFF?)" );
    return *mrExpData.mxSst;
}

XclExpPalette& XclExpRoot::GetPalette() const
{
    DBG_ASSERT( mrExpData.mxPalette, "XclExpRoot::GetPalette - missing object (wrong BIFF?)" );
    return *mrExpData.mxPalette;
}

XclExpFontBuffer& XclExpRoot::GetFontBuffer() const
{
    DBG_ASSERT( mrExpData.mxFontBfr, "XclExpRoot::GetFontBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxFontBfr;
}

XclExpNumFmtBuffer& XclExpRoot::GetNumFmtBuffer() const
{
    DBG_ASSERT( mrExpData.mxNumFmtBfr, "XclExpRoot::GetNumFmtBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxNumFmtBfr;
}

XclExpXFBuffer& XclExpRoot::GetXFBuffer() const
{
    DBG_ASSERT( mrExpData.mxXFBfr, "XclExpRoot::GetXFBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxXFBfr;
}

XclExpLinkManager& XclExpRoot::GetGlobalLinkManager() const
{
    DBG_ASSERT( mrExpData.mxGlobLinkMgr, "XclExpRoot::GetGlobalLinkManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxGlobLinkMgr;
}

XclExpLinkManager& XclExpRoot::GetLocalLinkManager() const
{
    DBG_ASSERT( GetLocalLinkMgrRef(), "XclExpRoot::GetLocalLinkManager - missing object (wrong BIFF?)" );
    return *GetLocalLinkMgrRef();
}

XclExpNameManager& XclExpRoot::GetNameManager() const
{
    DBG_ASSERT( mrExpData.mxNameMgr, "XclExpRoot::GetNameManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxNameMgr;
}

XclExpObjectManager& XclExpRoot::GetObjectManager() const
{
    DBG_ASSERT( mrExpData.mxObjMgr, "XclExpRoot::GetObjectManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxObjMgr;
}

XclExpFilterManager& XclExpRoot::GetFilterManager() const
{
    DBG_ASSERT( mrExpData.mxFilterMgr, "XclExpRoot::GetFilterManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxFilterMgr;
}

XclExpPivotTableManager& XclExpRoot::GetPivotTableManager() const
{
    DBG_ASSERT( mrExpData.mxPTableMgr, "XclExpRoot::GetPivotTableManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxPTableMgr;
}

void XclExpRoot::InitializeConvert()
{
    mrExpData.mxTabInfo.reset( new XclExpTabInfo( GetRoot() ) );
    mrExpData.mxAddrConv.reset( new XclExpAddressConverter( GetRoot() ) );
    mrExpData.mxFmlaComp.reset( new XclExpFormulaCompiler( GetRoot() ) );
    mrExpData.mxProgress.reset( new XclExpProgressBar( GetRoot() ) );

    GetProgressBar().Initialize();
}

void XclExpRoot::InitializeGlobals()
{
    SetCurrScTab( SCTAB_GLOBAL );

    if( GetBiff() >= EXC_BIFF5 )
    {
        mrExpData.mxPalette.reset( new XclExpPalette( GetRoot() ) );
        mrExpData.mxFontBfr.reset( new XclExpFontBuffer( GetRoot() ) );
        mrExpData.mxNumFmtBfr.reset( new XclExpNumFmtBuffer( GetRoot() ) );
        mrExpData.mxXFBfr.reset( new XclExpXFBuffer( GetRoot() ) );
        mrExpData.mxGlobLinkMgr.reset( new XclExpLinkManager( GetRoot() ) );
        mrExpData.mxNameMgr.reset( new XclExpNameManager( GetRoot() ) );
    }

    if( GetBiff() == EXC_BIFF8 )
    {
        mrExpData.mxSst.reset( new XclExpSst );
        mrExpData.mxObjMgr.reset( new XclExpObjectManager( GetRoot() ) );
        mrExpData.mxFilterMgr.reset( new XclExpFilterManager( GetRoot() ) );
        mrExpData.mxPTableMgr.reset( new XclExpPivotTableManager( GetRoot() ) );
        // BIFF8: only one link manager for all sheets
        mrExpData.mxLocLinkMgr = mrExpData.mxGlobLinkMgr;
    }

    GetXFBuffer().Initialize();
    GetNameManager().Initialize();
}

void XclExpRoot::InitializeTable( SCTAB nScTab )
{
    SetCurrScTab( nScTab );
    if( GetBiff() == EXC_BIFF5 )
    {
        // local link manager per sheet
        mrExpData.mxLocLinkMgr.reset( new XclExpLinkManager( GetRoot() ) );
    }
}

void XclExpRoot::InitializeSave()
{
    GetPalette().Finalize();
    GetXFBuffer().Finalize();
}

XclExpRecordRef XclExpRoot::CreateRecord( sal_uInt16 nRecId ) const
{
    XclExpRecordRef xRec;
    switch( nRecId )
    {
        case EXC_ID_PALETTE:        xRec = mrExpData.mxPalette;     break;
        case EXC_ID_FONTLIST:       xRec = mrExpData.mxFontBfr;     break;
        case EXC_ID_FORMATLIST:     xRec = mrExpData.mxNumFmtBfr;   break;
        case EXC_ID_XFLIST:         xRec = mrExpData.mxXFBfr;       break;
        case EXC_ID_SST:            xRec = mrExpData.mxSst;         break;
        case EXC_ID_EXTERNSHEET:    xRec = GetLocalLinkMgrRef();    break;
        case EXC_ID_NAME:           xRec = mrExpData.mxNameMgr;     break;
    }
    DBG_ASSERT( xRec, "XclExpRoot::CreateRecord - unknown record ID or missing object" );
    return xRec;
}

bool XclExpRoot::IsDocumentEncrypted() const
{
    // We need to encrypt the content when the document structure is protected.
    const ScDocProtection* pDocProt = GetDoc().GetDocProtection();
    if (pDocProt && pDocProt->isProtected() && pDocProt->isOptionEnabled(ScDocProtection::STRUCTURE))
        return true;

    if ( GetEncryptionData().getLength() > 0 )
        // Password is entered directly into the save dialog.
        return true;

    return false;
}

uno::Sequence< beans::NamedValue > XclExpRoot::GenerateEncryptionData( const ::rtl::OUString& aPass ) const
{
    uno::Sequence< beans::NamedValue > aEncryptionData;

    if ( aPass.getLength() > 0 && aPass.getLength() < 16 )
    {
        TimeValue aTime;
        osl_getSystemTime( &aTime );
        rtlRandomPool aRandomPool = rtl_random_createPool ();
        rtl_random_addBytes ( aRandomPool, &aTime, 8 );

        sal_uInt8 pnDocId[16];
        rtl_random_getBytes( aRandomPool, pnDocId, 16 );

        rtl_random_destroyPool( aRandomPool );

        sal_uInt16 pnPasswd[16];
        memset( pnPasswd, 0, sizeof( pnPasswd ) );
        for (xub_StrLen nChar = 0; nChar < aPass.getLength(); ++nChar )
            pnPasswd[nChar] = aPass.getStr()[nChar];

        ::msfilter::MSCodec_Std97 aCodec;
        aCodec.InitKey( pnPasswd, pnDocId );
        aEncryptionData = aCodec.GetEncryptionData();
    }

    return aEncryptionData;
}

uno::Sequence< beans::NamedValue > XclExpRoot::GetEncryptionData() const
{
    uno::Sequence< beans::NamedValue > aEncryptionData;
    SFX_ITEMSET_ARG( GetMedium().GetItemSet(), pEncryptionDataItem, SfxUnoAnyItem, SID_ENCRYPTIONDATA, false );
    if ( pEncryptionDataItem )
        pEncryptionDataItem->GetValue() >>= aEncryptionData;
    else
    {
        // try to get the encryption data from the password
        SFX_ITEMSET_ARG( GetMedium().GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD, false );
        if ( pPasswordItem && pPasswordItem->GetValue().Len() )
            aEncryptionData = GenerateEncryptionData( pPasswordItem->GetValue() );
    }

    return aEncryptionData;
}

uno::Sequence< beans::NamedValue > XclExpRoot::GenerateDefaultEncryptionData() const
{
    uno::Sequence< beans::NamedValue > aEncryptionData;
    if ( GetDefaultPassword().Len() > 0 )
        aEncryptionData = GenerateEncryptionData( GetDefaultPassword() );

    return aEncryptionData;
}

XclExpRootData::XclExpLinkMgrRef XclExpRoot::GetLocalLinkMgrRef() const
{
    return IsInGlobals() ? mrExpData.mxGlobLinkMgr : mrExpData.mxLocLinkMgr;
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
