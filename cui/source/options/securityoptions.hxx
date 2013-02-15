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
#ifndef _SVX_SECURITYOPTIONS_HXX
#define _SVX_SECURITYOPTIONS_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>

#include "readonlyimage.hxx"

class SvtSecurityOptions;

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= class SecurityOptionsDialog
    //====================================================================
    class SecurityOptionsDialog : public ModalDialog
    {
    private:
        CheckBox* m_pSaveOrSendDocsCB;
        CheckBox* m_pSignDocsCB;
        CheckBox* m_pPrintDocsCB;
        CheckBox* m_pCreatePdfCB;

        CheckBox* m_pRemovePersInfoCB;
        CheckBox* m_pRecommPasswdCB;
        CheckBox* m_pCtrlHyperlinkCB;


    public:
        SecurityOptionsDialog( Window* pParent, SvtSecurityOptions* pOptions );
        ~SecurityOptionsDialog();

        bool IsSaveOrSendDocsChecked() const { return m_pSaveOrSendDocsCB->IsChecked() != sal_False; }
        bool IsSignDocsChecked() const { return m_pSignDocsCB->IsChecked() != sal_False; }
        bool IsPrintDocsChecked() const { return m_pPrintDocsCB->IsChecked() != sal_False; }
        bool IsCreatePdfChecked() const { return m_pCreatePdfCB->IsChecked() != sal_False; }
        bool IsRemovePersInfoChecked() const { return m_pRemovePersInfoCB->IsChecked() != sal_False; }
        bool IsRecommPasswdChecked() const { return m_pRecommPasswdCB->IsChecked() != sal_False; }
        bool IsCtrlHyperlinkChecked() const { return m_pCtrlHyperlinkCB->IsChecked() != sal_False; }
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // #ifndef _SVX_SECURITYOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
