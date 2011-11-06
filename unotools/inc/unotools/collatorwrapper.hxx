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

#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#define _UNOTOOLS_COLLATORWRAPPER_HXX

#ifndef _COM_SUN_STAR_UTIL_XCOLLATOR_HPP_
#include <com/sun/star/i18n/XCollator.hpp>
#endif

namespace com { namespace sun { namespace star { namespace lang {
        class XMultiServiceFactory;
}}}}

class UNOTOOLS_DLLPUBLIC CollatorWrapper
{
    private:

        ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory
            > mxServiceFactory;

        ::com::sun::star::uno::Reference<
                ::com::sun::star::i18n::XCollator
            > mxInternationalCollator;

    public:

        CollatorWrapper (
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory > &xServiceFactoryIn);

        ~CollatorWrapper();

        sal_Int32
        compareSubstring (
                const ::rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
                const ::rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) const;

           sal_Int32
        compareString (
                const ::rtl::OUString& s1, const ::rtl::OUString& s2) const;

        ::com::sun::star::uno::Sequence< ::rtl::OUString >
        listCollatorAlgorithms (
                const ::com::sun::star::lang::Locale& rLocale) const;

        ::com::sun::star::uno::Sequence< sal_Int32 >
        listCollatorOptions (
                const ::rtl::OUString& rAlgorithm) const;

        sal_Int32
        loadDefaultCollator (
                const ::com::sun::star::lang::Locale& rLocale, sal_Int32 nOption);

        sal_Int32
        loadCollatorAlgorithm (
                const ::rtl::OUString& rAlgorithm,
                const ::com::sun::star::lang::Locale& rLocale, sal_Int32 nOption);

        void
        loadCollatorAlgorithmWithEndUserOption (
                const ::rtl::OUString& rAlgorithm,
                const ::com::sun::star::lang::Locale& rLocale,
                const ::com::sun::star::uno::Sequence< sal_Int32 >& rOption);

    protected:

        CollatorWrapper();
};

#endif /* _UNOTOOLS_COLLATORWRAPPER_HXX */
