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
    using ::com::sun::star::i18n::XCollator;
    /** === end UNO using === **/

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
        default:
            break;
        }
        return pComparator;
    }

//......................................................................................................................
} // namespace comphelper
//......................................................................................................................
