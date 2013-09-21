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
#include "precompiled_sd.hxx"

#include "TableDesignPanel.hxx"

#include "ViewShellBase.hxx"


namespace sd {
    extern ::Window * createTableDesignPanel (::Window* pParent, ViewShellBase& rBase);
}


namespace sd { namespace sidebar {


TableDesignPanel::TableDesignPanel (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
    : PanelBase(pParentWindow, rViewShellBase)
{
#ifdef DEBUG
    SetText(OUString("sd:TableDesignPanel"));
#endif
}




TableDesignPanel::~TableDesignPanel (void)
{
}




::Window* TableDesignPanel::CreateWrappedControl (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
{
    return createTableDesignPanel(pParentWindow, rViewShellBase);
}




css::ui::LayoutSize TableDesignPanel::GetHeightForWidth (const sal_Int32 nWidth)
{
    //TODO: make the sizes depend on the font size.
    return css::ui::LayoutSize(350,-1, 400);
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
