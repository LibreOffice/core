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

    virtual bool GetInfo( SfxPoolItem& rInfo ) const;

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
    assert( m_pTxtNode );
    return *m_pTxtNode;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
