/*************************************************************************
 *
 *  $RCSfile: dp_interact.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:07:46 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "dp_interact.h"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_misc
{

//==============================================================================
class InteractionContinuationImpl : public ::cppu::OWeakObject,
                                    public task::XInteractionContinuation
{
    Type m_type;
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

//==============================================================================
bool interactContinuation( Any const & request,
                           Type const & continuation,
                           Reference<XCommandEnvironment> const & xCmdEnv,
                           bool * pabort )
{
    OSL_ASSERT(
        task::XInteractionContinuation::static_type().isAssignableFrom(
            continuation ) );
    bool cont = false;
    bool abort = false;
    if (xCmdEnv.is()) {
        Reference<task::XInteractionHandler> xInteractionHandler(
            xCmdEnv->getInteractionHandler() );
        if (xInteractionHandler.is()) {
            Sequence< Reference<task::XInteractionContinuation> > conts( 2 );
            conts[ 0 ] = new InteractionContinuationImpl(
                task::XInteractionAbort::static_type(), &abort );
            conts[ 1 ] = new InteractionContinuationImpl(
                continuation, &cont );
            xInteractionHandler->handle(
                new InteractionRequest( request, conts ) );
        }
    }
    if (pabort != 0)
        *pabort = abort;
    return cont;
}

//==============================================================================
void interactContinuation_throw(
    deployment::DeploymentException const & exc,
    Type const & continuation,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    OSL_ASSERT( exc.Cause.getValueTypeClass() == TypeClass_EXCEPTION );

    RuntimeException rt_exc;
    if (exc.Cause >>= rt_exc) {
        OSL_ENSURE( 0, "### missing RuntimeException rethrow?" );
        ::cppu::throwException( exc.Cause );
    }
    deployment::DeploymentException depl_exc;
    if (exc.Cause >>= depl_exc) {
        OSL_ENSURE( 0, "### missing DeploymentException rethrow?" );
        ::cppu::throwException( exc.Cause );
    }
    CommandFailedException cf_exc;
    if (exc.Cause >>= cf_exc) {
        OSL_ENSURE( 0, "### missing CommandFailedException rethrow?" );
        ::cppu::throwException( exc.Cause );
    }

    bool abort = false;
    if (interactContinuation( exc.Cause, continuation, xCmdEnv, &abort )) {
        OSL_ASSERT( ! abort );
        return;
    }
    if (abort)
        throw CommandFailedException( exc.Message, exc.Context, exc.Cause );
    else
        throw exc;
}

// XAbortChannel
//______________________________________________________________________________
void AbortChannel::sendAbort() throw (RuntimeException)
{
    m_aborted = true;
    if (m_xNext.is())
        m_xNext->sendAbort();
}

}

