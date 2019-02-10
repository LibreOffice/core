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


#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/XCertificateExtension.hpp>
#include <com/sun/star/security/XSanExtension.hpp>
#include <com/sun/star/security/ExtAltNameType.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/ucb/CertificateValidationRequest.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <svl/zforlist.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <ids.hxx>
#include <ids.hrc>
#include "getcontinuations.hxx"
#include "sslwarndlg.hxx"
#include "unknownauthdlg.hxx"

#include "iahndl.hxx"

#include <memory>

#define OID_SUBJECT_ALTERNATIVE_NAME "2.5.29.17"


using namespace com::sun::star;

namespace {

OUString
getContentPart( const OUString& _rRawString )
{
    // search over some parts to find a string
    static char const * aIDs[] = { "CN=", "OU=", "O=", "E=", nullptr };
    OUString sPart;
    int i = 0;
    while ( aIDs[i] )
    {
        OUString sPartId = OUString::createFromAscii( aIDs[i++] );
        sal_Int32 nContStart = _rRawString.indexOf( sPartId );
        if ( nContStart != -1 )
        {
            nContStart += sPartId.getLength();
            sal_Int32 nContEnd = _rRawString.indexOf( ',', nContStart );
            if ( nContEnd != -1 )
                sPart = _rRawString.copy( nContStart, nContEnd - nContStart );
            else
                sPart = _rRawString.copy( nContStart );
            break;
        }
    }
    return sPart;
}

bool
isDomainMatch(
              const OUString& hostName, const uno::Sequence< OUString >& certHostNames)
{
    for ( int i = 0; i < certHostNames.getLength(); i++){
       OUString element = certHostNames[i];

       if (element.isEmpty())
           continue;

       if (hostName.equalsIgnoreAsciiCase( element ))
           return true;

       if (element.startsWith("*") &&
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
    tools::Time  aTime( tools::Time::EMPTY );

    aDate = Date( rDateTime.Day, rDateTime.Month, rDateTime.Year );
    aTime = tools::Time( rDateTime.Hours, rDateTime.Minutes, rDateTime.Seconds );

    LanguageType eUILang = Application::GetSettings().GetUILanguageTag().getLanguageType();
    SvNumberFormatter *pNumberFormatter = new SvNumberFormatter( xContext, eUILang );
    OUString      aTmpStr;
    Color*      pColor = nullptr;
    const Date&  rNullDate = pNumberFormatter->GetNullDate();
    sal_uInt32  nFormat
        = pNumberFormatter->GetStandardFormat( SvNumFormatType::DATE, eUILang );

    pNumberFormatter->GetOutputString( aDate - rNullDate, nFormat, aTmpStr, &pColor );
    aDateTimeStr = aTmpStr + " ";

    nFormat = pNumberFormatter->GetStandardFormat( SvNumFormatType::TIME, eUILang );
    pNumberFormatter->GetOutputString(
        aTime.GetTimeInDays(), nFormat, aTmpStr, &pColor );
    aDateTimeStr += aTmpStr;

    return aDateTimeStr;
}

bool
executeUnknownAuthDialog(
    weld::Window * pParent,
    uno::Reference< uno::XComponentContext > const & xContext,
    const uno::Reference< security::XCertificate >& rXCert)
{
    SolarMutexGuard aGuard;

    UnknownAuthDialog aDialog(pParent, rXCert, xContext);

    // Get correct resource string
    OUString aMessage;

    std::vector< OUString > aArguments;
    aArguments.push_back( getContentPart( rXCert->getSubjectName()) );

    std::locale aResLocale(Translate::Create("uui"));
    ErrorResource aErrorResource(RID_UUI_ERRHDL, aResLocale);

    aMessage = Translate::get(STR_UUI_UNKNOWNAUTH_UNTRUSTED, aResLocale);
    aMessage = UUIInteractionHelper::replaceMessageWithArguments(
            aMessage, aArguments );
    aDialog.setDescriptionText( aMessage );

    return static_cast<bool>(aDialog.run());
}

enum class SslWarnType {
    DOMAINMISMATCH, EXPIRED, INVALID
};

bool
executeSSLWarnDialog(
    weld::Window * pParent,
    uno::Reference< uno::XComponentContext > const & xContext,
    const uno::Reference< security::XCertificate >& rXCert,
    SslWarnType failure,
    const OUString & hostName )
{
    SolarMutexGuard aGuard;

    SSLWarnDialog aDialog(pParent, rXCert, xContext);

    // Get correct resource string
    std::vector< OUString > aArguments_1;
    char const * pMessageKey = nullptr;
    char const * pTitleKey = nullptr;

    switch( failure )
    {
        case SslWarnType::DOMAINMISMATCH:
            pMessageKey = STR_UUI_SSLWARN_DOMAINMISMATCH;
            pTitleKey = STR_UUI_SSLWARN_DOMAINMISMATCH_TITLE;
            aArguments_1.push_back( hostName );
            aArguments_1.push_back(
                getContentPart( rXCert->getSubjectName()) );
            aArguments_1.push_back( hostName );
            break;
        case SslWarnType::EXPIRED:
            pMessageKey = STR_UUI_SSLWARN_EXPIRED;
            pTitleKey = STR_UUI_SSLWARN_EXPIRED_TITLE;
            aArguments_1.push_back(
                getContentPart( rXCert->getSubjectName()) );
            aArguments_1.push_back(
                getLocalizedDatTimeStr( xContext,
                                        rXCert->getNotValidAfter() ) );
            aArguments_1.push_back(
                getLocalizedDatTimeStr( xContext,
                                        rXCert->getNotValidAfter() ) );
            break;
        case SslWarnType::INVALID:
            pMessageKey = STR_UUI_SSLWARN_INVALID;
            pTitleKey = STR_UUI_SSLWARN_INVALID_TITLE;
            break;
        default: assert(false);
    }

    std::locale aResLocale(Translate::Create("uui"));

    OUString aMessage_1 = Translate::get(pMessageKey, aResLocale);
    aMessage_1 = UUIInteractionHelper::replaceMessageWithArguments(
            aMessage_1, aArguments_1 );
    aDialog.setDescription1Text( aMessage_1 );

    OUString aTitle = Translate::get(pTitleKey, aResLocale);
    aDialog.set_title(aTitle);

    return static_cast<bool>(aDialog.run());
}

void
handleCertificateValidationRequest_(
    weld::Window * pParent,
    uno::Reference< uno::XComponentContext > const & xContext,
    ucb::CertificateValidationRequest const & rRequest,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations)
{
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    getContinuations(rContinuations, &xApprove, &xAbort);

    if ( comphelper::LibreOfficeKit::isActive() && xApprove.is() )
    {
        xApprove->select();
        return;
    }

    sal_Int32 failures = rRequest.CertificateValidity;
    bool trustCert = true;

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
    uno::Reference< security::XSanExtension > sanExtension;
    for (sal_Int32 i = 0 ; i < extensions.getLength(); ++i)
    {
        uno::Reference< security::XCertificateExtension >element = extensions[i];
        OString aId ( reinterpret_cast<const char *>(element->getExtensionId().getConstArray()), element->getExtensionId().getLength());
        if (aId == OID_SUBJECT_ALTERNATIVE_NAME)
        {
           sanExtension = uno::Reference<security::XSanExtension>(element, uno::UNO_QUERY);
           break;
        }
    }

    std::vector<security::CertAltNameEntry> altNames;
    if (sanExtension.is())
    {
        altNames = comphelper::sequenceToContainer<std::vector<security::CertAltNameEntry>>(sanExtension->getAlternativeNames());
    }

    OUString certHostName = getContentPart( rRequest.Certificate->getSubjectName() );
    uno::Sequence< OUString > certHostNames(altNames.size() + 1);

    certHostNames[0] = certHostName;

    for (size_t n = 0; n < altNames.size(); ++n)
    {
        if (altNames[n].Type ==  security::ExtAltNameType_DNS_NAME)
        {
           altNames[n].Value >>= certHostNames[n+1];
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
                                          SslWarnType::DOMAINMISMATCH,
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
                                          SslWarnType::EXPIRED,
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
                                          SslWarnType::INVALID,
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
{
    uno::Any aAnyRequest(rRequest->getRequest());

    ucb::CertificateValidationRequest aCertificateValidationRequest;
    if (aAnyRequest >>= aCertificateValidationRequest)
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        handleCertificateValidationRequest_(Application::GetFrameWeld(xParent),
                                            m_xContext,
                                            aCertificateValidationRequest,
                                            rRequest->getContinuations());
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
