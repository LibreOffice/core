/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwUndoField.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:52:32 $
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
#include <tools/rtti.hxx>
#include <SwUndoField.hxx>
#include <swundo.hxx>
#include <doc.hxx>
#include <txtfld.hxx>
#include <fldbas.hxx>
#include <ndtxt.hxx>
#include <fmtfld.hxx>
#include <dbfld.hxx>
#include <docsh.hxx>

using namespace ::com::sun::star::uno;

SwUndoField::SwUndoField(const SwPosition & rPos, SwUndoId _nId )
    : SwUndo(_nId)
{
    nNodeIndex = rPos.nNode.GetIndex();
    nOffset = rPos.nContent.GetIndex();
    pDoc = rPos.GetDoc();
}

SwUndoField::~SwUndoField()
{
}

SwPosition SwUndoField::GetPosition()
{
    SwNode * pNode = pDoc->GetNodes()[nNodeIndex];
    SwNodeIndex aNodeIndex(*pNode);
    SwIndex aIndex(pNode->GetCntntNode(), nOffset);
    SwPosition aResult(aNodeIndex, aIndex);

    return aResult;
}

SwUndoFieldFromDoc::SwUndoFieldFromDoc(const SwPosition & rPos,
                                       const SwField & _aOldField,
                         const SwField & _aNewField,
                         SwMsgPoolItem * _pHnt, BOOL _bUpdate, SwUndoId _nId)
    : SwUndoField(rPos,_nId), pOldField(_aOldField.Copy()),
      pNewField(_aNewField.Copy()), pHnt(_pHnt),
      bUpdate(_bUpdate)
{
    ASSERT(pOldField, "No old field!");
    ASSERT(pNewField, "No new field!");
    ASSERT(pDoc, "No document!");
}

SwUndoFieldFromDoc::~SwUndoFieldFromDoc()
{
    delete pOldField;
    delete pNewField;
}

void SwUndoFieldFromDoc::Undo( SwUndoIter& )
{
    SwTxtFld * pTxtFld = SwDoc::GetTxtFld(GetPosition());
    const SwField * pField = pTxtFld->GetFld().GetFld();

    if (pField)
    {
        BOOL bUndo = pDoc->DoesUndo();

        pDoc->DoUndo(FALSE);
        pDoc->UpdateFld(pTxtFld, *pOldField, pHnt, bUpdate);
        pDoc->DoUndo(bUndo);
    }
}

void SwUndoFieldFromDoc::Redo( SwUndoIter& )
{
    SwTxtFld * pTxtFld = SwDoc::GetTxtFld(GetPosition());
    const SwField * pField = pTxtFld->GetFld().GetFld();

    if (pField)
    {
        BOOL bUndo = pDoc->DoesUndo();

        pDoc->DoUndo(FALSE);
        pDoc->UpdateFld(pTxtFld, *pNewField, pHnt, bUpdate);
        SwFmtFld* pDstFmtFld = (SwFmtFld*)&pTxtFld->GetFld();

        if ( pDoc->GetFldType(RES_POSTITFLD, aEmptyStr,false) == pDstFmtFld->GetFld()->GetTyp() )
            pDoc->GetDocShell()->Broadcast( SwFmtFldHint( pDstFmtFld, SWFMTFLD_INSERTED ) );
        pDoc->DoUndo(bUndo);
    }
}

void SwUndoFieldFromDoc::Repeat(SwUndoIter & rIt)
{
    Redo(rIt);
}

SwUndoFieldFromAPI::SwUndoFieldFromAPI(const SwPosition & rPos,
                                       const Any & rOldVal, const Any & rNewVal,
                                       USHORT _nWhich)
    : SwUndoField(rPos), aOldVal(rOldVal), aNewVal(rNewVal), nWhich(_nWhich)
{
}

SwUndoFieldFromAPI::~SwUndoFieldFromAPI()
{
}

void SwUndoFieldFromAPI::Undo( SwUndoIter& )
{
    SwField * pField = SwDoc::GetField(GetPosition());

    if (pField)
        pField->PutValue(aOldVal, nWhich);
}

void SwUndoFieldFromAPI::Redo( SwUndoIter& )
{
    SwField * pField = SwDoc::GetField(GetPosition());

    if (pField)
        pField->PutValue(aNewVal, nWhich);
}


void SwUndoFieldFromAPI::Repeat(SwUndoIter & rIter)
{
    Redo(rIter);
}
