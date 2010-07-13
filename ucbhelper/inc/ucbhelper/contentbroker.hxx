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

#ifndef _UCBHELPER_CONTENTBROKER_HXX
#define _UCBHELPER_CONTENTBROKER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.h>

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace com { namespace sun { namespace star { namespace ucb {
    class XContentIdentifierFactory;
    class XContentProvider;
    class XContentProviderManager;
    class XCommandProcessor;
} } } }
#include <ucbhelper/registerucb.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace ucbhelper
{

class ContentBroker_Impl;

//=========================================================================

/**
  * This class simplifies access to the Universal Content Broker (UCB).
  * Currently there can only be one UCB instance per process. This class can
  * be used to initialize and deinitialize the Broker and to access its
  * interfaces directly.
  */
class UCBHELPER_DLLPUBLIC ContentBroker
{
    ContentBroker_Impl*     m_pImpl;
    // The "one and only" Broker.
    static ContentBroker*   m_pTheBroker;

private:
    UCBHELPER_DLLPRIVATE ContentBroker( const ContentBroker& );                 // n.i.
    UCBHELPER_DLLPRIVATE ContentBroker& operator=( const ContentBroker& );  // n.i.

    /** Constructor.
      *
      * @param rxSMgr is a Service Manager.
      * @param rArguments are the arguments to pass to the
      *        com.sun.star.ucb.UniversalContentBroker service when creating it.
      *        Currently, this must be a sequence containing exactly two
      *        strings, a primary and a secondary configuration key.
      *        Refer to http://ucb.openoffice.org/docs/ucb-configuration.html
      *        for more information on UCB configuration.
      */
    UCBHELPER_DLLPRIVATE ContentBroker( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::lang::XMultiServiceFactory >&    rSMgr,
                   const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Any >& rArguments );

    /** Constructor.
      *
      * @param rxSMgr is a Service Manager.
      * @param rData are the data for the for the content providers for
      *        the new UCB.
      */
    UCBHELPER_DLLPRIVATE ContentBroker( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::lang::XMultiServiceFactory >&    rSMgr,
                   const ContentProviderDataList & rData );

protected:
    /**
      * Destructor.
      */
    ~ContentBroker();

public:
    /** Initialize "the one and only" Broker.  This method must be called
      * exactly once, before the Broker is used in any way.
      *
      * @param rSMgr is a factory to create services needed in the Broker's
      *        implementation.
      *
      * @param rArguments are the arguments to pass to the
      *        com.sun.star.ucb.UniversalContentBroker service when creating
      *        it. Currently, this must be a sequence containing exactly two
      *        strings, a primary and a secondary configuration key.
      *        Refer to http://ucb.openoffice.org/docs/ucb-configuration.html
      *        for more information on UCB configuration.
      *
      * @return True if creation and possible configuration of the Broker
      *         was successful.
      */
    static sal_Bool
    initialize( const::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Any >& rArguments );

    /** Initialize "the one and only" Broker.  This method must be called
      * exactly once, before the Broker is used in any way.
      *
      * @param rSMgr is a factory to create services needed in the Broker's
      *        implementation.
      *
      * @param rData are the data for the for the content providers for
      *        the UCB to initialize.
      *
      * @return True if creation and possible configuration of the Broker
      *         was successful.
      */
    static sal_Bool
    initialize( const::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                const ContentProviderDataList & rData );

    /** Deinitialize "the one and only" Broker.  Once this method has been
      * called, the Broker must not be used any longer.
      */
    static void
    deinitialize();

    /**
      * This method returns the Broker, if it was already initialized.
      *
      * @return the Broker or 0, if ContentBroker::initialize() was not yet
      *         called or did fail.
      */
    static ContentBroker*
    get();

    /**
      * This method returns the Service Manager used to instanciate the Broker.
      *
      * @return a Service Manager.
      */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory >
    getServiceManager() const;

    /**
      * This method returns the XContentIdentifierFactory interface of the
      * Broker.
      *
      * @return a XContentIdentifierFactory interface.
      */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentIdentifierFactory >
    getContentIdentifierFactoryInterface() const;

    /**
      * This method returns the XContentProvider interface of the Broker.
      *
      * @return a XContentProvider interface.
      */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider >
    getContentProviderInterface() const;

    /**
      * This method returns the XContentProviderManager interface of the Broker.
      *
      * @return a XContentProviderManager interface.
      */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProviderManager >
    getContentProviderManagerInterface() const;

    /**
      * This method returns the XCommandProcessor interface of the Broker.
      *
      * @return a XCommandProcessor interface.
      */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XCommandProcessor >
    getCommandProcessorInterface() const;
};

} /* namespace ucbhelper */

#endif /* !_UCBHELPER_CONTENTBROKER_HXX */
