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

#include "richtextengine.hxx"
#include <svl/itempool.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <vcl/svapp.hxx>
#include <tools/mapunit.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <unotools/lingucfg.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <memory>

namespace frm
{
    //= RichTextEngine


    RichTextEngine* RichTextEngine::Create()
    {
        SolarMutexGuard g;

        rtl::Reference<SfxItemPool> pPool = EditEngine::CreatePool();
        RichTextEngine* pReturn = new RichTextEngine( pPool.get() );
        OutputDevice* pOutputDevice = pReturn->GetRefDevice();
        const MapMode& aDeviceMapMode( pOutputDevice->GetMapMode() );

        pReturn->SetStatusEventHdl( LINK( pReturn, RichTextEngine, EditEngineStatusChanged ) );

        pPool->SetDefaultMetric(aDeviceMapMode.GetMapUnit());

        // defaults
        vcl::Font aFont = Application::GetSettings().GetStyleSettings().GetAppFont();
        aFont.SetFamilyName( u"Times New Roman"_ustr );
        pPool->SetUserDefaultItem( SvxFontItem( aFont.GetFamilyType(), aFont.GetFamilyName(), OUString(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO ) );

        // 12 pt font size
        MapMode aPointMapMode( MapUnit::MapPoint );
        Size a12PointSize( OutputDevice::LogicToLogic( Size( 12, 0 ), aPointMapMode, aDeviceMapMode ) );
        pPool->SetUserDefaultItem( SvxFontHeightItem( a12PointSize.Width(), 100, EE_CHAR_FONTHEIGHT ) );

        // font languages
        SvtLinguOptions aLinguOpt;
        pPool->SetUserDefaultItem( SvxLanguageItem( aLinguOpt.nDefaultLanguage, EE_CHAR_LANGUAGE ) );
        pPool->SetUserDefaultItem( SvxLanguageItem( aLinguOpt.nDefaultLanguage_CJK, EE_CHAR_LANGUAGE_CJK ) );
        pPool->SetUserDefaultItem( SvxLanguageItem( aLinguOpt.nDefaultLanguage_CTL, EE_CHAR_LANGUAGE_CTL ) );

        return pReturn;
    }


    RichTextEngine* RichTextEngine::Clone()
    {
        RichTextEngine* pClone( nullptr );
        {
            SolarMutexGuard aGuard;
            std::unique_ptr<EditTextObject> pMyText(CreateTextObject());
            OSL_ENSURE( pMyText, "RichTextEngine::Clone: CreateTextObject returned nonsense!" );

            pClone = Create();

            if ( pMyText )
                pClone->SetText( *pMyText );
        }

        return pClone;
    }


    RichTextEngine::RichTextEngine( SfxItemPool* _pPool )
        :EditEngine( _pPool )
    {
    }


    RichTextEngine::~RichTextEngine( )
    {
    }


    void RichTextEngine::registerEngineStatusListener( IEngineStatusListener* _pListener )
    {
        OSL_ENSURE( _pListener, "RichTextEngine::registerEngineStatusListener: invalid listener!" );
        if ( _pListener )
            m_aStatusListeners.push_back( _pListener );
    }


    void RichTextEngine::revokeEngineStatusListener( IEngineStatusListener const * _pListener )
    {
        ::std::vector< IEngineStatusListener* >::iterator aPos = ::std::find(
            m_aStatusListeners.begin(),
            m_aStatusListeners.end(),
            _pListener
        );
        OSL_ENSURE( aPos != m_aStatusListeners.end(), "RichTextEngine::revokeEngineStatusListener: listener not registered!" );
        if ( aPos != m_aStatusListeners.end() )
            m_aStatusListeners.erase( aPos );
    }


    IMPL_LINK( RichTextEngine, EditEngineStatusChanged, EditStatus&, _rStatus, void )
    {
        for (auto const& statusListener : m_aStatusListeners)
            statusListener->EditEngineStatusChanged( _rStatus );
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
