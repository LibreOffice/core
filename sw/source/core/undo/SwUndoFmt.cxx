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
#include <comcore.hrc>

SwUndoFmtCreate::SwUndoFmtCreate
(SwUndoId nUndoId, SwFmt * _pNew, SwFmt * _pDerivedFrom, SwDoc * _pDoc)
    : SwUndo(nUndoId), pNew(_pNew),
      pDoc(_pDoc), pNewSet(NULL), nId(0), bAuto(sal_False)
{
    if (_pDerivedFrom)
        sDerivedFrom = _pDerivedFrom->GetName();
}

SwUndoFmtCreate::~SwUndoFmtCreate()
{
}

void SwUndoFmtCreate::UndoImpl(::sw::UndoRedoContext &)
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
            nId = pNew->GetPoolFmtId() & COLL_GET_RANGE_BITS;
            bAuto = pNew->IsAuto();

            Delete();
        }
    }
}

void SwUndoFmtCreate::RedoImpl(::sw::UndoRedoContext &)
{
    SwFmt * pDerivedFrom = Find(sDerivedFrom);
    SwFmt * pFmt = Create(pDerivedFrom);

    if (pFmt && pNewSet)
    {
        pFmt->SetAuto(bAuto);
        pDoc->ChgFmt(*pFmt, *pNewSet);
        pFmt->SetPoolFmtId((pFmt->GetPoolFmtId()
                            & ~COLL_GET_RANGE_BITS)
                           | nId);

        pNew = pFmt;
    }
    else
        pNew = NULL;
}

SwRewriter SwUndoFmtCreate::GetRewriter() const
{
    if (sNewName.isEmpty() && pNew)
        sNewName = pNew->GetName();

    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, sNewName);

    return aRewriter;
}

SwUndoFmtDelete::SwUndoFmtDelete
(SwUndoId nUndoId, SwFmt * _pOld, SwDoc * _pDoc)
    : SwUndo(nUndoId),
      pDoc(_pDoc), sOldName(_pOld->GetName()),
      aOldSet(_pOld->GetAttrSet())
{
    sDerivedFrom = _pOld->DerivedFrom()->GetName();
    nId = _pOld->GetPoolFmtId() & COLL_GET_RANGE_BITS;
    bAuto = _pOld->IsAuto();
}

SwUndoFmtDelete::~SwUndoFmtDelete()
{
}

void SwUndoFmtDelete::UndoImpl(::sw::UndoRedoContext &)
{
    SwFmt * pDerivedFrom = Find(sDerivedFrom);

    SwFmt * pFmt = Create(pDerivedFrom);

    if (pFmt)
    {
        pDoc->ChgFmt(*pFmt, aOldSet);
        pFmt->SetAuto(bAuto);
        pFmt->SetPoolFmtId((pFmt->GetPoolFmtId() &
                                ~COLL_GET_RANGE_BITS)
                               | nId);
    }
}

void SwUndoFmtDelete::RedoImpl(::sw::UndoRedoContext &)
{
    SwFmt * pOld = Find(sOldName);

    if (pOld)
    {
        Delete(pOld);
    }
}

SwRewriter SwUndoFmtDelete::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, sOldName);

    return aRewriter;
}

SwUndoRenameFmt::SwUndoRenameFmt(SwUndoId nUndoId,
                                 const OUString & _sOldName,
                                 const OUString & _sNewName,
                                 SwDoc * _pDoc)
    : SwUndo(nUndoId), sOldName(_sOldName),
      sNewName(_sNewName), pDoc(_pDoc)
{
}

SwUndoRenameFmt::~SwUndoRenameFmt()
{
}

void SwUndoRenameFmt::UndoImpl(::sw::UndoRedoContext &)
{
    SwFmt * pFmt = Find(sNewName);

    if (pFmt)
    {
        pDoc->RenameFmt(*pFmt, sOldName, true);
    }
}

