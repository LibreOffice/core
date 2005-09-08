/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interact.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:58:38 $
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

#if !defined INCLUDED_STOC_JAVAVM_INTERACT_HXX
#define INCLUDED_STOC_JAVAVM_INTERACT_HXX

#include "com/sun/star/task/XInteractionRequest.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "cppuhelper/implbase1.hxx"
#include "rtl/ref.hxx"

namespace com { namespace sun { namespace star { namespace task {
    class XInteractionContinuation;
} } } }

namespace stoc_javavm {

class InteractionRequest:
    public cppu::WeakImplHelper1< com::sun::star::task::XInteractionRequest >
{
public:
    explicit InteractionRequest(com::sun::star::uno::Any const & rRequest);

    virtual com::sun::star::uno::Any SAL_CALL getRequest()
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionContinuation > > SAL_CALL
    getContinuations() throw (com::sun::star::uno::RuntimeException);

    bool retry() const;

private:
    class RetryContinuation;

    InteractionRequest(InteractionRequest &); // not implemented
    void operator =(InteractionRequest); // not implemented

    virtual ~InteractionRequest();

    com::sun::star::uno::Any m_aRequest;
    com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionContinuation > > m_aContinuations;
    rtl::Reference< RetryContinuation > m_xRetryContinuation;
};

}

#endif // INCLUDED_STOC_JAVAVM_INTERACT_HXX
