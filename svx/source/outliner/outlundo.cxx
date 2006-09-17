/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outlundo.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:31:30 $
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
#include "precompiled_svx.hxx"

#include <outl_pch.hxx>

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
