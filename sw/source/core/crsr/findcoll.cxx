/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
sal_uLong SwCursor::Find( const SwTxtFmtColl& rFmtColl,
                    SwDocPositions nStart, SwDocPositions nEnde, sal_Bool& bCancel,
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

    sal_uLong nRet = FindAll( aSwFindParaFmtColl, nStart, nEnde, eFndRngs, bCancel );
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
