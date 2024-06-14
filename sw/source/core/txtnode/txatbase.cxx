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

#include <optional>
#include <libxml/xmlwriter.h>
#include <svl/itempool.hxx>
#include <txatbase.hxx>
#include <fmtfld.hxx>

SwTextAttr::SwTextAttr( const SfxPoolItemHolder& rAttr, sal_Int32 nStart )
    : m_aAttr( rAttr )
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

const sal_Int32* SwTextAttr::GetEnd() const
{
    return nullptr;
}

void SwTextAttr::SetEnd(sal_Int32 )
{
    assert(false);
}

void SwTextAttr::Destroy( SwTextAttr * pToDestroy )
{
    if (!pToDestroy) return;
    delete pToDestroy;
}

bool SwTextAttr::operator==( const SwTextAttr& rAttr ) const
{
    return GetAttr() == rAttr.GetAttr();
}

SwTextAttrEnd::SwTextAttrEnd(
    const SfxPoolItemHolder& rAttr,
    sal_Int32 nStart,
    sal_Int32 nEnd ) :
    SwTextAttr( rAttr, nStart ), m_nEnd( nEnd )
{
}

const sal_Int32* SwTextAttrEnd::GetEnd() const
{
    return & m_nEnd;
}

void SwTextAttrEnd::SetEnd(sal_Int32 n)
{
    if (m_nEnd != n)
    {
        sal_Int32 nOldEndPos = m_nEnd;
        m_nEnd = n;
        if (m_pHints)
            m_pHints->EndPosChanged(Which(), GetStart(), nOldEndPos, m_nEnd);
    }
}

void SwTextAttr::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextAttr"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s",
                                      BAD_CAST(typeid(*this).name()));

    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("start"), BAD_CAST(OString::number(m_nStart).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("dont-expand"),
                                      BAD_CAST(OString::boolean(m_bDontExpand).getStr()));
    if (End())
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("end"), BAD_CAST(OString::number(*End()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("attr-item"), "%p", m_aAttr.getItem());
    const char* pWhich = nullptr;
    std::optional<OString> oValue;
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
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("which"), BAD_CAST(pWhich));
    if (oValue)
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(oValue->getStr()));
    switch (Which())
    {
        case RES_TXTATR_AUTOFMT:
            GetAutoFormat().dumpAsXml(pWriter);
            break;
        case RES_TXTATR_FIELD:
        case RES_TXTATR_ANNOTATION:
        case RES_TXTATR_INPUTFIELD:
            GetFormatField().dumpAsXml(pWriter);
            break;
        case RES_TXTATR_FTN:
            GetFootnote().dumpAsXml(pWriter);
            break;
        case RES_TXTATR_LINEBREAK:
            GetLineBreak().dumpAsXml(pWriter);
            break;
        case RES_TXTATR_META:
            break;
        case RES_TXTATR_CONTENTCONTROL:
            GetContentControl().dumpAsXml(pWriter);
            break;
        case RES_TXTATR_FLYCNT:
            GetFlyCnt().dumpAsXml(pWriter);
            break;
        case RES_TXTATR_CHARFMT:
            GetCharFormat().dumpAsXml(pWriter);
            break;
        case RES_TXTATR_REFMARK:
            GetRefMark().dumpAsXml(pWriter);
            break;
        case RES_TXTATR_INETFMT:
            GetINetFormat().dumpAsXml(pWriter);
            break;
        case RES_TXTATR_CJK_RUBY:
            GetRuby().dumpAsXml(pWriter);
            break;
        default:
            SAL_WARN("sw.core", "Unhandled TXTATR: " << Which());
            break;
    }

    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
