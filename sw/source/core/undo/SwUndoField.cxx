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
    OSL_ENSURE(pOldField, "No old field!");
    OSL_ENSURE(pNewField, "No new field!");
    OSL_ENSURE(pDoc, "No document!");
}

SwUndoFieldFromDoc::~SwUndoFieldFromDoc()
{
    delete pOldField;
    delete pNewField;
}

void SwUndoFieldFromDoc::UndoImpl(::sw::UndoRedoContext &)
{
    SwTxtFld * pTxtFld = SwDoc::GetTxtFldAtPos(GetPosition());
    const SwField * pField = pTxtFld ? pTxtFld->GetFmtFld().GetField() : NULL;

    if (pField)
    {
        pDoc->UpdateFld(pTxtFld, *pOldField, pHnt, bUpdate);
    }
}

void SwUndoFieldFromDoc::DoImpl()
{
    SwTxtFld * pTxtFld = SwDoc::GetTxtFldAtPos(GetPosition());
    const SwField * pField = pTxtFld ? pTxtFld->GetFmtFld().GetField() : NULL;

    if (pField)
    {
        pDoc->UpdateFld(pTxtFld, *pNewField, pHnt, bUpdate);
        SwFmtFld* pDstFmtFld = (SwFmtFld*)&pTxtFld->GetFmtFld();

        if ( pDoc->GetFldType(RES_POSTITFLD, aEmptyOUStr, false) == pDstFmtFld->GetField()->GetTyp() )
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
    SwField * pField = SwDoc::GetFieldAtPos(GetPosition());

    if (pField)
        pField->PutValue(aOldVal, nWhich);
}

void SwUndoFieldFromAPI::DoImpl()
{
    SwField * pField = SwDoc::GetFieldAtPos(GetPosition());

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
