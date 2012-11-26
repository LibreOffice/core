/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

//----------------------------------------------------------------------------

// for functions that act on a database range - takes care of the unnamed database range
// (collected separately, before the undo action, for showing dialogs etc.)

class ScDBFuncUndo: public ScSimpleUndo
{
    ScDBData*       pAutoDBRange;
    ScRange         aOriginalRange;
    SdrUndoAction*  mpDrawUndo;

public:
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
                            ScUndoWrapper( SfxUndoAction* pUndo );
    virtual                 ~ScUndoWrapper();

    SfxUndoAction*          GetWrappedUndo()        { return pWrappedUndo; }
    void                    ForgetWrappedUndo();

    virtual sal_Bool            IsLinked();
    virtual void            SetLinked( sal_Bool bIsLinked );
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool            CanRepeat(SfxRepeatTarget& rTarget) const;
    virtual sal_Bool            Merge( SfxUndoAction *pNextAction );
    virtual String          GetComment() const;
    virtual String          GetRepeatComment(SfxRepeatTarget&) const;
    virtual sal_uInt16          GetId() const;
};


#endif

