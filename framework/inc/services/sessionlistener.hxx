/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sessionlistener.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:39:14 $
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

#ifndef __FRAMEWORK_SERVICES_SESSIONLISTENER_HXX_
#define __FRAMEWORK_SERVICES_SESSIONLISTENER_HXX_

//_______________________________________________
// my own includes

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#include <classes/filtercache.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ARGUMENTANALYZER_HXX_
#include <classes/argumentanalyzer.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
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

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif


#include <com/sun/star/frame/XSessionManagerListener.hpp>
#include <com/sun/star/frame/XSessionManagerClient.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

//_______________________________________________
// other includes

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// exported const

//_______________________________________________
// exported definitions

/// @HTML
/** @short  implements flat/deep detection of file/stream formats and provides
            further read/write access to the global office type configuration.

    @descr  Using of this class makes it possible to get information about the
            format type of a given URL or stream. The returned internal type name
            can be used to get more informations about this format. Further this
            class provides full access to the configuration data and following
            implementations will support some special query modes.

    @author     as96863

    @docdate    10.03.2003 by as96863

    @todo       <ul>
                    <li>implementation of query mode</li>
                    <li>simple restore mechanism of last consistent cache state,
                        if flush failed</li>
                </ul>
 */
/// @NOHTML

class SessionListener :   // interfaces
                        public css::lang::XTypeProvider,
                        public css::lang::XInitialization,
                        public css::frame::XSessionManagerListener,
                        public css::frame::XStatusListener,
                        public css::lang::XServiceInfo,
                        // baseclasses (order important for initialization!)
                        // Struct for right initalization of mutex member! Must be the first one of baseclasses!
                        private ThreadHelpBase,
                        public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // member

    private:

        /** reference to the uno service manager, which created this service.
            It can be used to create own needed helper services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        css::uno::Reference< css::frame::XSessionManagerClient > m_rSessionManager;

        // restore handling
        sal_Bool m_bRestored;

    public:

        //---------------------------------------
        // XInterface, XTypeProvider, XServiceInfo

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        #ifdef ENABLE_AUTODOC_FIX
        ;
        #endif

        //---------------------------------------

        /** @short  initialize new instance of this class.

            @param  xSMGR
                    reference to the global uno service manager, which created this new
                    factory instance. It must be used during runtime to create own
                    needed services.
         */

        SessionListener( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );

        virtual ~SessionListener();

        virtual void SAL_CALL disposing(const com::sun::star::lang::EventObject&) throw (css::uno::RuntimeException);


        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any  >& args) throw (css::uno::RuntimeException);

        // XSessionManagerListener
        virtual void SAL_CALL doSave( sal_Bool bShutdown, sal_Bool bCancelable )
            throw (css::uno::RuntimeException);
        virtual void SAL_CALL approveInteraction( sal_Bool bInteractionGranted )
            throw (css::uno::RuntimeException);
       virtual void SAL_CALL shutdownCanceled()
            throw (css::uno::RuntimeException);
       virtual sal_Bool SAL_CALL doRestore()
            throw (css::uno::RuntimeException);

       // XStatusListener
       virtual void SAL_CALL statusChanged(const com::sun::star::frame::FeatureStateEvent& event)
           throw (css::uno::RuntimeException);

        void doSaveImpl( sal_Bool bShutdown, sal_Bool bCancelable ) throw (css::uno::RuntimeException);
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_SERVICES_TYPEDETECTION_HXX_
