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

#include <ooxml/resourceids.hxx>
#include <resourcemodel/QNameToString.hxx>
#include "Handler.hxx"

namespace writerfilter {
namespace ooxml
{

/*
  class OOXMLFootnoteHandler
 */
OOXMLFootnoteHandler::OOXMLFootnoteHandler(OOXMLFastContextHandler * pContext)
: mpFastContext(pContext)
{
}

OOXMLFootnoteHandler::~OOXMLFootnoteHandler()
{
}

void OOXMLFootnoteHandler::attribute(Id name, Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_FtnEdnRef_id:
        mpFastContext->resolveFootnote(sal_Int32(val.getInt()));
        break;
    default:
        break;
    }
}

void OOXMLFootnoteHandler::sprm(Sprm & /*sprm*/)
{
}

/*
  class OOXMLEndnoteHandler
 */
OOXMLEndnoteHandler::OOXMLEndnoteHandler(OOXMLFastContextHandler * pContext)
: mpFastContext(pContext)
{
}

OOXMLEndnoteHandler::~OOXMLEndnoteHandler()
{
}

void OOXMLEndnoteHandler::attribute(Id name, Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_FtnEdnRef_id:
        mpFastContext->resolveEndnote(sal_Int32(val.getInt()));
        break;
    default:
        break;
    }
}

void OOXMLEndnoteHandler::sprm(Sprm & /*sprm*/)
{
}

/*
  class OOXMLCommentHandler
*/
OOXMLCommentHandler::OOXMLCommentHandler(OOXMLFastContextHandler * pContext)
: mpFastContext(pContext)
{
}

OOXMLCommentHandler::~OOXMLCommentHandler()
{
}

void OOXMLCommentHandler::attribute(Id name, Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_Markup_id:
        mpFastContext->resolveComment(val.getInt());
        break;
    default:
        ;
    }
}

void OOXMLCommentHandler::sprm(Sprm & /*sprm*/)
{
}

/*
   class OOXMLOLEHandler
*/
OOXMLOLEHandler::OOXMLOLEHandler(OOXMLFastContextHandler * pContext)
: mpFastContext(pContext)
{
}

OOXMLOLEHandler::~OOXMLOLEHandler()
{
}

void OOXMLOLEHandler::attribute(Id name, Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_OLEObject_r_id:
        mpFastContext->resolveData(val.getString());
        break;
    default:
        ;
    }
}

void OOXMLOLEHandler::sprm(Sprm & /*sprm*/)
{
}

OOXMLEmbeddedFontHandler::OOXMLEmbeddedFontHandler(OOXMLFastContextHandler * pContext)
: mpFastContext(pContext)
{
}

OOXMLEmbeddedFontHandler::~OOXMLEmbeddedFontHandler()
{
}

void OOXMLEmbeddedFontHandler::attribute(Id name, Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_Rel_id:
        mpFastContext->resolveData(val.getString());
        break;
    default:
        break;
    }
}

void OOXMLEmbeddedFontHandler::sprm(Sprm & /*sprm*/)
{
}

/*
  class OOXMLFooterHandler
 */
OOXMLFooterHandler::OOXMLFooterHandler(OOXMLFastContextHandler * pContext)
: mpFastContext(pContext), msStreamId(), mnType(0)
{
}

OOXMLFooterHandler::~OOXMLFooterHandler()
{
    mpFastContext->resolveFooter(mnType, msStreamId);
}

void OOXMLFooterHandler::attribute(Id name, Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_HdrFtrRef_id:
        msStreamId = val.getString();
        break;
    case NS_ooxml::LN_CT_HdrFtrRef_type:
        mnType = val.getInt();
        break;
    default:
        break;
    }
}

void OOXMLFooterHandler::sprm(Sprm & /*sprm*/)
{
}

/*
  class OOXMLHeaderHandler
 */
OOXMLHeaderHandler::OOXMLHeaderHandler(OOXMLFastContextHandler * pContext)
: mpFastContext(pContext), msStreamId(), mnType(0)
{
}

