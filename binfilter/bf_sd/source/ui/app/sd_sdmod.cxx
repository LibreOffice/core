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

#include <vcl/virdev.hxx>

#include <bf_svtools/ehdl.hxx>

#include <bf_svx/eeitem.hxx>
#include <bf_svx/svdfield.hxx>
#include <bf_svx/outliner.hxx>

#define _SD_DLL                 // fuer SD_MOD()
#include "optsitem.hxx"
#include "bf_sd/docshell.hxx"
#include "drawdoc.hxx"
#include "glob.hrc"
#include "strings.hrc"

#include <legacysmgr/legacy_binfilters_smgr.hxx>

namespace binfilter {

TYPEINIT1( SdModuleDummy, SfxModule );
TYPEINIT1( SdModule, SdModuleDummy );

SFX_IMPL_MODULE_DLL(Sd)

SdModule::SdModule(SvFactory* pDrawObjFact, SvFactory* pGraphicObjFact)
: SdModuleDummy(SFX_APP()->CreateResManager("bf_sd"), FALSE, pDrawObjFact, pGraphicObjFact)
, pImpressOptions(NULL)
, pDrawOptions(NULL)
{
    SetName( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarDraw" ) ) );	// Nicht uebersetzen!
    StartListening( *SFX_APP() );

    mpErrorHdl = new SfxErrorHandler( RID_SD_ERRHDL,
                                         ERRCODE_AREA_SD,
                                         ERRCODE_AREA_SD_END,
                                         GetResMgr() );

    mpVirtualRefDevice = new VirtualDevice;
    mpVirtualRefDevice->SetMapMode( MAP_100TH_MM );
}

SdModule::~SdModule()
{
    delete mpErrorHdl;
    delete static_cast< VirtualDevice* >( mpVirtualRefDevice );
}

SfxModule* SdModuleDummy::Load()
{
    return (NULL);
}

SfxModule* SdModule::Load()
{
    return (this);
}

IMPL_LINK(SdModule, CalcFieldValueHdl, EditFieldInfo*, pInfo)
{
    if( pInfo )
    {
        const String aStr( RTL_CONSTASCII_STRINGPARAM( "???" ) );
        pInfo->SetRepresentation( aStr );
    }

    return(0);
}

void SdModule::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( rHint.ISA( SfxSimpleHint ) &&
        ( (SfxSimpleHint&) rHint ).GetId() == SFX_HINT_DEINITIALIZING )
    {
         delete pImpressOptions, pImpressOptions = NULL;
         delete pDrawOptions, pDrawOptions = NULL;
    }
}

void SdModule::Free()
{
}

SdOptions* SdModule::GetSdOptions(DocumentType eDocType)
{
    SdOptions* pOptions = NULL;
    if (eDocType == DOCUMENT_TYPE_DRAW)
    {
        if (!pDrawOptions)
            pDrawOptions = new SdOptions( SDCFG_DRAW );
        pOptions = pDrawOptions;
    }
    else if (eDocType == DOCUMENT_TYPE_IMPRESS)
    {
        if (!pImpressOptions)
            pImpressOptions = new SdOptions( SDCFG_IMPRESS );
        pOptions = pImpressOptions;
    }
    if( pOptions )
     {
         UINT16 nMetric = pOptions->GetMetric();
         SdDrawDocShell* pDocSh = PTR_CAST( SdDrawDocShell, SfxObjectShell::Current() );
         SdDrawDocument* pDoc = NULL;
         if (pDocSh)
            pDoc = pDocSh->GetDoc();
        if( nMetric != 0xffff && pDoc && eDocType == pDoc->GetDocumentType() )
            PutItem( SfxUInt16Item( SID_ATTR_METRIC, nMetric ) );
    }
    return(pOptions);
}

OutputDevice* SdModule::GetVirtualRefDevice (void)
{
    return mpVirtualRefDevice;
}

OutputDevice* SdModule::GetRefDevice (SdDrawDocShell& rDocShell)
{
    return GetVirtualRefDevice();
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
