/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SW_UNDO_PAGE_DESC_HXX
#define SW_UNDO_PAGE_DESC_HXX

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

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    virtual SwRewriter GetRewriter() const;
};

class SwUndoPageDescCreate : public SwUndo
{
    const SwPageDesc * pDesc; // #116530#
    SwPageDescExt aNew;
    SwDoc * pDoc;

    void DoImpl();

public:
    SwUndoPageDescCreate(const SwPageDesc * pNew, SwDoc * pDoc); // #116530#
    virtual ~SwUndoPageDescCreate();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    virtual SwRewriter GetRewriter() const;
};

class SwUndoPageDescDelete : public SwUndo
{
    SwPageDescExt aOld;
    SwDoc * pDoc;

    void DoImpl();

public:
    SwUndoPageDescDelete(const SwPageDesc & aOld, SwDoc * pDoc);
    virtual ~SwUndoPageDescDelete();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    virtual SwRewriter GetRewriter() const;
};
#endif // _SW_UNDO_PAGE_DESC_CHANGE_HXX
