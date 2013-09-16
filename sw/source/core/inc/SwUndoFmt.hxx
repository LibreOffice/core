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

#ifndef SW_UNDO_TXT_FMT_COLL_HXX
#define SW_UNDO_TXT_FMT_COLL_HXX

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
    String sDerivedFrom;
    SwDoc * pDoc;
    mutable String sNewName;
    SfxItemSet * pNewSet;
    sal_uInt16 nId;     // FmtId related
    sal_Bool bAuto;

public:
    SwUndoFmtCreate(SwUndoId nUndoId, SwFmt * pNew, SwFmt * pDerivedFrom,
                    SwDoc * pDoc);
    virtual ~SwUndoFmtCreate();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    virtual SwRewriter GetRewriter() const;

    virtual SwFmt * Create(SwFmt * pDerivedFrom) = 0;
    virtual void Delete() = 0;
    virtual SwFmt * Find(const OUString & rName) const = 0;
};

class SwUndoFmtDelete : public SwUndo
{
protected:
    String sDerivedFrom;
    SwDoc * pDoc;
    String sOldName;
    SfxItemSet aOldSet;
    sal_uInt16 nId;     // FmtId related
    sal_Bool bAuto;

public:
    SwUndoFmtDelete(SwUndoId nUndoId, SwFmt * pOld, SwDoc * pDoc);
    ~SwUndoFmtDelete();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    virtual SwRewriter GetRewriter() const;

    virtual SwFmt * Create(SwFmt * pDerivedFrom) = 0;
    virtual void Delete(SwFmt * pFmt) = 0;
    virtual SwFmt * Find(const OUString & rName) const = 0;
};

class SwUndoRenameFmt : public SwUndo
{
protected:
    String sOldName, sNewName;
    SwDoc * pDoc;

public:
    SwUndoRenameFmt(SwUndoId nUndoId, const String & sOldName,
                    const String & sNewName,
                    SwDoc * pDoc);
    ~SwUndoRenameFmt();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    SwRewriter GetRewriter() const;

    virtual SwFmt * Find(const OUString & rName) const = 0;
};

class SwUndoTxtFmtCollCreate : public SwUndoFmtCreate
{
public:
    SwUndoTxtFmtCollCreate(SwTxtFmtColl * pNew, SwTxtFmtColl * pDerivedFrom,
                           SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete();
    virtual SwFmt * Find(const OUString & rName) const;
};

class SwUndoTxtFmtCollDelete : public SwUndoFmtDelete
{
public:
    SwUndoTxtFmtCollDelete(SwTxtFmtColl * pOld, SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete(SwFmt * pFmt);
    virtual SwFmt * Find(const OUString & rName) const;
};

class SwUndoRenameFmtColl : public SwUndoRenameFmt
{
public:
    SwUndoRenameFmtColl(const String & sOldName,
                        const String & sNewName,
                        SwDoc * pDoc);

    virtual SwFmt * Find(const OUString & rName) const;
};

class SwUndoCharFmtCreate : public SwUndoFmtCreate
{
public:
    SwUndoCharFmtCreate(SwCharFmt * pNew, SwCharFmt * pDerivedFrom,
                           SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete();
    virtual SwFmt * Find(const OUString & rName) const;
};

class SwUndoCharFmtDelete : public SwUndoFmtDelete
{
public:
    SwUndoCharFmtDelete(SwCharFmt * pOld, SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete(SwFmt * pFmt);
    virtual SwFmt * Find(const OUString & rName) const;
};

class SwUndoRenameCharFmt : public SwUndoRenameFmt
{
public:
    SwUndoRenameCharFmt(const String & sOldName,
                        const String & sNewName,
                        SwDoc * pDoc);

    virtual SwFmt * Find(const OUString & rName) const;
};

class SwUndoFrmFmtCreate : public SwUndoFmtCreate
{
    sal_Bool bAuto;

public:
    SwUndoFrmFmtCreate(SwFrmFmt * pNew, SwFrmFmt * pDerivedFrom,
                       SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete();
    virtual SwFmt * Find(const OUString & rName) const;
};

class SwUndoFrmFmtDelete : public SwUndoFmtDelete
{
public:
    SwUndoFrmFmtDelete(SwFrmFmt * pOld, SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete(SwFmt * pFmt);
    virtual SwFmt * Find(const OUString & rName) const;
};

class SwUndoRenameFrmFmt : public SwUndoRenameFmt
{
public:
    SwUndoRenameFrmFmt(const String & sOldName,
                       const String & sNewName,
                       SwDoc * pDoc);

    virtual SwFmt * Find(const OUString & rName) const;
};

class SwUndoNumruleCreate : public SwUndo
{
    const SwNumRule * pNew;
    mutable SwNumRule aNew;
    SwDoc * pDoc;
    mutable bool bInitialized;

public:
    SwUndoNumruleCreate(const SwNumRule * pNew, SwDoc * pDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    SwRewriter GetRewriter() const;
};

class SwUndoNumruleDelete : public SwUndo
{
    SwNumRule aOld;
    SwDoc * pDoc;

public:
    SwUndoNumruleDelete(const SwNumRule  & aRule, SwDoc * pDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    SwRewriter GetRewriter() const;
};

class SwUndoNumruleRename : public SwUndo
{
    String aOldName, aNewName;
    SwDoc * pDoc;

 public:
    SwUndoNumruleRename(const String & aOldName, const String & aNewName,
                        SwDoc * pDoc);

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    SwRewriter GetRewriter() const;
};

#endif // SW_UNDO_TXT_FMT_COLL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
