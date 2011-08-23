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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include"xmlnmspe.hxx"

#include "ximpgrp.hxx"



#include "eventimp.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_EVENTS;
using ::binfilter::xmloff::token::XML_GLUE_POINT;

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLGroupShapeContext, SvXMLImportContext );

SdXMLGroupShapeContext::SdXMLGroupShapeContext( 
    SvXMLImport& rImport,
    USHORT nPrfx, const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes) 
:	SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLGroupShapeContext::~SdXMLGroupShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLGroupShapeContext::CreateChildContext( USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = 0L;
    
    if( nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_EVENTS ) )
    {
        pContext = new SdXMLEventsContext( GetImport(), nPrefix, rLocalName, xAttrList, mxShape );
    }
    else if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_GLUE_POINT ) )
    {
        addGluePoint( xAttrList );
    }
    else
    {
        // call GroupChildContext function at common ShapeImport
        pContext = GetImport().GetShapeImport()->CreateGroupChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList, mxChilds);
    }

    // call parent when no own context was created
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext(
        nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLGroupShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create new group shape and add it to rShapes, use it
    // as base for the new group import
    AddShape( "com.sun.star.drawing.GroupShape" );

    if(mxShape.is())
    {
        SetStyle( false );

        mxChilds = uno::Reference< drawing::XShapes >::query( mxShape );
        if( mxChilds.is() )
            GetImport().GetShapeImport()->pushGroupForSorting( mxChilds );
    }

    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLGroupShapeContext::EndElement()
{
    if( mxChilds.is() )
        GetImport().GetShapeImport()->popGroupAndSort();
}


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
