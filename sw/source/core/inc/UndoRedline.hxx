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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOREDLINE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOREDLINE_HXX

#include <memory>
#include <undobj.hxx>
#include <tools/long.hxx>

struct SwSortOptions;
class SwRangeRedline;
class SwRedlineSaveDatas;
class SwUndoDelete;

class SwUndoRedline : public SwUndo, public SwUndRng
{
protected:
    std::unique_ptr<SwRedlineData> mpRedlData;
    std::unique_ptr<SwRedlineSaveDatas> mpRedlSaveData;
    SwUndoId mnUserId;
    bool mbHiddenRedlines;

    virtual void UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam);
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam);

public:
    SwUndoRedline( SwUndoId nUserId, const SwPaM& rRange );

    virtual ~SwUndoRedline() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    sal_uInt16 GetRedlSaveCount() const;
};

class SwUndoRedlineDelete final : public SwUndoRedline
{
    bool m_bCanGroup : 1;
    bool m_bIsDelim : 1;
    bool m_bIsBackspace : 1;

    OUString m_sRedlineText;

    virtual void UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;

public:
    SwUndoRedlineDelete( const SwPaM& rRange, SwUndoId nUserId );
    virtual SwRewriter GetRewriter() const override;

    bool CanGrouping( const SwUndoRedlineDelete& rPrev );

    // SwUndoTableCpyTable needs this information:
    SwNodeOffset NodeDiff() const { return m_nSttNode - m_nEndNode; }
    sal_Int32 ContentStart() const { return m_nSttContent; }

    void SetRedlineText(const OUString & rText);
};

class SwUndoRedlineSort final : public SwUndoRedline
{
    std::unique_ptr<SwSortOptions> m_pOpt;
    SwNodeOffset m_nSaveEndNode;
    sal_Int32 m_nSaveEndContent;

    virtual void UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;

public:
    SwUndoRedlineSort( const SwPaM& rRange, const SwSortOptions& rOpt );

    virtual ~SwUndoRedlineSort() override;

    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetSaveRange( const SwPaM& rRange );
};

class SwUndoAcceptRedline final : public SwUndoRedline
{
private:
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;

public:
    SwUndoAcceptRedline( const SwPaM& rRange );

    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

class SwUndoRejectRedline final : public SwUndoRedline
{
private:
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;

public:
    SwUndoRejectRedline( const SwPaM& rRange );

    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

class SwUndoCompDoc final : public SwUndo, public SwUndRng
{
    std::unique_ptr<SwRedlineData> m_pRedlineData;
    std::unique_ptr<SwUndoDelete> m_pUndoDelete, m_pUndoDelete2;
    std::unique_ptr<SwRedlineSaveDatas> m_pRedlineSaveDatas;
    bool m_bInsert;

public:
    SwUndoCompDoc( const SwPaM& rRg, bool bIns );
    SwUndoCompDoc( const SwRangeRedline& rRedl );

    virtual ~SwUndoCompDoc() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOREDLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
