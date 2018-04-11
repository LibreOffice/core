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
#include <cairo.h>

#undef Status

#include <unx/saldisp.hxx>
#include <qt5/Qt5Frame.hxx>
#include <unx/salgdi.h>

#include "KDE5SalGraphics.hxx"

class QWidget;
class KDE5SalGraphics;

class KDE5SalFrame : public Qt5Frame
{
    private:
        static const int nMaxGraphics = 2;

        struct GraphicsHolder
        {
            std::unique_ptr<KDE5SalGraphics> pGraphics;
            bool bInUse;
            cairo_surface_t* pSurface;

            GraphicsHolder() : bInUse( false ), pSurface(nullptr) {}
        };

        GraphicsHolder m_aGraphics[ nMaxGraphics ];

        QWidget* m_pWindow;

    public:
        KDE5SalFrame( KDE5SalFrame* pParent, SalFrameStyleFlags nStyle, bool bUseCairo );

        virtual SalGraphics* AcquireGraphics() override;
        virtual void ReleaseGraphics( SalGraphics *pGraphics ) override;
        void updateGraphics( bool bClear );
        virtual void UpdateSettings( AllSettings& rSettings ) override;

        virtual void DrawMenuBar() override { return; }

        virtual LanguageType GetInputLanguage() override { return LANGUAGE_SYSTEM; }
        virtual void Beep() override { return; }
        virtual const SystemEnvData* GetSystemData() const override { return nullptr; }
        virtual SalPointerState GetPointerState() override { return SalPointerState(); }
        virtual KeyIndicatorState GetIndicatorState() override { return KeyIndicatorState(); }

        virtual void SetScreenNumber( unsigned int ) override { return; }

        cairo_t* getCairoContext() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
