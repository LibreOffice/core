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


#ifndef _unotools_SOURCEVIEWCONFIG_HXX
#define _unotools_SOURCEVIEWCONFIG_HXX

// include ---------------------------------------------------------------

#include "unotools/unotoolsdllapi.h"
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>

/* -----------------------------12.10.00 11:40--------------------------------

 ---------------------------------------------------------------------------*/
namespace utl
{
    class SourceViewConfig_Impl;
    class UNOTOOLS_DLLPUBLIC SourceViewConfig: public utl::detail::Options
    {
        static SourceViewConfig_Impl* m_pImplConfig;
        static sal_Int32              m_nRefCount;

        public:
            SourceViewConfig();
            virtual ~SourceViewConfig();

            const rtl::OUString&    GetFontName() const;
            void                    SetFontName(const rtl::OUString& rName);

            sal_Int16               GetFontHeight() const;
            void                    SetFontHeight(sal_Int16 nHeight);

            sal_Bool                IsShowProportionalFontsOnly() const;
            void                    SetShowProportionalFontsOnly(sal_Bool bSet);
    };
}
#endif

