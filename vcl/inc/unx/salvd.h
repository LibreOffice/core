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



#ifndef _SV_SALVD_H
#define _SV_SALVD_H

// -=-= #includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <unx/salstd.hxx>
#include <salvd.hxx>

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalDisplay;
class X11SalGraphics;

// -=-= SalVirDevData -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class X11SalVirtualDevice : public SalVirtualDevice
{
    SalDisplay      *pDisplay_;
    X11SalGraphics  *pGraphics_;


    Pixmap          hDrawable_;
    int             m_nScreen;

    int             nDX_;
    int             nDY_;
    sal_uInt16          nDepth_;
    sal_Bool            bGraphics_;         // is Graphics used
    sal_Bool            bExternPixmap_;

public:
    X11SalVirtualDevice();
    virtual ~X11SalVirtualDevice();

    sal_Bool            Init( SalDisplay *pDisplay,
                          long nDX, long nDY,
                          sal_uInt16 nBitCount,
                          int nScreen,
                          Pixmap hDrawable = None,
                          void* pRenderFormat = NULL );
    inline  void            InitGraphics( X11SalVirtualDevice *pVD );

    inline  Display        *GetXDisplay() const;
    inline  SalDisplay     *GetDisplay() const;
    inline  sal_Bool            IsDisplay() const;
    inline  Pixmap          GetDrawable() const { return hDrawable_; }
    inline  sal_uInt16          GetDepth() const { return nDepth_; }
    int                     GetWidth() const { return nDX_; }
    int                     GetHeight() const { return nDY_; }
    int                     GetScreenNumber() const { return m_nScreen; }

    virtual SalGraphics*    GetGraphics();
    virtual void            ReleaseGraphics( SalGraphics* pGraphics );

                            // Set new size, without saving the old contents
    virtual sal_Bool            SetSize( long nNewDX, long nNewDY );
    virtual void            GetSize( long& rWidth, long& rHeight );
};

#ifdef _SV_SALDISP_HXX

inline void X11SalVirtualDevice::InitGraphics( X11SalVirtualDevice *pVD )
{ pGraphics_->Init( pVD ); }

inline Display *X11SalVirtualDevice::GetXDisplay() const
{ return pDisplay_->GetDisplay(); }

inline SalDisplay *X11SalVirtualDevice::GetDisplay() const
{ return pDisplay_; }

inline sal_Bool X11SalVirtualDevice::IsDisplay() const
{ return pDisplay_->IsDisplay(); }

#endif

#endif // _SV_SALVD_H

