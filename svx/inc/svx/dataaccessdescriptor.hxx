/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dataaccessdescriptor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:43:23 $
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

#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#define _SVX_DATACCESSDESCRIPTOR_HXX_

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//........................................................................
namespace svx
{
//........................................................................

    class ODADescriptorImpl;

    //====================================================================
    //= DataAccessDescriptorProperty
    //====================================================================
    enum DataAccessDescriptorProperty
    {
        daDataSource,           /// data source name            (string)
        daDatabaseLocation,     /// database file URL           (string)
        daConnectionResource,   /// database driver URL         (string)
        daConnection,           /// connection                  (XConnection)

        daCommand,              /// command                     (string)
        daCommandType,          /// command type                (long)
        daEscapeProcessing,     /// escape processing           (boolean)
        daFilter,               /// additional filter           (string)
        daCursor,               /// the cursor                  (XResultSet)

        daColumnName,           /// column name                 (string)
        daColumnObject,         /// column object               (XPropertySet)

        daSelection,            /// selection                   (sequence< any >)
        daBookmarkSelection,    /// selection are bookmarks?    (boolean)

        daComponent             /// component name              (XContent)
    };

    //====================================================================
    //= ODataAccessDescriptor
    //====================================================================
    /** class encapsulating the <type scope="com::sun::star::sdb">DataAccessDescriptor</type> service.
    */
    class SVX_DLLPUBLIC ODataAccessDescriptor
    {
    protected:
        ODADescriptorImpl*      m_pImpl;

    public:
        ODataAccessDescriptor();
        ODataAccessDescriptor( const ODataAccessDescriptor& _rSource );
        ODataAccessDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rValues );
        ODataAccessDescriptor( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rValues );

        // allows to construct a descriptor from an Any containing either an XPropertySet or a property value sequence
        ODataAccessDescriptor( const ::com::sun::star::uno::Any& _rValues );

        const ODataAccessDescriptor& operator=(const ODataAccessDescriptor& _rSource);

        ~ODataAccessDescriptor();

        /** returns the descriptor as property set
            <p>If you call this method more than once, without writing any values between both calls, the same object
            is returned. If you wrote values, a new object is returned.</p>
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    createPropertySet();
        /** returns the descriptor as property value sequence
            <p>If you call this method more than once, without writing any values between both calls, the same object
            is returned. If you wrote values, a new object is returned.</p>
        */
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                    createPropertyValueSequence();

        /** returns the descriptor as Any sequence
            <p>If you call this method more than once, without writing any values between both calls, the same object
            is returned. If you wrote values, a new object is returned.</p>
        */
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
                    createAnySequence();

        /** initialized the descriptor from the property set given
            <p>If <arg>_bClear</arg> is <TRUE/>, the descriptor will clear all it's current settings before
            initializing with the new ones.</p>
        */
        void        initializeFrom(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxValues,
                        sal_Bool _bClear = sal_True);

        /** initialized the descriptor from the property values given
            <p>If <arg>_bClear</arg> is <TRUE/>, the descriptor will clear all it's current settings before
            initializing with the new ones.</p>
        */
        void        initializeFrom(
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rValues,
                        sal_Bool _bClear = sal_True);

        /// checks whether or not a given property is present in the descriptor
        sal_Bool    has(DataAccessDescriptorProperty _eWhich) const;

        /** erases the given property from the descriptor
        */
        void        erase(DataAccessDescriptorProperty _eWhich);

        /** empties the descriptor
        */
        void        clear();

        /// return the value of a given property
        const   ::com::sun::star::uno::Any& operator [] ( DataAccessDescriptorProperty _eWhich ) const;

        /** return the (modifiable) value of a given property
            <p>This operator is not allowed to be called if the descriptor is readonly.</p>
        */
                ::com::sun::star::uno::Any& operator [] ( DataAccessDescriptorProperty _eWhich );


        /** returns either the data source name if given or the database location
        */
        ::rtl::OUString getDataSource() const;


        /** set the data source name, if it is not file URL
            @param  _sDataSourceNameOrLocation
                the data source name or database location
        */
        void setDataSource(const ::rtl::OUString& _sDataSourceNameOrLocation);
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // _SVX_DATACCESSDESCRIPTOR_HXX_


