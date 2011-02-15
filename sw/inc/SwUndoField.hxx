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
#ifndef SW_UNDO_FIELD_HXX
#define SW_UNDO_FIELD_HXX

#include <undobj.hxx>

#include <com/sun/star/uno/Any.h>


class SwDoc;
class SwField;
class SwMsgPoolItem;

class SwUndoField : public SwUndo
{
    sal_uLong nNodeIndex;
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
    sal_Bool bUpdate;

    void DoImpl();

public:
    SwUndoFieldFromDoc(const SwPosition & rPos, const SwField & aOldField,
                       const SwField & aNewField,
                       SwMsgPoolItem * pHnt, sal_Bool bUpdate,
                       SwUndoId nId = UNDO_FIELD );

    virtual ~SwUndoFieldFromDoc();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};

class SwUndoFieldFromAPI : public SwUndoField
{
    com::sun::star::uno::Any aOldVal, aNewVal;
    sal_uInt16 nWhich;

    void DoImpl();

public:
    SwUndoFieldFromAPI(const SwPosition & rPos,
                       const com::sun::star::uno::Any & rOldVal,
                       const com::sun::star::uno::Any & rNewVal,
                       sal_uInt16 nWhich);
    virtual ~SwUndoFieldFromAPI();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};

#endif // SW_UNDO_FIELD_HXX
