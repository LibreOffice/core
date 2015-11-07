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

#include <txatbase.hxx>
#include <calbck.hxx>

class SwTextNode;
class SwCharFormat;

namespace sw {
    class MetaFieldManager;
}

class SwTextCharFormat : public SwTextAttrEnd
{
    SwTextNode * m_pTextNode;
    sal_uInt16 m_nSortNumber;

public:
    SwTextCharFormat( SwFormatCharFormat& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    virtual ~SwTextCharFormat( );

    // Passed from SwFormatCharFormat (no derivation from SwClient!).
    void ModifyNotification( const SfxPoolItem*, const SfxPoolItem* );
    bool GetInfo( SfxPoolItem& rInfo ) const;

    // get and set TextNode pointer
    void ChgTextNode( SwTextNode* pNew ) { m_pTextNode = pNew; }

    void SetSortNumber( sal_uInt16 nSortNumber ) { m_nSortNumber = nSortNumber; }
    sal_uInt16 GetSortNumber() const { return m_nSortNumber; }
};


class SwTextMeta : public SwTextAttrNesting
{
private:
    SwTextMeta( SwFormatMeta & i_rAttr,
        const sal_Int32 i_nStart, const sal_Int32 i_nEnd );

public:
    static SwTextMeta * CreateTextMeta(
        ::sw::MetaFieldManager & i_rTargetDocManager,
        SwTextNode *const i_pTargetTextNode,
        SwFormatMeta & i_rAttr,
        sal_Int32 const i_nStart, sal_Int32 const i_nEnd,
        bool const i_bIsCopy);

    virtual ~SwTextMeta();

    void ChgTextNode(SwTextNode * const pNode);
};


class SW_DLLPUBLIC SwTextRuby : public SwTextAttrNesting, public SwClient
{
    SwTextNode* m_pTextNode;
protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
public:
    SwTextRuby( SwFormatRuby& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    virtual ~SwTextRuby();

    virtual bool GetInfo( SfxPoolItem& rInfo ) const override;

    SAL_DLLPRIVATE void InitRuby(SwTextNode & rNode);

    /// Get and set TextNode pointer.
    inline const SwTextNode& GetTextNode() const;
    void ChgTextNode( SwTextNode* pNew ) { m_pTextNode = pNew; }

          SwCharFormat* GetCharFormat();
    const SwCharFormat* GetCharFormat() const
            { return (const_cast<SwTextRuby*>(this))->GetCharFormat(); }
};

inline const SwTextNode& SwTextRuby::GetTextNode() const
{
    assert( m_pTextNode );
    return *m_pTextNode;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
