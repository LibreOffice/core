/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _TXTATR_HXX
#define _TXTATR_HXX

#include <txatbase.hxx>     // SwTxtAttr/SwTxtAttrEnd
#include <calbck.hxx>


class SwTxtNode;    // fuer SwTxtFld
class SwCharFmt;

// ATT_CHARFMT *********************************************

class SwTxtCharFmt : public SwTxtAttrEnd
{
    SwTxtNode * m_pTxtNode;
    sal_uInt16 m_nSortNumber;

public:
    SwTxtCharFmt( SwFmtCharFmt& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    virtual ~SwTxtCharFmt( );

<<<<<<< local
    // werden vom SwFmtCharFmt hierher weitergeleitet (no derivation from SwClient!)
    void ModifyNotification( const SfxPoolItem*, const SfxPoolItem* );
    bool GetInfo( SfxPoolItem& rInfo ) const;
=======
    // werden vom SwFmtCharFmt hierher weitergeleitet
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );    // SwClient
    virtual sal_Bool GetInfo( SfxPoolItem& rInfo ) const;
>>>>>>> other

    // get and set TxtNode pointer
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

    void SetSortNumber( sal_uInt16 nSortNumber ) { m_nSortNumber = nSortNumber; }
    sal_uInt16 GetSortNumber() const { return m_nSortNumber; }
};


// ******************************

class SwTxtAttrNesting : public SwTxtAttrEnd
{
public:
    SwTxtAttrNesting( SfxPoolItem & i_rAttr,
        const xub_StrLen i_nStart, const xub_StrLen i_nEnd );
    virtual ~SwTxtAttrNesting();
};

class SwTxtMeta : public SwTxtAttrNesting
{
private:
    SwTxtNode * m_pTxtNode;

public:
    SwTxtMeta( SwFmtMeta & i_rAttr,
        const xub_StrLen i_nStart, const xub_StrLen i_nEnd );
    virtual ~SwTxtMeta();

    void ChgTxtNode(SwTxtNode * const pNode);
    SwTxtNode * GetTxtNode() const { return m_pTxtNode; }

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
    TYPEINFO();

<<<<<<< local
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;
=======
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    virtual sal_Bool GetInfo( SfxPoolItem& rInfo ) const;
>>>>>>> other

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
