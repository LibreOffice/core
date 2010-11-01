/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <com/sun/star/text/XTextDocument.hpp>
#include <i18npool/mslangid.hxx>
#include "vbafont.hxx"
#include "vbapalette.hxx"
#include "vbaparagraphformat.hxx"
#include "vbastyles.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;


SwVbaStyle::SwVbaStyle( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel>& xModel, const uno::Reference< beans::XPropertySet >& _xPropertySet ) throw ( script::BasicErrorException, uno::RuntimeException ) : SwVbaStyle_BASE( xParent, xContext ) , mxModel( xModel ), mxStyleProps( _xPropertySet )
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
    else
        nType = word::WdStyleType::wdStyleTypeList;
    return nType;
}

uno::Reference< word::XFont > SAL_CALL
SwVbaStyle::getFont() throw ( uno::RuntimeException )
{
    VbaPalette aColors;
    return new SwVbaFont( mxParent, mxContext, aColors.getPalette(), mxStyleProps );
}


void SAL_CALL SwVbaStyle::LinkToListTemplate( const uno::Reference< word::XListTemplate >& /*ListTemplate*/, const uno::Any& /*ListLevelNumber*/ ) throw (uno::RuntimeException)
{
}

void SwVbaStyle::setStyle( const uno::Reference< beans::XPropertySet >& xParaProps, const uno::Any& rStyle )throw (uno::RuntimeException)
{
    rtl::OUString sStyle;
    uno::Reference< word::XStyle > xStyle;
    if( rStyle >>= xStyle )
    {
        sStyle = xStyle->getName();
    }
    else
    {
        rStyle >>= sStyle;
    }

    if( sStyle.getLength() )
    {
        xParaProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParaStyleName") ), uno::makeAny( sStyle ) );
        return;
    }

    throw uno::RuntimeException();
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

::rtl::OUString SAL_CALL SwVbaStyle::getNameLocal() throw (uno::RuntimeException)
{
    rtl::OUString sNameLocal;
    mxStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DisplayName") ) ) >>= sNameLocal;
    return sNameLocal;
}

void SAL_CALL SwVbaStyle::setNameLocal( const ::rtl::OUString& _namelocal ) throw (uno::RuntimeException)
{
    mxStyleProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DisplayName") ), uno::makeAny( _namelocal ) );
}

uno::Reference< word::XParagraphFormat > SAL_CALL SwVbaStyle::getParagraphFormat() throw (uno::RuntimeException)
{
    if( word::WdStyleType::wdStyleTypeParagraph == getType() )
    {
        uno::Reference< text::XTextDocument > xTextDocument( mxModel, uno::UNO_QUERY_THROW );
        return uno::Reference< word::XParagraphFormat >( new SwVbaParagraphFormat( this, mxContext, xTextDocument, mxStyleProps ) );
    }
    else
    {
        throw uno::RuntimeException();
    }
    return uno::Reference< word::XParagraphFormat >();
}

::sal_Bool SAL_CALL SwVbaStyle::getAutomaticallyUpdate() throw (uno::RuntimeException)
{
    sal_Bool isAutoUpdate = sal_False;
    mxStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsAutoUpdate") ) ) >>= isAutoUpdate;
    return isAutoUpdate;
}

void SAL_CALL SwVbaStyle::setAutomaticallyUpdate( ::sal_Bool _automaticallyupdate ) throw (uno::RuntimeException)
{
    mxStyleProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsAutoUpdate") ), uno::makeAny( _automaticallyupdate ) );
}

uno::Any SAL_CALL SwVbaStyle::getBaseStyle() throw (uno::RuntimeException)
{
    // ParentStyle
    rtl::OUString sBaseStyle;
    mxStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParentStyle") ) ) >>= sBaseStyle;
    if( sBaseStyle.getLength() > 0 )
    {
        uno::Reference< XCollection > xCol( new SwVbaStyles( this, mxContext, mxModel ) );
        return xCol->Item( uno::makeAny( sBaseStyle ), uno::Any() );
    }
    else
    {
        throw uno::RuntimeException();
    }
    return uno::Any();
}

void SAL_CALL SwVbaStyle::setBaseStyle( const uno::Any& _basestyle ) throw (uno::RuntimeException)
{
    uno::Reference< word::XStyle > xStyle;
    _basestyle >>= xStyle;
    if( xStyle.is() )
    {
        rtl::OUString sBaseStyle = xStyle->getName();
        mxStyleProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParentStyle") ), uno::makeAny( sBaseStyle ) );
    }
    else
    {
        throw uno::RuntimeException();
    }
}

uno::Any SAL_CALL SwVbaStyle::getNextParagraphStyle() throw (uno::RuntimeException)
{
    //FollowStyle
    rtl::OUString sFollowStyle;
    mxStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FollowStyle") ) ) >>= sFollowStyle;
    if( sFollowStyle.getLength() > 0 )
    {
        uno::Reference< XCollection > xCol( new SwVbaStyles( this, mxContext, mxModel ) );
        return xCol->Item( uno::makeAny( sFollowStyle ), uno::Any() );
    }
    else
    {
        throw uno::RuntimeException();
    }
    return uno::Any();
}

void SAL_CALL SwVbaStyle::setNextParagraphStyle( const uno::Any& _nextparagraphstyle ) throw (uno::RuntimeException)
{
    uno::Reference< word::XStyle > xStyle;
    _nextparagraphstyle >>= xStyle;
    if( xStyle.is() )
    {
        rtl::OUString sFollowStyle = xStyle->getName();
        mxStyleProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FollowStyle") ), uno::makeAny( sFollowStyle ) );
    }
    else
    {
        throw uno::RuntimeException();
    }
}

::sal_Int32 SAL_CALL SwVbaStyle::getListLevelNumber() throw (uno::RuntimeException)
{
    sal_Int16 nNumberingLevel = 0;
    mxStyleProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberingLevel") ) ) >>= nNumberingLevel;
    return nNumberingLevel;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
