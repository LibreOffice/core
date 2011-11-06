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



#ifndef _TOXHLP_HXX
#define _TOXHLP_HXX

#include <tools/solar.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <swunodef.hxx>

namespace com { namespace sun { namespace star {
namespace i18n {
    class XExtendedIndexEntrySupplier;
}
namespace lang {
    class XMultiServiceFactory;
}
}}}


class String;

class IndexEntrySupplierWrapper
{
    STAR_NMSPC::lang::Locale aLcl;
    STAR_NMSPC::uno::Reference < com::sun::star::i18n::XExtendedIndexEntrySupplier > xIES;

public:
    IndexEntrySupplierWrapper(
            const STAR_NMSPC::lang::Locale& rLcl,
            STAR_REFERENCE( lang::XMultiServiceFactory )& rxMSF );
    ~IndexEntrySupplierWrapper();

    String GetIndexKey( const String& rTxt, const String& rTxtReading,
                        const STAR_NMSPC::lang::Locale& rLocale ) const;

    String GetFollowingText( sal_Bool bMorePages ) const;

    STAR_NMSPC::uno::Sequence< ::rtl::OUString >
    GetAlgorithmList( const STAR_NMSPC::lang::Locale& rLcl ) const;

    sal_Bool LoadAlgorithm( const STAR_NMSPC::lang::Locale& rLcl,
                            const String& sSortAlgorithm, long nOptions ) const;

    sal_Int16 CompareIndexEntry( const String& rTxt1, const String& rTxtReading1,
                                 const STAR_NMSPC::lang::Locale& rLcl1,
                                 const String& rTxt2, const String& rTxtReading2,
                                 const STAR_NMSPC::lang::Locale& rLcl2 ) const;
};

#endif
