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



#ifndef _VCL_I18NHELP_HXX
#define _VCL_I18NHELP_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/Locale.hpp>
#include <osl/mutex.hxx>
#include <tools/string.hxx>
#include <vcl/dllapi.h>

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory;
}
}}}

namespace utl {
    class TransliterationWrapper;
}

class LocaleDataWrapper;

class Date;

namespace vcl
{

class VCL_DLLPUBLIC I18nHelper
{
private:
    ::osl::Mutex                    maMutex;
    ::com::sun::star::lang::Locale  maLocale;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;

    LocaleDataWrapper*              mpLocaleDataWrapper;
    utl::TransliterationWrapper*    mpTransliterationWrapper;

    sal_Bool                        mbTransliterateIgnoreCase;

    SAL_DLLPRIVATE void             ImplDestroyWrappers();

protected:
    ::osl::Mutex& GetMutex() { return maMutex; }

    SAL_DLLPRIVATE utl::TransliterationWrapper&    ImplGetTransliterationWrapper() const;
    SAL_DLLPRIVATE LocaleDataWrapper&              ImplGetLocaleDataWrapper() const;

public:

                I18nHelper( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF, const ::com::sun::star::lang::Locale& rLocale );
                ~I18nHelper();

    const ::com::sun::star::lang::Locale& getLocale() const;

    sal_Int32   CompareString( const String& rStr1, const String& rStr2 ) const;

    sal_Bool    MatchString( const String& rStr1, const String& rStr2 ) const;
    sal_Bool    MatchMnemonic( const String& rString, sal_Unicode cMnemonicChar ) const;

    String      GetDate( const Date& rDate ) const;
    String      GetNum( long nNumber, sal_uInt16 nDecimals, sal_Bool bUseThousandSep = sal_True, sal_Bool bTrailingZeros = sal_True ) const;

    static String filterFormattingChars( const String& );
};

}   // namespace vcl

#endif  // _VCL_I18NHELP_HXX

