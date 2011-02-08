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

#include "svx/formatpaintbrushctrl.hxx"

// header for class SfxBoolItem
#include <svl/eitem.hxx>

// header for define SFX_APP
#include <sfx2/app.hxx>

// header for class ToolBox
#include <vcl/toolbox.hxx>

//.............................................................................
namespace svx
{
//.............................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

SFX_IMPL_TOOLBOX_CONTROL( FormatPaintBrushToolBoxControl, SfxBoolItem );

FormatPaintBrushToolBoxControl::FormatPaintBrushToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    : SfxToolBoxControl( nSlotId, nId, rTbx )
    , m_bPersistentCopy(false)
    , m_aDoubleClickTimer()
{
    sal_uIntPtr nDblClkTime = rTbx.GetSettings().GetMouseSettings().GetDoubleClickTime();

    m_aDoubleClickTimer.SetTimeoutHdl( LINK(this, FormatPaintBrushToolBoxControl, WaitDoubleClickHdl) );
    m_aDoubleClickTimer.SetTimeout(nDblClkTime);
}

// -----------------------------------------------------------------------

FormatPaintBrushToolBoxControl::~FormatPaintBrushToolBoxControl()
{
    m_aDoubleClickTimer.Stop();
}

// -----------------------------------------------------------------------
void FormatPaintBrushToolBoxControl::impl_executePaintBrush()
{
    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PersistentCopy" ));
    aArgs[0].Value = makeAny( static_cast<sal_Bool>(m_bPersistentCopy) );
    Dispatch( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormatPaintbrush" ))
        , aArgs );
}

// -----------------------------------------------------------------------
void FormatPaintBrushToolBoxControl::DoubleClick()
{
    m_aDoubleClickTimer.Stop();

    m_bPersistentCopy = true;
    this->impl_executePaintBrush();
}

// -----------------------------------------------------------------------
void FormatPaintBrushToolBoxControl::Click()
{
    m_bPersistentCopy = false;
    m_aDoubleClickTimer.Start();
}

// -----------------------------------------------------------------------
IMPL_LINK(FormatPaintBrushToolBoxControl, WaitDoubleClickHdl, void*, EMPTYARG )
{
    //there was no second click during waiting
    this->impl_executePaintBrush();
    return 0;
}

// -----------------------------------------------------------------------
void FormatPaintBrushToolBoxControl::Select( sal_Bool )
{
}

// -----------------------------------------------------------------------
void FormatPaintBrushToolBoxControl::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                const SfxPoolItem* pState )
{
    if( ( eState & SFX_ITEM_SET ) == 0 )
        m_bPersistentCopy = false;
    SfxToolBoxControl::StateChanged( nSID, eState, pState );
}

//.............................................................................
} //namespace svx
//.............................................................................
