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

#ifndef INCLUDED_COMPHELPER_NAMEDVALUECOLLECTION_HXX
#define INCLUDED_COMPHELPER_NAMEDVALUECOLLECTION_HXX

#include <comphelper/comphelperdllapi.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <memory>
#include <algorithm>
#include <vector>


namespace comphelper
{



    // = NamedValueCollection

    struct NamedValueCollection_Impl;
    /** a collection of named values, packed in various formats.
    */
    class COMPHELPER_DLLPUBLIC NamedValueCollection
    {
    private:
        ::std::unique_ptr< NamedValueCollection_Impl >    m_pImpl;

    public:
        NamedValueCollection();

        NamedValueCollection( const NamedValueCollection& _rCopySource );

        NamedValueCollection& operator=( const NamedValueCollection& i_rCopySource );

        /** constructs a collection
            @param  _rElements
                the wrapped elements of the collection. The <code>Any</code> might contain a sequence of
                property values, a sequence of named values, or directly a property value or named value.
                All other cases are worth an assertion in non-product builds.
        */
        NamedValueCollection( const css::uno::Any& _rElements );

        /** constructs a collection
            @param _rArguments
                a sequence of Any's containing either PropertyValue's or NamedValue's.
        */
        NamedValueCollection( const css::uno::Sequence< css::uno::Any >& _rArguments );

        /** constructs a collection
            @param _rArguments
                a sequence of PropertyValues's
        */
        NamedValueCollection( const css::uno::Sequence< css::beans::PropertyValue >& _rArguments );

        /** constructs a collection
            @param _rArguments
                a sequence of NamedValue's
        */
        NamedValueCollection( const css::uno::Sequence< css::beans::NamedValue >& _rArguments );

        ~NamedValueCollection();

        inline void assign( const css::uno::Sequence< css::uno::Any >& _rArguments )
        {
            impl_assign( _rArguments );
        }

        inline void clear()
        {
            impl_assign( css::uno::Sequence< css::beans::NamedValue >() );
        }

        /** determines whether or not named values can be extracted from the given value

            @return
                <TRUE/> if and only if the given <code>Any</code> contains a <code>NamedValue</code>, a
                <code>PropertyValue</code>, or a sequence thereof.
        */
        static bool canExtractFrom( css::uno::Any const & i_value );

        /// returns the number of elements in the collection
        size_t  size() const;

        /// determines whether the collection is empty
        bool    empty() const;

        /** returns the names of all elements in the collection
        */
        ::std::vector< OUString >
                getNames() const;

        /** merges the content of another collection into |this|
            @param _rAdditionalValues
                the collection whose values are to be merged
            @param _bOverwriteExisting
                defines whether or not elements which are already present in |this|
                should be overwritten (<TRUE/>) or preserved (<FALSE/>).
            @return |*this|
        */
        NamedValueCollection&
                merge(
                    const NamedValueCollection& _rAdditionalValues,
                    bool _bOverwriteExisting
                );

        /** retrieves a value with a given name from the collection, if it is present

            @param _pAsciiValueName
                the ASCII name of the value to retrieve

            @param _out_rValue
                is the output parameter taking the desired value upon successful return. If
                a value with the given name is not present in the collection, or if a wrong-typed
                value is present, then this parameter will not be touched.

            @return
                <TRUE/> if there is a value with the given name, which could successfully
                be extraced. In this case, <arg>_out_rValue</arg> will contain the requested
                value.<br/>
                <FALSE/>, if there is no value with the given name.
            @throws IllegalArgumentException
                in case there is a value with the given name, but it cannot legally assigned to
                _out_rValue.
        */
        template < typename VALUE_TYPE >
        bool get_ensureType( const sal_Char* _pAsciiValueName, VALUE_TYPE& _out_rValue ) const
        {
            return get_ensureType( OUString::createFromAscii( _pAsciiValueName ), &_out_rValue, ::cppu::UnoType< VALUE_TYPE >::get() );
        }

        template < typename VALUE_TYPE >
        bool    get_ensureType( const OUString& _rValueName, VALUE_TYPE& _out_rValue ) const
        {
            return get_ensureType( _rValueName, &_out_rValue, ::cppu::UnoType< VALUE_TYPE >::get() );
        }

        /** retrieves a value with a given name, or defaults it to a given value, if its not present
            in the collection
        */
        template < typename VALUE_TYPE >
        VALUE_TYPE  getOrDefault( const sal_Char* _pAsciiValueName, const VALUE_TYPE& _rDefault ) const
        {
            return getOrDefault( OUString::createFromAscii( _pAsciiValueName ), _rDefault );
        }

        template < typename VALUE_TYPE >
        VALUE_TYPE  getOrDefault( const OUString& _rValueName, const VALUE_TYPE& _rDefault ) const
        {
            VALUE_TYPE retVal( _rDefault );
            get_ensureType( _rValueName, retVal );
            return retVal;
        }

        /** retrieves a (untyped) value with a given name

            If the collection does not contain a value with the given name, an empty
            Any is returned.
        */
        const css::uno::Any& get( const sal_Char* _pAsciiValueName ) const
        {
            return get( OUString::createFromAscii( _pAsciiValueName ) );
        }

