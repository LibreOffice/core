/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplecertificatevalidationrequest.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-07 10:08:02 $
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

#ifndef _UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX
#define _UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _UCBHELPER_INTERATIONREQUEST_HXX
#include <ucbhelper/interactionrequest.hxx>
#endif
#ifndef INCLUDED_UCBHELPERDLLAPI_H
#include "ucbhelper/ucbhelperdllapi.h"
#endif
#ifndef _COM_SUN_STAR_SECURITY_XCERTIFICATE_HPP_
#include <com/sun/star/security/XCertificate.hpp>
#endif


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
    const sal_Int32 getResponse() const;
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX */
