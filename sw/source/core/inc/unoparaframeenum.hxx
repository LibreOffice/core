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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOPARAFRAMEENUM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOPARAFRAMEENUM_HXX

#include <deque>

#include <calbck.hxx>
#include <unobaseclass.hxx>


class SwNodeIndex;
class SwPaM;
class SwFrameFormat;

namespace sw
{
    struct FrameClient final : public SwClient
    {
        FrameClient(sw::BroadcastingModify* pModify) : SwClient(pModify) {};
    };
}
struct FrameClientSortListEntry
{
    sal_Int32 nIndex;
    sal_uInt32 nOrder;
    std::shared_ptr<sw::FrameClient> pFrameClient;

    FrameClientSortListEntry (sal_Int32 const i_nIndex,
                sal_uInt32 const i_nOrder, std::shared_ptr<sw::FrameClient> i_pClient)
        : nIndex(i_nIndex), nOrder(i_nOrder), pFrameClient(std::move(i_pClient)) { }
};

typedef std::deque< FrameClientSortListEntry >
    FrameClientSortList_t;

typedef std::deque< std::shared_ptr<sw::FrameClient> >
    FrameClientList_t;

// #i28701# - adjust 4th parameter
void CollectFrameAtNode( const SwNodeIndex& rIdx,
                         FrameClientSortList_t& rFrames,
                         const bool bAtCharAnchoredObjs );

enum ParaFrameMode
{
    PARAFRAME_PORTION_PARAGRAPH,
    PARAFRAME_PORTION_CHAR,
    PARAFRAME_PORTION_TEXTRANGE,
};

struct SwXParaFrameEnumeration
    : public SwSimpleEnumeration_Base
{
    static rtl::Reference<SwXParaFrameEnumeration> Create(const SwPaM& rPaM, const enum ParaFrameMode eParaFrameMode, SwFrameFormat* const pFormat = nullptr);
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNOPARAFRAMEENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
