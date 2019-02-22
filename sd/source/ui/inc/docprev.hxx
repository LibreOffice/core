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

#ifndef INCLUDED_SD_SOURCE_UI_INC_DOCPREV_HXX
#define INCLUDED_SD_SOURCE_UI_INC_DOCPREV_HXX

#include <tools/gen.hxx>
#include <tools/wintypes.hxx>

#include <rtl/ref.hxx>

#include <svl/lstner.hxx>
#include <sddllapi.h>

namespace sd {
    class SlideShow;
}

class SD_DLLPUBLIC SdDocPreviewWin final : public Control, public SfxListener
{
    rtl::Reference< sd::SlideShow > mxSlideShow;

    virtual void    Paint( vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect ) override;
    virtual Size    GetOptimalSize() const override;
    static void     ImpPaint( OutputDevice* pVDev );

    static const int FRAME;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

public:
                    SdDocPreviewWin( vcl::Window* pParent, const WinBits nStyle );
                    virtual ~SdDocPreviewWin() override;
    virtual void    dispose() override;
    virtual void    Resize() override;

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
