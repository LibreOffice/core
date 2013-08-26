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

#include <sfx2/linkmgr.hxx>

#include "areasave.hxx"
#include "arealink.hxx"
#include "document.hxx"

// -----------------------------------------------------------------------

ScAreaLinkSaver::ScAreaLinkSaver( const ScAreaLink& rSource ) :
    aFileName   ( rSource.GetFile() ),
    aFilterName ( rSource.GetFilter() ),
    aOptions    ( rSource.GetOptions() ),
    aSourceArea ( rSource.GetSource() ),
    aDestArea   ( rSource.GetDestArea() ),
    nRefresh    ( rSource.GetRefreshDelay() )       // seconds
{
}

ScAreaLinkSaver::ScAreaLinkSaver( const ScAreaLinkSaver& rCopy ) :
    aFileName   ( rCopy.aFileName ),
    aFilterName ( rCopy.aFilterName ),
    aOptions    ( rCopy.aOptions ),
    aSourceArea ( rCopy.aSourceArea ),
    aDestArea   ( rCopy.aDestArea ),
    nRefresh    ( rCopy.nRefresh )
{
}

ScAreaLinkSaver::~ScAreaLinkSaver() {}

bool ScAreaLinkSaver::IsEqualSource( const ScAreaLink& rCompare ) const
{
    return ( aFileName.equals(rCompare.GetFile()) &&
             aFilterName.equals(rCompare.GetFilter()) &&
             aOptions.equals(rCompare.GetOptions()) &&
             aSourceArea.equals(rCompare.GetSource()) &&
             nRefresh == rCompare.GetRefreshDelay() );
}

bool ScAreaLinkSaver::IsEqual( const ScAreaLink& rCompare ) const
{
    return ( IsEqualSource( rCompare ) &&
             aDestArea == rCompare.GetDestArea() );
}

void ScAreaLinkSaver::WriteToLink( ScAreaLink& rLink ) const
{
    rLink.SetDestArea( aDestArea );
}

void ScAreaLinkSaver::InsertNewLink( ScDocument* pDoc ) const
{
    // (see ScUndoRemoveAreaLink::Undo)

    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    SfxObjectShell* pObjSh = pDoc->GetDocumentShell();

    if ( pLinkManager && pObjSh )
    {
        ScAreaLink* pLink = new ScAreaLink( pObjSh, aFileName, aFilterName, aOptions,
                                            aSourceArea, aDestArea.aStart, nRefresh );
        pLink->SetInCreate( sal_True );
        pLink->SetDestArea( aDestArea );
        OUString aTmp1(aFilterName), aTmp2(aSourceArea);
        pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aFileName, &aTmp1, &aTmp2 );
        pLink->Update();
        pLink->SetInCreate( false );
    }
}

ScAreaLinkSaveCollection::ScAreaLinkSaveCollection() {}

ScAreaLinkSaveCollection::ScAreaLinkSaveCollection( const ScAreaLinkSaveCollection& r ) :
    maData(r.maData) {}

ScAreaLinkSaveCollection::~ScAreaLinkSaveCollection() {}

bool ScAreaLinkSaveCollection::IsEqual( const ScDocument* pDoc ) const
{
    // IsEqual can be checked in sequence.
    // Neither ref-update nor removing links will change the order.

    sfx2::LinkManager* pLinkManager = const_cast<ScDocument*>(pDoc)->GetLinkManager();
    if (pLinkManager)
    {
        size_t nPos = 0;
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nLinkCount = rLinks.size();
        for (sal_uInt16 i=0; i<nLinkCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScAreaLink))
            {
                if ( nPos >= size() || !(*this)[nPos]->IsEqual( *(ScAreaLink*)pBase ) )
                    return false;

                ++nPos;
            }
        }
        if ( nPos < size() )
            return false;           // fewer links in the document than in the save collection
    }

    return true;
}

static ScAreaLink* lcl_FindLink( const ::sfx2::SvBaseLinks& rLinks, const ScAreaLinkSaver& rSaver )
{
    sal_uInt16 nLinkCount = rLinks.size();
    for (sal_uInt16 i=0; i<nLinkCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        if ( pBase->ISA(ScAreaLink) &&
             rSaver.IsEqualSource( *static_cast<ScAreaLink*>(pBase) ) )
        {
            return static_cast<ScAreaLink*>(pBase);     // found
        }
    }
    return NULL;    // not found
}

void ScAreaLinkSaveCollection::Restore( ScDocument* pDoc ) const
{
    // The save collection may contain additional entries that are not in the document.
    // They must be inserted again.
    // Entries from the save collection must be searched via source data, as the order
    // of links changes if deleted entries are re-added to the link manager (always at the end).

    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    if (pLinkManager)
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        size_t nSaveCount = size();
        for (size_t nPos=0; nPos<nSaveCount; ++nPos)
        {
            const ScAreaLinkSaver* pSaver = (*this)[nPos];
            ScAreaLink* pLink = lcl_FindLink( rLinks, *pSaver );
            if ( pLink )
                pSaver->WriteToLink( *pLink );          // restore output position
            else
                pSaver->InsertNewLink( pDoc );          // re-insert deleted link
        }
    }
}

ScAreaLinkSaveCollection* ScAreaLinkSaveCollection::CreateFromDoc( const ScDocument* pDoc )
{
    ScAreaLinkSaveCollection* pColl = NULL;

    sfx2::LinkManager* pLinkManager = const_cast<ScDocument*>(pDoc)->GetLinkManager();
    if (pLinkManager)
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nLinkCount = rLinks.size();
        for (sal_uInt16 i=0; i<nLinkCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScAreaLink))
            {
                if (!pColl)
                    pColl = new ScAreaLinkSaveCollection;

                ScAreaLinkSaver* pSaver = new ScAreaLinkSaver( *(ScAreaLink*)pBase );
                pColl->push_back(pSaver);
            }
        }
    }

    return pColl;
}

const ScAreaLinkSaver* ScAreaLinkSaveCollection::operator [](size_t nIndex) const
{
    return &maData[nIndex];
}

size_t ScAreaLinkSaveCollection::size() const
{
    return maData.size();
}

void ScAreaLinkSaveCollection::push_back(ScAreaLinkSaver* p)
{
    maData.push_back(p);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
