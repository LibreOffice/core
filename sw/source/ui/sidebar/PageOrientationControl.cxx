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

#include "precompiled_sw.hxx"

#include "PageOrientationControl.hxx"
#include "PagePropertyPanel.hxx"
#include "PagePropertyPanel.hrc"

#include <swtypes.hxx>

#include <svx/sidebar/ValueSetWithTextControl.hxx>

namespace sw { namespace sidebar {

PageOrientationControl::PageOrientationControl(
    Window* pParent,
    PagePropertyPanel& rPanel,
    const sal_Bool bLandscape )
    : ::svx::sidebar::PopupControl( pParent, SW_RES(RID_POPUP_SWPAGE_ORIENTATION) )
    , mpOrientationValueSet( new ::svx::sidebar::ValueSetWithTextControl( ::svx::sidebar::ValueSetWithTextControl::IMAGE_TEXT, this, SW_RES(VS_ORIENTATION) ) )
    , mbLandscape( bLandscape )
    , mrPagePropPanel(rPanel)
{
    mpOrientationValueSet->SetStyle( mpOrientationValueSet->GetStyle() | WB_3DLOOK | WB_NO_DIRECTSELECT );
    mpOrientationValueSet->SetColor(GetSettings().GetStyleSettings().GetMenuColor());

    // initialize <ValueSetWithText> control
    {
        mpOrientationValueSet->AddItem( SW_RES(IMG_PORTRAIT), 0, SW_RES(STR_PORTRAIT), 0 );
        mpOrientationValueSet->AddItem( SW_RES(IMG_LANDSCAPE), 0, SW_RES(STR_LANDSCAPE), 0 );
    }

    Link aLink = LINK(this, PageOrientationControl,ImplOrientationHdl );
    mpOrientationValueSet->SetSelectHdl(aLink);
    mpOrientationValueSet->SetNoSelection();
    mpOrientationValueSet->StartSelection();
    mpOrientationValueSet->Show();
    mpOrientationValueSet->SelectItem( (mbLandscape == sal_True) ? 2 : 1 );
    mpOrientationValueSet->GrabFocus();
    mpOrientationValueSet->Format();
    mpOrientationValueSet->StartSelection();

    FreeResource();
}


PageOrientationControl::~PageOrientationControl(void)
{
    delete mpOrientationValueSet;
}


IMPL_LINK(PageOrientationControl, ImplOrientationHdl, void *, pControl)
{
    mpOrientationValueSet->SetNoSelection();
    if ( pControl == mpOrientationValueSet )
    {
        const sal_uInt32 iPos = mpOrientationValueSet->GetSelectItemId();
        const bool bChanged = ( ( iPos == 1 ) && mbLandscape ) ||
                              ( ( iPos == 2 ) && !mbLandscape );
        if ( bChanged )
        {
            mbLandscape = !mbLandscape;
            mrPagePropPanel.ExecuteOrientationChange( mbLandscape );
        }
    }

    mrPagePropPanel.ClosePageOrientationPopup();
    return 0;
}


} } // end of namespace sw::sidebar

