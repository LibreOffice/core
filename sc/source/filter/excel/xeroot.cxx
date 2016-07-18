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
#include <xepivotxml.hxx>
#include "xedbdata.hxx"

#include "excrecds.hxx"
#include "tabprotection.hxx"
#include "document.hxx"
#include "scextopt.hxx"

#include "formulabase.hxx"
#include <com/sun/star/sheet/FormulaOpCodeMapEntry.hpp>

using namespace ::com::sun::star;

// Global data ================================================================

XclExpRootData::XclExpRootData( XclBiff eBiff, SfxMedium& rMedium,
        tools::SvRef<SotStorage> xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc ) :
    XclRootData( eBiff, rMedium, xRootStrg, rDoc, eTextEnc, true )
{
    SvtSaveOptions aSaveOpt;
    mbRelUrl = mrMedium.IsRemote() ? aSaveOpt.IsSaveRelINet() : aSaveOpt.IsSaveRelFSys();
    maStringBuf = OStringBuffer();
}

XclExpRootData::~XclExpRootData()
{
}

XclExpRoot::XclExpRoot( XclExpRootData& rExpRootData ) :
    XclRoot( rExpRootData ),
    mrExpData( rExpRootData )
{
}

XclExpTabInfo& XclExpRoot::GetTabInfo() const
{
    OSL_ENSURE( mrExpData.mxTabInfo, "XclExpRoot::GetTabInfo - missing object (wrong BIFF?)" );
    return *mrExpData.mxTabInfo;
}

XclExpAddressConverter& XclExpRoot::GetAddressConverter() const
{
    OSL_ENSURE( mrExpData.mxAddrConv, "XclExpRoot::GetAddressConverter - missing object (wrong BIFF?)" );
    return *mrExpData.mxAddrConv;
}

XclExpFormulaCompiler& XclExpRoot::GetFormulaCompiler() const
{
    OSL_ENSURE( mrExpData.mxFmlaComp, "XclExpRoot::GetFormulaCompiler - missing object (wrong BIFF?)" );
    return *mrExpData.mxFmlaComp;
}

XclExpProgressBar& XclExpRoot::GetProgressBar() const
{
    OSL_ENSURE( mrExpData.mxProgress, "XclExpRoot::GetProgressBar - missing object (wrong BIFF?)" );
    return *mrExpData.mxProgress;
}

XclExpSst& XclExpRoot::GetSst() const
{
    OSL_ENSURE( mrExpData.mxSst, "XclExpRoot::GetSst - missing object (wrong BIFF?)" );
    return *mrExpData.mxSst;
}

XclExpPalette& XclExpRoot::GetPalette() const
{
    OSL_ENSURE( mrExpData.mxPalette, "XclExpRoot::GetPalette - missing object (wrong BIFF?)" );
    return *mrExpData.mxPalette;
}

