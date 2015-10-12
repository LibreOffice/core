/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_SECURITYOPTIONS_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_SECURITYOPTIONS_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>


class SvtSecurityOptions;


namespace svx
{

    class SecurityOptionsDialog : public ModalDialog
    {
    private:
        VclPtr<CheckBox> m_pSaveOrSendDocsCB;
        VclPtr<CheckBox> m_pSignDocsCB;
        VclPtr<CheckBox> m_pPrintDocsCB;
        VclPtr<CheckBox> m_pCreatePdfCB;

        VclPtr<CheckBox> m_pRemovePersInfoCB;
        VclPtr<CheckBox> m_pRecommPasswdCB;
        VclPtr<CheckBox> m_pCtrlHyperlinkCB;
        VclPtr<CheckBox> m_pBlockUntrustedRefererLinksCB;

    public:
        SecurityOptionsDialog( vcl::Window* pParent, SvtSecurityOptions* pOptions );
        virtual ~SecurityOptionsDialog();
        virtual void dispose() override;

        bool IsSaveOrSendDocsChecked() const { return m_pSaveOrSendDocsCB->IsChecked(); }
        bool IsSignDocsChecked() const { return m_pSignDocsCB->IsChecked(); }
        bool IsPrintDocsChecked() const { return m_pPrintDocsCB->IsChecked(); }
        bool IsCreatePdfChecked() const { return m_pCreatePdfCB->IsChecked(); }
        bool IsRemovePersInfoChecked() const { return m_pRemovePersInfoCB->IsChecked(); }
        bool IsRecommPasswdChecked() const { return m_pRecommPasswdCB->IsChecked(); }
        bool IsCtrlHyperlinkChecked() const { return m_pCtrlHyperlinkCB->IsChecked(); }
        bool IsBlockUntrustedRefererLinksChecked() const { return m_pBlockUntrustedRefererLinksCB->IsChecked(); }
    };


}


#endif // INCLUDED_CUI_SOURCE_OPTIONS_SECURITYOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
