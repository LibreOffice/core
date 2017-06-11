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

#include <poolfmt.hxx>
#include <charfmt.hxx>
#include <frmfmt.hxx>
#include <SwUndoFmt.hxx>
#include <SwRewriter.hxx>
#include <swundo.hxx>
#include <undobj.hxx>
#include <fmtcol.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <strings.hrc>

SwUndoFormatCreate::SwUndoFormatCreate
(SwUndoId nUndoId, SwFormat * _pNew, SwFormat * _pDerivedFrom, SwDoc * _pDoc)
    : SwUndo(nUndoId, _pDoc), pNew(_pNew),
      pDoc(_pDoc), pNewSet(nullptr), nId(0), bAuto(false)
{
    if (_pDerivedFrom)
        sDerivedFrom = _pDerivedFrom->GetName();
}

SwUndoFormatCreate::~SwUndoFormatCreate()
{
}

void SwUndoFormatCreate::UndoImpl(::sw::UndoRedoContext &)
{
    if (pNew)
    {
        if (sNewName.isEmpty() && pNew)
            sNewName = pNew->GetName();

        if (!sNewName.isEmpty())
            pNew = Find(sNewName);

        if (pNew)
        {
            pNewSet = new SfxItemSet(pNew->GetAttrSet());
            nId = pNew->GetPoolFormatId() & COLL_GET_RANGE_BITS;
            bAuto = pNew->IsAuto();

            Delete();
        }
    }
}

void SwUndoFormatCreate::RedoImpl(::sw::UndoRedoContext &)
{
    SwFormat * pDerivedFrom = Find(sDerivedFrom);
    SwFormat * pFormat = Create(pDerivedFrom);

    if (pFormat && pNewSet)
    {
        pFormat->SetAuto(bAuto);
        pDoc->ChgFormat(*pFormat, *pNewSet);
        pFormat->SetPoolFormatId((pFormat->GetPoolFormatId()
                            & ~COLL_GET_RANGE_BITS)
                           | nId);

        pNew = pFormat;
    }
    else
        pNew = nullptr;
}

SwRewriter SwUndoFormatCreate::GetRewriter() const
{
    if (sNewName.isEmpty() && pNew)
        sNewName = pNew->GetName();

    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, sNewName);

    return aRewriter;
}

SwUndoFormatDelete::SwUndoFormatDelete
(SwUndoId nUndoId, SwFormat * _pOld, SwDoc * _pDoc)
    : SwUndo(nUndoId, _pDoc),
      pDoc(_pDoc), sOldName(_pOld->GetName()),
      aOldSet(_pOld->GetAttrSet())
{
    sDerivedFrom = _pOld->DerivedFrom()->GetName();
    nId = _pOld->GetPoolFormatId() & COLL_GET_RANGE_BITS;
    bAuto = _pOld->IsAuto();
}

SwUndoFormatDelete::~SwUndoFormatDelete()
{
}

void SwUndoFormatDelete::UndoImpl(::sw::UndoRedoContext &)
{
    SwFormat * pDerivedFrom = Find(sDerivedFrom);

    SwFormat * pFormat = Create(pDerivedFrom);

    if (pFormat)
    {
        pDoc->ChgFormat(*pFormat, aOldSet);
        pFormat->SetAuto(bAuto);
        pFormat->SetPoolFormatId((pFormat->GetPoolFormatId() &
                                ~COLL_GET_RANGE_BITS)
                               | nId);
    }
}

void SwUndoFormatDelete::RedoImpl(::sw::UndoRedoContext &)
{
    SwFormat * pOld = Find(sOldName);

    if (pOld)
    {
        Delete(pOld);
    }
}

SwRewriter SwUndoFormatDelete::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, sOldName);

    return aRewriter;
}

SwUndoRenameFormat::SwUndoRenameFormat(SwUndoId nUndoId,
                                 const OUString & _sOldName,
                                 const OUString & _sNewName,
                                 SwDoc * _pDoc)
    : SwUndo(nUndoId, _pDoc), sOldName(_sOldName),
      sNewName(_sNewName), pDoc(_pDoc)
{
}

SwUndoRenameFormat::~SwUndoRenameFormat()
{
}

void SwUndoRenameFormat::UndoImpl(::sw::UndoRedoContext &)
{
    SwFormat * pFormat = Find(sNewName);

    if (pFormat)
    {
        pDoc->RenameFormat(*pFormat, sOldName, true);
    }
}

void SwUndoRenameFormat::RedoImpl(::sw::UndoRedoContext &)
{
    SwFormat *  pFormat = Find(sOldName);

    if (pFormat)
    {
        pDoc->RenameFormat(*pFormat, sNewName, true);
    }
}

SwRewriter SwUndoRenameFormat::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, sOldName);
    aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
    aRewriter.AddRule(UndoArg3, sNewName);

    return aRewriter;
}

SwUndoTextFormatCollCreate::SwUndoTextFormatCollCreate
(SwTextFormatColl * _pNew, SwTextFormatColl * _pDerivedFrom, SwDoc * _pDoc)
    : SwUndoFormatCreate(SwUndoId::TXTFMTCOL_CREATE, _pNew, _pDerivedFrom, _pDoc)
{
}

