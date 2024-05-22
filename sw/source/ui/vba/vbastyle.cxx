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
#include <i18nlangtag/languagetag.hxx>
#include <utility>
#include "vbafont.hxx"
#include "vbapalette.hxx"
#include "vbaparagraphformat.hxx"
#include "vbastyles.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaStyle::SwVbaStyle( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, uno::Reference< frame::XModel>  xModel, const uno::Reference< beans::XPropertySet >& _xPropertySet ) : SwVbaStyle_BASE( xParent, xContext ) , mxModel(std::move( xModel )), mxStyleProps( _xPropertySet )
{
    mxStyle.set( _xPropertySet, uno::UNO_QUERY_THROW );
}

void SAL_CALL
SwVbaStyle::setName( const OUString& Name )
{
    mxStyle->setName(Name);
}

OUString SAL_CALL
SwVbaStyle::getName()
{
    return mxStyle->getName();
}

LanguageType SwVbaStyle::getLanguageID( const uno::Reference< beans::XPropertySet >& xTCProps )
{
    lang::Locale aLocale;
    xTCProps->getPropertyValue(u"CharLocale"_ustr) >>= aLocale;
    return LanguageTag::convertToLanguageType( aLocale, false);
}

void SwVbaStyle::setLanguageID( const uno::Reference< beans::XPropertySet >& xTCProps, LanguageType _languageid )
{
    lang::Locale aLocale = LanguageTag( _languageid ).getLocale();
    xTCProps->setPropertyValue(u"CharLocale"_ustr, uno::Any( aLocale ) ) ;
}

::sal_Int32 SAL_CALL SwVbaStyle::getLanguageID()
{
    return static_cast<sal_uInt16>(getLanguageID( mxStyleProps ));
}

void SAL_CALL SwVbaStyle::setLanguageID( ::sal_Int32 _languageid )
{
    setLanguageID( mxStyleProps, LanguageType(_languageid) );
}

::sal_Int32 SAL_CALL SwVbaStyle::getType()
{
    sal_Int32 nType = word::WdStyleType::wdStyleTypeParagraph;
    uno::Reference< lang::XServiceInfo > xServiceInfo( mxStyle, uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService(u"com.sun.star.style.ParagraphStyle"_ustr) )
        nType = word::WdStyleType::wdStyleTypeParagraph;
    else if( xServiceInfo->supportsService(u"com.sun.star.style.CharacterStyle"_ustr) )
        nType = word::WdStyleType::wdStyleTypeCharacter;
    else
        nType = word::WdStyleType::wdStyleTypeList;
    return nType;
}

uno::Reference< word::XFont > SAL_CALL
SwVbaStyle::getFont()
{
    VbaPalette aColors;
    return new SwVbaFont( mxParent, mxContext, aColors.getPalette(), mxStyleProps );
}

void SwVbaStyle::setStyle( const uno::Reference< beans::XPropertySet >& xParaProps, const uno::Any& rStyle )
{
    OUString sStyle;
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
        xParaProps->setPropertyValue(u"ParaStyleName"_ustr, uno::Any( sStyle ) );
        return;
    }

    throw uno::RuntimeException();
}

OUString SAL_CALL SwVbaStyle::getNameLocal()
{
    OUString sNameLocal;
    mxStyleProps->getPropertyValue(u"DisplayName"_ustr) >>= sNameLocal;
    return sNameLocal;
}

void SAL_CALL SwVbaStyle::setNameLocal( const OUString& _namelocal )
{
    mxStyleProps->setPropertyValue(u"DisplayName"_ustr, uno::Any( _namelocal ) );
}

uno::Reference< word::XParagraphFormat > SAL_CALL SwVbaStyle::getParagraphFormat()
{
    if( word::WdStyleType::wdStyleTypeParagraph != getType() )
    {
        throw uno::RuntimeException();
    }

    uno::Reference< text::XTextDocument > xTextDocument( mxModel, uno::UNO_QUERY_THROW );
    return uno::Reference< word::XParagraphFormat >( new SwVbaParagraphFormat( this, mxContext, mxStyleProps ) );
}

sal_Bool SAL_CALL SwVbaStyle::getAutomaticallyUpdate()
{
    bool isAutoUpdate = false;
    mxStyleProps->getPropertyValue(u"IsAutoUpdate"_ustr) >>= isAutoUpdate;
    return isAutoUpdate;
}

void SAL_CALL SwVbaStyle::setAutomaticallyUpdate( sal_Bool _automaticallyupdate )
{
    mxStyleProps->setPropertyValue(u"IsAutoUpdate"_ustr, uno::Any( _automaticallyupdate ) );
}

uno::Any SAL_CALL SwVbaStyle::getBaseStyle()
{
    // ParentStyle
    OUString sBaseStyle;
    mxStyleProps->getPropertyValue(u"ParentStyle"_ustr) >>= sBaseStyle;
    if( sBaseStyle.isEmpty() )
    {
        throw uno::RuntimeException();
    }

    uno::Reference< XCollection > xCol( new SwVbaStyles( this, mxContext, mxModel ) );
    return xCol->Item( uno::Any( sBaseStyle ), uno::Any() );
}

void SAL_CALL SwVbaStyle::setBaseStyle( const uno::Any& _basestyle )
{
    uno::Reference< word::XStyle > xStyle;
    _basestyle >>= xStyle;
    if( !xStyle.is() )
    {
        throw uno::RuntimeException();
    }

    OUString sBaseStyle = xStyle->getName();
    mxStyleProps->setPropertyValue(u"ParentStyle"_ustr, uno::Any( sBaseStyle ) );
}

uno::Any SAL_CALL SwVbaStyle::getNextParagraphStyle()
{
    //FollowStyle
    OUString sFollowStyle;
    mxStyleProps->getPropertyValue(u"FollowStyle"_ustr) >>= sFollowStyle;
    if( sFollowStyle.isEmpty() )
    {
        throw uno::RuntimeException();
    }

    uno::Reference< XCollection > xCol( new SwVbaStyles( this, mxContext, mxModel ) );
    return xCol->Item( uno::Any( sFollowStyle ), uno::Any() );
}

void SAL_CALL SwVbaStyle::setNextParagraphStyle( const uno::Any& _nextparagraphstyle )
{
    uno::Reference< word::XStyle > xStyle;
    _nextparagraphstyle >>= xStyle;
    if( !xStyle.is() )
    {
        throw uno::RuntimeException();
    }

    OUString sFollowStyle = xStyle->getName();
    mxStyleProps->setPropertyValue(u"FollowStyle"_ustr, uno::Any( sFollowStyle ) );
}

::sal_Int32 SAL_CALL SwVbaStyle::getListLevelNumber()
{
    sal_Int16 nNumberingLevel = 0;
    mxStyleProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nNumberingLevel;
    return nNumberingLevel;
}

OUString
SwVbaStyle::getServiceImplName()
{
    return u"SwVbaStyle"_ustr;
}

uno::Sequence< OUString >
SwVbaStyle::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.XStyle"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
