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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOSECTION_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOSECTION_HXX

#include <undobj.hxx>
#include <memory>
#include <swdllapi.h>

class SfxItemSet;
class SwTextNode;
class SwSectionData;
class SwSectionFormat;
class SwTOXBase;

namespace sw {
    enum class RedlineMode;
};

class SwUndoInsSection : public SwUndo, private SwUndRng
{
private:
    const std::unique_ptr<SwSectionData> m_pSectionData;
    const std::unique_ptr<std::pair<SwTOXBase *, sw::RedlineMode>> m_pTOXBase; /// set iff section is TOX
    const std::unique_ptr<SfxItemSet> m_pAttrSet;
    std::unique_ptr<SwHistory> m_pHistory;
    std::unique_ptr<SwRedlineData> m_pRedlData;
    std::unique_ptr<SwRedlineSaveDatas> m_pRedlineSaveData;
    sal_uLong m_nSectionNodePos;
    bool m_bSplitAtStart : 1;
    bool m_bSplitAtEnd : 1;
    bool m_bUpdateFootnote : 1;

    void Join( SwDoc& rDoc, sal_uLong nNode );

public:
    SwUndoInsSection(SwPaM const&, SwSectionData const&,
        SfxItemSet const* pSet,
        std::pair<SwTOXBase const*, sw::RedlineMode> const* pTOXBase);

    virtual ~SwUndoInsSection() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetSectNdPos(sal_uLong const nPos)     { m_nSectionNodePos = nPos; }
    void SaveSplitNode(SwTextNode *const pTextNd, bool const bAtStart);
    void SetUpdateFootnoteFlag(bool const bFlag)   { m_bUpdateFootnote = bFlag; }
};

std::unique_ptr<SwUndo> MakeUndoDelSection(SwSectionFormat const&);

std::unique_ptr<SwUndo> MakeUndoUpdateSection(SwSectionFormat const&, bool const);

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOSECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
