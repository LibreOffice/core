/*************************************************************************
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

#ifndef COMPHELPER_ANYCOMPARE_HXX
#define COMPHELPER_ANYCOMPARE_HXX

#include "comphelper/comphelperdllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
/** === end UNO includes === **/

#include <comphelper/extract.hxx>

#include <functional>
#include <memory>

//......................................................................................................................
namespace comphelper
{
//......................................................................................................................

    //==================================================================================================================
    //= IKeyPredicateLess
    //==================================================================================================================
    class SAL_NO_VTABLE IKeyPredicateLess
    {
    public:
        virtual bool isLess( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const = 0;
        virtual ~IKeyPredicateLess() {}
    };

    //==================================================================================================================
    //= LessPredicateAdapter
    //==================================================================================================================
    struct LessPredicateAdapter : public ::std::binary_function< ::com::sun::star::uno::Any, ::com::sun::star::uno::Any, bool >
    {
        LessPredicateAdapter( const IKeyPredicateLess& _predicate )
            :m_predicate( _predicate )
        {
        }

        bool operator()( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const
        {
            return m_predicate.isLess( _lhs, _rhs );
        }

    private:
        IKeyPredicateLess const &   m_predicate;

    private:
        LessPredicateAdapter(); // never implemented
    };

    //==================================================================================================================
    //= ScalarPredicateLess
    //==================================================================================================================
    template< typename SCALAR >
    class ScalarPredicateLess : public IKeyPredicateLess
    {
    public:
        virtual bool isLess( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const
        {
            SCALAR lhs(0), rhs(0);
            if  (   !( _lhs >>= lhs )
                ||  !( _rhs >>= rhs )
                )
                throw ::com::sun::star::lang::IllegalArgumentException();
            return lhs < rhs;
        }
    };

    //==================================================================================================================
    //= StringPredicateLess
    //==================================================================================================================
    class StringPredicateLess : public IKeyPredicateLess
    {
    public:
        virtual bool isLess( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const
        {
            ::rtl::OUString lhs, rhs;
            if  (   !( _lhs >>= lhs )
                ||  !( _rhs >>= rhs )
                )
                throw ::com::sun::star::lang::IllegalArgumentException();
            return lhs < rhs;
        }
    };

    //==================================================================================================================
    //= StringCollationPredicateLess
    //==================================================================================================================
    class StringCollationPredicateLess : public IKeyPredicateLess
    {
    public:
        StringCollationPredicateLess( ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator > const & i_collator )
            :m_collator( i_collator )
        {
        }

        virtual bool isLess( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const
        {
            ::rtl::OUString lhs, rhs;
            if  (   !( _lhs >>= lhs )
                ||  !( _rhs >>= rhs )
                )
                throw ::com::sun::star::lang::IllegalArgumentException();
            return m_collator->compareString( lhs, rhs ) < 0;
        }

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator > const m_collator;
    };

    //==================================================================================================================
    //= TypePredicateLess
    //==================================================================================================================
    class TypePredicateLess : public IKeyPredicateLess
    {
    public:
        virtual bool isLess( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const
        {
            ::com::sun::star::uno::Type lhs, rhs;
            if  (   !( _lhs >>= lhs )
                ||  !( _rhs >>= rhs )
                )
                throw ::com::sun::star::lang::IllegalArgumentException();
            return lhs.getTypeName() < rhs.getTypeName();
        }
    };

    //==================================================================================================================
    //= EnumPredicateLess
    //==================================================================================================================
    class EnumPredicateLess : public IKeyPredicateLess
    {
    public:
        EnumPredicateLess( ::com::sun::star::uno::Type const & _enumType )
            :m_enumType( _enumType )
        {
        }

        virtual bool isLess( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const
        {
            sal_Int32 lhs(0), rhs(0);
            if  (   !::cppu::enum2int( lhs, _lhs )
                ||  !::cppu::enum2int( rhs, _rhs )
                ||  !_lhs.getValueType().equals( m_enumType )
                ||  !_rhs.getValueType().equals( m_enumType )
                )
                throw ::com::sun::star::lang::IllegalArgumentException();
            return lhs < rhs;
        }

    private:
        ::com::sun::star::uno::Type const   m_enumType;
    };

    //==================================================================================================================
    //= InterfacePredicateLess
    //==================================================================================================================
    class InterfacePredicateLess : public IKeyPredicateLess
    {
    public:
        virtual bool isLess( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const
        {
            if  (   ( _lhs.getValueTypeClass() != ::com::sun::star::uno::TypeClass_INTERFACE )
                ||  ( _rhs.getValueTypeClass() != ::com::sun::star::uno::TypeClass_INTERFACE )
                )
                throw ::com::sun::star::lang::IllegalArgumentException();

            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > lhs( _lhs, ::com::sun::star::uno::UNO_QUERY );
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > rhs( _rhs, ::com::sun::star::uno::UNO_QUERY );
            return lhs.get() < rhs.get();
        }
    };

    //==================================================================================================================
    //= getStandardLessPredicate
    //==================================================================================================================
    /** creates a default IKeyPredicateLess instance for the given UNO type
        @param i_type
            the type for which a predicate instance should be created
        @param i_collator
            specifies a collator instance to use, or <NULL/>. If <NULL/>, strings will be compared using the <code>&lt</code>
            operator, otherwise the collator will be used. The parameter is ignored if <arg>i_type</arg> does not specify
            the string type.
        @return
            a default implementation of IKeyPredicateLess, which is able to compare values of the given type. If no
            such default implementation is known for the given type, then <NULL/> is returned.
    */
    ::std::auto_ptr< IKeyPredicateLess > COMPHELPER_DLLPUBLIC
        getStandardLessPredicate(
            ::com::sun::star::uno::Type const & i_type,
            ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator > const & i_collator
        );

//......................................................................................................................
} // namespace comphelper
//......................................................................................................................

#endif // COMPHELPER_ANYCOMPARE_HXX
