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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include "FillStyleContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/GradientStyle.hxx>
#include <xmloff/HatchStyle.hxx>
#include <xmloff/ImageStyle.hxx>
#include <TransGradientStyle.hxx>
#include <xmloff/MarkerStyle.hxx>
#include <xmloff/DashStyle.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>

using namespace ::com::sun::star;


XMLGradientStyleContext::XMLGradientStyleContext( SvXMLImport& rImport, sal_Int32 ,
                                              const uno::Reference< xml::sax::XFastAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport)
{
    // start import
    XMLGradientStyleImport aGradientStyle( GetImport() );
    aGradientStyle.importXML( xAttrList, maAny, maStrName );
}

XMLGradientStyleContext::~XMLGradientStyleContext()
{
}

void XMLGradientStyleContext::endFastElement(sal_Int32 )
{
    uno::Reference< container::XNameContainer > xGradient( GetImport().GetGradientHelper() );

    try
    {
        if(xGradient.is())
        {
            if( xGradient->hasByName( maStrName ) )
            {
                xGradient->replaceByName( maStrName, maAny );
            }
            else
            {
                xGradient->insertByName( maStrName, maAny );
            }
        }
    }
    catch( container::ElementExistException& )
    {}
}

bool XMLGradientStyleContext::IsTransient() const
{
    return true;
}


XMLHatchStyleContext::XMLHatchStyleContext( SvXMLImport& rImport, sal_Int32 /*nElement*/,
                                            const uno::Reference< xml::sax::XFastAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport)
{
    // start import
    XMLHatchStyleImport aHatchStyle( GetImport() );
    aHatchStyle.importXML( xAttrList, maAny, maStrName );
}

XMLHatchStyleContext::~XMLHatchStyleContext()
{
}

void XMLHatchStyleContext::endFastElement(sal_Int32 )
{
    uno::Reference< container::XNameContainer > xHatch( GetImport().GetHatchHelper() );

    try
    {
        if(xHatch.is())
        {
            if( xHatch->hasByName( maStrName ) )
            {
                xHatch->replaceByName( maStrName, maAny );
            }
            else
            {
                xHatch->insertByName( maStrName, maAny );
            }
        }
    }
    catch( container::ElementExistException& )
    {}
}

bool XMLHatchStyleContext::IsTransient() const
{
    return true;
}


XMLBitmapStyleContext::XMLBitmapStyleContext( SvXMLImport& rImport, sal_Int32 /*nElement*/,
                                              const uno::Reference< xml::sax::XFastAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport)
{
    // start import
    XMLImageStyle::importXML( xAttrList, maAny, maStrName, rImport );
}

XMLBitmapStyleContext::~XMLBitmapStyleContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLBitmapStyleContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if( nElement == XML_ELEMENT(OFFICE, xmloff::token::XML_BINARY_DATA) )
    {
        OUString sURL;
        maAny >>= sURL;
        if( sURL.isEmpty() && !mxBase64Stream.is() )
        {
            mxBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( mxBase64Stream.is() )
                return new XMLBase64ImportContext( GetImport(), mxBase64Stream );
        }
    }

    return nullptr;
}

void XMLBitmapStyleContext::endFastElement(sal_Int32 )
{
    if (!maAny.has<uno::Reference<graphic::XGraphic>>() && mxBase64Stream.is())
    {
        // No graphic so far? Then see if it's inline.
        uno::Reference<graphic::XGraphic> xGraphic = GetImport().loadGraphicFromBase64(mxBase64Stream);
        if (xGraphic.is())
        {
            maAny <<= xGraphic;
        }
    }

    if (!maAny.has<uno::Reference<graphic::XGraphic>>())
        return;

    uno::Reference<container::XNameContainer> xBitmapContainer(GetImport().GetBitmapHelper());

    uno::Reference<graphic::XGraphic> xGraphic = maAny.get<uno::Reference<graphic::XGraphic>>();
    uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);

    try
    {
        if (xBitmapContainer.is())
        {
            if (xBitmapContainer->hasByName(maStrName))
            {
                xBitmapContainer->replaceByName(maStrName, uno::Any(xBitmap));
            }
            else
            {
                xBitmapContainer->insertByName(maStrName, uno::Any(xBitmap));
            }
        }
    }
    catch (container::ElementExistException&)
    {}
}

bool XMLBitmapStyleContext::IsTransient() const
{
    return true;
}


XMLTransGradientStyleContext::XMLTransGradientStyleContext( SvXMLImport& rImport, sal_Int32 /*nElement*/,
                                              const uno::Reference< xml::sax::XFastAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport)
{
    // start import
    XMLTransGradientStyleImport aTransGradientStyle( GetImport() );
    aTransGradientStyle.importXML( xAttrList, maAny, maStrName );
}

XMLTransGradientStyleContext::~XMLTransGradientStyleContext()
{
}

void XMLTransGradientStyleContext::endFastElement(sal_Int32 )
{
    uno::Reference< container::XNameContainer > xTransGradient( GetImport().GetTransGradientHelper() );

    try
    {
        if(xTransGradient.is())
        {
            if( xTransGradient->hasByName( maStrName ) )
            {
                xTransGradient->replaceByName( maStrName, maAny );
            }
            else
            {
                xTransGradient->insertByName( maStrName, maAny );
            }
        }
    }
    catch( container::ElementExistException& )
    {}
}

bool XMLTransGradientStyleContext::IsTransient() const
{
    return true;
}


XMLMarkerStyleContext::XMLMarkerStyleContext( SvXMLImport& rImport, sal_Int32 /*nElement*/,
                                              const uno::Reference< xml::sax::XFastAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport)
{
    // start import
    XMLMarkerStyleImport aMarkerStyle( GetImport() );
    aMarkerStyle.importXML( xAttrList, maAny, maStrName );
}

XMLMarkerStyleContext::~XMLMarkerStyleContext()
{
}

void XMLMarkerStyleContext::endFastElement(sal_Int32 )
{
    uno::Reference< container::XNameContainer > xMarker( GetImport().GetMarkerHelper() );

    try
    {
        if(xMarker.is())
        {
            if( xMarker->hasByName( maStrName ) )
            {
                xMarker->replaceByName( maStrName, maAny );
            }
            else
            {
                xMarker->insertByName( maStrName, maAny );
            }
        }
    }
    catch( container::ElementExistException& )
    {}
}

bool XMLMarkerStyleContext::IsTransient() const
{
    return true;
}


XMLDashStyleContext::XMLDashStyleContext( SvXMLImport& rImport, sal_Int32 /*nElement*/,
                                          const uno::Reference< xml::sax::XFastAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport)
{
    // start import
    XMLDashStyleImport aDashStyle( GetImport() );
    aDashStyle.importXML( xAttrList, maAny, maStrName );
}

XMLDashStyleContext::~XMLDashStyleContext()
{
}

void XMLDashStyleContext::endFastElement(sal_Int32 )
{
    uno::Reference< container::XNameContainer > xDashes( GetImport().GetDashHelper() );

    try
    {
        if(xDashes.is())
        {
            if( xDashes->hasByName( maStrName ) )
            {
                xDashes->replaceByName( maStrName, maAny );
            }
            else
            {
                xDashes->insertByName( maStrName, maAny );
            }
        }
    }
    catch( container::ElementExistException& )
    {}
}

bool XMLDashStyleContext::IsTransient() const
{
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
