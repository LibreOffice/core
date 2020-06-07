/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_UIOBJECT_HXX
#define INCLUDED_SVX_UIOBJECT_HXX

#include <svx/svxdllapi.h>

#include <svx/SvxColorValueSet.hxx>

#include <vcl/uitest/uiobject.hxx>

class SdrObject;
class SvxColorValueSet;

class SVXCORE_DLLPUBLIC SdrUIObject : public UIObject
{
public:

    virtual ~SdrUIObject() override;

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual OUString get_type() const override;

    virtual SdrObject* get_object() = 0;
};

class SvxColorValueSetUIObject final : public WindowUIObject
{
    SvxColorValueSet* mpColorSet;

public:

    SvxColorValueSetUIObject(vcl::Window*  xColorSetWin, SvxColorValueSet* pColorSet);

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

    virtual StringMap get_state() override;

private:

    OUString get_name() const override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
