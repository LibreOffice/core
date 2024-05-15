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
#include <xmloff/xmltoken.hxx>

#include <sax/tools/converter.hxx>

#include <com/sun/star/xforms/XModel2.hpp>

#include <sal/log.hxx>

using com::sun::star::xforms::XModel2;

using namespace com::sun::star::uno;
using namespace xmloff::token;


XFormsSubmissionContext::XFormsSubmissionContext(
    SvXMLImport& rImport,
    const Reference<XModel2>& xModel ) :
        TokenContext( rImport )
{
    // register submission with model
    SAL_WARN_IF( !xModel.is(), "xmloff", "need model" );
    mxSubmission = xModel->createSubmission().get();
    SAL_WARN_IF( !mxSubmission.is(), "xmloff", "can't create submission" );
    xModel->getSubmissions()->insert( Any( mxSubmission ) );
}

namespace {

Any toBool( std::string_view rValue )
{
    Any aValue;
    bool bValue(false);
    if (::sax::Converter::convertBool( bValue, rValue ))
    {
        aValue <<= bValue;
    }
    return aValue;
}

} // namespace

void XFormsSubmissionContext::HandleAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch( aIter.getToken() & TOKEN_MASK )
    {
    case XML_ID:
        xforms_setValue( mxSubmission, u"ID"_ustr, aIter.toString() );
        break;
    case XML_BIND:
        xforms_setValue( mxSubmission, u"Bind"_ustr, aIter.toString() );
        break;
    case XML_REF:
        xforms_setValue( mxSubmission, u"Ref"_ustr, aIter.toString() );
        break;
    case XML_ACTION:
        xforms_setValue( mxSubmission, u"Action"_ustr, aIter.toString() );
        break;
    case XML_METHOD:
        xforms_setValue( mxSubmission, u"Method"_ustr, aIter.toString() );
        break;
    case XML_VERSION:
        xforms_setValue( mxSubmission, u"Version"_ustr, aIter.toString() );
        break;
    case XML_INDENT:
        xforms_setValue( mxSubmission, u"Indent"_ustr, toBool( aIter.toView() ) );
        break;
    case XML_MEDIATYPE:
        xforms_setValue( mxSubmission, u"MediaType"_ustr, aIter.toString() );
        break;
    case XML_ENCODING:
        xforms_setValue( mxSubmission, u"Encoding"_ustr, aIter.toString() );
        break;
    case XML_OMIT_XML_DECLARATION:
        xforms_setValue( mxSubmission, u"OmitXmlDeclaration"_ustr,
                      toBool( aIter.toView() ) );
        break;
    case XML_STANDALONE:
        xforms_setValue( mxSubmission, u"Standalone"_ustr, toBool( aIter.toView() ) );
        break;
    case XML_CDATA_SECTION_ELEMENTS:
        xforms_setValue( mxSubmission, u"CDataSectionElement"_ustr, aIter.toString() );
        break;
    case XML_REPLACE:
        xforms_setValue( mxSubmission, u"Replace"_ustr, aIter.toString() );
        break;
    case XML_SEPARATOR:
        xforms_setValue( mxSubmission, u"Separator"_ustr, aIter.toString() );
        break;
    case XML_INCLUDENAMESPACEPREFIXES:
        xforms_setValue( mxSubmission, u"IncludeNamespacePrefixes"_ustr, aIter.toString() );
        break;
    default:
        XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        assert( false && "unknown attribute" );
        break;
    }
}

/** will be called for each child element */
SvXMLImportContext* XFormsSubmissionContext::HandleChild(
    sal_Int32,
    const Reference<css::xml::sax::XFastAttributeList>& )
{
    assert( false && "no children supported" );
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
