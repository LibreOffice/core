/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>

#include <string_view>

#include <vcl/weld.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>

class SignatureLineDialogBase : public weld::GenericDialogController
{
public:
    SignatureLineDialogBase(weld::Widget* pParent, css::uno::Reference<css::frame::XModel> xModel,
                            const OUString& rUIFile, const OString& rDialogId);

    virtual short run() override;

protected:
    css::uno::Reference<css::frame::XModel> m_xModel;
    virtual void Apply() = 0;
    static OUString getCDataString(std::u16string_view rString);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
