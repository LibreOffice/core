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

#include "EmptyPanel.hxx"
#include "EmptyPanel.hrc"
#include "area/AreaPropertyPanel.hrc"
#include "svx/dialogs.hrc"
#include "svx/dialmgr.hxx"


namespace svx { namespace sidebar {

namespace
{
    class SidebarResource : public Resource
    {
    public:
        SidebarResource (const ResId& rResId) : Resource(rResId) {}
        ~SidebarResource (void) { FreeResource(); }
    };
}




EmptyPanel::EmptyPanel (::Window* pParent)
    : Control(pParent, SVX_RES(RID_SIDEBAR_EMPTY_PANEL)),
      maMessageControl(this, SVX_RES(FT_MESSAGE))
{
    maMessageControl.setPosSizePixel(5,5, 250,15);
    maMessageControl.SetStyle(WB_WORDBREAK);// | WB_NOMNEMONICS);
    //maMessageControl.GetStyle()
    //        & ~(WB_NOMULTILINE | WB_PATHELLIPSIS)
    //        | WB_WORDBREAK | WB_NOMNEMONICS);
    FreeResource();

    SetBackground(Wallpaper());

    maMessageControl.Show();
    Show();
}




EmptyPanel::~EmptyPanel (void)
{
}




void EmptyPanel::Resize (void)
{
    const Size aSize (GetSizePixel());
    maMessageControl.SetSizePixel(aSize);
}


} } // end of namespace ::svx::sidebar
