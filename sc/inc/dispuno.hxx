/*************************************************************************
 *
 *  $RCSfile: dispuno.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-13 19:21:34 $
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

#ifndef SC_DISPUNO_HXX
#define SC_DISPUNO_HXX

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

namespace com { namespace sun { namespace star { namespace frame {
    class XDispatchProviderInterception;
} } } }

class ScTabViewShell;


class ScDispatchProviderInterceptor : public cppu::WeakImplHelper2<
                                        com::sun::star::frame::XDispatchProviderInterceptor,
                                        com::sun::star::lang::XEventListener>
{
    ScTabViewShell*     pViewShell;

    // the component which's dispatches we're intercepting
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProviderInterception> m_xIntercepted;

    // chaining
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProvider> m_xSlaveDispatcher;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProvider> m_xMasterDispatcher;

    // own dispatch
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatch> m_xMyDispatch;

public:

                            ScDispatchProviderInterceptor(ScTabViewShell* pViewSh);
    virtual                 ~ScDispatchProviderInterceptor();

                            // XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL
                            queryDispatch( const ::com::sun::star::util::URL& aURL,
                                        const ::rtl::OUString& aTargetFrameName,
                                        sal_Int32 nSearchFlags )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
                                        ::com::sun::star::frame::XDispatch > > SAL_CALL
                            queryDispatches( const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::frame::DispatchDescriptor >& aDescripts )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
                            getSlaveDispatchProvider() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setSlaveDispatchProvider( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XDispatchProvider >& xNewDispatchProvider )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
                            getMasterDispatchProvider() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setMasterDispatchProvider( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XDispatchProvider >& xNewSupplier )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XEventListener
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& Source )
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScDispatch : public cppu::WeakImplHelper1<com::sun::star::frame::XDispatch>
{
    ScTabViewShell*     pViewShell;

public:

                            ScDispatch(ScTabViewShell* pViewSh);
    virtual                 ~ScDispatch();

                            // XDispatch
    virtual void SAL_CALL   dispatch( const ::com::sun::star::util::URL& aURL,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& aArgs )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addStatusListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::frame::XStatusListener >& xControl,
                                const ::com::sun::star::util::URL& aURL )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeStatusListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::frame::XStatusListener >& xControl,
                                const ::com::sun::star::util::URL& aURL )
                                throw(::com::sun::star::uno::RuntimeException);
};


#endif

