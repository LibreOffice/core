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

#include <comphelper/interaction.hxx>

#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <osl/diagnose.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace dp_misc {
namespace {


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
            cppu::UnoType<task::XInteractionContinuation>::get().isAssignableFrom(m_type) ); }

    // XInterface
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;
    virtual Any SAL_CALL queryInterface( Type const & type )
        throw (RuntimeException, std::exception) override;

    // XInteractionContinuation
    virtual void SAL_CALL select() throw (RuntimeException, std::exception) override;
};

// XInterface

void InteractionContinuationImpl::acquire() throw ()
{
    OWeakObject::acquire();
}


void InteractionContinuationImpl::release() throw ()
{
    OWeakObject::release();
}


Any InteractionContinuationImpl::queryInterface( Type const & type )
    throw (RuntimeException, std::exception)
{
    if (type.isAssignableFrom( m_type )) {
        Reference<task::XInteractionContinuation> xThis(this);
        return Any( &xThis, type );
    }
    else
        return OWeakObject::queryInterface(type);
}

// XInteractionContinuation

void InteractionContinuationImpl::select() throw (RuntimeException, std::exception)
{
    *m_pselect = true;
}

} // anon namespace


bool interactContinuation( Any const & request,
                           Type const & continuation,
                           Reference<XCommandEnvironment> const & xCmdEnv,
                           bool * pcont, bool * pabort )
{
    OSL_ASSERT(
        cppu::UnoType<task::XInteractionContinuation>::get().isAssignableFrom(
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
                cppu::UnoType<task::XInteractionAbort>::get(), &abort );
            xInteractionHandler->handle(
                new ::comphelper::OInteractionRequest( request, conts ) );
            if (cont || abort) {
                if (pcont != nullptr)
                    *pcont = cont;
                if (pabort != nullptr)
                    *pabort = abort;
                return true;
            }
        }
    }
    return false;
}

// XAbortChannel

void AbortChannel::sendAbort() throw (RuntimeException, std::exception)
{
    m_aborted = true;
    if (m_xNext.is())
        m_xNext->sendAbort();
}

} // dp_misc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
