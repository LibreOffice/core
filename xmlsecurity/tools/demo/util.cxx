/*************************************************************************
 *
 *  $RCSfile: util.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mmi $ $Date: 2004-08-12 02:30:19 $
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

#include "util.hxx"

#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <xmlsecurity/biginteger.hxx>

#include <rtl/ustrbuf.hxx>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;

/** convert util::DateTime to ISO Date String */
void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
    const com::sun::star::util::DateTime& rDateTime )
{
    String aString( String::CreateFromInt32( rDateTime.Year ) );
    aString += '-';
    if( rDateTime.Month < 10 )
        aString += '0';
    aString += String::CreateFromInt32( rDateTime.Month );
    aString += '-';
    if( rDateTime.Day < 10 )
        aString += '0';
    aString += String::CreateFromInt32( rDateTime.Day );

    if( rDateTime.Seconds != 0 ||
        rDateTime.Minutes != 0 ||
        rDateTime.Hours   != 0 )
    {
        aString += 'T';
        if( rDateTime.Hours < 10 )
            aString += '0';
        aString += String::CreateFromInt32( rDateTime.Hours );
        aString += ':';
        if( rDateTime.Minutes < 10 )
            aString += '0';
        aString += String::CreateFromInt32( rDateTime.Minutes );
        aString += ':';
        if( rDateTime.Seconds < 10 )
            aString += '0';
        aString += String::CreateFromInt32( rDateTime.Seconds );
        if ( rDateTime.HundredthSeconds > 0)
        {
            aString += ',';
            if (rDateTime.HundredthSeconds < 10)
                aString += '0';
            aString += String::CreateFromInt32( rDateTime.HundredthSeconds );
        }
    }

    rBuffer.append( aString );
}

::rtl::OUString printHexString(cssu::Sequence< sal_Int8 > data)
{
    int length = data.getLength();
    ::rtl::OUString result;

    char number[4];
    for (int j=0; j<length; j++)
    {
        sprintf(number, "%02X ", (unsigned char)data[j]);
        result += rtl::OUString::createFromAscii( number );
    }

    return result;
}

cssu::Reference< cssl::XMultiServiceFactory > serviceManager(
    cssu::Reference< cssu::XComponentContext > &xContext,
    rtl::OUString sUnoUrl,
    rtl::OUString sRdbUrl )
    throw( cssu::RuntimeException , cssu::Exception )
{
    cssu::Reference< cssl::XMultiComponentFactory > xLocalServiceManager = NULL ;
    cssu::Reference< cssu::XComponentContext > xLocalComponentContext = NULL ;

    cssu::Reference< ::com::sun::star::registry::XSimpleRegistry > xSimpleRegistry
        = ::cppu::createSimpleRegistry();
    OSL_ENSURE( xSimpleRegistry.is(),
        "serviceManager - "
        "Cannot create simple registry" ) ;

    xSimpleRegistry->open(sRdbUrl, sal_True, sal_False);
    OSL_ENSURE( xSimpleRegistry->isValid() ,
        "serviceManager - "
        "Cannot open xml security registry rdb" ) ;

    xLocalComponentContext = ::cppu::bootstrap_InitialComponentContext( xSimpleRegistry ) ;
    OSL_ENSURE( xLocalComponentContext.is() ,
        "serviceManager - "
        "Cannot create intial component context" ) ;

    xLocalServiceManager = xLocalComponentContext->getServiceManager() ;
    OSL_ENSURE( xLocalServiceManager.is() ,
        "serviceManager - "
        "Cannot create intial service manager" ) ;

    xContext = xLocalComponentContext ;
    return cssu::Reference< cssl::XMultiServiceFactory >(xLocalServiceManager, cssu::UNO_QUERY) ;
}

