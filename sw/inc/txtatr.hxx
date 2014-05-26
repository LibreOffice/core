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

class SwTxtNode;
class SwCharFmt;

namespace sw {
    class MetaFieldManager;
}

class SwTxtCharFmt : public SwTxtAttrEnd
{
    SwTxtNode * m_pTxtNode;
    sal_uInt16 m_nSortNumber;

public:
    SwTxtCharFmt( SwFmtCharFmt& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    virtual ~SwTxtCharFmt( );

    // Passed from SwFmtCharFmt (no derivation from SwClient!).
    void ModifyNotification( const SfxPoolItem*, const SfxPoolItem* );
    bool GetInfo( SfxPoolItem& rInfo ) const;

    // get and set TxtNode pointer
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

    void SetSortNumber( sal_uInt16 nSortNumber ) { m_nSortNumber = nSortNumber; }
    sal_uInt16 GetSortNumber() const { return m_nSortNumber; }
};


class SwTxtAttrNesting : public SwTxtAttrEnd
{
protected:
    SwTxtAttrNesting( SfxPoolItem & i_rAttr,
        const sal_Int32 i_nStart, const sal_Int32 i_nEnd );
    virtual ~SwTxtAttrNesting();
};

class SwTxtMeta : public SwTxtAttrNesting
{
private:
    SwTxtMeta( SwFmtMeta & i_rAttr,
        const sal_Int32 i_nStart, const sal_Int32 i_nEnd );

public:
    static SwTxtMeta * CreateTxtMeta(
        ::sw::MetaFieldManager & i_rTargetDocManager,
        SwTxtNode *const i_pTargetTxtNode,
        SwFmtMeta & i_rAttr,
        sal_Int32 const i_nStart, sal_Int32 const i_nEnd,
        bool const i_bIsCopy);

    virtual ~SwTxtMeta();

    void ChgTxtNode(SwTxtNode * const pNode);
};


class SW_DLLPUBLIC SwTxtRuby : public SwTxtAttrNesting, public SwClient
{
    SwTxtNode* m_pTxtNode;
protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) SAL_OVERRIDE;
public:
    SwTxtRuby( SwFmtRuby& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    virtual ~SwTxtRuby();
    TYPEINFO_OVERRIDE();

    virtual bool GetInfo( SfxPoolItem& rInfo ) const SAL_OVERRIDE;

    SAL_DLLPRIVATE void InitRuby(SwTxtNode & rNode);

    /// Get and set TxtNode pointer.
           const SwTxtNode* GetpTxtNode() const { return m_pTxtNode; }
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

          SwCharFmt* GetCharFmt();
    const SwCharFmt* GetCharFmt() const
            { return (const_cast<SwTxtRuby*>(this))->GetCharFmt(); }
};

inline const SwTxtNode& SwTxtRuby::GetTxtNode() const
{
    assert( m_pTxtNode );
    return *m_pTxtNode;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
