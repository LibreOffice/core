/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filtermanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:41:24 $
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
#ifndef CONNECTIVITY_FILTERMANAGER_HXX
#define CONNECTIVITY_FILTERMANAGER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
/** === end UNO includes === **/

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include <vector>

//........................................................................
namespace dbtools
{
//........................................................................

    //====================================================================
    //= FilterManager
    //====================================================================
    /** manages the filter of a database component with filter properties

        The idea is that the filter which a database component actually really uses is composed of several single
        filter components (which all are conjunctive).

        First, there is a component which is visible to the clients of the database component itself - if they ask
        the database component for the Filter property, they will get this public filter.

        Second, there is an implicit filter, which is (to be) created from the MasterFields and DetailFields
        property of the database component, if the latter denote columns.<br/>
        For instance, if there is a link-pair CustomerID->cid, where |CustomerID| is a column of the master
        database component, and |cid| is a column of the detail database component (the database component we're responsible for), then there will
        be an implicit filter "cid = :param_cid_link" (or something like this), which is never visible
        to the clients of the database component, but nevertheless needs to be propagated to the aggregated RowSet.<br/>
        Actually, this implicit filter is maintained by the FormParameterManager.

        Potentially, there could be more filter components (for instance, you could imagine database component
        controls which act as live filter, which could be implemented with a third component), but
        at the moment there are only these two.
    */
    class FilterManager
    {
    public:
        enum FilterComponent
        {
            fcPublicFilter = 0,     // the filter which is to be published as "Filter" property of the database component
            fcLinkFilter,           // the filter part which is implicitly created for a database component when connecting
                                    // master and detail database components via column names

            FC_COMPONENT_COUNT      // boundary delimiter, not to be used from outside
        };

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                            m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                            m_xComponent;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                            m_xComponentAggregate;
        ::std::vector< ::rtl::OUString >    m_aFilterComponents;
        sal_Bool                            m_bApplyPublicFilter;

    public:
        /// ctor
        explicit FilterManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        /// late ctor
        void    initialize(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxFormAggregate
                );

        /// makes the object forgetting the references to the database component
        void    dispose( );

        const ::rtl::OUString&  getFilterComponent( FilterComponent _eWhich ) const;
        void                    setFilterComponent( FilterComponent _eWhich, const ::rtl::OUString& _rComponent );

        inline sal_Bool isApplyPublicFilter( ) const { return m_bApplyPublicFilter; }
               void     setApplyPublicFilter( sal_Bool _bApply );

    private:
        /** retrieves a filter which is a conjunction of all single filter components
        */
        ::rtl::OUString         getComposedFilter( ) const;

        /** appends one filter component to the statement in our composer
        */
        void    appendFilterComponent( ::rtl::OUString& /* [inout] */ _rAppendTo, const ::rtl::OUString& _rComponent ) const;

        /// checks whether there is only one (or even no) non-empty filter component
        bool    isThereAtMostOneComponent( ::rtl::OUString& _rOnlyComponent ) const;

        /// returns the index of the first filter component which should be considered when building the composed filter
        inline  sal_Int32   getFirstApplicableFilterIndex() const
        {
            return m_bApplyPublicFilter ? fcPublicFilter : fcPublicFilter + 1;
        }
    };

//........................................................................
} // namespacefrm
//........................................................................

#endif // CONNECTIVITY_FORMFILTERMANAGER_HXX

