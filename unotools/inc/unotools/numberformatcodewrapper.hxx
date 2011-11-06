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



#ifndef _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX
#define _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX

#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include "unotools/unotoolsdllapi.h"

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}


class UNOTOOLS_DLLPUBLIC NumberFormatCodeWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XNumberFormatCode >   xNFC;
    ::com::sun::star::lang::Locale          aLocale;

                                // not implemented, prevent usage
                                NumberFormatCodeWrapper( const NumberFormatCodeWrapper& );
            NumberFormatCodeWrapper&    operator=( const NumberFormatCodeWrapper& );

public:
                                NumberFormatCodeWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                                    const ::com::sun::star::lang::Locale& rLocale
                                    );

                                ~NumberFormatCodeWrapper();


    /// set a new Locale
            void                setLocale( const ::com::sun::star::lang::Locale& rLocale );

    /// get current Locale
    const ::com::sun::star::lang::Locale& getLocale() const { return aLocale; }


    // Wrapper implementations of class NumberFormatCodeMapper

    ::com::sun::star::i18n::NumberFormatCode getDefault( sal_Int16 nFormatType, sal_Int16 nFormatUsage ) const;
    ::com::sun::star::i18n::NumberFormatCode getFormatCode( sal_Int16 nFormatIndex ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > getAllFormatCode( sal_Int16 nFormatUsage ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > getAllFormatCodes() const;

};

#endif // _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX
