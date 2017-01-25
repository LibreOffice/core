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

#include <unotools/pathoptions.hxx>
#include <svl/languageoptions.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <vcl/status.hxx>
#include <svl/intitem.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <svx/pszctrl.hxx>
#include <svx/zoomctrl.hxx>
#include <svx/modctrl.hxx>
#include <svl/zforlist.hxx>
#include <comphelper/processfactory.hxx>
#include <svtools/ehdl.hxx>

#include <svx/svxids.hrc>
#include <svl/srchitem.hxx>
#include <svx/svxerr.hxx>

#include <svx/xmlsecctrl.hxx>
#include <svtools/colorcfg.hxx>

#include "sderror.hxx"
#include "sdmod.hxx"
#include "sdresid.hxx"
#include "optsitem.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "cfgids.hxx"


#define SdModule
#include "sdslots.hxx"

SFX_IMPL_INTERFACE(SdModule, SfxModule)

void SdModule::InitInterface_Impl()
{
    GetStaticInterface()->RegisterStatusBar(RID_DRAW_STATUSBAR);
}

// Ctor
SdModule::SdModule(SfxObjectFactory* pFact1, SfxObjectFactory* pFact2 )
:   SfxModule( ResMgr::CreateResMgr("sd"), {pFact1, pFact2} ),
    pTransferClip(nullptr),
    pTransferDrag(nullptr),
    pTransferSelection(nullptr),
    pImpressOptions(nullptr),
    pDrawOptions(nullptr),
    pSearchItem(nullptr),
    pNumberFormatter( nullptr ),
    bWaterCan(false),
    mbEventListenerAdded(false),
    mpColorConfig(new svtools::ColorConfig)
{
    SetName( "StarDraw" );  // Do not translate!
    pSearchItem = new SvxSearchItem(SID_SEARCH_ITEM);
    pSearchItem->SetAppFlag(SvxSearchApp::DRAW);
    StartListening( *SfxGetpApp() );
    SvxErrorHandler::ensure();
    mpErrorHdl = new SfxErrorHandler( RID_SD_ERRHDL,
                                         ERRCODE_AREA_SD,
                                         ERRCODE_AREA_SD_END,
                                         GetResMgr() );

    // Create a new ref device and (by calling SetReferenceDevice())
    // set its resolution to 600 DPI.  This leads to a visually better
    // formatting of text in small sizes (6 point and below.)
    mpVirtualRefDevice.reset(VclPtr<VirtualDevice>::Create());
    mpVirtualRefDevice->SetMapMode( MapUnit::Map100thMM );
    mpVirtualRefDevice->SetReferenceDevice ( VirtualDevice::RefDevMode::Dpi600 );
}

// Dtor
SdModule::~SdModule()
{
    delete pSearchItem;
    delete pNumberFormatter;

    if (mbEventListenerAdded)
    {
        Application::RemoveEventListener( LINK( this, SdModule, EventListenerHdl ) );
    }

    delete mpErrorHdl;
    mpVirtualRefDevice.disposeAndClear();
}

/// get notifications
void SdModule::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if( rHint.GetId() == SfxHintId::Deinitializing )
    {
        delete pImpressOptions;
        pImpressOptions = nullptr;
        delete pDrawOptions;
        pDrawOptions = nullptr;
    }
}

/// Return options
SdOptions* SdModule::GetSdOptions(DocumentType eDocType)
{
    SdOptions* pOptions = nullptr;

    if (eDocType == DocumentType::Draw)
    {
        if (!pDrawOptions)
            pDrawOptions = new SdOptions( SDCFG_DRAW );

        pOptions = pDrawOptions;
    }
    else if (eDocType == DocumentType::Impress)
    {
        if (!pImpressOptions)
            pImpressOptions = new SdOptions( SDCFG_IMPRESS );

        pOptions = pImpressOptions;
    }
    if( pOptions )
    {
        sal_uInt16 nMetric = pOptions->GetMetric();

        ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
        SdDrawDocument* pDoc = nullptr;
        if (pDocSh)
            pDoc = pDocSh->GetDoc();

        if( nMetric != 0xffff && pDoc && eDocType == pDoc->GetDocumentType() )
            PutItem( SfxUInt16Item( SID_ATTR_METRIC, nMetric ) );
    }

    return pOptions;
}

/**
 * Open and return option stream for internal options;
 * if the stream is opened for reading but does not exist, an 'empty'
 * RefObject is returned
 */
tools::SvRef<SotStorageStream> SdModule::GetOptionStream( const OUString& rOptionName,
                                              SdOptionStreamMode eMode )
{
    ::sd::DrawDocShell*     pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
    tools::SvRef<SotStorageStream>  xStm;

    if( pDocSh )
    {
        DocumentType    eType = pDocSh->GetDoc()->GetDocumentType();

        if( !xOptionStorage.is() )
        {
            INetURLObject aURL( SvtPathOptions().GetUserConfigPath() );

            aURL.Append( "drawing.cfg" );

            SvStream* pStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READWRITE );

            if( pStm )
                xOptionStorage = new SotStorage( pStm, true );
        }

        OUString        aStmName;

        if( DocumentType::Draw == eType )
            aStmName = "Draw_";
        else
            aStmName = "Impress_";

        aStmName += rOptionName;

        if( SD_OPTION_STORE == eMode || xOptionStorage->IsContained( aStmName ) )
            xStm = xOptionStorage->OpenSotStream( aStmName );
    }

    return xStm;
}

SvNumberFormatter* SdModule::GetNumberFormatter()
{
    if( !pNumberFormatter )
        pNumberFormatter = new SvNumberFormatter( ::comphelper::getProcessComponentContext(), LANGUAGE_SYSTEM );

    return pNumberFormatter;
}

/** This method is deprecated and only an alias to
 *   <member>GetVirtualRefDevice()</member>.  The given argument is ignored.
 */
OutputDevice* SdModule::GetRefDevice (::sd::DrawDocShell& )
{
    return GetVirtualRefDevice();
}

svtools::ColorConfig& SdModule::GetColorConfig()
{
    return *mpColorConfig;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
