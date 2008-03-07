/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: soundhandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:57:50 $
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

#ifndef __FRAMEWORK_HANDLER_SOUNDHANDLER_HXX_
#define __FRAMEWORK_HANDLER_SOUNDHANDLER_HXX_

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEXTENDEDFILTERDETECTION_HPP_
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#endif

#ifndef _COM_SUN_STAR_MEDIA_XPLAYER_HPP_
#include <com/sun/star/media/XPlayer.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#include <vcl/timer.hxx>
#include <tools/link.hxx>
#include <avmedia/mediawindow.hxx>
#include <vos/mutex.hxx>

namespace css = ::com::sun::star;

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace avmedia{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

struct ThreadHelpBase
{
    public:
        mutable ::vos::OMutex m_aLock;
};

/*-************************************************************************************************************//**
    @short          handler to detect and play sounds ("wav" and "au" only!)
    @descr          Register this implementation as a content handler to detect and/or play wav- and au-sounds.
                    It doesn't depend from the target platform. But one instance of this class
                    can play one sound at the same time only. Means every new dispatch request will stop the
                    might still running one. So we support one operation/one URL/one listener at the same time
                    only.

    @devstatus      ready
    @threadsafe     yes
*//*-*************************************************************************************************************/
class SoundHandler  :   // interfaces
                        public  css::lang::XTypeProvider
                    ,   public  css::lang::XServiceInfo
                    ,   public  css::frame::XNotifyingDispatch // => XDispatch
                    ,   public  css::document::XExtendedFilterDetection
                        // baseclasses
                        // Order is neccessary for right initialization!
                    ,   private ThreadHelpBase
                    ,   public  ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------
                 SoundHandler( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        virtual ~SoundHandler(                                                                        );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Any  SAL_CALL queryInterface( const css::uno::Type& aType   ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();
        virtual css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes () throw( css::uno::RuntimeException );
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw( css::uno::RuntimeException );


    /* interface XServiceInfo */
       virtual ::rtl::OUString                                        SAL_CALL getImplementationName              (                                                                               ) throw( css::uno::RuntimeException );
       virtual sal_Bool                                               SAL_CALL supportsService                    ( const ::rtl::OUString&                                        sServiceName    ) throw( css::uno::RuntimeException );
       virtual css::uno::Sequence< ::rtl::OUString >                  SAL_CALL getSupportedServiceNames           (                                                                               ) throw( css::uno::RuntimeException );
    /* Helper for XServiceInfo */
       static css::uno::Sequence< ::rtl::OUString >                   SAL_CALL impl_getStaticSupportedServiceNames(                                                                               );
       static ::rtl::OUString                                         SAL_CALL impl_getStaticImplementationName   (                                                                               );
    /* Helper for registry */
       static css::uno::Reference< css::uno::XInterface >             SAL_CALL impl_createInstance                ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager ) throw( css::uno::Exception );
       static css::uno::Reference< css::lang::XSingleServiceFactory > SAL_CALL impl_createFactory                 ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );
    /* Helper for initialization of service by using own reference! */
       virtual void                                                   SAL_CALL impl_initService                   (                                                                               );

        //---------------------------------------------------------------------------------------------------------
        //  XNotifyingDispatch
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                       const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                       const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw(css::uno::RuntimeException);

        //---------------------------------------------------------------------------------------------------------
        //  XDispatch
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL dispatch              (   const   css::util::URL&                                     aURL        ,
                                                        const   css::uno::Sequence< css::beans::PropertyValue >&    lArguments  ) throw( css::uno::RuntimeException );
        // not supported !
        virtual void SAL_CALL addStatusListener     (   const   css::uno::Reference< css::frame::XStatusListener >& /*xListener*/   ,
                                                        const   css::util::URL&                                     /*aURL*/        ) throw( css::uno::RuntimeException ) {};
        virtual void SAL_CALL removeStatusListener  (   const   css::uno::Reference< css::frame::XStatusListener >& /*xListener*/   ,
                                                        const   css::util::URL&                                     /*aURL*/        ) throw( css::uno::RuntimeException ) {};

        //---------------------------------------------------------------------------------------------------------
        //  XExtendedFilterDetection
        //---------------------------------------------------------------------------------------------------------
        virtual ::rtl::OUString SAL_CALL detect     (           css::uno::Sequence< css::beans::PropertyValue >&    lDescriptor ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:
        DECL_LINK( implts_PlayerNotify, void* );

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    private:

        bool m_bError;
        css::uno::Reference< css::lang::XMultiServiceFactory >     m_xFactory          ;   /// global uno service factory to create new services
        css::uno::Reference< css::uno::XInterface >                m_xSelfHold         ;   /// we must protect us against dieing during async(!) dispatch() call!
        css::uno::Reference< css::media::XPlayer >                 m_xPlayer           ;   /// uses avmedia player to play sounds ...

        css::uno::Reference< css::frame::XDispatchResultListener > m_xListener         ;
        Timer m_aUpdateTimer;

};      //  class SoundHandler

}       //  namespace avmedia

#endif  //  #ifndef __FRAMEWORK_HANDLER_SOUNDHANDLER_HXX_
