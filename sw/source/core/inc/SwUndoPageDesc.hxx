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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWUNDOPAGEDESC_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWUNDOPAGEDESC_HXX

#include <undobj.hxx>
#include <pagedesc.hxx>

class SwDoc;

class SwUndoPageDesc final : public SwUndo
{
    SwPageDescExt m_aOld, m_aNew;
    SwDoc& m_rDoc;
    bool m_bExchange;

    // To avoid duplication of (header/footer)content nodes for simple page desc changes
    void ExchangeContentNodes( SwPageDesc& rSource, SwPageDesc &rDest );

    // tdf#153220 use to Exit HeaderFooter EditMode
    void ExitHeaderFooterEdit();
public:
    SwUndoPageDesc(const SwPageDesc & aOld, const SwPageDesc & aNew,
                   SwDoc& rDoc);
    virtual ~SwUndoPageDesc() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

class SwUndoPageDescCreate final : public SwUndo
{
    const SwPageDesc * m_pDesc;
    SwPageDescExt m_aNew;
    SwDoc& m_rDoc;

    void DoImpl();

public:
    SwUndoPageDescCreate(const SwPageDesc * pNew, SwDoc& rDoc);
    virtual ~SwUndoPageDescCreate() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

class SwUndoPageDescDelete final : public SwUndo
{
    SwPageDescExt m_aOld;
    SwDoc& m_rDoc;

    void DoImpl();

public:
    SwUndoPageDescDelete(const SwPageDesc & aOld, SwDoc& rDoc);
    virtual ~SwUndoPageDescDelete() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};
#endif // _SW_UNDO_PAGE_DESC_CHANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
