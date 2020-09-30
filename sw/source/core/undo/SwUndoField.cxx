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


#include <SwUndoField.hxx>
#include <swundo.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentFieldsManager.hxx>
#include <txtfld.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <docsh.hxx>
#include <pam.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;

SwUndoField::SwUndoField(const SwPosition & rPos )
    : SwUndo(SwUndoId::FIELD, &rPos.GetDoc())
{
    m_nNodeIndex = rPos.nNode.GetIndex();
    m_nOffset = rPos.nContent.GetIndex();
    m_pDoc = &rPos.GetDoc();
}

SwUndoField::~SwUndoField()
{
}

SwPosition SwUndoField::GetPosition()
{
    SwNode * pNode = m_pDoc->GetNodes()[m_nNodeIndex];
    SwNodeIndex aNodeIndex(*pNode);
    SwIndex aIndex(pNode->GetContentNode(), m_nOffset);
    SwPosition aResult(aNodeIndex, aIndex);

    return aResult;
}

SwUndoFieldFromDoc::SwUndoFieldFromDoc(const SwPosition & rPos,
                         const SwField & rOldField,
                         const SwField & rNewField,
                         SwMsgPoolItem * _pHint, bool _bUpdate)
    : SwUndoField(rPos)
    , m_pOldField(rOldField.CopyField())
    , m_pNewField(rNewField.CopyField())
    , m_pHint(_pHint)
    , m_bUpdate(_bUpdate)
{
    OSL_ENSURE(m_pOldField, "No old field!");
    OSL_ENSURE(m_pNewField, "No new field!");
    OSL_ENSURE(m_pDoc, "No document!");
}

SwUndoFieldFromDoc::~SwUndoFieldFromDoc()
{
}

void SwUndoFieldFromDoc::UndoImpl(::sw::UndoRedoContext &)
{
    SwTextField * pTextField = sw::DocumentFieldsManager::GetTextFieldAtPos(GetPosition());
    const SwField * pField = pTextField ? pTextField->GetFormatField().GetField() : nullptr;

    if (pField)
    {
        m_pDoc->getIDocumentFieldsAccess().UpdateField(pTextField, *m_pOldField, m_pHint, m_bUpdate);
    }
}

void SwUndoFieldFromDoc::DoImpl()
{
    SwTextField * pTextField = sw::DocumentFieldsManager::GetTextFieldAtPos(GetPosition());
    const SwField * pField = pTextField ? pTextField->GetFormatField().GetField() : nullptr;

    if (pField)
    {
        m_pDoc->getIDocumentFieldsAccess().UpdateField(pTextField, *m_pNewField, m_pHint, m_bUpdate);
        SwFormatField* pDstFormatField = const_cast<SwFormatField*>(&pTextField->GetFormatField());

        if (m_pDoc->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Postit, OUString(), false) == pDstFormatField->GetField()->GetTyp())
            m_pDoc->GetDocShell()->Broadcast( SwFormatFieldHint( pDstFormatField, SwFormatFieldHintWhich::INSERTED ) );
    }
}

void SwUndoFieldFromDoc::RedoImpl(::sw::UndoRedoContext &)
{
    DoImpl();
}

void SwUndoFieldFromDoc::RepeatImpl(::sw::RepeatContext &)
{
    ::sw::UndoGuard const undoGuard(m_pDoc->GetIDocumentUndoRedo());
    DoImpl();
}

SwUndoFieldFromAPI::SwUndoFieldFromAPI(const SwPosition & rPos,
                                       const Any & rOldVal, const Any & rNewVal,
                                       sal_uInt16 _nWhich)
    : SwUndoField(rPos), m_aOldVal(rOldVal), m_aNewVal(rNewVal), m_nWhich(_nWhich)
{
}

SwUndoFieldFromAPI::~SwUndoFieldFromAPI()
{
}

void SwUndoFieldFromAPI::UndoImpl(::sw::UndoRedoContext &)
{
    SwField * pField = sw::DocumentFieldsManager::GetFieldAtPos(GetPosition());

    if (pField)
        pField->PutValue(m_aOldVal, m_nWhich);
}

void SwUndoFieldFromAPI::DoImpl()
{
    SwField * pField = sw::DocumentFieldsManager::GetFieldAtPos(GetPosition());

    if (pField)
        pField->PutValue(m_aNewVal, m_nWhich);
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
