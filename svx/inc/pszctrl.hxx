/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pszctrl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:22:48 $
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
#ifndef _SVX_PSZCTRL_HXX
#define _SVX_PSZCTRL_HXX

// include ---------------------------------------------------------------

#ifndef _SFXSTBITEM_HXX //autogen
#include <sfx2/stbitem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// forward ---------------------------------------------------------------

class  SvxSizeItem;
struct SvxPosSizeStatusBarControl_Impl;

// class SvxPosSizeToolBoxControl ----------------------------------------

class SVX_DLLPUBLIC SvxPosSizeStatusBarControl : public SfxStatusBarControl
{
private:
    SvxPosSizeStatusBarControl_Impl*    pImp;
public:
    SFX_DECL_STATUSBAR_CONTROL();

    SvxPosSizeStatusBarControl( USHORT nSlotId, USHORT nId, StatusBar& rStb );
    ~SvxPosSizeStatusBarControl();

    virtual void    StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void    Paint( const UserDrawEvent& rEvt );
    virtual void    Command( const CommandEvent& rCEvt );

    static ULONG    GetDefItemWidth( const StatusBar& rStb );
};


#endif

