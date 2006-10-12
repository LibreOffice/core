/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formatpaintbrushctrl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 11:42:32 $
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

#ifndef _SVX_FORMATPAINTBRUSHCTRL_HXX
#define _SVX_FORMATPAINTBRUSHCTRL_HXX

// header for class SfxToolBoxControl
#ifndef _SFXTBXCTRL_HXX
#include <sfx2/tbxctrl.hxx>
#endif
// header for class Timer
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//.............................................................................
namespace svx
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class SVX_DLLPUBLIC FormatPaintBrushToolBoxControl : public SfxToolBoxControl
{
    using SfxToolBoxControl::Select;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    FormatPaintBrushToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~FormatPaintBrushToolBoxControl();

    virtual void    DoubleClick();
    virtual void    Click();
    virtual void    Select( BOOL bMod1 = FALSE );

    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
private:
    DECL_LINK( WaitDoubleClickHdl, void* );
    void impl_executePaintBrush();

private:
    bool    m_bPersistentCopy;//indicates that the content of the format clipboard should not be erased after paste
    Timer   m_aDoubleClickTimer;
};

//.............................................................................
} //namespace svx
//.............................................................................
#endif
