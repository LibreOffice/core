/*************************************************************************
 *
 *  $RCSfile: i18nhelp.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mt $ $Date: 2001-08-03 13:46:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include <i18nhelp.hxx>

/*
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif


#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
*/

// #include <cppuhelper/servicefactory.hxx>


#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/TransliterationModules.hpp>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

using namespace ::com::sun::star;

vcl::I18nHelper::I18nHelper(  ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF, const ::com::sun::star::lang::Locale& rLocale )
{
    mxMSF = rxMSF;
    maLocale = rLocale;
    mpLocaleDataWrapper = NULL;
    mpTransliterationWrapper= NULL;
    mbTransliterateIgnoreCase = sal_False;
}

vcl::I18nHelper::~I18nHelper()
{
}

void vcl::I18nHelper::ImplDestroyWrappers()
{
    delete mpLocaleDataWrapper;
    mpLocaleDataWrapper = NULL;

    delete mpTransliterationWrapper;
    mpTransliterationWrapper= NULL;
}

utl::TransliterationWrapper& vcl::I18nHelper::ImplGetTransliterationWrapper() const
{
    if ( !mpTransliterationWrapper )
    {
        sal_Int32 nModules = i18n::TransliterationModules_IGNORE_WIDTH;
        if ( mbTransliterateIgnoreCase )
            nModules |= i18n::TransliterationModules_IGNORE_CASE;
        ((vcl::I18nHelper*)this)->mpTransliterationWrapper = new utl::TransliterationWrapper( mxMSF, (i18n::TransliterationModules)nModules );
        ((vcl::I18nHelper*)this)->mpTransliterationWrapper->loadModuleIfNeeded( ConvertIsoNamesToLanguage( maLocale.Language, maLocale.Country ) );
    }
    return *mpTransliterationWrapper;
}

void vcl::I18nHelper::setLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

//    if ( maLocale != rLocale )
    {
        maLocale = rLocale;
        ImplDestroyWrappers();
    }
}

const ::com::sun::star::lang::Locale& vcl::I18nHelper::getLocale() const
{
    return maLocale;
}

sal_Bool vcl::I18nHelper::equals( const String& rStr1, sal_Int32 nPos1, sal_Int32 nCount1, const String& rStr2, sal_Int32 nPos2, sal_Int32 nCount2, sal_Bool bIgnoreCase ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

    // !!! TRANSLITERATION DOESN'T WORK !!!
    /*
    if ( bIgnoreCase != mbTransliterateIgnoreCase )
    {
        // Change mbTransliterateIgnoreCase and destroy the warpper, next call to
        // ImplGetTransliterationWrapper() will create a wrapper with the correct bIgnoreCase
        ((vcl::I18nHelper*)this)->mbTransliterateIgnoreCase = bIgnoreCase;
        delete ((vcl::I18nHelper*)this)->mpTransliterationWrapper;
        ((vcl::I18nHelper*)this)->mpTransliterationWrapper = NULL;
    }

    if ( nCount1 > ( rStr1.Len() - nPos1 ) )
        nCount1 = rStr1.Len() - nPos1;
    if ( nCount2 > ( rStr2.Len() - nPos2 ) )
        nCount2 = rStr2.Len() - nPos2;


    sal_Int32 nMatch1, nMatch2;
    return ImplGetTransliterationWrapper().equals( rStr1, nPos1, nCount1, nMatch1, rStr2, nPos2, nCount2, nMatch2 );
    */

    BOOL bEqual;
    if ( bIgnoreCase )
    {
        bEqual = String( rStr1, nPos1, nCount1 ).EqualsIgnoreCaseAscii( rStr2, nPos2, nCount2 );
    }
    else
    {
        bEqual = String( rStr1, nPos1, nCount1 ).Equals( rStr2, nPos2, nCount2 );
    }
    return bEqual;
}


String vcl::I18nHelper::getDate( const Date& rDate, sal_Bool bLongFormat ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!
    return String();
}
