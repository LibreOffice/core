/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <fwidllapi.h>

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

class FWI_DLLPUBLIC Converter
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
