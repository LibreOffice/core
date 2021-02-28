/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/uitest/uiobject.hxx>
#include <ElementsDockingWindow.hxx>

class ElementUIObject : public UIObject
{
private:
    SmElementsControl* mpElementsSelector;
    OUString           maID;

public:

    ElementUIObject(SmElementsControl* pElementSelector,
            const OUString& rID);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

private:
    SmElement* get_element();
};

class ElementSelectorUIObject : public DrawingAreaUIObject
{
private:
    SmElementsControl* mpElementsSelector;

public:

    explicit ElementSelectorUIObject(vcl::Window* pElementSelectorWindow);

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID) override;

    virtual std::set<OUString> get_children() const override;

protected:

    virtual OUString get_name() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
