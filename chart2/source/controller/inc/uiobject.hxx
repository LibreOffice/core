/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <vcl/uitest/uiobject.hxx>

#include "ChartWindow.hxx"

class ChartUIObject final : public UIObject
{
public:

    ChartUIObject(const VclPtr<chart::ChartWindow>& xChartWindow,
            OUString aCID);

    StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID) override;

    virtual std::set<OUString> get_children() const override;

    virtual OUString get_type() const override;

    virtual bool equals(const UIObject& rOther) const override;

private:

    OUString maCID;
    VclPtr<chart::ChartWindow> mxChartWindow;
    std::vector<std::unique_ptr<OUString>> maCommands;

    DECL_LINK(PostCommand, void*, void);
};

class ChartWindowUIObject final : public WindowUIObject
{
    VclPtr<chart::ChartWindow> mxChartWindow;

public:

    ChartWindowUIObject(const VclPtr<chart::ChartWindow>& xChartWindow);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID) override;

    virtual std::set<OUString> get_children() const override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
