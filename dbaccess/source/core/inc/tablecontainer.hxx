/*************************************************************************
 *
 *  $RCSfile: tablecontainer.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 15:03:35 $
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

#ifndef _DBA_CORE_TABLECONTAINER_HXX_
#define _DBA_CORE_TABLECONTAINER_HXX_

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
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
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _DBA_CONFIGNODE_HXX_
#include "confignode.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include <connectivity/sdbcx/VCollection.hxx>
#endif


#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

class WildCard;
namespace dbaccess
{
    typedef ::cppu::ImplHelper1< ::com::sun::star::util::XFlushable > OTableContainer_Base;


    //==========================================================================
    //= OTableContainer
    //==========================================================================
    class OTable;

    class OTableContainer :  public ::connectivity::sdbcx::OCollection,
                             public OTableContainer_Base
    {
    protected:

        OConfigurationTreeRoot  m_aCommitLocation; // need to commit new table nodes
        OConfigurationNode      m_aTablesConfig;

        // holds the original tables which where set in construct but they can be null
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xMasterTables;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >   m_xMetaData;

        sal_Bool m_bConstructed : 1;        // late ctor called

        sal_Bool isNameValid(const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableFilter,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableTypeFilter,
            const ::std::vector< WildCard >& _rWCSearch) const;

        // ::connectivity::sdbcx::OCollection
        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >     createObject(const ::rtl::OUString& _rName);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   createEmptyObject();
    public:
        /** ctor of the container. The parent has to support the <type scope="com::sun::star::sdbc">XConnection</type>
            interface.<BR>
            @param          _rParent            the object which acts as parent for the container.
                                                all refcounting is rerouted to this object
            @param          _rMutex             the access safety object of the parent
            @param          _rTableFilter       restricts the visible tables by name
            @param          _rTableTypeFilter   restricts the visible tables by type
            @see            construct
        */
        OTableContainer( const OConfigurationNode& _rTablesConfig,const OConfigurationTreeRoot& _rCommitLocation,
            ::cppu::OWeakObject& _rParent,
            ::osl::Mutex& _rMutex,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xCon
            );
        ~OTableContainer();

        /** late ctor. The container will fill itself with the data got by the connection meta data, considering the
            filters given (the connection is the parent object you passed in the ctor).
        */
        void construct(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableFilter,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableTypeFilter
            );

        /** late ctor. The container will fill itself with wrapper objects for the tables returned by the given
            name container.
        */
        void construct(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxMasterContainer,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableFilter,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableTypeFilter
            );

        sal_Bool isInitialized() const { return m_bConstructed; }

        /** tell the container to free all elements and all additional resources.<BR>
            After using this method the object may be reconstructed by calling one of the <code>constrcuct</code> methods.
        */
        virtual void SAL_CALL disposing();

        // XInterface
        virtual void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)
        {
            m_rParent.acquire();
        }
        virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)
        {
            m_rParent.release();
        }
    // ::com::sun::star::lang::XServiceInfo
        DECLARE_SERVICE_INFO();

        // ::com::sun::star::util::XFlushable
        virtual void SAL_CALL flush(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw(::com::sun::star::uno::RuntimeException){}
        virtual void SAL_CALL removeFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw(::com::sun::star::uno::RuntimeException){}

        // XAppend
        virtual void SAL_CALL appendByDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
        // XDrop
        virtual void SAL_CALL dropByName( const ::rtl::OUString& elementName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dropByIndex( sal_Int32 index ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif // _DBA_CORE_TABLECONTAINER_HXX_


