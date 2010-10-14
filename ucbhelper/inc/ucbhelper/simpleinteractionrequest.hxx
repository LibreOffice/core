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

#ifndef _UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX
#define _UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX

#include <ucbhelper/interactionrequest.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace ucbhelper {

/** These are the constants that can be passed to the constructor of class
  * SimpleInteractionRequest and that are returned by method
  * SimpleInteractionRequest::getResponse().
  */

/** The request was not (yet) handled by the interaction handler. */
static const sal_Int32 CONTINUATION_UNKNOWN    = 0;

/** The interaction handler selected XInteractionAbort. */
static const sal_Int32 CONTINUATION_ABORT      = 1;

/** The interaction handler selected XInteractionRetry. */
static const sal_Int32 CONTINUATION_RETRY      = 2;

/** The interaction handler selected XInteractionApprove. */
static const sal_Int32 CONTINUATION_APPROVE    = 4;

/** The interaction handler selected XInteractionDisapprove. */
static const sal_Int32 CONTINUATION_DISAPPROVE = 8;

/**
  * This class implements a simple interaction request. The user must not deal
  * with XInteractionContinuations directly, but can use constants that are
  * mapped internally to the according objects. This class encapsulates the
  * standard Interaction Continuations "Abort", "Retry", "Approve" and
  * "Disaprrove". Instances can be passed directly to
  * XInteractionHandler::handle(...).
  *
  * @see InteractionRequest
  * @see InteractionAbort
  * @see InteractionRetry
  * @see InteractionApprove
  * @see InteractionDisapprove
  */
class UCBHELPER_DLLPUBLIC SimpleInteractionRequest : public ucbhelper::InteractionRequest
{
public:
    /**
      * Constructor.
      *
      * @param rRequest is the exception describing the error.
      * @param nContinuations contains the possible "answers" for the request.
      *        This can be any of the CONTINUATION_* constants combinations
      *        listed above.
      */
    SimpleInteractionRequest( const com::sun::star::uno::Any & rRequest,
                              const sal_Int32 nContinuations );

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

#endif /* !_UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
