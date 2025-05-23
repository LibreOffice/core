/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/uitest/uiobject.hxx>

class ToolBox;

class ToolBoxUIObject final : public WindowUIObject
{
private:
    VclPtr<ToolBox> mxToolBox;

public:
    ToolBoxUIObject(const VclPtr<ToolBox>& mxToolBox);
    virtual ~ToolBoxUIObject() override;

    virtual void execute(const OUString& rAction, const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

    virtual OUString get_action(VclEventId nEvent) const override;

private:
    virtual OUString get_name() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
