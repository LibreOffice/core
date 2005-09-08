/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lboxctrl.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:00:05 $
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

#ifndef _SVX_LBOXCTRL_HXX_
#define _SVX_LBOXCTRL_HXX_

#ifndef _SFXTBXCTRL_HXX
#include <sfx2/tbxctrl.hxx>
#endif
#include <rtl/ustring.hxx>
#include <vector>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class ToolBox;
class SvxPopupWindowListBox;

/////////////////////////////////////////////////////////////////

class SvxListBoxControl : public SfxToolBoxControl
{
protected:
    String                  aActionStr;
    SvxPopupWindowListBox * pPopupWin;

    void    Impl_SetInfo( USHORT nCount );

    DECL_LINK( PopupModeEndHdl, void * );
    DECL_LINK( SelectHdl, void * );

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxListBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    virtual ~SvxListBoxControl();

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );
};

/////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SvxUndoRedoControl : public SvxListBoxControl
{
    std::vector< ::rtl::OUString > aUndoRedoList;
    ::rtl::OUString                aDefaultText;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxUndoRedoControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );

    virtual ~SvxUndoRedoControl();
    virtual void StateChanged( USHORT nSID,
                               SfxItemState eState,
                               const SfxPoolItem* pState );

    virtual SfxPopupWindow*  CreatePopupWindow();
};

#endif

