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

#ifndef _UNDO_HXX //autogen
#include <bf_svtools/undo.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
namespace binfilter {

class ScDocument;
class ScDocShell;
class SdrUndoAction;
class ScRefUndoData;

//----------------------------------------------------------------------------

class ScSimpleUndo: public SfxUndoAction
{
public:
                    ScSimpleUndo( ScDocShell* pDocSh );
    virtual 		~ScSimpleUndo();

    virtual BOOL	Merge( SfxUndoAction *pNextAction );

protected:
    ScDocShell*		pDocShell;
    SfxUndoAction*	pDetectiveUndo;

};

//----------------------------------------------------------------------------

enum ScBlockUndoMode { SC_UNDO_SIMPLE, SC_UNDO_MANUALHEIGHT, SC_UNDO_AUTOHEIGHT };

class ScBlockUndo: public ScSimpleUndo
{
public:
                    ScBlockUndo( ScDocShell* pDocSh, const ScRange& rRange,
                                 ScBlockUndoMode eBlockMode );
    virtual 		~ScBlockUndo();

protected:
    ScRange			aBlockRange;
    SdrUndoAction*	pDrawUndo;
    ScBlockUndoMode	eMode;

//	void			BeginRedo();

};

//----------------------------------------------------------------------------



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

    virtual BOOL            IsLinked();
    virtual void            SetLinked( BOOL bIsLinked );
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL            CanRepeat(SfxRepeatTarget& rTarget) const;
    virtual BOOL            Merge( SfxUndoAction *pNextAction );
    virtual String          GetComment() const;
    virtual String          GetRepeatComment(SfxRepeatTarget&) const;
    virtual USHORT          GetId() const;
};


} //namespace binfilter
#endif

