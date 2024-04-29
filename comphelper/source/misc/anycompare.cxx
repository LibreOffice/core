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
#include <optional>
#include <comphelper/anycompare.hxx>
#include <typelib/typedescription.hxx>

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include "typedescriptionref.hxx"

namespace comphelper
{
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::TypeDescription;
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
    using ::comphelper::detail::TypeDescriptionRef;

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
                throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
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
                throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
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
                throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
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

    bool anyLess( void const * lhs, typelib_TypeDescriptionReference * lhsType,
                  void const * rhs, typelib_TypeDescriptionReference * rhsType );

    // For compound types we need to compare them member by member until we've
    // checked them all or found a member that differs. For inequality checks
    // we need to call anyLess() twice in both directions, this function does that.
    std::optional<bool> anyCompare( void const * lhs, typelib_TypeDescriptionReference * lhsType,
              void const * rhs, typelib_TypeDescriptionReference * rhsType )
    {
        if( anyLess( lhs, lhsType, rhs, rhsType ))
            return std::optional( true );
        if( anyLess( rhs, rhsType, lhs, lhsType ))
            return std::optional( false );
        return std::nullopt; // equal, so can't yet tell if anyLess() should return
    }

    // This is typelib_typedescription_equals(), but returns -1/0/1 values like strcmp().
    int compareTypes( const typelib_TypeDescription * lhsType,
                      const typelib_TypeDescription * rhsType )
    {
        if( lhsType == rhsType )
            return 0;
        if( lhsType->eTypeClass != rhsType->eTypeClass )
            return lhsType->eTypeClass - rhsType->eTypeClass;
        if( lhsType->pTypeName->length != rhsType->pTypeName->length )
            return lhsType->pTypeName->length - rhsType->pTypeName->length;
        return rtl_ustr_compare( lhsType->pTypeName->buffer, rhsType->pTypeName->buffer );
    }

