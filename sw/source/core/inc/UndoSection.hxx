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

#include <o3tl/deleter.hxx>
#include <undobj.hxx>
#include <tuple>
#include <memory>
#include <optional>

class SfxItemSet;
class SwTextNode;
class SwSectionData;
class SwSectionFormat;
class SwTOXBase;

namespace sw {
    enum class RedlineMode;
    enum class FieldmarkMode;
};

class SwUndoInsSection final : public SwUndo, private SwUndRng
{
private:
    const std::unique_ptr<SwSectionData> m_pSectionData;
    std::optional<std::tuple<std::unique_ptr<SwTOXBase>, sw::RedlineMode, sw::FieldmarkMode>> m_xTOXBase; /// set iff section is TOX
    const std::unique_ptr<SfxItemSet> m_pAttrSet;
    std::unique_ptr<SwHistory> m_pHistory;
    std::unique_ptr<SwRedlineData> m_pRedlData;
    std::unique_ptr<SwRedlineSaveDatas> m_pRedlineSaveData;
    SwNodeOffset m_nSectionNodePos;
    bool m_bSplitAtStart : 1;
    bool m_bSplitAtEnd : 1;
    bool m_bUpdateFootnote : 1;

    void Join( SwDoc& rDoc, SwNodeOffset nNode );

public:
    SwUndoInsSection(SwPaM const&, SwSectionData const&,
        SfxItemSet const* pSet,
        std::tuple<SwTOXBase const*, sw::RedlineMode, sw::FieldmarkMode> const* pTOXBase);

    virtual ~SwUndoInsSection() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetSectNdPos(SwNodeOffset const nPos)     { m_nSectionNodePos = nPos; }
    void SaveSplitNode(SwTextNode *const pTextNd, bool const bAtStart);
    void SetUpdateFootnoteFlag(bool const bFlag)   { m_bUpdateFootnote = bFlag; }
};

std::unique_ptr<SwUndo> MakeUndoDelSection(SwSectionFormat const&);

std::unique_ptr<SwUndo> MakeUndoUpdateSection(SwSectionFormat const&, bool const);


class SwTOXBaseSection;
class SwUndoDelSection;

class SwUndoUpdateIndex final : public SwUndo
{
private:
    std::unique_ptr<SwUndoDelSection> m_pTitleSectionUpdated;
    std::unique_ptr<SwUndoSaveSection, o3tl::default_delete<SwUndoSaveSection>> const m_pSaveSectionOriginal;
    std::unique_ptr<SwUndoSaveSection, o3tl::default_delete<SwUndoSaveSection>> const m_pSaveSectionUpdated;
    SwNodeOffset const m_nStartIndex;

public:
    SwUndoUpdateIndex(SwTOXBaseSection &);
    virtual ~SwUndoUpdateIndex() override;

    void TitleSectionInserted(SwSectionFormat & rSectionFormat);

    virtual void UndoImpl(::sw::UndoRedoContext &) override;
    virtual void RedoImpl(::sw::UndoRedoContext &) override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOSECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
