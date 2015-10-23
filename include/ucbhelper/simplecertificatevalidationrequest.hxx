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

#ifndef INCLUDED_UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX
#define INCLUDED_UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX

#include <rtl/ref.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include <ucbhelper/ucbhelperdllapi.h>
#include <com/sun/star/security/XCertificate.hpp>


namespace ucbhelper {

/**
  * This class implements a simple validation interaction request of a certificate.
  * Instances can be passed directly to XInteractionHandler::handle(...). Each
  * instance contains an CertificateValidationRequest and two interaction
  * continuations: "Abort" and "Approved". The parameters
  * for the CertificateValidationRequest object are partly taken from contructors parameters and partly defaulted
  * as follows:
  *
  * Read-write values: certificateValidity, certificate
  *
  * @see css::ucb::CertificateValidationRequest
  * @see InteractionApproved
  * @see InteractionRetry
  */
class UCBHELPER_DLLPUBLIC SimpleCertificateValidationRequest : public ucbhelper::InteractionRequest
{
public:
    /**
      * Constructor.
      *
      * @param lCertificateValidity contains a bitmask which validation error occur.
      * @param pCertificate contains the server certificate.
      */
    SimpleCertificateValidationRequest( const sal_Int32 & lCertificateValidity,
        const css::uno::Reference<css::security::XCertificate>& certificate,
        const OUString & hostname );
};

} // namespace ucbhelper

#endif /* ! INCLUDED_UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
