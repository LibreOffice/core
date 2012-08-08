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

#ifndef __SVTOOLS_POPUPMENUCONTROLLERBASE_HXX_
#define __SVTOOLS_POPUPMENUCONTROLLERBASE_HXX_

#include "svtools/svtdllapi.h"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/compbase7.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace svt
{
    struct PopupMenuControllerBaseDispatchInfo;

    typedef ::cppu::WeakComponentImplHelper7<
                        com::sun::star::lang::XServiceInfo            ,
                        com::sun::star::frame::XPopupMenuController ,
                        com::sun::star::lang::XInitialization         ,
                        com::sun::star::frame::XStatusListener        ,
                        com::sun::star::awt::XMenuListener            ,
                        com::sun::star::frame::XDispatchProvider      ,
                        com::sun::star::frame::XDispatch > PopupMenuControllerBaseType;

    class SVT_DLLPUBLIC PopupMenuControllerBase : protected ::comphelper::OBaseMutex,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                                  public PopupMenuControllerBaseType
    {
        public:
            PopupMenuControllerBase( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            virtual ~PopupMenuControllerBase();

            // XServiceInfo
            virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
            virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException) = 0;

            // XPopupMenuController
            virtual void SAL_CALL setPopupMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& PopupMenu ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updatePopupMenu() throw (::com::sun::star::uno::RuntimeException);

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

            // XStatusListener
            virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException ) = 0;

            // XMenuListener
            virtual void SAL_CALL highlight( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL select( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL activate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL deactivate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

            // XDispatchProvider
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& sTarget, sal_Int32 nFlags ) throw( ::com::sun::star::uno::RuntimeException );
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException );

            // XDispatch
            virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& seqProperties ) throw( ::com::sun::star::uno::RuntimeException );
            virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw( ::com::sun::star::uno::RuntimeException );
            virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw( ::com::sun::star::uno::RuntimeException );

            // XEventListener
            virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

            void dispatchCommand( const ::rtl::OUString& sCommandURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs );

    protected:
            void throwIfDisposed() throw ( ::com::sun::star::uno::RuntimeException );

            /** helper method to cause statusChanged is called once for the given command url */
            void SAL_CALL updateCommand( const rtl::OUString& rCommandURL );

            /** this function is called upon disposing the component
            */
            virtual void SAL_CALL disposing();

            virtual void resetPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );
            virtual void impl_setPopupMenu();
            virtual void impl_select(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& _xDispatch,const ::com::sun::star::util::URL& aURL);
            ::rtl::OUString determineBaseURL( const ::rtl::OUString& aURL );

            DECL_STATIC_LINK( PopupMenuControllerBase, ExecuteHdl_Impl, PopupMenuControllerBaseDispatchInfo* );


            bool                                                                             m_bInitialized;
            rtl::OUString                                                                    m_aCommandURL;
            rtl::OUString                                                                    m_aBaseURL;
            rtl::OUString                                                                    m_aModuleName;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >           m_xDispatch;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >              m_xFrame;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >      m_xURLTransformer;
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >            m_xPopupMenu;
    };
}

#endif // __SVTOOLS_POPUPMENUCONTROLLERBASE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
