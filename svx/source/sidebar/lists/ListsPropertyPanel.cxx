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
#include "ListsPropertyPanel.hxx"
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace css;
using namespace css::uno;

namespace svx::sidebar
{
std::unique_ptr<PanelLayout>
ListsPropertyPanel::Create(weld::Widget* pParent,
                           const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            u"no parent Window given to ListsPropertyPanel::Create"_ustr, nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException(u"no XFrame given to ListsPropertyPanel::Create"_ustr,
                                             nullptr, 1);

    return std::make_unique<ListsPropertyPanel>(pParent, rxFrame);
}

ListsPropertyPanel::ListsPropertyPanel(weld::Widget* pParent,
                                       const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, u"ListsPropertyPanel"_ustr, u"svx/ui/sidebarlists.ui"_ustr)
    , mxTBxNumBullet(m_xBuilder->weld_toolbar(u"numberbullet"_ustr))
    , mxNumBulletDispatcher(new ToolbarUnoDispatcher(*mxTBxNumBullet, *m_xBuilder, rxFrame))
    , mxTBxOutline(m_xBuilder->weld_toolbar(u"outline"_ustr))
    , mxOutlineDispatcher(new ToolbarUnoDispatcher(*mxTBxOutline, *m_xBuilder, rxFrame))
{
}

ListsPropertyPanel::~ListsPropertyPanel()
{
    mxOutlineDispatcher.reset();
    mxTBxOutline.reset();
    mxNumBulletDispatcher.reset();
    mxTBxNumBullet.reset();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
