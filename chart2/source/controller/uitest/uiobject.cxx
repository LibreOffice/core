/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uiobject.hxx"

ChartWindowUIObject::ChartWindowUIObject(VclPtr<chart::ChartWindow> xChartWindow):
    WindowUIObject(xChartWindow),
    mxChartWindow(xChartWindow)
{
}

StringMap ChartWindowUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    return aMap;
}

void ChartWindowUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> ChartWindowUIObject::get_child(const OUString& rID)
{
    return nullptr;
}

std::set<OUString> ChartWindowUIObject::get_children() const
{
    std::set<OUString> aChildren;

    return aChildren;
}

std::unique_ptr<UIObject> ChartWindowUIObject::create(vcl::Window* pWindow)
{
    chart::ChartWindow* pChartWindow = dynamic_cast<chart::ChartWindow*>(pWindow);
    assert(pChartWindow);

    return std::unique_ptr<UIObject>(new ChartWindowUIObject(pChartWindow));
}

OUString ChartWindowUIObject::get_name() const
{
    return OUString("ChartWindowUIObject");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
