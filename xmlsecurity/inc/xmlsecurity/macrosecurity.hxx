/*************************************************************************
 *
 *  $RCSfile: macrosecurity.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: gt $ $Date: 2004-07-16 07:52:00 $
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

#ifndef _XMLSECURITY_MACROSECURITY_HXX
#define _XMLSECURITY_MACROSECURITY_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/simptabl.hxx>
#include <svtools/svmedit.hxx>

#include <xmlsecurity/documentsignaturehelper.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory; }
namespace io {
    class XStream; }
namespace embed {
    class XStorage; }
}}}

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;

class MacroSecurity : public TabDialog
{
private:
    friend class MacroSecurityLevelTP;
    friend class MacroSecurityTrustedSourcesTP;

    TabControl          maTabCtrl;
    OKButton            maOkBtn;
    CancelButton        maCancelBtn;
    HelpButton          maHelpBtn;
    PushButton          maResetBtn;

    XMLSignatureHelper      maSignatureHelper;
    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    SignatureInformations   maCurrentSignatureInformations;
public:
    MacroSecurity( Window* pParent,
                    cssu::Reference< css::lang::XMultiServiceFactory >& rxMSF,
                    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment );
    virtual             ~MacroSecurity();
};


class MacroSecurityTP : public TabPage
{
protected:
    MacroSecurity*      mpDlg;
public:
                        MacroSecurityTP( Window* _pParent, const ResId& _rResId, MacroSecurity* _pDlg );
    inline void         SetTabDlg( MacroSecurity* pTabDlg );
};

inline void MacroSecurityTP::SetTabDlg( MacroSecurity* _pTabDlg )
{
    mpDlg = _pTabDlg;
}


class MacroSecurityLevelTP : public MacroSecurityTP
{
private:
    FixedLine           maSecLevelFL;
    RadioButton         maVeryHighRB;
    RadioButton         maHighRB;
    RadioButton         maMediumRB;
    RadioButton         maLowRB;
public:
                        MacroSecurityLevelTP( Window* pParent, MacroSecurity* _pDlg );

    virtual void        ActivatePage();
};


class MacroSecurityTrustedSourcesTP : public MacroSecurityTP
{
private:
    FixedLine           maTrustCertFL;
    SvxSimpleTable      maTrustCertLB;
    PushButton          maAddCertPB;
    PushButton          maViewCertPB;
    PushButton          maRemoveCertPB;
    FixedLine           maTrustFileLocFL;
    FixedInfo           maTrustFileLocFI;
    ListBox             maTrustFileLocLB;
    PushButton          maAddLocPB;
    PushButton          maRemoveLocPB;

    DECL_LINK(          AddCertPBHdl, void* );
    DECL_LINK(          ViewCertPBHdl, void* );
    DECL_LINK(          RemoveCertPBHdl, void* );
    DECL_LINK(          AddLocPBHdl, void* );
    DECL_LINK(          RemoveLocPBHdl, void* );
    DECL_LINK(          TrustCertLBSelectHdl, void* );
    DECL_LINK(          TrustFileLocLBSelectHdl, void* );

//  void                InsertCert( cssu::Reference< css::security::XCertificate >& _rxCert, USHORT _nInd );
    void                FillCertLB( void );
public:
                        MacroSecurityTrustedSourcesTP( Window* pParent, MacroSecurity* _pDlg );

    virtual void        ActivatePage();
};


#endif // _XMLSECURITY_MACROSECURITY_HXX

