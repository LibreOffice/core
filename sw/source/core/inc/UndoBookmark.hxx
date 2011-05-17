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

