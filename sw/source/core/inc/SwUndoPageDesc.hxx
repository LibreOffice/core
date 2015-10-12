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
class SwPageDesc;

class SwUndoPageDesc : public SwUndo
{
    SwPageDescExt aOld, aNew;
    SwDoc * pDoc;
    bool bExchange;

    // To avoid duplication of (header/footer)content nodes for simple page desc changes
    void ExchangeContentNodes( SwPageDesc& rSource, SwPageDesc &rDest );

public:
    SwUndoPageDesc(const SwPageDesc & aOld, const SwPageDesc & aNew,
                   SwDoc * pDoc);
    virtual ~SwUndoPageDesc();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

class SwUndoPageDescCreate : public SwUndo
{
    const SwPageDesc * pDesc;
    SwPageDescExt aNew;
    SwDoc * pDoc;

    void DoImpl();

public:
    SwUndoPageDescCreate(const SwPageDesc * pNew, SwDoc * pDoc);
    virtual ~SwUndoPageDescCreate();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

class SwUndoPageDescDelete : public SwUndo
{
    SwPageDescExt aOld;
    SwDoc * pDoc;

    void DoImpl();

public:
    SwUndoPageDescDelete(const SwPageDesc & aOld, SwDoc * pDoc);
    virtual ~SwUndoPageDescDelete();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};
#endif // _SW_UNDO_PAGE_DESC_CHANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
