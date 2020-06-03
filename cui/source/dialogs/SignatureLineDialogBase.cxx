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

#include <config_folders.h>
#include <vcl/weld.hxx>
#include <sal/log.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/stream.hxx>

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

OUString SignatureLineDialogBase::getCDataString(const OUString& rString)
{
    return "<![CDATA[" + rString + "]]>";
}

OUString SignatureLineDialogBase::getSignatureImage()
{
    OUString aPath("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/filter/signature-line.svg");
    rtl::Bootstrap::expandMacros(aPath);
    SvFileStream aStream(aPath, StreamMode::READ);
    if (aStream.GetError() != ERRCODE_NONE)
    {
        SAL_WARN("cui.dialogs", "failed to open signature-line.svg");
    }

    OString const svg = read_uInt8s_ToOString(aStream, aStream.remainingSize());
    return OUString::fromUtf8(svg);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
