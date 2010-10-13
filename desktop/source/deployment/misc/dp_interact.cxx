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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_interact.h"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/task/XInteractionAbort.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

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
