/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layctrl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:59:49 $
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
#ifndef _SVX_LAYCTRL_HXX
#define _SVX_LAYCTRL_HXX

// include ---------------------------------------------------------------


#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif
#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxTableToolBoxControl ------------------------------------------

class SVX_DLLPUBLIC SvxTableToolBoxControl : public SfxToolBoxControl
{
private:
    FASTBOOL    bEnabled;

public:
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual SfxPopupWindow*     CreatePopupWindowCascading();
    virtual void                StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );

    SFX_DECL_TOOLBOX_CONTROL();

    SvxTableToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxTableToolBoxControl();
};

// class SvxColumnsToolBoxControl ----------------------------------------

class SVX_DLLPUBLIC SvxColumnsToolBoxControl : public SfxToolBoxControl
{
    FASTBOOL    bEnabled;
public:
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual SfxPopupWindow*     CreatePopupWindowCascading();

    SFX_DECL_TOOLBOX_CONTROL();

    SvxColumnsToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxColumnsToolBoxControl();

    virtual void                StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );
};


#endif

