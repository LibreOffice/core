/*************************************************************************
 *
 *  $RCSfile: SwUndoFmt.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-01-05 11:45:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SW_UNDO_TXT_FMT_COLL_HXX
#define _SW_UNDO_TXT_FMT_COLL_HXX

#include <undobj.hxx>
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
    USHORT nId;
    BOOL bAuto;

public:

    SwUndoFmtCreate(USHORT nUndoId, SwFmt * pNew, SwFmt * pDerivedFrom,
                    SwDoc * pDoc);
    virtual ~SwUndoFmtCreate();

    virtual void Undo(SwUndoIter & rIter);
    virtual void Redo(SwUndoIter & rIter);

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
    USHORT nId;
    BOOL bAuto;

public:
    SwUndoFmtDelete(USHORT nUndoId, SwFmt * pOld, SwDoc * pDoc);
    ~SwUndoFmtDelete();

    virtual void Undo(SwUndoIter & rIter);
    virtual void Redo(SwUndoIter & rIter);

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
    USHORT nId;

public:
    SwUndoRenameFmt(USHORT nUndoId, const String & sOldName,
                    const String & sNewName,
                    SwDoc * pDoc);
    ~SwUndoRenameFmt();

    void Undo(SwUndoIter & rIter);
    void Redo(SwUndoIter & rIter);

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
    BOOL bAuto;

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

    virtual void Undo(SwUndoIter & rIter);
    virtual void Redo(SwUndoIter & rIter);

    SwRewriter GetRewriter() const;
};

class SwUndoNumruleDelete : public SwUndo
{
    SwNumRule aOld;
    SwDoc * pDoc;

public:
    SwUndoNumruleDelete(const SwNumRule  & aRule, SwDoc * pDoc);

    virtual void Undo(SwUndoIter & rIter);
    virtual void Redo(SwUndoIter & rIter);

    SwRewriter GetRewriter() const;
};

class SwUndoNumruleRename : public SwUndo
{
    String aOldName, aNewName;
    SwDoc * pDoc;

 public:
    SwUndoNumruleRename(const String & aOldName, const String & aNewName,
                        SwDoc * pDoc);

    virtual void Undo(SwUndoIter & rIter);
    virtual void Redo(SwUndoIter & rIter);

    SwRewriter GetRewriter() const;
};
#endif // _SW_UNDO_TXT_FMT_COLL_HXX
