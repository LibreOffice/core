/*************************************************************************
 *
 *  $RCSfile: certificateviewer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: gt $ $Date: 2004-07-15 06:19:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLSECURITY_CERTIFICATEVIEWER_HXX
#define _XMLSECURITY_CERTIFICATEVIEWER_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/simptabl.hxx>
#include <svtools/svmedit.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;

class CertificateViewer : public TabDialog
{
private:
    friend class CertificateViewerGeneralTP;
    friend class CertificateViewerDetailsTP;
    friend class CertificateViewerCertPathTP;

    TabControl          maTabCtrl;
    OKButton            maOkBtn;
    HelpButton          maHelpBtn;

    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    cssu::Reference< dcss::security::XCertificate > mxCert;
public:
        CertificateViewer( Window* pParent, cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment, cssu::Reference< dcss::security::XCertificate >& rXCert );
    virtual             ~CertificateViewer();
};


class CertificateViewerTP : public TabPage
{
protected:
    CertificateViewer*  mpDlg;
public:
    CertificateViewerTP( Window* _pParent, const ResId& _rResId, CertificateViewer* _pDlg );
    inline void         SetTabDlg( CertificateViewer* pTabDlg );
};

inline void CertificateViewerTP::SetTabDlg( CertificateViewer* _pTabDlg )
{
    mpDlg = _pTabDlg;
}


class CertificateViewerGeneralTP : public CertificateViewerTP
{
private:
    Window              maFrameWin;
    FixedImage          maCertImg;
    FixedInfo           maCertInfoFI;
    FixedLine           maSep1FL;
    FixedInfo           maHintNotTrustedFI;
    FixedLine           maSep2FL;
    FixedInfo           maIssuedToLabelFI;
    FixedInfo           maIssuedToFI;
    FixedInfo           maIssuedByLabelFI;
    FixedInfo           maIssuedByFI;
    FixedInfo           maValidFromLabelFI;
    FixedInfo           maValidFromFI;
    FixedInfo           maValidToLabelFI;
    FixedInfo           maValidToFI;
    FixedImage          maKeyImg;
    FixedInfo           maHintCorrespPrivKeyFI;
public:
                        CertificateViewerGeneralTP( Window* pParent, CertificateViewer* _pDlg );

    virtual void        ActivatePage();
};


class CertificateViewerDetailsTP : public CertificateViewerTP
{
private:
    SvxSimpleTable      maElementsLB;
    MultiLineEdit       maElementML;
    Font                maStdFont;
    Font                maFixedWidthFont;

    DECL_LINK(          ElementSelectHdl, void* );
    void                Clear( void );
    void                InsertElement( const String& _rField, const String& _rValue,
                                        const String& _rDetails, bool _bFixedWidthFont = false );
public:
                        CertificateViewerDetailsTP( Window* pParent, CertificateViewer* _pDlg );
    virtual             ~CertificateViewerDetailsTP();

    virtual void        ActivatePage();
};


class CertificateViewerCertPathTP : public CertificateViewerTP
{
private:
    FixedText           maCertPathFT;
    SvTreeListBox       maCertPathLB;
    FixedText           maCertStatusFT;
    MultiLineEdit       maCertStatusML;

    DECL_LINK(          CertSelectHdl, void* );
    void                Clear( void );
    SvLBoxEntry*        InsertCert( SvLBoxEntry* _pParent, const String& _rName, const String& _rStatus );
public:
                        CertificateViewerCertPathTP( Window* pParent, CertificateViewer* _pDlg );
    virtual             ~CertificateViewerCertPathTP();

    virtual void        ActivatePage();
};


#endif // _XMLSECURITY_CERTIFICATEVIEWER_HXX

