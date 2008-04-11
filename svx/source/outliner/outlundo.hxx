/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outlundo.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _OUTLUNDO_HXX
#define _OUTLUNDO_HXX

#include <svx/outliner.hxx>

#ifndef _EDITDATA_HXX
#include <svx/editdata.hxx>
#endif
#include <editund2.hxx>

class OutlinerUndoBase : public EditUndo
{
private:
    Outliner*   mpOutliner;

public:
                OutlinerUndoBase( USHORT nId, Outliner* pOutliner );

    Outliner*   GetOutliner() const { return mpOutliner; }
};

class OutlinerUndoChangeDepth : public OutlinerUndoBase
{
    using SfxUndoAction::Repeat;
private:
    USHORT          mnPara;
    USHORT          mnOldDepth;
    USHORT          mnNewDepth;

public:
                    OutlinerUndoChangeDepth( Outliner* pOutliner, USHORT nPara, USHORT nOldDepth, USHORT nNewDepth );

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};

// Hilfs-Undo: Wenn es fuer eine Aktion keine OutlinerUndoAction gibst, weil
// die EditEngine das handelt, aber z.B. noch das Bullet neu berechnet werden muss.

class OutlinerUndoCheckPara : public OutlinerUndoBase
{
    using SfxUndoAction::Repeat;
private:
    USHORT          mnPara;

public:
                    OutlinerUndoCheckPara( Outliner* pOutliner, USHORT nPara );

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};



// -------------------------------------


class OLUndoExpand : public EditUndo
{
    using SfxUndoAction::Repeat;
    void Restore( BOOL bUndo );
public:
    OLUndoExpand( Outliner* pOut, USHORT nId );
    ~OLUndoExpand();
    virtual void Undo();
    virtual void Redo();
    virtual void Repeat();

    USHORT* pParas;  // 0 == nCount enthaelt Absatznummer
    Outliner* pOutliner;
    USHORT nCount;
};

#endif
