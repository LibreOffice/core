/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: unknownauthdlg.hxx,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: rt $ $Date: 2007-11-07 10:12:10 $
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
#ifndef UUI_UNKNOWNAUTH_HXX
#define UUI_UNKNOWNAUTH_HXX


#include <vcl/dialog.hxx>

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

#ifndef _COM_SUN_STAR_SECURITY_XCERTIFICATE_HPP_
#include <com/sun/star/security/XCertificate.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_XXSECURITYENVIRONMENT_HPP_
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;


using namespace com::sun::star;

//=====================================================================
//= Https_UADialog
//=====================================================================
class UnknownAuthDialog : public ModalDialog
{
private:
    PushButton  m_aCommandButtonOK;
    CancelButton  m_aCommandButtonCancel;
    HelpButton  m_aCommandButtonHelp;
    PushButton  m_aView_Certificate;
    RadioButton m_aOptionButtonAccept;
    RadioButton m_aOptionButtonDontAccept;
    FixedLine   m_aLine;
    FixedText   m_aLabel1;
    FixedImage  m_aWarnImage;

    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& m_xServiceFactory;
    const cssu::Reference< dcss::security::XCertificate >& m_rXCert;
    Window* m_pParent;
    ResMgr* pResourceMgr;

    DECL_LINK( OKHdl_Impl, PushButton * );
    DECL_LINK( ViewCertHdl_Impl, PushButton * );

    public:
    UnknownAuthDialog( Window* pParent,
                       const cssu::Reference< dcss::security::XCertificate >& rXCert,
                       const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
                       ResMgr * pResMgr );

    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > getServiceFactory() { return m_xServiceFactory; };

    cssu::Reference< dcss::security::XCertificate > getCert() { return m_rXCert; };

    Window* getParent() { return m_pParent; };

    void setDescriptionText( const rtl::OUString &aText ) { m_aLabel1.SetText( aText ); };

};

#endif // UUI_UNKNOWNAUTH_HXX
