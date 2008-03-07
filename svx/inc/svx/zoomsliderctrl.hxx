/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoomsliderctrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:14:55 $
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

#ifndef _ZOOMSLIDER_STBCONTRL_HXX
#define _ZOOMSLIDER_STBCONTRL_HXX

// include ---------------------------------------------------------------

#include <sfx2/stbitem.hxx>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxZoomSliderControl ----------------------------------------

class SVX_DLLPUBLIC SvxZoomSliderControl : public SfxStatusBarControl
{
private:

    struct SvxZoomSliderControl_Impl;
    SvxZoomSliderControl_Impl* mpImpl;

    USHORT Offset2Zoom( long nOffset ) const;
    long Zoom2Offset( USHORT nZoom ) const;

public:

    SFX_DECL_STATUSBAR_CONTROL();

    SvxZoomSliderControl( USHORT _nSlotId, USHORT _nId, StatusBar& _rStb );
    ~SvxZoomSliderControl();

    virtual void  StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void  Paint( const UserDrawEvent& rEvt );
    virtual BOOL  MouseButtonDown( const MouseEvent & );
    virtual BOOL  MouseMove( const MouseEvent & rEvt );
};

#endif

