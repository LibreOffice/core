/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
