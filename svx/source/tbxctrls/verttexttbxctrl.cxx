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

#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/verttexttbxctrl.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/app.hxx>
#include <svl/eitem.hxx>
#include <vcl/toolbox.hxx>
#include <rtl/ustring.hxx>

SFX_IMPL_TOOLBOX_CONTROL(SvxCTLTextTbxCtrl, SfxBoolItem);
SFX_IMPL_TOOLBOX_CONTROL(SvxVertTextTbxCtrl, SfxBoolItem);

// -----------------------------27.04.01 15:50--------------------------------

SvxCTLTextTbxCtrl::SvxCTLTextTbxCtrl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxVertCTLTextTbxCtrl( nSlotId, nId, rTbx )
{
    SetVert(sal_False);
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CTLFontState" )));
}

SvxVertTextTbxCtrl::SvxVertTextTbxCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxVertCTLTextTbxCtrl( nSlotId, nId, rTbx )
{
    SetVert(sal_True);
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:VerticalTextState" )));
}

/* ---------------------------------------------------------------------------*/
SvxVertCTLTextTbxCtrl::SvxVertCTLTextTbxCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    bCheckVertical(sal_True)
{
}

/* -----------------------------27.04.01 15:53--------------------------------

 ---------------------------------------------------------------------------*/
SvxVertCTLTextTbxCtrl::~SvxVertCTLTextTbxCtrl( )
{
}
/* -----------------------------27.04.01 15:50--------------------------------

 ---------------------------------------------------------------------------*/
void SvxVertCTLTextTbxCtrl::StateChanged(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState )
{
    SvtLanguageOptions aLangOptions;
    sal_Bool bCalc = sal_False;
    sal_Bool bVisible = GetToolBox().IsItemVisible(GetId());
    sal_Bool bEnabled = sal_False;
    if ( nSID == SID_VERTICALTEXT_STATE )
        bEnabled = aLangOptions.IsVerticalTextEnabled();
    else if ( nSID == SID_CTLFONT_STATE )
        bEnabled = aLangOptions.IsCTLFontEnabled();
    else
    {
        SfxToolBoxControl::StateChanged(nSID, eState, pState);
        return;
    }

    if(bEnabled)
    {
        if(!bVisible)
        {
            GetToolBox().ShowItem( GetId(), sal_True );
            bCalc = sal_True;
        }
    }
    else if(bVisible)
    {
        GetToolBox().HideItem( GetId() );
        bCalc = sal_True;
    }
    if(bCalc)
    {
        ToolBox& rTbx = GetToolBox();
        Window* pParent = rTbx.GetParent();
        WindowType nWinType = pParent->GetType();
        if(WINDOW_FLOATINGWINDOW == nWinType)
        {
            Size aSize(rTbx.CalcWindowSizePixel());
            rTbx.SetPosSizePixel( Point(), aSize );
            pParent->SetOutputSizePixel( aSize );
        }
    }
}
/* -----------------------------27.04.01 15:50--------------------------------

 ---------------------------------------------------------------------------*/

