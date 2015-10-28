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
#include "TextCharacterSpacingPopup.hxx"
#include "TextCharacterSpacingControl.hxx"
#include <boost/bind.hpp>
#include <unotools/viewoptions.hxx>

namespace svx { namespace sidebar {

TextCharacterSpacingPopup::TextCharacterSpacingPopup (
    vcl::Window* pParent,
    const ::std::function<VclPtr<PopupControl>(PopupContainer*)>& rControlCreator)
    : Popup(
        pParent,
        rControlCreator,
        OUString( "Character Spacing"))
{
    SetPopupModeEndHandler(::boost::bind(&TextCharacterSpacingPopup::PopupModeEndCallback, this));
}

TextCharacterSpacingPopup::~TextCharacterSpacingPopup()
{
}

void TextCharacterSpacingPopup::Rearrange (bool bLBAvailable,bool bAvailable, long nKerning)
{
    ProvideContainerAndControl();

    TextCharacterSpacingControl* pControl = dynamic_cast<TextCharacterSpacingControl*>(mxControl.get());
    if (pControl != NULL)
        pControl->Rearrange(bLBAvailable,bAvailable,nKerning);
}

void TextCharacterSpacingPopup::PopupModeEndCallback()
{
    ProvideContainerAndControl();
    TextCharacterSpacingControl* pControl = dynamic_cast<TextCharacterSpacingControl*>(mxControl.get());
    if (pControl == NULL)
        return;

    if( pControl->GetLastCustomState() == SPACING_CLOSE_BY_CUS_EDIT)
    {
        SvtViewOptions aWinOpt( E_WINDOW, SIDEBAR_SPACING_GLOBAL_VALUE );
        css::uno::Sequence < css::beans::NamedValue > aSeq
            { { "Spacing", css::uno::makeAny(OUString::number(pControl->GetLastCustomValue())) } };
        aWinOpt.SetUserData( aSeq );

    }
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
