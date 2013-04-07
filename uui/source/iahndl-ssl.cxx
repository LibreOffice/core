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


#include "com/sun/star/security/CertificateValidity.hpp"
#include "com/sun/star/security/XCertificateExtension.hpp"
#include "com/sun/star/security/XSanExtension.hpp"
#include <com/sun/star/security/ExtAltNameType.hpp>
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/task/XInteractionRequest.hpp"
#include "com/sun/star/ucb/CertificateValidationRequest.hpp"
#include <com/sun/star/uno/Reference.hxx>

#include "osl/mutex.hxx"
#include <com/sun/star/uno/Sequence.hxx>
#include "svl/zforlist.hxx"
#include "vcl/svapp.hxx"

#include "ids.hrc"
#include "getcontinuations.hxx"
#include "sslwarndlg.hxx"
#include "unknownauthdlg.hxx"

#include "iahndl.hxx"

#include <boost/scoped_ptr.hpp>

#define DESCRIPTION_1 1
#define TITLE 3

#define OID_SUBJECT_ALTERNATIVE_NAME "2.5.29.17"


using namespace com::sun::star;

namespace {

String
getContentPart( const String& _rRawString )
{
    // search over some parts to find a string
    static char const * aIDs[] = { "CN=", "OU=", "O=", "E=", NULL };
    String sPart;
    int i = 0;
    while ( aIDs[i] )
    {
        String sPartId = OUString::createFromAscii( aIDs[i++] );
        xub_StrLen nContStart = _rRawString.Search( sPartId );
        if ( nContStart != STRING_NOTFOUND )
        {
            nContStart = nContStart + sPartId.Len();
            xub_StrLen nContEnd
                = _rRawString.Search( sal_Unicode( ',' ), nContStart );
            sPart = String( _rRawString, nContStart, nContEnd - nContStart );
            break;
        }
    }
    return sPart;
}

bool
isDomainMatch(
              OUString hostName, uno::Sequence< OUString > certHostNames)
{
    for ( int i = 0; i < certHostNames.getLength(); i++){
        OUString element = certHostNames[i];

       if (element.isEmpty())
           continue;

       if (hostName.equalsIgnoreAsciiCase( element ))
           return true;

       if ( 0 == element.indexOf( '*' ) &&
                 hostName.getLength() >= element.getLength()  )
       {
           OUString cmpStr = element.copy( 1 );
           if ( hostName.matchIgnoreAsciiCase(
                    cmpStr, hostName.getLength() - cmpStr.getLength()) )
               return true;
       }
    }

    return false;
}

OUString
getLocalizedDatTimeStr(
    uno::Reference< uno::XComponentContext> const & xContext,
    util::DateTime const & rDateTime )
{
    OUString aDateTimeStr;
    Date  aDate( Date::EMPTY );
    Time  aTime( Time::EMPTY );

    aDate = Date( rDateTime.Day, rDateTime.Month, rDateTime.Year );
    aTime = Time( rDateTime.Hours, rDateTime.Minutes, rDateTime.Seconds );

    LanguageType eUILang = Application::GetSettings().GetUILanguageTag().getLanguageType();
    SvNumberFormatter *pNumberFormatter = new SvNumberFormatter( xContext, eUILang );
    String      aTmpStr;
    Color*      pColor = NULL;
    Date*       pNullDate = pNumberFormatter->GetNullDate();
    sal_uInt32  nFormat
        = pNumberFormatter->GetStandardFormat( NUMBERFORMAT_DATE, eUILang );

    pNumberFormatter->GetOutputString(
        aDate - *pNullDate, nFormat, aTmpStr, &pColor );
    aDateTimeStr = aTmpStr + OUString(" ");

    nFormat = pNumberFormatter->GetStandardFormat( NUMBERFORMAT_TIME, eUILang );
    pNumberFormatter->GetOutputString(
        aTime.GetTimeInDays(), nFormat, aTmpStr, &pColor );
    aDateTimeStr += aTmpStr;

    return aDateTimeStr;
}

sal_Bool
executeUnknownAuthDialog(
    Window * pParent,
    uno::Reference< uno::XComponentContext > const & xContext,
    const uno::Reference< security::XCertificate >& rXCert)
    SAL_THROW((uno::RuntimeException))
{
    try
    {
        SolarMutexGuard aGuard;

        boost::scoped_ptr< ResMgr > xManager(ResMgr::CreateResMgr("uui"));
        boost::scoped_ptr< UnknownAuthDialog > xDialog(
            new UnknownAuthDialog( pParent,
                                   rXCert,
                                   xContext,
                                   xManager.get()));

        // Get correct resource string
        OUString aMessage;

        std::vector< OUString > aArguments;
        aArguments.push_back( getContentPart( rXCert->getSubjectName()) );

        if (xManager.get())
        {
            ResId aResId(RID_UUI_ERRHDL, *xManager.get());
            if (ErrorResource(aResId).getString(
                    ERRCODE_UUI_UNKNOWNAUTH_UNTRUSTED, aMessage))
            {
                aMessage = UUIInteractionHelper::replaceMessageWithArguments(
                    aMessage, aArguments );
                xDialog->setDescriptionText( aMessage );
            }
        }

        return static_cast<sal_Bool> (xDialog->Execute());
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
                  OUString("out of memory"),
                  uno::Reference< uno::XInterface >());
    }
}

