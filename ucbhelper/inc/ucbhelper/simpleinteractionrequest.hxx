/*************************************************************************
 *
 *  $RCSfile: simpleinteractionrequest.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2001-05-28 10:42:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX
#define _UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX

#ifndef _UCBHELPER_INTERATIONREQUEST_HXX
#include <ucbhelper/interactionrequest.hxx>
#endif

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
class SimpleInteractionRequest : public ucbhelper::InteractionRequest
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
    const sal_Int32 getResponse() const;
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX */
