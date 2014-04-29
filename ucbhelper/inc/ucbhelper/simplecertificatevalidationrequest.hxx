/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
      * returns the continuation that was chosen by the interaction handler.
      *
      * @return the continuation chosen by an interaction handler or
      *         CONTINUATION_UNKNOWN, if the request was not (yet) handled.
      */
    sal_Int32 getResponse() const;
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX */
