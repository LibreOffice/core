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

#include "vbahyperlink.hxx"
#include <vbahelper/helperdecl.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <ooo/vba/office/MsoHyperlinkType.hpp>
#include <ooo/vba/msforms/XShape.hpp>
#include "vbarange.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

ScVbaHyperlink::ScVbaHyperlink( const uno::Sequence< uno::Any >& rArgs,
        const uno::Reference< uno::XComponentContext >& rxContext ) throw (lang::IllegalArgumentException) :
    HyperlinkImpl_BASE( getXSomethingFromArgs< XHelperInterface >( rArgs, 0 ), rxContext ),
    mxCell( getXSomethingFromArgs< table::XCell >( rArgs, 1, false ) ),
    mnType( office::MsoHyperlinkType::msoHyperlinkRange )
{
    uno::Reference< text::XTextFieldsSupplier > xTextFields( mxCell, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndex( xTextFields->getTextFields(), uno::UNO_QUERY_THROW );
    mxTextField.set( xIndex->getByIndex(0), uno::UNO_QUERY_THROW );
}

ScVbaHyperlink::ScVbaHyperlink( const uno::Reference< XHelperInterface >& rxAnchor,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Any& rAddress, const uno::Any& rSubAddress,
        const uno::Any& rScreenTip, const uno::Any& rTextToDisplay ) throw (uno::RuntimeException) :
    HyperlinkImpl_BASE( rxAnchor, rxContext ) // parent of Hyperlink is the anchor object
{
    // extract parameters, Address must not be empty
    UrlComponents aUrlComp;
    OUString aTextToDisplay;
    if( !(rAddress >>= aUrlComp.first) || (aUrlComp.first.getLength() == 0) )
        throw uno::RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM( "Cannot get address" ) ), uno::Reference< uno::XInterface >() );
    rSubAddress >>= aUrlComp.second;
    rScreenTip >>= maScreenTip;
    rTextToDisplay >>= aTextToDisplay;

    // get anchor range or anchor shape
    uno::Reference< excel::XRange > xAnchorRange( rxAnchor, uno::UNO_QUERY );
    if( xAnchorRange.is() )
    {
        mnType = office::MsoHyperlinkType::msoHyperlinkRange;
        // only single ranges are allowed
        uno::Reference< table::XCellRange > xUnoRange( ScVbaRange::getCellRange( xAnchorRange ), uno::UNO_QUERY_THROW );
        // insert the hyperlink into the top-left cell only
        mxCell.set( xUnoRange->getCellByPosition( 0, 0 ), uno::UNO_SET_THROW );
        uno::Reference< text::XText > xText( mxCell, uno::UNO_QUERY_THROW );
        // use cell text or URL if no TextToDisplay has been passed
        if( aTextToDisplay.getLength() == 0 )
        {
            aTextToDisplay = xText->getString();
            if( aTextToDisplay.getLength() == 0 )
            {
                OUStringBuffer aBuffer( aUrlComp.first );
                if( aUrlComp.second.getLength() > 0 )
                    aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( " - " ) ).append( aUrlComp.second );
                aTextToDisplay = aBuffer.makeStringAndClear();
            }
        }
        // create and initialize a new URL text field
        uno::Reference< lang::XMultiServiceFactory > xFactory( ScVbaRange::getUnoModel( xAnchorRange ), uno::UNO_QUERY_THROW );
        uno::Reference< text::XTextContent > xUrlField( xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.TextField.URL" ) ) ), uno::UNO_QUERY_THROW );
        mxTextField.set( xUrlField, uno::UNO_QUERY_THROW );
        setUrlComponents( aUrlComp );
        setTextToDisplay( aTextToDisplay );
        // insert the text field into the document
        xText->setString( OUString() );
        uno::Reference< text::XTextRange > xRange( xText->createTextCursor(), uno::UNO_QUERY_THROW );
        xText->insertTextContent( xRange, xUrlField, false );
    }
    else
    {
        uno::Reference< msforms::XShape > xAnchorShape( rxAnchor, uno::UNO_QUERY_THROW );
        mnType = office::MsoHyperlinkType::msoHyperlinkShape;
        // FIXME: insert hyperlink into shape
        throw uno::RuntimeException();
    }
}

ScVbaHyperlink::~ScVbaHyperlink()
{
}

OUString ScVbaHyperlink::getName() throw (uno::RuntimeException)
{
    // it seems this attribute is same as TextToDisplay
    return getTextToDisplay();
}

