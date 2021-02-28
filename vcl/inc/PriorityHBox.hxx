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

#include <vcl/layout.hxx>
#include "IPrioritable.hxx"

#include <vector>

/*
 * PriorityHBox is a VclHBox which hides its own children if there is no sufficient space.
 * Hiding order can be modified using child's priorities. If a control have default
 * priority assigned (VCL_PRIORITY_DEFAULT), it is always shown.
 */

class PriorityHBox : public VclHBox
{
protected:
    bool m_bInitialized;

    std::vector<vcl::IPrioritable*> m_aSortedChildren;

    virtual int GetHiddenCount() const;

    virtual void GetChildrenWithPriorities();

public:
    explicit PriorityHBox(vcl::Window* pParent);

    virtual ~PriorityHBox() override;

    void Initialize();

    void SetSizeFromParent();

    virtual Size calculateRequisition() const override;

    virtual void Resize() override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
