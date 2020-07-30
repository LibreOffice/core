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

#include <poolfmt.hxx>
#include <charfmt.hxx>
#include <frmfmt.hxx>
#include <SwUndoFmt.hxx>
#include <SwRewriter.hxx>
#include <swundo.hxx>
#include <undobj.hxx>
#include <fmtcol.hxx>
#include <doc.hxx>
#include <strings.hrc>

SwUndoFormatCreate::SwUndoFormatCreate
(SwUndoId nUndoId, SwFormat * _pNew, SwFormat const * _pDerivedFrom, SwDoc * _pDoc)
    : SwUndo(nUndoId, _pDoc), m_pNew(_pNew),
      m_pDoc(_pDoc), m_pNewSet(nullptr), m_nId(0), m_bAuto(false)
{
    if (_pDerivedFrom)
        m_sDerivedFrom = _pDerivedFrom->GetName();
}

SwUndoFormatCreate::~SwUndoFormatCreate()
{
}

void SwUndoFormatCreate::UndoImpl(::sw::UndoRedoContext &)
{
    if (!m_pNew)
        return;

    if (m_sNewName.isEmpty())
        m_sNewName = m_pNew->GetName();

    if (!m_sNewName.isEmpty())
        m_pNew = Find(m_sNewName);

    if (m_pNew)
    {
        m_pNewSet = new SfxItemSet(m_pNew->GetAttrSet());
        m_nId = m_pNew->GetPoolFormatId() & COLL_GET_RANGE_BITS;
        m_bAuto = m_pNew->IsAuto();

        Delete();
    }
}

void SwUndoFormatCreate::RedoImpl(::sw::UndoRedoContext &)
{
    SwFormat * pDerivedFrom = Find(m_sDerivedFrom);
    SwFormat * pFormat = Create(pDerivedFrom);

    if (pFormat && m_pNewSet)
    {
        pFormat->SetAuto(m_bAuto);
        m_pDoc->ChgFormat(*pFormat, *m_pNewSet);
        pFormat->SetPoolFormatId((pFormat->GetPoolFormatId()
                            & ~COLL_GET_RANGE_BITS)
                           | m_nId);

        m_pNew = pFormat;
    }
    else
        m_pNew = nullptr;
}

SwRewriter SwUndoFormatCreate::GetRewriter() const
{
    if (m_sNewName.isEmpty() && m_pNew)
        m_sNewName = m_pNew->GetName();

    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, m_sNewName);

    return aRewriter;
}

SwUndoFormatDelete::SwUndoFormatDelete
(SwUndoId nUndoId, SwFormat const * _pOld, SwDoc * _pDoc)
    : SwUndo(nUndoId, _pDoc),
      m_pDoc(_pDoc), m_sOldName(_pOld->GetName()),
      m_aOldSet(_pOld->GetAttrSet())
{
    m_sDerivedFrom = _pOld->DerivedFrom()->GetName();
    m_nId = _pOld->GetPoolFormatId() & COLL_GET_RANGE_BITS;
    m_bAuto = _pOld->IsAuto();
}

SwUndoFormatDelete::~SwUndoFormatDelete()
{
}

void SwUndoFormatDelete::UndoImpl(::sw::UndoRedoContext &)
{
    SwFormat * pDerivedFrom = Find(m_sDerivedFrom);

    SwFormat * pFormat = Create(pDerivedFrom);

    if (pFormat)
    {
        m_pDoc->ChgFormat(*pFormat, m_aOldSet);
        pFormat->SetAuto(m_bAuto);
        pFormat->SetPoolFormatId((pFormat->GetPoolFormatId() &
                                ~COLL_GET_RANGE_BITS)
                               | m_nId);
    }
}

void SwUndoFormatDelete::RedoImpl(::sw::UndoRedoContext &)
{
    SwFormat * pOld = Find(m_sOldName);

    if (pOld)
    {
        Delete(pOld);
    }
}

SwRewriter SwUndoFormatDelete::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, m_sOldName);

    return aRewriter;
}

SwUndoRenameFormat::SwUndoRenameFormat(SwUndoId nUndoId,
                                 const OUString & _sOldName,
                                 const OUString & _sNewName,
                                 SwDoc * _pDoc)
    : SwUndo(nUndoId, _pDoc), m_sOldName(_sOldName),
      m_sNewName(_sNewName), m_pDoc(_pDoc)
{
}

SwUndoRenameFormat::~SwUndoRenameFormat()
{
}

void SwUndoRenameFormat::UndoImpl(::sw::UndoRedoContext &)
{
    SwFormat * pFormat = Find(m_sNewName);

    if (pFormat)
    {
        m_pDoc->RenameFormat(*pFormat, m_sOldName, true);
    }
}

void SwUndoRenameFormat::RedoImpl(::sw::UndoRedoContext &)
{
    SwFormat *  pFormat = Find(m_sOldName);

    if (pFormat)
    {
        m_pDoc->RenameFormat(*pFormat, m_sNewName, true);
    }
}

SwRewriter SwUndoRenameFormat::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, m_sOldName);
    aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
    aRewriter.AddRule(UndoArg3, m_sNewName);

    return aRewriter;
}

SwUndoTextFormatCollCreate::SwUndoTextFormatCollCreate
(SwTextFormatColl * _pNew, SwTextFormatColl const * _pDerivedFrom, SwDoc * _pDoc)
    : SwUndoFormatCreate(SwUndoId::TXTFMTCOL_CREATE, _pNew, _pDerivedFrom, _pDoc)
{
}

SwFormat * SwUndoTextFormatCollCreate::Create(SwFormat * pDerivedFrom)
{
    return m_pDoc->MakeTextFormatColl(m_sNewName, static_cast<SwTextFormatColl *>(pDerivedFrom), true);
}

void SwUndoTextFormatCollCreate::Delete()
{
    m_pDoc->DelTextFormatColl(static_cast<SwTextFormatColl *>(m_pNew), true);
}

SwFormat * SwUndoTextFormatCollCreate::Find(const OUString & rName) const
{
    return m_pDoc->FindTextFormatCollByName(rName);
}

SwUndoTextFormatCollDelete::SwUndoTextFormatCollDelete(SwTextFormatColl const * _pOld,
                                               SwDoc * _pDoc)
    : SwUndoFormatDelete(SwUndoId::TXTFMTCOL_DELETE, _pOld, _pDoc)
{
}

SwFormat * SwUndoTextFormatCollDelete::Create(SwFormat * pDerivedFrom)
{
    return m_pDoc->MakeTextFormatColl(m_sOldName, static_cast<SwTextFormatColl *>(pDerivedFrom), true);
}

void SwUndoTextFormatCollDelete::Delete(SwFormat * pOld)
{
    m_pDoc->DelTextFormatColl(static_cast<SwTextFormatColl *>(pOld), true);
}

SwFormat * SwUndoTextFormatCollDelete::Find(const OUString & rName) const
{
    return m_pDoc->FindTextFormatCollByName(rName);
}

SwUndoCondTextFormatCollCreate::SwUndoCondTextFormatCollCreate(SwConditionTextFormatColl *_pNew,
    SwTextFormatColl const *_pDerivedFrom, SwDoc *_pDoc)
    : SwUndoTextFormatCollCreate(_pNew, _pDerivedFrom, _pDoc)
{
}

SwFormat * SwUndoCondTextFormatCollCreate::Create(SwFormat * pDerivedFrom)
{
    return m_pDoc->MakeCondTextFormatColl(m_sNewName, static_cast<SwTextFormatColl *>(pDerivedFrom), true);
}

SwUndoCondTextFormatCollDelete::SwUndoCondTextFormatCollDelete(SwTextFormatColl const * _pOld,
                                                       SwDoc * _pDoc)
    : SwUndoTextFormatCollDelete(_pOld, _pDoc)
{
}

