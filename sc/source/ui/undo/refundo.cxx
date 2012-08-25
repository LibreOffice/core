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

#include "refundo.hxx"
#include "undobase.hxx"
#include "document.hxx"
#include "dbdata.hxx"
#include "rangenam.hxx"
#include "pivot.hxx"
#include "chartarr.hxx"
#include "stlpool.hxx"
#include "detdata.hxx"
#include "prnsave.hxx"
#include "chartlis.hxx"
#include "dpobject.hxx"
#include "areasave.hxx"
#include "unoreflist.hxx"

ScRefUndoData::ScRefUndoData( const ScDocument* pDoc ) :
    pUnoRefs( NULL )
{
    ScDBCollection* pOldDBColl = pDoc->GetDBCollection();
    pDBCollection = pOldDBColl ? new ScDBCollection(*pOldDBColl) : NULL;

    ScRangeName* pOldRanges = ((ScDocument*)pDoc)->GetRangeName();          //! const
    pRangeName = pOldRanges ? new ScRangeName(*pOldRanges) : NULL;

    pPrintRanges = pDoc->CreatePrintRangeSaver();       // recreated

    // when handling Pivot solely keep the range?

    ScDPCollection* pOldDP = ((ScDocument*)pDoc)->GetDPCollection();        //! const
    pDPCollection = pOldDP ? new ScDPCollection(*pOldDP) : NULL;

    ScDetOpList* pOldDetOp = pDoc->GetDetOpList();
    pDetOpList = pOldDetOp ? new ScDetOpList(*pOldDetOp) : 0;

    ScChartListenerCollection* pOldChartListenerCollection =
        pDoc->GetChartListenerCollection();
    pChartListenerCollection = pOldChartListenerCollection ?
        new ScChartListenerCollection( *pOldChartListenerCollection ) : NULL;

    pAreaLinks = ScAreaLinkSaveCollection::CreateFromDoc(pDoc);     // returns NULL if empty

    const_cast<ScDocument*>(pDoc)->BeginUnoRefUndo();
}

ScRefUndoData::~ScRefUndoData()
{
    delete pDBCollection;
    delete pRangeName;
    delete pPrintRanges;
    delete pDPCollection;
    delete pDetOpList;
    delete pChartListenerCollection;
    delete pAreaLinks;
    delete pUnoRefs;
}

void ScRefUndoData::DeleteUnchanged( const ScDocument* pDoc )
{
    if (pDBCollection)
    {
        ScDBCollection* pNewDBColl = pDoc->GetDBCollection();
        if ( pNewDBColl && *pDBCollection == *pNewDBColl )
            DELETEZ(pDBCollection);
    }
    if (pRangeName)
    {
        ScRangeName* pNewRanges = ((ScDocument*)pDoc)->GetRangeName();      //! const
        if ( pNewRanges && *pRangeName == *pNewRanges )
            DELETEZ(pRangeName);
    }

    if (pPrintRanges)
    {
        ScPrintRangeSaver* pNewRanges = pDoc->CreatePrintRangeSaver();
        if ( pNewRanges && *pPrintRanges == *pNewRanges )
            DELETEZ(pPrintRanges);
        delete pNewRanges;
    }

    if (pDPCollection)
    {
        ScDPCollection* pNewDP = ((ScDocument*)pDoc)->GetDPCollection();    //! const
        if ( pNewDP && pDPCollection->RefsEqual(*pNewDP) )
            DELETEZ(pDPCollection);
    }

    if (pDetOpList)
    {
        ScDetOpList* pNewDetOp = pDoc->GetDetOpList();
        if ( pNewDetOp && *pDetOpList == *pNewDetOp )
            DELETEZ(pDetOpList);
    }

    if ( pChartListenerCollection )
    {
        ScChartListenerCollection* pNewChartListenerCollection =
            pDoc->GetChartListenerCollection();
        if ( pNewChartListenerCollection &&
                *pChartListenerCollection == *pNewChartListenerCollection )
            DELETEZ( pChartListenerCollection );
    }

    if (pAreaLinks)
    {
        if ( pAreaLinks->IsEqual( pDoc ) )
            DELETEZ(pAreaLinks);
    }

    if ( pDoc->HasUnoRefUndo() )
    {
        pUnoRefs = const_cast<ScDocument*>(pDoc)->EndUnoRefUndo();
        if ( pUnoRefs && pUnoRefs->IsEmpty() )
        {
            DELETEZ( pUnoRefs );
        }
    }
}

void ScRefUndoData::DoUndo( ScDocument* pDoc, sal_Bool bUndoRefFirst )
{
    if (pDBCollection)
        pDoc->SetDBCollection( new ScDBCollection(*pDBCollection) );
    if (pRangeName)
        pDoc->SetRangeName( new ScRangeName(*pRangeName) );

    if (pPrintRanges)
        pDoc->RestorePrintRanges(*pPrintRanges);

    if (pDPCollection)
    {
        ScDPCollection* pDocDP = pDoc->GetDPCollection();
        if (pDocDP)
            pDPCollection->WriteRefsTo( *pDocDP );
    }

    if (pDetOpList)
        pDoc->SetDetOpList( new ScDetOpList(*pDetOpList) );

    // bUndoRefFirst is bSetChartRangeLists
    if ( pChartListenerCollection )
        pDoc->SetChartListenerCollection( new ScChartListenerCollection(
            *pChartListenerCollection ), bUndoRefFirst );

    if (pDBCollection || pRangeName)
    {
        sal_Bool bOldAutoCalc = pDoc->GetAutoCalc();
        pDoc->SetAutoCalc( false ); // Avoid multiple calculations
        pDoc->CompileAll();
        pDoc->SetDirty();
        pDoc->SetAutoCalc( bOldAutoCalc );
    }

    if (pAreaLinks)
        pAreaLinks->Restore( pDoc );

    if ( pUnoRefs )
        pUnoRefs->Undo( pDoc );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
