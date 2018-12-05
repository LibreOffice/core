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

#include <swcrsr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <pamtyp.hxx>
#include <swundo.hxx>
#include <SwRewriter.hxx>
#include <strings.hrc>

/// parameters for a search for FormatCollections
struct SwFindParaFormatColl : public SwFindParas
{
    const SwTextFormatColl *pFormatColl, *pReplColl;
    SwRootFrame const* m_pLayout;
    SwFindParaFormatColl(const SwTextFormatColl& rFormatColl,
            const SwTextFormatColl *const pRpColl,
            SwRootFrame const*const pLayout)
        : pFormatColl( &rFormatColl )
        , pReplColl( pRpColl )
        , m_pLayout(pLayout)
    {}
    virtual ~SwFindParaFormatColl() {}
    virtual int DoFind(SwPaM &, SwMoveFnCollection const &, const SwPaM &, bool bInReadOnly) override;
    virtual bool IsReplaceMode() const override;
};

int SwFindParaFormatColl::DoFind(SwPaM & rCursor, SwMoveFnCollection const & fnMove,
        const SwPaM & rRegion, bool bInReadOnly)
{
    int nRet = FIND_FOUND;
    if( bInReadOnly && pReplColl )
        bInReadOnly = false;

    if (!sw::FindFormatImpl(rCursor, *pFormatColl, fnMove, rRegion, bInReadOnly, m_pLayout))
        nRet = FIND_NOT_FOUND;
    else if( pReplColl )
    {
        rCursor.GetDoc()->SetTextFormatColl(rCursor,
            const_cast<SwTextFormatColl*>(pReplColl), true, false, m_pLayout);
        nRet = FIND_NO_RING;
    }
    return nRet;
}

bool SwFindParaFormatColl::IsReplaceMode() const
{
    return nullptr != pReplColl;
}

/// search for Format-Collections
sal_uLong SwCursor::FindFormat( const SwTextFormatColl& rFormatColl, SwDocPositions nStart,
                          SwDocPositions nEnd, bool& bCancel,
                          FindRanges eFndRngs, const SwTextFormatColl* pReplFormatColl,
                          SwRootFrame const*const pLayout)
{
    // switch off OLE-notifications
    SwDoc* pDoc = GetDoc();
    Link<bool,void> aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link<bool,void>() );

    bool const bStartUndo =
        pDoc->GetIDocumentUndoRedo().DoesUndo() && pReplFormatColl;
    if (bStartUndo)
    {
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, rFormatColl.GetName());
        aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
        aRewriter.AddRule(UndoArg3, pReplFormatColl->GetName());

        pDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::UI_REPLACE_STYLE,
                &aRewriter );
    }

    SwFindParaFormatColl aSwFindParaFormatColl(rFormatColl, pReplFormatColl, pLayout);

    sal_uLong nRet = FindAll( aSwFindParaFormatColl, nStart, nEnd, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );

    if( nRet && pReplFormatColl )
        pDoc->getIDocumentState().SetModified();

    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::END, nullptr);
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
