/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gconfbackend.hxx,v $
 * $Revision: 1.8 $
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

#ifndef GCONFBACKEND_HXX_
#define GCONFBACKEND_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

//#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
//#include <com/sun/star/lang/XMultiServiceFactory.hpp>
//#endif // _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#include <cppuhelper/compbase3.hxx>

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif
//#ifndef _VOS_THREAD_HXX_
//#include <vos/thread.hxx>
//#endif

#include <gconf/gconf-client.h>


namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;


/** Structure containing the mapping between OOffice and Gconf keys.
    AlOO specifies whether the key is protected, if key is protected it
    can not be over riden in subsequent higher layers
*/
struct keyMapping
{
    keyMapping(){};
    rtl::OUString mOOName;
    rtl::OUString mOOType;
    rtl::OUString mGconfName;
    sal_Bool mbProtected;
};

typedef keyMapping KeyMappingInfo;
typedef std::multimap<rtl::OUString, KeyMappingInfo> KeyMappingTable;

/*Time Stamp mapping table used to store timestamps of updated components
  when a notification is recieved. It is needed as you cannot access gconf key
  timestamps via the Gconf api */

typedef std::multimap<rtl::OUString, rtl::OUString>  TSMappingTable;

//------------------------------------------------------------------------------

/*
class ONotificationThread: public vos::OThread
{


public:
    ONotificationThread()
    {}

    ~ONotificationThread()
    {
        g_main_loop_quit(mLoop);
    }

private:
    virtual void SAL_CALL onTerminated()
    {
        delete this;
    }

    virtual void SAL_CALL run();
    GMainLoop* mLoop;
};

*/

//------------------------------------------------------------------------------
typedef cppu::WeakComponentImplHelper3<backend::XSingleLayerStratum,
                                       backend::XBackendChangesNotifier,
                       lang::XServiceInfo> BackendBase ;

/**
  Implements the SingleLayerStratum service for gconf access.
  */
class GconfBackend : public BackendBase {
    public :

    static GconfBackend* createInstance(const uno::Reference<uno::XComponentContext>& xContext);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName(  )
            throw (uno::RuntimeException) ;

        virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& aServiceName )
            throw (uno::RuntimeException) ;

        virtual uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames(  )
            throw (uno::RuntimeException) ;

        /**
          Provides the implementation name.

          @return   implementation name
          */
        static rtl::OUString SAL_CALL getBackendName(void) ;

        /**
          Provides the supported services names

          @return   service names
          */
        static uno::Sequence<rtl::OUString> SAL_CALL getBackendServiceNames(void) ;

        /* returns a GconfClient */
        static GConfClient* getGconfClient();

        //XSingleLayerStratum
        virtual uno::Reference<backend::XLayer> SAL_CALL
            getLayer( const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp )
                throw (backend::BackendAccessException, lang::IllegalArgumentException) ;

        virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL
            getUpdatableLayer( const rtl::OUString& aLayerId )
                throw (backend::BackendAccessException, lang::NoSupportException,
                       lang::IllegalArgumentException) ;

        // XBackendChangesNotifier
        virtual void SAL_CALL addChangesListener(
            const uno::Reference<backend::XBackendChangesListener>& xListener,
            const rtl::OUString& aComponent)
                throw (::com::sun::star::uno::RuntimeException);


        virtual void SAL_CALL removeChangesListener(
            const uno::Reference<backend::XBackendChangesListener>& xListener,
            const rtl::OUString& aComponent)
                throw (::com::sun::star::uno::RuntimeException);

        //Notify all listener of component change
        void notifyListeners(const rtl::OUString& aGconfKey);

    protected:
        /**
          Service constructor from a service factory.

          @param xContext   component context
          */
         GconfBackend(const uno::Reference<uno::XComponentContext>& xContext)
            throw (backend::BackendAccessException);

        /** Destructor */
        ~GconfBackend(void) ;

    private:

        typedef uno::Reference<backend::XBackendChangesListener> ListenerRef;
        typedef std::multimap<rtl::OUString,ListenerRef> ListenerList;

        /** Build Gconf/OO mapping table */
        void initializeMappingTable ();


        /** The component context */
        uno::Reference<uno::XComponentContext> m_xContext;

        /** Mutex for reOOurces protection */
        osl::Mutex mMutex ;

        KeyMappingTable mKeyMap;

        /** List of component TimeStamps */
        TSMappingTable mTSMap;

        static GconfBackend* mInstance;

        /** List of listener */
        ListenerList mListenerList;

        /**Connection to Gconf */
        static GConfClient* mClient;

//       ONotificationThread* mNotificationThread;
} ;


#endif // CONFIGMGR_LOCALBE_LOCALSINGLESTRATUM_HXX_