SwFormat * SwUndoCondTextFormatCollDelete::Create(SwFormat * pDerivedFrom)
{
    return m_pDoc->MakeCondTextFormatColl(m_sOldName, static_cast<SwTextFormatColl *>(pDerivedFrom), true);
}

SwUndoRenameFormatColl::SwUndoRenameFormatColl(const OUString & sInitOldName,
                                         const OUString & sInitNewName,
                                         SwDoc * _pDoc)
    : SwUndoRenameFormat(SwUndoId::TXTFMTCOL_RENAME, sInitOldName, sInitNewName, _pDoc)
{
}

SwFormat * SwUndoRenameFormatColl::Find(const OUString & rName) const
{
    return m_pDoc->FindTextFormatCollByName(rName);
}

SwUndoCharFormatCreate::SwUndoCharFormatCreate(SwCharFormat * pNewFormat,
                                         SwCharFormat const * pDerivedFrom,
                                         SwDoc * pDocument)
    : SwUndoFormatCreate(SwUndoId::CHARFMT_CREATE, pNewFormat, pDerivedFrom, pDocument)
{
}

SwFormat * SwUndoCharFormatCreate::Create(SwFormat * pDerivedFrom)
{
    return m_pDoc->MakeCharFormat(m_sNewName, static_cast<SwCharFormat *>(pDerivedFrom), true);
}

void SwUndoCharFormatCreate::Delete()
{
    m_pDoc->DelCharFormat(static_cast<SwCharFormat *>(m_pNew), true);
}

SwFormat * SwUndoCharFormatCreate::Find(const OUString & rName) const
{
    return m_pDoc->FindCharFormatByName(rName);
}

SwUndoCharFormatDelete::SwUndoCharFormatDelete(SwCharFormat const * pOld, SwDoc * pDocument)
    : SwUndoFormatDelete(SwUndoId::CHARFMT_DELETE, pOld, pDocument)
{
}

SwFormat * SwUndoCharFormatDelete::Create(SwFormat * pDerivedFrom)
{
    return m_pDoc->MakeCharFormat(m_sOldName, static_cast<SwCharFormat *>(pDerivedFrom), true);
}

void SwUndoCharFormatDelete::Delete(SwFormat * pFormat)
{
    m_pDoc->DelCharFormat(static_cast<SwCharFormat *>(pFormat), true);
}

SwFormat * SwUndoCharFormatDelete::Find(const OUString & rName) const
{
    return m_pDoc->FindCharFormatByName(rName);
}

SwUndoRenameCharFormat::SwUndoRenameCharFormat(const OUString & sInitOldName,
                                         const OUString & sInitNewName,
                                         SwDoc * pDocument)
    : SwUndoRenameFormat(SwUndoId::CHARFMT_RENAME, sInitOldName, sInitNewName, pDocument)
{
}

SwFormat * SwUndoRenameCharFormat::Find(const OUString & rName) const
{
    return m_pDoc->FindCharFormatByName(rName);
}

SwUndoFrameFormatCreate::SwUndoFrameFormatCreate(SwFrameFormat * pNewFormat,
                                       SwFrameFormat const * pDerivedFrom,
                                       SwDoc * pDocument)
    : SwUndoFormatCreate(SwUndoId::FRMFMT_CREATE, pNewFormat, pDerivedFrom, pDocument)
{
}

SwFormat * SwUndoFrameFormatCreate::Create(SwFormat * pDerivedFrom)
{
    return m_pDoc->MakeFrameFormat(m_sNewName, static_cast<SwFrameFormat *>(pDerivedFrom), true, m_pNew->IsAuto());
}

void SwUndoFrameFormatCreate::Delete()
{
    m_pDoc->DelFrameFormat(static_cast<SwFrameFormat *>(m_pNew), true);
}

SwFormat * SwUndoFrameFormatCreate::Find(const OUString & rName) const
{
    return m_pDoc->FindFrameFormatByName(rName);
}

SwUndoFrameFormatDelete::SwUndoFrameFormatDelete(SwFrameFormat const * pOld, SwDoc * pDocument)
    : SwUndoFormatDelete(SwUndoId::FRMFMT_DELETE, pOld, pDocument)
{
}

