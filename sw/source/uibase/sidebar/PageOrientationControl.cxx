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

#include "PageOrientationControl.hxx"
#include "PagePropertyPanel.hxx"
#include "PagePropertyPanel.hrc"

#include <swtypes.hxx>

#include <svx/sidebar/ValueSetWithTextControl.hxx>
#include <vcl/settings.hxx>

namespace sw { namespace sidebar {

PageOrientationControl::PageOrientationControl(
    vcl::Window* pParent,
    PagePropertyPanel& rPanel,
    const bool bLandscape )
    : svx::sidebar::PopupControl( pParent, SW_RES(RID_POPUP_SWPAGE_ORIENTATION) )
    , mpOrientationValueSet( VclPtr<svx::sidebar::ValueSetWithTextControl>::Create( svx::sidebar::ValueSetWithTextControl::IMAGE_TEXT, this, SW_RES(VS_ORIENTATION) ) )
    , mbLandscape( bLandscape )
    , mrPagePropPanel(rPanel)
{
    mpOrientationValueSet->SetStyle( mpOrientationValueSet->GetStyle() | WB_3DLOOK | WB_NO_DIRECTSELECT );
    mpOrientationValueSet->SetColor(GetSettings().GetStyleSettings().GetMenuColor());

    // initialize <ValueSetWithText> control
    {
        mpOrientationValueSet->AddItem(Image(SW_RES(IMG_PORTRAIT)), nullptr,
                SW_RES(STR_PORTRAIT), nullptr);
        mpOrientationValueSet->AddItem(Image(SW_RES(IMG_LANDSCAPE)), nullptr,
                    SW_RES(STR_LANDSCAPE), nullptr );
    }

    mpOrientationValueSet->SetSelectHdl(LINK(this, PageOrientationControl,ImplOrientationHdl ));
    mpOrientationValueSet->SetNoSelection();
    mpOrientationValueSet->StartSelection();
    mpOrientationValueSet->Show();
    mpOrientationValueSet->SelectItem( mbLandscape ? 2 : 1 );
    mpOrientationValueSet->GrabFocus();
    mpOrientationValueSet->SetFormat();
    mpOrientationValueSet->Invalidate();
    mpOrientationValueSet->StartSelection();

    FreeResource();
}

PageOrientationControl::~PageOrientationControl()
{
    disposeOnce();
}

void PageOrientationControl::dispose()
{
    mpOrientationValueSet.disposeAndClear();
    svx::sidebar::PopupControl::dispose();
}

IMPL_LINK_TYPED(PageOrientationControl, ImplOrientationHdl, ValueSet*, pControl, void)
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
}

} } // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
