/*************************************************************************
 *
 *  $RCSfile: querycontainer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:41:49 $
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

#ifndef _DBA_CORE_QUERYCONTAINER_HXX_
#define _DBA_CORE_QUERYCONTAINER_HXX_

#ifndef _CPPUHELPER_IMPLBASE9_HXX_
#include <cppuhelper/implbase9.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif

#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_CORE_CONFIGURATIONFLUSHABLE_HXX_
#include "configurationflushable.hxx"
#endif
#ifndef _DBA_CONFIGNODE_HXX_
#include "confignode.hxx"
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

    typedef ::cppu::WeakImplHelper9< ::com::sun::star::container::XEnumerationAccess,
                                     ::com::sun::star::container::XContainerListener,
                                     ::com::sun::star::container::XNameAccess,
                                     ::com::sun::star::container::XIndexAccess,
                                     ::com::sun::star::container::XContainer,
                                     ::com::sun::star::sdbcx::XDataDescriptorFactory,
                                     ::com::sun::star::sdbcx::XAppend,
                                     ::com::sun::star::sdbcx::XDrop,
                                     ::com::sun::star::lang::XServiceInfo > OQueryContainer_Base;

    //==========================================================================
    //= OQueryContainer
    //==========================================================================
    class OQuery;
    class OCommandsListener;
    class OQueryContainer   :public OQueryContainer_Base
                            ,public OConfigurationFlushable
    {
    protected:

        ::cppu::OWeakObject&        m_rParent;
        ::osl::Mutex&               m_rMutex;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                                    m_xCommandDefinitions;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                    m_xORB;

        ::cppu::OInterfaceContainerHelper
                                    m_aContainerListeners;

        DECLARE_STL_USTRINGACCESS_MAP(OQuery*, Queries);
        DECLARE_STL_VECTOR(QueriesIterator, QueriesIndexAccess);
        Queries                     m_aQueries;
        QueriesIndexAccess          m_aQueriesIndexed;

        // possible actions on our "aggregate"
        enum AGGREGATE_ACTION { NONE, INSERTING, FLUSHING };
        AGGREGATE_ACTION        m_eDoingCurrently;

        OCommandsListener*          m_pCommandsListener;
        // ------------------------------------------------------------------------
        /** a class which automatically resets m_eDoingCurrently in it's destructor
        */
        class OAutoActionReset; // just for the following friend declaration
        friend class OAutoActionReset;
        class OAutoActionReset
        {
            OQueryContainer*        m_pActor;
        public:
            OAutoActionReset(OQueryContainer* _pActor) : m_pActor(_pActor) { }
            ~OAutoActionReset() { m_pActor->m_eDoingCurrently = NONE; }
        };

    public:
        /** ctor of the container. The parent has to support the <type scope="com::sun::star::sdbc">XConnection</type>
            interface.<BR>
            @param          _rConnection        the connection object which acts as parent for the container.
                                                all refcounting is rerouted to this object
            @param          _rMutex             the access safety object of the parent
            @param          _rQueryFilter       restricts the visible tables by name
            @param          _rQueryTypeFilter   restricts the visible tables by type
            @param          _rxMasterQueries    the container for the "master objects", i.e. the objects implementing
                                                the <type scope="com::sun::star::sdb">CommandDefinition</type> service
                                                which this container's elements extend
        */
        OQueryContainer(
            ::cppu::OWeakObject& _rConnection,
            ::osl::Mutex& _rMutex,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxCommandDefinitions,
            const OConfigurationTreeRoot& _rRootConfigNode,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
            );
        ~OQueryContainer();

        /** tell the container to free all elements and all additional resources.<BR>
            After using this method the object may be reconstructed by calling one of the <code>constrcuct</code> methods.
        */
        virtual void SAL_CALL dispose();

    // ::com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException) { m_rParent.acquire(); }
        virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException) { m_rParent.release(); }

    // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        DECLARE_SERVICE_INFO();

    // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XEnumerationAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainer
        virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDataDescriptorFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XAppend
        virtual void SAL_CALL appendByDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDrop
        virtual void SAL_CALL dropByName( const ::rtl::OUString& elementName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dropByIndex( sal_Int32 index ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    protected:
        // OConfigurationFlushable
        virtual void flush_NoBroadcast_NoCommit();

    private:
        // helper
        /** create a query object wrapping a CommandDefinition given by name. To retrieve the object, the CommandDescription
            container will be asked for the given name.<BR>
            The returned object is acquired once.
        */
        OQuery* implCreateWrapper(const ::rtl::OUString& _rName);
        /// create a query object wrapping a CommandDefinition. The returned object is acquired once.
        OQuery* implCreateWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxCommandDesc);

        /// search the index of the object with the given name within m_aQueriesIndexed
        sal_Int32 implGetIndex(const ::rtl::OUString& _rName);

        /// search the object key for the given name
        OConfigurationNode implGetObjectKey(const ::rtl::OUString& _rName, sal_Bool bCreate = sal_False);
    };

    //==========================================================================
    //= OCommandsListener
    // is helper class to avoid a cycle in refcount between the OQueyContainer
    // and the member m_xCommandDefinitions
    //==========================================================================
    typedef ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener > OCommandsListener_BASE;
    class OCommandsListener : public OCommandsListener_BASE
    {
        OQueryContainer* m_pDestination;
    public:
        OCommandsListener(OQueryContainer* _pDestination) : m_pDestination(_pDestination){}

        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException)
        {
            m_pDestination->elementInserted(Event);
        }
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException)
        {
            m_pDestination->elementRemoved(Event);
        }
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException)
        {
            m_pDestination->elementReplaced(Event);
        }
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException)
        {
            m_pDestination->disposing(Source);
        }
    };

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_CORE_QUERYCONTAINER_HXX_


