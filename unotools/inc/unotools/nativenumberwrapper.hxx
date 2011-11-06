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


#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_NATIVENUMBERWRAPPER_HXX
#define _UNOTOOLS_NATIVENUMBERWRAPPER_HXX
#include <com/sun/star/i18n/XNativeNumberSupplier.hpp>

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}


class UNOTOOLS_DLLPUBLIC NativeNumberWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XNativeNumberSupplier >   xNNS;
                                // not implemented, prevent usage
                                NativeNumberWrapper( const NativeNumberWrapper& );
            NativeNumberWrapper&    operator=( const NativeNumberWrapper& );

public:
                                NativeNumberWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF
                                    );

                                ~NativeNumberWrapper();

    // Wrapper implementations of XNativeNumberSupplier

    ::rtl::OUString getNativeNumberString(
                        const ::rtl::OUString& rNumberString,
                        const ::com::sun::star::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    sal_Bool        isValidNatNum(
                        const ::com::sun::star::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    ::com::sun::star::i18n::NativeNumberXmlAttributes convertToXmlAttributes(
                        const ::com::sun::star::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    sal_Int16       convertFromXmlAttributes(
                        const ::com::sun::star::i18n::NativeNumberXmlAttributes& rAttr ) const;

};

#endif // _UNOTOOLS_NATIVENUMBERWRAPPER_HXX