        /** retrieves a (untyped) value with a given name

            If the collection does not contain a value with the given name, an empty
            Any is returned.
        */
        const css::uno::Any& get( const OUString& _rValueName ) const
        {
            return impl_get( _rValueName );
        }

        /// determines whether a value with a given name is present in the collection
        inline bool has( const sal_Char* _pAsciiValueName ) const
        {
            return impl_has( OUString::createFromAscii( _pAsciiValueName ) );
        }

        /// determines whether a value with a given name is present in the collection
        inline bool has( const OUString& _rValueName ) const
        {
            return impl_has( _rValueName );
        }

        /** puts a value into the collection

            @return <TRUE/> if and only if a value was already present previously, in
                which case it has been overwritten.
        */
        template < typename VALUE_TYPE >
        inline bool put( const sal_Char* _pAsciiValueName, const VALUE_TYPE& _rValue )
        {
            return impl_put( OUString::createFromAscii( _pAsciiValueName ), css::uno::makeAny( _rValue ) );
        }

        /** puts a value into the collection

            @return <TRUE/> if and only if a value was already present previously, in
                which case it has been overwritten.
        */
        template < typename VALUE_TYPE >
        inline bool put( const OUString& _rValueName, const VALUE_TYPE& _rValue )
        {
            return impl_put( _rValueName, css::uno::makeAny( _rValue ) );
        }

        inline bool put( const sal_Char* _pAsciiValueName, const css::uno::Any& _rValue )
        {
            return impl_put( OUString::createFromAscii( _pAsciiValueName ), _rValue );
        }

        inline bool put( const OUString& _rValueName, const css::uno::Any& _rValue )
        {
            return impl_put( _rValueName, _rValue );
        }

        /** removes the value with the given name from the collection

            @return <TRUE/> if and only if a value with the given name existed in the collection.
        */
        inline bool remove( const sal_Char* _pAsciiValueName )
        {
            return impl_remove( OUString::createFromAscii( _pAsciiValueName ) );
        }

        /** removes the value with the given name from the collection

            @return <TRUE/> if and only if a value with the given name existed in the collection.
        */
        inline bool remove( const OUString& _rValueName )
        {
            return impl_remove( _rValueName );
        }

        /** transforms the collection to a sequence of PropertyValues

            @return
                the  number of elements in the sequence
        */
        sal_Int32 operator >>= ( css::uno::Sequence< css::beans::PropertyValue >& _out_rValues ) const;

        /** transforms the collection to a sequence of NamedValues

            @return
                the  number of elements in the sequence
        */
        sal_Int32 operator >>= ( css::uno::Sequence< css::beans::NamedValue >& _out_rValues ) const;

        /** transforms the collection into a sequence of PropertyValues
        */
        inline css::uno::Sequence< css::beans::PropertyValue >
                getPropertyValues() const
        {
            css::uno::Sequence< css::beans::PropertyValue > aValues;
            *this >>= aValues;
            return aValues;
        }

        /** returns a Sequence< Any >, containing PropertyValues
        */
        inline css::uno::Sequence< css::uno::Any >
                getWrappedPropertyValues() const
        {
            return impl_wrap< css::beans::PropertyValue >();
        }

        /** returns a Sequence< Any >, containing NamedValues
        */
        inline css::uno::Sequence< css::uno::Any >
                getWrappedNamedValues() const
        {
            return impl_wrap< css::beans::NamedValue >();
        }

        /** transforms the collection into a sequence of NamedValues
        */
        inline css::uno::Sequence< css::beans::NamedValue >
                getNamedValues() const
        {
            css::uno::Sequence< css::beans::NamedValue > aValues;
            *this >>= aValues;
            return aValues;
        }

    private:
        void    impl_assign( const css::uno::Any& i_rWrappedElements );
        void    impl_assign( const css::uno::Sequence< css::uno::Any >& _rArguments );
        void    impl_assign( const css::uno::Sequence< css::beans::PropertyValue >& _rArguments );
        void    impl_assign( const css::uno::Sequence< css::beans::NamedValue >& _rArguments );

        bool    get_ensureType(
                    const OUString& _rValueName,
                    void* _pValueLocation,
                    const css::uno::Type& _rExpectedValueType
                ) const;

        const css::uno::Any&
                impl_get( const OUString& _rValueName ) const;

        bool    impl_has( const OUString& _rValueName ) const;

        bool    impl_put( const OUString& _rValueName, const css::uno::Any& _rValue );

        bool    impl_remove( const OUString& _rValueName );

        template< class VALUE_TYPE >
        css::uno::Sequence< css::uno::Any > impl_wrap() const
        {
            css::uno::Sequence< VALUE_TYPE > aValues;
            *this >>= aValues;
            css::uno::Sequence< css::uno::Any > aWrappedValues( aValues.getLength() );

            css::uno::Any* pO = aWrappedValues.getArray();
            const VALUE_TYPE* pV = aValues.getConstArray();
            const sal_Int32 nLen = aValues.getLength();
            for( sal_Int32 i = 0; i < nLen; ++i )
                *(pO++) = css::uno::makeAny<VALUE_TYPE>( *(pV++) );

            return aWrappedValues;
        }
    };


} // namespace comphelper


#endif // INCLUDED_COMPHELPER_NAMEDVALUECOLLECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
