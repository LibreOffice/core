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
#include "vbawrapformat.hxx"
#include <ooo/vba/word/WdWrapSideType.hpp>
#include <ooo/vba/word/WdWrapType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/helperdecl.hxx>

using namespace ooo::vba;
using namespace com::sun::star;

SwVbaWrapFormat::SwVbaWrapFormat( uno::Sequence< uno::Any > const& aArgs, uno::Reference< uno::XComponentContext >const& xContext ) : SwVbaWrapFormat_BASE( getXSomethingFromArgs< XHelperInterface >( aArgs, 0 ), xContext ), m_xShape( getXSomethingFromArgs< drawing::XShape >( aArgs, 1, false ) ), mnWrapFormatType( 0 ), mnSide( word::WdWrapSideType::wdWrapBoth )
{
    m_xPropertySet.set( m_xShape, uno::UNO_QUERY_THROW );
}

void SwVbaWrapFormat::makeWrap() throw (uno::RuntimeException)
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
                eTextMode = text::WrapTextMode_THROUGHT;
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
                m_xPropertySet->setPropertyValue("SurroundContour", uno::makeAny( false ) );
                break;
            }
            case word::WdWrapType::wdWrapTight:
            {
                eTextMode = text::WrapTextMode_PARALLEL;
                m_xPropertySet->setPropertyValue("SurroundContour", uno::makeAny( true ) );
                break;
            }
            default:
            {
                DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_ARGUMENT, OUString());
            }
        }
    }
    m_xPropertySet->setPropertyValue("TextWrap", uno::makeAny( eTextMode ) );
}

::sal_Int32 SAL_CALL SwVbaWrapFormat::getType() throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nType = word::WdWrapType::wdWrapSquare;
    text::WrapTextMode eTextMode;
    m_xPropertySet->getPropertyValue("TextWrap") >>= eTextMode;
    switch( eTextMode )
    {
        case text::WrapTextMode_NONE:
        {
            nType = word::WdWrapType::wdWrapTopBottom;
            break;
        }
        case text::WrapTextMode_THROUGHT:
        {
            nType = word::WdWrapType::wdWrapNone;
            break;
        }
        case text::WrapTextMode_PARALLEL:
        {
            bool bContour = false;
            m_xPropertySet->getPropertyValue("SurroundContour") >>= bContour;
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

void SAL_CALL SwVbaWrapFormat::setType( ::sal_Int32 _type ) throw (uno::RuntimeException, std::exception)
{
    mnWrapFormatType = _type;
    makeWrap();
}

::sal_Int32 SAL_CALL SwVbaWrapFormat::getSide() throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nSide = word::WdWrapSideType::wdWrapBoth;
    text::WrapTextMode eTextMode;
    m_xPropertySet->getPropertyValue("TextWrap") >>= eTextMode;
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

void SAL_CALL SwVbaWrapFormat::setSide( ::sal_Int32 _side ) throw (uno::RuntimeException, std::exception)
{
    mnSide = _side;
    makeWrap();
}

float SwVbaWrapFormat::getDistance( const OUString& sName ) throw (uno::RuntimeException)
{
    sal_Int32 nDistance = 0;
    m_xPropertySet->getPropertyValue( sName ) >>= nDistance;
    return static_cast< float >( Millimeter::getInPoints( nDistance ) );
}

void SwVbaWrapFormat::setDistance( const OUString& sName, float _distance ) throw (uno::RuntimeException)
{
    sal_Int32 nDistance = Millimeter::getInHundredthsOfOneMillimeter( _distance );
    m_xPropertySet->setPropertyValue( sName, uno::makeAny( nDistance ) );
}

float SAL_CALL SwVbaWrapFormat::getDistanceTop() throw (uno::RuntimeException, std::exception)
{
    return getDistance( "TopMargin" );
}

void SAL_CALL SwVbaWrapFormat::setDistanceTop( float _distancetop ) throw (uno::RuntimeException, std::exception)
{
    setDistance( "TopMargin", _distancetop );
}

float SAL_CALL SwVbaWrapFormat::getDistanceBottom() throw (uno::RuntimeException, std::exception)
{
    return getDistance( "BottomMargin" );
}

void SAL_CALL SwVbaWrapFormat::setDistanceBottom( float _distancebottom ) throw (uno::RuntimeException, std::exception)
{
    setDistance( "BottomMargin", _distancebottom );
}

float SAL_CALL SwVbaWrapFormat::getDistanceLeft() throw (uno::RuntimeException, std::exception)
{
    return getDistance( "LeftMargin" );
}

void SAL_CALL SwVbaWrapFormat::setDistanceLeft( float _distanceleft ) throw (uno::RuntimeException, std::exception)
{
    setDistance( "LeftMargin", _distanceleft );
}

float SAL_CALL SwVbaWrapFormat::getDistanceRight() throw (uno::RuntimeException, std::exception)
{
    return getDistance( "RightMargin" );
}

void SAL_CALL SwVbaWrapFormat::setDistanceRight( float _distanceright ) throw (uno::RuntimeException, std::exception)
{
    setDistance( "RightMargin", _distanceright );
}

OUString
SwVbaWrapFormat::getServiceImplName()
{
    return OUString("SwVbaWrapFormat");
}

uno::Sequence< OUString >
SwVbaWrapFormat::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.WrapFormat";
    }
    return aServiceNames;
}

namespace wrapformat
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<SwVbaWrapFormat, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "SwVbaWrapFormat",
    "ooo.vba.word.WrapFormat" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
