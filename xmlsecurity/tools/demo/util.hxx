/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: util.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:39:00 $
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

#include <rtl/ustring.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>

// Get the demo.rdb servcie manager...
::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > CreateDemoServiceFactory();

// Ask user to show more signature details...
void QueryPrintSignatureDetails( const SignatureInformations& SignatureInformations, ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > );

// Query value from user.
int QuerySelectNumber( int nMin, int nMax );

// Ask to verify the signature
long QueryVerifySignature();

// Open In/Output Stream
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > OpenInputStream( const ::rtl::OUString& rStreamName );
::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > OpenOutputStream( const ::rtl::OUString& rStreamName );

::rtl::OUString getSignatureInformations( const SignatureInformations& SignatureInformations, ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecurityEnvironment );
::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > getCertificateFromEnvironment( ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecurityEnvironment, BOOL nType);
