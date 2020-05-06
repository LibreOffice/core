/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_UITEST_FORMATTEDFIELDUIOBJECT_HXX
#define INCLUDED_VCL_UITEST_FORMATTEDFIELDUIOBJECT_HXX

#include <vcl/uitest/uiobject.hxx>

class FormattedField;

class UITEST_DLLPUBLIC FormattedFieldUIObject : public SpinFieldUIObject
{
    VclPtr<FormattedField> mxFormattedField;

public:
    FormattedFieldUIObject(const VclPtr<FormattedField>& xEdit);
    virtual ~FormattedFieldUIObject() override;

    virtual void execute(const OUString& rAction, const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:
    virtual OUString get_name() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
