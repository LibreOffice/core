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
#include <pamtyp.hxx>
#include <swundo.hxx>
#include <SwRewriter.hxx>
#include <comcore.hrc>

/// parameters for a search for FormatCollections
struct SwFindParaFmtColl : public SwFindParas
{
    const SwTxtFmtColl *pFmtColl, *pReplColl;
    SwCursor& rCursor;
    SwFindParaFmtColl( const SwTxtFmtColl& rFmtColl,
                        const SwTxtFmtColl* pRpColl, SwCursor& rCrsr )
        : pFmtColl( &rFmtColl ), pReplColl( pRpColl ), rCursor( rCrsr )
    {}
    virtual ~SwFindParaFmtColl() {}
    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, sal_Bool bInReadOnly );
    virtual int IsReplaceMode() const;
};

int SwFindParaFmtColl::Find( SwPaM* pCrsr, SwMoveFn fnMove, const SwPaM* pRegion,
                            sal_Bool bInReadOnly )
{
    int nRet = FIND_FOUND;
    if( bInReadOnly && pReplColl )
        bInReadOnly = sal_False;

    if( !pCrsr->Find( *pFmtColl, fnMove, pRegion, bInReadOnly ) )
        nRet = FIND_NOT_FOUND;
    else if( pReplColl )
    {
        pCrsr->GetDoc()->SetTxtFmtColl( *pCrsr, (SwTxtFmtColl*)pReplColl );
        nRet = FIND_NO_RING;
    }
    return nRet;
}

int SwFindParaFmtColl::IsReplaceMode() const
{
    return 0 != pReplColl;
}

/// search for Format-Collections
sal_uLong SwCursor::Find( const SwTxtFmtColl& rFmtColl, SwDocPositions nStart,
                          SwDocPositions nEnd, sal_Bool& bCancel,
                          FindRanges eFndRngs, const SwTxtFmtColl* pReplFmtColl )
{
    // switch off OLE-notifications
    SwDoc* pDoc = GetDoc();
    Link aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    bool const bStartUndo =
        pDoc->GetIDocumentUndoRedo().DoesUndo() && pReplFmtColl;
    if (bStartUndo)
    {
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, rFmtColl.GetName());
        aRewriter.AddRule(UndoArg2, SW_RES(STR_YIELDS));
        aRewriter.AddRule(UndoArg3, pReplFmtColl->GetName());

        pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_UI_REPLACE_STYLE,
                &aRewriter );
    }

    SwFindParaFmtColl aSwFindParaFmtColl( rFmtColl, pReplFmtColl, *this );

    sal_uLong nRet = FindAll( aSwFindParaFmtColl, nStart, nEnd, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );

    if( nRet && pReplFmtColl )
        pDoc->SetModified();

    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_END, 0);
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
