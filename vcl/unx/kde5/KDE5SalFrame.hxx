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

#pragma once

#include <memory>

#include <unx/saldisp.hxx>
#include <unx/salframe.h>
#include <unx/salgdi.h>

#include "KDE5SalGraphics.hxx"

class QWidget;
class KDE5SalGraphics;

class KDE5SalFrame : public SalFrame
{
    private:
        static const int nMaxGraphics = 2;

        struct GraphicsHolder
        {
            std::unique_ptr<KDE5SalGraphics> pGraphics;
            bool bInUse;

            GraphicsHolder() : bInUse( false ) {}
        };

        GraphicsHolder m_aGraphics[ nMaxGraphics ];

        QWidget* m_pWindow;

    public:
        KDE5SalFrame( SalFrame* pParent, SalFrameStyleFlags nStyle );

        virtual SalGraphics* AcquireGraphics() override;
        virtual void ReleaseGraphics( SalGraphics *pGraphics ) override;
        void updateGraphics( bool bClear );
        virtual void UpdateSettings( AllSettings& rSettings ) override;
        virtual void Show( bool bVisible, bool bNoActivate = false ) override;

        virtual bool PostEvent(ImplSVEvent* pData) override { return false; }

        virtual void SetTitle( const OUString& rTitle ) override { return; }
        virtual void SetIcon( sal_uInt16 nIcon ) override { return; }

        virtual void SetMenu( SalMenu *pSalMenu ) override { return; }
        virtual void DrawMenuBar() override { return; }

        virtual void SetExtendedFrameStyle( SalExtStyle nExtStyle ) override { return; }
        virtual void SetMinClientSize( long nWidth, long nHeight ) override { return; }
        virtual void SetMaxClientSize( long nWidth, long nHeight ) override { return; }

        virtual void SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags ) override { return; }
        virtual void GetClientSize( long& rWidth, long& rHeight ) override { return; }
        virtual void GetWorkArea( tools::Rectangle& rRect ) override { return; }
        virtual SalFrame* GetParent() const override { return nullptr; }
        virtual void SetWindowState( const SalFrameState* pState ) override { return; }
        virtual bool GetWindowState( SalFrameState* pState ) override { return false; }
        virtual void ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay ) override { return; }
        virtual void StartPresentation( bool bStart ) override { return; }
        virtual void SetAlwaysOnTop( bool bOnTop ) override { return; }

        virtual void  ToTop( SalFrameToTop nFlags ) override { return; }
        virtual void  SetPointer( PointerStyle ePointerStyle ) override { return; }
        virtual void  CaptureMouse( bool bMouse ) override { return; }
        virtual void  SetPointerPos( long nX, long nY ) override { return; }

        virtual void SetInputContext( SalInputContext* pContext ) override { return; }
        virtual void EndExtTextInput( EndExtTextInputFlags nFlags ) override { return; }
        virtual OUString GetKeyName( sal_uInt16 nKeyCode ) override { return OUString(); }
        virtual bool MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) override { return false; }

        virtual LanguageType GetInputLanguage() override { return LANGUAGE_SYSTEM; }
        virtual void Beep() override { return; }
        virtual const SystemEnvData* GetSystemData() const override { return nullptr; }
        virtual SalPointerState GetPointerState() override { return SalPointerState(); }
        virtual KeyIndicatorState GetIndicatorState() override { return KeyIndicatorState(); }
        virtual void SimulateKeyPress( sal_uInt16 nKeyCode ) override { return; }

        // set new parent window
        virtual void SetParent( SalFrame* pNewParent ) override { return; }
        virtual bool SetPluginParent( SystemParentData* pNewParent ) override { return false; }

        virtual void SetScreenNumber( unsigned int ) override { return; }
        virtual void SetApplicationID( const OUString &rWMClass ) override { return; }

        virtual void ResetClipRegion() override { return; }
        virtual void BeginSetClipRegion( sal_uLong nRects ) override { return; }
        virtual void UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) override { return; }
        virtual void EndSetClipRegion() override { return; }

        virtual void Flush() override { return; }

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
