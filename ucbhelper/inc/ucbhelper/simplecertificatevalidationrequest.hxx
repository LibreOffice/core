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

#ifndef _UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX
#define _UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX

#include <rtl/ref.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include "ucbhelper/ucbhelperdllapi.h"
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
  * @see com::sun::star::ucb::CertificateValidationRequest
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
      * @param pCertificate contaisn the server certificate.
      */
    SimpleCertificateValidationRequest( const sal_Int32 & lCertificateValidity,
        const com::sun::star::uno::Reference<com::sun::star::security::XCertificate> pCertificate,
        const rtl::OUString & hostname );

    /**
      * After passing this request to XInteractionHandler::handle, this method
      * returns the continuation that was choosen by the interaction handler.
      *
      * @return the continuation choosen by an interaction handler or
      *         CONTINUATION_UNKNOWN, if the request was not (yet) handled.
      */
    sal_Int32 getResponse() const;
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
