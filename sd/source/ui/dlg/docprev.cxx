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

#include <slideshow.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/settings.hxx>

#include <docprev.hxx>
#include <ViewShell.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

const int SdDocPreviewWin::FRAME = 4;

VCL_BUILDER_FACTORY_CONSTRUCTOR(SdDocPreviewWin, 0)

SdDocPreviewWin::SdDocPreviewWin( vcl::Window* pParent, const WinBits nStyle )
: Control(pParent, nStyle)
{
    SetBorderStyle( WindowBorderStyle::MONO );
    svtools::ColorConfig aColorConfig;
    Wallpaper aEmpty;
    SetBackground( aEmpty );
}

SdDocPreviewWin::~SdDocPreviewWin()
{
    disposeOnce();
}

void SdDocPreviewWin::dispose()
{
    if (mxSlideShow.is())
    {
        mxSlideShow->end();
        mxSlideShow.clear();
    }
    Control::dispose();
}

Size SdDocPreviewWin::GetOptimalSize() const
{
    return LogicToPixel(Size(122, 96), MapMode(MapUnit::MapAppFont));
}

void SdDocPreviewWin::Resize()
{
    Invalidate();
    if( mxSlideShow.is() )
        mxSlideShow->resize( GetSizePixel() );
}

void SdDocPreviewWin::ImpPaint( OutputDevice* pVDev )
{
    svtools::ColorConfig aColorConfig;

    pVDev->SetLineColor();
    pVDev->SetFillColor( aColorConfig.GetColorValue( svtools::APPBACKGROUND ).nColor );
    pVDev->DrawRect(::tools::Rectangle( Point(0,0 ), pVDev->GetOutputSize()));
}

void SdDocPreviewWin::Paint( vcl::RenderContext& /*rRenderContext*/, const ::tools::Rectangle& /*rRect*/ )
{
    if( (!mxSlideShow.is()) || (!mxSlideShow->isRunning() ) )
    {
        SvtAccessibilityOptions aAccOptions;
        bool bUseContrast = aAccOptions.GetIsForPagePreviews() && Application::GetSettings().GetStyleSettings().GetHighContrastMode();
        SetDrawMode( bUseContrast
            ? ::sd::OUTPUT_DRAWMODE_CONTRAST
            : ::sd::OUTPUT_DRAWMODE_COLOR );

        ImpPaint( this );
    }
    else
    {
        mxSlideShow->paint();
    }
}

void SdDocPreviewWin::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if( rHint.GetId() == SfxHintId::ColorsChanged )
    {
        Invalidate();
    }
}
void SdDocPreviewWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        Invalidate();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
