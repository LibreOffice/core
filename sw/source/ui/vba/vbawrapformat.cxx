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
                m_xPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SurroundContour") ), uno::makeAny( sal_False ) );
                break;
            }
            case word::WdWrapType::wdWrapTight:
            {
                eTextMode = text::WrapTextMode_PARALLEL;
                m_xPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SurroundContour") ), uno::makeAny( sal_True ) );
                break;
            }
            default:
            {
                DebugHelper::exception(SbERR_BAD_ARGUMENT, rtl::OUString());
            }
        }
    }
    m_xPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextWrap") ), uno::makeAny( eTextMode ) );
}

::sal_Int32 SAL_CALL SwVbaWrapFormat::getType() throw (uno::RuntimeException)
{
    sal_Int32 nType = word::WdWrapType::wdWrapSquare;
    text::WrapTextMode eTextMode;
    m_xPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextWrap") )) >>= eTextMode;
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
            sal_Bool bContour = sal_False;
            m_xPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SurroundContour") )) >>= bContour;
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

void SAL_CALL SwVbaWrapFormat::setType( ::sal_Int32 _type ) throw (uno::RuntimeException)
{
    mnWrapFormatType = _type;
    makeWrap();
}

::sal_Int32 SAL_CALL SwVbaWrapFormat::getSide() throw (uno::RuntimeException)
{
    sal_Int32 nSide = word::WdWrapSideType::wdWrapBoth;
    text::WrapTextMode eTextMode;
    m_xPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextWrap") )) >>= eTextMode;
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

void SAL_CALL SwVbaWrapFormat::setSide( ::sal_Int32 _side ) throw (uno::RuntimeException)
{
    mnSide = _side;
    makeWrap();
}

float SwVbaWrapFormat::getDistance( const rtl::OUString& sName ) throw (uno::RuntimeException)
{
    sal_Int32 nDistance = 0;
    m_xPropertySet->getPropertyValue( sName ) >>= nDistance;
    return static_cast< float >( Millimeter::getInPoints( nDistance ) );
}

void SwVbaWrapFormat::setDistance( const rtl::OUString& sName, float _distance ) throw (uno::RuntimeException)
{
    sal_Int32 nDistance = Millimeter::getInHundredthsOfOneMillimeter( _distance );
    m_xPropertySet->setPropertyValue( sName, uno::makeAny( nDistance ) );
}

float SAL_CALL SwVbaWrapFormat::getDistanceTop() throw (uno::RuntimeException)
{
    return getDistance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin") ) );
}

void SAL_CALL SwVbaWrapFormat::setDistanceTop( float _distancetop ) throw (uno::RuntimeException)
{
    setDistance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin") ), _distancetop );
}

float SAL_CALL SwVbaWrapFormat::getDistanceBottom() throw (uno::RuntimeException)
{
    return getDistance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin") ) );
}

void SAL_CALL SwVbaWrapFormat::setDistanceBottom( float _distancebottom ) throw (uno::RuntimeException)
{
    setDistance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin") ), _distancebottom );
}

float SAL_CALL SwVbaWrapFormat::getDistanceLeft() throw (uno::RuntimeException)
{
    return getDistance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LeftMargin") ) );
}

void SAL_CALL SwVbaWrapFormat::setDistanceLeft( float _distanceleft ) throw (uno::RuntimeException)
{
    setDistance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LeftMargin") ), _distanceleft );
}

float SAL_CALL SwVbaWrapFormat::getDistanceRight() throw (uno::RuntimeException)
{
    return getDistance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightMargin") ) );
}

void SAL_CALL SwVbaWrapFormat::setDistanceRight( float _distanceright ) throw (uno::RuntimeException)
{
    setDistance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightMargin") ), _distanceright );
}

rtl::OUString&
SwVbaWrapFormat::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaWrapFormat") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaWrapFormat::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.WrapFormat" ) );
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
