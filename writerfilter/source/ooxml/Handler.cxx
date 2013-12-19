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
        mpFastContext->resolveFootnote(val.getInt());
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
        mpFastContext->resolveEndnote(val.getInt());
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
        mpFastContext->resolveOLE(val.getString());
        break;
    default:
        ;
    }
}

void OOXMLOLEHandler::sprm(Sprm & /*sprm*/)
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
OOXMLBreakHandler::OOXMLBreakHandler(Stream &rStream,
                                     OOXMLFastContextHandler * pContext)
: mpFastContext(pContext), mnType(0), mnClear(0),
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
    ::rtl::OUString sReturn(RTL_CONSTASCII_USTRINGPARAM(" HYPERLINK \""));

    sReturn += mURL;
    sReturn += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\""));
    sReturn += mFieldCode;

    mpFastContext->characters(sReturn);
}

void OOXMLHyperlinkHandler::attribute(Id name, Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_Hyperlink_tgtFrame:
        mFieldCode += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" \\t \""));
        mFieldCode += val.getString();
        mFieldCode += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\""));
        break;
    case NS_ooxml::LN_CT_Hyperlink_tooltip:
        mFieldCode += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" \\o \""));
        mFieldCode += val.getString();
        mFieldCode += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\""));
        break;
    case NS_ooxml::LN_CT_Hyperlink_docLocation:
        break;
    case NS_ooxml::LN_CT_Hyperlink_history:
        break;
    case NS_ooxml::LN_CT_Hyperlink_anchor:
        mFieldCode += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" \\l \""));
        mFieldCode += val.getString();
        mFieldCode += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\""));
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
