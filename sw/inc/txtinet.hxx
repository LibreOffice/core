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
#ifndef _TXTINET_HXX
#define _TXTINET_HXX

#include <txatbase.hxx>
#include <txtatr.hxx>
#include <calbck.hxx>

class SwTxtNode;
class SwCharFmt;

// ATT_INETFMT *********************************************

class SW_DLLPUBLIC SwTxtINetFmt : public SwTxtAttrNesting, public SwClient
{
    SwTxtNode * m_pTxtNode;
    bool m_bVisited         : 1; // visited link?
    bool m_bVisitedValid    : 1; // is m_bVisited valid?

protected:
virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    SwTxtINetFmt( SwFmtINetFmt& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    virtual ~SwTxtINetFmt();
    TYPEINFO();

    virtual sal_Bool GetInfo( SfxPoolItem& rInfo ) const;

    SW_DLLPRIVATE void InitINetFmt(SwTxtNode & rNode);

    // get and set TxtNode pointer
    const SwTxtNode* GetpTxtNode() const { return m_pTxtNode; }
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

          SwCharFmt* GetCharFmt();
    const SwCharFmt* GetCharFmt() const
            { return const_cast<SwTxtINetFmt*>(this)->GetCharFmt(); }

    bool IsVisited() const { return m_bVisited; }
    void SetVisited( bool bNew ) { m_bVisited = bNew; }

    bool IsVisitedValid() const { return m_bVisitedValid; }
    void SetVisitedValid( bool bNew ) { m_bVisitedValid = bNew; }

    sal_Bool IsProtect() const;
};

inline const SwTxtNode& SwTxtINetFmt::GetTxtNode() const
{
    ASSERT( m_pTxtNode, "SwTxtINetFmt: where is my TxtNode?" );
    return *m_pTxtNode;
}

#endif

