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
#ifndef SW_UNDO_TXT_FMT_COLL_HXX
#define SW_UNDO_TXT_FMT_COLL_HXX

#include <undobj.hxx>
#include <swundo.hxx>
#include <numrule.hxx>


class SwDoc;
class SwTxtFmtColl;
class String;
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
    virtual SwFmt * Find(const String & rName) const = 0;
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
    virtual SwFmt * Find(const String & rName) const = 0;
};

class SwUndoRenameFmt : public SwUndo
{
protected:
    String sOldName, sNewName;
    SwDoc * pDoc;
//    SwUndoId nId;

public:
    SwUndoRenameFmt(SwUndoId nUndoId, const String & sOldName,
                    const String & sNewName,
                    SwDoc * pDoc);
    ~SwUndoRenameFmt();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    SwRewriter GetRewriter() const;

    virtual SwFmt * Find(const String & rName) const = 0;
};

class SwUndoTxtFmtCollCreate : public SwUndoFmtCreate
{
public:
    SwUndoTxtFmtCollCreate(SwTxtFmtColl * pNew, SwTxtFmtColl * pDerivedFrom,
                           SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete();
    virtual SwFmt * Find(const String & rName) const;
};

class SwUndoTxtFmtCollDelete : public SwUndoFmtDelete
{
public:
    SwUndoTxtFmtCollDelete(SwTxtFmtColl * pOld, SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete(SwFmt * pFmt);
    virtual SwFmt * Find(const String & rName) const;
};

class SwUndoRenameFmtColl : public SwUndoRenameFmt
{
public:
    SwUndoRenameFmtColl(const String & sOldName,
                        const String & sNewName,
                        SwDoc * pDoc);

    virtual SwFmt * Find(const String & rName) const;
};

class SwUndoCharFmtCreate : public SwUndoFmtCreate
{
public:
    SwUndoCharFmtCreate(SwCharFmt * pNew, SwCharFmt * pDerivedFrom,
                           SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete();
    virtual SwFmt * Find(const String & rName) const;
};

class SwUndoCharFmtDelete : public SwUndoFmtDelete
{
public:
    SwUndoCharFmtDelete(SwCharFmt * pOld, SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete(SwFmt * pFmt);
    virtual SwFmt * Find(const String & rName) const;
};

class SwUndoRenameCharFmt : public SwUndoRenameFmt
{
public:
    SwUndoRenameCharFmt(const String & sOldName,
                        const String & sNewName,
                        SwDoc * pDoc);

    virtual SwFmt * Find(const String & rName) const;
};

class SwUndoFrmFmtCreate : public SwUndoFmtCreate
{
    sal_Bool bAuto;

public:
    SwUndoFrmFmtCreate(SwFrmFmt * pNew, SwFrmFmt * pDerivedFrom,
                       SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete();
    virtual SwFmt * Find(const String & rName) const;
};

class SwUndoFrmFmtDelete : public SwUndoFmtDelete
{
public:
    SwUndoFrmFmtDelete(SwFrmFmt * pOld, SwDoc * pDoc);

    virtual SwFmt * Create(SwFmt * pDerivedFrom);
    virtual void Delete(SwFmt * pFmt);
    virtual SwFmt * Find(const String & rName) const;
};

class SwUndoRenameFrmFmt : public SwUndoRenameFmt
{
public:
    SwUndoRenameFrmFmt(const String & sOldName,
                       const String & sNewName,
                       SwDoc * pDoc);

    virtual SwFmt * Find(const String & rName) const;
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
