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

#ifndef INCLUDED_UCBHELPER_SIMPLEIOERRORREQUEST_HXX
#define INCLUDED_UCBHELPER_SIMPLEIOERRORREQUEST_HXX

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
  * @see css::ucb::InteractiveIOException
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
    SimpleIOErrorRequest( const css::ucb::IOErrorCode eError,
                          const css::uno::Sequence< css::uno::Any > & rArgs,
                          const OUString & rMessage,
                          const css::uno::Reference< css::ucb::XCommandProcessor > & xContext
                         );
};

} // namespace ucbhelper

#endif /* ! INCLUDED_UCBHELPER_SIMPLEIOERRORREQUEST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
