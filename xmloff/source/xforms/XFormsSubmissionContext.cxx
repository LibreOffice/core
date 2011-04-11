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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include "XFormsSubmissionContext.hxx"

#include "xformsapi.hxx"

#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlerror.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmltkmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xforms/XModel.hpp>

#include <tools/debug.hxx>

using rtl::OUString;
using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XNameContainer;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xforms::XModel;

using namespace com::sun::star::uno;
using namespace xmloff::token;




static struct SvXMLTokenMapEntry aAttributeMap[] =
{
    TOKEN_MAP_ENTRY( NONE, ID ),
    TOKEN_MAP_ENTRY( NONE, BIND ),
    TOKEN_MAP_ENTRY( NONE, REF ),
    TOKEN_MAP_ENTRY( NONE, ACTION ),
    TOKEN_MAP_ENTRY( NONE, METHOD ),
    TOKEN_MAP_ENTRY( NONE, VERSION ),
    TOKEN_MAP_ENTRY( NONE, INDENT ),
    TOKEN_MAP_ENTRY( NONE, MEDIATYPE ),
    TOKEN_MAP_ENTRY( NONE, ENCODING ),
    TOKEN_MAP_ENTRY( NONE, OMIT_XML_DECLARATION ),
    TOKEN_MAP_ENTRY( NONE, STANDALONE ),
    TOKEN_MAP_ENTRY( NONE, CDATA_SECTION_ELEMENTS ),
    TOKEN_MAP_ENTRY( NONE, REPLACE ),
    TOKEN_MAP_ENTRY( NONE, SEPARATOR ),
    TOKEN_MAP_ENTRY( NONE, INCLUDENAMESPACEPREFIXES ),
    XML_TOKEN_MAP_END
};

// helper function; see below
void lcl_fillNamespaceContainer( const SvXMLNamespaceMap&,
                                 Reference<XNameContainer>& );

XFormsSubmissionContext::XFormsSubmissionContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XPropertySet>& xModel ) :
        TokenContext( rImport, nPrefix, rLocalName, aAttributeMap, aEmptyMap ),
        mxSubmission()
{
    // register submission with model
    DBG_ASSERT( xModel.is(), "need model" );
    Reference<XModel> xXModel( xModel, UNO_QUERY );
    DBG_ASSERT( xXModel.is(), "need XModel" );
    mxSubmission = xXModel->createSubmission().get();
    DBG_ASSERT( mxSubmission.is(), "can't create submission" );
    xXModel->getSubmissions()->insert( makeAny( mxSubmission ) );
}

XFormsSubmissionContext::~XFormsSubmissionContext()
{
}

Any toBool( const OUString& rValue )
{
    Any aValue;
    bool bValue;
    if( SvXMLUnitConverter::convertBool( bValue, rValue ) )
    {
        aValue <<= ( bValue ? true : false );
    }
    return aValue;
}

void XFormsSubmissionContext::HandleAttribute( sal_uInt16 nToken,
                                               const OUString& rValue )
{
    switch( nToken )
    {
    case XML_ID:
        lcl_setValue( mxSubmission, OUSTRING("ID"), rValue );
        break;
    case XML_BIND:
        lcl_setValue( mxSubmission, OUSTRING("Bind"), rValue );
        break;
    case XML_REF:
        lcl_setValue( mxSubmission, OUSTRING("Ref"), rValue );
        break;
    case XML_ACTION:
        lcl_setValue( mxSubmission, OUSTRING("Action"), rValue );
        break;
    case XML_METHOD:
        lcl_setValue( mxSubmission, OUSTRING("Method"), rValue );
        break;
    case XML_VERSION:
        lcl_setValue( mxSubmission, OUSTRING("Version"), rValue );
        break;
    case XML_INDENT:
        lcl_setValue( mxSubmission, OUSTRING("Indent"), toBool( rValue ) );
        break;
    case XML_MEDIATYPE:
        lcl_setValue( mxSubmission, OUSTRING("MediaType"), rValue );
        break;
    case XML_ENCODING:
        lcl_setValue( mxSubmission, OUSTRING("Encoding"), rValue );
        break;
    case XML_OMIT_XML_DECLARATION:
        lcl_setValue( mxSubmission, OUSTRING("OmitXmlDeclaration"),
                      toBool( rValue ) );
        break;
    case XML_STANDALONE:
        lcl_setValue( mxSubmission, OUSTRING("Standalone"), toBool( rValue ) );
        break;
    case XML_CDATA_SECTION_ELEMENTS:
        lcl_setValue( mxSubmission, OUSTRING("CDataSectionElement"), rValue );
        break;
    case XML_REPLACE:
        lcl_setValue( mxSubmission, OUSTRING("Replace"), rValue );
        break;
    case XML_SEPARATOR:
        lcl_setValue( mxSubmission, OUSTRING("Separator"), rValue );
        break;
    case XML_INCLUDENAMESPACEPREFIXES:
        lcl_setValue( mxSubmission, OUSTRING("IncludeNamespacePrefixes"), rValue );
        break;
    default:
        OSL_FAIL( "unknown attribute" );
        break;
    }
}

/** will be called for each child element */
SvXMLImportContext* XFormsSubmissionContext::HandleChild(
    sal_uInt16,
    sal_uInt16,
    const OUString&,
    const Reference<XAttributeList>& )
{
    OSL_FAIL( "no children supported" );
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
