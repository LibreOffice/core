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

#ifndef _SVX_FORMATPAINTBRUSHCTRL_HXX
#define _SVX_FORMATPAINTBRUSHCTRL_HXX

// header for class SfxToolBoxControl
#include <sfx2/tbxctrl.hxx>
// header for class Timer
#include <vcl/timer.hxx>
#include "svx/svxdllapi.h"

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
    FormatPaintBrushToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~FormatPaintBrushToolBoxControl();

    virtual void    DoubleClick();
    virtual void    Click();
    virtual void    Select( sal_Bool bMod1 = sal_False );

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
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
