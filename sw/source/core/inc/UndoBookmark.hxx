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



#ifndef SW_UNDO_BOOKMARK_HXX
#define SW_UNDO_BOOKMARK_HXX

#include <undobj.hxx>


class SwHistoryBookmark;

namespace sw {
    namespace mark {
        class IMark;
    }
}


class SwUndoBookmark : public SwUndo
{
    const ::std::auto_ptr<SwHistoryBookmark> m_pHistoryBookmark;

protected:
    SwUndoBookmark( SwUndoId nUndoId, const ::sw::mark::IMark& );

    void SetInDoc( SwDoc* );
    void ResetInDoc( SwDoc* );

public:
    virtual ~SwUndoBookmark();

    // #111827#
    /**
       Returns the rewriter for this undo object.

       The rewriter contains the following rule:

           $1 -> <name of bookmark>

       <name of bookmark> is the name of the bookmark whose
       insertion/deletion is recorded by this undo object.

       @return the rewriter for this undo object
     */
    virtual SwRewriter GetRewriter() const;
};


class SwUndoInsBookmark : public SwUndoBookmark
{
public:
    SwUndoInsBookmark( const ::sw::mark::IMark& );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
};

#endif // SW_UNDO_BOOKMARK_HXX

