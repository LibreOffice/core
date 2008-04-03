/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwUndoField.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:49:30 $
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
#ifndef _SW_UNDO_FIELD_HXX
#define _SW_UNDO_FIELD_HXX

#include <undobj.hxx>

class SwDoc;
class SwField;
class SwMsgPoolItem;

class SwUndoField : public SwUndo
{
    ULONG nNodeIndex;
    xub_StrLen nOffset;

protected:
    SwDoc * pDoc;
    SwPosition GetPosition();

public:
    SwUndoField(const SwPosition & rPos, SwUndoId nId = UNDO_FIELD );
    virtual ~SwUndoField();
};

class SwUndoFieldFromDoc : public SwUndoField
{
    SwField * pOldField, * pNewField;
    SwMsgPoolItem * pHnt;
    BOOL bUpdate;

public:
    SwUndoFieldFromDoc(const SwPosition & rPos, const SwField & aOldField,
                       const SwField & aNewField,
                       SwMsgPoolItem * pHnt, BOOL bUpdate,
                       SwUndoId nId = UNDO_FIELD );
    virtual ~SwUndoFieldFromDoc();

    virtual void Undo(SwUndoIter & rIt);
    virtual void Redo(SwUndoIter & rIt);
    virtual void Repeat(SwUndoIter & rIt);
};

class SwUndoFieldFromAPI : public SwUndoField
{
    com::sun::star::uno::Any aOldVal, aNewVal;
    USHORT nWhich;

public:
    SwUndoFieldFromAPI(const SwPosition & rPos,
                       const com::sun::star::uno::Any & rOldVal,
                       const com::sun::star::uno::Any & rNewVal,
                       USHORT nWhich);
    virtual ~SwUndoFieldFromAPI();

    virtual void Undo(SwUndoIter & rIt);
    virtual void Redo(SwUndoIter & rIt);
    virtual void Repeat(SwUndoIter & rIt);
};

#endif // _SW_UNDO_FIELD_HXX
