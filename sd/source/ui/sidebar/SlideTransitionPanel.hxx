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
#ifndef SD_SIDEBAR_PANELS_SLIDE_TRANSITION_PANEL_HXX
#define SD_SIDEBAR_PANELS_SLIDE_TRANSITION_PANEL_HXX

#include "PanelBase.hxx"
#include <sfx2/sidebar/ControlFactory.hxx>

namespace sd { namespace sidebar {

class SlideTransitionPanel
    : public PanelBase
{
public:
    SlideTransitionPanel (
        ::Window* pParentWindow,
        ViewShellBase& rViewShellBase,
        const cssu::Reference<css::frame::XFrame>& rxFrame );
    virtual ~SlideTransitionPanel (void);

    // ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth);

protected:
    virtual ::Window* CreateWrappedControl (
        ::Window* pParentWindow,
        ViewShellBase& rViewShellBase);

private:
    cssu::Reference<css::frame::XFrame> mxFrame;
};

} } // end of namespace sd::sidebar


#endif
