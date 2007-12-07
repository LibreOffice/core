/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbatextframe.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:04:15 $
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
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/text/XText.hpp>
#include "vbatextframe.hxx"
#include "vbacharacters.hxx"

using namespace ::org::openoffice;
using namespace ::com::sun::star;

ScVbaTextFrame::ScVbaTextFrame( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, uno::Reference< drawing::XShape > xShape ) : ScVbaTextFrame_BASE( xParent, xContext ), m_xShape( xShape )
{
    m_xPropertySet.set( m_xShape, uno::UNO_QUERY_THROW );
}

void
ScVbaTextFrame::setAsMSObehavior()
{
    //set property TextWordWrap default as False.
    // TextFitToSize control the text content. it seems we should set the default as False.
    // com.sun.star.drawing.TextFitToSizeType.NONE
    m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "TextWordWrap" ), uno::makeAny( sal_False ) );
    m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "TextFitToSize" ), uno::makeAny( drawing::TextFitToSizeType_NONE ) );
}

sal_Int32 ScVbaTextFrame::getMargin( rtl::OUString sMarginType )
{
    sal_Int32 nMargin = 0;
    uno::Any aMargin = m_xPropertySet->getPropertyValue( sMarginType );
    aMargin >>= nMargin;
    return nMargin;
}

void ScVbaTextFrame::setMargin( rtl::OUString sMarginType, float fMargin )
{
    sal_Int32 nMargin = Millimeter::getInHundredthsOfOneMillimeter( fMargin );
    m_xPropertySet->setPropertyValue( sMarginType, uno::makeAny( nMargin ) );
}

// Attributes
sal_Bool SAL_CALL
ScVbaTextFrame::getAutoSize() throw (uno::RuntimeException)
{
    // I don't know why, but in OOo, TextAutoGrowHeight is the property control autosize. not TextFitToSize.
    // TextFitToSize control the text content.
    // and in mso, there isnot option TextWordWrap which means auto wrap. the default is False.
    sal_Bool bAutosize = sal_False;
    uno::Any aTextAutoGrowHeight = m_xPropertySet->getPropertyValue( rtl::OUString::createFromAscii( "TextAutoGrowHeight" ) );
    aTextAutoGrowHeight >>= bAutosize;
    return bAutosize;
}

void SAL_CALL
ScVbaTextFrame::setAutoSize( sal_Bool _autosize ) throw (uno::RuntimeException)
{
    setAsMSObehavior();
    m_xPropertySet->setPropertyValue( rtl::OUString::createFromAscii( "TextAutoGrowHeight" ), uno::makeAny( _autosize ) );
}

float SAL_CALL
ScVbaTextFrame::getMarginBottom() throw (uno::RuntimeException)
{
    sal_Int32 nMargin = getMargin( rtl::OUString::createFromAscii( "TextLowerDistance" ) );
    float fMargin = (float)Millimeter::getInPoints( nMargin );
    return fMargin;
}

void SAL_CALL
ScVbaTextFrame::setMarginBottom( float _marginbottom ) throw (uno::RuntimeException)
{
    setMargin( rtl::OUString::createFromAscii( "TextLowerDistance" ), _marginbottom );
}

float SAL_CALL
ScVbaTextFrame::getMarginTop() throw (uno::RuntimeException)
{
    sal_Int32 nMargin = getMargin( rtl::OUString::createFromAscii( "TextUpperDistance" ) );
    float fMargin = (float)Millimeter::getInPoints( nMargin );
    return fMargin;
}

void SAL_CALL
ScVbaTextFrame::setMarginTop( float _margintop ) throw (uno::RuntimeException)
{
    setMargin( rtl::OUString::createFromAscii( "TextUpperDistance" ), _margintop );
}

float SAL_CALL
ScVbaTextFrame::getMarginLeft() throw (uno::RuntimeException)
{
    sal_Int32 nMargin = getMargin( rtl::OUString::createFromAscii( "TextLeftDistance" ) );
    float fMargin = (float)Millimeter::getInPoints( nMargin );
    return fMargin;
}

void SAL_CALL
ScVbaTextFrame::setMarginLeft( float _marginleft ) throw (uno::RuntimeException)
{
    setMargin( rtl::OUString::createFromAscii( "TextLeftDistance" ), _marginleft );
}

float SAL_CALL
ScVbaTextFrame::getMarginRight() throw (uno::RuntimeException)
{
    sal_Int32 nMargin = getMargin( rtl::OUString::createFromAscii( "TextRightDistance" ) );
    float fMargin = (float)Millimeter::getInPoints( nMargin );
    return fMargin;
}

void SAL_CALL
ScVbaTextFrame::setMarginRight( float _marginright ) throw (uno::RuntimeException)
{
    setMargin( rtl::OUString::createFromAscii( "TextRightDistance" ), _marginright );
}


// Methods
uno::Reference< oo::excel::XCharacters > SAL_CALL
ScVbaTextFrame::Characters() throw (uno::RuntimeException)
{
    uno::Reference< text::XSimpleText > xSimpleText( m_xShape, uno::UNO_QUERY_THROW );
    ScVbaPalette aPalette( SfxObjectShell::Current() );
    uno::Any aStart( sal_Int32( 1 ) );
    uno::Any aLength(sal_Int32( -1 ) );
    return uno::Reference< oo::excel::XCharacters >( new ScVbaCharacters( this, mxContext, aPalette, xSimpleText, aStart, aLength, sal_True ) );
}

rtl::OUString&
ScVbaTextFrame::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaTextFrame") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaTextFrame::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.TextFrame" ) );
    }
    return aServiceNames;
}

