/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "vbastyle.hxx"
#include <ooo/vba/word/WdStyleType.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <i18npool/mslangid.hxx>
#include "vbafont.hxx"
#include "vbapalette.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;


SwVbaStyle::SwVbaStyle( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< beans::XPropertySet >& _xPropertySet ) throw ( script::BasicErrorException, uno::RuntimeException ) : SwVbaStyle_BASE( xParent, xContext ) , mxStyleProps( _xPropertySet )
{
    mxStyle.set( _xPropertySet, uno::UNO_QUERY_THROW );
}

void SAL_CALL
SwVbaStyle::setName( const ::rtl::OUString& Name ) throw (uno::RuntimeException)
{
    mxStyle->setName(Name);
}

::rtl::OUString SAL_CALL
SwVbaStyle::getName() throw (uno::RuntimeException)
{
    return mxStyle->getName();
}

sal_Int32 SwVbaStyle::getLanguageID( const uno::Reference< beans::XPropertySet >& xTCProps ) throw (uno::RuntimeException)
{
    lang::Locale aLocale;
    xTCProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CharLocale") ) ) >>= aLocale;
    return MsLangId::convertLocaleToLanguage( aLocale );
}

void SwVbaStyle::setLanguageID( const uno::Reference< beans::XPropertySet >& xTCProps, sal_Int32 _languageid ) throw (uno::RuntimeException)
{
    lang::Locale aLocale = MsLangId::convertLanguageToLocale( static_cast<LanguageType>(_languageid) );
    xTCProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CharLocale") ), uno::makeAny( aLocale ) ) ;
}

::sal_Int32 SAL_CALL SwVbaStyle::getLanguageID() throw (uno::RuntimeException)
{
    return getLanguageID( mxStyleProps );
}

void SAL_CALL SwVbaStyle::setLanguageID( ::sal_Int32 _languageid ) throw (uno::RuntimeException)
{
    setLanguageID( mxStyleProps, _languageid );
}

::sal_Int32 SAL_CALL SwVbaStyle::getType() throw (uno::RuntimeException)
{
    sal_Int32 nType = word::WdStyleType::wdStyleTypeParagraph;
    uno::Reference< lang::XServiceInfo > xServiceInfo( mxStyle, uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.ParagraphStyle") ) ) )
        nType = word::WdStyleType::wdStyleTypeParagraph;
    else if( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.CharacterStyle") ) ) )
        nType = word::WdStyleType::wdStyleTypeCharacter;
    else // if( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.NumberingStyle") ) ) )
        nType = word::WdStyleType::wdStyleTypeList;
    return nType;
}

uno::Reference< word::XFont > SAL_CALL
SwVbaStyle::getFont() throw ( uno::RuntimeException )
{
    VbaPalette aColors;
    return new SwVbaFont( mxParent, mxContext, aColors.getPalette(), mxStyleProps );
}

void SwVbaStyle::setStyle( const uno::Reference< beans::XPropertySet >& xTCProps, const uno::Reference< ooo::vba::word::XStyle >& xStyle )throw (uno::RuntimeException)
{
    rtl::OUString aStyleType = getOOoStyleTypeFromMSWord( xStyle->getType() );
    xTCProps->setPropertyValue( aStyleType, uno::makeAny( xStyle->getName() ) );
}

rtl::OUString SwVbaStyle::getOOoStyleTypeFromMSWord( sal_Int32 _wdStyleType )
{
    rtl::OUString aStyleType;
    switch( _wdStyleType )
    {
        case word::WdStyleType::wdStyleTypeParagraph:
        case word::WdStyleType::wdStyleTypeTable:
        {
            aStyleType = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParaStyleName") );
            break;
        }
        case word::WdStyleType::wdStyleTypeCharacter:
        {
            aStyleType = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CharStyleName") );
            break;
        }
        case word::WdStyleType::wdStyleTypeList:
        {
            aStyleType = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberingStyleName") );
            break;
        }
        default:
            DebugHelper::exception( SbERR_INTERNAL_ERROR, rtl::OUString() );
    }
    return aStyleType;
}

rtl::OUString&
SwVbaStyle::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaStyle") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaStyle::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.XStyle" ) );
    }
    return aServiceNames;
}
