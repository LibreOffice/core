/*************************************************************************
 *
 *  $RCSfile: outlundo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:23 $
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

#include <outl_pch.hxx>

#pragma hdrstop

#define _OUTLINER_CXX
#include <outliner.hxx>
#include <outlundo.hxx>


OutlinerUndoBase::OutlinerUndoBase( USHORT nId, Outliner* pOutliner )
    : EditUndo( nId, NULL )
{
    DBG_ASSERT( pOutliner, "Undo: Outliner?!" );
    mpOutliner = pOutliner;
}


OutlinerUndoChangeDepth::OutlinerUndoChangeDepth( Outliner* pOutliner, USHORT nPara, USHORT nOldDepth, USHORT nNewDepth )
    : OutlinerUndoBase( OLUNDO_DEPTH, pOutliner )
{
    mnPara = nPara;
    mnOldDepth = nOldDepth;
    mnNewDepth = nNewDepth;
}

void OutlinerUndoChangeDepth::Undo()
{
    GetOutliner()->ImplInitDepth( mnPara, mnOldDepth, FALSE );
}

void OutlinerUndoChangeDepth::Redo()
{
    GetOutliner()->ImplInitDepth( mnPara, mnNewDepth, FALSE );
}

void OutlinerUndoChangeDepth::Repeat()
{
    DBG_ERROR( "Repeat not implemented!" );
}


OutlinerUndoCheckPara::OutlinerUndoCheckPara( Outliner* pOutliner, USHORT nPara )
    : OutlinerUndoBase( OLUNDO_DEPTH, pOutliner )
{
    mnPara = nPara;
}

void OutlinerUndoCheckPara::Undo()
{
    Paragraph* pPara = GetOutliner()->GetParagraph( mnPara );
    pPara->Invalidate();
    GetOutliner()->ImplCalcBulletText( mnPara, FALSE, FALSE );
}

void OutlinerUndoCheckPara::Redo()
{
    Paragraph* pPara = GetOutliner()->GetParagraph( mnPara );
    pPara->Invalidate();
    GetOutliner()->ImplCalcBulletText( mnPara, FALSE, FALSE );
}

void OutlinerUndoCheckPara::Repeat()
{
    DBG_ERROR( "Repeat not implemented!" );
}


OutlinerUndoMoveParagraphs::OutlinerUndoMoveParagraphs( Outliner* pOutliner, USHORT nStartPara, USHORT nEndPara, short nDiff )
    : OutlinerUndoBase( OLUNDO_MOVEPARAGRAPHS, pOutliner )
{
    mnStartPara = nStartPara;
    mnEndPara = nEndPara;
    mnDiff = nDiff;
}

void OutlinerUndoMoveParagraphs::Undo()
{
    // ...
}

void OutlinerUndoMoveParagraphs::Redo()
{
    // ...
}

void OutlinerUndoMoveParagraphs::Repeat()
{
    DBG_ERROR( "Repeat not implemented!" );
}




DBG_NAME(OLUndoHeight);


OLUndoHeight::OLUndoHeight( Outliner* pOut, USHORT nId )
    : EditUndo( nId, 0 )
{
    DBG_CTOR(OLUndoHeight,0);
    DBG_ASSERT(pOut,"Undo:No Outliner");
    pOutliner = pOut;
    ppBulletTexts = 0;
    pDepths = 0;
    nAbsCount = 0;
}


OLUndoHeight::~OLUndoHeight()
{
    DBG_DTOR(OLUndoHeight,0);
    Outliner::ImpDeleteBulletArray( ppBulletTexts, nAbsCount );
    delete pDepths;
}


void OLUndoHeight::Restore( BOOL bUndo )
{
    DBG_CHKTHIS(OLUndoHeight,0);
    DBG_ASSERT(pOutliner,"Undo:No Outliner");
    DBG_ASSERT(pOutliner->pEditEngine,"Outliner already deleted");

    // Bullets restaurieren & Undo->Redo bzw. Redo->Undo vorbereiten
    XubString** ppNewBulletArray = pOutliner->ImpCreateBulletArray();
    USHORT* pNewDepths = pOutliner->ImpCreateDepthArray();

    pOutliner->ImpSetBulletArray( ppBulletTexts );
    Outliner::ImpDeleteBulletArray( ppBulletTexts, nAbsCount );
    ppBulletTexts = ppNewBulletArray;

    pOutliner->ImpSetDepthArray( pDepths );
    delete pDepths;
    pDepths = pNewDepths;
}



void OLUndoHeight::Undo()
{
    DBG_CHKTHIS(OLUndoHeight,0);
    Restore( TRUE );
}


void OLUndoHeight::Redo()
{
    DBG_CHKTHIS(OLUndoHeight,0);
    Restore( FALSE );
}


void OLUndoHeight::Repeat()
{
    DBG_CHKTHIS(OLUndoHeight,0);
    DBG_ERROR("Not implemented");
}

DBG_NAME(OLUndoExpand);


OLUndoExpand::OLUndoExpand(Outliner* pOut, USHORT nId )
    : EditUndo( nId, 0 )
{
    DBG_CTOR(OLUndoExpand,0);
    DBG_ASSERT(pOut,"Undo:No Outliner");
    pOutliner = pOut;
    nCount = 0;
    pParas = 0;
}


OLUndoExpand::~OLUndoExpand()
{
    DBG_DTOR(OLUndoExpand,0);
    delete pParas;
}


void OLUndoExpand::Restore( BOOL bUndo )
{
    DBG_CHKTHIS(OLUndoExpand,0);
    DBG_ASSERT(pOutliner,"Undo:No Outliner");
    DBG_ASSERT(pOutliner->pEditEngine,"Outliner already deleted");
    Paragraph* pPara;

    BOOL bExpand = FALSE;
    USHORT nId = GetId();
    if((nId == OLUNDO_EXPAND && !bUndo) || (nId == OLUNDO_COLLAPSE && bUndo))
        bExpand = TRUE;
    if( !pParas )
    {
        pPara = pOutliner->GetParagraph( (ULONG)nCount );
        if( bExpand )
            pOutliner->Expand( pPara );
        else
            pOutliner->Collapse( pPara );
    }
    else
    {
        for( USHORT nIdx = 0; nIdx < nCount; nIdx++ )
        {
            pPara = pOutliner->GetParagraph( (ULONG)(pParas[nIdx]) );
            if( bExpand )
                pOutliner->Expand( pPara );
            else
                pOutliner->Collapse( pPara );
        }
    }
}


void OLUndoExpand::Undo()
{
    DBG_CHKTHIS(OLUndoExpand,0);
    Restore( TRUE );
}


void OLUndoExpand::Redo()
{
    DBG_CHKTHIS(OLUndoExpand,0);
    Restore( FALSE );
}


void OLUndoExpand::Repeat()
{
    DBG_CHKTHIS(OLUndoExpand,0);
    DBG_ERROR("Not implemented");
}
