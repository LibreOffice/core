/*************************************************************************
 *
 *  $RCSfile: resultsethelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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

#ifndef _UCBHELPER_RESULTSETHELPER_HXX
#define _UCBHELPER_RESULTSETHELPER_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif
#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif

namespace cppu {
    class OInterfaceContainerHelper;
}

namespace ucb {

//=========================================================================

#define DYNAMICRESULTSET_SERVICE_NAME \
                            "com.sun.star.ucb.DynamicContentResultSet"

//=========================================================================

/**
  * This is an abstract base class for implementations of the service
  * com.sun.star.ucb.DynamicResultSet, which is the result of the command
  * "open" executed at a UCB folder content.
  *
  * Features of the base class implementation:
  * - standard interfaces ( XInterface, XTypeProvider, XServiceInfo )
  * - all required interfaces for service com::sun::star::ucb::DynamicResultSet
  */
class ResultSetImplHelper :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XDynamicResultSet
{
    cppu::OInterfaceContainerHelper* m_pDisposeEventListeners;
    sal_Bool                         m_bStatic;
    sal_Bool                         m_bInitDone;

protected:
    osl::Mutex                                           m_aMutex;
    com::sun::star::ucb::OpenCommandArgument2            m_aCommand;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >     m_xSMgr;
    // Resultset #1
    com::sun::star::uno::Reference<
        com::sun::star::sdbc::XResultSet >               m_xResultSet1;
    // Resultset #2
    com::sun::star::uno::Reference<
        com::sun::star::sdbc::XResultSet >               m_xResultSet2;
    // Resultset changes listener.
    com::sun::star::uno::Reference<
        com::sun::star::ucb::XDynamicResultSetListener > m_xListener;

private:
    void init( sal_Bool bStatic );

    /**
      * Your implementation of this method has to fill the protected member
      * m_xResultSet1. This resultset must implement a complete static
      * resultset ( service com.sun.star.ucb.ContentResultSet ). This method
      * will be called at most once in the life of your implementation object.
      * After this method was called, the type of this resultset will be
      * "static". There is no way to change the type afterwards.
      * If this method gets called the client wants to use your resultset
      * exclusively statically. You may deploy this factum to optimize your
      * implementation (i.e. "switch off" all changes detection code in
      * your implementation).
      * Note that you may use the class ucb::ResultSet to implement the
      * static resultset, that is required here.
      */
    virtual void initStatic() = 0;

    /**
      * Your implementation of this method has to fill the protected members
      * m_xResultSet1 and m_xResultSet2 of this base class. Each of these
      * resultsets must implement a complete static resultset
      * ( service com.sun.star.ucb.ContentResultSet ). This method will be
      * called at most once in the life of your implementation object.
      * After this method was called, the type of this resultset will be
      * "dynamic". There is no way to change the type afterwards.
      * If this method gets called the client wants to use your resultset
      * exclusively dynamically. This means, it is interested in getting
      * notifications on changes of data of the resultset contents. ( These
      * changes are to propagate by your implementation throw the member
      * m_xListener of this base class ).
      * If your implementation cannot detect changes of relevant data, you
      * may fill m_xResultSet1 and m_xResultSet2 with the same static resultset
      * implementation object. This normally will be the same instance you put
      * into m_xResultSet1 when initStatic() is called.
      */
    virtual void initDynamic() = 0;

public:
    /**
      * Construtor.
      *
      * @param rxSMgr is a Service Manager.
      */
    ResultSetImplHelper(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr );

    /**
      * Construtor.
      *
      * @param rxSMgr is a Service Manager.
      * @param rCommand is the paramter for the open command that produces
      *        this resultset.
      */
    ResultSetImplHelper(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const com::sun::star::ucb::OpenCommandArgument2& rCommand );

    /**
      * Destructor.
      */
    virtual ~ResultSetImplHelper();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_NOFACTORY_DECL()

    // XComponent ( base class of XDynamicResultSet )
    virtual void SAL_CALL
    dispose()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XDynamicResultSet
    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet > SAL_CALL
    getStaticResultSet()
        throw( com::sun::star::ucb::ListenerAlreadySetException,
        com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setListener( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XDynamicResultSetListener >& Listener )
        throw( com::sun::star::ucb::ListenerAlreadySetException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    connectToCache( const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > & xCache )
        throw( com::sun::star::ucb::ListenerAlreadySetException,
               com::sun::star::ucb::AlreadyInitializedException,
               com::sun::star::ucb::ServiceNotFoundException,
               com::sun::star::uno::RuntimeException );

    /**
      * The implemetation of this method always returns 0. Override this
      * method, if necassary.
      */
    virtual sal_Int16 SAL_CALL
    getCapabilities()
        throw( com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    /**
      * This method returns, whether the resultset is static or dynamic.
      * If neither getStatic() nor getDynamic() was called, the type
      * of the resultset is "dynamic".
      *
      * @return true, if the resultset type is "static". False, otherwise.
      */
    sal_Bool isStatic() const { return m_bStatic; }
};

}

#endif /* !_UCBHELPER_RESULTSETHELPER_HXX */
