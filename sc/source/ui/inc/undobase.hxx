/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SC_UNDOBASE_HXX
#define SC_UNDOBASE_HXX

#include <svl/undo.hxx>
#include "global.hxx"
#include "address.hxx"
#include "docsh.hxx"

class ScDocument;
class ScDocShell;
class SdrUndoAction;
class ScRefUndoData;
class ScDBData;

//----------------------------------------------------------------------------

class ScSimpleUndo: public SfxUndoAction
{
public:
                    TYPEINFO();
                    ScSimpleUndo( ScDocShell* pDocSh );
    virtual         ~ScSimpleUndo();

    virtual sal_Bool    Merge( SfxUndoAction *pNextAction );

protected:
    ScDocShell*     pDocShell;
    SfxUndoAction*  pDetectiveUndo;

    bool            IsPaintLocked() const { return pDocShell->IsPaintLocked(); }

    bool            SetViewMarkData( const ScMarkData& rMarkData );

    void            BeginUndo();
    void            EndUndo();
    void            BeginRedo();
    void            EndRedo();

    static void     ShowTable( SCTAB nTab );
    static void     ShowTable( const ScRange& rRange );
};

//----------------------------------------------------------------------------

enum ScBlockUndoMode { SC_UNDO_SIMPLE, SC_UNDO_MANUALHEIGHT, SC_UNDO_AUTOHEIGHT };

class ScBlockUndo: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScBlockUndo( ScDocShell* pDocSh, const ScRange& rRange,
                                 ScBlockUndoMode eBlockMode );
    virtual         ~ScBlockUndo();

protected:
    ScRange         aBlockRange;
    SdrUndoAction*  pDrawUndo;
    ScBlockUndoMode eMode;

    void            BeginUndo();
    void            EndUndo();
//  void            BeginRedo();
    void            EndRedo();

    sal_Bool            AdjustHeight();
    void            ShowBlock();
};

class ScMultiBlockUndo: public ScSimpleUndo
{
public:
    TYPEINFO();
    ScMultiBlockUndo(ScDocShell* pDocSh, const ScRangeList& rRanges,
                     ScBlockUndoMode eBlockMode);
    virtual ~ScMultiBlockUndo();

protected:
    ScRangeList     maBlockRanges;
    SdrUndoAction*  mpDrawUndo;
    ScBlockUndoMode meMode;

    void BeginUndo();
    void EndUndo();
    void EndRedo();

    void AdjustHeight();
    void ShowBlock();
};

//----------------------------------------------------------------------------

// for functions that act on a database range - takes care of the unnamed database range
// (collected separately, before the undo action, for showing dialogs etc.)

class ScDBFuncUndo: public ScSimpleUndo
{
protected:
    ScDBData*       pAutoDBRange;
    ScRange         aOriginalRange;
    SdrUndoAction*  mpDrawUndo;

public:
                    TYPEINFO();
                    ScDBFuncUndo( ScDocShell* pDocSh, const ScRange& rOriginal, SdrUndoAction* pDrawUndo = 0 );
    virtual         ~ScDBFuncUndo();

    void            SetDrawUndoAction( SdrUndoAction* pDrawUndo );

    void            BeginUndo();
    void            EndUndo();
    void            BeginRedo();
    void            EndRedo();
};

//----------------------------------------------------------------------------

enum ScMoveUndoMode { SC_UNDO_REFFIRST, SC_UNDO_REFLAST };

class ScMoveUndo: public ScSimpleUndo               // mit Referenzen
{
public:
                    TYPEINFO();
                    ScMoveUndo( ScDocShell* pDocSh,
                                ScDocument* pRefDoc, ScRefUndoData* pRefData,
                                ScMoveUndoMode eRefMode );
    virtual         ~ScMoveUndo();

protected:
    SdrUndoAction*  pDrawUndo;
    ScDocument*     pRefUndoDoc;
    ScRefUndoData*  pRefUndoData;
    ScMoveUndoMode  eMode;

    void            BeginUndo();
    void            EndUndo();
//  void            BeginRedo();
//  void            EndRedo();

private:
    void            UndoRef();
};

//----------------------------------------------------------------------------

class ScUndoWrapper: public SfxUndoAction           // for manual merging of actions
{
    SfxUndoAction*  pWrappedUndo;

public:
                            TYPEINFO();
                            ScUndoWrapper( SfxUndoAction* pUndo );
    virtual                 ~ScUndoWrapper();

    SfxUndoAction*          GetWrappedUndo()        { return pWrappedUndo; }
    void                    ForgetWrappedUndo();

    virtual sal_Bool            IsLinked();
    virtual void            SetLinked( sal_Bool bIsLinked );
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool        CanRepeat(SfxRepeatTarget& rTarget) const;
    virtual sal_Bool        Merge( SfxUndoAction *pNextAction );
    virtual OUString        GetComment() const;
    virtual OUString        GetRepeatComment(SfxRepeatTarget&) const;
    virtual sal_uInt16      GetId() const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
