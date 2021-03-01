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

#include <vcl/floatwin.hxx>
#include <vcl/image.hxx>

class VCL_DLLPUBLIC BubbleWindow final : public FloatingWindow
{
    Point           maTipPos;
    vcl::Region     maBounds;
    tools::Polygon  maRectPoly;
    tools::Polygon  maTriPoly;
    OUString        maBubbleTitle;
    OUString        maBubbleText;
    Image           maBubbleImage;
    Size            maMaxTextSize;
    tools::Rectangle       maTitleRect;
    tools::Rectangle       maTextRect;
    tools::Long     mnTipOffset;

private:
    void            RecalcTextRects();

public:
                    BubbleWindow( vcl::Window* pParent, const OUString& rTitle,
                                  const OUString& rText, const Image& rImage );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    void            Resize() override;
    void            Show( bool bVisible = true );
    void            SetTipPosPixel( const Point& rTipPos ) { maTipPos = rTipPos; }
    void            SetTitleAndText( const OUString& rTitle, const OUString& rText,
                                     const Image& rImage );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
