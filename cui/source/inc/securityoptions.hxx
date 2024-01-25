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
#pragma once

#include <vcl/weld.hxx>

namespace svx
{
    class SecurityOptionsDialog : public weld::GenericDialogController
    {
    private:
        std::unique_ptr<weld::CheckButton> m_xSaveOrSendDocsCB;
        std::unique_ptr<weld::Widget> m_xSaveOrSendDocsImg;
        std::unique_ptr<weld::CheckButton> m_xSignDocsCB;
        std::unique_ptr<weld::Widget> m_xSignDocsImg;
        std::unique_ptr<weld::CheckButton> m_xPrintDocsCB;
        std::unique_ptr<weld::Widget> m_xPrintDocsImg;
        std::unique_ptr<weld::CheckButton> m_xCreatePdfCB;
        std::unique_ptr<weld::Widget> m_xCreatePdfImg;

        std::unique_ptr<weld::CheckButton> m_xRemovePersInfoCB;
        std::unique_ptr<weld::Widget> m_xRemovePersInfoImg;
        std::unique_ptr<weld::CheckButton> m_xRecommPasswdCB;
        std::unique_ptr<weld::Widget> m_xRecommPasswdImg;
        std::unique_ptr<weld::CheckButton> m_xCtrlHyperlinkCB;
        std::unique_ptr<weld::Widget> m_xCtrlHyperlinkImg;
        std::unique_ptr<weld::CheckButton> m_xBlockUntrustedRefererLinksCB;
        std::unique_ptr<weld::Widget> m_xBlockUntrustedRefererLinksImg;
        std::unique_ptr<weld::CheckButton> m_xDisableActiveContentCB;
        std::unique_ptr<weld::Widget> m_xDisableActiveContentImg;

        std::unique_ptr<weld::CheckButton> m_xRedlineinfoCB;
        std::unique_ptr<weld::Widget> m_xRedlineinfoImg;
        std::unique_ptr<weld::CheckButton> m_xDocPropertiesCB;
        std::unique_ptr<weld::Widget> m_xDocPropertiesImg;
        std::unique_ptr<weld::CheckButton> m_xNoteAuthorCB;
        std::unique_ptr<weld::Widget> m_xNoteAuthorImg;
        std::unique_ptr<weld::CheckButton> m_xDocumentVersionCB;
        std::unique_ptr<weld::Widget> m_xDocumentVersionImg;
        std::unique_ptr<weld::CheckButton> m_xPrinterSettingsCB;
        std::unique_ptr<weld::Widget> m_xPrinterSettingsImg;

    public:
        SecurityOptionsDialog(weld::Window* pParent);

        void init();

        bool IsSaveOrSendDocsChecked() const { return m_xSaveOrSendDocsCB->get_active(); }
        bool IsSignDocsChecked() const { return m_xSignDocsCB->get_active(); }
        bool IsPrintDocsChecked() const { return m_xPrintDocsCB->get_active(); }
        bool IsCreatePdfChecked() const { return m_xCreatePdfCB->get_active(); }
        bool IsRemovePersInfoChecked() const { return m_xRemovePersInfoCB->get_active(); }
        bool IsRemoveRedlineInfoChecked() const { return m_xRedlineinfoCB->get_active(); }
        bool IsRemoveDocUserInfoChecked() const { return m_xDocPropertiesCB->get_active(); }
        bool IsRemoveNoteAuthorInfoChecked() const { return m_xNoteAuthorCB->get_active(); }
        bool IsRemoveDocVersionInfoChecked() const { return m_xDocumentVersionCB->get_active(); }
        bool IsKeepPrinterSettingsChecked() const { return m_xPrinterSettingsCB->get_active(); }
        bool IsRecommPasswdChecked() const { return m_xRecommPasswdCB->get_active(); }
        bool IsCtrlHyperlinkChecked() const { return m_xCtrlHyperlinkCB->get_active(); }
        bool IsBlockUntrustedRefererLinksChecked() const { return m_xBlockUntrustedRefererLinksCB->get_active(); }
        bool IsDisableActiveContentChecked() const { return m_xDisableActiveContentCB->get_active(); }

        DECL_LINK(ShowPersonalInfosToggle, weld::Toggleable&, void);

        bool SetSecurityOptions();
        void changeKeepSecurityInfosEnabled();
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
