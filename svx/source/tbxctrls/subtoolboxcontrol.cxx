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
