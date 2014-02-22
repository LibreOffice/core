/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include <svx/sidebar/ColorPopup.hxx>
#include <svx/sidebar/ColorControl.hxx>


namespace svx { namespace sidebar {

ColorPopup::ColorPopup (
    Window* pParent,
    const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator)
    : Popup(
        pParent,
        rControlCreator,
        OUString("Color"))
{
}




ColorPopup::~ColorPopup (void)
{
}




void ColorPopup::SetCurrentColor (
    const Color aCurrentColor,
    const bool bIsColorAvailable)
{
    ProvideContainerAndControl();

    ColorControl* pColorControl = dynamic_cast<ColorControl*>(mpControl.get());
    if (pColorControl != NULL)
        pColorControl->SetCurColorSelect(aCurrentColor, bIsColorAvailable);
}


} } 
