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

#include <sal/config.h>

#include "dx_impltools.hxx"
#include "dx_surfacegraphics.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        struct GraphicsDeleter
        {
            sal::systools::COMReference<surface_type> mpSurface;
            HDC                        maHDC;

            GraphicsDeleter(const sal::systools::COMReference<surface_type>& rSurface, HDC hdc) :
                mpSurface(rSurface),
                maHDC(hdc)
            {}

            void operator()( Gdiplus::Graphics* pGraphics )
            {
                if(!pGraphics)
                    return;

                pGraphics->Flush(Gdiplus::FlushIntentionSync);
                delete pGraphics;

                if(mpSurface.is())
                    mpSurface->ReleaseDC( maHDC );
            }
        };
    }

    GraphicsSharedPtr createSurfaceGraphics(const sal::systools::COMReference<surface_type>& rSurface )
    {
        GraphicsSharedPtr  pRet;
        HDC aHDC;
        if( SUCCEEDED(rSurface->GetDC( &aHDC )) )
        {
            Gdiplus::Graphics* pGraphics = Gdiplus::Graphics::FromHDC( aHDC );
            if(pGraphics)
            {
                tools::setupGraphics( *pGraphics );
                pRet.reset(pGraphics,
                           GraphicsDeleter(rSurface, aHDC));
                return pRet;
            }
            else
                rSurface->ReleaseDC( aHDC );
        }

        throw uno::RuntimeException("could not get the DC to rSurface");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
