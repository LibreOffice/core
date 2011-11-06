/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
