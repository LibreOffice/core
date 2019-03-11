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

#include <boost/optional.hpp>
#include <libxml/xmlwriter.h>
#include <svl/itempool.hxx>
#include <txatbase.hxx>
#include <fmtfld.hxx>
#include <docufld.hxx>

SwTextAttr::SwTextAttr( SfxPoolItem& rAttr, sal_Int32 nStart )
    : m_pAttr( &rAttr )
    , m_nStart( nStart )
    , m_bDontExpand( false )
    , m_bLockExpandFlag( false )
    , m_bDontMoveAttr( false )
    , m_bCharFormatAttr( false )
    , m_bOverlapAllowedAttr( false )
    , m_bPriorityAttr( false )
    , m_bDontExpandStart( false )
    , m_bNesting( false )
    , m_bHasDummyChar( false )
    , m_bFormatIgnoreStart(false)
    , m_bFormatIgnoreEnd(false)
    , m_bHasContent( false )
{
}

SwTextAttr::~SwTextAttr() COVERITY_NOEXCEPT_FALSE
{
}

sal_Int32* SwTextAttr::GetEnd()
{
    return nullptr;
}

void SwTextAttr::Destroy( SwTextAttr * pToDestroy, SfxItemPool& rPool )
{
    if (!pToDestroy) return;
    SfxPoolItem * const pAttr = pToDestroy->m_pAttr;
    delete pToDestroy;
    rPool.Remove( *pAttr );
}

bool SwTextAttr::operator==( const SwTextAttr& rAttr ) const
{
    return GetAttr() == rAttr.GetAttr();
}

SwTextAttrEnd::SwTextAttrEnd( SfxPoolItem& rAttr,
        sal_Int32 nStart, sal_Int32 nEnd ) :
    SwTextAttr( rAttr, nStart ), m_nEnd( nEnd )
{
}

sal_Int32* SwTextAttrEnd::GetEnd()
{
    return & m_nEnd;
}

void SwTextAttr::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextAttr"));

    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("start"), BAD_CAST(OString::number(m_nStart).getStr()));
    if (End())
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("end"), BAD_CAST(OString::number(*End()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    const char* pWhich = nullptr;
    boost::optional<OString> oValue;
    switch (Which())
    {
    case RES_TXTATR_AUTOFMT:
        pWhich = "autofmt";
        break;
    case RES_TXTATR_ANNOTATION:
        pWhich = "annotation";
        break;
    case RES_TXTATR_FLYCNT:
        pWhich = "fly content";
        break;
    case RES_TXTATR_CHARFMT:
        {
            pWhich = "character format";
            if (SwCharFormat* pCharFormat = GetCharFormat().GetCharFormat())
                oValue = OString("name: " + OUStringToOString(pCharFormat->GetName(), RTL_TEXTENCODING_UTF8));
            break;
        }
    case RES_TXTATR_INETFMT:
        {
            pWhich = "inet format";
            const SwFormatINetFormat& rFormat = GetINetFormat();
            oValue = OString("url: " + rFormat.GetValue().toUtf8());
            break;
        }
    case RES_TXTATR_CJK_RUBY:
        {
            pWhich = "ruby";
            const SwFormatRuby& rFormat = GetRuby();
            oValue = OString("rubytext: " + rFormat.GetText().toUtf8());
            break;
        }
    case RES_TXTATR_META:
        {
            pWhich = "meta";
            break;
        }
    case RES_TXTATR_FIELD:
        {
            pWhich = "field";
            break;
        }
    default:
        break;
    }
    if (pWhich)
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("which"), BAD_CAST(pWhich));
    if (oValue)
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(oValue->getStr()));
    switch (Which())
    {
        case RES_TXTATR_AUTOFMT:
            GetAutoFormat().dumpAsXml(pWriter);
            break;
        case RES_TXTATR_FIELD:
            GetFormatField().dumpAsXml(pWriter);
            break;
        default:
            break;
    }

    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
