/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include "dx_surfacegraphics.hxx"
#include "dx_impltools.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        struct GraphicsDeleter
        {
            COMReference<surface_type> mpSurface;
            HDC                        maHDC;

            GraphicsDeleter(const COMReference<surface_type>& rSurface, HDC hdc) :
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

    GraphicsSharedPtr createSurfaceGraphics(const COMReference<surface_type>& rSurface )
    {
        Gdiplus::Graphics* pGraphics;
        GraphicsSharedPtr  pRet;
        HDC aHDC;
        if( SUCCEEDED(rSurface->GetDC( &aHDC )) )
        {
            pGraphics = Gdiplus::Graphics::FromHDC( aHDC );
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

        throw uno::RuntimeException();
    }
}
