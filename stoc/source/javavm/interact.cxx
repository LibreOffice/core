/*************************************************************************
 *
 *  $RCSfile: interact.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-01 09:03:13 $
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

#include "interact.hxx"

#include "com/sun/star/java/JavaDisabledException.hpp"
#include "com/sun/star/java/JavaVMCreationFailureException.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionRetry.hpp"
#include "com/sun/star/task/XInteractionContinuation.hpp"
#include "cppuhelper/implbase1.hxx"
#include "osl/mutex.hxx"

namespace css = com::sun::star;

using stoc_javavm::InteractionRequest;

namespace {

class AbortContinuation:
    public cppu::WeakImplHelper1< css::task::XInteractionAbort >
{
public:
    inline AbortContinuation() {}

    virtual inline void SAL_CALL select() throw (css::uno::RuntimeException) {}

private:
    AbortContinuation(AbortContinuation &); // not implemented
    void operator =(AbortContinuation); // not implemented

    virtual inline ~AbortContinuation() {}
};

}

class InteractionRequest::RetryContinuation:
    public cppu::WeakImplHelper1< css::task::XInteractionRetry >
{
public:
    inline RetryContinuation(): m_bSelected(false) {}

    virtual void SAL_CALL select() throw (css::uno::RuntimeException);

    bool isSelected() const;

private:
    RetryContinuation(RetryContinuation &); // not implemented
    void operator =(RetryContinuation); // not implemented

    virtual inline ~RetryContinuation() {}

    mutable osl::Mutex m_aMutex;
    bool m_bSelected;
};

void SAL_CALL InteractionRequest::RetryContinuation::select()
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    m_bSelected = true;
}

bool InteractionRequest::RetryContinuation::isSelected() const
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_bSelected;
}

InteractionRequest::InteractionRequest(css::uno::Any const & rRequest):
    m_aRequest(rRequest)
{
    m_aContinuations.realloc(2);
    m_xRetryContinuation = new RetryContinuation;
    m_aContinuations[0] = new AbortContinuation;
    m_aContinuations[1] = m_xRetryContinuation.get();
}

css::uno::Any SAL_CALL InteractionRequest::getRequest()
    throw (css::uno::RuntimeException)
{
    return m_aRequest;
}

css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > >
SAL_CALL InteractionRequest::getContinuations()
    throw (css::uno::RuntimeException)
{
    return m_aContinuations;
}

bool InteractionRequest::retry() const
{
    return m_xRetryContinuation.is() && m_xRetryContinuation->isSelected();
}

InteractionRequest::~InteractionRequest()
{}
