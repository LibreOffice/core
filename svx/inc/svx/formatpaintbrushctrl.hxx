/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
