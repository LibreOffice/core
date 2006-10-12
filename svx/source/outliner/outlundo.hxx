/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outlundo.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 13:02:55 $
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

#ifndef _OUTLUNDO_HXX
#define _OUTLUNDO_HXX

#include <outliner.hxx>

#ifndef _EDITDATA_HXX
#include <editdata.hxx>
#endif

#ifndef _EDITUND2_HXX
#include <editund2.hxx>
#endif

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
