/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

