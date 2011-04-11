/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
        FixedLine           m_aWarningsFL;
        FixedInfo           m_aWarningsFI;
        ReadOnlyImage       m_aSaveOrSendDocsFI;
        CheckBox            m_aSaveOrSendDocsCB;
        ReadOnlyImage       m_aSignDocsFI;
        CheckBox            m_aSignDocsCB;
        ReadOnlyImage       m_aPrintDocsFI;
        CheckBox            m_aPrintDocsCB;
        ReadOnlyImage       m_aCreatePdfFI;
        CheckBox            m_aCreatePdfCB;

        FixedLine           m_aOptionsFL;
        ReadOnlyImage       m_aRemovePersInfoFI;
        CheckBox            m_aRemovePersInfoCB;
        ReadOnlyImage       m_aRecommPasswdFI;
        CheckBox            m_aRecommPasswdCB;
        ReadOnlyImage       m_aCtrlHyperlinkFI;
        CheckBox            m_aCtrlHyperlinkCB;

        FixedLine           m_aButtonsFL;
        OKButton            m_aOKBtn;
        CancelButton        m_aCancelBtn;
        HelpButton          m_aHelpBtn;

    public:
        SecurityOptionsDialog( Window* pParent, SvtSecurityOptions* pOptions );
        ~SecurityOptionsDialog();

        inline bool         IsSaveOrSendDocsChecked() const { return m_aSaveOrSendDocsCB.IsChecked() != sal_False; }
        inline bool         IsSignDocsChecked() const { return m_aSignDocsCB.IsChecked() != sal_False; }
        inline bool         IsPrintDocsChecked() const { return m_aPrintDocsCB.IsChecked() != sal_False; }
        inline bool         IsCreatePdfChecked() const { return m_aCreatePdfCB.IsChecked() != sal_False; }
        inline bool         IsRemovePersInfoChecked() const { return m_aRemovePersInfoCB.IsChecked() != sal_False; }
        inline bool         IsRecommPasswdChecked() const { return m_aRecommPasswdCB.IsChecked() != sal_False; }
        inline bool         IsCtrlHyperlinkChecked() const { return m_aCtrlHyperlinkCB.IsChecked() != sal_False; }
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // #ifndef _SVX_SECURITYOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
