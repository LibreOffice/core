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


#include "XFormsSubmissionContext.hxx"

#include "xformsapi.hxx"

#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlerror.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmltkmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>

#include <sax/tools/converter.hxx>

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
    bool bValue(false);
    if (::sax::Converter::convertBool( bValue, rValue ))
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
        xforms_setValue( mxSubmission, OUSTRING("ID"), rValue );
        break;
    case XML_BIND:
        xforms_setValue( mxSubmission, OUSTRING("Bind"), rValue );
        break;
    case XML_REF:
        xforms_setValue( mxSubmission, OUSTRING("Ref"), rValue );
        break;
    case XML_ACTION:
        xforms_setValue( mxSubmission, OUSTRING("Action"), rValue );
        break;
    case XML_METHOD:
        xforms_setValue( mxSubmission, OUSTRING("Method"), rValue );
        break;
    case XML_VERSION:
        xforms_setValue( mxSubmission, OUSTRING("Version"), rValue );
        break;
    case XML_INDENT:
        xforms_setValue( mxSubmission, OUSTRING("Indent"), toBool( rValue ) );
        break;
    case XML_MEDIATYPE:
        xforms_setValue( mxSubmission, OUSTRING("MediaType"), rValue );
        break;
    case XML_ENCODING:
        xforms_setValue( mxSubmission, OUSTRING("Encoding"), rValue );
        break;
    case XML_OMIT_XML_DECLARATION:
        xforms_setValue( mxSubmission, OUSTRING("OmitXmlDeclaration"),
                      toBool( rValue ) );
        break;
    case XML_STANDALONE:
        xforms_setValue( mxSubmission, OUSTRING("Standalone"), toBool( rValue ) );
        break;
    case XML_CDATA_SECTION_ELEMENTS:
        xforms_setValue( mxSubmission, OUSTRING("CDataSectionElement"), rValue );
        break;
    case XML_REPLACE:
        xforms_setValue( mxSubmission, OUSTRING("Replace"), rValue );
        break;
    case XML_SEPARATOR:
        xforms_setValue( mxSubmission, OUSTRING("Separator"), rValue );
        break;
    case XML_INCLUDENAMESPACEPREFIXES:
        xforms_setValue( mxSubmission, OUSTRING("IncludeNamespacePrefixes"), rValue );
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
