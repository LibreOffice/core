/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "SignatureLineDialogBase.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Reference.hxx>

class SignatureLineDialog : public SignatureLineDialogBase
{
public:
    SignatureLineDialog(weld::Widget* pParent, css::uno::Reference<css::frame::XModel> xModel,
                        bool bEditExisting);

private:
    std::unique_ptr<weld::Entry> m_xEditName;
    std::unique_ptr<weld::Entry> m_xEditTitle;
    std::unique_ptr<weld::Entry> m_xEditEmail;
    std::unique_ptr<weld::TextView> m_xEditInstructions;
    std::unique_ptr<weld::CheckButton> m_xCheckboxCanAddComments;
    std::unique_ptr<weld::CheckButton> m_xCheckboxShowSignDate;

    css::uno::Reference<css::beans::XPropertySet> m_xExistingShapeProperties;
    OUString m_aSignatureLineId;

    virtual void Apply() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
