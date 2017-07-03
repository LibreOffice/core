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

struct SwSortOptions;
class SwRangeRedline;
class SwRedlineSaveDatas;
class SwUndoDelete;

class SwUndoRedline : public SwUndo, public SwUndRng
{
protected:
    SwRedlineData* mpRedlData;
    SwRedlineSaveDatas* mpRedlSaveData;
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

class SwUndoRedlineDelete : public SwUndoRedline
{
    bool bCanGroup : 1;
    bool bIsDelim : 1;
    bool bIsBackspace : 1;

    OUString m_sRedlineText;

    virtual void UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;

public:
    SwUndoRedlineDelete( const SwPaM& rRange, SwUndoId nUserId );
    virtual SwRewriter GetRewriter() const override;

    bool CanGrouping( const SwUndoRedlineDelete& rPrev );

    // SwUndoTableCpyTable needs this information:
    long NodeDiff() const { return nSttNode - nEndNode; }
    sal_Int32 ContentStart() const { return nSttContent; }

    void SetRedlineText(const OUString & rText);
};

class SwUndoRedlineSort : public SwUndoRedline
{
    std::unique_ptr<SwSortOptions> pOpt;
    sal_uLong nSaveEndNode, nOffset;
    sal_Int32 nSaveEndContent;

    virtual void UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;

public:
    SwUndoRedlineSort( const SwPaM& rRange, const SwSortOptions& rOpt );

    virtual ~SwUndoRedlineSort() override;

    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetSaveRange( const SwPaM& rRange );
    void SetOffset( const SwNodeIndex& rIdx );
};

class SwUndoAcceptRedline : public SwUndoRedline
{
private:
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;

public:
    SwUndoAcceptRedline( const SwPaM& rRange );

    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

class SwUndoRejectRedline : public SwUndoRedline
{
private:
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam) override;

public:
    SwUndoRejectRedline( const SwPaM& rRange );

    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

class SwUndoCompDoc : public SwUndo, public SwUndRng
{
    SwRedlineData* pRedlData;
    SwUndoDelete* pUnDel, *pUnDel2;
    SwRedlineSaveDatas* pRedlSaveData;
    bool bInsert;

public:
    SwUndoCompDoc( const SwPaM& rRg, bool bIns );
    SwUndoCompDoc( const SwRangeRedline& rRedl );

    virtual ~SwUndoCompDoc() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOREDLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
