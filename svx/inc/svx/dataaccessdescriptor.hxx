/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#define _SVX_DATACCESSDESCRIPTOR_HXX_

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "svx/svxdllapi.h"

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
