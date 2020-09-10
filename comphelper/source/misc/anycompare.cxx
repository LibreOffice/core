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

#include <memory>
#include <comphelper/anycompare.hxx>

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

namespace comphelper
{
    using ::com::sun::star::uno::Reference;
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

    namespace {

    class DatePredicateLess : public IKeyPredicateLess
    {
    public:
        virtual bool isLess( css::uno::Any const & _lhs, css::uno::Any const & _rhs ) const override
        {
            Date lhs, rhs;
            if  (   !( _lhs >>= lhs )
                ||  !( _rhs >>= rhs )
                )
                throw css::lang::IllegalArgumentException();
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

    class TimePredicateLess : public IKeyPredicateLess
    {
    public:
        virtual bool isLess( css::uno::Any const & _lhs, css::uno::Any const & _rhs ) const override
        {
            Time lhs, rhs;
            if  (   !( _lhs >>= lhs )
                ||  !( _rhs >>= rhs )
                )
                throw css::lang::IllegalArgumentException();
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

    class DateTimePredicateLess : public IKeyPredicateLess
    {
    public:
        virtual bool isLess( css::uno::Any const & _lhs, css::uno::Any const & _rhs ) const override
        {
            DateTime lhs, rhs;
            if  (   !( _lhs >>= lhs )
                ||  !( _rhs >>= rhs )
                )
                throw css::lang::IllegalArgumentException();
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

    }

    std::unique_ptr< IKeyPredicateLess > getStandardLessPredicate( Type const & i_type, Reference< XCollator > const & i_collator )
    {
        std::unique_ptr< IKeyPredicateLess > pComparator;
        switch ( i_type.getTypeClass() )
        {
        case TypeClass_CHAR:
            pComparator.reset( new ScalarPredicateLess< sal_Unicode > );
            break;
        case TypeClass_BOOLEAN:
            pComparator.reset( new ScalarPredicateLess< bool > );
            break;
        case TypeClass_BYTE:
            pComparator.reset( new ScalarPredicateLess< sal_Int8 > );
            break;
        case TypeClass_SHORT:
            pComparator.reset( new ScalarPredicateLess< sal_Int16 > );
            break;
        case TypeClass_UNSIGNED_SHORT:
            pComparator.reset( new ScalarPredicateLess< sal_uInt16 > );
            break;
        case TypeClass_LONG:
            pComparator.reset( new ScalarPredicateLess< sal_Int32 > );
            break;
        case TypeClass_UNSIGNED_LONG:
            pComparator.reset( new ScalarPredicateLess< sal_uInt32 > );
            break;
        case TypeClass_HYPER:
            pComparator.reset( new ScalarPredicateLess< sal_Int64 > );
            break;
        case TypeClass_UNSIGNED_HYPER:
            pComparator.reset( new ScalarPredicateLess< sal_uInt64 > );
            break;
        case TypeClass_FLOAT:
            pComparator.reset( new ScalarPredicateLess< float > );
            break;
        case TypeClass_DOUBLE:
            pComparator.reset( new ScalarPredicateLess< double > );
            break;
        case TypeClass_STRING:
            if ( i_collator.is() )
                pComparator.reset( new StringCollationPredicateLess( i_collator ) );
            else
                pComparator.reset( new StringPredicateLess );
            break;
        case TypeClass_TYPE:
            pComparator.reset( new TypePredicateLess );
            break;
        case TypeClass_ENUM:
            pComparator.reset( new EnumPredicateLess( i_type ) );
            break;
        case TypeClass_INTERFACE:
            pComparator.reset( new InterfacePredicateLess );
            break;
        case TypeClass_STRUCT:
            if ( i_type.equals( ::cppu::UnoType< Date >::get() ) )
                pComparator.reset( new DatePredicateLess );
            else if ( i_type.equals( ::cppu::UnoType< Time >::get() ) )
                pComparator.reset( new TimePredicateLess );
            else if ( i_type.equals( ::cppu::UnoType< DateTime >::get() ) )
                pComparator.reset( new DateTimePredicateLess );
            break;
        default:
            break;
        }
        return pComparator;
    }

    bool anyLess( css::uno::Any const & lhs, css::uno::Any const & rhs)
    {
        auto lhsTypeClass = lhs.getValueType().getTypeClass();
        auto rhsTypeClass = rhs.getValueType().getTypeClass();
        if (lhsTypeClass != rhsTypeClass)
            return lhsTypeClass < rhsTypeClass;

        switch ( lhsTypeClass )
        {
        case TypeClass_CHAR:
            return ScalarPredicateLess< sal_Unicode >().isLess(lhs, rhs);
        case TypeClass_BOOLEAN:
            return ScalarPredicateLess< bool >().isLess(lhs, rhs);
            break;
        case TypeClass_BYTE:
            return ScalarPredicateLess< sal_Int8 >().isLess(lhs, rhs);
            break;
        case TypeClass_SHORT:
            return ScalarPredicateLess< sal_Int16 >().isLess(lhs, rhs);
            break;
        case TypeClass_UNSIGNED_SHORT:
            return ScalarPredicateLess< sal_uInt16 >().isLess(lhs, rhs);
            break;
        case TypeClass_LONG:
            return ScalarPredicateLess< sal_Int32 >().isLess(lhs, rhs);
            break;
        case TypeClass_UNSIGNED_LONG:
            return ScalarPredicateLess< sal_uInt32 >().isLess(lhs, rhs);
            break;
        case TypeClass_HYPER:
            return ScalarPredicateLess< sal_Int64 >().isLess(lhs, rhs);
            break;
        case TypeClass_UNSIGNED_HYPER:
            return ScalarPredicateLess< sal_uInt64 >().isLess(lhs, rhs);
            break;
        case TypeClass_FLOAT:
            return ScalarPredicateLess< float >().isLess(lhs, rhs);
            break;
        case TypeClass_DOUBLE:
            return ScalarPredicateLess< double >().isLess(lhs, rhs);
            break;
        case TypeClass_STRING:
            return StringPredicateLess().isLess(lhs, rhs);
            break;
        case TypeClass_TYPE:
            return TypePredicateLess().isLess(lhs, rhs);
            break;
        case TypeClass_ENUM:
            return EnumPredicateLess( lhs.getValueType() ).isLess(lhs, rhs);
            break;
        case TypeClass_INTERFACE:
            return InterfacePredicateLess().isLess(lhs, rhs);
            break;
        case TypeClass_STRUCT:
            if ( lhs.getValueType().equals( ::cppu::UnoType< Date >::get() ) )
                return DatePredicateLess().isLess(lhs, rhs);
            else if ( lhs.getValueType().equals( ::cppu::UnoType< Time >::get() ) )
                return TimePredicateLess().isLess(lhs, rhs);
            else if ( lhs.getValueType().equals( ::cppu::UnoType< DateTime >::get() ) )
                return DateTimePredicateLess().isLess(lhs, rhs);
            break;
        default: ;
        }

        // type==VOID
        return false;
    }

} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
