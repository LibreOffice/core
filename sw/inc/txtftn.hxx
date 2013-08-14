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
#ifndef _TXTFTN_HXX
#define _TXTFTN_HXX

#include <txatbase.hxx>

namespace rtl { class OUString; }

class SwNodeIndex;
class SwTxtNode;
class SwNodes;
class SwDoc;
class SwFrm;

// ATT_FTN **********************************************************

class SW_DLLPUBLIC SwTxtFtn : public SwTxtAttr
{
    SwNodeIndex * m_pStartNode;
    SwTxtNode * m_pTxtNode;
    sal_uInt16 m_nSeqNo;

public:
    SwTxtFtn( SwFmtFtn& rAttr, xub_StrLen nStart );
    virtual ~SwTxtFtn();

    inline SwNodeIndex *GetStartNode() const { return m_pStartNode; }
    void SetStartNode( const SwNodeIndex *pNode, sal_Bool bDelNodes = sal_True );
    void SetNumber( const sal_uInt16 nNumber, const OUString &sNumStr );
    void CopyFtn(SwTxtFtn & rDest, SwTxtNode & rDestNode) const;

    // Get and set TxtNode pointer.
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

    // Create a new empty TextSection for this footnote.
    void MakeNewTextSection( SwNodes& rNodes );

    // Delete the FtnFrame from page.
    void DelFrms( const SwFrm* );

    // Check conditional paragraph styles.
    void CheckCondColl();

    // For references to footnotes.
    sal_uInt16 SetSeqRefNo();
    void SetSeqNo( sal_uInt16 n )       { m_nSeqNo = n; }   // For Readers.
    sal_uInt16 GetSeqRefNo() const      { return m_nSeqNo; }

    static void SetUniqueSeqRefNo( SwDoc& rDoc );
};

inline const SwTxtNode& SwTxtFtn::GetTxtNode() const
{
    assert( m_pTxtNode );
    return *m_pTxtNode;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
