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

#include "precompiled_comphelper.hxx"

#include "comphelper/anycompare.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
/** === end UNO includes === **/

//......................................................................................................................
namespace comphelper
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::TypeClass_CHAR;
    using ::com::sun::star::uno::TypeClass_BOOLEAN;
    using ::com::sun::star::uno::TypeClass_BYTE;
    using ::com::sun::star::uno::TypeClass_SHORT;
    using ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT;
    using ::com::sun::star::uno::TypeClass_LONG;
    using ::com::sun::star::uno::TypeClass_UNSIGNED_LONG;
    using ::com::sun::star::uno::TypeClass_HYPER;
    using ::com::sun::star::uno::TypeClass_UNSIGNED_HYPER;
    using ::com::sun::star::uno::TypeClass_FLOAT;
    using ::com::sun::star::uno::TypeClass_DOUBLE;
    using ::com::sun::star::uno::TypeClass_STRING;
    using ::com::sun::star::uno::TypeClass_TYPE;
    using ::com::sun::star::uno::TypeClass_ENUM;
    using ::com::sun::star::uno::TypeClass_INTERFACE;
    using ::com::sun::star::uno::TypeClass_STRUCT;
    using ::com::sun::star::i18n::XCollator;
    using ::com::sun::star::util::Date;
    using ::com::sun::star::util::Time;
    using ::com::sun::star::util::DateTime;
    /** === end UNO using === **/

    //==================================================================================================================
    //= DatePredicateLess
    //==================================================================================================================
    class DatePredicateLess : public IKeyPredicateLess
    {
    public:
        virtual bool isLess( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const
        {
            Date lhs, rhs;
            if  (   !( _lhs >>= lhs )
                ||  !( _rhs >>= rhs )
                )
                throw ::com::sun::star::lang::IllegalArgumentException();

            if ( lhs.Year < rhs.Year )
                return true;
            if ( lhs.Year > rhs.Year )
                return false;

            if ( lhs.Month < rhs.Month )
                return true;
            if ( lhs.Month > rhs.Month )
                return false;

            if ( lhs.Day < rhs.Day )
                return true;
            return false;
        }
    };

    //==================================================================================================================
    //= TimePredicateLess
    //==================================================================================================================
    class TimePredicateLess : public IKeyPredicateLess
    {
    public:
        virtual bool isLess( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const
        {
            Time lhs, rhs;
            if  (   !( _lhs >>= lhs )
                ||  !( _rhs >>= rhs )
                )
                throw ::com::sun::star::lang::IllegalArgumentException();

            if ( lhs.Hours < rhs.Hours )
                return true;
            if ( lhs.Hours > rhs.Hours )
                return false;

            if ( lhs.Minutes < rhs.Minutes )
                return true;
            if ( lhs.Minutes > rhs.Minutes )
                return false;

            if ( lhs.Seconds < rhs.Seconds )
                return true;
            if ( lhs.Seconds > rhs.Seconds )
                return false;

            if ( lhs.HundredthSeconds < rhs.HundredthSeconds )
                return true;
            return false;
        }
    };

    //==================================================================================================================
    //= DateTimePredicateLess
    //==================================================================================================================
    class DateTimePredicateLess : public IKeyPredicateLess
    {
    public:
        virtual bool isLess( ::com::sun::star::uno::Any const & _lhs, ::com::sun::star::uno::Any const & _rhs ) const
        {
            DateTime lhs, rhs;
            if  (   !( _lhs >>= lhs )
                ||  !( _rhs >>= rhs )
                )
                throw ::com::sun::star::lang::IllegalArgumentException();

            if ( lhs.Year < rhs.Year )
                return true;
            if ( lhs.Year > rhs.Year )
                return false;

            if ( lhs.Month < rhs.Month )
                return true;
            if ( lhs.Month > rhs.Month )
                return false;

            if ( lhs.Day < rhs.Day )
                return true;
            if ( lhs.Day > rhs.Day )
                return false;

            if ( lhs.Hours < rhs.Hours )
                return true;
            if ( lhs.Hours > rhs.Hours )
                return false;

            if ( lhs.Minutes < rhs.Minutes )
                return true;
            if ( lhs.Minutes > rhs.Minutes )
                return false;

            if ( lhs.Seconds < rhs.Seconds )
                return true;
            if ( lhs.Seconds > rhs.Seconds )
                return false;

            if ( lhs.HundredthSeconds < rhs.HundredthSeconds )
                return true;
            return false;
        }
    };

    //------------------------------------------------------------------------------------------------------------------
    ::std::auto_ptr< IKeyPredicateLess > getStandardLessPredicate( Type const & i_type, Reference< XCollator > const & i_collator )
    {
        ::std::auto_ptr< IKeyPredicateLess > pComparator;
        switch ( i_type.getTypeClass() )
        {
        case TypeClass_CHAR:
            pComparator.reset( new ScalarPredicateLess< sal_Unicode >() );
            break;
        case TypeClass_BOOLEAN:
            pComparator.reset( new ScalarPredicateLess< sal_Bool >() );
            break;
        case TypeClass_BYTE:
            pComparator.reset( new ScalarPredicateLess< sal_Int8 >() );
            break;
        case TypeClass_SHORT:
            pComparator.reset( new ScalarPredicateLess< sal_Int16 >() );
            break;
        case TypeClass_UNSIGNED_SHORT:
            pComparator.reset( new ScalarPredicateLess< sal_uInt16 >() );
            break;
        case TypeClass_LONG:
            pComparator.reset( new ScalarPredicateLess< sal_Int32 >() );
            break;
        case TypeClass_UNSIGNED_LONG:
            pComparator.reset( new ScalarPredicateLess< sal_uInt32 >() );
            break;
        case TypeClass_HYPER:
            pComparator.reset( new ScalarPredicateLess< sal_Int64 >() );
            break;
        case TypeClass_UNSIGNED_HYPER:
            pComparator.reset( new ScalarPredicateLess< sal_uInt64 >() );
            break;
        case TypeClass_FLOAT:
            pComparator.reset( new ScalarPredicateLess< float >() );
            break;
        case TypeClass_DOUBLE:
            pComparator.reset( new ScalarPredicateLess< double >() );
            break;
        case TypeClass_STRING:
            if ( i_collator.is() )
                pComparator.reset( new StringCollationPredicateLess( i_collator ) );
            else
                pComparator.reset( new StringPredicateLess() );
            break;
        case TypeClass_TYPE:
            pComparator.reset( new TypePredicateLess() );
            break;
        case TypeClass_ENUM:
            pComparator.reset( new EnumPredicateLess( i_type ) );
            break;
        case TypeClass_INTERFACE:
            pComparator.reset( new InterfacePredicateLess() );
            break;
        case TypeClass_STRUCT:
            if ( i_type.equals( ::cppu::UnoType< Date >::get() ) )
                pComparator.reset( new DatePredicateLess() );
            else if ( i_type.equals( ::cppu::UnoType< Time >::get() ) )
                pComparator.reset( new TimePredicateLess() );
            else if ( i_type.equals( ::cppu::UnoType< DateTime >::get() ) )
                pComparator.reset( new DateTimePredicateLess() );
            break;
        default:
            break;
        }
        return pComparator;
    }

//......................................................................................................................
} // namespace comphelper
//......................................................................................................................
