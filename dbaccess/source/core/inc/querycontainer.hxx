/*************************************************************************
 *
 *  $RCSfile: querycontainer.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:14:47 $
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

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
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
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _DBA_CORE_DEFINITIONCONTAINER_HXX_
#include "definitioncontainer.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef DBA_CONTAINERLISTENER_HXX
#include "ContainerListener.hxx"
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

    typedef ::cppu::ImplHelper4<     ::com::sun::star::container::XContainerListener,
                                     ::com::sun::star::sdbcx::XDataDescriptorFactory,
                                     ::com::sun::star::sdbcx::XAppend,
                                     ::com::sun::star::sdbcx::XDrop> OQueryContainer_Base;

    //==========================================================================
    //= OQueryContainer
    //==========================================================================
    class OQueryContainer;
    typedef OContainerListener<OQueryContainer> OCommandsListener;
    class OQuery;
    class IWarningsContainer;
    class OQueryContainer   : public ODefinitionContainer
                            , public OQueryContainer_Base
    {
    protected:
        IWarningsContainer*         m_pWarnings;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                                    m_xCommandDefinitions;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                                    m_xConnection;
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

        // ODefinitionContainer
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > createObject( const ::rtl::OUString& _rName);
        virtual sal_Bool checkExistence(const ::rtl::OUString& _rName);

        // helper
        virtual void SAL_CALL disposing();
        virtual ~OQueryContainer();
    public:
        /** ctor of the container. The parent has to support the <type scope="com::sun::star::sdbc">XConnection</type>
            interface.<BR>
            @param          _rQueryFilter       restricts the visible tables by name
            @param          _rQueryTypeFilter   restricts the visible tables by type
            @param          _rxMasterQueries    the container for the "master objects", i.e. the objects implementing
                                                the <type scope="com::sun::star::sdb">CommandDefinition</type> service
                                                which this container's elements extend
            @param _pWarnings
                specifies a warnings container (May be <NULL/>)
                <p>Any errors which occur during the lifetime of the query container,
                which cannot be reported as exceptionts (for instance in methods where throwing an SQLException is
                not allowed) will be appended to this container.</p>
                <p>The caller is responsible for ensuring the lifetime of the object pointed to by this parameter.
                Usually, it's the same object as referenced by _rConnection</p>
        */
        OQueryContainer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxCommandDefinitions,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            IWarningsContainer* _pWarnings
            );

        DECLARE_XINTERFACE( )
    // ::com::sun::star::lang::XServiceInfo
        DECLARE_SERVICE_INFO();

    // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDataDescriptorFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XAppend
        virtual void SAL_CALL appendByDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDrop
        virtual void SAL_CALL dropByName( const ::rtl::OUString& elementName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dropByIndex( sal_Int32 index ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAccess
        virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::container::XNameAccess
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);

    private:
        // helper
        /** create a query object wrapping a CommandDefinition given by name. To retrieve the object, the CommandDescription
            container will be asked for the given name.<BR>
            The returned object is acquired once.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > implCreateWrapper(const ::rtl::OUString& _rName);
        /// create a query object wrapping a CommandDefinition. The returned object is acquired once.
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > implCreateWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _rxCommandDesc);

    };
//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_CORE_QUERYCONTAINER_HXX_


