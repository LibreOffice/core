/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: findcoll.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:29:33 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif

//------------------ Methoden der CrsrShell ---------------------------

// Parameter fuer das Suchen vom FormatCollections
struct SwFindParaFmtColl : public SwFindParas
{
    const SwTxtFmtColl *pFmtColl, *pReplColl;
    SwCursor& rCursor;
    SwFindParaFmtColl( const SwTxtFmtColl& rFmtColl,
                        const SwTxtFmtColl* pRpColl, SwCursor& rCrsr )
        : pFmtColl( &rFmtColl ), pReplColl( pRpColl ), rCursor( rCrsr )
    {}
    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, BOOL bInReadOnly );
    virtual int IsReplaceMode() const;
};


int SwFindParaFmtColl::Find( SwPaM* pCrsr, SwMoveFn fnMove, const SwPaM* pRegion,
                            BOOL bInReadOnly )
{
    int nRet = FIND_FOUND;
    if( bInReadOnly && pReplColl )
        bInReadOnly = FALSE;

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


// Suchen nach Format-Collections


ULONG SwCursor::Find( const SwTxtFmtColl& rFmtColl,
                    SwDocPositions nStart, SwDocPositions nEnde, BOOL& bCancel,
                    FindRanges eFndRngs, const SwTxtFmtColl* pReplFmtColl )
{
    // OLE-Benachrichtigung abschalten !!
    SwDoc* pDoc = GetDoc();
    Link aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    BOOL bSttUndo = pDoc->DoesUndo() && pReplFmtColl;
    if( bSttUndo )
    {
        SwRewriter aRewriter;
        aRewriter.AddRule(UNDO_ARG1, rFmtColl.GetName());
        aRewriter.AddRule(UNDO_ARG2, SW_RES(STR_YIELDS));
        aRewriter.AddRule(UNDO_ARG3, pReplFmtColl->GetName());

        pDoc->StartUndo( UNDO_UI_REPLACE_STYLE, &aRewriter );
    }

    SwFindParaFmtColl aSwFindParaFmtColl( rFmtColl, pReplFmtColl, *this );

    ULONG nRet = FindAll( aSwFindParaFmtColl, nStart, nEnde, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );

    if( nRet && pReplFmtColl )
        pDoc->SetModified();

    if( bSttUndo )
        pDoc->EndUndo( UNDO_UI_REPLACE_STYLE, NULL );
    return nRet;
}



