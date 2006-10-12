/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formatpaintbrushctrl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 13:20:40 $
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

#include "formatpaintbrushctrl.hxx"

// header for class SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif

// header for define SFX_APP
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif

// header for class ToolBox
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

//.............................................................................
namespace svx
{
//.............................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

SFX_IMPL_TOOLBOX_CONTROL( FormatPaintBrushToolBoxControl, SfxBoolItem );

FormatPaintBrushToolBoxControl::FormatPaintBrushToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    : SfxToolBoxControl( nSlotId, nId, rTbx )
    , m_bPersistentCopy(false)
    , m_aDoubleClickTimer()
{
    ULONG nDblClkTime = rTbx.GetSettings().GetMouseSettings().GetDoubleClickTime();

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
void FormatPaintBrushToolBoxControl::Select( BOOL )
{
}

// -----------------------------------------------------------------------
void FormatPaintBrushToolBoxControl::StateChanged( USHORT nSID, SfxItemState eState,
                const SfxPoolItem* pState )
{
    if( ( eState & SFX_ITEM_SET ) == 0 )
        m_bPersistentCopy = false;
    SfxToolBoxControl::StateChanged( nSID, eState, pState );
}

//.............................................................................
} //namespace svx
//.............................................................................