sal_Bool
executeSSLWarnDialog(
    Window * pParent,
    uno::Reference< uno::XComponentContext > const & xContext,
    const uno::Reference< security::XCertificate >& rXCert,
    sal_Int32 const & failure,
    const OUString & hostName )
    SAL_THROW((uno::RuntimeException))
{
    try
    {
        SolarMutexGuard aGuard;

        boost::scoped_ptr< ResMgr > xManager(ResMgr::CreateResMgr("uui"));
        boost::scoped_ptr< SSLWarnDialog > xDialog(
           new SSLWarnDialog( pParent,
                              rXCert,
                              xContext,
                              xManager.get()));

        // Get correct resource string
        OUString aMessage_1;
        std::vector< OUString > aArguments_1;

        switch( failure )
        {
            case SSLWARN_TYPE_DOMAINMISMATCH:
                aArguments_1.push_back( hostName );
                aArguments_1.push_back(
                    getContentPart( rXCert->getSubjectName()) );
                aArguments_1.push_back( hostName );
                break;
            case SSLWARN_TYPE_EXPIRED:
                aArguments_1.push_back(
                    getContentPart( rXCert->getSubjectName()) );
                aArguments_1.push_back(
                    getLocalizedDatTimeStr( xContext,
                                            rXCert->getNotValidAfter() ) );
                aArguments_1.push_back(
                    getLocalizedDatTimeStr( xContext,
                                            rXCert->getNotValidAfter() ) );
                break;
            case SSLWARN_TYPE_INVALID:
                break;
        }

        if (xManager.get())
        {
            ResId aResId(RID_UUI_ERRHDL, *xManager.get());
            if (ErrorResource(aResId).getString(
                    ERRCODE_AREA_UUI_UNKNOWNAUTH + failure + DESCRIPTION_1,
                    aMessage_1))
            {
                aMessage_1 = UUIInteractionHelper::replaceMessageWithArguments(
                    aMessage_1, aArguments_1 );
                xDialog->setDescription1Text( aMessage_1 );
            }

            OUString aTitle;
            ErrorResource(aResId).getString(
                ERRCODE_AREA_UUI_UNKNOWNAUTH + failure + TITLE, aTitle);
            xDialog->SetText( aTitle );
        }

        return static_cast<sal_Bool> (xDialog->Execute());
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
                  OUString("out of memory"),
                  uno::Reference< uno::XInterface >());
    }
}

