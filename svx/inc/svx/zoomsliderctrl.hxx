/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _ZOOMSLIDER_STBCONTRL_HXX
#define _ZOOMSLIDER_STBCONTRL_HXX

// include ---------------------------------------------------------------

#include <sfx2/stbitem.hxx>
#include "svx/svxdllapi.h"

// class SvxZoomSliderControl ----------------------------------------

class SVX_DLLPUBLIC SvxZoomSliderControl : public SfxStatusBarControl
{
private:

    struct SvxZoomSliderControl_Impl;
    SvxZoomSliderControl_Impl* mpImpl;

    sal_uInt16 Offset2Zoom( long nOffset ) const;
    long Zoom2Offset( sal_uInt16 nZoom ) const;

public:

    SFX_DECL_STATUSBAR_CONTROL();

    SvxZoomSliderControl( sal_uInt16 _nSlotId, sal_uInt16 _nId, StatusBar& _rStb );
    ~SvxZoomSliderControl();

    virtual void  StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void  Paint( const UserDrawEvent& rEvt );
    virtual sal_Bool  MouseButtonDown( const MouseEvent & );
    virtual sal_Bool  MouseMove( const MouseEvent & rEvt );
};

#endif