void SwUndoRenameFmt::RedoImpl(::sw::UndoRedoContext &)
{
    SwFmt *  pFmt = Find(sOldName);

    if (pFmt)
    {
        pDoc->RenameFmt(*pFmt, sNewName, true);
    }
}

SwRewriter SwUndoRenameFmt::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, sOldName);
    aRewriter.AddRule(UndoArg2, SW_RES(STR_YIELDS));
    aRewriter.AddRule(UndoArg3, sNewName);

    return aRewriter;
}

SwUndoTxtFmtCollCreate::SwUndoTxtFmtCollCreate
(SwTxtFmtColl * _pNew, SwTxtFmtColl * _pDerivedFrom, SwDoc * _pDoc)
    : SwUndoFmtCreate(UNDO_TXTFMTCOL_CREATE, _pNew, _pDerivedFrom, _pDoc)
{
}

SwFmt * SwUndoTxtFmtCollCreate::Create(SwFmt * pDerivedFrom)
{
    return pDoc->MakeTxtFmtColl(sNewName, (SwTxtFmtColl *)pDerivedFrom, true);
}

void SwUndoTxtFmtCollCreate::Delete()
{
    pDoc->DelTxtFmtColl((SwTxtFmtColl *) pNew, true);
}

SwFmt * SwUndoTxtFmtCollCreate::Find(const OUString & rName) const
{
    return pDoc->FindTxtFmtCollByName(rName);
}

SwUndoTxtFmtCollDelete::SwUndoTxtFmtCollDelete(SwTxtFmtColl * _pOld,
                                               SwDoc * _pDoc)
    : SwUndoFmtDelete(UNDO_TXTFMTCOL_DELETE, _pOld, _pDoc)
{
}

SwFmt * SwUndoTxtFmtCollDelete::Create(SwFmt * pDerivedFrom)
{
    return pDoc->MakeTxtFmtColl(sOldName, (SwTxtFmtColl *) pDerivedFrom, true);
}

void SwUndoTxtFmtCollDelete::Delete(SwFmt * pOld)
{
    pDoc->DelTxtFmtColl((SwTxtFmtColl *) pOld, true);
}

SwFmt * SwUndoTxtFmtCollDelete::Find(const OUString & rName) const
{
    return pDoc->FindTxtFmtCollByName(rName);
}

SwUndoRenameFmtColl::SwUndoRenameFmtColl(const OUString & sInitOldName,
                                         const OUString & sInitNewName,
                                         SwDoc * _pDoc)
    : SwUndoRenameFmt(UNDO_TXTFMTCOL_RENAME, sInitOldName, sInitNewName, _pDoc)
{
}

SwFmt * SwUndoRenameFmtColl::Find(const OUString & rName) const
{
    return pDoc->FindTxtFmtCollByName(rName);
}

SwUndoCharFmtCreate::SwUndoCharFmtCreate(SwCharFmt * pNewFmt,
                                         SwCharFmt * pDerivedFrom,
                                         SwDoc * pDocument)
    : SwUndoFmtCreate(UNDO_CHARFMT_CREATE, pNewFmt, pDerivedFrom, pDocument)
{
}

SwFmt * SwUndoCharFmtCreate::Create(SwFmt * pDerivedFrom)
{
    return pDoc->MakeCharFmt(sNewName, (SwCharFmt *) pDerivedFrom, true);
}

void SwUndoCharFmtCreate::Delete()
{
    pDoc->DelCharFmt((SwCharFmt *) pNew, true);
}

SwFmt * SwUndoCharFmtCreate::Find(const OUString & rName) const
{
    return pDoc->FindCharFmtByName(rName);
}

SwUndoCharFmtDelete::SwUndoCharFmtDelete(SwCharFmt * pOld, SwDoc * pDocument)
    : SwUndoFmtDelete(UNDO_CHARFMT_DELETE, pOld, pDocument)
{
}

