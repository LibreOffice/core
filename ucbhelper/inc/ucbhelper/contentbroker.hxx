/*************************************************************************
 *
 *  $RCSfile: contentbroker.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:36 $
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
 *  This library is free software; you can redistribute it and/or
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
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
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
} } } }

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
class ContentBroker
{
    ContentBroker_Impl*     m_pImpl;
    // The "one and only" Broker.
    static ContentBroker*   m_pTheBroker;

private:
    ContentBroker( const ContentBroker& );              // n.i.
    ContentBroker& operator=( const ContentBroker& );   // n.i.

    /** Constructor.
      *
      * @param rxSMgr is a Service Manager.
      * @param rArguments are the arguments to pass to the
      *        com.sun.star.ucb.UniversalContentBroker service when creating it.
      *        Currently, this must be a sequence containing exactly one
      *        boolean. If this boolean is true, the Broker is automatically
      *        configured (the appropriate content provider services are
      *        registered); if the boolean is false, the Broker is left
      *        unconfigured (no content providers are registered).
      */
    ContentBroker( const ::com::sun::star::uno::Reference<
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
      * @param  rSMgr is a factory to create services needed in the Broker's
      *         implementation.
      *
      * @param  rArguments are the arguments to pass to the
      *         com.sun.star.ucb.UniversalContentBroker service when creating
      *         it. Currently, this must be a sequence containing exactly one
      *         boolean.    If this boolean is true, the Broker is automatically
      *         configured (the appropriate content provider services are
      *         registered); if the boolean is false, the Broker is left
      *         unconfigured (no content providers are registered).
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
};

} /* namespace ucb */

#endif /* !_UCBHELPER_CONTENTBROKER_HXX */
