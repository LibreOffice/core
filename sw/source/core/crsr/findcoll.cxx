/*************************************************************************
 *
 *  $RCSfile: findcoll.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

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


//------------------ Methoden der CrsrShell ---------------------------

// Parameter fuer das Suchen vom FormatCollections
struct SwFindParaFmtColl : public SwFindParas
{
    const SwTxtFmtColl *pFmtColl, *pReplColl;
    SwCursor& rCursor;
    SwFindParaFmtColl( const SwTxtFmtColl& rFmtColl,
                        const SwTxtFmtColl* pRpColl, SwCursor& rCrsr )
        : rCursor( rCrsr ), pFmtColl( &rFmtColl ), pReplColl( pRpColl )
    {}
    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, FASTBOOL bInReadOnly );
    virtual int IsReplaceMode() const;
};


int SwFindParaFmtColl::Find( SwPaM* pCrsr, SwMoveFn fnMove, const SwPaM* pRegion,
                            FASTBOOL bInReadOnly )
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
                    SwDocPositions nStart, SwDocPositions nEnde,
                    FindRanges eFndRngs, const SwTxtFmtColl* pReplFmtColl )
{
    // OLE-Benachrichtigung abschalten !!
    SwDoc* pDoc = GetDoc();
    Link aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    BOOL bSttUndo = pDoc->DoesUndo() && pReplFmtColl;
    if( bSttUndo )
        pDoc->StartUndo( UNDO_REPLACE );

    SwFindParaFmtColl aSwFindParaFmtColl( rFmtColl, pReplFmtColl, *this );

    ULONG nRet = FindAll( aSwFindParaFmtColl, nStart, nEnde, eFndRngs );
    pDoc->SetOle2Link( aLnk );

    if( nRet && pReplFmtColl )
        pDoc->SetModified();

    if( bSttUndo )
        pDoc->EndUndo( UNDO_REPLACE );

    return nRet;
}



