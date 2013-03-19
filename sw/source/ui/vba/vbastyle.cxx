/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "vbastyle.hxx"
#include <ooo/vba/word/WdStyleType.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <i18npool/languagetag.hxx>
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
    xTCProps->getPropertyValue( rtl::OUString("CharLocale") ) >>= aLocale;
    return LanguageTag( aLocale ).getLanguageType( false);
}

void SwVbaStyle::setLanguageID( const uno::Reference< beans::XPropertySet >& xTCProps, sal_Int32 _languageid ) throw (uno::RuntimeException)
{
    lang::Locale aLocale = LanguageTag( static_cast<LanguageType>(_languageid) ).getLocale();
    xTCProps->setPropertyValue( rtl::OUString("CharLocale"), uno::makeAny( aLocale ) ) ;
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
    if( xServiceInfo->supportsService( rtl::OUString("com.sun.star.style.ParagraphStyle") ) )
        nType = word::WdStyleType::wdStyleTypeParagraph;
    else if( xServiceInfo->supportsService( rtl::OUString("com.sun.star.style.CharacterStyle") ) )
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

    if( !sStyle.isEmpty() )
    {
        xParaProps->setPropertyValue( rtl::OUString("ParaStyleName"), uno::makeAny( sStyle ) );
        return;
    }

    throw uno::RuntimeException();
}

::rtl::OUString SAL_CALL SwVbaStyle::getNameLocal() throw (uno::RuntimeException)
{
    rtl::OUString sNameLocal;
    mxStyleProps->getPropertyValue( rtl::OUString("DisplayName") ) >>= sNameLocal;
    return sNameLocal;
}

void SAL_CALL SwVbaStyle::setNameLocal( const ::rtl::OUString& _namelocal ) throw (uno::RuntimeException)
{
    mxStyleProps->setPropertyValue( rtl::OUString("DisplayName"), uno::makeAny( _namelocal ) );
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
}

::sal_Bool SAL_CALL SwVbaStyle::getAutomaticallyUpdate() throw (uno::RuntimeException)
{
    sal_Bool isAutoUpdate = sal_False;
    mxStyleProps->getPropertyValue( rtl::OUString("IsAutoUpdate") ) >>= isAutoUpdate;
    return isAutoUpdate;
}

void SAL_CALL SwVbaStyle::setAutomaticallyUpdate( ::sal_Bool _automaticallyupdate ) throw (uno::RuntimeException)
{
    mxStyleProps->setPropertyValue( rtl::OUString("IsAutoUpdate"), uno::makeAny( _automaticallyupdate ) );
}

uno::Any SAL_CALL SwVbaStyle::getBaseStyle() throw (uno::RuntimeException)
{
    // ParentStyle
    rtl::OUString sBaseStyle;
    mxStyleProps->getPropertyValue( rtl::OUString("ParentStyle") ) >>= sBaseStyle;
    if( !sBaseStyle.isEmpty() )
    {
        uno::Reference< XCollection > xCol( new SwVbaStyles( this, mxContext, mxModel ) );
        return xCol->Item( uno::makeAny( sBaseStyle ), uno::Any() );
    }
    else
    {
        throw uno::RuntimeException();
    }
}

void SAL_CALL SwVbaStyle::setBaseStyle( const uno::Any& _basestyle ) throw (uno::RuntimeException)
{
    uno::Reference< word::XStyle > xStyle;
    _basestyle >>= xStyle;
    if( xStyle.is() )
    {
        rtl::OUString sBaseStyle = xStyle->getName();
        mxStyleProps->setPropertyValue( rtl::OUString("ParentStyle"), uno::makeAny( sBaseStyle ) );
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
    mxStyleProps->getPropertyValue( rtl::OUString("FollowStyle") ) >>= sFollowStyle;
    if( !sFollowStyle.isEmpty() )
    {
        uno::Reference< XCollection > xCol( new SwVbaStyles( this, mxContext, mxModel ) );
        return xCol->Item( uno::makeAny( sFollowStyle ), uno::Any() );
    }
    else
    {
        throw uno::RuntimeException();
    }
}

void SAL_CALL SwVbaStyle::setNextParagraphStyle( const uno::Any& _nextparagraphstyle ) throw (uno::RuntimeException)
{
    uno::Reference< word::XStyle > xStyle;
    _nextparagraphstyle >>= xStyle;
    if( xStyle.is() )
    {
        rtl::OUString sFollowStyle = xStyle->getName();
        mxStyleProps->setPropertyValue( rtl::OUString("FollowStyle"), uno::makeAny( sFollowStyle ) );
    }
    else
    {
        throw uno::RuntimeException();
    }
}

::sal_Int32 SAL_CALL SwVbaStyle::getListLevelNumber() throw (uno::RuntimeException)
{
    sal_Int16 nNumberingLevel = 0;
    mxStyleProps->getPropertyValue( rtl::OUString("NumberingLevel") ) >>= nNumberingLevel;
    return nNumberingLevel;
}

rtl::OUString
SwVbaStyle::getServiceImplName()
{
    return rtl::OUString("SwVbaStyle");
}

uno::Sequence< rtl::OUString >
SwVbaStyle::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.word.XStyle" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
