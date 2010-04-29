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

#ifndef __FRAMEWORK_HELPER_POPUPMENUCONTROLLERBASE_HXX_
#define __FRAMEWORK_HELPER_POPUPMENUCONTROLLERBASE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
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
//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <fwidllapi.h>

namespace framework
{
    class PopupMenuControllerBase :  public com::sun::star::lang::XTypeProvider             ,
                                     public com::sun::star::lang::XServiceInfo            ,
                                     public ::com::sun::star::frame::XPopupMenuController ,
                                     public com::sun::star::lang::XInitialization         ,
                                     public com::sun::star::frame::XStatusListener        ,
                                     public com::sun::star::awt::XMenuListener            ,
                                     public com::sun::star::frame::XDispatchProvider      ,
                                     public com::sun::star::frame::XDispatch              ,
                                     public com::sun::star::lang::XComponent              ,
                                     protected ThreadHelpBase                             , // Struct for right initalization of mutex member! Must be first of baseclasses.
                                     public ::cppu::OBroadcastHelper                      ,
                                     public ::cppu::OWeakObject
    {
        public:
            FWI_DLLPUBLIC PopupMenuControllerBase( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            FWI_DLLPUBLIC virtual ~PopupMenuControllerBase();

            //  XInterface
            virtual FWI_DLLPUBLIC ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw( ::com::sun::star::uno::RuntimeException );
            virtual FWI_DLLPUBLIC void SAL_CALL acquire() throw();
            virtual FWI_DLLPUBLIC void SAL_CALL release() throw();

            // XTypeProvider
            virtual FWI_DLLPUBLIC ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw( ::com::sun::star::uno::RuntimeException );
            virtual FWI_DLLPUBLIC ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw( ::com::sun::star::uno::RuntimeException );

            // XServiceInfo
            virtual FWI_DLLPUBLIC ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
            virtual FWI_DLLPUBLIC sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException) = 0;
            virtual FWI_DLLPUBLIC ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException) = 0;

            // XPopupMenuController
            virtual FWI_DLLPUBLIC void SAL_CALL setPopupMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& PopupMenu ) throw (::com::sun::star::uno::RuntimeException);
            virtual FWI_DLLPUBLIC void SAL_CALL updatePopupMenu() throw (::com::sun::star::uno::RuntimeException);

            // XInitialization
            virtual FWI_DLLPUBLIC void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

            // XStatusListener
            virtual FWI_DLLPUBLIC void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException ) = 0;

            // XMenuListener
            virtual FWI_DLLPUBLIC void SAL_CALL highlight( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual FWI_DLLPUBLIC void SAL_CALL select( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual FWI_DLLPUBLIC void SAL_CALL activate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual FWI_DLLPUBLIC void SAL_CALL deactivate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

            // XDispatchProvider
            virtual FWI_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& sTarget, sal_Int32 nFlags ) throw( ::com::sun::star::uno::RuntimeException );
            virtual FWI_DLLPUBLIC ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException );

            // XDispatch
            virtual FWI_DLLPUBLIC void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& seqProperties ) throw( ::com::sun::star::uno::RuntimeException );
            virtual FWI_DLLPUBLIC void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw( ::com::sun::star::uno::RuntimeException );
            virtual FWI_DLLPUBLIC void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw( ::com::sun::star::uno::RuntimeException );

            // XEventListener
            virtual FWI_DLLPUBLIC void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

            // XComponent
            virtual FWI_DLLPUBLIC void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
            virtual FWI_DLLPUBLIC void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
            virtual FWI_DLLPUBLIC void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        protected:
            virtual FWI_DLLPUBLIC void resetPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );
            virtual FWI_DLLPUBLIC void impl_setPopupMenu();
            virtual FWI_DLLPUBLIC void impl_select(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& _xDispatch,const ::com::sun::star::util::URL& aURL);
            FWI_DLLPUBLIC ::rtl::OUString determineBaseURL( const ::rtl::OUString& aURL );


            bool                                                                             m_bInitialized;
            bool                                                                             m_bDisposed;
            rtl::OUString                                                                    m_aCommandURL;
            rtl::OUString                                                                    m_aBaseURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >           m_xDispatch;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >              m_xFrame;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >      m_xURLTransformer;
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >            m_xPopupMenu;
            ::cppu::OMultiTypeInterfaceContainerHelper                                       m_aListenerContainer; // container for ALL Listener
    };
}

#endif // __FRAMEWORK_HELPER_POPUPMENUCONTROLLERBASE_HXX_
