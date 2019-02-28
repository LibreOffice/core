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
#ifndef INCLUDED_SW_INC_FMTCNTNT_HXX
#define INCLUDED_SW_INC_FMTCNTNT_HXX

#include <memory>
#include <svl/poolitem.hxx>
#include "hintids.hxx"
#include "format.hxx"

class SwNodeIndex;
class SwStartNode;

/// Content, content of frame (header, footer, fly).
class SW_DLLPUBLIC SwFormatContent: public SfxPoolItem
{
    std::unique_ptr<SwNodeIndex> m_pStartNode;

    SwFormatContent &operator=( const SwFormatContent & ) = delete;

public:
    SwFormatContent( const SwStartNode* pStartNode = nullptr );
    SwFormatContent( const SwFormatContent &rCpy );
    virtual ~SwFormatContent() override;

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;

    const SwNodeIndex *GetContentIdx() const { return m_pStartNode.get(); }
    void SetNewContentIdx( const SwNodeIndex *pIdx );

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline const SwFormatContent &SwAttrSet::GetContent(bool bInP) const
    { return Get( RES_CNTNT,bInP); }

inline const SwFormatContent &SwFormat::GetContent(bool bInP) const
    { return m_aSet.GetContent(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
