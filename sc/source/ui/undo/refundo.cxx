/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: refundo.cxx,v $
 * $Revision: 1.6.32.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// -----------------------------------------------------------------------



// INCLUDE ---------------------------------------------------------------

#include "refundo.hxx"
#include "undobase.hxx"
#include "document.hxx"
#include "dbcolect.hxx"
#include "rangenam.hxx"
#include "pivot.hxx"
#include "chartarr.hxx"
#include "stlpool.hxx"
#include "conditio.hxx"
#include "detdata.hxx"
#include "prnsave.hxx"
#include "chartlis.hxx"
#include "dpobject.hxx"
#include "areasave.hxx"
#include "unoreflist.hxx"

// -----------------------------------------------------------------------

ScRefUndoData::ScRefUndoData( const ScDocument* pDoc ) :
    pUnoRefs( NULL )
{
    ScDBCollection* pOldDBColl = pDoc->GetDBCollection();
    pDBCollection = pOldDBColl ? new ScDBCollection(*pOldDBColl) : NULL;

    ScRangeName* pOldRanges = ((ScDocument*)pDoc)->GetRangeName();          //! const
    pRangeName = pOldRanges ? new ScRangeName(*pOldRanges) : NULL;

    pPrintRanges = pDoc->CreatePrintRangeSaver();       // neu erzeugt

#if OLD_PIVOT_IMPLEMENTATION
    ScPivotCollection* pOldPivot = pDoc->GetPivotCollection();
    pPivotCollection = pOldPivot ? new ScPivotCollection(*pOldPivot) : NULL;
#endif
    //! bei Pivot nur Bereiche merken ???

    ScDPCollection* pOldDP = ((ScDocument*)pDoc)->GetDPCollection();        //! const
    pDPCollection = pOldDP ? new ScDPCollection(*pOldDP) : NULL;

    ScConditionalFormatList* pOldCond = pDoc->GetCondFormList();
    pCondFormList = pOldCond ? new ScConditionalFormatList(*pOldCond) : NULL;

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
#if OLD_PIVOT_IMPLEMENTATION
    delete pPivotCollection;
#endif
    delete pDPCollection;
    delete pCondFormList;
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

#if OLD_PIVOT_IMPLEMENTATION
    if (pPivotCollection)
    {
        ScPivotCollection* pNewPivot = pDoc->GetPivotCollection();
        if ( pNewPivot && *pPivotCollection == *pNewPivot )
            DELETEZ(pPivotCollection);
    }
#endif

    if (pDPCollection)
    {
        ScDPCollection* pNewDP = ((ScDocument*)pDoc)->GetDPCollection();    //! const
        if ( pNewDP && pDPCollection->RefsEqual(*pNewDP) )
            DELETEZ(pDPCollection);
    }

    if (pCondFormList)
    {
        ScConditionalFormatList* pNewCond = pDoc->GetCondFormList();
        if ( pNewCond && *pCondFormList == *pNewCond )
            DELETEZ(pCondFormList);
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

void ScRefUndoData::DoUndo( ScDocument* pDoc, BOOL bUndoRefFirst )
{
    if (pDBCollection)
        pDoc->SetDBCollection( new ScDBCollection(*pDBCollection) );
    if (pRangeName)
        pDoc->SetRangeName( new ScRangeName(*pRangeName) );

    if (pPrintRanges)
        pDoc->RestorePrintRanges(*pPrintRanges);

#if OLD_PIVOT_IMPLEMENTATION
    if (pPivotCollection)
        pDoc->SetPivotCollection( new ScPivotCollection(*pPivotCollection) );
#endif

    if (pDPCollection)
    {
        ScDPCollection* pDocDP = pDoc->GetDPCollection();
        if (pDocDP)
            pDPCollection->WriteRefsTo( *pDocDP );
    }

    if (pCondFormList)
        pDoc->SetCondFormList( new ScConditionalFormatList(*pCondFormList) );
    if (pDetOpList)
        pDoc->SetDetOpList( new ScDetOpList(*pDetOpList) );

    // #65055# bUndoRefFirst ist bSetChartRangeLists
    if ( pChartListenerCollection )
        pDoc->SetChartListenerCollection( new ScChartListenerCollection(
            *pChartListenerCollection ), bUndoRefFirst );

    if (pDBCollection || pRangeName)
    {
        BOOL bOldAutoCalc = pDoc->GetAutoCalc();
        pDoc->SetAutoCalc( FALSE ); // Mehrfachberechnungen vermeiden
        pDoc->CompileAll();
        pDoc->SetDirty();
        pDoc->SetAutoCalc( bOldAutoCalc );
    }

    if (pAreaLinks)
        pAreaLinks->Restore( pDoc );

    if ( pUnoRefs )
        pUnoRefs->Undo( pDoc );
}




