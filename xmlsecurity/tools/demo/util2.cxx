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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

#include <rtl/locale.h>
#include <osl/nlsupport.h>
#include <osl/process.h>

#include <util.hxx>

#include <stdio.h>

#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/security/KeyUsage.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <xmlsecurity/biginteger.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamhelper.hxx>

#include <rtl/ustrbuf.hxx>
#include <tools/string.hxx>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssi = com::sun::star::io;

using namespace ::com::sun::star;

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


::rtl::OUString getSignatureInformation(
    const SignatureInformation& infor,
    cssu::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >& xSecurityEnvironment )
{
    char* status[50] = {
        "STATUS_UNKNOWN",
        "OPERATION_SUCCEEDED",
        "RUNTIMEERROR_FAILED",
        "ENGINE_FAILED",
        "MALLOC_FAILED",
        "STRDUP_FAILED",
        "CRYPTO_FAILED",
        "XML_FAILED",
        "XSLT_FAILED",
        "IO_FAILED",
        "DISABLED",
        "NOT_IMPLEMENTED",
        "INVALID_SIZE",
        "INVALID_DATA",
        "INVALID_RESULT",
        "INVALID_TYPE",
        "INVALID_OPERATION",
        "INVALID_STATUS",
        "INVALID_FORMAT",
        "DATA_NOT_MATCH",
        "INVALID_NODE",
        "INVALID_NODE_CONTENT",
        "INVALID_NODE_ATTRIBUTE",
        "MISSING_NODE_ATTRIBUTE",
        "NODE_ALREADY_PRESENT",
        "UNEXPECTED_NODE",
        "NODE_NOT_FOUND",
        "INVALID_TRANSFORM",
        "INVALID_TRANSFORM_KEY",
        "INVALID_URI_TYPE",
        "TRANSFORM_SAME_DOCUMENT_REQUIRED",
        "TRANSFORM_DISABLED",
        "INVALID_KEY_DATA",
        "KEY_DATA_NOT_FOUND",
        "KEY_DATA_ALREADY_EXIST",
        "INVALID_KEY_DATA_SIZE",
        "KEY_NOT_FOUND",
        "KEYDATA_DISABLED",
        "MAX_RETRIEVALS_LEVEL",
        "MAX_RETRIEVAL_TYPE_MISMATCH",
        "MAX_ENCKEY_LEVEL",
        "CERT_VERIFY_FAILED",
        "CERT_NOT_FOUND",
        "CERT_REVOKED",
        "CERT_ISSUER_FAILED",
        "CERT_NOT_YET_VALID",
        "CERT_HAS_EXPIRED",
        "DSIG_NO_REFERENCES",
        "DSIG_INVALID_REFERENCE",
        "ASSERTION"};

    rtl::OUString result;

    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Security Id : "))
        +rtl::OUString::valueOf(infor.nSecurityId)
        +rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Status : ["))
        +rtl::OUString::valueOf((sal_Int32)(infor.nStatus))
        +rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("] "))
        +rtl::OUString::createFromAscii(status[infor.nStatus])
        +rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    const SignatureReferenceInformations& rInfors = infor.vSignatureReferenceInfors;
    int i;
    int size = rInfors.size();

    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("--References :\n"));
    for (i=0; i<size; i++)
    {
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("---URI : "));
        result += rInfors[i].ouURI;
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("---DigestValue : "));
        result += rInfors[i].ouDigestValue;
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
    }

        if (infor.ouX509IssuerName.getLength()>0)
        {
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("--X509IssuerName :\n"));
            result += infor.ouX509IssuerName;
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
        }

        if (infor.ouX509SerialNumber.getLength()>0)
        {
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("--X509SerialNumber :\n"));
            result += infor.ouX509SerialNumber;
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
        }

        if (infor.ouX509Certificate.getLength()>0)
        {
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("--X509Certificate :\n"));
            result += infor.ouX509Certificate;
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
        }

        if (infor.ouSignatureValue.getLength()>0)
        {
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("--SignatureValue :\n"));
            result += infor.ouSignatureValue;
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
        }

           result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("--Date :\n"));

    ::rtl::OUStringBuffer buffer;
    convertDateTime( buffer, infor.stDateTime );
    result += buffer.makeStringAndClear();
           result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

        if (infor.ouX509IssuerName.getLength()>0 && infor.ouX509SerialNumber.getLength()>0 && xSecurityEnvironment.is())
        {
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("--Certificate Path :\n"));
            cssu::Reference< ::com::sun::star::security::XCertificate > xCert = xSecurityEnvironment->getCertificate( infor.ouX509IssuerName, numericStringToBigInteger(infor.ouX509SerialNumber) );
            cssu::Sequence < cssu::Reference< ::com::sun::star::security::XCertificate > > xCertPath;
            if(! xCert.is() )
            {
                fprintf(stdout , " xCert is NULL , so can not buildCertificatePath\n");
                return result ;
            }
            else
            {
                xCertPath = xSecurityEnvironment->buildCertificatePath( xCert ) ;
            }

        for( int i = 0; i < xCertPath.getLength(); i++ )
        {
            result += xCertPath[i]->getSubjectName();
                    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n    Subject public key algorithm : "));
                    result += xCertPath[i]->getSubjectPublicKeyAlgorithm();
                    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n    Signature algorithm : "));
                    result += xCertPath[i]->getSignatureAlgorithm();

                    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n    Subject public key value : "));
                    cssu::Sequence< sal_Int8 > keyValue = xCertPath[i]->getSubjectPublicKeyValue();
                    result += printHexString(keyValue);

                    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n    Thumbprint (SHA1) : "));
                    cssu::Sequence< sal_Int8 > SHA1Thumbprint = xCertPath[i]->getSHA1Thumbprint();
                    result += printHexString(SHA1Thumbprint);

                    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n    Thumbprint (MD5) : "));
                    cssu::Sequence< sal_Int8 > MD5Thumbprint = xCertPath[i]->getMD5Thumbprint();
                    result += printHexString(MD5Thumbprint);

                    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n  <<\n"));
        }

                   result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n    Key Usage : "));
                   sal_Int32 usage = xCert->getCertificateUsage();

                   if (usage & ::com::sun::star::security::KeyUsage::DIGITAL_SIGNATURE)
                   {
                       result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DIGITAL_SIGNATURE "));
                   }

                   if (usage & ::com::sun::star::security::KeyUsage::NON_REPUDIATION)
                   {
                       result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NON_REPUDIATION "));
                   }

                   if (usage & ::com::sun::star::security::KeyUsage::KEY_ENCIPHERMENT)
                   {
                       result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KEY_ENCIPHERMENT "));
                   }

                   if (usage & ::com::sun::star::security::KeyUsage::DATA_ENCIPHERMENT)
                   {
                       result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATA_ENCIPHERMENT "));
                   }

                   if (usage & ::com::sun::star::security::KeyUsage::KEY_AGREEMENT)
                   {
                       result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KEY_AGREEMENT "));
                   }

                   if (usage & ::com::sun::star::security::KeyUsage::KEY_CERT_SIGN)
                   {
                       result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KEY_CERT_SIGN "));
                   }

                   if (usage & ::com::sun::star::security::KeyUsage::CRL_SIGN)
                   {
                       result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CRL_SIGN "));
                   }

                   result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
        }

    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
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

    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    return result;
}