void ScVbaHyperlink::setName( const OUString& rName ) throw (uno::RuntimeException)
{
    setTextToDisplay( rName );
}

OUString ScVbaHyperlink::getAddress() throw (uno::RuntimeException)
{
    return getUrlComponents().first;
}

void ScVbaHyperlink::setAddress( const OUString& rAddress ) throw (uno::RuntimeException)
{
    UrlComponents aUrlComp = getUrlComponents();
    aUrlComp.first = rAddress;
    setUrlComponents( aUrlComp );
}

OUString ScVbaHyperlink::getSubAddress() throw (uno::RuntimeException)
{
    return getUrlComponents().second;
}

void ScVbaHyperlink::setSubAddress( const OUString& rSubAddress ) throw (uno::RuntimeException)
{
    UrlComponents aUrlComp = getUrlComponents();
    aUrlComp.second = rSubAddress;
    setUrlComponents( aUrlComp );
}

OUString SAL_CALL ScVbaHyperlink::getScreenTip() throw (uno::RuntimeException)
{
    return maScreenTip;
}

void SAL_CALL ScVbaHyperlink::setScreenTip( const OUString& rScreenTip ) throw (uno::RuntimeException)
{
    maScreenTip = rScreenTip;
}

OUString ScVbaHyperlink::getTextToDisplay() throw (uno::RuntimeException)
{
    ensureTextField();
    OUString aTextToDisplay;
    mxTextField->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Representation" ) ) ) >>= aTextToDisplay;
    return aTextToDisplay;
}

void ScVbaHyperlink::setTextToDisplay( const OUString& rTextToDisplay ) throw (uno::RuntimeException)
{
    ensureTextField();
    mxTextField->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Representation" ) ), uno::Any( rTextToDisplay ) );
}

sal_Int32 SAL_CALL ScVbaHyperlink::getType() throw (uno::RuntimeException)
{
    return mnType;
}

uno::Reference< excel::XRange > SAL_CALL ScVbaHyperlink::getRange() throw (uno::RuntimeException)
{
    if( mnType == office::MsoHyperlinkType::msoHyperlinkRange )
    {
        // if constructed from Hyperlinks object, range has been passed as parent
        uno::Reference< excel::XRange > xAnchorRange( getParent(), uno::UNO_QUERY );
        if( !xAnchorRange.is() )
        {
            // if constructed via service c'tor, create new range based on cell
            uno::Reference< table::XCellRange > xRange( mxCell, uno::UNO_QUERY_THROW );
            // FIXME: need to pass current worksheet as the parent of XRange.
            xAnchorRange.set( new ScVbaRange( uno::Reference< XHelperInterface >(), mxContext, xRange ) );
        }
        return xAnchorRange;
    }
    // error if called at a shape Hyperlink object
    throw uno::RuntimeException();
}

uno::Reference< msforms::XShape > SAL_CALL ScVbaHyperlink::getShape() throw (uno::RuntimeException)
{
    // error if called at a range Hyperlink object
    return uno::Reference< msforms::XShape >( getParent(), uno::UNO_QUERY_THROW );
}

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaHyperlink, "ooo.vba.excel.Hyperlink" )

// private --------------------------------------------------------------------

void ScVbaHyperlink::ensureTextField() throw (uno::RuntimeException)
{
    if( !mxTextField.is() )
        throw uno::RuntimeException();
}

ScVbaHyperlink::UrlComponents ScVbaHyperlink::getUrlComponents() throw (uno::RuntimeException)
{
    ensureTextField();
    OUString aUrl;
    mxTextField->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) ) >>= aUrl;
    sal_Int32 nHashPos = aUrl.indexOf( '#' );
    if( nHashPos < 0 )
        return UrlComponents( aUrl, OUString() );
    return UrlComponents( aUrl.copy( 0, nHashPos ), aUrl.copy( nHashPos + 1 ) );
}

void ScVbaHyperlink::setUrlComponents( const UrlComponents& rUrlComp ) throw (uno::RuntimeException)
{
    ensureTextField();
    OUStringBuffer aUrl( rUrlComp.first );
    if( rUrlComp.second.getLength() > 0 )
        aUrl.append( sal_Unicode( '#' ) ).append( rUrlComp.second );
    mxTextField->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ), uno::Any( aUrl.makeStringAndClear() ) );
}

namespace hyperlink
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaHyperlink, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaHyperlink",
    "ooo.vba.excel.Hyperlink" );
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
