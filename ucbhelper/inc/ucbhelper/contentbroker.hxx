/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _UCBHELPER_CONTENTBROKER_HXX
#define _UCBHELPER_CONTENTBROKER_HXX

#include <boost/noncopyable.hpp>
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
class UCBHELPER_DLLPUBLIC ContentBroker : private boost::noncopyable
{
    ContentBroker_Impl*     m_pImpl;
    // The "one and only" Broker.
    static ContentBroker*   m_pTheBroker;

private:
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
