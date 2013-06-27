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

#include "comphelper/anycompare.hxx"

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

namespace comphelper
{
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
            // FIXME Timezone?

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
            // FIXME Timezone?

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

            if ( lhs.NanoSeconds < rhs.NanoSeconds )
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
            // FIXME Timezone?

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

            if ( lhs.NanoSeconds < rhs.NanoSeconds )
                return true;
            return false;
        }
    };

    //------------------------------------------------------------------------------------------------------------------
    ::std::auto_ptr< IKeyPredicateLess > getStandardLessPredicate( Type const & i_type, Reference< XCollator > const & i_collator )
    {
	SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< IKeyPredicateLess > pComparator;
	SAL_WNODEPRECATED_DECLARATIONS_POP
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
