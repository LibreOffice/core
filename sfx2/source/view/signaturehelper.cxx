/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "view/signaturehelper.hxx"

#include <sfx2/infobar.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfx.hrc>
#include <vcl/button.hxx>
#include <vcl/vclptr.hxx>
#include <tools/link.hxx>


SignatureHelper::SignatureHelper(SfxViewFrame* rViewFrame)
{
    mrViewFrame = rViewFrame;
}

void SignatureHelper::updateInfoBarState(SignatureState nSignatureState)
{
    InfoBarType aInfoBarType(InfoBarType::Info);
    OUString sMessage("");

    switch (nSignatureState)
    {
    case SignatureState::BROKEN:
        sMessage = SfxResId(STR_SIGNATURE_BROKEN);
        aInfoBarType = InfoBarType::Danger;
        break;
    case SignatureState::NOTVALIDATED:
        sMessage = SfxResId(STR_SIGNATURE_NOTVALIDATED);
        aInfoBarType = InfoBarType::Warning;
        break;
    case SignatureState::PARTIAL_OK:
        sMessage = SfxResId(STR_SIGNATURE_PARTIAL_OK);
        aInfoBarType = InfoBarType::Warning;
        break;
    case SignatureState::OK:
        sMessage = SfxResId(STR_SIGNATURE_OK);
        aInfoBarType = InfoBarType::Info;
        break;
    default:
        break;
    }

    mrViewFrame->RemoveInfoBar("signature");
    if (!sMessage.isEmpty())
    {
        auto pInfoBar = mrViewFrame->AppendInfoBar("signature", sMessage, aInfoBarType);
        if (pInfoBar == nullptr)
            return;
        VclPtrInstance<PushButton> xBtn(&(mrViewFrame->GetWindow()));
        xBtn->SetText(SfxResId(STR_SIGNATURE_SHOW));
        xBtn->SetSizePixel(xBtn->GetOptimalSize());
        xBtn->SetCommandHandler(".uno:Signature", false); // false = No status listener, else we get in an infinite loop
        pInfoBar->addButton(xBtn);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