SwFmt * SwUndoCharFmtDelete::Create(SwFmt * pDerivedFrom)
{
    return pDoc->MakeCharFmt(sOldName, (SwCharFmt *) pDerivedFrom, true);
}

void SwUndoCharFmtDelete::Delete(SwFmt * pFmt)
{
    pDoc->DelCharFmt((SwCharFmt *) pFmt, true);
}

SwFmt * SwUndoCharFmtDelete::Find(const OUString & rName) const
{
    return pDoc->FindCharFmtByName(rName);
}

SwUndoRenameCharFmt::SwUndoRenameCharFmt(const OUString & sInitOldName,
                                         const OUString & sInitNewName,
                                         SwDoc * pDocument)
    : SwUndoRenameFmt(UNDO_CHARFMT_RENAME, sInitOldName, sInitNewName, pDocument)
{
}

SwFmt * SwUndoRenameCharFmt::Find(const OUString & rName) const
{
    return pDoc->FindCharFmtByName(rName);
}

SwUndoFrmFmtCreate::SwUndoFrmFmtCreate(SwFrmFmt * pNewFmt,
                                       SwFrmFmt * pDerivedFrom,
                                       SwDoc * pDocument)
    : SwUndoFmtCreate(UNDO_FRMFMT_CREATE, pNewFmt, pDerivedFrom, pDocument),
      bAuto(pNewFmt->IsAuto())
{
}

SwFmt * SwUndoFrmFmtCreate::Create(SwFmt * pDerivedFrom)
{
    return pDoc->MakeFrmFmt(sNewName, (SwFrmFmt *) pDerivedFrom, true, bAuto);
}

void SwUndoFrmFmtCreate::Delete()
{
    pDoc->DelFrmFmt((SwFrmFmt *) pNew, true);
}

SwFmt * SwUndoFrmFmtCreate::Find(const OUString & rName) const
{
    return pDoc->FindFrmFmtByName(rName);
}

SwUndoFrmFmtDelete::SwUndoFrmFmtDelete(SwFrmFmt * pOld, SwDoc * pDocument)
    : SwUndoFmtDelete(UNDO_FRMFMT_DELETE, pOld, pDocument)
{
}

SwFmt * SwUndoFrmFmtDelete::Create(SwFmt * pDerivedFrom)
{
    return pDoc->MakeFrmFmt(sOldName, (SwFrmFmt *) pDerivedFrom, true);
}

void SwUndoFrmFmtDelete::Delete(SwFmt * pFmt)
{
    pDoc->DelFrmFmt((SwFrmFmt *) pFmt, true);
}

SwFmt * SwUndoFrmFmtDelete::Find(const OUString & rName) const
{
    return pDoc->FindFrmFmtByName(rName);
}

SwUndoRenameFrmFmt::SwUndoRenameFrmFmt(const OUString & sInitOldName,
                                       const OUString & sInitNewName,
                                       SwDoc * pDocument)
    : SwUndoRenameFmt(UNDO_FRMFMT_RENAME, sInitOldName, sInitNewName, pDocument)
{
}

SwFmt * SwUndoRenameFrmFmt::Find(const OUString & rName) const
{
    return pDoc->FindFrmFmtByName(rName);
}

SwUndoNumruleCreate::SwUndoNumruleCreate(const SwNumRule * _pNew,
                                         SwDoc * _pDoc)
    : SwUndo(UNDO_NUMRULE_CREATE), pNew(_pNew), aNew(*_pNew), pDoc(_pDoc),
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
    : SwUndo(UNDO_NUMRULE_DELETE), aOld(rRule), pDoc(_pDoc)
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
    : SwUndo(UNDO_NUMRULE_RENAME), aOldName(_aOldName), aNewName(_aNewName),
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
    aRewriter.AddRule(UndoArg2, SW_RESSTR(STR_YIELDS));
    aRewriter.AddRule(UndoArg3, aNewName);

    return aRewriter;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
