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

#ifndef INCLUDED_VCL_IMGCTRL_HXX
#define INCLUDED_VCL_IMGCTRL_HXX

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <config_options.h>
#include <vcl/dllapi.h>
#include <vcl/toolkit/fixed.hxx>

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) ImageControl : public FixedImage
{
private:
    ::sal_Int16     mnScaleMode;

public:
                    ImageControl( vcl::Window* pParent, WinBits nStyle );

    // set/get the scale mode. This is one of the css.awt.ImageScaleMode constants
    void            SetScaleMode( const ::sal_Int16 _nMode );
    ::sal_Int16     GetScaleMode() const { return mnScaleMode; }

    virtual void    Resize() override;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, DrawFlags nFlags ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;

protected:
    void    ImplDraw( OutputDevice& rDev, const Point& rPos, const Size& rSize ) const;
};

#endif // INCLUDED_VCL_IMGCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
