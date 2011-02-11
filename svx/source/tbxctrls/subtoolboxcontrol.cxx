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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svl/intitem.hxx>

#include <vcl/toolbox.hxx>
#include <sfx2/app.hxx>
#include <svx/subtoolboxcontrol.hxx>
#include <svx/svxids.hrc>


SFX_IMPL_TOOLBOX_CONTROL( SvxSubToolBoxControl, SfxUInt16Item );

/**********************************************************************

**********************************************************************/

SvxSubToolBoxControl::SvxSubToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
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