SwFormat * SwUndoTextFormatCollCreate::Create(SwFormat * pDerivedFrom)
{
    return pDoc->MakeTextFormatColl(sNewName, static_cast<SwTextFormatColl *>(pDerivedFrom), true);
}

void SwUndoTextFormatCollCreate::Delete()
{
    pDoc->DelTextFormatColl(static_cast<SwTextFormatColl *>(pNew), true);
}

SwFormat * SwUndoTextFormatCollCreate::Find(const OUString & rName) const
{
    return pDoc->FindTextFormatCollByName(rName);
}

SwUndoTextFormatCollDelete::SwUndoTextFormatCollDelete(SwTextFormatColl * _pOld,
                                               SwDoc * _pDoc)
    : SwUndoFormatDelete(SwUndoId::TXTFMTCOL_DELETE, _pOld, _pDoc)
{
}

SwFormat * SwUndoTextFormatCollDelete::Create(SwFormat * pDerivedFrom)
{
    return pDoc->MakeTextFormatColl(sOldName, static_cast<SwTextFormatColl *>(pDerivedFrom), true);
}

void SwUndoTextFormatCollDelete::Delete(SwFormat * pOld)
{
    pDoc->DelTextFormatColl(static_cast<SwTextFormatColl *>(pOld), true);
}

SwFormat * SwUndoTextFormatCollDelete::Find(const OUString & rName) const
{
    return pDoc->FindTextFormatCollByName(rName);
}

SwUndoCondTextFormatCollCreate::SwUndoCondTextFormatCollCreate(SwConditionTextFormatColl *_pNew,
    SwTextFormatColl *_pDerivedFrom, SwDoc *_pDoc)
    : SwUndoTextFormatCollCreate(_pNew, _pDerivedFrom, _pDoc)
{
}

SwFormat * SwUndoCondTextFormatCollCreate::Create(SwFormat * pDerivedFrom)
{
    return pDoc->MakeCondTextFormatColl(sNewName, static_cast<SwTextFormatColl *>(pDerivedFrom), true);
}

SwUndoCondTextFormatCollDelete::SwUndoCondTextFormatCollDelete(SwTextFormatColl * _pOld,
                                                       SwDoc * _pDoc)
    : SwUndoTextFormatCollDelete(_pOld, _pDoc)
{
}

SwFormat * SwUndoCondTextFormatCollDelete::Create(SwFormat * pDerivedFrom)
{
    return pDoc->MakeCondTextFormatColl(sOldName, static_cast<SwTextFormatColl *>(pDerivedFrom), true);
}

SwUndoRenameFormatColl::SwUndoRenameFormatColl(const OUString & sInitOldName,
                                         const OUString & sInitNewName,
                                         SwDoc * _pDoc)
    : SwUndoRenameFormat(SwUndoId::TXTFMTCOL_RENAME, sInitOldName, sInitNewName, _pDoc)
{
}

SwFormat * SwUndoRenameFormatColl::Find(const OUString & rName) const
{
    return pDoc->FindTextFormatCollByName(rName);
}

SwUndoCharFormatCreate::SwUndoCharFormatCreate(SwCharFormat * pNewFormat,
                                         SwCharFormat * pDerivedFrom,
                                         SwDoc * pDocument)
    : SwUndoFormatCreate(SwUndoId::CHARFMT_CREATE, pNewFormat, pDerivedFrom, pDocument)
{
}

SwFormat * SwUndoCharFormatCreate::Create(SwFormat * pDerivedFrom)
{
    return pDoc->MakeCharFormat(sNewName, static_cast<SwCharFormat *>(pDerivedFrom), true);
}

void SwUndoCharFormatCreate::Delete()
{
    pDoc->DelCharFormat(static_cast<SwCharFormat *>(pNew), true);
}

SwFormat * SwUndoCharFormatCreate::Find(const OUString & rName) const
{
    return pDoc->FindCharFormatByName(rName);
}

SwUndoCharFormatDelete::SwUndoCharFormatDelete(SwCharFormat * pOld, SwDoc * pDocument)
    : SwUndoFormatDelete(SwUndoId::CHARFMT_DELETE, pOld, pDocument)
{
}

SwFormat * SwUndoCharFormatDelete::Create(SwFormat * pDerivedFrom)
{
    return pDoc->MakeCharFormat(sOldName, static_cast<SwCharFormat *>(pDerivedFrom), true);
}

void SwUndoCharFormatDelete::Delete(SwFormat * pFormat)
{
    pDoc->DelCharFormat(static_cast<SwCharFormat *>(pFormat), true);
}

SwFormat * SwUndoCharFormatDelete::Find(const OUString & rName) const
{
    return pDoc->FindCharFormatByName(rName);
}

SwUndoRenameCharFormat::SwUndoRenameCharFormat(const OUString & sInitOldName,
                                         const OUString & sInitNewName,
                                         SwDoc * pDocument)
    : SwUndoRenameFormat(SwUndoId::CHARFMT_RENAME, sInitOldName, sInitNewName, pDocument)
{
}

