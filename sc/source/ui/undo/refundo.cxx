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
#include <document.hxx>
#include <dbdata.hxx>
#include <rangenam.hxx>
#include <detdata.hxx>
#include <prnsave.hxx>
#include <chartlis.hxx>
#include <dpobject.hxx>
#include <areasave.hxx>
#include <unoreflist.hxx>
#include <scopetools.hxx>
#include <refupdatecontext.hxx>

ScRefUndoData::ScRefUndoData( ScDocument& rDoc ) :
    pPrintRanges(rDoc.CreatePrintRangeSaver())
{
    const ScDBCollection* pOldDBColl = rDoc.GetDBCollection();
    if (pOldDBColl && !pOldDBColl->empty())
        pDBCollection.reset(new ScDBCollection(*pOldDBColl));

    const ScRangeName* pOldRanges = rDoc.GetRangeName();
    if (pOldRanges && !pOldRanges->empty())
        pRangeName.reset(new ScRangeName(*pOldRanges));

    // when handling Pivot solely keep the range?

    const ScDPCollection* pOldDP = rDoc.GetDPCollection();
    if (pOldDP && pOldDP->GetCount())
        pDPCollection.reset(new ScDPCollection(*pOldDP));

    const ScDetOpList* pOldDetOp = rDoc.GetDetOpList();
    if (pOldDetOp && pOldDetOp->Count())
        pDetOpList.reset(new ScDetOpList(*pOldDetOp));

    const ScChartListenerCollection* pOldChartLisColl = rDoc.GetChartListenerCollection();
    if (pOldChartLisColl)
        pChartListenerCollection.reset(new ScChartListenerCollection(*pOldChartLisColl));

    pAreaLinks = ScAreaLinkSaveCollection::CreateFromDoc(rDoc);     // returns NULL if empty

    rDoc.BeginUnoRefUndo();
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

void ScRefUndoData::DeleteUnchanged( ScDocument& rDoc )
{
    if (pDBCollection)
    {
        ScDBCollection* pNewDBColl = rDoc.GetDBCollection();
        if ( pNewDBColl && *pDBCollection == *pNewDBColl )
            pDBCollection.reset();
    }
    if (pRangeName)
    {
        ScRangeName* pNewRanges = rDoc.GetRangeName();
        if ( pNewRanges && *pRangeName == *pNewRanges )
            pRangeName.reset();
    }

    if (pPrintRanges)
    {
        std::unique_ptr<ScPrintRangeSaver> pNewRanges = rDoc.CreatePrintRangeSaver();
        if ( pNewRanges && *pPrintRanges == *pNewRanges )
            pPrintRanges.reset();
    }

    if (pDPCollection)
    {
        ScDPCollection* pNewDP = rDoc.GetDPCollection();    //! const
        if ( pNewDP && pDPCollection->RefsEqual(*pNewDP) )
            pDPCollection.reset();
    }

    if (pDetOpList)
    {
        ScDetOpList* pNewDetOp = rDoc.GetDetOpList();
        if ( pNewDetOp && *pDetOpList == *pNewDetOp )
            pDetOpList.reset();
    }

    if ( pChartListenerCollection )
    {
        ScChartListenerCollection* pNewChartListenerCollection =
            rDoc.GetChartListenerCollection();
        if ( pNewChartListenerCollection &&
                *pChartListenerCollection == *pNewChartListenerCollection )
            pChartListenerCollection.reset();
    }

    if (pAreaLinks)
    {
        if ( pAreaLinks->IsEqual( rDoc ) )
            pAreaLinks.reset();
    }

    if ( rDoc.HasUnoRefUndo() )
    {
        pUnoRefs = rDoc.EndUnoRefUndo();
        if ( pUnoRefs && pUnoRefs->IsEmpty() )
        {
            pUnoRefs.reset();
        }
    }
}

void ScRefUndoData::DoUndo( ScDocument& rDoc, bool bUndoRefFirst )
{
    if (pDBCollection)
        rDoc.SetDBCollection( std::unique_ptr<ScDBCollection>(new ScDBCollection(*pDBCollection)) );
    if (pRangeName)
        rDoc.SetRangeName( std::unique_ptr<ScRangeName>(new ScRangeName(*pRangeName)) );

    if (pPrintRanges)
        rDoc.RestorePrintRanges(*pPrintRanges);

    if (pDPCollection)
    {
        ScDPCollection* pDocDP = rDoc.GetDPCollection();
        if (pDocDP)
            pDPCollection->WriteRefsTo( *pDocDP );
    }

    if (pDetOpList)
        rDoc.SetDetOpList( std::unique_ptr<ScDetOpList>(new ScDetOpList(*pDetOpList)) );

    // bUndoRefFirst is bSetChartRangeLists
    if ( pChartListenerCollection )
        rDoc.SetChartListenerCollection( std::make_unique<ScChartListenerCollection>(
            *pChartListenerCollection ), bUndoRefFirst );

    if (pDBCollection || pRangeName)
    {
        sc::AutoCalcSwitch aACSwitch(rDoc, false);
        rDoc.CompileAll();

        sc::SetFormulaDirtyContext aCxt;
        rDoc.SetAllFormulasDirty(aCxt);
    }

    if (pAreaLinks)
        pAreaLinks->Restore( rDoc );

    if ( pUnoRefs )
        pUnoRefs->Undo( rDoc );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
