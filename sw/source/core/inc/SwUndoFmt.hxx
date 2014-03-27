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
class SwTxtFmtColl;
class SwRewriter;
class SfxItemSet;

class SwUndoFmtCreate : public SwUndo
{
protected:
    SwFmt * pNew;
    OUString sDerivedFrom;
    SwDoc * pDoc;
    mutable OUString sNewName;
    SfxItemSet * pNewSet;
    sal_uInt16 nId;     // FmtId related
    sal_Bool bAuto;

public:
    SwUndoFmtCreate(SwUndoId nUndoId, SwFmt * pNew, SwFmt * pDerivedFrom,
                    SwDoc * pDoc);
    virtual ~SwUndoFmtCreate();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    virtual SwRewriter GetRewriter() const SAL_OVERRIDE;

    virtual SwFmt * Create(SwFmt * pDerivedFrom) = 0;
    virtual void Delete() = 0;
    virtual SwFmt * Find(const OUString & rName) const = 0;
};

class SwUndoFmtDelete : public SwUndo
{
protected:
    OUString sDerivedFrom;
    SwDoc * pDoc;
    OUString sOldName;
    SfxItemSet aOldSet;
    sal_uInt16 nId;     // FmtId related
    sal_Bool bAuto;

public:
    SwUndoFmtDelete(SwUndoId nUndoId, SwFmt * pOld, SwDoc * pDoc);
    ~SwUndoFmtDelete();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    virtual SwRewriter GetRewriter() const SAL_OVERRIDE;

    virtual SwFmt * Create(SwFmt * pDerivedFrom) = 0;
    virtual void Delete(SwFmt * pFmt) = 0;
    virtual SwFmt * Find(const OUString & rName) const = 0;
};

class SwUndoRenameFmt : public SwUndo
{
protected:
    OUString sOldName, sNewName;
    SwDoc * pDoc;

public:
    SwUndoRenameFmt(SwUndoId nUndoId, const OUString & sOldName,
                    const OUString & sNewName,
                    SwDoc * pDoc);
    ~SwUndoRenameFmt();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    SwRewriter GetRewriter() const SAL_OVERRIDE;

    virtual SwFmt * Find(const OUString & rName) const = 0;
};

class SwUndoTxtFmtCollCreate : public SwUndoFmtCreate
{
public:
    SwUndoTxtFmtCollCreate(SwTxtFmtColl * pNew, SwTxtFmtColl * pDerivedFrom,
                           SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom) SAL_OVERRIDE;
    virtual void Delete() SAL_OVERRIDE;
    virtual SwFmt * Find(const OUString & rName) const SAL_OVERRIDE;
};

class SwUndoTxtFmtCollDelete : public SwUndoFmtDelete
{
public:
    SwUndoTxtFmtCollDelete(SwTxtFmtColl * pOld, SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom) SAL_OVERRIDE;
    virtual void Delete(SwFmt * pFmt) SAL_OVERRIDE;
    virtual SwFmt * Find(const OUString & rName) const SAL_OVERRIDE;
};

class SwUndoRenameFmtColl : public SwUndoRenameFmt
{
public:
    SwUndoRenameFmtColl(const OUString & sOldName,
                        const OUString & sNewName,
                        SwDoc * pDoc);

    virtual SwFmt * Find(const OUString & rName) const SAL_OVERRIDE;
};

class SwUndoCharFmtCreate : public SwUndoFmtCreate
{
public:
    SwUndoCharFmtCreate(SwCharFmt * pNew, SwCharFmt * pDerivedFrom,
                           SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom) SAL_OVERRIDE;
    virtual void Delete() SAL_OVERRIDE;
    virtual SwFmt * Find(const OUString & rName) const SAL_OVERRIDE;
};

class SwUndoCharFmtDelete : public SwUndoFmtDelete
{
public:
    SwUndoCharFmtDelete(SwCharFmt * pOld, SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom) SAL_OVERRIDE;
    virtual void Delete(SwFmt * pFmt) SAL_OVERRIDE;
    virtual SwFmt * Find(const OUString & rName) const SAL_OVERRIDE;
};

class SwUndoRenameCharFmt : public SwUndoRenameFmt
{
public:
    SwUndoRenameCharFmt(const OUString & sOldName,
                        const OUString & sNewName,
                        SwDoc * pDoc);

    virtual SwFmt * Find(const OUString & rName) const SAL_OVERRIDE;
};

class SwUndoFrmFmtCreate : public SwUndoFmtCreate
{
    sal_Bool bAuto;

public:
    SwUndoFrmFmtCreate(SwFrmFmt * pNew, SwFrmFmt * pDerivedFrom,
                       SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom) SAL_OVERRIDE;
    virtual void Delete() SAL_OVERRIDE;
    virtual SwFmt * Find(const OUString & rName) const SAL_OVERRIDE;
};

class SwUndoFrmFmtDelete : public SwUndoFmtDelete
{
public:
    SwUndoFrmFmtDelete(SwFrmFmt * pOld, SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom) SAL_OVERRIDE;
    virtual void Delete(SwFmt * pFmt) SAL_OVERRIDE;
    virtual SwFmt * Find(const OUString & rName) const SAL_OVERRIDE;
};

class SwUndoRenameFrmFmt : public SwUndoRenameFmt
{
public:
    SwUndoRenameFrmFmt(const OUString & sOldName,
                       const OUString & sNewName,
                       SwDoc * pDoc);

    virtual SwFmt * Find(const OUString & rName) const SAL_OVERRIDE;
};

class SwUndoNumruleCreate : public SwUndo
{
    const SwNumRule * pNew;
    mutable SwNumRule aNew;
    SwDoc * pDoc;
    mutable bool bInitialized;

public:
    SwUndoNumruleCreate(const SwNumRule * pNew, SwDoc * pDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    SwRewriter GetRewriter() const SAL_OVERRIDE;
};

class SwUndoNumruleDelete : public SwUndo
{
    SwNumRule aOld;
    SwDoc * pDoc;

public:
    SwUndoNumruleDelete(const SwNumRule  & aRule, SwDoc * pDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    SwRewriter GetRewriter() const SAL_OVERRIDE;
};

class SwUndoNumruleRename : public SwUndo
{
    OUString aOldName, aNewName;
    SwDoc * pDoc;

 public:
    SwUndoNumruleRename(const OUString & aOldName, const OUString & aNewName,
                        SwDoc * pDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    SwRewriter GetRewriter() const SAL_OVERRIDE;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_SWUNDOFMT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
