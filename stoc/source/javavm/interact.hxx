/*************************************************************************
 *
 *  $RCSfile: interact.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jl $ $Date: 2002-07-23 14:07:17 $
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
#ifndef _JVM_INTERACT_HXX_
#define _JVM_INTERACT_HXX_

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include <com/sun/star/task/XInteractionRetry.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTION_ABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif

#include "javavm.hxx"

using namespace ::cppu;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;
namespace stoc_javavm
{
class InteractionAbort: public WeakImplHelper1<XInteractionAbort>
{
    // Holds the JavaVirtualMachine_Impl. Calls on XInteractionAbort::select
    // in this class are delegated to JavaVirtualMachine_Impl.
    WeakReference<XInterface> m_xJVM;
    JavaVirtualMachine_Impl *m_pJVM;
public:
    InteractionAbort( const WeakReference<XInterface>& xJVM, JavaVirtualMachine_Impl* _pJVM);
    //com.sun.star.task.XInteractionAbort
    virtual void SAL_CALL select(  ) throw (RuntimeException);
};

class InteractionRetry: public WeakImplHelper1<XInteractionRetry>
{
    // Holds the JavaVirtualMachine_Impl. Calls on XInteractionAbort::select
    // in this class are delegated to JavaVirtualMachine_Impl.
    WeakReference<XInterface> m_xJVM;
    JavaVirtualMachine_Impl *m_pJVM;
public:
    InteractionRetry( const WeakReference<XInterface>& xJVM, JavaVirtualMachine_Impl* _pJVM);
    //com.sun.star.task.XInteractionAbort
    virtual void SAL_CALL select(  ) throw (RuntimeException);
};

class InteractionRequest: public WeakImplHelper1<XInteractionRequest>
{
    // Holds the JavaVirtualMachine_Impl. Calls on XInteractionAbort::select
    // in this class are delegated to JavaVirtualMachine_Impl.
    WeakReference<XInterface> m_xJVM;
    JavaVirtualMachine_Impl *m_pJVM;
    Any m_anyRequest;
    Sequence< Reference<XInteractionContinuation> > m_seqContinuations;
    Sequence< Reference<XInteractionContinuation> > * m_pseqContinuations;

public:
    InteractionRequest( const Reference<XInterface>& xJVM,
                        JavaVirtualMachine_Impl* _pJVM, Any& _request);
    virtual Any SAL_CALL getRequest(  ) throw (RuntimeException);
    virtual Sequence< Reference< XInteractionContinuation > >
    SAL_CALL getContinuations(  ) throw (RuntimeException);
};

} // end namespace
#endif