void
handleCertificateValidationRequest_(
    Window * pParent,
    uno::Reference< uno::XComponentContext > const & xContext,
    ucb::CertificateValidationRequest const & rRequest,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((uno::RuntimeException))
{
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    getContinuations(rContinuations, &xApprove, &xAbort);

    sal_Int32 failures = rRequest.CertificateValidity;
    sal_Bool trustCert = sal_True;

    if ( ((failures & security::CertificateValidity::UNTRUSTED)
             == security::CertificateValidity::UNTRUSTED ) ||
         ((failures & security::CertificateValidity::ISSUER_UNTRUSTED)
             == security::CertificateValidity::ISSUER_UNTRUSTED) ||
         ((failures & security::CertificateValidity::ROOT_UNTRUSTED)
             == security::CertificateValidity::ROOT_UNTRUSTED) )
    {
        trustCert = executeUnknownAuthDialog( pParent,
                                              xContext,
                                              rRequest.Certificate );
    }

    uno::Sequence< uno::Reference< security::XCertificateExtension > > extensions = rRequest.Certificate->getExtensions();
    uno::Sequence< security::CertAltNameEntry > altNames;
    for (sal_Int32 i = 0 ; i < extensions.getLength(); i++){
        uno::Reference< security::XCertificateExtension >element = extensions[i];

        OString aId ( (const sal_Char *)element->getExtensionId().getArray(), element->getExtensionId().getLength());
        if (aId.equals(OID_SUBJECT_ALTERNATIVE_NAME))
        {
           uno::Reference< security::XSanExtension > sanExtension ( element, uno::UNO_QUERY );
           altNames =  sanExtension->getAlternativeNames();
           break;
        }
    }

    OUString certHostName = getContentPart( rRequest.Certificate->getSubjectName() );
    uno::Sequence< OUString > certHostNames(altNames.getLength() + 1);

    certHostNames[0] = certHostName;

    for(int n = 1; n < altNames.getLength(); n++){
        if (altNames[n].Type ==  security::ExtAltNameType_DNS_NAME){
           altNames[n].Value >>= certHostNames[n];
        }
    }

    if ( (!isDomainMatch(
              rRequest.HostName,
              certHostNames )) &&
          trustCert )
    {
        trustCert = executeSSLWarnDialog( pParent,
                                          xContext,
                                          rRequest.Certificate,
                                          SSLWARN_TYPE_DOMAINMISMATCH,
                                          rRequest.HostName );
    }

    else if ( (((failures & security::CertificateValidity::TIME_INVALID)
                == security::CertificateValidity::TIME_INVALID) ||
               ((failures & security::CertificateValidity::NOT_TIME_NESTED)
                == security::CertificateValidity::NOT_TIME_NESTED)) &&
              trustCert )
    {
        trustCert = executeSSLWarnDialog( pParent,
                                          xContext,
                                          rRequest.Certificate,
                                          SSLWARN_TYPE_EXPIRED,
                                          rRequest.HostName );
    }

    else if ( (((failures & security::CertificateValidity::REVOKED)
                == security::CertificateValidity::REVOKED) ||
               ((failures & security::CertificateValidity::SIGNATURE_INVALID)
                == security::CertificateValidity::SIGNATURE_INVALID) ||
               ((failures & security::CertificateValidity::EXTENSION_INVALID)
                == security::CertificateValidity::EXTENSION_INVALID) ||
               ((failures & security::CertificateValidity::INVALID)
                == security::CertificateValidity::INVALID)) &&
              trustCert )
    {
        trustCert = executeSSLWarnDialog( pParent,
                                          xContext,
                                          rRequest.Certificate,
                                          SSLWARN_TYPE_INVALID,
                                          rRequest.HostName );
    }

    if ( trustCert )
    {
        if (xApprove.is())
            xApprove->select();
    }
    else
    {
        if (xAbort.is())
            xAbort->select();
    }
}

} // namespace

bool
UUIInteractionHelper::handleCertificateValidationRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((uno::RuntimeException))
{
    uno::Any aAnyRequest(rRequest->getRequest());

    ucb::CertificateValidationRequest aCertificateValidationRequest;
    if (aAnyRequest >>= aCertificateValidationRequest)
    {
        handleCertificateValidationRequest_(getParentProperty(),
                                            m_xContext,
                                            aCertificateValidationRequest,
                                            rRequest->getContinuations());
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
