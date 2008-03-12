/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: subtoolboxcontrol.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:08:36 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#include <vcl/toolbox.hxx>
#include <sfx2/app.hxx>
#include <svx/subtoolboxcontrol.hxx>
#include <svx/svxids.hrc>


SFX_IMPL_TOOLBOX_CONTROL( SvxSubToolBoxControl, SfxUInt16Item );

/**********************************************************************

**********************************************************************/

SvxSubToolBoxControl::SvxSubToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
: SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

/**********************************************************************

**********************************************************************/

SvxSubToolBoxControl::~SvxSubToolBoxControl()
{
}

/**********************************************************************

**********************************************************************/

SfxPopupWindow* SvxSubToolBoxControl::CreatePopupWindow()
{
    const sal_Char* pResource = 0;
    switch( GetSlotId() )
    {
    case SID_OPTIMIZE_TABLE:
        pResource = "private:resource/toolbar/optimizetablebar";
        break;
    }

    if( pResource )
        createAndPositionSubToolBar( rtl::OUString::createFromAscii( pResource ) );
    return NULL;
}

/**********************************************************************

**********************************************************************/

SfxPopupWindowType  SvxSubToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}
