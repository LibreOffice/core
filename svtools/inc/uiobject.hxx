/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_INC_UIOBJECT_HXX
#define INCLUDED_SVTOOLS_INC_UIOBJECT_HXX

#include <memory>
#include <vcl/uitest/uiobject.hxx>
#include <svtools/valueset.hxx>

class ValueSet;

class ValueSetUIObject final : public WindowUIObject
{
    ValueSet* mpSet;

public:
    ValueSetUIObject(const VclPtr<vcl::Window>& xSetWin, ValueSet* pSet);

    virtual void execute(const OUString& rAction, const StringMap& rParameters) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

    virtual StringMap get_state() override;

private:
    OUString get_name() const override;
};

#endif // INCLUDED_SVX_INC_UIOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */