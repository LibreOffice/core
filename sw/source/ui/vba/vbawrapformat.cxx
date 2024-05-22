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

#include <sal/config.h>

#include "vbawrapformat.hxx"
#include <ooo/vba/word/WdWrapSideType.hpp>
#include <ooo/vba/word/WdWrapType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <basic/sberrors.hxx>
#include <vbahelper/vbahelper.hxx>

using namespace ooo::vba;
using namespace com::sun::star;

SwVbaWrapFormat::SwVbaWrapFormat( uno::Sequence< uno::Any > const& aArgs, uno::Reference< uno::XComponentContext >const& xContext ) : SwVbaWrapFormat_BASE( getXSomethingFromArgs< XHelperInterface >( aArgs, 0 ), xContext ), m_xShape( getXSomethingFromArgs< drawing::XShape >( aArgs, 1, false ) ), mnWrapFormatType( 0 ), mnSide( word::WdWrapSideType::wdWrapBoth )
{
    m_xPropertySet.set( m_xShape, uno::UNO_QUERY_THROW );
}

void SwVbaWrapFormat::makeWrap()
{
    text::WrapTextMode eTextMode = text::WrapTextMode_NONE;
    if( mnSide == word::WdWrapSideType::wdWrapLeft )
    {
        eTextMode = text::WrapTextMode_LEFT;
    }
    else if(  mnSide == word::WdWrapSideType::wdWrapRight )
    {
        eTextMode = text::WrapTextMode_RIGHT;
    }
    else if( mnSide == word::WdWrapSideType::wdWrapBoth ||
            mnSide == word::WdWrapSideType::wdWrapLargest )
    {
        switch( mnWrapFormatType )
        {
            case word::WdWrapType::wdWrapNone:
            case word::WdWrapType::wdWrapThrough:
            {
                eTextMode = text::WrapTextMode_THROUGH;
                break;
            }
            case word::WdWrapType::wdWrapInline:
            case word::WdWrapType::wdWrapTopBottom:
            {
                eTextMode = text::WrapTextMode_NONE;
                break;
            }
            case word::WdWrapType::wdWrapSquare:
            {
                eTextMode = text::WrapTextMode_PARALLEL;
                m_xPropertySet->setPropertyValue(u"SurroundContour"_ustr, uno::Any( false ) );
                break;
            }
            case word::WdWrapType::wdWrapTight:
            {
                eTextMode = text::WrapTextMode_PARALLEL;
                m_xPropertySet->setPropertyValue(u"SurroundContour"_ustr, uno::Any( true ) );
                break;
            }
            default:
            {
                DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_ARGUMENT);
            }
        }
    }
    m_xPropertySet->setPropertyValue(u"TextWrap"_ustr, uno::Any( eTextMode ) );
}

::sal_Int32 SAL_CALL SwVbaWrapFormat::getType()
{
    sal_Int32 nType = word::WdWrapType::wdWrapSquare;
    text::WrapTextMode eTextMode;
    m_xPropertySet->getPropertyValue(u"TextWrap"_ustr) >>= eTextMode;
    switch( eTextMode )
    {
        case text::WrapTextMode_NONE:
        {
            nType = word::WdWrapType::wdWrapTopBottom;
            break;
        }
        case text::WrapTextMode_THROUGH:
        {
            nType = word::WdWrapType::wdWrapNone;
            break;
        }
        case text::WrapTextMode_PARALLEL:
        {
            bool bContour = false;
            m_xPropertySet->getPropertyValue(u"SurroundContour"_ustr) >>= bContour;
            if( bContour )
                nType = word::WdWrapType::wdWrapTight;
            else
                nType = word::WdWrapType::wdWrapSquare;
            break;
        }
        case text::WrapTextMode_DYNAMIC:
        case text::WrapTextMode_LEFT:
        case text::WrapTextMode_RIGHT:
        {
            nType = word::WdWrapType::wdWrapThrough;
            break;
        }
        default:
        {
            nType = word::WdWrapType::wdWrapSquare;
        }
    }
    return nType;
}

void SAL_CALL SwVbaWrapFormat::setType( ::sal_Int32 _type )
{
    mnWrapFormatType = _type;
    makeWrap();
}

::sal_Int32 SAL_CALL SwVbaWrapFormat::getSide()
{
    sal_Int32 nSide = word::WdWrapSideType::wdWrapBoth;
    text::WrapTextMode eTextMode;
    m_xPropertySet->getPropertyValue(u"TextWrap"_ustr) >>= eTextMode;
    switch( eTextMode )
    {
        case text::WrapTextMode_LEFT:
        {
            nSide = word::WdWrapSideType::wdWrapLeft;
            break;
        }
        case text::WrapTextMode_RIGHT:
        {
            nSide = word::WdWrapSideType::wdWrapRight;
            break;
        }
        default:
        {
            nSide = word::WdWrapSideType::wdWrapBoth;
        }
    }
    return nSide;
}

void SAL_CALL SwVbaWrapFormat::setSide( ::sal_Int32 _side )
{
    mnSide = _side;
    makeWrap();
}

float SwVbaWrapFormat::getDistance( const OUString& sName )
{
    sal_Int32 nDistance = 0;
    m_xPropertySet->getPropertyValue( sName ) >>= nDistance;
    return static_cast< float >( Millimeter::getInPoints( nDistance ) );
}

void SwVbaWrapFormat::setDistance( const OUString& sName, float _distance )
{
    sal_Int32 nDistance = Millimeter::getInHundredthsOfOneMillimeter( _distance );
    m_xPropertySet->setPropertyValue( sName, uno::Any( nDistance ) );
}

float SAL_CALL SwVbaWrapFormat::getDistanceTop()
{
    return getDistance( u"TopMargin"_ustr );
}

void SAL_CALL SwVbaWrapFormat::setDistanceTop( float _distancetop )
{
    setDistance( u"TopMargin"_ustr, _distancetop );
}

float SAL_CALL SwVbaWrapFormat::getDistanceBottom()
{
    return getDistance( u"BottomMargin"_ustr );
}

void SAL_CALL SwVbaWrapFormat::setDistanceBottom( float _distancebottom )
{
    setDistance( u"BottomMargin"_ustr, _distancebottom );
}

float SAL_CALL SwVbaWrapFormat::getDistanceLeft()
{
    return getDistance( u"LeftMargin"_ustr );
}

void SAL_CALL SwVbaWrapFormat::setDistanceLeft( float _distanceleft )
{
    setDistance( u"LeftMargin"_ustr, _distanceleft );
}

float SAL_CALL SwVbaWrapFormat::getDistanceRight()
{
    return getDistance( u"RightMargin"_ustr );
}

void SAL_CALL SwVbaWrapFormat::setDistanceRight( float _distanceright )
{
    setDistance( u"RightMargin"_ustr, _distanceright );
}

OUString
SwVbaWrapFormat::getServiceImplName()
{
    return u"SwVbaWrapFormat"_ustr;
}

uno::Sequence< OUString >
SwVbaWrapFormat::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.WrapFormat"_ustr
    };
    return aServiceNames;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Writer_SwVbaWrapFormat_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    return cppu::acquire(new SwVbaWrapFormat(args, context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
