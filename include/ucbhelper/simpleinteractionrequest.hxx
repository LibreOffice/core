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

#ifndef INCLUDED_UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX
#define INCLUDED_UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX

#include <ucbhelper/interactionrequest.hxx>
#include <ucbhelper/ucbhelperdllapi.h>
#include <o3tl/typed_flags_set.hxx>

/** These are the constants that can be passed to the constructor of class
  * SimpleInteractionRequest and that are returned by method
  * SimpleInteractionRequest::getResponse().
  */

enum class ContinuationFlags
{
    /** The request was not (yet) handled by the interaction handler. */
    NONE = 0,
    /** The interaction handler selected XInteractionAbort. */
    Abort = 1,
    /** The interaction handler selected XInteractionRetry. */
    Retry = 2,
    /** The interaction handler selected XInteractionApprove. */
    Approve = 4,
    /** The interaction handler selected XInteractionDisapprove. */
    Disapprove = 8,
};
namespace o3tl
{
template <> struct typed_flags<ContinuationFlags> : is_typed_flags<ContinuationFlags, 0x0f>
{
};
}

namespace ucbhelper
{
/**
  * This class implements a simple interaction request. The user must not deal
  * with XInteractionContinuations directly, but can use constants that are
  * mapped internally to the according objects. This class encapsulates the
  * standard Interaction Continuations "Abort", "Retry", "Approve" and
  * "Disapprove". Instances can be passed directly to
  * XInteractionHandler::handle(...).
  *
  * @see InteractionRequest
  * @see InteractionAbort
  * @see InteractionRetry
  * @see InteractionApprove
  * @see InteractionDisapprove
  */
class UCBHELPER_DLLPUBLIC SimpleInteractionRequest final : public ucbhelper::InteractionRequest
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
    SimpleInteractionRequest(const css::uno::Any& rRequest, const ContinuationFlags nContinuations);

    /**
      * After passing this request to XInteractionHandler::handle, this method
      * returns the continuation that was chosen by the interaction handler.
      *
      * @return the continuation chosen by an interaction handler or
      *         ContinuationFlags::NONE, if the request was not (yet) handled.
      */
    ContinuationFlags getResponse() const;
};

} // namespace ucbhelper

#endif /* ! INCLUDED_UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
