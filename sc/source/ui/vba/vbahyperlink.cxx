/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
    HyperlinkImpl_BASE( rxAnchor, rxContext ) 
{
    
    UrlComponents aUrlComp;
    OUString aTextToDisplay;
    if( !(rAddress >>= aUrlComp.first) || aUrlComp.first.isEmpty() )
        throw uno::RuntimeException("Cannot get address", uno::Reference< uno::XInterface >() );
    rSubAddress >>= aUrlComp.second;
    rScreenTip >>= maScreenTip;
    rTextToDisplay >>= aTextToDisplay;

    
    uno::Reference< excel::XRange > xAnchorRange( rxAnchor, uno::UNO_QUERY );
    if( xAnchorRange.is() )
    {
        mnType = office::MsoHyperlinkType::msoHyperlinkRange;
        
        uno::Reference< table::XCellRange > xUnoRange( ScVbaRange::getCellRange( xAnchorRange ), uno::UNO_QUERY_THROW );
        
        mxCell.set( xUnoRange->getCellByPosition( 0, 0 ), uno::UNO_SET_THROW );
        uno::Reference< text::XText > xText( mxCell, uno::UNO_QUERY_THROW );
        
        if( aTextToDisplay.isEmpty() )
        {
            aTextToDisplay = xText->getString();
            if( aTextToDisplay.isEmpty() )
            {
                OUStringBuffer aBuffer( aUrlComp.first );
                if( !aUrlComp.second.isEmpty() )
                    aBuffer.append( " - " ).append( aUrlComp.second );
                aTextToDisplay = aBuffer.makeStringAndClear();
            }
        }
        
        uno::Reference< lang::XMultiServiceFactory > xFactory( ScVbaRange::getUnoModel( xAnchorRange ), uno::UNO_QUERY_THROW );
        uno::Reference< text::XTextContent > xUrlField( xFactory->createInstance("com.sun.star.text.TextField.URL"), uno::UNO_QUERY_THROW );
        mxTextField.set( xUrlField, uno::UNO_QUERY_THROW );
        setUrlComponents( aUrlComp );
        setTextToDisplay( aTextToDisplay );
        
        xText->setString( OUString() );
        uno::Reference< text::XTextRange > xRange( xText->createTextCursor(), uno::UNO_QUERY_THROW );
        xText->insertTextContent( xRange, xUrlField, false );
    }
    else
    {
        uno::Reference< msforms::XShape > xAnchorShape( rxAnchor, uno::UNO_QUERY_THROW );
        mnType = office::MsoHyperlinkType::msoHyperlinkShape;
        
        throw uno::RuntimeException();
    }
}

ScVbaHyperlink::~ScVbaHyperlink()
{
}

OUString ScVbaHyperlink::getName() throw (uno::RuntimeException)
{
    
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
    mxTextField->getPropertyValue("Representation") >>= aTextToDisplay;
    return aTextToDisplay;
}

void ScVbaHyperlink::setTextToDisplay( const OUString& rTextToDisplay ) throw (uno::RuntimeException)
{
    ensureTextField();
    mxTextField->setPropertyValue("Representation", uno::Any( rTextToDisplay ) );
}

sal_Int32 SAL_CALL ScVbaHyperlink::getType() throw (uno::RuntimeException)
{
    return mnType;
}

uno::Reference< excel::XRange > SAL_CALL ScVbaHyperlink::getRange() throw (uno::RuntimeException)
{
    if( mnType == office::MsoHyperlinkType::msoHyperlinkRange )
    {
        
        uno::Reference< excel::XRange > xAnchorRange( getParent(), uno::UNO_QUERY );
        if( !xAnchorRange.is() )
        {
            
            uno::Reference< table::XCellRange > xRange( mxCell, uno::UNO_QUERY_THROW );
            
            xAnchorRange.set( new ScVbaRange( uno::Reference< XHelperInterface >(), mxContext, xRange ) );
        }
        return xAnchorRange;
    }
    
    throw uno::RuntimeException();
}

uno::Reference< msforms::XShape > SAL_CALL ScVbaHyperlink::getShape() throw (uno::RuntimeException)
{
    
    return uno::Reference< msforms::XShape >( getParent(), uno::UNO_QUERY_THROW );
}

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaHyperlink, "ooo.vba.excel.Hyperlink" )



void ScVbaHyperlink::ensureTextField() throw (uno::RuntimeException)
{
    if( !mxTextField.is() )
        throw uno::RuntimeException();
}

ScVbaHyperlink::UrlComponents ScVbaHyperlink::getUrlComponents() throw (uno::RuntimeException)
{
    ensureTextField();
    OUString aUrl;
    mxTextField->getPropertyValue("URL") >>= aUrl;
    sal_Int32 nHashPos = aUrl.indexOf( '#' );
    if( nHashPos < 0 )
        return UrlComponents( aUrl, OUString() );
    return UrlComponents( aUrl.copy( 0, nHashPos ), aUrl.copy( nHashPos + 1 ) );
}

void ScVbaHyperlink::setUrlComponents( const UrlComponents& rUrlComp ) throw (uno::RuntimeException)
{
    ensureTextField();
    OUStringBuffer aUrl( rUrlComp.first );
    if( !rUrlComp.second.isEmpty() )
        aUrl.append( '#' ).append( rUrlComp.second );
    mxTextField->setPropertyValue("URL", uno::Any( aUrl.makeStringAndClear() ) );
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
