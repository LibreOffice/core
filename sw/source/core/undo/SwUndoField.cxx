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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <tools/rtti.hxx>

#include <SwUndoField.hxx>
#include <swundo.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
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
                         const SwField & rOldField,
                         const SwField & rNewField,
                         SwMsgPoolItem * _pHnt, sal_Bool _bUpdate, SwUndoId _nId)
    : SwUndoField(rPos,_nId)
    , pOldField(rOldField.CopyField())
    , pNewField(rNewField.CopyField())
    , pHnt(_pHnt)
    , bUpdate(_bUpdate)
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

void SwUndoFieldFromDoc::UndoImpl(::sw::UndoRedoContext &)
{
    SwTxtFld * pTxtFld = SwDoc::GetTxtFld(GetPosition());
    const SwField * pField = pTxtFld->GetFld().GetFld();

    if (pField)
    {
        pDoc->UpdateFld(pTxtFld, *pOldField, pHnt, bUpdate);
    }
}

void SwUndoFieldFromDoc::DoImpl()
{
    SwTxtFld * pTxtFld = SwDoc::GetTxtFld(GetPosition());
    const SwField * pField = pTxtFld->GetFld().GetFld();

    if (pField)
    {
        pDoc->UpdateFld(pTxtFld, *pNewField, pHnt, bUpdate);
        SwFmtFld* pDstFmtFld = (SwFmtFld*)&pTxtFld->GetFld();

        if ( pDoc->GetFldType(RES_POSTITFLD, aEmptyStr,false) == pDstFmtFld->GetFld()->GetTyp() )
            pDoc->GetDocShell()->Broadcast( SwFmtFldHint( pDstFmtFld, SWFMTFLD_INSERTED ) );
    }
}

void SwUndoFieldFromDoc::RedoImpl(::sw::UndoRedoContext &)
{
    DoImpl();
}

void SwUndoFieldFromDoc::RepeatImpl(::sw::RepeatContext &)
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
    DoImpl();
}

SwUndoFieldFromAPI::SwUndoFieldFromAPI(const SwPosition & rPos,
                                       const Any & rOldVal, const Any & rNewVal,
                                       sal_uInt16 _nWhich)
    : SwUndoField(rPos), aOldVal(rOldVal), aNewVal(rNewVal), nWhich(_nWhich)
{
}

SwUndoFieldFromAPI::~SwUndoFieldFromAPI()
{
}

void SwUndoFieldFromAPI::UndoImpl(::sw::UndoRedoContext &)
{
    SwField * pField = SwDoc::GetField(GetPosition());

    if (pField)
        pField->PutValue(aOldVal, nWhich);
}

void SwUndoFieldFromAPI::DoImpl()
{
    SwField * pField = SwDoc::GetField(GetPosition());

    if (pField)
        pField->PutValue(aNewVal, nWhich);
}

void SwUndoFieldFromAPI::RedoImpl(::sw::UndoRedoContext &)
{
    DoImpl();
}

void SwUndoFieldFromAPI::RepeatImpl(::sw::RepeatContext &)
{
    DoImpl();
}

