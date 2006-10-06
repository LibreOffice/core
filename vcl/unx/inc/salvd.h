/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salvd.h,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-06 10:02:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

// -=-= #includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef _SALSTD_HXX
#include <salstd.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif

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
    USHORT          nDepth_;
    BOOL            bGraphics_;         // is Graphics used
    BOOL            bExternPixmap_;

public:
    X11SalVirtualDevice();
    virtual ~X11SalVirtualDevice();

    BOOL            Init( SalDisplay *pDisplay,
                          long nDX, long nDY,
                          USHORT nBitCount,
                          int nScreen,
                          Pixmap hDrawable = None,
                          void* pRenderFormat = NULL );
    inline  void            InitGraphics( X11SalVirtualDevice *pVD );

    inline  Display        *GetXDisplay() const;
    inline  SalDisplay     *GetDisplay() const;
    inline  BOOL            IsDisplay() const;
    inline  Pixmap          GetDrawable() const { return hDrawable_; }
    inline  USHORT          GetDepth() const { return nDepth_; }
    int                     GetWidth() const { return nDX_; }
    int                     GetHeight() const { return nDY_; }
    int                     GetScreenNumber() const { return m_nScreen; }

    virtual SalGraphics*    GetGraphics();
    virtual void            ReleaseGraphics( SalGraphics* pGraphics );

                            // Set new size, without saving the old contents
    virtual BOOL            SetSize( long nNewDX, long nNewDY );
    virtual void            GetSize( long& rWidth, long& rHeight );
};

#ifdef _SV_SALDISP_HXX

inline void X11SalVirtualDevice::InitGraphics( X11SalVirtualDevice *pVD )
{ pGraphics_->Init( pVD ); }

inline Display *X11SalVirtualDevice::GetXDisplay() const
{ return pDisplay_->GetDisplay(); }

inline SalDisplay *X11SalVirtualDevice::GetDisplay() const
{ return pDisplay_; }

inline BOOL X11SalVirtualDevice::IsDisplay() const
{ return pDisplay_->IsDisplay(); }

#endif

#endif // _SV_SALVD_H

