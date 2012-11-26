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


#ifndef _TXTATR_HXX
#define _TXTATR_HXX

#include <txatbase.hxx>     // SwTxtAttr/SwTxtAttrEnd
#include <calbck.hxx>


class SwTxtNode;    // fuer SwTxtFld
class SwCharFmt;

namespace sw {
    class MetaFieldManager;
}


// ATT_CHARFMT *********************************************

class SwTxtCharFmt : public SwTxtAttrEnd
{
    SwTxtNode * m_pTxtNode;
    sal_uInt16 m_nSortNumber;

public:
    SwTxtCharFmt( SwFmtCharFmt& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    virtual ~SwTxtCharFmt( );

    // werden vom SwFmtCharFmt hierher weitergeleitet (no derivation from SwClient!)
    void ModifyNotification( const SfxPoolItem*, const SfxPoolItem* );
    bool GetInfo( SfxPoolItem& rInfo ) const;

    // get and set TxtNode pointer
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

    void SetSortNumber( sal_uInt16 nSortNumber ) { m_nSortNumber = nSortNumber; }
    sal_uInt16 GetSortNumber() const { return m_nSortNumber; }
};


// ******************************

class SwTxtAttrNesting : public SwTxtAttrEnd
{
protected:
    SwTxtAttrNesting( SfxPoolItem & i_rAttr,
        const xub_StrLen i_nStart, const xub_StrLen i_nEnd );
    virtual ~SwTxtAttrNesting();
};

class SwTxtMeta : public SwTxtAttrNesting
{
private:
    SwTxtMeta( SwFmtMeta & i_rAttr,
        const xub_StrLen i_nStart, const xub_StrLen i_nEnd );

public:
    static SwTxtMeta * CreateTxtMeta(
        ::sw::MetaFieldManager & i_rTargetDocManager,
        SwTxtNode *const i_pTargetTxtNode,
        SwFmtMeta & i_rAttr,
        xub_StrLen const i_nStart, xub_StrLen const i_nEnd,
        bool const i_bIsCopy);

    virtual ~SwTxtMeta();

    void ChgTxtNode(SwTxtNode * const pNode);
};


// ******************************

class SW_DLLPUBLIC SwTxtRuby : public SwTxtAttrNesting, public SwClient
{
    SwTxtNode* m_pTxtNode;
protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
public:
    SwTxtRuby( SwFmtRuby& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    virtual ~SwTxtRuby();

    virtual sal_Bool GetInfo( SfxPoolItem& rInfo ) const;

    SW_DLLPRIVATE void InitRuby(SwTxtNode & rNode);

    /// get and set TxtNode pointer
           const SwTxtNode* GetpTxtNode() const { return m_pTxtNode; }
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

          SwCharFmt* GetCharFmt();
    const SwCharFmt* GetCharFmt() const
            { return (const_cast<SwTxtRuby*>(this))->GetCharFmt(); }
};

// --------------- Inline Implementierungen ------------------------

inline const SwTxtNode& SwTxtRuby::GetTxtNode() const
{
    ASSERT( m_pTxtNode, "SwTxtRuby: where is my TxtNode?" );
    return *m_pTxtNode;
}

#endif
