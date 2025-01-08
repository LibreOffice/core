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
#ifndef INCLUDED_SW_INC_TXTATR_HXX
#define INCLUDED_SW_INC_TXTATR_HXX

#include "txatbase.hxx"
#include "calbck.hxx"

class SwTextNode;
class SwCharFormat;
class SwFormatMeta;

namespace sw {
    class MetaFieldManager;
    class AutoFormatUsedHint;
}

class SwTextCharFormat final : public SwTextAttrEnd
{
    SwTextNode * m_pTextNode;
    sal_uInt16 m_nSortNumber;

public:
    SwTextCharFormat(
        const SfxPoolItemHolder& rAttr,
        sal_Int32 nStart,
        sal_Int32 nEnd );
    virtual ~SwTextCharFormat( ) override;

    void TriggerNodeUpdate(const sw::LegacyModifyHint&);
    void TriggerNodeUpdate(const SwFormatChangeHint&);

    // get and set TextNode pointer
    void ChgTextNode( SwTextNode* pNew ) { m_pTextNode = pNew; }

    void SetSortNumber( sal_uInt16 nSortNumber ) { m_nSortNumber = nSortNumber; }
    sal_uInt16 GetSortNumber() const { return m_nSortNumber; }
    void HandleAutoFormatUsedHint(const sw::AutoFormatUsedHint&);
};


class SwTextMeta final : public SwTextAttrNesting
{
private:
    SwTextMeta(
        const SfxPoolItemHolder& rAttr,
        const sal_Int32 i_nStart,
        const sal_Int32 i_nEnd );

public:
    static SwTextMeta * CreateTextMeta(
        ::sw::MetaFieldManager & i_rTargetDocManager,
        SwTextNode *const i_pTargetTextNode,
        const SfxPoolItemHolder& rAttr,
        // SwFormatMeta & i_rAttr,
        sal_Int32 const i_nStart, sal_Int32 const i_nEnd,
        bool const i_bIsCopy);

    virtual ~SwTextMeta() override;

    void ChgTextNode(SwTextNode * const pNode);
};


class SW_DLLPUBLIC SwTextRuby final: public SwTextAttrNesting, public SwClient
{
    SwTextNode* m_pTextNode;
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;
public:
    SwTextRuby(
        const SfxPoolItemHolder& rAttr,
        sal_Int32 nStart,
        sal_Int32 nEnd );
    virtual ~SwTextRuby() override;

    SAL_DLLPRIVATE void InitRuby(SwTextNode & rNode);

    /// Get and set TextNode pointer.
    inline const SwTextNode& GetTextNode() const;
    void ChgTextNode( SwTextNode* pNew ) { m_pTextNode = pNew; }

          SwCharFormat* GetCharFormat();
    const SwCharFormat* GetCharFormat() const
            { return const_cast<SwTextRuby*>(this)->GetCharFormat(); }
};

inline const SwTextNode& SwTextRuby::GetTextNode() const
{
    assert( m_pTextNode );
    return *m_pTextNode;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