    bool anyLess( void const * lhs, typelib_TypeDescriptionReference * lhsType,
                  void const * rhs, typelib_TypeDescriptionReference * rhsType )
    {
        if (lhsType->eTypeClass != rhsType->eTypeClass)
            return lhsType->eTypeClass < rhsType->eTypeClass;

        if (lhsType->eTypeClass == typelib_TypeClass_VOID) {
            return false;
        }
        assert(lhs != nullptr);
        assert(rhs != nullptr);

        switch (lhsType->eTypeClass) {
        case typelib_TypeClass_INTERFACE:
            return lhs < rhs;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION: {
            TypeDescription lhsTypeDescr( lhsType );
            if (!lhsTypeDescr.is())
                lhsTypeDescr.makeComplete();
            if (!lhsTypeDescr.is())
                throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
            TypeDescription rhsTypeDescr( rhsType );
            if (!rhsTypeDescr.is())
                rhsTypeDescr.makeComplete();
            if (!rhsTypeDescr.is())
                throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
            int compare = compareTypes( lhsTypeDescr.get(), rhsTypeDescr.get());
            if( compare != 0 )
                return compare < 0;

            typelib_CompoundTypeDescription * compType =
                reinterpret_cast< typelib_CompoundTypeDescription * >(
                    lhsTypeDescr.get() );
            sal_Int32 nDescr = compType->nMembers;

            if (compType->pBaseTypeDescription) {
                std::optional<bool> subLess = anyCompare(
                    lhs, reinterpret_cast<
                    typelib_TypeDescription * >(
                        compType->pBaseTypeDescription)->pWeakRef,
                    rhs, reinterpret_cast<
                    typelib_TypeDescription * >(
                        compType->pBaseTypeDescription)->pWeakRef);
                if(subLess.has_value())
                    return *subLess;
            }

            typelib_TypeDescriptionReference ** ppTypeRefs =
                compType->ppTypeRefs;
            sal_Int32 * memberOffsets = compType->pMemberOffsets;

            for ( sal_Int32 nPos = 0; nPos < nDescr; ++nPos )
            {
                TypeDescriptionRef memberType( ppTypeRefs[ nPos ] );
                if (!memberType.is())
                    throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
                std::optional<bool> subLess = anyCompare(
                     static_cast< char const * >(
                         lhs ) + memberOffsets[ nPos ],
                     memberType->pWeakRef,
                     static_cast< char const * >(
                         rhs ) + memberOffsets[ nPos ],
                     memberType->pWeakRef);
                if(subLess.has_value())
                    return *subLess;
            }
            return false; // equal
        }
        case typelib_TypeClass_SEQUENCE: {
            uno_Sequence * lhsSeq = *static_cast< uno_Sequence * const * >(lhs);
            uno_Sequence * rhsSeq = *static_cast< uno_Sequence * const * >(rhs);
            if( lhsSeq->nElements != rhsSeq->nElements)
                return lhsSeq->nElements < rhsSeq->nElements;
            sal_Int32 nElements = lhsSeq->nElements;

            TypeDescriptionRef lhsTypeDescr( lhsType );
            if (!lhsTypeDescr.is())
                throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
            TypeDescriptionRef rhsTypeDescr( rhsType );
            if (!rhsTypeDescr.is())
                throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
            int compare = compareTypes( lhsTypeDescr.get(), rhsTypeDescr.get());
            if( compare != 0 )
                return compare < 0;

            typelib_TypeDescriptionReference * elementTypeRef =
                reinterpret_cast< typelib_IndirectTypeDescription * >(lhsTypeDescr.get())->pType;
            TypeDescriptionRef elementTypeDescr( elementTypeRef );
            if (!elementTypeDescr.is())
                throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
            assert( elementTypeDescr.equals( TypeDescriptionRef(
                        reinterpret_cast< typelib_IndirectTypeDescription * >(lhsTypeDescr.get())->pType )));

            sal_Int32 nElementSize = elementTypeDescr->nSize;
            if (nElements > 0)
            {
                char const * lhsElements = lhsSeq->elements;
                char const * rhsElements = rhsSeq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    std::optional<bool> subLess = anyCompare(
                        lhsElements + (nElementSize * nPos),
                        elementTypeDescr->pWeakRef,
                        rhsElements + (nElementSize * nPos),
                        elementTypeDescr->pWeakRef );
                    if(subLess.has_value())
                        return *subLess;
                }
            }
            return false; // equal
        }
        case typelib_TypeClass_ANY: {
            uno_Any const * lhsAny = static_cast< uno_Any const * >(lhs);
            uno_Any const * rhsAny = static_cast< uno_Any const * >(rhs);
            return anyLess( lhsAny->pData, lhsAny->pType, rhsAny->pData, rhsAny->pType );
        }
        case typelib_TypeClass_TYPE: {
            OUString const & lhsTypeName = OUString::unacquired(
                &(*static_cast< typelib_TypeDescriptionReference * const * >(lhs))->pTypeName);
            OUString const & rhsTypeName = OUString::unacquired(
                &(*static_cast< typelib_TypeDescriptionReference * const * >(rhs))->pTypeName);
            return lhsTypeName < rhsTypeName;
        }
        case typelib_TypeClass_STRING: {
            OUString const & lhsStr = OUString::unacquired(
                static_cast< rtl_uString * const * >(lhs) );
            OUString const & rhsStr = OUString::unacquired(
                static_cast< rtl_uString * const * >(rhs) );
            return lhsStr < rhsStr;
        }
        case typelib_TypeClass_ENUM: {
            TypeDescription lhsTypeDescr( lhsType );
            if (!lhsTypeDescr.is())
                lhsTypeDescr.makeComplete();
            if (!lhsTypeDescr.is())
                throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
            TypeDescription rhsTypeDescr( rhsType );
            if (!rhsTypeDescr.is())
                rhsTypeDescr.makeComplete();
            if (!rhsTypeDescr.is())
                throw css::lang::IllegalArgumentException(u"bad ordering"_ustr, css::uno::Reference<css::uno::XInterface>(), -1);
            int compare = compareTypes( lhsTypeDescr.get(), rhsTypeDescr.get());
            if( compare != 0 )
                return compare < 0;

            return *static_cast< int const * >(lhs) < *static_cast< int const * >(rhs);
        }
        case typelib_TypeClass_BOOLEAN:
            return *static_cast< sal_Bool const * >(lhs) < *static_cast< sal_Bool const * >(rhs);
        case typelib_TypeClass_CHAR:
            return *static_cast< sal_Unicode const * >(lhs) < *static_cast< sal_Unicode const * >(rhs);
        case typelib_TypeClass_FLOAT:
            return *static_cast< float const * >(lhs) < *static_cast< float const * >(rhs);
        case typelib_TypeClass_DOUBLE:
            return *static_cast< double const * >(lhs) < *static_cast< double const * >(rhs);
        case typelib_TypeClass_BYTE:
            return *static_cast< sal_Int8 const * >(lhs) < *static_cast< sal_Int8 const * >(rhs);
        case typelib_TypeClass_SHORT:
            return *static_cast< sal_Int16 const * >(lhs) < *static_cast< sal_Int16 const * >(rhs);
        case typelib_TypeClass_UNSIGNED_SHORT:
            return *static_cast< sal_uInt16 const * >(lhs) < *static_cast< sal_uInt16 const * >(rhs);
        case typelib_TypeClass_LONG:
            return *static_cast< sal_Int32 const * >(lhs) < *static_cast< sal_Int32 const * >(rhs);
        case typelib_TypeClass_UNSIGNED_LONG:
            return *static_cast< sal_uInt32 const * >(lhs) < *static_cast< sal_uInt32 const * >(rhs);
        case typelib_TypeClass_HYPER:
            return *static_cast< sal_Int64 const * >(lhs) < *static_cast< sal_Int64 const * >(rhs);
        case typelib_TypeClass_UNSIGNED_HYPER:
            return *static_cast< sal_uInt64 const * >(lhs) < *static_cast< sal_uInt64 const * >(rhs);
    //     case typelib_TypeClass_UNKNOWN:
    //     case typelib_TypeClass_SERVICE:
    //     case typelib_TypeClass_MODULE:
        default:
            return false;
        }
    }

    } // namespace

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
        return anyLess( lhs.getValue(), lhs.getValueTypeRef(), rhs.getValue(), rhs.getValueTypeRef());
    }

} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
