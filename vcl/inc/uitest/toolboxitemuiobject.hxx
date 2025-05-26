/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/toolbox.hxx>
#include <vcl/uitest/uiobject.hxx>

class ToolBoxItemUIObject final : public UIObject
{
private:
    VclPtr<ToolBox> m_pToolBox;
    ToolBoxItemId m_nId;

public:
    ToolBoxItemUIObject(const VclPtr<ToolBox>& pToolBox, ToolBoxItemId nId);
    virtual ~ToolBoxItemUIObject() override;

    virtual StringMap get_state() override;

    virtual bool equals(const UIObject& rOther) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
