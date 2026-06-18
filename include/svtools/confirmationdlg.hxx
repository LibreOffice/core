/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/configuration.hxx>
#include <svtools/svtdllapi.h>
#include <vcl/abstdlg.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/weld/Builder.hxx>
class SVT_DLLPUBLIC ConfirmationDlg
{
public:
    template <class ConfigurationLocalizedProperty>
    static bool Query(weld::Window* pParent, const OUString& rTitle, const OUString& rText,
                      const OUString& rQuestion, const OUString& rYesLabel = u""_ustr,
                      const OUString& rNoLabel = u""_ustr)
    {
        const bool bHideShowAgain = ConfigurationLocalizedProperty::isReadOnly();

        if (ConfigurationLocalizedProperty::get())
        {
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            auto pDlg = pFact->CreateQueryDialog(pParent, rTitle, rText, rQuestion, bHideShowAgain);
            if (!rYesLabel.isEmpty())
                pDlg->SetYesLabel(rYesLabel);
            if (!rNoLabel.isEmpty())
                pDlg->SetNoLabel(rNoLabel);

            VclResponseType nResult = pDlg->Execute();

            if (pDlg->DontShowAgain() && !bHideShowAgain)
            {
                std::shared_ptr<comphelper::ConfigurationChanges> aBatch(
                    comphelper::ConfigurationChanges::create());
                ConfigurationLocalizedProperty::set(false, aBatch);
                aBatch->commit();
            }

            pDlg->disposeOnce();
            return nResult == RET_YES;
        }
        else
            return true; // don't ask and assume yes
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
