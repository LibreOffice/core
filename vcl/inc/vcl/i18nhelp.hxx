/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i18nhelp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:56:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _VCL_I18NHELP_HXX
#define _VCL_I18NHELP_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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
    String      GetNum( long nNumber, USHORT nDecimals, BOOL bUseThousandSep = TRUE, BOOL bTrailingZeros = TRUE ) const;
};

}   // namespace vcl

#endif  // _VCL_I18NHELP_HXX

