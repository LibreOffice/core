/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwUndoPageDesc.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 07:55:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SW_UNDO_PAGE_DESC_HXX
#define _SW_UNDO_PAGE_DESC_HXX

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

    virtual void Undo(SwUndoIter & rIt);
    virtual void Redo(SwUndoIter & rIt);
    virtual void Repeat(SwUndoIter & rIt);

    virtual SwRewriter GetRewriter() const;
};

class SwUndoPageDescCreate : public SwUndo
{
    const SwPageDesc * pDesc; // #116530#
    SwPageDescExt aNew;
    SwDoc * pDoc;

public:
    SwUndoPageDescCreate(const SwPageDesc * pNew, SwDoc * pDoc); // #116530#
    virtual ~SwUndoPageDescCreate();

    virtual void Undo(SwUndoIter & rIt);
    virtual void Redo(SwUndoIter & rIt);
    virtual void Repeat(SwUndoIter & rIt);

    virtual SwRewriter GetRewriter() const;
};

class SwUndoPageDescDelete : public SwUndo
{
    SwPageDescExt aOld;
    SwDoc * pDoc;

public:
    SwUndoPageDescDelete(const SwPageDesc & aOld, SwDoc * pDoc);
    virtual ~SwUndoPageDescDelete();

    virtual void Undo(SwUndoIter & rIt);
    virtual void Redo(SwUndoIter & rIt);
    virtual void Repeat(SwUndoIter & rIt);

    virtual SwRewriter GetRewriter() const;
};
#endif // _SW_UNDO_PAGE_DESC_CHANGE_HXX
