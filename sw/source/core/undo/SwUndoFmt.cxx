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

#include <tools/resid.hxx>
#include <poolfmt.hxx>
#include <charfmt.hxx>
#include <frmfmt.hxx>
#include <SwUndoFmt.hxx>
#include <SwRewriter.hxx>
#include <swundo.hxx>
#include <undobj.hxx>
#include <fmtcol.hxx>
#include <tools/string.hxx>
#include <doc.hxx>
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

void SwUndoFmtCreate::Undo(SwUndoIter &)
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

            sal_Bool bDoesUndo = pDoc->DoesUndo();

            pDoc->DoUndo(sal_False);
            Delete();
            pDoc->DoUndo(bDoesUndo);
        }
    }
}

void SwUndoFmtCreate::Redo(SwUndoIter &)
{
    sal_Bool bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(sal_False);
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

    pDoc->DoUndo(bDoesUndo);
}

SwRewriter SwUndoFmtCreate::GetRewriter() const
{
    if (sNewName.Len() == 0 && pNew)
        sNewName = pNew->GetName();

    SwRewriter aRewriter;

    aRewriter.AddRule(UNDO_ARG1, sNewName);

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

void SwUndoFmtDelete::Undo(SwUndoIter &)
{
    sal_Bool bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(sal_False);

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

    pDoc->DoUndo(bDoesUndo);
}

void SwUndoFmtDelete::Redo(SwUndoIter &)
{
    SwFmt * pOld = Find(sOldName);

    if (pOld)
    {
        sal_Bool bDoesUndo = pDoc->DoesUndo();

        pDoc->DoUndo(sal_False);
        Delete(pOld);
        pDoc->DoUndo(bDoesUndo);
    }
}

SwRewriter SwUndoFmtDelete::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UNDO_ARG1, sOldName);

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

void SwUndoRenameFmt::Undo(SwUndoIter &)
{
    SwFmt * pFmt = Find(sNewName);

    if (pFmt)
    {
        sal_Bool bDoesUndo = pDoc->DoesUndo();

        pDoc->DoUndo(sal_False);
        pDoc->RenameFmt(*pFmt, sOldName, sal_True);
        pDoc->DoUndo(bDoesUndo);
    }
}

void SwUndoRenameFmt::Redo(SwUndoIter &)
{
    SwFmt *  pFmt = Find(sOldName);

    if (pFmt)
    {
        sal_Bool bDoesUndo = pDoc->DoesUndo();

        pDoc->DoUndo(sal_False);
        pDoc->RenameFmt(*pFmt, sNewName, sal_True);
        pDoc->DoUndo(bDoesUndo);
    }
}

SwRewriter SwUndoRenameFmt::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UNDO_ARG1, sOldName);
    aRewriter.AddRule(UNDO_ARG2, SW_RES(STR_YIELDS));
    aRewriter.AddRule(UNDO_ARG3, sNewName);

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
    pDoc->DelTxtFmtColl((SwTxtFmtColl *) pNew, sal_True);
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
    pDoc->DelTxtFmtColl((SwTxtFmtColl *) pOld, sal_True);
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
    return pDoc->MakeCharFmt(sNewName, (SwCharFmt *) pDerivedFrom, sal_True);
}

void SwUndoCharFmtCreate::Delete()
{
    pDoc->DelCharFmt((SwCharFmt *) pNew, sal_True);
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
    return pDoc->MakeCharFmt(sOldName, (SwCharFmt *) pDerivedFrom, sal_True);
}

void SwUndoCharFmtDelete::Delete(SwFmt * pFmt)
{
    pDoc->DelCharFmt((SwCharFmt *) pFmt, sal_True);
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
    return pDoc->MakeFrmFmt(sNewName, (SwFrmFmt *) pDerivedFrom, sal_True, bAuto);
}

void SwUndoFrmFmtCreate::Delete()
{
    pDoc->DelFrmFmt((SwFrmFmt *) pNew, sal_True);
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
    return pDoc->MakeFrmFmt(sOldName, (SwFrmFmt *) pDerivedFrom, sal_True);
}

void SwUndoFrmFmtDelete::Delete(SwFmt * pFmt)
{
    pDoc->DelFrmFmt((SwFrmFmt *) pFmt, sal_True);
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

void SwUndoNumruleCreate::Undo(SwUndoIter &)
{
    sal_Bool bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(sal_False);

    if (! bInitialized)
    {
        aNew = *pNew;
        bInitialized = true;
    }

    pDoc->DelNumRule(aNew.GetName(), sal_True);
    pDoc->DoUndo(bDoesUndo);
}

void SwUndoNumruleCreate::Redo(SwUndoIter &)
{
    sal_Bool bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(sal_False);
    pDoc->MakeNumRule(aNew.GetName(), &aNew, sal_True);
    pDoc->DoUndo(bDoesUndo);
}

SwRewriter SwUndoNumruleCreate::GetRewriter() const
{
    SwRewriter aResult;

    if (! bInitialized)
    {
        aNew = *pNew;
        bInitialized = true;
    }

    aResult.AddRule(UNDO_ARG1, aNew.GetName());

    return aResult;
}

SwUndoNumruleDelete::SwUndoNumruleDelete(const SwNumRule & rRule,
                                         SwDoc * _pDoc)
    : SwUndo(UNDO_NUMRULE_DELETE), aOld(rRule), pDoc(_pDoc)
{
}

void SwUndoNumruleDelete::Undo(SwUndoIter &)
{
    sal_Bool bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(sal_False);
    pDoc->MakeNumRule(aOld.GetName(), &aOld, sal_True);
    pDoc->DoUndo(bDoesUndo);
}

void SwUndoNumruleDelete::Redo(SwUndoIter &)
{
    sal_Bool bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(sal_False);
    pDoc->DelNumRule(aOld.GetName(), sal_True);
    pDoc->DoUndo(bDoesUndo);
}

SwRewriter SwUndoNumruleDelete::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UNDO_ARG1, aOld.GetName());

    return aResult;
}

SwUndoNumruleRename::SwUndoNumruleRename(const String & _aOldName,
                                         const String & _aNewName,
                                         SwDoc * _pDoc)
    : SwUndo(UNDO_NUMRULE_RENAME), aOldName(_aOldName), aNewName(_aNewName),
      pDoc(_pDoc)
{
}

void SwUndoNumruleRename::Undo(SwUndoIter &)
{
    sal_Bool bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(sal_False);
    pDoc->RenameNumRule(aNewName, aOldName, sal_True);
    pDoc->DoUndo(bDoesUndo);
}

void SwUndoNumruleRename::Redo(SwUndoIter &)
{
    sal_Bool bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(sal_False);
    pDoc->RenameNumRule(aOldName, aNewName, sal_True);
    pDoc->DoUndo(bDoesUndo);
}

SwRewriter SwUndoNumruleRename::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UNDO_ARG1, aOldName);
    aRewriter.AddRule(UNDO_ARG2, SW_RES(STR_YIELDS));
    aRewriter.AddRule(UNDO_ARG3, aNewName);

    return aRewriter;
}
