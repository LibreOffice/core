/*************************************************************************
 *
 *  $RCSfile: gconfbackend.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2005-03-23 09:14:02 $
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
 *  This library is free Software; you can redistribute it and/or
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
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SoFTWARE IS FREE OF DEFECTS,
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

#ifndef GCONFBACKEND_HXX_
#define GCONFBACKEND_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif // _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif // _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_

//#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
//#include <com/sun/star/lang/XMultiServiceFactory.hpp>
//#endif // _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_INVALIDBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_CANNOTCONNECTEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDCHANGESNOTIFIER_HPP_
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif // _CPPUHELPER_COMPBASE3_HXX_

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

        /**
          Provides the supported component nodes

          @return supported component nodes
        */
        static uno::Sequence<rtl::OUString> SAL_CALL getSupportedComponents(void) ;

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

        static GconfBackend* mInstance;

        /** List of listener */
        ListenerList mListenerList;

        /**Connection to Gconf */
        static GConfClient* mClient;

//       ONotificationThread* mNotificationThread;
} ;


#endif // CONFIGMGR_LOCALBE_LOCALSINGLESTRATUM_HXX_
