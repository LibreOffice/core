/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwUndoFmt.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 21:48:58 $
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
(USHORT nUndoId, SwFmt * _pNew, SwFmt * _pDerivedFrom, SwDoc * _pDoc)
    : SwUndo(nUndoId), pNew(_pNew),
      pDoc(_pDoc), pNewSet(NULL), nId(0), bAuto(FALSE)
{
    if (_pDerivedFrom)
        sDerivedFrom = _pDerivedFrom->GetName();
}

SwUndoFmtCreate::~SwUndoFmtCreate()
{
}

void SwUndoFmtCreate::Undo(SwUndoIter & rIter)
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

            BOOL bDoesUndo = pDoc->DoesUndo();

            pDoc->DoUndo(FALSE);
            Delete();
            pDoc->DoUndo(bDoesUndo);
        }
    }
}

void SwUndoFmtCreate::Redo(SwUndoIter & rIter)
{
    BOOL bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(FALSE);
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
(USHORT nUndoId, SwFmt * _pOld, SwDoc * _pDoc)
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

void SwUndoFmtDelete::Undo(SwUndoIter & rIter)
{
    BOOL bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(FALSE);

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

void SwUndoFmtDelete::Redo(SwUndoIter & rIter)
{
    SwFmt * pOld = Find(sOldName);

    if (pOld)
    {
        BOOL bDoesUndo = pDoc->DoesUndo();

        pDoc->DoUndo(FALSE);
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

SwUndoRenameFmt::SwUndoRenameFmt(USHORT nUndoId,
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

void SwUndoRenameFmt::Undo(SwUndoIter & rIter)
{
    SwFmt * pFmt = Find(sNewName);

    if (pFmt)
    {
        BOOL bDoesUndo = pDoc->DoesUndo();

        pDoc->DoUndo(FALSE);
        pDoc->RenameFmt(*pFmt, sOldName, TRUE);
        pDoc->DoUndo(bDoesUndo);
    }
}

void SwUndoRenameFmt::Redo(SwUndoIter & rIter)
{
    SwFmt *  pFmt = Find(sOldName);

    if (pFmt)
    {
        BOOL bDoesUndo = pDoc->DoesUndo();

        pDoc->DoUndo(FALSE);
        pDoc->RenameFmt(*pFmt, sNewName, TRUE);
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
    return pDoc->MakeTxtFmtColl(sNewName, (SwTxtFmtColl *)pDerivedFrom, TRUE);
}

void SwUndoTxtFmtCollCreate::Delete()
{
    pDoc->DelTxtFmtColl((SwTxtFmtColl *) pNew, TRUE);
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
    return pDoc->MakeTxtFmtColl(sOldName, (SwTxtFmtColl *) pDerivedFrom, TRUE);
}

void SwUndoTxtFmtCollDelete::Delete(SwFmt * pOld)
{
    pDoc->DelTxtFmtColl((SwTxtFmtColl *) pOld, TRUE);
}

SwFmt * SwUndoTxtFmtCollDelete::Find(const String & rName) const
{
    return pDoc->FindTxtFmtCollByName(rName);
}

SwUndoRenameFmtColl::SwUndoRenameFmtColl(const String & sOldName,
                                         const String & sNewName,
                                         SwDoc * _pDoc)
    : SwUndoRenameFmt(UNDO_TXTFMTCOL_RENAME, sOldName, sNewName, _pDoc)
{
}

SwFmt * SwUndoRenameFmtColl::Find(const String & rName) const
{
    return pDoc->FindTxtFmtCollByName(rName);
}

SwUndoCharFmtCreate::SwUndoCharFmtCreate(SwCharFmt * pNew,
                                         SwCharFmt * pDerivedFrom,
                                         SwDoc * pDoc)
    : SwUndoFmtCreate(UNDO_CHARFMT_CREATE, pNew, pDerivedFrom, pDoc)
{
}

SwFmt * SwUndoCharFmtCreate::Create(SwFmt * pDerivedFrom)
{
    return pDoc->MakeCharFmt(sNewName, (SwCharFmt *) pDerivedFrom, TRUE);
}

void SwUndoCharFmtCreate::Delete()
{
    pDoc->DelCharFmt((SwCharFmt *) pNew, TRUE);
}

SwFmt * SwUndoCharFmtCreate::Find(const String & rName) const
{
    return pDoc->FindCharFmtByName(rName);
}

SwUndoCharFmtDelete::SwUndoCharFmtDelete(SwCharFmt * pOld, SwDoc * pDoc)
    : SwUndoFmtDelete(UNDO_CHARFMT_DELETE, pOld, pDoc)
{
}

SwFmt * SwUndoCharFmtDelete::Create(SwFmt * pDerivedFrom)
{
    return pDoc->MakeCharFmt(sOldName, (SwCharFmt *) pDerivedFrom, TRUE);
}

void SwUndoCharFmtDelete::Delete(SwFmt * pFmt)
{
    pDoc->DelCharFmt((SwCharFmt *) pFmt, TRUE);
}

SwFmt * SwUndoCharFmtDelete::Find(const String & rName) const
{
    return pDoc->FindCharFmtByName(rName);
}

SwUndoRenameCharFmt::SwUndoRenameCharFmt(const String & sOldName,
                                         const String & sNewName,
                                         SwDoc * pDoc)
    : SwUndoRenameFmt(UNDO_CHARFMT_RENAME, sOldName, sNewName, pDoc)
{
}

SwFmt * SwUndoRenameCharFmt::Find(const String & rName) const
{
    return pDoc->FindCharFmtByName(rName);
}

SwUndoFrmFmtCreate::SwUndoFrmFmtCreate(SwFrmFmt * pNew,
                                       SwFrmFmt * pDerivedFrom,
                                       SwDoc * pDoc)
    : SwUndoFmtCreate(UNDO_FRMFMT_CREATE, pNew, pDerivedFrom, pDoc),
      bAuto(pNew->IsAuto())
{
}

SwFmt * SwUndoFrmFmtCreate::Create(SwFmt * pDerivedFrom)
{
    return pDoc->MakeFrmFmt(sNewName, (SwFrmFmt *) pDerivedFrom, TRUE, bAuto);
}

void SwUndoFrmFmtCreate::Delete()
{
    pDoc->DelFrmFmt((SwFrmFmt *) pNew, TRUE);
}

SwFmt * SwUndoFrmFmtCreate::Find(const String & rName) const
{
    return pDoc->FindFrmFmtByName(rName);
}

SwUndoFrmFmtDelete::SwUndoFrmFmtDelete(SwFrmFmt * pOld, SwDoc * pDoc)
    : SwUndoFmtDelete(UNDO_FRMFMT_DELETE, pOld, pDoc)
{
}

SwFmt * SwUndoFrmFmtDelete::Create(SwFmt * pDerivedFrom)
{
    return pDoc->MakeFrmFmt(sOldName, (SwFrmFmt *) pDerivedFrom, TRUE);
}

void SwUndoFrmFmtDelete::Delete(SwFmt * pFmt)
{
    pDoc->DelFrmFmt((SwFrmFmt *) pFmt, TRUE);
}

SwFmt * SwUndoFrmFmtDelete::Find(const String & rName) const
{
    return pDoc->FindFrmFmtByName(rName);
}

SwUndoRenameFrmFmt::SwUndoRenameFrmFmt(const String & sOldName,
                                       const String & sNewName,
                                       SwDoc * pDoc)
    : SwUndoRenameFmt(UNDO_FRMFMT_RENAME, sOldName, sNewName, pDoc)
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

void SwUndoNumruleCreate::Undo(SwUndoIter & rIter)
{
    BOOL bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(FALSE);

    if (! bInitialized)
    {
        aNew = *pNew;
        bInitialized = true;
    }

    pDoc->DelNumRule(aNew.GetName(), TRUE);
    pDoc->DoUndo(bDoesUndo);
}

void SwUndoNumruleCreate::Redo(SwUndoIter & rIter)
{
    BOOL bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(FALSE);
    pDoc->MakeNumRule(aNew.GetName(), &aNew, TRUE);
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

void SwUndoNumruleDelete::Undo(SwUndoIter & rIter)
{
    BOOL bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(FALSE);
    pDoc->MakeNumRule(aOld.GetName(), &aOld, TRUE);
    pDoc->DoUndo(bDoesUndo);
}

void SwUndoNumruleDelete::Redo(SwUndoIter & rIter)
{
    BOOL bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(FALSE);
    pDoc->DelNumRule(aOld.GetName(), TRUE);
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

void SwUndoNumruleRename::Undo(SwUndoIter & rIter)
{
    BOOL bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(FALSE);
    pDoc->RenameNumRule(aNewName, aOldName, TRUE);
    pDoc->DoUndo(bDoesUndo);
}

void SwUndoNumruleRename::Redo(SwUndoIter & rIter)
{
    BOOL bDoesUndo = pDoc->DoesUndo();

    pDoc->DoUndo(FALSE);
    pDoc->RenameNumRule(aOldName, aNewName, TRUE);
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
