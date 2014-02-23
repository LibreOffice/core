/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EDITENG_SOURCE_OUTLINER_OUTLUNDO_HXX
#define INCLUDED_EDITENG_SOURCE_OUTLINER_OUTLUNDO_HXX

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
    sal_Int32       mnPara;
    sal_uInt16      mnOldFlags;
    sal_uInt16      mnNewFlags;

    void ImplChangeFlags( sal_uInt16 nFlags );

public:
    OutlinerUndoChangeParaFlags( Outliner* pOutliner, sal_Int32 nPara, sal_uInt16 nOldDepth, sal_uInt16 nNewDepth );

    virtual void    Undo();
    virtual void    Redo();
};

class OutlinerUndoChangeParaNumberingRestart : public OutlinerUndoBase
{
private:
    sal_Int32       mnPara;

    struct ParaRestartData
    {
        sal_Int16       mnNumberingStartValue;
        sal_Bool        mbParaIsNumberingRestart;
    };

    ParaRestartData maUndoData;
    ParaRestartData maRedoData;

    void ImplApplyData( const ParaRestartData& rData );
public:
    OutlinerUndoChangeParaNumberingRestart( Outliner* pOutliner, sal_Int32 nPara,
        sal_Int16 nOldNumberingStartValue, sal_Int16 mnNewNumberingStartValue,
        sal_Bool  nOldbParaIsNumberingRestart, sal_Bool nbNewParaIsNumberingRestart );

    virtual void    Undo();
    virtual void    Redo();
};

class OutlinerUndoChangeDepth : public OutlinerUndoBase
{
    using SfxUndoAction::Repeat;
private:
    sal_Int32       mnPara;
    sal_Int16       mnOldDepth;
    sal_Int16       mnNewDepth;

public:
                    OutlinerUndoChangeDepth( Outliner* pOutliner, sal_Int32 nPara, sal_Int16 nOldDepth, sal_Int16 nNewDepth );

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};

// Help-Undo: If it does not exist an OutlinerUndoAction for a certain action
// because this is handled by the EditEngine, but for example the bullet has
// to be recalculated.
class OutlinerUndoCheckPara : public OutlinerUndoBase
{
    using SfxUndoAction::Repeat;
private:
    sal_Int32       mnPara;

public:
                    OutlinerUndoCheckPara( Outliner* pOutliner, sal_Int32 nPara );

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat();
};






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

    sal_uInt16* pParas;  // 0 == nCount contains paragraph number
    Outliner* pOutliner;
    sal_Int32 nCount;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
