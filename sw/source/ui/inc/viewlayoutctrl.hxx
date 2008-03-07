/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewlayoutctrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:05:27 $
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
#ifndef _VIEWLAYOUTCTRL_HXX
#define _VIEWLAYOUTCTRL_HXX

// include ---------------------------------------------------------------

#ifndef _SFXSTBITEM_HXX //autogen
#include <sfx2/stbitem.hxx>
#endif

// class SwViewLayoutControl ----------------------------------------------

class SwViewLayoutControl : public SfxStatusBarControl
{
private:

    struct SwViewLayoutControl_Impl;
    SwViewLayoutControl_Impl* mpImpl;

public:

    SFX_DECL_STATUSBAR_CONTROL();

    SwViewLayoutControl( USHORT nSlotId, USHORT nId, StatusBar& rStb );
    ~SwViewLayoutControl();

    virtual void  StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void  Paint( const UserDrawEvent& rEvt );
    virtual BOOL  MouseButtonDown( const MouseEvent & );
};

#endif

