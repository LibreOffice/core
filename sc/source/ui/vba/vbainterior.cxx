/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbainterior.cxx,v $
 * $Revision: 1.5 $
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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>

#include <org/openoffice/excel/XlColorIndex.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <svx/xtable.hxx>

#include "vbainterior.hxx"
#include "vbapalette.hxx"
using namespace ::com::sun::star;
using namespace ::org::openoffice;
static const rtl::OUString BACKCOLOR( RTL_CONSTASCII_USTRINGPARAM( "CellBackColor" ) );

ScVbaInterior::ScVbaInterior( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< beans::XPropertySet >&  xProps, ScDocument* pScDoc ) throw ( lang::IllegalArgumentException) : ScVbaInterior_BASE( xParent, xContext ), m_xProps(xProps), m_pScDoc( pScDoc )
{
    if ( !m_xProps.is() )
        throw lang::IllegalArgumentException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "properties") ), uno::Reference< uno::XInterface >(), 2 );
}

uno::Any
ScVbaInterior::getColor() throw (uno::RuntimeException)
{
    uno::Any aAny;
    aAny = OORGBToXLRGB( m_xProps->getPropertyValue( BACKCOLOR ) );
    return aAny;
}

void
ScVbaInterior::setColor( const uno::Any& _color  ) throw (uno::RuntimeException)
{
    m_xProps->setPropertyValue( BACKCOLOR , XLRGBToOORGB(_color));
}

uno::Reference< container::XIndexAccess >
ScVbaInterior::getPalette()
{
    if ( !m_pScDoc )
        throw uno::RuntimeException();
    SfxObjectShell* pShell = m_pScDoc->GetDocumentShell();
    ScVbaPalette aPalette( pShell );
    return aPalette.getPalette();
}

void SAL_CALL
ScVbaInterior::setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    _colorindex >>= nIndex;
    // #FIXME  xlColorIndexAutomatic & xlColorIndexNone are not really
    // handled properly here

    if ( !nIndex || ( nIndex == excel::XlColorIndex::xlColorIndexAutomatic ) || ( nIndex == excel::XlColorIndex::xlColorIndexNone )  )
        nIndex = 2; // default is white ( this maybe will probably break, e.g. we may at some stage need to know what this interior is,  a cell or something else and then pick a default colour based on that )
    --nIndex; // OOo indices are zero bases
    uno::Reference< container::XIndexAccess > xIndex = getPalette();
    // setColor expects colors in XL RGB values
    // #FIXME this is daft we convert OO RGB val to XL RGB val and
    // then back again to OO RGB value
    setColor( OORGBToXLRGB(xIndex->getByIndex( nIndex )) );
}

uno::Any SAL_CALL
ScVbaInterior::getColorIndex() throw ( css::uno::RuntimeException )
{
    sal_Int32 nColor = 0;
    // getColor returns Xl ColorValue, need to convert it to OO val
    // as the palette deals with OO RGB values
    // #FIXME this is daft in getColor we convert OO RGB val to XL RGB val
    // and then back again to OO RGB value
    XLRGBToOORGB( getColor() ) >>= nColor;
    uno::Reference< container::XIndexAccess > xIndex = getPalette();
    sal_Int32 nElems = xIndex->getCount();
    sal_Int32 nIndex = -1;
    for ( sal_Int32 count=0; count<nElems; ++count )
           {
        sal_Int32 nPaletteColor = 0;
        xIndex->getByIndex( count ) >>= nPaletteColor;
        if ( nPaletteColor == nColor )
        {
            nIndex = count + 1; // 1 based
            break;
        }
    }
    return uno::makeAny( nIndex );
}

rtl::OUString&
ScVbaInterior::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaInterior") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaInterior::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.Interior" ) );
    }
    return aServiceNames;
}

