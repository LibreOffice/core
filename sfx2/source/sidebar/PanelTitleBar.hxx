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
#ifndef SFX_SIDEBAR_PANEL_TITLE_BAR_HXX
#define SFX_SIDEBAR_PANEL_TITLE_BAR_HXX

#include "TitleBar.hxx"

#include <boost/function.hpp>


namespace sfx2 { namespace sidebar {

class Panel;

class PanelTitleBar
    : public TitleBar
{
public:
    PanelTitleBar (
        const ::rtl::OUString& rsTitle,
        Window* pParentWindow,
        Panel* pPanel );
    virtual ~PanelTitleBar (void);

    void SetMenuAction ( const ::boost::function<void(void)>& rMenuAction );

    virtual void DataChanged (const DataChangedEvent& rEvent);
    virtual void MouseButtonDown (const MouseEvent& rMouseEvent);
    virtual void MouseButtonUp (const MouseEvent& rMouseEvent);

protected:
    virtual Rectangle GetTitleArea (const Rectangle& rTitleBarBox);
    virtual void PaintDecoration (const Rectangle& rTitleBarBox);
    virtual sidebar::Paint GetBackgroundPaint (void);
    virtual Color GetTextColor (void);
    virtual void HandleToolBoxItemClick (const sal_uInt16 nItemIndex);

private:
    bool mbIsLeftButtonDown;
    Panel* mpPanel;
    const sal_uInt16 mnMenuItemIndex;
    ::boost::function<void(void)> maMenuAction;
};


} } // end of namespace sfx2::sidebar

#endif
