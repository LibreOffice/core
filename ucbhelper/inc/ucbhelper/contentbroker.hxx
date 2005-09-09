/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contentbroker.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:26:55 $
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

#ifndef _UCBHELPER_CONTENTBROKER_HXX
#define _UCBHELPER_CONTENTBROKER_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace com { namespace sun { namespace star { namespace ucb {
    class XContentIdentifierFactory;
    class XContentProvider;
    class XContentProviderManager;
    class XCommandProcessor;
} } } }

#ifndef _UCBHELPER_CONFIGUREUCB_HXX_
#include <ucbhelper/configureucb.hxx>
#endif
#ifndef INCLUDED_UCBHELPERDLLAPI_H
#include "ucbhelper/ucbhelperdllapi.h"
#endif

namespace ucb
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

} /* namespace ucb */

#endif /* !_UCBHELPER_CONTENTBROKER_HXX */
