/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: popupmenucontrollerbase.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:04:10 $
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

#ifndef __FRAMEWORK_HELPER_POPUPMENUCONTROLLERBASE_HXX_
#define __FRAMEWORK_HELPER_POPUPMENUCONTROLLERBASE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif
#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif
#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif
#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif
#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif
#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XPOPUPMENUCONTROLLER_HPP_
#include <com/sun/star/frame/XPopupMenuController.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XURIREFERENCEFACTORY_HPP_
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XURIREFERENCE_HPP_
#include <com/sun/star/uri/XUriReference.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _TOOLKIT_AWT_VCLXMENU_HXX_
#include <toolkit/awt/vclxmenu.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace framework
{
    class PopupMenuControllerBase :  public com::sun::star::lang::XTypeProvider           ,
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
            PopupMenuControllerBase( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            virtual ~PopupMenuControllerBase();

            //  XInterface, XTypeProvider, XServiceInfo
            FWK_DECLARE_XINTERFACE
            FWK_DECLARE_XTYPEPROVIDER

            // XServiceInfo
            virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
            virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException) = 0;
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException) = 0;

            // XPopupMenuController
            virtual void SAL_CALL setPopupMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& PopupMenu ) throw (::com::sun::star::uno::RuntimeException) = 0;
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

            // XComponent
            virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        protected:
            virtual void resetPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );
            ::rtl::OUString determineBaseURL( const ::rtl::OUString& aURL );

            bool                                                                             m_bInitialized;
            bool                                                                             m_bDisposed;
            rtl::OUString                                                                    m_aCommandURL;
            rtl::OUString                                                                    m_aBaseURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >           m_xDispatch;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >              m_xFrame;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >            m_xPopupMenu;
            ::cppu::OMultiTypeInterfaceContainerHelper                                       m_aListenerContainer; // container for ALL Listener
    };
}

#endif // __FRAMEWORK_HELPER_POPUPMENUCONTROLLERBASE_HXX_
