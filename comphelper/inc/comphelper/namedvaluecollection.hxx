/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: namedvaluecollection.hxx,v $
 * $Revision: 1.9 $
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

#ifndef COMPHELPER_NAMEDVALUECOLLECTION_HXX
#define COMPHELPER_NAMEDVALUECOLLECTION_HXX

#include <comphelper/comphelperdllapi.h>

/** === begin UNO includes === **/
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
/** === end UNO includes === **/

#include <memory>

//........................................................................
namespace comphelper
{
//........................................................................

    // ====================================================================
    // = NamedValueCollection
    // ====================================================================
    struct NamedValueCollection_Impl;
    /** a collection of named values, packed in various formats.
    */
    class COMPHELPER_DLLPUBLIC NamedValueCollection
    {
    private:
        ::std::auto_ptr< NamedValueCollection_Impl >    m_pImpl;

    public:
        NamedValueCollection();

        /** constructs a collection
            @param  _rElements
                the wrapped elements of the collection. The <code>Any</code> might contain a sequence of
                property values, a sequence of named values, or directly a property value or named value.
                All other cases are worth an assertion in non-product builds.
        */
        NamedValueCollection( const ::com::sun::star::uno::Any& _rElements );

        /** constructs a collection
            @param _rArguments
                a sequence of Any's containing either PropertyValue's or NamedValue's.
        */
        NamedValueCollection( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments );

        /** constructs a collection
            @param _rArguments
                a sequence of PropertyValues's
        */
        NamedValueCollection( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArguments );

        /** constructs a collection
            @param _rArguments
                a sequence of NamedValue's
        */
        NamedValueCollection( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rArguments );

        ~NamedValueCollection();

        inline void assign( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments )
        {
            impl_assign( _rArguments );
        }

        inline void assign( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArguments )
        {
            impl_assign( _rArguments );
        }

        inline void assign( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rArguments )
        {
            impl_assign( _rArguments );
        }

        /// returns the number of elements in the collection
        size_t  size() const;

        /// determines whether the collection is empty
        bool    empty() const;

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
            return get_ensureType( ::rtl::OUString::createFromAscii( _pAsciiValueName ), &_out_rValue, ::cppu::UnoType< VALUE_TYPE >::get() );
        }

        template < typename VALUE_TYPE >
        bool    get_ensureType( const ::rtl::OUString& _rValueName, VALUE_TYPE& _out_rValue ) const
        {
            return get_ensureType( _rValueName, &_out_rValue, ::cppu::UnoType< VALUE_TYPE >::get() );
        }

        /** retrieves a value with a given name, or defaults it to a given value, if its not present
            in the colllection
        */
        template < typename VALUE_TYPE >
        VALUE_TYPE  getOrDefault( const sal_Char* _pAsciiValueName, const VALUE_TYPE& _rDefault ) const
        {
            return getOrDefault( ::rtl::OUString::createFromAscii( _pAsciiValueName ), _rDefault );
        }

        template < typename VALUE_TYPE >
        VALUE_TYPE  getOrDefault( const ::rtl::OUString& _rValueName, const VALUE_TYPE& _rDefault ) const
        {
            VALUE_TYPE retVal( _rDefault );
            get_ensureType( _rValueName, retVal );
            return retVal;
        }

        /** retrieves a (untyped) value with a given name

            If the collection does not contain a value with the given name, an empty
            Any is returned.
        */
        const ::com::sun::star::uno::Any& get( const sal_Char* _pAsciiValueName ) const
        {
            return get( ::rtl::OUString::createFromAscii( _pAsciiValueName ) );
        }

        /** retrieves a (untyped) value with a given name

            If the collection does not contain a value with the given name, an empty
            Any is returned.
        */
        const ::com::sun::star::uno::Any& get( const ::rtl::OUString& _rValueName ) const
        {
            return impl_get( _rValueName );
        }

        /// determines whether a value with a given name is present in the collection
        inline bool has( const sal_Char* _pAsciiValueName ) const
        {
            return impl_has( ::rtl::OUString::createFromAscii( _pAsciiValueName ) );
        }

        /// determines whether a value with a given name is present in the collection
        inline bool has( const ::rtl::OUString& _rValueName ) const
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
            return impl_put( ::rtl::OUString::createFromAscii( _pAsciiValueName ), ::com::sun::star::uno::makeAny( _rValue ) );
        }

        /** puts a value into the collection

            @return <TRUE/> if and only if a value was already present previously, in
                which case it has been overwritten.
        */
        template < typename VALUE_TYPE >
        inline bool put( const ::rtl::OUString& _rValueName, const VALUE_TYPE& _rValue )
        {
            return impl_put( _rValueName, ::com::sun::star::uno::makeAny( _rValue ) );
        }

        inline bool put( const sal_Char* _pAsciiValueName, const ::com::sun::star::uno::Any& _rValue )
        {
            return impl_put( ::rtl::OUString::createFromAscii( _pAsciiValueName ), _rValue );
        }

        inline bool put( const ::rtl::OUString& _rValueName, const ::com::sun::star::uno::Any& _rValue )
        {
            return impl_put( _rValueName, _rValue );
        }

        /** removes the value with the given name from the collection

            @return <TRUE/> if and only if a value with the given name existed in the collection.
        */
        inline bool remove( const sal_Char* _pAsciiValueName )
        {
            return impl_remove( ::rtl::OUString::createFromAscii( _pAsciiValueName ) );
        }

        /** removes the value with the given name from the collection

            @return <TRUE/> if and only if a value with the given name existed in the collection.
        */
        inline bool remove( const ::rtl::OUString& _rValueName )
        {
            return impl_remove( _rValueName );
        }

        /** transforms the collection to a sequence of PropertyValues

            @return
                the  number of elements in the sequence
        */
        sal_Int32 operator >>= ( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _out_rValues ) const;

        /** transforms the collection to a sequence of NamedValues

            @return
                the  number of elements in the sequence
        */
        sal_Int32 operator >>= ( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _out_rValues ) const;

        /** transforms the collection into a sequence of PropertyValues
        */
        inline ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                getPropertyValues() const
        {
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aValues;
            *this >>= aValues;
            return aValues;
        }

        /** transforms the collection into a sequence of NamedValues
        */
        inline ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >
                getNamedValues() const
        {
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > aValues;
            *this >>= aValues;
            return aValues;
        }

    private:
        void    impl_assign( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments );
        void    impl_assign( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArguments );
        void    impl_assign( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rArguments );

        bool    get_ensureType(
                    const ::rtl::OUString& _rValueName,
                    void* _pValueLocation,
                    const ::com::sun::star::uno::Type& _rExpectedValueType
                ) const;

        const ::com::sun::star::uno::Any&
                impl_get( const ::rtl::OUString& _rValueName ) const;

        bool    impl_has( const ::rtl::OUString& _rValueName ) const;

        bool    impl_put( const ::rtl::OUString& _rValueName, const ::com::sun::star::uno::Any& _rValue );

        bool    impl_remove( const ::rtl::OUString& _rValueName );
    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_NAMEDVALUECOLLECTION_HXX

