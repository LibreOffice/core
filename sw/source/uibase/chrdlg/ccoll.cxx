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

#include <cmdid.h>

#include <ccoll.hxx>
#include <fmtcol.hxx>
#include <o3tl/string_view.hxx>

//!! order of entries has to be the same as in
//!! CommandStruct SwCondCollItem::aCmds[]

// note: also keep this in sync with the list of conditions in xmloff/source/style/prstylecond.cxx

constexpr OUString aCommandContext[COND_COMMAND_COUNT] =
{
    u"TableHeader"_ustr,
    u"Table"_ustr,
    u"Frame"_ustr,
    u"Section"_ustr,
    u"Footnote"_ustr,
    u"Endnote"_ustr,
    u"Header"_ustr,
    u"Footer"_ustr,
    u"OutlineLevel1"_ustr,
    u"OutlineLevel2"_ustr,
    u"OutlineLevel3"_ustr,
    u"OutlineLevel4"_ustr,
    u"OutlineLevel5"_ustr,
    u"OutlineLevel6"_ustr,
    u"OutlineLevel7"_ustr,
    u"OutlineLevel8"_ustr,
    u"OutlineLevel9"_ustr,
    u"OutlineLevel10"_ustr,
    u"NumberingLevel1"_ustr,
    u"NumberingLevel2"_ustr,
    u"NumberingLevel3"_ustr,
    u"NumberingLevel4"_ustr,
    u"NumberingLevel5"_ustr,
    u"NumberingLevel6"_ustr,
    u"NumberingLevel7"_ustr,
    u"NumberingLevel8"_ustr,
    u"NumberingLevel9"_ustr,
    u"NumberingLevel10"_ustr
};

sal_Int16 GetCommandContextIndex( std::u16string_view rContextName )
{
    sal_Int16 nRes = -1;
    for (sal_Int16 i = 0;  nRes == -1 && i < COND_COMMAND_COUNT;  ++i)
    {
        if (aCommandContext[i] == rContextName)
            nRes = i;
    }
    return nRes;
}

OUString GetCommandContextByIndex( sal_Int16 nIndex )
{
    OUString aRes;
    if (0 <= nIndex  &&  nIndex < COND_COMMAND_COUNT)
    {
        aRes = aCommandContext[ nIndex ];
    }
    return aRes;
}

// Globals

const CommandStruct SwCondCollItem::s_aCmds[] =
{
    { Master_CollCondition::PARA_IN_TABLEHEAD,    0 },
    { Master_CollCondition::PARA_IN_TABLEBODY,    0 },
    { Master_CollCondition::PARA_IN_FRAME,        0 },
    { Master_CollCondition::PARA_IN_SECTION,      0 },
    { Master_CollCondition::PARA_IN_FOOTNOTE,     0 },
    { Master_CollCondition::PARA_IN_ENDNOTE,      0 },
    { Master_CollCondition::PARA_IN_HEADER,       0 },
    { Master_CollCondition::PARA_IN_FOOTER,       0 },
    { Master_CollCondition::PARA_IN_OUTLINE,      0 },
    { Master_CollCondition::PARA_IN_OUTLINE,      1 },
    { Master_CollCondition::PARA_IN_OUTLINE,      2 },
    { Master_CollCondition::PARA_IN_OUTLINE,      3 },
    { Master_CollCondition::PARA_IN_OUTLINE,      4 },
    { Master_CollCondition::PARA_IN_OUTLINE,      5 },
    { Master_CollCondition::PARA_IN_OUTLINE,      6 },
    { Master_CollCondition::PARA_IN_OUTLINE,      7 },
    { Master_CollCondition::PARA_IN_OUTLINE,      8 },
    { Master_CollCondition::PARA_IN_OUTLINE,      9 },
    { Master_CollCondition::PARA_IN_LIST,         0 },
    { Master_CollCondition::PARA_IN_LIST,         1 },
    { Master_CollCondition::PARA_IN_LIST,         2 },
    { Master_CollCondition::PARA_IN_LIST,         3 },
    { Master_CollCondition::PARA_IN_LIST,         4 },
    { Master_CollCondition::PARA_IN_LIST,         5 },
    { Master_CollCondition::PARA_IN_LIST,         6 },
    { Master_CollCondition::PARA_IN_LIST,         7 },
    { Master_CollCondition::PARA_IN_LIST,         8 },
    { Master_CollCondition::PARA_IN_LIST,         9 }
};


// Item for the transport of the condition table
SwCondCollItem::SwCondCollItem() :
    SfxPoolItem(FN_COND_COLL)
{
}

SwCondCollItem::~SwCondCollItem()
{
}

SwCondCollItem* SwCondCollItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwCondCollItem(*this);
}

bool SwCondCollItem::operator==( const SfxPoolItem& rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    bool bReturn = true;
    for(sal_uInt16 i = 0; i < COND_COMMAND_COUNT; i++)
        if (m_sStyles[i] !=
                static_cast<SwCondCollItem const&>(rItem).m_sStyles[i])
        {
            bReturn = false;
            break;
        }

    return bReturn;
}

OUString SwCondCollItem::GetStyle(sal_uInt16 const nPos) const
{
    return (nPos < COND_COMMAND_COUNT) ? m_sStyles[nPos] : OUString();
}

void
SwCondCollItem::SetStyle(OUString const*const pStyle, sal_uInt16 const nPos)
{
    if( nPos < COND_COMMAND_COUNT )
        m_sStyles[nPos] = pStyle ? *pStyle : OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
