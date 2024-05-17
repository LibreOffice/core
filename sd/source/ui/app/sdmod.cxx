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
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/resmgr.hxx>
#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <svl/numformat.hxx>
#include <svl/intitem.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <comphelper/processfactory.hxx>
#include <svtools/ehdl.hxx>

#include <svx/svxids.hrc>
#include <svl/srchitem.hxx>
#include <svx/svxerr.hxx>

#include <svtools/colorcfg.hxx>

#include <sdmod.hxx>
#include <sdresid.hxx>
#include <optsitem.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <errhdl.hrc>

#define ShellClass_SdModule
#include <sdslots.hxx>

SFX_IMPL_INTERFACE(SdModule, SfxModule)

void SdModule::InitInterface_Impl()
{
    GetStaticInterface()->RegisterStatusBar(StatusBarId::DrawStatusBar);
}

// Ctor
SdModule::SdModule(SfxObjectFactory* pFact1, SfxObjectFactory* pFact2 )
:   SfxModule("sd"_ostr, {pFact1, pFact2}),
    pTransferClip(nullptr),
    pTransferDrag(nullptr),
    pTransferSelection(nullptr),
    pImpressOptions(nullptr),
    pDrawOptions(nullptr),
    bWaterCan(false),
    mbEventListenerAdded(false),
    mpColorConfig(new svtools::ColorConfig)
{
    SetName( u"StarDraw"_ustr );  // Do not translate!
    pSearchItem.reset( new SvxSearchItem(SID_SEARCH_ITEM) );
    pSearchItem->SetAppFlag(SvxSearchApp::DRAW);
    StartListening( *SfxGetpApp() );
    SvxErrorHandler::ensure();
    mpErrorHdl.reset( new SfxErrorHandler(RID_SD_ERRHDL, ErrCodeArea::Sd, ErrCodeArea::Sd, GetResLocale()) );

    // Create a new ref device and (by calling SetReferenceDevice())
    // set its resolution to 600 DPI.  This leads to a visually better
    // formatting of text in small sizes (6 point and below.)
    mpVirtualRefDevice.reset(VclPtr<VirtualDevice>::Create());
    mpVirtualRefDevice->SetMapMode(MapMode(MapUnit::Map100thMM));
    mpVirtualRefDevice->SetReferenceDevice ( VirtualDevice::RefDevMode::Dpi600 );
}

OUString SdResId(TranslateId aId)
{
    return Translate::get(aId, SD_MOD()->GetResLocale());
}

OUString SdResId(TranslateNId aContextSingularPlural, int nCardinality)
{
    return Translate::nget(aContextSingularPlural, nCardinality, SD_MOD()->GetResLocale());
}

// Dtor
SdModule::~SdModule()
{
    pSearchItem.reset();
    pNumberFormatter.reset();

    if (mbEventListenerAdded)
    {
        Application::RemoveEventListener( LINK( this, SdModule, EventListenerHdl ) );
    }

    mpErrorHdl.reset();
    mpVirtualRefDevice.disposeAndClear();
}

void SdModule::SetSearchItem(std::unique_ptr<SvxSearchItem> pItem)
{
    pSearchItem = std::move(pItem);
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
            pDrawOptions = new SdOptions(false);

        pOptions = pDrawOptions;
    }
    else if (eDocType == DocumentType::Impress)
    {
        if (!pImpressOptions)
            pImpressOptions = new SdOptions(true);

        pOptions = pImpressOptions;
    }

    return pOptions;
}

SvNumberFormatter* SdModule::GetNumberFormatter()
{
    if( !pNumberFormatter )
        pNumberFormatter.reset( new SvNumberFormatter( ::comphelper::getProcessComponentContext(), LANGUAGE_SYSTEM ) );

    return pNumberFormatter.get();
}

svtools::ColorConfig& SdModule::GetColorConfig()
{
    return *mpColorConfig;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
