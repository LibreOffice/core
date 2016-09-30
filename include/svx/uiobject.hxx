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

#include <vcl/uitest/uiobject.hxx>

class SdrObject;

class SVX_DLLPUBLIC SdrUIObject : public UIObject
{
public:

    virtual ~SdrUIObject() override;

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual OUString get_type() const override;

    virtual SdrObject* get_object() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
