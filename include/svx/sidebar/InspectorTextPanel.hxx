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

#include <sfx2/sidebar/PanelLayout.hxx>
#include <svx/svxdllapi.h>

namespace svx::sidebar
{
struct TreeNode
{
    OUString sNodeName;
    css::uno::Any aValue;
    bool isGrey;
    enum
    {
        Category,
        ComplexProperty,
        SimpleProperty
    } NodeType;
    std::vector<TreeNode> children;
    TreeNode()
        : isGrey(false)
        , NodeType(SimpleProperty)
    {
    }
};
class SVX_DLLPUBLIC InspectorTextPanel : public PanelLayout
{
public:
    virtual ~InspectorTextPanel() override;

    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent);

    InspectorTextPanel(weld::Widget* pParent);

    void updateEntries(const std::vector<TreeNode>& rStore);

private:
    std::unique_ptr<weld::TreeView> mpListBoxStyles;
};

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
