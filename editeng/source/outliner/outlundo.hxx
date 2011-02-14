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

#ifndef _OUTLUNDO_HXX
#define _OUTLUNDO_HXX

#include <editeng/outliner.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editund2.hxx>

class OutlinerUndoBase : public EditUndo
{
private:
    Outliner*   mpOutliner;

public:
                OutlinerUndoBase( sal_uInt16 nId, Outliner* pOutliner );

    Outliner*   GetOutliner() const { return mpOutliner; }
};

class OutlinerUndoChangeParaFlags : public OutlinerUndoBase
{
private:
    sal_uInt16      mnPara;
    sal_uInt16      mnOldFlags;
    sal_uInt16      mnNewFlags;

    void ImplChangeFlags( sal_uInt16 nFlags );

public:
    OutlinerUndoChangeParaFlags( Outliner* pOutliner, sal_uInt16 nPara, sal_uInt16 nOldDepth, sal_uInt16 nNewDepth );

    virtual void    Undo();
    virtual void    Redo();
};

class OutlinerUndoChangeParaNumberingRestart : public OutlinerUndoBase
{
private:
    sal_uInt16      mnPara;

    struct ParaRestartData
    {
        sal_Int16       mnNumberingStartValue;
        sal_Bool        mbParaIsNumberingRestart;
    };

    ParaRestartData maUndoData;
    ParaRestartData maRedoData;

    void ImplApplyData( const ParaRestartData& rData );
public:
    OutlinerUndoChangeParaNumberingRestart( Outliner* pOutliner, sal_uInt16 nPara,
        sal_Int16 nOldNumberingStartValue, sal_Int16 mnNewNumberingStartValue,
        sal_Bool  nOldbParaIsNumberingRestart, sal_Bool nbNewParaIsNumberingRestart );

    virtual void    Undo();
    virtual void    Redo();
};

class OutlinerUndoChangeDepth : public OutlinerUndoBase
{
    using SfxUndoAction::Repeat;
private:
    sal_uInt16          mnPara;
    sal_Int16       mnOldDepth;
    sal_Int16       mnNewDepth;

public:
                    OutlinerUndoChangeDepth( Outliner* pOutliner, sal_uInt16 nPara, sal_Int16 nOldDepth, sal_Int16 nNewDepth );

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
    sal_uInt16          mnPara;

public:
                    OutlinerUndoCheckPara( Outliner* pOutliner, sal_uInt16 nPara );

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};



// -------------------------------------


class OLUndoExpand : public EditUndo
{
    using SfxUndoAction::Repeat;
    void Restore( sal_Bool bUndo );
public:
    OLUndoExpand( Outliner* pOut, sal_uInt16 nId );
    ~OLUndoExpand();
    virtual void Undo();
    virtual void Redo();
    virtual void Repeat();

    sal_uInt16* pParas;  // 0 == nCount enthaelt Absatznummer
    Outliner* pOutliner;
    sal_uInt16 nCount;
};

#endif
