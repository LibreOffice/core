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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_DECKTITLEBAR_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_DECKTITLEBAR_HXX

#include <sfx2/sidebar/TitleBar.hxx>

namespace sfx2 { namespace sidebar {

class DeckTitleBar : public TitleBar
{
public:
    DeckTitleBar(const OUString& rsTitle,
                 vcl::Window* pParentWindow,
                 const std::function<void()>& rCloserAction);

    void SetCloserVisible(const bool bIsCloserVisible);

    virtual void DataChanged(const DataChangedEvent& rEvent) override;

protected:
    virtual Rectangle GetTitleArea(const Rectangle& rTitleBarBox) override;
    virtual void PaintDecoration(vcl::RenderContext& rRenderContext, const Rectangle& rTitleBarBox) override;
    virtual sidebar::Paint GetBackgroundPaint() override;
    virtual void HandleToolBoxItemClick(const sal_uInt16 nItemIndex) override;
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

private:
    const sal_uInt16 mnCloserItemIndex;
    const std::function<void()> maCloserAction;
    bool mbIsCloserVisible;
};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
