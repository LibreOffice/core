/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/uitest/uiobject.hxx>

class WindowUIObject : public UIObject
{
    VclPtr<vcl::Window> mxWindow;

public:

    WindowUIObject(VclPtr<vcl::Window> xWindow);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual UIObjectType getType() const override;

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
