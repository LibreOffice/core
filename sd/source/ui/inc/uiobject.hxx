/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <vcl/uitest/uiobject.hxx>
#include "Window.hxx"

class ImpressWindowUIObject : public WindowUIObject
{
public:
    ImpressWindowUIObject(const VclPtr<sd::Window>& xWindow);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction, const StringMap& rParameters) override;

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID) override;

    virtual std::set<OUString> get_children() const override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:
    virtual OUString get_name() const override;

private:
    VclPtr<sd::Window> mxWindow;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
