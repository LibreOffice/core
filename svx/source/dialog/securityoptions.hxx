/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: securityoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 16:40:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

        inline bool         IsSaveOrSendDocsChecked() const { return m_aSaveOrSendDocsCB.IsChecked() != FALSE; }
        inline bool         IsSignDocsChecked() const { return m_aSignDocsCB.IsChecked() != FALSE; }
        inline bool         IsPrintDocsChecked() const { return m_aPrintDocsCB.IsChecked() != FALSE; }
        inline bool         IsCreatePdfChecked() const { return m_aCreatePdfCB.IsChecked() != FALSE; }
        inline bool         IsRemovePersInfoChecked() const { return m_aRemovePersInfoCB.IsChecked() != FALSE; }
        inline bool         IsRecommPasswdChecked() const { return m_aRecommPasswdCB.IsChecked() != FALSE; }
        inline bool         IsCtrlHyperlinkChecked() const { return m_aCtrlHyperlinkCB.IsChecked() != FALSE; }
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // #ifndef _SVX_SECURITYOPTIONS_HXX

