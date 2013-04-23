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

#ifndef _SV_IMGCTRL_HXX
#define _SV_IMGCTRL_HXX

#include <vcl/dllapi.h>

#include <vcl/fixed.hxx>
#include <vcl/bitmapex.hxx>

// ----------------
// - ImageControl -
// ----------------

class VCL_DLLPUBLIC ImageControl : public FixedImage
{
private:
    ::sal_Int16     mnScaleMode;

public:
                    ImageControl( Window* pParent, WinBits nStyle = 0 );
                    ImageControl( Window* pParent, const ResId& rResId );

    // set/get the scale mode. This is one of the css.awt.ImageScaleMode constants
    void            SetScaleMode( const ::sal_Int16 _nMode );
    ::sal_Int16     GetScaleMode() const { return mnScaleMode; }

    virtual void    Resize();
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    GetFocus();
    virtual void    LoseFocus();

protected:
    void    ImplDraw( OutputDevice& rDev, sal_uLong nDrawFlags, const Point& rPos, const Size& rSize ) const;
};

#endif  // _SV_IMGCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
