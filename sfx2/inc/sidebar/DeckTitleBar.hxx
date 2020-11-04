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
#pragma once

#include <sidebar/TitleBar.hxx>
#include <vcl/customweld.hxx>

namespace sfx2::sidebar {

class GripWidget;

class DeckTitleBar final : public TitleBar
{
public:
    DeckTitleBar(const OUString& rsTitle,
                 vcl::Window* pParentWindow,
                 const std::function<void()>& rCloserAction);
    virtual void dispose() override;
    virtual ~DeckTitleBar() override;

    virtual void SetTitle (const OUString& rsTitle) override;
    virtual OUString GetTitle() const override;

    void SetCloserVisible(const bool bIsCloserVisible);
    tools::Rectangle GetDragArea();

    virtual void DataChanged(const DataChangedEvent& rEvent) override;

private:
    virtual void HandleToolBoxItemClick() override;

    DECL_LINK(DrawGripHdl, weld::DrawingArea::draw_args, void);

    std::unique_ptr<GripWidget> mxGripWidget;
    std::unique_ptr<weld::CustomWeld> mxGripWeld;
    std::unique_ptr<weld::Label> mxLabel;

    const std::function<void()> maCloserAction;
    BitmapEx maGrip;
    bool mbIsCloserVisible;
};

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
