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

#include <rtl/ustring.hxx>
#include "txatbase.hxx"
#include "ndindex.hxx"

class SwNodeIndex;
class SwNodes;
class SwDoc;
class SwRootFrame;

/// SwTextAttr subclass for footnotes and endnotes. If it's an endnote is decided by the owned
/// SwFormatFootnote's m_bEndNote.
class SW_DLLPUBLIC SwTextFootnote final : public SwTextAttr
{
    std::optional<SwNodeIndex> m_oStartNode;
    SwTextNode * m_pTextNode;
    sal_uInt16 m_nSeqNo;

public:
    SwTextFootnote(
        const SfxPoolItemHolder& rAttr,
        sal_Int32 nStart );
    virtual ~SwTextFootnote() override;

    const SwNodeIndex *GetStartNode() const { return m_oStartNode ? &*m_oStartNode : nullptr; }
    void SetStartNode( const SwNodeIndex *pNode, bool bDelNodes = true );
    void SetNumber(sal_uInt16 nNumber, sal_uInt16 nNumberRLHidden, const OUString &sNumStr);
    void CopyFootnote(SwTextFootnote & rDest, SwTextNode & rDestNode) const;

    // Get and set TextNode pointer.
    inline const SwTextNode& GetTextNode() const;
    void ChgTextNode( SwTextNode* pNew ) { m_pTextNode = pNew; }

    // Create a new empty TextSection for this footnote.
    void MakeNewTextSection( SwNodes& rNodes );

    // Delete the FootnoteFrame from page.
    void DelFrames(const SwRootFrame *);

    // Check conditional paragraph styles.
    void CheckCondColl();

    void InvalidateNumberInLayout();

    // For references to footnotes.
    void SetSeqRefNo();
    void SetSeqNo( sal_uInt16 n )       { m_nSeqNo = n; }   // For Readers.
    sal_uInt16 GetSeqRefNo() const      { return m_nSeqNo; }

    static void SetUniqueSeqRefNo( SwDoc& rDoc );
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline const SwTextNode& SwTextFootnote::GetTextNode() const
{
    assert( m_pTextNode );
    return *m_pTextNode;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
