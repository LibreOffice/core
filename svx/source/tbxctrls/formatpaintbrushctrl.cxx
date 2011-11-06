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