SwFormat * SwUndoRenameCharFormat::Find(const OUString & rName) const
{
    return pDoc->FindCharFormatByName(rName);
}

SwUndoFrameFormatCreate::SwUndoFrameFormatCreate(SwFrameFormat * pNewFormat,
                                       SwFrameFormat * pDerivedFrom,
                                       SwDoc * pDocument)
    : SwUndoFormatCreate(SwUndoId::FRMFMT_CREATE, pNewFormat, pDerivedFrom, pDocument)
{
}

SwFormat * SwUndoFrameFormatCreate::Create(SwFormat * pDerivedFrom)
{
    return pDoc->MakeFrameFormat(sNewName, static_cast<SwFrameFormat *>(pDerivedFrom), true, pNew->IsAuto());
}

void SwUndoFrameFormatCreate::Delete()
{
    pDoc->DelFrameFormat(static_cast<SwFrameFormat *>(pNew), true);
}

SwFormat * SwUndoFrameFormatCreate::Find(const OUString & rName) const
{
    return pDoc->FindFrameFormatByName(rName);
}

SwUndoFrameFormatDelete::SwUndoFrameFormatDelete(SwFrameFormat * pOld, SwDoc * pDocument)
    : SwUndoFormatDelete(SwUndoId::FRMFMT_DELETE, pOld, pDocument)
{
}

SwFormat * SwUndoFrameFormatDelete::Create(SwFormat * pDerivedFrom)
{
    return pDoc->MakeFrameFormat(sOldName, static_cast<SwFrameFormat *>(pDerivedFrom), true);
}

void SwUndoFrameFormatDelete::Delete(SwFormat * pFormat)
{
    pDoc->DelFrameFormat(static_cast<SwFrameFormat *>(pFormat), true);
}

SwFormat * SwUndoFrameFormatDelete::Find(const OUString & rName) const
{
    return pDoc->FindFrameFormatByName(rName);
}

SwUndoRenameFrameFormat::SwUndoRenameFrameFormat(const OUString & sInitOldName,
                                       const OUString & sInitNewName,
                                       SwDoc * pDocument)
    : SwUndoRenameFormat(SwUndoId::FRMFMT_RENAME, sInitOldName, sInitNewName, pDocument)
{
}

SwFormat * SwUndoRenameFrameFormat::Find(const OUString & rName) const
{
    return pDoc->FindFrameFormatByName(rName);
}

SwUndoNumruleCreate::SwUndoNumruleCreate(const SwNumRule * _pNew,
                                         SwDoc * _pDoc)
    : SwUndo(SwUndoId::NUMRULE_CREATE, _pDoc), pNew(_pNew), aNew(*_pNew), pDoc(_pDoc),
      bInitialized(false)
{
}

void SwUndoNumruleCreate::UndoImpl(::sw::UndoRedoContext &)
{
    if (! bInitialized)
    {
        aNew = *pNew;
        bInitialized = true;
    }

    pDoc->DelNumRule(aNew.GetName(), true);
}

void SwUndoNumruleCreate::RedoImpl(::sw::UndoRedoContext &)
{
    pDoc->MakeNumRule(aNew.GetName(), &aNew, true);
}

SwRewriter SwUndoNumruleCreate::GetRewriter() const
{
    SwRewriter aResult;

    if (! bInitialized)
    {
        aNew = *pNew;
        bInitialized = true;
    }

    aResult.AddRule(UndoArg1, aNew.GetName());

    return aResult;
}

SwUndoNumruleDelete::SwUndoNumruleDelete(const SwNumRule & rRule,
                                         SwDoc * _pDoc)
    : SwUndo(SwUndoId::NUMRULE_DELETE, _pDoc), aOld(rRule), pDoc(_pDoc)
{
}

void SwUndoNumruleDelete::UndoImpl(::sw::UndoRedoContext &)
{
    pDoc->MakeNumRule(aOld.GetName(), &aOld, true);
}

void SwUndoNumruleDelete::RedoImpl(::sw::UndoRedoContext &)
{
    pDoc->DelNumRule(aOld.GetName(), true);
}

SwRewriter SwUndoNumruleDelete::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UndoArg1, aOld.GetName());

    return aResult;
}

SwUndoNumruleRename::SwUndoNumruleRename(const OUString & _aOldName,
                                         const OUString & _aNewName,
                                         SwDoc * _pDoc)
    : SwUndo(SwUndoId::NUMRULE_RENAME, _pDoc), aOldName(_aOldName), aNewName(_aNewName),
      pDoc(_pDoc)
{
}

void SwUndoNumruleRename::UndoImpl(::sw::UndoRedoContext &)
{
    pDoc->RenameNumRule(aNewName, aOldName, true);
}

void SwUndoNumruleRename::RedoImpl(::sw::UndoRedoContext &)
{
    pDoc->RenameNumRule(aOldName, aNewName, true);
}

SwRewriter SwUndoNumruleRename::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, aOldName);
    aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
    aRewriter.AddRule(UndoArg3, aNewName);

    return aRewriter;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
