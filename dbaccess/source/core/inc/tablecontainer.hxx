/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablecontainer.hxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:35:50 $
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
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef DBACCESS_CORE_FILTERED_CONTAINER_HXX
#include "FilteredContainer.hxx"
#endif
#ifndef DBA_CORE_WARNINGS_HXX
#include "warnings.hxx"
#endif
#ifndef DBA_CORE_REFRESHLISTENER_HXX
#include "RefreshListener.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

namespace dbaccess
{
    typedef ::cppu::ImplHelper1< ::com::sun::star::container::XContainerListener> OTableContainer_Base;

    //==========================================================================
    //= OTableContainer
    //==========================================================================
    class OTable;
    class OTableContainer;
    class OContainerMediator;

    class OTableContainer :  public OFilteredContainer,
                             public OTableContainer_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xTableDefinitions;
        ::rtl::Reference< OContainerMediator >                                          m_pTableMediator;
        sal_Bool                m_bInDrop;                  // set when we are in the drop method


        // OFilteredContainer
        virtual void addMasterContainerListener();
        virtual void removeMasterContainerListener();
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > getTableTypeFilter(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableTypeFilter) const;

        // ::connectivity::sdbcx::OCollection
        virtual connectivity::sdbcx::ObjectType     createObject(const ::rtl::OUString& _rName);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   createDescriptor();
        virtual connectivity::sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
        virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);

        virtual sal_Bool isNameValid(const ::rtl::OUString& _rName,
                                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableFilter,
                                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableTypeFilter,
                                    const ::std::vector< WildCard >& _rWCSearch) const;
        virtual void SAL_CALL disposing();

        /** retrieve a table type filter to pass to <member scope="com::sun::star::sdbc">XDatabaseMetaData::getTables</member>,
            according to the current data source settings
        */
        void    getAllTableTypeFilter( ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* [out] */ _rFilter ) const;

        inline virtual void SAL_CALL acquire() throw(){ OFilteredContainer::acquire();}
        inline virtual void SAL_CALL release() throw(){ OFilteredContainer::release();}
    // ::com::sun::star::lang::XServiceInfo
        DECLARE_SERVICE_INFO();

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

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
        OTableContainer( ::cppu::OWeakObject& _rParent,
            ::osl::Mutex& _rMutex,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xCon,
            sal_Bool _bCase,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >&  _xTableDefinitions,
            IRefreshListener*   _pRefreshListener,
            IWarningsContainer* _pWarningsContainer,
            oslInterlockedCount& _nInAppend
            );

        virtual ~OTableContainer();
    };
}
#endif // _DBA_CORE_TABLECONTAINER_HXX_