::rtl::OUString getSignatureInformation(
    const SignatureInformation& infor,
    cssu::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >& xSecurityEnvironment )
{
    char* status[5]={"INIT", "CREATION_SUCCEED", "CREATION_FAIL", "VERIFY_SUCCEED", "VERIFY_FAIL"};

    rtl::OUString result;

    result += rtl::OUString::createFromAscii( "Security Id : " )
        +rtl::OUString::valueOf(infor.nSecurityId)
        +rtl::OUString::createFromAscii( "\n" );
    result += rtl::OUString::createFromAscii( "Status : " )
        +rtl::OUString::createFromAscii( status[infor.nStatus] )
        +rtl::OUString::createFromAscii( "\n" );

    const SignatureReferenceInformations& rInfors = infor.vSignatureReferenceInfors;
    int i;
    int size = rInfors.size();

    result += rtl::OUString::createFromAscii( "--References :\n" );
    for (i=0; i<size; i++)
    {
            result += rtl::OUString::createFromAscii( "---URI : " );
        result += rInfors[i].ouURI;
        result += rtl::OUString::createFromAscii( "\n" );
            result += rtl::OUString::createFromAscii( "---DigestValue : " );
        result += rInfors[i].ouDigestValue;
        result += rtl::OUString::createFromAscii( "\n" );
    }

        if (infor.ouX509IssuerName.getLength()>0)
        {
            result += rtl::OUString::createFromAscii( "--X509IssuerName :\n" );
            result += infor.ouX509IssuerName;
            result += rtl::OUString::createFromAscii( "\n" );
        }

        if (infor.ouX509SerialNumber.getLength()>0)
        {
            result += rtl::OUString::createFromAscii( "--X509SerialNumber :\n" );
            result += infor.ouX509SerialNumber;
            result += rtl::OUString::createFromAscii( "\n" );
        }

        if (infor.ouX509Certificate.getLength()>0)
        {
            result += rtl::OUString::createFromAscii( "--X509Certificate :\n" );
            result += infor.ouX509Certificate;
            result += rtl::OUString::createFromAscii( "\n" );
        }

        if (infor.ouSignatureValue.getLength()>0)
        {
            result += rtl::OUString::createFromAscii( "--SignatureValue :\n" );
            result += infor.ouSignatureValue;
            result += rtl::OUString::createFromAscii( "\n" );
        }

           result += rtl::OUString::createFromAscii( "--Date :\n" );

    ::rtl::OUStringBuffer buffer;
    convertDateTime( buffer, infor.stDateTime );
    result += buffer.makeStringAndClear();
           result += rtl::OUString::createFromAscii( "\n" );

        /*
        if (infor.ouDate.getLength()>0)
        {
            result += rtl::OUString::createFromAscii( "--Date :\n" );
            result += infor.ouDate;
            result += rtl::OUString::createFromAscii( "\n" );
        }

        if (infor.ouTime.getLength()>0)
        {
            result += rtl::OUString::createFromAscii( "--Time :\n" );
            result += infor.ouTime;
            result += rtl::OUString::createFromAscii( "\n" );
        }
        */

        if (infor.ouX509IssuerName.getLength()>0 && infor.ouX509SerialNumber.getLength()>0 && xSecurityEnvironment.is())
        {
            result += rtl::OUString::createFromAscii( "--Certificate Path :\n" );
            cssu::Reference< ::com::sun::star::security::XCertificate > xCert
                = xSecurityEnvironment->getCertificate( infor.ouX509IssuerName, numericStringToBigInteger(infor.ouX509SerialNumber) );

        cssu::Sequence < cssu::Reference< ::com::sun::star::security::XCertificate > > xCertPath
            = xSecurityEnvironment->buildCertificatePath( xCert ) ;

        for( int i = 0; i < xCertPath.getLength(); i++ )
        {
            result += xCertPath[i]->getSubjectName();
                    result += rtl::OUString::createFromAscii( "\n    Subject public key algorithm : " );
                    result += xCertPath[i]->getSubjectPublicKeyAlgorithm();
                    result += rtl::OUString::createFromAscii( "\n    Signature algorithm : " );
                    result += xCertPath[i]->getSignatureAlgorithm();

                    result += rtl::OUString::createFromAscii( "\n    Subject public key value : " );
                    cssu::Sequence< sal_Int8 > keyValue = xCertPath[i]->getSubjectPublicKeyValue();
                    result += printHexString(keyValue);

                    result += rtl::OUString::createFromAscii( "\n    Thumbprint (SHA1) : " );
                    cssu::Sequence< sal_Int8 > SHA1Thumbprint = xCertPath[i]->getSHA1Thumbprint();
                    result += printHexString(SHA1Thumbprint);

                    result += rtl::OUString::createFromAscii( "\n    Thumbprint (MD5) : " );
                    cssu::Sequence< sal_Int8 > MD5Thumbprint = xCertPath[i]->getMD5Thumbprint();
                    result += printHexString(MD5Thumbprint);

                    result += rtl::OUString::createFromAscii( "\n  <<\n" );
        }

            result += rtl::OUString::createFromAscii( "\n" );
        }

    result += rtl::OUString::createFromAscii( "\n" );
    return result;
}

::rtl::OUString getSignatureInformations(
    const SignatureInformations& SignatureInformations,
    cssu::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecurityEnvironment )
{
    rtl::OUString result;
    int i;
    int size = SignatureInformations.size();

    for (i=0; i<size; i++)
    {
        const SignatureInformation& infor = SignatureInformations[i];
        result += getSignatureInformation( infor, xSecurityEnvironment );
    }

    result += rtl::OUString::createFromAscii( "\n" );

    return result;
}
