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

#ifndef SW_UNDO_SECTION_HXX
#define SW_UNDO_SECTION_HXX

#include <undobj.hxx>
#include <memory>
#include <swdllapi.h>

class SfxItemSet;
class SwTxtNode;
class SwSectionData;
class SwSectionFmt;
class SwTOXBase;

//------------ Undo of Insert-/Delete-Section ----------------------
class SwUndoInsSection : public SwUndo, private SwUndRng
{
private:
    const ::std::auto_ptr<SwSectionData> m_pSectionData;
    const ::std::auto_ptr<SwTOXBase> m_pTOXBase; /// set iff section is TOX
    const ::std::auto_ptr<SfxItemSet> m_pAttrSet;
    ::std::auto_ptr<SwHistory> m_pHistory;
    ::std::auto_ptr<SwRedlineData> m_pRedlData;
    sal_uLong m_nSectionNodePos;
    bool m_bSplitAtStart : 1;
    bool m_bSplitAtEnd : 1;
    bool m_bUpdateFtn : 1;

    void Join( SwDoc& rDoc, sal_uLong nNode );

public:
    SwUndoInsSection(SwPaM const&, SwSectionData const&,
        SfxItemSet const*const pSet, SwTOXBase const*const pTOXBase);

    virtual ~SwUndoInsSection();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetSectNdPos(sal_uLong const nPos)     { m_nSectionNodePos = nPos; }
    void SaveSplitNode(SwTxtNode *const pTxtNd, bool const bAtStart);
    void SetUpdtFtnFlag(bool const bFlag)   { m_bUpdateFtn = bFlag; }
};

SW_DLLPRIVATE SwUndo * MakeUndoDelSection(SwSectionFmt const&);

SW_DLLPRIVATE SwUndo * MakeUndoUpdateSection(SwSectionFmt const&, bool const);

#endif // SW_UNDO_SECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