OOXMLHeaderHandler::~OOXMLHeaderHandler()
{
    mpFastContext->resolveHeader(mnType, msStreamId);
}

void OOXMLHeaderHandler::attribute(Id name, Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_HdrFtrRef_id:
        msStreamId = val.getString();
        break;
    case NS_ooxml::LN_CT_HdrFtrRef_type:
        mnType = val.getInt();
        break;
    default:
        break;
    }
}

void OOXMLHeaderHandler::sprm(Sprm & /*sprm*/)
{
}

/*
  class OOXMLBreakHandler
 */
OOXMLBreakHandler::OOXMLBreakHandler(Stream &rStream)
: mnType(0), mnClear(0),
  mrStream(rStream)
{
}

OOXMLBreakHandler::~OOXMLBreakHandler()
{
    sal_uInt8 tmpBreak[1];
    switch (mnType)
    {
    case NS_ooxml::LN_Value_ST_BrType_column:
        tmpBreak[0] = 0x0E;
        break;
    case NS_ooxml::LN_Value_ST_BrType_page:
        tmpBreak[0] = 0x0C;
        break;
    case NS_ooxml::LN_Value_ST_BrType_textWrapping:
    default: // when no attribute type is present, the spec assume textWrapping
        tmpBreak[0] = 0x0A;
        break;
    }
    mrStream.text(&tmpBreak[0], 1);
}

void OOXMLBreakHandler::attribute(Id name, Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_Br_type:
        mnType = val.getInt();
        break;
    case NS_ooxml::LN_CT_Br_clear:
        mnClear = val.getInt();
        break;
    default:
        break;
    }
}

void OOXMLBreakHandler::sprm(Sprm & /*sprm*/)
{
}

/*
  class OOXMLPictureHandler
 */
OOXMLPictureHandler::OOXMLPictureHandler(OOXMLFastContextHandler * pContext)
: mpFastContext(pContext)
{
}

OOXMLPictureHandler::~OOXMLPictureHandler()
{
}

void OOXMLPictureHandler::attribute(Id name, Value & val)
{
    if (name == NS_ooxml::LN_AG_Blob_r_embed)
        mpFastContext->resolvePicture(val.getString());
    else
    {
        writerfilter::Reference<Properties>::Pointer_t pProps
            (val.getProperties());
        if (pProps.get() != NULL)
            pProps->resolve(*this);
    }
}

void OOXMLPictureHandler::sprm(Sprm & rSprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProps
        (rSprm.getProps());

    if (pProps.get() != NULL)
        pProps->resolve(*this);
}

/**
   class OOXMLHyperlinkHandler
 */

OOXMLHyperlinkHandler::OOXMLHyperlinkHandler(OOXMLFastContextHandler * pContext)
: mpFastContext(pContext)
{
}

OOXMLHyperlinkHandler::~OOXMLHyperlinkHandler()
{
    OUString sReturn(" HYPERLINK \"");

    sReturn += mURL;
    sReturn += "\"";
    sReturn += mFieldCode;

    mpFastContext->text(sReturn);
}

void OOXMLHyperlinkHandler::attribute(Id name, Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_Hyperlink_tgtFrame:
        mFieldCode += " \\t \"";
        mFieldCode += val.getString();
        mFieldCode += "\"";
        break;
    case NS_ooxml::LN_CT_Hyperlink_tooltip:
        mFieldCode += " \\o \"";
        mFieldCode += val.getString();
        mFieldCode += "\"";
        break;
    case NS_ooxml::LN_CT_Hyperlink_docLocation:
        break;
    case NS_ooxml::LN_CT_Hyperlink_history:
        break;
    case NS_ooxml::LN_CT_Hyperlink_anchor:
        mFieldCode += " \\l \"";
        mFieldCode += val.getString();
        mFieldCode += "\"";
        break;
    case NS_ooxml::LN_CT_Hyperlink_r_id:
        mURL = mpFastContext->getTargetForId(val.getString());
        break;
    default:
        break;
    }
}

void OOXMLHyperlinkHandler::sprm(Sprm & /*rSprm*/)
{
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
