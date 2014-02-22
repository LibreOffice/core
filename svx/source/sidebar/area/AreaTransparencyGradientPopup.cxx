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
#include "AreaTransparencyGradientPopup.hxx"
#include "AreaTransparencyGradientControl.hxx"


namespace svx { namespace sidebar {

AreaTransparencyGradientPopup::AreaTransparencyGradientPopup (
    Window* pParent,
    const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator)
    : Popup(
        pParent,
        rControlCreator,
        OUString( "Transparency and Gradient"))
{
}




AreaTransparencyGradientPopup::~AreaTransparencyGradientPopup (void)
{
}




void AreaTransparencyGradientPopup::Rearrange (XFillFloatTransparenceItem* pItem)
{
    ProvideContainerAndControl();

    AreaTransparencyGradientControl* pControl = dynamic_cast<AreaTransparencyGradientControl*>(mpControl.get());
    if (pControl != NULL)
        pControl->Rearrange(pItem);
}


} } 


