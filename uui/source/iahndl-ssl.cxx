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

#include "com/sun/star/security/CertificateValidity.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/task/XInteractionRequest.hpp"
#include "com/sun/star/ucb/CertificateValidationRequest.hpp"

#include "osl/mutex.hxx"
#include "tools/datetime.hxx"
#include "svl/zforlist.hxx"
#include "vcl/svapp.hxx"

#include "ids.hrc"
#include "getcontinuations.hxx"
#include "sslwarndlg.hxx"
#include "unknownauthdlg.hxx"

#include "iahndl.hxx"

#define DESCRIPTION_1 1
#define DESCRIPTION_2 2
#define TITLE 3

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
        String sPartId = String::CreateFromAscii( aIDs[i++] );
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
    rtl::OUString hostName, rtl::OUString certHostName)
{
    if (hostName.equalsIgnoreAsciiCase( certHostName ))
        return true;

    if ( 0 == certHostName.indexOf( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "*" )) ) &&
              hostName.getLength() >= certHostName.getLength()  )
    {
        rtl::OUString cmpStr = certHostName.copy( 1 );

        if ( hostName.matchIgnoreAsciiCase(
                 cmpStr, hostName.getLength() - cmpStr.getLength()) )
            return true;
    }

    return false;
}

rtl::OUString
getLocalizedDatTimeStr(
    uno::Reference< lang::XMultiServiceFactory > const & xServiceFactory,
    util::DateTime const & rDateTime )
{
    rtl::OUString aDateTimeStr;
    Date  aDate;
    Time  aTime;

    aDate = Date( rDateTime.Day, rDateTime.Month, rDateTime.Year );
    aTime = Time( rDateTime.Hours, rDateTime.Minutes, rDateTime.Seconds );

    LanguageType eUILang = Application::GetSettings().GetUILanguage();
    SvNumberFormatter *pNumberFormatter
        = new SvNumberFormatter( xServiceFactory, eUILang );
    String      aTmpStr;
    Color*      pColor = NULL;
    Date*       pNullDate = pNumberFormatter->GetNullDate();
    sal_uInt32  nFormat
        = pNumberFormatter->GetStandardFormat( NUMBERFORMAT_DATE, eUILang );

    pNumberFormatter->GetOutputString(
        aDate - *pNullDate, nFormat, aTmpStr, &pColor );
    aDateTimeStr = aTmpStr + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));

    nFormat = pNumberFormatter->GetStandardFormat( NUMBERFORMAT_TIME, eUILang );
    pNumberFormatter->GetOutputString(
        aTime.GetTimeInDays(), nFormat, aTmpStr, &pColor );
    aDateTimeStr += aTmpStr;

    return aDateTimeStr;
}

sal_Bool
executeUnknownAuthDialog(
    Window * pParent,
    uno::Reference< lang::XMultiServiceFactory > const & xServiceFactory,
    const uno::Reference< security::XCertificate >& rXCert)
    SAL_THROW((uno::RuntimeException))
{
    try
    {
        SolarMutexGuard aGuard;

        std::auto_ptr< ResMgr > xManager(
            ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        std::auto_ptr< UnknownAuthDialog > xDialog(
            new UnknownAuthDialog( pParent,
                                   rXCert,
                                   xServiceFactory,
                                   xManager.get()));

        // Get correct resource string
        rtl::OUString aMessage;

        std::vector< rtl::OUString > aArguments;
        aArguments.push_back( getContentPart( rXCert->getSubjectName()) );

        if (xManager.get())
        {
            ResId aResId(RID_UUI_ERRHDL, *xManager.get());
            if (ErrorResource(aResId).getString(
                    ERRCODE_UUI_UNKNOWNAUTH_UNTRUSTED, &aMessage))
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
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  uno::Reference< uno::XInterface >());
    }
}

sal_Bool
executeSSLWarnDialog(
    Window * pParent,
    uno::Reference< lang::XMultiServiceFactory > const & xServiceFactory,
    const uno::Reference< security::XCertificate >& rXCert,
    sal_Int32 const & failure,
    const rtl::OUString & hostName )
    SAL_THROW((uno::RuntimeException))
{
    try
    {
        SolarMutexGuard aGuard;

        std::auto_ptr< ResMgr > xManager(
           ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        std::auto_ptr< SSLWarnDialog > xDialog(
           new SSLWarnDialog( pParent,
                              rXCert,
                              xServiceFactory,
                              xManager.get()));

        // Get correct resource string
        rtl::OUString aMessage_1;
        std::vector< rtl::OUString > aArguments_1;

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
                    getLocalizedDatTimeStr( xServiceFactory,
                                            rXCert->getNotValidAfter() ) );
                aArguments_1.push_back(
                    getLocalizedDatTimeStr( xServiceFactory,
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
                    &aMessage_1))
            {
                aMessage_1 = UUIInteractionHelper::replaceMessageWithArguments(
                    aMessage_1, aArguments_1 );
                xDialog->setDescription1Text( aMessage_1 );
            }

            rtl::OUString aTitle;
            ErrorResource(aResId).getString(
                ERRCODE_AREA_UUI_UNKNOWNAUTH + failure + TITLE, &aTitle);
            xDialog->SetText( aTitle );
        }

        return static_cast<sal_Bool> (xDialog->Execute());
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  uno::Reference< uno::XInterface >());
    }
}

void
handleCertificateValidationRequest_(
    Window * pParent,
    uno::Reference< lang::XMultiServiceFactory > const & xServiceFactory,
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
                                              xServiceFactory,
                                              rRequest.Certificate );
    }

    if ( (!isDomainMatch(
              rRequest.HostName,
              getContentPart(
                  rRequest.Certificate->getSubjectName()) )) &&
          trustCert )
    {
        trustCert = executeSSLWarnDialog( pParent,
                                          xServiceFactory,
                                          rRequest.Certificate,
                                          SSLWARN_TYPE_DOMAINMISMATCH,
                                          rRequest.HostName );
    }

    if ( (((failures & security::CertificateValidity::TIME_INVALID)
              == security::CertificateValidity::TIME_INVALID) ||
          ((failures & security::CertificateValidity::NOT_TIME_NESTED)
              == security::CertificateValidity::NOT_TIME_NESTED)) &&
         trustCert )
    {
        trustCert = executeSSLWarnDialog( pParent,
                                          xServiceFactory,
                                          rRequest.Certificate,
                                          SSLWARN_TYPE_EXPIRED,
                                          rRequest.HostName );
    }

    if ( (((failures & security::CertificateValidity::REVOKED)
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
                                          xServiceFactory,
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
                                            m_xServiceFactory,
                                            aCertificateValidationRequest,
                                            rRequest->getContinuations());
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
