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

#include<xmloff/xmlnamespace.hxx>
#include "ximpgrp.hxx"
#include <xmloff/xmltoken.hxx>
#include "ximpshap.hxx"
#include "eventimp.hxx"
#include "descriptionimp.hxx"

using namespace ::com::sun::star;
using namespace ::xmloff::token;


SdXMLGroupShapeContext::SdXMLGroupShapeContext(
    SvXMLImport& rImport,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLGroupShapeContext::~SdXMLGroupShapeContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLGroupShapeContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // #i68101#
    if( nElement == XML_ELEMENT(SVG, XML_TITLE) ||
        nElement == XML_ELEMENT(SVG, XML_DESC ) ||
        nElement == XML_ELEMENT(SVG_COMPAT, XML_TITLE) ||
        nElement == XML_ELEMENT(SVG_COMPAT, XML_DESC ) )
    {
        return new SdXMLDescriptionContext( GetImport(), nElement, mxShape );
    }
    else if( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) )
    {
        return new SdXMLEventsContext( GetImport(), mxShape );
    }
    else if( nElement == XML_ELEMENT(DRAW, XML_GLUE_POINT) )
    {
        addGluePoint( xAttrList );
    }
    else
    {
        // call GroupChildContext function at common ShapeImport
        return XMLShapeImportHelper::CreateGroupChildContext(
            GetImport(), nElement, xAttrList, mxChildren);
    }
    return nullptr;
}

void SdXMLGroupShapeContext::startFastElement (sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/)
{
    // create new group shape and add it to rShapes, use it
    // as base for the new group import
    AddShape( u"com.sun.star.drawing.GroupShape"_ustr );

    if(mxShape.is())
    {
        SetStyle( false );

        mxChildren.set( mxShape, uno::UNO_QUERY );
        if( mxChildren.is() )
            GetImport().GetShapeImport()->pushGroupForPostProcessing( mxChildren );
    }

    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
}

void SdXMLGroupShapeContext::endFastElement(sal_Int32 nElement)
{
    if( mxChildren.is() )
        GetImport().GetShapeImport()->popGroupAndPostProcess();

    SdXMLShapeContext::endFastElement(nElement);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
