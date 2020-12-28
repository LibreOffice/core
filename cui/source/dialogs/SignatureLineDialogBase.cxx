/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SignatureLineDialogBase.hxx>

#include <utility>

#include <vcl/weld.hxx>

using namespace css;
using namespace css::uno;
using namespace css::frame;

SignatureLineDialogBase::SignatureLineDialogBase(weld::Widget* pParent, Reference<XModel> xModel,
                                                 const OUString& rUIFile, const OString& rDialogId)
    : GenericDialogController(pParent, rUIFile, rDialogId)
    , m_xModel(std::move(xModel))
{
}

short SignatureLineDialogBase::run()
{
    short nRet = GenericDialogController::run();
    if (nRet == RET_OK)
        Apply();
    return nRet;
}

OUString SignatureLineDialogBase::getCDataString(std::u16string_view rString)
{
    return OUString::Concat("<![CDATA[") + rString + "]]>";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
