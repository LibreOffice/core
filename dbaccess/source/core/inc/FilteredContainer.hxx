/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FilteredContainer.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:35:18 $
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
#ifndef DBACCESS_CORE_FILTERED_CONTAINER_HXX
#define DBACCESS_CORE_FILTERED_CONTAINER_HXX

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include <connectivity/sdbcx/VCollection.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

class WildCard;
namespace dbaccess
{
    class IWarningsContainer;
    class IRefreshListener;

    class OFilteredContainer : public ::connectivity::sdbcx::OCollection
    {
    protected:
        IWarningsContainer*     m_pWarningsContainer;
        IRefreshListener*       m_pRefreshListener;
        oslInterlockedCount&    m_nInAppend;

        // holds the original container which where set in construct but they can be null
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xMasterContainer;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection >     m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >   m_xMetaData;

        mutable sal_Bool m_bConstructed;        // late ctor called

        virtual sal_Bool isNameValid(const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableFilter,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableTypeFilter,
            const ::std::vector< WildCard >& _rWCSearch) const;

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > getTableTypeFilter(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableTypeFilter) const = 0;

        inline virtual void addMasterContainerListener(){}
        inline virtual void removeMasterContainerListener(){}

        // ::connectivity::sdbcx::OCollection
        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);

        virtual ::rtl::OUString getNameForObject(const ::connectivity::sdbcx::ObjectType& _xObject);

        /** tell the container to free all elements and all additional resources.<BR>
            After using this method the object may be reconstructed by calling one of the <code>constrcuct</code> methods.
        */
        virtual void SAL_CALL disposing();

        class EnsureReset
        {
        public:
            EnsureReset( oslInterlockedCount& _rValueLocation)
                :m_rValue( _rValueLocation )
            {
                osl_incrementInterlockedCount(&m_rValue);
            }

            ~EnsureReset()
            {
                osl_decrementInterlockedCount(&m_rValue);
            }

        private:
            oslInterlockedCount&   m_rValue;
        };
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
        OFilteredContainer( ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xCon,
                        sal_Bool _bCase,
                        IRefreshListener*   _pRefreshListener,
                        IWarningsContainer* _pWarningsContainer,
                        oslInterlockedCount& _nInAppend
                        );

        inline void dispose() { disposing(); }

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

        inline sal_Bool isInitialized() const { return m_bConstructed; }
    };
// ..............................................................................
} // namespace
// ..............................................................................

#endif // DBACCESS_CORE_FILTERED_CONTAINER_HXX
