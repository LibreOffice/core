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


#include "dp_interact.h"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/task/XInteractionAbort.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace dp_misc {
namespace {

//==============================================================================
class InteractionContinuationImpl : public ::cppu::OWeakObject,
                                    public task::XInteractionContinuation
{
    const Type m_type;
    bool * m_pselect;

public:
    inline InteractionContinuationImpl( Type const & type, bool * pselect )
        : m_type( type ),
          m_pselect( pselect )
        { OSL_ASSERT(
            ::getCppuType(
                static_cast< Reference<task::XInteractionContinuation>
                const *>(0) ).isAssignableFrom(m_type) ); }

    // XInterface
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();
    virtual Any SAL_CALL queryInterface( Type const & type )
        throw (RuntimeException);

    // XInteractionContinuation
    virtual void SAL_CALL select() throw (RuntimeException);
};

// XInterface
//______________________________________________________________________________
void InteractionContinuationImpl::acquire() throw ()
{
    OWeakObject::acquire();
}

//______________________________________________________________________________
void InteractionContinuationImpl::release() throw ()
{
    OWeakObject::release();
}

//______________________________________________________________________________
Any InteractionContinuationImpl::queryInterface( Type const & type )
    throw (RuntimeException)
{
    if (type.isAssignableFrom( m_type )) {
        Reference<task::XInteractionContinuation> xThis(this);
        return Any( &xThis, type );
    }
    else
        return OWeakObject::queryInterface(type);
}

// XInteractionContinuation
//______________________________________________________________________________
void InteractionContinuationImpl::select() throw (RuntimeException)
{
    *m_pselect = true;
}

//==============================================================================
class InteractionRequest :
    public ::cppu::WeakImplHelper1<task::XInteractionRequest>
{
    Any m_request;
    Sequence< Reference<task::XInteractionContinuation> > m_conts;

public:
    inline InteractionRequest(
        Any const & request,
        Sequence< Reference<task::XInteractionContinuation> > const & conts )
        : m_request( request ),
          m_conts( conts )
        {}

    // XInteractionRequest
    virtual Any SAL_CALL getRequest()
        throw (RuntimeException);
    virtual Sequence< Reference<task::XInteractionContinuation> >
    SAL_CALL getContinuations() throw (RuntimeException);
};

// XInteractionRequest
//______________________________________________________________________________
Any InteractionRequest::getRequest() throw (RuntimeException)
{
    return m_request;
}

//______________________________________________________________________________
Sequence< Reference< task::XInteractionContinuation > >
InteractionRequest::getContinuations() throw (RuntimeException)
{
    return m_conts;
}

} // anon namespace

//==============================================================================
bool interactContinuation( Any const & request,
                           Type const & continuation,
                           Reference<XCommandEnvironment> const & xCmdEnv,
                           bool * pcont, bool * pabort )
{
    OSL_ASSERT(
        task::XInteractionContinuation::static_type().isAssignableFrom(
            continuation ) );
    if (xCmdEnv.is()) {
        Reference<task::XInteractionHandler> xInteractionHandler(
            xCmdEnv->getInteractionHandler() );
        if (xInteractionHandler.is()) {
            bool cont = false;
            bool abort = false;
            Sequence< Reference<task::XInteractionContinuation> > conts( 2 );
            conts[ 0 ] = new InteractionContinuationImpl(
                continuation, &cont );
            conts[ 1 ] = new InteractionContinuationImpl(
                task::XInteractionAbort::static_type(), &abort );
            xInteractionHandler->handle(
                new InteractionRequest( request, conts ) );
            if (cont || abort) {
                if (pcont != 0)
                    *pcont = cont;
                if (pabort != 0)
                    *pabort = abort;
                return true;
            }
        }
    }
    return false;
}

// XAbortChannel
//______________________________________________________________________________
void AbortChannel::sendAbort() throw (RuntimeException)
{
    m_aborted = true;
    if (m_xNext.is())
        m_xNext->sendAbort();
}

} // dp_misc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