SwFormat * SwUndoFrameFormatDelete::Create(SwFormat * pDerivedFrom)
{
    return m_pDoc->MakeFrameFormat(m_sOldName, static_cast<SwFrameFormat *>(pDerivedFrom), true);
}

void SwUndoFrameFormatDelete::Delete(SwFormat * pFormat)
{
    m_pDoc->DelFrameFormat(static_cast<SwFrameFormat *>(pFormat), true);
}

SwFormat * SwUndoFrameFormatDelete::Find(const OUString & rName) const
{
    return m_pDoc->FindFrameFormatByName(rName);
}

SwUndoRenameFrameFormat::SwUndoRenameFrameFormat(const OUString & sInitOldName,
                                       const OUString & sInitNewName,
                                       SwDoc * pDocument)
    : SwUndoRenameFormat(SwUndoId::FRMFMT_RENAME, sInitOldName, sInitNewName, pDocument)
{
}

SwFormat * SwUndoRenameFrameFormat::Find(const OUString & rName) const
{
    return m_pDoc->FindFrameFormatByName(rName);
}

SwUndoNumruleCreate::SwUndoNumruleCreate(const SwNumRule * _pNew,
                                         SwDoc * _pDoc)
    : SwUndo(SwUndoId::NUMRULE_CREATE, _pDoc), m_pNew(_pNew), m_aNew(*_pNew), m_pDoc(_pDoc),
      m_bInitialized(false)
{
}

void SwUndoNumruleCreate::UndoImpl(::sw::UndoRedoContext &)
{
    if (! m_bInitialized)
    {
        m_aNew = *m_pNew;
        m_bInitialized = true;
    }

    m_pDoc->DelNumRule(m_aNew.GetName(), true);
}

void SwUndoNumruleCreate::RedoImpl(::sw::UndoRedoContext &)
{
    m_pDoc->MakeNumRule(m_aNew.GetName(), &m_aNew, true);
}

SwRewriter SwUndoNumruleCreate::GetRewriter() const
{
    SwRewriter aResult;

    if (! m_bInitialized)
    {
        m_aNew = *m_pNew;
        m_bInitialized = true;
    }

    aResult.AddRule(UndoArg1, m_aNew.GetName());

    return aResult;
}

SwUndoNumruleDelete::SwUndoNumruleDelete(const SwNumRule & rRule,
                                         SwDoc * _pDoc)
    : SwUndo(SwUndoId::NUMRULE_DELETE, _pDoc), m_aOld(rRule), m_pDoc(_pDoc)
{
}

void SwUndoNumruleDelete::UndoImpl(::sw::UndoRedoContext &)
{
    m_pDoc->MakeNumRule(m_aOld.GetName(), &m_aOld, true);
}

void SwUndoNumruleDelete::RedoImpl(::sw::UndoRedoContext &)
{
    m_pDoc->DelNumRule(m_aOld.GetName(), true);
}

SwRewriter SwUndoNumruleDelete::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UndoArg1, m_aOld.GetName());

    return aResult;
}

SwUndoNumruleRename::SwUndoNumruleRename(const OUString & _aOldName,
                                         const OUString & _aNewName,
                                         SwDoc * _pDoc)
    : SwUndo(SwUndoId::NUMRULE_RENAME, _pDoc), m_aOldName(_aOldName), m_aNewName(_aNewName),
      m_pDoc(_pDoc)
{
}

void SwUndoNumruleRename::UndoImpl(::sw::UndoRedoContext &)
{
    m_pDoc->RenameNumRule(m_aNewName, m_aOldName, true);
}

void SwUndoNumruleRename::RedoImpl(::sw::UndoRedoContext &)
{
    m_pDoc->RenameNumRule(m_aOldName, m_aNewName, true);
}

SwRewriter SwUndoNumruleRename::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, m_aOldName);
    aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
    aRewriter.AddRule(UndoArg3, m_aNewName);

    return aRewriter;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
