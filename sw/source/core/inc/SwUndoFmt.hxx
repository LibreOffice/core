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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWUNDOFMT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWUNDOFMT_HXX

#include <undobj.hxx>
#include <swundo.hxx>
#include <numrule.hxx>
#include <memory>

class SwDoc;
class SwTextFormatColl;
class SwConditionTextFormatColl;
class SwRewriter;

class SwUndoFormatCreate : public SwUndo
{
protected:
    SwFormat * m_pNew;
    UIName m_sDerivedFrom;
    SwDoc& m_rDoc;
    mutable UIName m_sNewName;
    std::unique_ptr<SfxItemSet> m_pNewSet;
    sal_uInt16 m_nId;     // FormatId related
    bool m_bAuto;

public:
    SwUndoFormatCreate(SwUndoId nUndoId, SwFormat * pNew, SwFormat const * pDerivedFrom,
                       SwDoc& rDoc);
    virtual ~SwUndoFormatCreate() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;

    virtual SwFormat * Create(SwFormat * pDerivedFrom) = 0;
    virtual void Delete() = 0;
    virtual SwFormat * Find(const UIName & rName) const = 0;
};

class SwUndoFormatDelete : public SwUndo
{
protected:
    UIName m_sDerivedFrom;
    SwDoc& m_rDoc;
    UIName m_sOldName;
    SfxItemSet m_aOldSet;
    sal_uInt16 m_nId;     // FormatId related
    bool m_bAuto;

public:
    SwUndoFormatDelete(SwUndoId nUndoId, SwFormat const * pOld, SwDoc& rDoc);
    virtual ~SwUndoFormatDelete() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;

    virtual SwFormat * Create(SwFormat * pDerivedFrom) = 0;
    virtual void Delete(SwFormat * pFormat) = 0;
    virtual SwFormat * Find(const UIName & rName) const = 0;
};

class SwUndoRenameFormat : public SwUndo
{
protected:
    UIName m_sOldName, m_sNewName;
    SwDoc& m_rDoc;

public:
    SwUndoRenameFormat(SwUndoId nUndoId, UIName sOldName,
                    UIName  sNewName,
                    SwDoc& rDoc);
    virtual ~SwUndoRenameFormat() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    SwRewriter GetRewriter() const override;

    virtual SwFormat * Find(const UIName & rName) const = 0;
};

class SwUndoTextFormatCollCreate : public SwUndoFormatCreate
{
public:
    SwUndoTextFormatCollCreate(SwTextFormatColl * pNew, SwTextFormatColl const * pDerivedFrom,
                               SwDoc& rDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete() override;
    virtual SwFormat * Find(const UIName & rName) const override;
};

class SwUndoTextFormatCollDelete : public SwUndoFormatDelete
{
public:
    SwUndoTextFormatCollDelete(SwTextFormatColl const * pOld, SwDoc& rDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete(SwFormat * pFormat) override;
    virtual SwFormat * Find(const UIName & rName) const override;
};

class SwUndoCondTextFormatCollCreate final : public SwUndoTextFormatCollCreate
{
public:
    SwUndoCondTextFormatCollCreate(SwConditionTextFormatColl * pNew, SwTextFormatColl const * pDerivedFrom, SwDoc& rDoc);
    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
};

class SwUndoCondTextFormatCollDelete final : public SwUndoTextFormatCollDelete
{
public:
    SwUndoCondTextFormatCollDelete(SwTextFormatColl const * pOld, SwDoc& rDoc);
    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
};

class SwUndoRenameFormatColl final : public SwUndoRenameFormat
{
public:
    SwUndoRenameFormatColl(const UIName & sOldName,
                        const UIName & sNewName,
                        SwDoc& rDoc);

    virtual SwFormat * Find(const UIName & rName) const override;
};

class SwUndoCharFormatCreate final : public SwUndoFormatCreate
{
public:
    SwUndoCharFormatCreate(SwCharFormat * pNew, SwCharFormat const * pDerivedFrom,
                           SwDoc& rDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete() override;
    virtual SwFormat * Find(const UIName & rName) const override;
};

class SwUndoCharFormatDelete final : public SwUndoFormatDelete
{
public:
    SwUndoCharFormatDelete(SwCharFormat const * pOld, SwDoc& rDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete(SwFormat * pFormat) override;
    virtual SwFormat * Find(const UIName & rName) const override;
};

class SwUndoRenameCharFormat final : public SwUndoRenameFormat
{
public:
    SwUndoRenameCharFormat(const UIName & sOldName,
                        const UIName & sNewName,
                        SwDoc& rDoc);

    virtual SwFormat * Find(const UIName & rName) const override;
};

class SwUndoFrameFormatCreate final : public SwUndoFormatCreate
{
public:
    SwUndoFrameFormatCreate(SwFrameFormat * pNew, SwFrameFormat const * pDerivedFrom,
                            SwDoc& rDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete() override;
    virtual SwFormat * Find(const UIName & rName) const override;
};

class SwUndoFrameFormatDelete final : public SwUndoFormatDelete
{
public:
    SwUndoFrameFormatDelete(SwFrameFormat const * pOld, SwDoc& rDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete(SwFormat * pFormat) override;
    virtual SwFormat * Find(const UIName & rName) const override;
};

class SwUndoRenameFrameFormat final : public SwUndoRenameFormat
{
public:
    SwUndoRenameFrameFormat(const UIName & sOldName,
                       const UIName & sNewName,
                       SwDoc& rDoc);

    virtual SwFormat * Find(const UIName & rName) const override;
};

class SwUndoNumruleCreate final : public SwUndo
{
    const SwNumRule * m_pNew;
    mutable SwNumRule m_aNew;
    SwDoc& m_rDoc;
    mutable bool m_bInitialized;

public:
    SwUndoNumruleCreate(const SwNumRule * pNew, SwDoc& rDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    SwRewriter GetRewriter() const override;
};

class SwUndoNumruleDelete final : public SwUndo
{
    SwNumRule m_aOld;
    SwDoc& m_rDoc;

public:
    SwUndoNumruleDelete(const SwNumRule& rRule, SwDoc& rDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    SwRewriter GetRewriter() const override;
};

class SwUndoNumruleRename final : public SwUndo
{
    UIName m_aOldName, m_aNewName;
    SwDoc& m_rDoc;

 public:
    SwUndoNumruleRename(UIName aOldName, UIName aNewName,
                        SwDoc& rDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    SwRewriter GetRewriter() const override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_SWUNDOFMT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
