/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_INC_UIOBJECT_HXX
#define INCLUDED_SVX_INC_UIOBJECT_HXX

#include <memory>
#include <vcl/uitest/uiobject.hxx>

class SvxShowCharSet;

class SvxShowCharSetUIObject : public WindowUIObject
{
    SvxShowCharSet* mpCharSet;

public:
    SvxShowCharSetUIObject(const VclPtr<vcl::Window>& xCharSetWin, SvxShowCharSet* pCharSet);

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    OUString get_name() const override;
};

#endif // INCLUDED_SVX_INC_UIOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
