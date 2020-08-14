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
    sal_uInt16 nPrfx, const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLGroupShapeContext::~SdXMLGroupShapeContext()
{
}

SvXMLImportContextRef SdXMLGroupShapeContext::CreateChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContextRef xContext;

    // #i68101#
    if( nPrefix == XML_NAMESPACE_SVG &&
        (IsXMLToken( rLocalName, XML_TITLE ) || IsXMLToken( rLocalName, XML_DESC ) ) )
    {
        xContext = new SdXMLDescriptionContext( GetImport(), nPrefix, rLocalName, xAttrList, mxShape );
    }
    else if( nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
    {
        xContext = new SdXMLEventsContext( GetImport(), nPrefix, rLocalName, xAttrList, mxShape );
    }
    else if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_GLUE_POINT ) )
    {
        addGluePoint( xAttrList );
    }
    else
    {
        // call GroupChildContext function at common ShapeImport
        xContext = GetImport().GetShapeImport()->CreateGroupChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList, mxChildren);
    }

    return xContext;
}

void SdXMLGroupShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>&)
{
    // create new group shape and add it to rShapes, use it
    // as base for the new group import
    AddShape( "com.sun.star.drawing.GroupShape" );

    if(mxShape.is())
    {
        SetStyle( false );

        mxChildren.set( mxShape, uno::UNO_QUERY );
        if( mxChildren.is() )
            GetImport().GetShapeImport()->pushGroupForPostProcessing( mxChildren );
    }

    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
}

void SdXMLGroupShapeContext::EndElement()
{
    if( mxChildren.is() )
        GetImport().GetShapeImport()->popGroupAndPostProcess();

    SdXMLShapeContext::EndElement();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
