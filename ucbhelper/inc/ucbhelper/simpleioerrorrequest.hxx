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

#ifndef _UCBHELPER_SIMPLEIOERRORREQUEST_HXX
#define _UCBHELPER_SIMPLEIOERRORREQUEST_HXX

#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <ucbhelper/interactionrequest.hxx>

namespace com { namespace sun { namespace star { namespace ucb {
    class XCommandProcessor;
} } } }

namespace ucbhelper {

/**
  * This class implements a simple IO error interaction request. Instances
  * can be passed directly to XInteractionHandler::handle(...). Each
  * instance contains an InteractiveIOException and one interaction
  * continuation: "Abort".
  *
  * @see com::sun::star::ucb::InteractiveIOException
  * @see InteractionAbort
  */
class SimpleIOErrorRequest : public ucbhelper::InteractionRequest
{
public:
    /**
      * Constructor.
      *
      * @param xContext contains the command processor that executes the
      *        command related to the request.
      *
      * @param eError is the error code to pass along with the request.
      *
      * qparam rArgs are additional parameters according to the specification
      *        of the error code. Refer to com/sun/star/ucb/IOErrorCode.idl
      *        for details.
      */
    SimpleIOErrorRequest( const com::sun::star::ucb::IOErrorCode eError,
                          const com::sun::star::uno::Sequence<
                            com::sun::star::uno::Any > & rArgs,
                          const rtl::OUString & rMessage,
                          const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandProcessor > & xContext
                         );
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_SIMPLEIOERRORREQUEST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
