/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: converter.hxx,v $
 * $Revision: 1.8 $
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

#ifndef __FRAMEWORK_CLASSES_CONVERTER_HXX_
#define __FRAMEWORK_CLASSES_CONVERTER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <general.h>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <tools/datetime.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

class Converter
{
    public:
        // Seq<Any> <=> Seq<beans.PropertyValue>
        static css::uno::Sequence< css::uno::Any >             convert_seqProp2seqAny          ( const css::uno::Sequence< css::beans::PropertyValue >& lSource );
        static css::uno::Sequence< css::beans::PropertyValue > convert_seqAny2seqProp          ( const css::uno::Sequence< css::uno::Any >&             lSource );

        // Seq<beans.NamedValue> <=> Seq<beans.PropertyValue>
        static css::uno::Sequence< css::beans::PropertyValue > convert_seqNamedVal2seqPropVal  ( const css::uno::Sequence< css::beans::NamedValue >&    lSource );
        static css::uno::Sequence< css::beans::NamedValue >    convert_seqPropVal2seqNamedVal  ( const css::uno::Sequence< css::beans::PropertyValue >& lSource );

        // Seq<String> => Vector<String>
        static OUStringList                                    convert_seqOUString2OUStringList( const css::uno::Sequence< ::rtl::OUString >&           lSource );
        static css::uno::Sequence< ::rtl::OUString >           convert_OUStringList2seqOUString( const OUStringList&                                    lSource );

        static css::uno::Sequence< css::beans::PropertyValue > convert_OUStringHash2seqProp    ( const OUStringHash&                                    lSource );
        static OUStringHash                                    convert_seqProp2OUStringHash    ( const css::uno::Sequence< css::beans::PropertyValue >& lSource );

        // String <=> tools.DateTime
        static DateTime                                        convert_String2DateTime         ( const ::rtl::OUString&                                 sSource );
        static ::rtl::OUString                                 convert_DateTime2String         ( const DateTime&                                        aSource );
        static ::rtl::OUString                                 convert_DateTime2ISO8601        ( const DateTime&                                        aSource );
};

}       // namespace framework

#endif  // #ifndef __FRAMEWORK_CLASSES_CONVERTER_HXX_
