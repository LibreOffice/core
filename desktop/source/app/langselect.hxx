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



#include <list>
#include <sal/types.h>
#include <tools/string.hxx>
#include <rtl/ustring.hxx>
#include <tools/resid.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <svl/languageoptions.hxx>

namespace desktop
{

class LanguageSelection
{
public:
    enum LanguageSelectionStatus
    {
        LS_STATUS_OK,
        LS_STATUS_CANNOT_DETERMINE_LANGUAGE,
        LS_STATUS_CONFIGURATIONACCESS_BROKEN
    };

    static com::sun::star::lang::Locale IsoStringToLocale(const rtl::OUString& str);
    static rtl::OUString getLanguageString();
    static bool prepareLanguage();
    static LanguageSelectionStatus getStatus();

private:
    static const rtl::OUString usFallbackLanguage;
    static rtl::OUString aFoundLanguage;
    static sal_Bool bFoundLanguage;
    static LanguageSelectionStatus m_eStatus;

    static com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >
        getConfigAccess(const sal_Char* pPath, sal_Bool bUpdate=sal_False);
    static com::sun::star::uno::Sequence< rtl::OUString > getInstalledLanguages();
    static sal_Bool isInstalledLanguage(rtl::OUString& usLocale, sal_Bool bExact=sal_False);
    static rtl::OUString getFirstInstalledLanguage();
    static rtl::OUString getUserUILanguage();
    static rtl::OUString getUserLanguage();
    static rtl::OUString getSystemLanguage();
    static void resetUserLanguage();
    static void setDefaultLanguage(const rtl::OUString&);
};

} //namespace desktop
