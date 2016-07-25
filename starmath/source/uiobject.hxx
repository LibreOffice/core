/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_STARMATH_SOURCE_UIOBJECT_HXX
#define INCLUDED_STARMATH_SOURCE_UIOBJECT_HXX

#include <vcl/uitest/uiobject.hxx>

class SmElementsControl;
class SmElement;

class ElementUIObject : public UIObject
{
private:
    VclPtr<SmElementsControl> mxElementsSelector;
    OUString maID;

public:

    ElementUIObject(VclPtr<SmElementsControl> xElementSelector,
            const OUString& rID);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

private:
    SmElement* get_element();
};

class ElementSelectorUIObject : public WindowUIObject
{
private:
    VclPtr<SmElementsControl> mxElementsSelector;

public:

    explicit ElementSelectorUIObject(VclPtr<SmElementsControl> xElementSelector);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID) override;

    virtual std::set<OUString> get_children() const override;

protected:

    virtual OUString get_name() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
