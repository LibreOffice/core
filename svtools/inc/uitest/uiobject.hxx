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

class ValueSet;

class ValueSetUIObject final : public WindowUIObject
{
private:
    VclPtr<ValueSet> mxValueSet;

public:
    ValueSetUIObject(const VclPtr<ValueSet>& xValueSet);
    virtual ~ValueSetUIObject() override;

    virtual void execute(const OUString& rAction, const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

    virtual OUString get_action(VclEventId nEvent) const override;

private:
    virtual OUString get_name() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
