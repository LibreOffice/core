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

#pragma once

#include <deque>

#include <calbck.hxx>
#include <unobaseclass.hxx>


class SwNode;
class SwNodeIndex;
class SwPaM;
class SwFrameFormat;

namespace com { namespace sun { namespace star { namespace text { class XTextContent; } } } }

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
    std::unique_ptr<sw::FrameClient> pFrameClient;

    FrameClientSortListEntry (sal_Int32 const i_nIndex,
                sal_uInt32 const i_nOrder, std::unique_ptr<sw::FrameClient> i_pClient)
        : nIndex(i_nIndex), nOrder(i_nOrder), pFrameClient(std::move(i_pClient)) { }
};

typedef std::deque< FrameClientSortListEntry >
    FrameClientSortList_t;

// #i28701# - adjust 4th parameter
void CollectFrameAtNode( const SwNode& rNd,
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

css::uno::Reference<css::text::XTextContent> FrameClientToXTextContent(sw::FrameClient* pClient);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
