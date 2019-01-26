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

#include <refundo.hxx>
#include <undobase.hxx>
#include <document.hxx>
#include <dbdata.hxx>
#include <rangenam.hxx>
#include <pivot.hxx>
#include <chartarr.hxx>
#include <stlpool.hxx>
#include <detdata.hxx>
#include <prnsave.hxx>
#include <chartlis.hxx>
#include <dpobject.hxx>
#include <areasave.hxx>
#include <unoreflist.hxx>
#include <scopetools.hxx>
#include <refupdatecontext.hxx>

ScRefUndoData::ScRefUndoData( const ScDocument* pDoc ) :
    pPrintRanges(pDoc->CreatePrintRangeSaver())
{
    const ScDBCollection* pOldDBColl = pDoc->GetDBCollection();
    if (pOldDBColl && !pOldDBColl->empty())
        pDBCollection.reset(new ScDBCollection(*pOldDBColl));

    const ScRangeName* pOldRanges = pDoc->GetRangeName();
    if (pOldRanges && !pOldRanges->empty())
        pRangeName.reset(new ScRangeName(*pOldRanges));

    // when handling Pivot solely keep the range?

    const ScDPCollection* pOldDP = pDoc->GetDPCollection();
    if (pOldDP && pOldDP->GetCount())
        pDPCollection.reset(new ScDPCollection(*pOldDP));

    const ScDetOpList* pOldDetOp = pDoc->GetDetOpList();
    if (pOldDetOp && pOldDetOp->Count())
        pDetOpList.reset(new ScDetOpList(*pOldDetOp));

    const ScChartListenerCollection* pOldChartLisColl = pDoc->GetChartListenerCollection();
    if (pOldChartLisColl)
        pChartListenerCollection.reset(new ScChartListenerCollection(*pOldChartLisColl));

    pAreaLinks = ScAreaLinkSaveCollection::CreateFromDoc(pDoc);     // returns NULL if empty

    const_cast<ScDocument*>(pDoc)->BeginUnoRefUndo();
}

ScRefUndoData::~ScRefUndoData()
{
    pDBCollection.reset();
    pRangeName.reset();
    pPrintRanges.reset();
    pDPCollection.reset();
    pDetOpList.reset();
    pChartListenerCollection.reset();
    pAreaLinks.reset();
}

void ScRefUndoData::DeleteUnchanged( const ScDocument* pDoc )
{
    if (pDBCollection)
    {
        ScDBCollection* pNewDBColl = pDoc->GetDBCollection();
        if ( pNewDBColl && *pDBCollection == *pNewDBColl )
            pDBCollection.reset();
    }
    if (pRangeName)
    {
        ScRangeName* pNewRanges = pDoc->GetRangeName();
        if ( pNewRanges && *pRangeName == *pNewRanges )
            pRangeName.reset();
    }

    if (pPrintRanges)
    {
        std::unique_ptr<ScPrintRangeSaver> pNewRanges = pDoc->CreatePrintRangeSaver();
        if ( pNewRanges && *pPrintRanges == *pNewRanges )
            pPrintRanges.reset();
    }

    if (pDPCollection)
    {
        ScDPCollection* pNewDP = const_cast<ScDocument*>(pDoc)->GetDPCollection();    //! const
        if ( pNewDP && pDPCollection->RefsEqual(*pNewDP) )
            pDPCollection.reset();
    }

    if (pDetOpList)
    {
        ScDetOpList* pNewDetOp = pDoc->GetDetOpList();
        if ( pNewDetOp && *pDetOpList == *pNewDetOp )
            pDetOpList.reset();
    }

    if ( pChartListenerCollection )
    {
        ScChartListenerCollection* pNewChartListenerCollection =
            pDoc->GetChartListenerCollection();
        if ( pNewChartListenerCollection &&
                *pChartListenerCollection == *pNewChartListenerCollection )
            pChartListenerCollection.reset();
    }

    if (pAreaLinks)
    {
        if ( pAreaLinks->IsEqual( pDoc ) )
            pAreaLinks.reset();
    }

    if ( pDoc->HasUnoRefUndo() )
    {
        pUnoRefs = const_cast<ScDocument*>(pDoc)->EndUnoRefUndo();
        if ( pUnoRefs && pUnoRefs->IsEmpty() )
        {
            pUnoRefs.reset();
        }
    }
}

void ScRefUndoData::DoUndo( ScDocument* pDoc, bool bUndoRefFirst )
{
    if (pDBCollection)
        pDoc->SetDBCollection( std::unique_ptr<ScDBCollection>(new ScDBCollection(*pDBCollection)) );
    if (pRangeName)
        pDoc->SetRangeName( std::unique_ptr<ScRangeName>(new ScRangeName(*pRangeName)) );

    if (pPrintRanges)
        pDoc->RestorePrintRanges(*pPrintRanges);

    if (pDPCollection)
    {
        ScDPCollection* pDocDP = pDoc->GetDPCollection();
        if (pDocDP)
            pDPCollection->WriteRefsTo( *pDocDP );
    }

    if (pDetOpList)
        pDoc->SetDetOpList( std::unique_ptr<ScDetOpList>(new ScDetOpList(*pDetOpList)) );

    // bUndoRefFirst is bSetChartRangeLists
    if ( pChartListenerCollection )
        pDoc->SetChartListenerCollection( std::make_unique<ScChartListenerCollection>(
            *pChartListenerCollection ), bUndoRefFirst );

    if (pDBCollection || pRangeName)
    {
        sc::AutoCalcSwitch aACSwitch(*pDoc, false);
        pDoc->CompileAll();

        sc::SetFormulaDirtyContext aCxt;
        pDoc->SetAllFormulasDirty(aCxt);
    }

    if (pAreaLinks)
        pAreaLinks->Restore( pDoc );

    if ( pUnoRefs )
        pUnoRefs->Undo( pDoc );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