::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >
    getCertificateFromEnvironment( ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >  xSecurityEnvironment , sal_Bool nType)
{
    cssu::Sequence< cssu::Reference< ::com::sun::star::security::XCertificate > > xPersonalCerts ;
    int length = 0;
    int i;

    // add By CP
    sal_uInt16 encoding ;
    rtl_Locale *pLocale = NULL ;
    osl_getProcessLocale( &pLocale ) ;
    encoding = osl_getTextEncodingFromLocale( pLocale ) ;
    // CP end

    if( nType != sal_False )
        xPersonalCerts = xSecurityEnvironment->getPersonalCertificates() ;
    else
        return NULL; // not support then;

    length = xPersonalCerts.getLength();
    if(length == 0)
    {
        fprintf( stdout, "\nNo certificate found!\n" ) ;
        return NULL;
    }

    fprintf( stdout, "\nSelect a certificate:\n" ) ;
    for( i = 0; i < length; i ++ )
    {
        rtl::OUString xxxIssuer;
        rtl::OUString xxxSubject;
        rtl::OString yyyIssuer;
        rtl::OString yyySubject;

        xxxIssuer=xPersonalCerts[i]->getIssuerName();
        yyyIssuer=rtl::OUStringToOString( xxxIssuer, encoding );

        xxxSubject=xPersonalCerts[i]->getSubjectName();
        yyySubject=rtl::OUStringToOString( xxxSubject, encoding );

        fprintf( stdout, "\n%d:\nsubject=[%s]\nissuer=[%s]\n",
            i+1,
            yyySubject.getStr(),
            yyyIssuer.getStr());
    }

    int sel = QuerySelectNumber( 1, length ) -1;
    return xPersonalCerts[sel] ;
}

void QueryPrintSignatureDetails( const SignatureInformations& SignatureInformations, ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > rSecEnv )
{
    char cShowDetails;
    fprintf( stdout, "\nDisplay details (y/n) [y]?" );
    fflush( stdout );
    fscanf( stdin, "%c", &cShowDetails);
    if ( cShowDetails == 'y' )
    {
        rtl_Locale *pLocale = NULL ;
        osl_getProcessLocale( &pLocale ) ;
        sal_uInt16 encoding = osl_getTextEncodingFromLocale( pLocale ) ;

        fprintf( stdout, "------------- Signature details START -------------\n" );
        fprintf( stdout, "%s",
            rtl::OUStringToOString(
                getSignatureInformations( SignatureInformations, rSecEnv),
                encoding).getStr());

        fprintf( stdout, "------------- Signature details END -------------\n" );
    }
}

int QuerySelectNumber( int nMin, int nMax )
{
    fprintf( stdout, "\n" ) ;
    int sel = 0;
    do
    {
        fprintf( stdout, "\nSelect <%d-%d>:", nMin, nMax ) ;
        fflush( stdout );
        fscanf( stdin, "%d", &sel ) ;
    } while( ( sel < nMin ) || ( sel > nMax ) );

    return sel;
}

long QueryVerifySignature()
{
    char answer;
    fprintf( stdout, "\nFound a signature - verify this one (y/n) [y]?" );
    fflush( stdout );
    fscanf( stdin, "%c", &answer);
    return  (answer == 'n')?0:1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
