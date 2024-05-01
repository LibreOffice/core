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


#include <editeng/outliner.hxx>
#include <tools/debug.hxx>
#include "outlundo.hxx"


OutlinerUndoBase::OutlinerUndoBase( sal_uInt16 _nId, Outliner* pOutliner )
    : EditUndo( _nId, nullptr )
{
    DBG_ASSERT( pOutliner, "Undo: Outliner?!" );
    mpOutliner = pOutliner;
}

OutlinerUndoChangeParaFlags::OutlinerUndoChangeParaFlags( Outliner* pOutliner, sal_Int32 nPara, ParaFlag nOldFlags, ParaFlag nNewFlags )
: OutlinerUndoBase( OLUNDO_DEPTH, pOutliner ), mnPara(nPara), mnOldFlags(nOldFlags), mnNewFlags(nNewFlags)
{
}

void OutlinerUndoChangeParaFlags::Undo()
{
    ImplChangeFlags( mnOldFlags );
}

void OutlinerUndoChangeParaFlags::Redo()
{
    ImplChangeFlags( mnNewFlags );
}

void OutlinerUndoChangeParaFlags::ImplChangeFlags( ParaFlag nFlags )
{
    Outliner* pOutliner = GetOutliner();
    Paragraph* pPara = pOutliner->GetParagraph( mnPara );
    if( pPara )
    {
        pOutliner->nDepthChangedHdlPrevDepth = pPara->GetDepth();
        ParaFlag nPrevFlags = pPara->nFlags;

        pPara->nFlags = nFlags;
        pOutliner->DepthChangedHdl(pPara, nPrevFlags);
    }
}

OutlinerUndoChangeParaNumberingRestart::OutlinerUndoChangeParaNumberingRestart( Outliner* pOutliner, sal_Int32 nPara,
        sal_Int16 nOldNumberingStartValue, sal_Int16 nNewNumberingStartValue,
        bool  bOldParaIsNumberingRestart, bool bNewParaIsNumberingRestart )
: OutlinerUndoBase( OLUNDO_DEPTH, pOutliner ), mnPara(nPara)
{
    maUndoData.mnNumberingStartValue = nOldNumberingStartValue;
    maUndoData.mbParaIsNumberingRestart = bOldParaIsNumberingRestart;
    maRedoData.mnNumberingStartValue = nNewNumberingStartValue;
    maRedoData.mbParaIsNumberingRestart = bNewParaIsNumberingRestart;
}

void OutlinerUndoChangeParaNumberingRestart::Undo()
{
    ImplApplyData( maUndoData );
}

void OutlinerUndoChangeParaNumberingRestart::Redo()
{
    ImplApplyData( maRedoData );
}

void OutlinerUndoChangeParaNumberingRestart::ImplApplyData( const ParaRestartData& rData )
{
    Outliner* pOutliner = GetOutliner();
    pOutliner->SetNumberingStartValue( mnPara, rData.mnNumberingStartValue );
    pOutliner->SetParaIsNumberingRestart( mnPara, rData.mbParaIsNumberingRestart );
}

OutlinerUndoChangeDepth::OutlinerUndoChangeDepth( Outliner* pOutliner, sal_Int32 nPara, sal_Int16 nOldDepth, sal_Int16 nNewDepth )
    : OutlinerUndoBase( OLUNDO_DEPTH, pOutliner ), mnPara(nPara), mnOldDepth(nOldDepth), mnNewDepth(nNewDepth)
{
}

void OutlinerUndoChangeDepth::Undo()
{
    GetOutliner()->ImplInitDepth( mnPara, mnOldDepth, false );
}

void OutlinerUndoChangeDepth::Redo()
{
    GetOutliner()->ImplInitDepth( mnPara, mnNewDepth, false );
}

OutlinerUndoCheckPara::OutlinerUndoCheckPara( Outliner* pOutliner, sal_Int32 nPara )
    : OutlinerUndoBase( OLUNDO_DEPTH, pOutliner ), mnPara(nPara)
{
}

void OutlinerUndoCheckPara::Undo()
{
    Paragraph* pPara = GetOutliner()->GetParagraph( mnPara );
    pPara->Invalidate();
    GetOutliner()->ImplCalcBulletText( mnPara, false, false );
}

void OutlinerUndoCheckPara::Redo()
{
    Paragraph* pPara = GetOutliner()->GetParagraph( mnPara );
    pPara->Invalidate();
    GetOutliner()->ImplCalcBulletText( mnPara, false, false );
}

OLUndoExpand::OLUndoExpand(Outliner* pOut, sal_uInt16 _nId )
    : EditUndo( _nId, nullptr ), pOutliner(pOut), nCount(0)
{
    DBG_ASSERT(pOut,"Undo:No Outliner");
}

OLUndoExpand::~OLUndoExpand()
{
}

void OLUndoExpand::Restore( bool bUndo )
{
    assert(pOutliner && "Undo:No Outliner");
    DBG_ASSERT(pOutliner->pEditEngine,"Outliner already deleted");
    Paragraph* pPara;

    bool bExpand = false;
    sal_uInt16 _nId = GetId();
    if((_nId == OLUNDO_EXPAND && !bUndo) || (_nId == OLUNDO_COLLAPSE && bUndo))
        bExpand = true;

    pPara = pOutliner->GetParagraph( nCount );
    if( bExpand )
        pOutliner->Expand( pPara );
    else
        pOutliner->Collapse( pPara );
}

void OLUndoExpand::Undo()
{
    Restore( true );
}

void OLUndoExpand::Redo()
{
    Restore( false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
