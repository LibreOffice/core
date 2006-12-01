/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namedvaluecollection.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 17:32:25 $
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

#ifndef COMPHELPER_NAMEDVALUECOLLECTION_HXX
#define COMPHELPER_NAMEDVALUECOLLECTION_HXX

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include <comphelper/comphelperdllapi.h>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
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

        /** retrieves a value with a given name from the collection, if it is present

            @param _pAsciiValueName
                the ASCII name of the value to retrieve

            @param _out_rValue
                is the output parameter taking the desired value upon successful return. If
                a value with the given name is not present in the collection, or if a wrong-typed
                value is present, then this parameter will not be touched.

            @return
                <TRUE/> if there either is no value with the given name, or there is
                such a value which could be successfully extraced. In the latter case,
                <arg>_out_rValue</arg> will contain the requested value.<br/>
                If there is a value with the given name, but of wrong type, <FALSE/> will
                be returned.
        */
        template < typename VALUE_TYPE >
        bool    getIfExists_ensureType( const sal_Char* _pAsciiValueName, VALUE_TYPE& _out_rValue ) const
        {
            return getIfExists_ensureType( ::rtl::OUString::createFromAscii( _pAsciiValueName ), &_out_rValue, ::cppu::UnoType< VALUE_TYPE >::get() );
        }

        template < typename VALUE_TYPE >
        bool    getIfExists_ensureType( const ::rtl::OUString& _rValueName, VALUE_TYPE& _out_rValue ) const
        {
            return getIfExists_ensureType( _rValueName, &_out_rValue, ::cppu::UnoType< VALUE_TYPE >::get() );
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
            getIfExists_ensureType( _rValueName, retVal );
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

        const ::com::sun::star::uno::Any& get( const ::rtl::OUString& _rValueName ) const
        {
            return impl_get( _rValueName );
        }

    private:
        void    impl_assign( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments );
        void    impl_assign( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArguments );
        void    impl_assign( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rArguments );

        bool    getIfExists_ensureType(
                    const ::rtl::OUString& _rValueName,
                    void* _pValueLocation,
                    const ::com::sun::star::uno::Type& _rExpectedValueType
                ) const;

        const ::com::sun::star::uno::Any&
                impl_get( const ::rtl::OUString& _rValueName ) const;
    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_NAMEDVALUECOLLECTION_HXX

