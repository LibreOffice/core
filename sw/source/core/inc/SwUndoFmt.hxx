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

class SwDoc;
class SwTextFormatColl;
class SwConditionTextFormatColl;
class SwRewriter;
class SfxItemSet;

class SwUndoFormatCreate : public SwUndo
{
protected:
    SwFormat * pNew;
    OUString sDerivedFrom;
    SwDoc * pDoc;
    mutable OUString sNewName;
    SfxItemSet * pNewSet;
    sal_uInt16 nId;     // FormatId related
    bool bAuto;

public:
    SwUndoFormatCreate(SwUndoId nUndoId, SwFormat * pNew, SwFormat * pDerivedFrom,
                    SwDoc * pDoc);
    virtual ~SwUndoFormatCreate();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;

    virtual SwFormat * Create(SwFormat * pDerivedFrom) = 0;
    virtual void Delete() = 0;
    virtual SwFormat * Find(const OUString & rName) const = 0;
};

class SwUndoFormatDelete : public SwUndo
{
protected:
    OUString sDerivedFrom;
    SwDoc * pDoc;
    OUString sOldName;
    SfxItemSet aOldSet;
    sal_uInt16 nId;     // FormatId related
    bool bAuto;

public:
    SwUndoFormatDelete(SwUndoId nUndoId, SwFormat * pOld, SwDoc * pDoc);
    virtual ~SwUndoFormatDelete();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;

    virtual SwFormat * Create(SwFormat * pDerivedFrom) = 0;
    virtual void Delete(SwFormat * pFormat) = 0;
    virtual SwFormat * Find(const OUString & rName) const = 0;
};

class SwUndoRenameFormat : public SwUndo
{
protected:
    OUString sOldName, sNewName;
    SwDoc * pDoc;

public:
    SwUndoRenameFormat(SwUndoId nUndoId, const OUString & sOldName,
                    const OUString & sNewName,
                    SwDoc * pDoc);
    virtual ~SwUndoRenameFormat();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    SwRewriter GetRewriter() const override;

    virtual SwFormat * Find(const OUString & rName) const = 0;
};

class SwUndoTextFormatCollCreate : public SwUndoFormatCreate
{
public:
    SwUndoTextFormatCollCreate(SwTextFormatColl * pNew, SwTextFormatColl * pDerivedFrom,
                           SwDoc * pDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete() override;
    virtual SwFormat * Find(const OUString & rName) const override;
};

class SwUndoTextFormatCollDelete : public SwUndoFormatDelete
{
public:
    SwUndoTextFormatCollDelete(SwTextFormatColl * pOld, SwDoc * pDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete(SwFormat * pFormat) override;
    virtual SwFormat * Find(const OUString & rName) const override;
};

class SwUndoCondTextFormatCollCreate : public SwUndoTextFormatCollCreate
{
public:
    SwUndoCondTextFormatCollCreate(SwConditionTextFormatColl * pNew, SwTextFormatColl * pDerivedFrom, SwDoc * pDoc);
    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
};

class SwUndoCondTextFormatCollDelete : public SwUndoTextFormatCollDelete
{
public:
    SwUndoCondTextFormatCollDelete(SwTextFormatColl * pOld, SwDoc * pDoc);
    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
};

class SwUndoRenameFormatColl : public SwUndoRenameFormat
{
public:
    SwUndoRenameFormatColl(const OUString & sOldName,
                        const OUString & sNewName,
                        SwDoc * pDoc);

    virtual SwFormat * Find(const OUString & rName) const override;
};

class SwUndoCharFormatCreate : public SwUndoFormatCreate
{
public:
    SwUndoCharFormatCreate(SwCharFormat * pNew, SwCharFormat * pDerivedFrom,
                           SwDoc * pDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete() override;
    virtual SwFormat * Find(const OUString & rName) const override;
};

class SwUndoCharFormatDelete : public SwUndoFormatDelete
{
public:
    SwUndoCharFormatDelete(SwCharFormat * pOld, SwDoc * pDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete(SwFormat * pFormat) override;
    virtual SwFormat * Find(const OUString & rName) const override;
};

class SwUndoRenameCharFormat : public SwUndoRenameFormat
{
public:
    SwUndoRenameCharFormat(const OUString & sOldName,
                        const OUString & sNewName,
                        SwDoc * pDoc);

    virtual SwFormat * Find(const OUString & rName) const override;
};

class SwUndoFrameFormatCreate : public SwUndoFormatCreate
{
    bool bAuto;

public:
    SwUndoFrameFormatCreate(SwFrameFormat * pNew, SwFrameFormat * pDerivedFrom,
                       SwDoc * pDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete() override;
    virtual SwFormat * Find(const OUString & rName) const override;
};

class SwUndoFrameFormatDelete : public SwUndoFormatDelete
{
public:
    SwUndoFrameFormatDelete(SwFrameFormat * pOld, SwDoc * pDoc);

    virtual SwFormat * Create(SwFormat * pDerivedFrom) override;
    virtual void Delete(SwFormat * pFormat) override;
    virtual SwFormat * Find(const OUString & rName) const override;
};

class SwUndoRenameFrameFormat : public SwUndoRenameFormat
{
public:
    SwUndoRenameFrameFormat(const OUString & sOldName,
                       const OUString & sNewName,
                       SwDoc * pDoc);

    virtual SwFormat * Find(const OUString & rName) const override;
};

class SwUndoNumruleCreate : public SwUndo
{
    const SwNumRule * pNew;
    mutable SwNumRule aNew;
    SwDoc * pDoc;
    mutable bool bInitialized;

public:
    SwUndoNumruleCreate(const SwNumRule * pNew, SwDoc * pDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    SwRewriter GetRewriter() const override;
};

class SwUndoNumruleDelete : public SwUndo
{
    SwNumRule aOld;
    SwDoc * pDoc;

public:
    SwUndoNumruleDelete(const SwNumRule  & aRule, SwDoc * pDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    SwRewriter GetRewriter() const override;
};

class SwUndoNumruleRename : public SwUndo
{
    OUString aOldName, aNewName;
    SwDoc * pDoc;

 public:
    SwUndoNumruleRename(const OUString & aOldName, const OUString & aNewName,
                        SwDoc * pDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    SwRewriter GetRewriter() const override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_SWUNDOFMT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
