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

#include "Kf5Frame.hxx"

Kf5Frame::Kf5Frame::Kf5Frame( Kf5Instance* pInstance,
                 SalFrame* pParent,
                 SalFrameStyleFlags nSalFrameStyle )
{
}

Kf5Frame::~Kf5Frame()
{
}

SalGraphics* Kf5Frame::AcquireGraphics()
{
    return nullptr;
}

void Kf5Frame::ReleaseGraphics( SalGraphics* pGraphics )
{
}


bool Kf5Frame::PostEvent(ImplSVEvent* pData)
{
    return false;
}

void Kf5Frame::SetTitle( const OUString& rTitle )
{
}

void Kf5Frame::SetIcon( sal_uInt16 nIcon )
{
}

void Kf5Frame::SetMenu( SalMenu* pMenu )
{
}

void Kf5Frame::DrawMenuBar()
{
}

void Kf5Frame::SetExtendedFrameStyle( SalExtStyle nExtStyle )
{
}

void Kf5Frame::Show( bool bVisible, bool bNoActivate )
{
}

void Kf5Frame::SetMinClientSize( long nWidth, long nHeight )
{
}

void Kf5Frame::SetMaxClientSize( long nWidth, long nHeight )
{
}

void Kf5Frame::SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags )
{
}

void Kf5Frame::GetClientSize( long& rWidth, long& rHeight )
{
}

void Kf5Frame::GetWorkArea( tools::Rectangle& rRect )
{
}

SalFrame* Kf5Frame::GetParent() const
{
    return nullptr;
}

void Kf5Frame::SetWindowState( const SalFrameState* pState )
{
}

bool Kf5Frame::GetWindowState( SalFrameState* pState )
{
    return false;
}

void Kf5Frame::ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay )
{
}

void Kf5Frame::StartPresentation( bool bStart )
{
}

void Kf5Frame::SetAlwaysOnTop( bool bOnTop )
{
}

void Kf5Frame::ToTop( SalFrameToTop nFlags )
{
}

void Kf5Frame::SetPointer( PointerStyle ePointerStyle )
{
}

void Kf5Frame::CaptureMouse( bool bMouse )
{
}

void Kf5Frame::SetPointerPos( long nX, long nY )
{
}

void Kf5Frame::Flush()
{
}

void Kf5Frame::Flush( const tools::Rectangle& rRect )
{
}

void Kf5Frame::SetInputContext( SalInputContext* pContext )
{
}

void Kf5Frame::EndExtTextInput( EndExtTextInputFlags nFlags )
{
}

OUString Kf5Frame::GetKeyName( sal_uInt16 nKeyCode )
{
    return OUString();
}

bool Kf5Frame::MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode )
{
    return false;
}

LanguageType Kf5Frame::GetInputLanguage()
{
    return LANGUAGE_DONTKNOW;
}

void Kf5Frame::UpdateSettings( AllSettings& rSettings )
{
}

void Kf5Frame::Beep()
{
}

const SystemEnvData* Kf5Frame::GetSystemData() const
{
    return nullptr;
}

SalFrame::SalPointerState Kf5Frame::GetPointerState()
{
    return SalPointerState();
}

KeyIndicatorState Kf5Frame::GetIndicatorState()
{
    return KeyIndicatorState();
}

void Kf5Frame::SimulateKeyPress( sal_uInt16 nKeyCode )
{
}

void Kf5Frame::SetParent( SalFrame* pNewParent )
{
}

bool Kf5Frame::SetPluginParent( SystemParentData* pNewParent )
{
    return false;
}

void Kf5Frame::ResetClipRegion()
{
}

void Kf5Frame::BeginSetClipRegion( sal_uLong nRects )
{
}

void Kf5Frame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
}

void Kf5Frame::EndSetClipRegion()
{
}

void Kf5Frame::SetScreenNumber( unsigned int )
{
}

void Kf5Frame::SetApplicationID(const OUString &)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