XclExpFontBuffer& XclExpRoot::GetFontBuffer() const
{
    OSL_ENSURE( mrExpData.mxFontBfr, "XclExpRoot::GetFontBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxFontBfr;
}

XclExpNumFmtBuffer& XclExpRoot::GetNumFmtBuffer() const
{
    OSL_ENSURE( mrExpData.mxNumFmtBfr, "XclExpRoot::GetNumFmtBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxNumFmtBfr;
}

XclExpXFBuffer& XclExpRoot::GetXFBuffer() const
{
    OSL_ENSURE( mrExpData.mxXFBfr, "XclExpRoot::GetXFBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxXFBfr;
}

XclExpLinkManager& XclExpRoot::GetGlobalLinkManager() const
{
    OSL_ENSURE( mrExpData.mxGlobLinkMgr, "XclExpRoot::GetGlobalLinkManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxGlobLinkMgr;
}

XclExpLinkManager& XclExpRoot::GetLocalLinkManager() const
{
    OSL_ENSURE( GetLocalLinkMgrRef(), "XclExpRoot::GetLocalLinkManager - missing object (wrong BIFF?)" );
    return *GetLocalLinkMgrRef();
}

XclExpNameManager& XclExpRoot::GetNameManager() const
{
    OSL_ENSURE( mrExpData.mxNameMgr, "XclExpRoot::GetNameManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxNameMgr;
}

XclExpObjectManager& XclExpRoot::GetObjectManager() const
{
    OSL_ENSURE( mrExpData.mxObjMgr, "XclExpRoot::GetObjectManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxObjMgr;
}

XclExpFilterManager& XclExpRoot::GetFilterManager() const
{
    OSL_ENSURE( mrExpData.mxFilterMgr, "XclExpRoot::GetFilterManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxFilterMgr;
}

XclExpDxfs& XclExpRoot::GetDxfs() const
{
    OSL_ENSURE( mrExpData.mxDxfs, "XclExpRoot::GetDxfs - missing object ( wrong BIFF?)" );
    return *mrExpData.mxDxfs;
}

XclExpPivotTableManager& XclExpRoot::GetPivotTableManager() const
{
    OSL_ENSURE( mrExpData.mxPTableMgr, "XclExpRoot::GetPivotTableManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxPTableMgr;
}

XclExpXmlPivotTableManager& XclExpRoot::GetXmlPivotTableManager()
{
    assert(mrExpData.mxXmlPTableMgr);
    return *mrExpData.mxXmlPTableMgr;
}

XclExpTablesManager& XclExpRoot::GetTablesManager()
{
    assert(mrExpData.mxTablesMgr);
    return *mrExpData.mxTablesMgr;
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
        mrExpData.mxDxfs.reset( new XclExpDxfs( GetRoot() ) );
    }

    if( GetOutput() == EXC_OUTPUT_XML_2007 )
    {
        mrExpData.mxXmlPTableMgr.reset(new XclExpXmlPivotTableManager(GetRoot()));
        mrExpData.mxTablesMgr.reset(new XclExpTablesManager(GetRoot()));

        do
        {
            ScDocument& rDoc = GetDoc();
            // Pass the model factory to OpCodeProvider, not the process
            // service factory, otherwise a FormulaOpCodeMapperObj would be
            // instantiated instead of a ScFormulaOpCodeMapperObj and the
            // ScCompiler virtuals not be called! Which would be the case with
            // the current (2013-01-24) rDoc.GetServiceManager()
            const SfxObjectShell* pShell = rDoc.GetDocumentShell();
            if (!pShell)
            {
                SAL_WARN( "sc", "XclExpRoot::InitializeGlobals - no object shell");
                break;
            }
            uno::Reference< lang::XComponent > xComponent( pShell->GetModel(), uno::UNO_QUERY);
            if (!xComponent.is())
            {
                SAL_WARN( "sc", "XclExpRoot::InitializeGlobals - no component");
                break;
            }
            uno::Reference< lang::XMultiServiceFactory > xModelFactory( xComponent, uno::UNO_QUERY);
            oox::xls::OpCodeProvider aOpCodeProvider(xModelFactory, false);
            // Compiler mocks about non-matching ctor or conversion from
            // Sequence<...> to Sequence<const ...> if directly created or passed,
            // conversion through Any works around.
            uno::Any aAny( aOpCodeProvider.getOoxParserMap());
            uno::Sequence< const sheet::FormulaOpCodeMapEntry > aOpCodeMapping;
            if (!(aAny >>= aOpCodeMapping))
            {
                SAL_WARN( "sc", "XclExpRoot::InitializeGlobals - no OpCodeMap");
                break;
            }
            ScCompiler aCompiler( &rDoc, ScAddress());
            aCompiler.SetGrammar( rDoc.GetGrammar());
            mrExpData.mxOpCodeMap = formula::FormulaCompiler::CreateOpCodeMap( aOpCodeMapping, true);
        } while(false);
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
        case EXC_ID_DXFS:           xRec = mrExpData.mxDxfs;        break;
    }
    OSL_ENSURE( xRec, "XclExpRoot::CreateRecord - unknown record ID or missing object" );
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

uno::Sequence< beans::NamedValue > XclExpRoot::GenerateEncryptionData( const OUString& aPass )
{
    uno::Sequence< beans::NamedValue > aEncryptionData;

    if ( !aPass.isEmpty() && aPass.getLength() < 16 )
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
        for( sal_Int32 nChar = 0; nChar < aPass.getLength(); ++nChar )
            pnPasswd[nChar] = aPass[nChar];

        ::msfilter::MSCodec_Std97 aCodec;
        aCodec.InitKey( pnPasswd, pnDocId );
        aEncryptionData = aCodec.GetEncryptionData();
    }

    return aEncryptionData;
}

uno::Sequence< beans::NamedValue > XclExpRoot::GetEncryptionData() const
{
    uno::Sequence< beans::NamedValue > aEncryptionData;
    const SfxUnoAnyItem* pEncryptionDataItem = SfxItemSet::GetItem<SfxUnoAnyItem>(GetMedium().GetItemSet(), SID_ENCRYPTIONDATA, false);
    if ( pEncryptionDataItem )
        pEncryptionDataItem->GetValue() >>= aEncryptionData;
    else
    {
        // try to get the encryption data from the password
        const SfxStringItem* pPasswordItem = SfxItemSet::GetItem<SfxStringItem>(GetMedium().GetItemSet(), SID_PASSWORD, false);
        if ( pPasswordItem && !pPasswordItem->GetValue().isEmpty() )
            aEncryptionData = GenerateEncryptionData( pPasswordItem->GetValue() );
    }

    return aEncryptionData;
}

uno::Sequence< beans::NamedValue > XclExpRoot::GenerateDefaultEncryptionData() const
{
    uno::Sequence< beans::NamedValue > aEncryptionData;
    if ( !GetDefaultPassword().isEmpty() )
        aEncryptionData = GenerateEncryptionData( GetDefaultPassword() );

    return aEncryptionData;
}

XclExpRootData::XclExpLinkMgrRef const & XclExpRoot::GetLocalLinkMgrRef() const
{
    return IsInGlobals() ? mrExpData.mxGlobLinkMgr : mrExpData.mxLocLinkMgr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
