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
#ifndef INCLUDED_SW_INC_TXTFTN_HXX
#define INCLUDED_SW_INC_TXTFTN_HXX

#include <txatbase.hxx>

namespace rtl { class OUString; }

class SwNodeIndex;
class SwTextNode;
class SwNodes;
class SwDoc;
class SwFrame;

class SW_DLLPUBLIC SwTextFootnote : public SwTextAttr
{
    SwNodeIndex * m_pStartNode;
    SwTextNode * m_pTextNode;
    sal_uInt16 m_nSeqNo;

public:
    SwTextFootnote( SwFormatFootnote& rAttr, sal_Int32 nStart );
    virtual ~SwTextFootnote();

    inline SwNodeIndex *GetStartNode() const { return m_pStartNode; }
    void SetStartNode( const SwNodeIndex *pNode, bool bDelNodes = true );
    void SetNumber( const sal_uInt16 nNumber, const OUString &sNumStr );
    void CopyFootnote(SwTextFootnote & rDest, SwTextNode & rDestNode) const;

    // Get and set TextNode pointer.
    inline const SwTextNode& GetTextNode() const;
    void ChgTextNode( SwTextNode* pNew ) { m_pTextNode = pNew; }

    // Create a new empty TextSection for this footnote.
    void MakeNewTextSection( SwNodes& rNodes );

    // Delete the FootnoteFrame from page.
    void DelFrames( const SwFrame* );

    // Check conditional paragraph styles.
    void CheckCondColl();

    // For references to footnotes.
    sal_uInt16 SetSeqRefNo();
    void SetSeqNo( sal_uInt16 n )       { m_nSeqNo = n; }   // For Readers.
    sal_uInt16 GetSeqRefNo() const      { return m_nSeqNo; }

    static void SetUniqueSeqRefNo( SwDoc& rDoc );
};

inline const SwTextNode& SwTextFootnote::GetTextNode() const
{
    assert( m_pTextNode );
    return *m_pTextNode;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
