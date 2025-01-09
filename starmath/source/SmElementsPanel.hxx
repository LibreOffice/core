/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <sfx2/bindings.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/customweld.hxx>
#include <vcl/EnumContext.hxx>

#include <ElementsDockingWindow.hxx>

#include <memory>

namespace sm::sidebar
{
class SmElementsPanel : public PanelLayout
{
public:
    static std::unique_ptr<PanelLayout> Create(weld::Widget& rParent, const SfxBindings& rBindings,
                                               sal_uInt64 nWindowId);

    SmElementsPanel(weld::Widget& rParent, const SfxBindings& rBindings, sal_uInt64 nWindowId);
    ~SmElementsPanel();

private:
    DECL_LINK(CategorySelectedHandle, weld::ComboBox&, void);
    DECL_LINK(ElementClickHandler, OUString, void);

    SmViewShell* GetView() const;

    const SfxBindings& mrBindings;

    std::unique_ptr<weld::ComboBox> mxCategoryList;
    std::unique_ptr<SmElementsControl> mxElementsControl;
};

} // end of namespace sm::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
