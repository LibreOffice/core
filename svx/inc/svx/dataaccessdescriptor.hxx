/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


