/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_DATAACCESSDESCRIPTOR_HXX
#define INCLUDED_SVX_DATAACCESSDESCRIPTOR_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <svx/svxdllapi.h>


namespace svx
{


    class ODADescriptorImpl;


    //= DataAccessDescriptorProperty

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


    //= ODataAccessDescriptor

    /** class encapsulating the css::sdb::DataAccessDescriptor service.
    */
    class SVX_DLLPUBLIC SAL_WARN_UNUSED ODataAccessDescriptor
    {
    protected:
        ODADescriptorImpl*      m_pImpl;

    public:
        ODataAccessDescriptor();
        ODataAccessDescriptor( const ODataAccessDescriptor& _rSource );
        ODataAccessDescriptor( const css::uno::Reference< css::beans::XPropertySet >& _rValues );
        ODataAccessDescriptor( const css::uno::Sequence< css::beans::PropertyValue >& _rValues );

        // allows to construct a descriptor from an Any containing either an XPropertySet or a property value sequence
        ODataAccessDescriptor( const css::uno::Any& _rValues );

        const ODataAccessDescriptor& operator=(const ODataAccessDescriptor& _rSource);

        ~ODataAccessDescriptor();

        /** returns the descriptor as property value sequence
            <p>If you call this method more than once, without writing any values between both calls, the same object
            is returned. If you wrote values, a new object is returned.</p>
        */
        css::uno::Sequence< css::beans::PropertyValue >
                    createPropertyValueSequence();

        /** initialized the descriptor from the property values given
            <p>If <arg>_bClear</arg> is <TRUE/>, the descriptor will clear all its current settings before
            initializing with the new ones.</p>
        */
        void        initializeFrom(
                        const css::uno::Sequence< css::beans::PropertyValue >& _rValues,
                        bool _bClear = true);

        /// checks whether or not a given property is present in the descriptor
        bool        has(DataAccessDescriptorProperty _eWhich) const;

        /** erases the given property from the descriptor
        */
        void        erase(DataAccessDescriptorProperty _eWhich);

        /** empties the descriptor
        */
        void        clear();

        /// return the value of a given property
        const   css::uno::Any& operator [] ( DataAccessDescriptorProperty _eWhich ) const;

        /** return the (modifiable) value of a given property
            <p>This operator is not allowed to be called if the descriptor is readonly.</p>
        */
                css::uno::Any& operator [] ( DataAccessDescriptorProperty _eWhich );


        /** returns either the data source name if given or the database location
        */
        OUString getDataSource() const;


        /** set the data source name, if it is not file URL
            @param  _sDataSourceNameOrLocation
                the data source name or database location
        */
        void setDataSource(const OUString& _sDataSourceNameOrLocation);
    };


}


#endif // INCLUDED_SVX_DATAACCESSDESCRIPTOR_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
