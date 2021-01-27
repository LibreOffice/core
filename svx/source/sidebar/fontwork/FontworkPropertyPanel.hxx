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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_AREA_FONTWORKPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_AREA_FONTWORKPROPERTYPANEL_HXX

#include <svx/sidebar/PanelLayout.hxx>
#include <com/sun/star/ui/XUIElement.hpp>
#include <sfx2/sidebar/SidebarToolBox.hxx>

namespace svx
{
namespace sidebar
{
class FontworkPropertyPanel : public PanelLayout
{
public:
    static VclPtr<vcl::Window> Create(vcl::Window* pParent,
                                      const css::uno::Reference<css::frame::XFrame>& rxFrame);

    // constructor/destructor
    FontworkPropertyPanel(vcl::Window* pParent,
                          const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual ~FontworkPropertyPanel() override;
    virtual void dispose() override;

private:
    VclPtr<sfx2::sidebar::SidebarToolBox> m_xToolbox;
};
}
} // end of namespace svx::sidebar

#endif // INCLUDED_SVX_SOURCE_SIDEBAR_AREA_FONTWORKPROPERTYPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
