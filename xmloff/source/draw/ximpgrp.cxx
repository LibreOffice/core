/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include"xmloff/xmlnmspe.hxx"
#include "ximpgrp.hxx"
#include <xmloff/xmltoken.hxx>
#include "ximpshap.hxx"
#include "eventimp.hxx"
#include "descriptionimp.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

//////////////////////////////////////////////////////////////////////////////

SdXMLGroupShapeContext::SdXMLGroupShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx, const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLGroupShapeContext::~SdXMLGroupShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLGroupShapeContext::CreateChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = 0L;

    // #i68101#
    if( nPrefix == XML_NAMESPACE_SVG &&
        (IsXMLToken( rLocalName, XML_TITLE ) || IsXMLToken( rLocalName, XML_DESC ) ) )
    {
        pContext = new SdXMLDescriptionContext( GetImport(), nPrefix, rLocalName, xAttrList, mxShape );
    }
    else if( nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
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

void SdXMLGroupShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>&)
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

    SdXMLShapeContext::EndElement();
}


