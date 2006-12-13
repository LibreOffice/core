/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: popupmenudispatcher.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:03:56 $
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

#ifndef __FRAMEWORK_DISPATCH_POPUPMENUDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_POPUPMENUDISPATCHER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
#include <services/frame.hxx>
#endif
/*
#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif
*/
#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif
#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif
#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif
/*
#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif
*/
#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif
#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif
#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADER_HPP_
#include <com/sun/star/frame/XFrameLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLOADEVENTLISTENER_HPP_
#include <com/sun/star/frame/XLoadEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XURLREFERENCEFACTORY_HPP_
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XURLREFERENCE_HPP_
#include <com/sun/star/uri/XUriReference.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{


//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    We must save informations about our listener and URL for listening.
    We implement this as a hashtable for strings.
*//*-*************************************************************************************************************/

typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<  ::rtl::OUString         ,
                                                        OUStringHashCode        ,
                                                        std::equal_to< ::rtl::OUString > > IMPL_ListenerHashContainer;


/*-************************************************************************************************************//**
    @short          helper for desktop only(!) to create new tasks on demand for dispatches
    @descr          Use this class as member only! Never use it as baseclass.
                    XInterface will be ambigous and we hold a weakcss::uno::Reference to ouer OWNER - not to our SUPERCLASS!

    @implements     XInterface
                    XDispatch
                    XLoadEventListener
                    XFrameActionListener
                    XEventListener
    @base           ThreadHelpBase
                    OWeakObject

    @devstatus      ready to use
*//*-*************************************************************************************************************/
class PopupMenuDispatcher :   // interfaces
                                public css::lang::XTypeProvider         ,
                                public css::lang::XServiceInfo          ,
                                public css::frame::XDispatchProvider    ,
                                public css::frame::XDispatch            ,
                                public css::frame::XFrameActionListener ,
                                public css::lang::XInitialization       ,
                                // baseclasses
                                // Order is neccessary for right initialization!
                                public ThreadHelpBase                   ,
                                public cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        //  constructor / destructor
        PopupMenuDispatcher( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArguments ) throw( css::uno::Exception       ,
                                                                                                         css::uno::RuntimeException);
        // XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch(
            const ::com::sun::star::util::URL&  aURL        ,
            const ::rtl::OUString& sTarget     ,
            sal_Int32              nFlags      )
        throw( ::com::sun::star::uno::RuntimeException );

        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches(
            const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor )
        throw( css::uno::RuntimeException );

        //  XDispatch
        virtual void SAL_CALL dispatch( const css::util::URL&                                  aURL,
                                        const css::uno::Sequence< css::beans::PropertyValue >& seqProperties ) throw( css::uno::RuntimeException );

        virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl,
                                                 const css::util::URL&                                     aURL ) throw( css::uno::RuntimeException );

        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl,
                                                    const css::util::URL&                                     aURL  ) throw( css::uno::RuntimeException );


        //   XFrameActionListener
        virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& aEvent ) throw ( css::uno::RuntimeException );

        //   XEventListener
        void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

    //  protected methods
    protected:
        virtual ~PopupMenuDispatcher();

        void impl_RetrievePopupControllerQuery();
        void impl_CreateUriRefFactory();

    //  private methods

    //  variables
    private:
        css::uno::WeakReference< css::frame::XFrame >           m_xWeakFrame        ;   /// css::uno::WeakReference to frame (Don't use a hard css::uno::Reference. Owner can't delete us then!)
        css::uno::Reference< css::container::XNameAccess >      m_xPopupCtrlQuery   ;   /// reference to query for popup controller
        css::uno::Reference< css::uri::XUriReferenceFactory >   m_xUriRefFactory    ;   /// reference to the uri reference factory
        css::uno::Reference< css::lang::XMultiServiceFactory >  m_xFactory          ;   /// factory shared with our owner to create new services!
        IMPL_ListenerHashContainer                              m_aListenerContainer;   /// hash table for listener at specified URLs
        sal_Bool                                                m_bAlreadyDisposed  ;   /// Protection against multiple disposing calls.
        sal_Bool                                                m_bActivateListener ;   /// dispatcher is listener for frame activation

}; //  class PopupMenuDispatcher

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_DISPATCH_POPUPMENUDISPATCHER_HXX_
