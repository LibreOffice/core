/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <tools/resid.hxx>
#include <tools/string.hxx>

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
        if (sNewName.Len() == 0 && pNew)
            sNewName = pNew->GetName();

        if (sNewName.Len() > 0)
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
    if (sNewName.Len() == 0 && pNew)
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
                                 const String & _sOldName,
                                 const String & _sNewName,
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
    return pDoc->MakeTxtFmtColl(sNewName, (SwTxtFmtColl *)pDerivedFrom, sal_True);
}

void SwUndoTxtFmtCollCreate::Delete()
{
    pDoc->DelTxtFmtColl((SwTxtFmtColl *) pNew, true);
}

SwFmt * SwUndoTxtFmtCollCreate::Find(const String & rName) const
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
    return pDoc->MakeTxtFmtColl(sOldName, (SwTxtFmtColl *) pDerivedFrom, sal_True);
}

void SwUndoTxtFmtCollDelete::Delete(SwFmt * pOld)
{
    pDoc->DelTxtFmtColl((SwTxtFmtColl *) pOld, true);
}

SwFmt * SwUndoTxtFmtCollDelete::Find(const String & rName) const
{
    return pDoc->FindTxtFmtCollByName(rName);
}

SwUndoRenameFmtColl::SwUndoRenameFmtColl(const String & sInitOldName,
                                         const String & sInitNewName,
                                         SwDoc * _pDoc)
    : SwUndoRenameFmt(UNDO_TXTFMTCOL_RENAME, sInitOldName, sInitNewName, _pDoc)
{
}

SwFmt * SwUndoRenameFmtColl::Find(const String & rName) const
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

SwFmt * SwUndoCharFmtCreate::Find(const String & rName) const
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

SwFmt * SwUndoCharFmtDelete::Find(const String & rName) const
{
    return pDoc->FindCharFmtByName(rName);
}

SwUndoRenameCharFmt::SwUndoRenameCharFmt(const String & sInitOldName,
                                         const String & sInitNewName,
                                         SwDoc * pDocument)
    : SwUndoRenameFmt(UNDO_CHARFMT_RENAME, sInitOldName, sInitNewName, pDocument)
{
}

SwFmt * SwUndoRenameCharFmt::Find(const String & rName) const
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

SwFmt * SwUndoFrmFmtCreate::Find(const String & rName) const
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

SwFmt * SwUndoFrmFmtDelete::Find(const String & rName) const
{
    return pDoc->FindFrmFmtByName(rName);
}

SwUndoRenameFrmFmt::SwUndoRenameFrmFmt(const String & sInitOldName,
                                       const String & sInitNewName,
                                       SwDoc * pDocument)
    : SwUndoRenameFmt(UNDO_FRMFMT_RENAME, sInitOldName, sInitNewName, pDocument)
{
}

SwFmt * SwUndoRenameFrmFmt::Find(const String & rName) const
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

    pDoc->DelNumRule(aNew.GetName(), sal_True);
}

void SwUndoNumruleCreate::RedoImpl(::sw::UndoRedoContext &)
{
    pDoc->MakeNumRule(aNew.GetName(), &aNew, sal_True);
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
    pDoc->MakeNumRule(aOld.GetName(), &aOld, sal_True);
}

void SwUndoNumruleDelete::RedoImpl(::sw::UndoRedoContext &)
{
    pDoc->DelNumRule(aOld.GetName(), sal_True);
}

SwRewriter SwUndoNumruleDelete::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UndoArg1, aOld.GetName());

    return aResult;
}

SwUndoNumruleRename::SwUndoNumruleRename(const String & _aOldName,
                                         const String & _aNewName,
                                         SwDoc * _pDoc)
    : SwUndo(UNDO_NUMRULE_RENAME), aOldName(_aOldName), aNewName(_aNewName),
      pDoc(_pDoc)
{
}

void SwUndoNumruleRename::UndoImpl(::sw::UndoRedoContext &)
{
    pDoc->RenameNumRule(aNewName, aOldName, sal_True);
}

void SwUndoNumruleRename::RedoImpl(::sw::UndoRedoContext &)
{
    pDoc->RenameNumRule(aOldName, aNewName, sal_True);
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
