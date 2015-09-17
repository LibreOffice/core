/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_DOCFUNCUTIL_HXX
#define INCLUDED_SC_DOCFUNCUTIL_HXX

#include <undobase.hxx>

#include <boost/shared_ptr.hpp>

class ScDocument;
class ScMarkData;
class ScRange;
struct InsertDeleteFlags;

namespace sc {

class DocFuncUtil
{
public:

    static bool hasProtectedTab( const ScDocument& rDoc, const ScMarkData& rMark );

    static ScDocument* createDeleteContentsUndoDoc(
        ScDocument& rDoc, const ScMarkData& rMark, const ScRange& rRange,
        InsertDeleteFlags nFlags, bool bOnlyMarked );

    static void addDeleteContentsUndo(
        svl::IUndoManager* pUndoMgr, ScDocShell* pDocSh, const ScMarkData& rMark,
        const ScRange& rRange, ScDocument* pUndoDoc, InsertDeleteFlags nFlags,
        const boost::shared_ptr<ScSimpleUndo::DataSpansType>& pSpans,
        bool bMulti, bool bDrawUndo );

    static ScSimpleUndo::DataSpansType* getNonEmptyCellSpans(
        const ScDocument& rDoc, const ScMarkData& rMark, const ScRange& rRange );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
