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

#include <tools/resid.hxx>
#include <swcrsr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <pamtyp.hxx>
#include <swundo.hxx>
#include <SwRewriter.hxx>
#include <comcore.hrc>

/// parameters for a search for FormatCollections
struct SwFindParaFormatColl : public SwFindParas
{
    const SwTextFormatColl *pFormatColl, *pReplColl;
    SwFindParaFormatColl(const SwTextFormatColl& rFormatColl, const SwTextFormatColl* pRpColl)
        : pFormatColl( &rFormatColl ), pReplColl( pRpColl )
    {}
    virtual ~SwFindParaFormatColl() {}
    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, bool bInReadOnly ) override;
    virtual bool IsReplaceMode() const override;
};

int SwFindParaFormatColl::Find( SwPaM* pCrsr, SwMoveFn fnMove, const SwPaM* pRegion,
                             bool bInReadOnly )
{
    int nRet = FIND_FOUND;
    if( bInReadOnly && pReplColl )
        bInReadOnly = false;

    if( !pCrsr->Find( *pFormatColl, fnMove, pRegion, bInReadOnly ) )
        nRet = FIND_NOT_FOUND;
    else if( pReplColl )
    {
        pCrsr->GetDoc()->SetTextFormatColl( *pCrsr, const_cast<SwTextFormatColl*>(pReplColl) );
        nRet = FIND_NO_RING;
    }
    return nRet;
}

bool SwFindParaFormatColl::IsReplaceMode() const
{
    return nullptr != pReplColl;
}

/// search for Format-Collections
sal_uLong SwCursor::Find( const SwTextFormatColl& rFormatColl, SwDocPositions nStart,
                          SwDocPositions nEnd, bool& bCancel,
                          FindRanges eFndRngs, const SwTextFormatColl* pReplFormatColl )
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
        aRewriter.AddRule(UndoArg2, SW_RES(STR_YIELDS));
        aRewriter.AddRule(UndoArg3, pReplFormatColl->GetName());

        pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_UI_REPLACE_STYLE,
                &aRewriter );
    }

    SwFindParaFormatColl aSwFindParaFormatColl(rFormatColl, pReplFormatColl);

    sal_uLong nRet = FindAll( aSwFindParaFormatColl, nStart, nEnd, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );

    if( nRet && pReplFormatColl )
        pDoc->getIDocumentState().SetModified();

    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_END, nullptr);
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
