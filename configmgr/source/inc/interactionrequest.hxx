/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interactionrequest.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:49:04 $
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

#ifndef CONFIGMGR_INTERACTIONREQUEST_HXX
#define CONFIGMGR_INTERACTIONREQUEST_HXX

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include <com/sun/star/task/XInteractionRetry.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONAPPROVE_HPP_
#include <com/sun/star/task/XInteractionApprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONDISAPPROVE_HPP_
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

namespace configmgr {
namespace apihelper {
    namespace uno  = com::sun::star::uno;
    namespace task = com::sun::star::task;
//============================================================================

/**
  * This class implements the interface XInteractionRequest. Instances can
  * be passed directly to XInteractionHandler::handle(...). Each interaction
  * request contains an exception describing the error and a number of
  * interaction continuations describing the possible "answers" for the request.
  * After the request was passed to XInteractionHandler::handle(...) the method
  * getSelection() returns the continuation choosen by the interaction handler.
  *
  * The typical usage of this class would be:
  *
  * 1) Create exception object that shall be handled by the interaction handler.
  * 2) Create InteractionRequest, supply exception as ctor parameter
  * 3) Create continuations needed and add them to a sequence
  * 4) Supply the continuations to the InteractionRequest by calling
  *    setContinuations(...)
  *
  * This class can also be used as base class for more specialized requests,
  * like authentication requests.
  */
class InteractionRequest : public cppu::WeakImplHelper1<com::sun::star::task::XInteractionRequest>
{
    struct Impl;
    Impl * m_pImpl;

protected:
    InteractionRequest();
    virtual ~InteractionRequest();

    void setRequest( const com::sun::star::uno::Any & rRequest );
public:
    /**
      * Constructor.
      *
      * @param rRequest is the exception describing the error.
      */
    InteractionRequest( const com::sun::star::uno::Any & rRequest );

    /**
      * This method sets the continuations for the request.
      *
      * @param rContinuations contains the possible continuations.
      */
    void setContinuations(
        const uno::Sequence< uno::Reference< task::XInteractionContinuation > > &
                    rContinuations );

    // XInteractionRequest
    virtual uno::Any SAL_CALL
        getRequest()
            throw( uno::RuntimeException );

    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL
        getContinuations()
            throw( com::sun::star::uno::RuntimeException );

    // Non-interface methods.

    /**
      * After passing this request to XInteractionHandler::handle, this method
      * returns the continuation that was choosen by the interaction handler.
      *
      * @return the continuation choosen by an interaction handler or an empty
      *         reference, if the request was not (yet) handled.
      */
    uno::Reference< task::XInteractionContinuation > getSelection() const;

    /**
      * This method sets a continuation for the request. It also can be used
      * to reset the continuation set by a previous XInteractionHandler::handle
      * call in order to use this request object more then once.
      *
      * @param rxSelection is the interaction continuation to activate for
      *        the request or an empty reference in order to reset the
      *        current selection.
      */
    void setSelection(
        const uno::Reference< task::XInteractionContinuation > & rxSelection );
};

//============================================================================

/**
  * This template class  implements a simple standard interaction continuation
  * interface provided as template parameter. Classes instantiated from this
  * template work together with class InteractionRequest.
  * Instances of such a class can be passed along with an interaction request
  * to indicate the possiblity to continue the operation that caused the request
  * as indicated by the interface.
  */
template <class XThisContinuation >
class InteractionContinuation   : public cppu::WeakImplHelper1< XThisContinuation >
{
    InteractionRequest * m_pRequest;
public:
    InteractionContinuation( InteractionRequest * pRequest )
    : m_pRequest( pRequest ) {}

    // XInteractionContinuation
    /**
      * This method marks this continuation as "selected" at the request it
      * belongs to.
      *
      * Derived classes must implement their XInteractionContinuation::select()
      * method the way that they call this method.
      */
    virtual void SAL_CALL select()
        throw( com::sun::star::uno::RuntimeException );
};
//============================================================================

template <class XThisContinuation >
void SAL_CALL InteractionContinuation< XThisContinuation >::select()
    throw( com::sun::star::uno::RuntimeException )
{
    m_pRequest->setSelection(this);
}
//============================================================================

typedef InteractionContinuation< task::XInteractionAbort > InteractionAbort;
typedef InteractionContinuation< task::XInteractionRetry > InteractionRetry;
typedef InteractionContinuation< task::XInteractionApprove > InteractionApprove;
typedef InteractionContinuation< task::XInteractionDisapprove > InteractionDisapprove;
//============================================================================


} // namespace apihelper
} // namespace configmgr

#endif /* !CONFIGMGR_INTERACTIONREQUEST_HXX */
