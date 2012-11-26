/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// -----------------------------------------------------------------------



// INCLUDE ---------------------------------------------------------------

#include <sfx2/linkmgr.hxx>
#include <tools/debug.hxx>

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
    ScDataObject(),
    aFileName   ( rCopy.aFileName ),
    aFilterName ( rCopy.aFilterName ),
    aOptions    ( rCopy.aOptions ),
    aSourceArea ( rCopy.aSourceArea ),
    aDestArea   ( rCopy.aDestArea ),
    nRefresh    ( rCopy.nRefresh )
{
}

ScAreaLinkSaver::~ScAreaLinkSaver()
{
}

ScDataObject*   ScAreaLinkSaver::Clone() const
{
    return new ScAreaLinkSaver( *this );
}

sal_Bool ScAreaLinkSaver::IsEqualSource( const ScAreaLink& rCompare ) const
{
    return ( aFileName   == rCompare.GetFile() &&
             aFilterName == rCompare.GetFilter() &&
             aOptions    == rCompare.GetOptions() &&
             aSourceArea == rCompare.GetSource() &&
             nRefresh    == rCompare.GetRefreshDelay() );
}

sal_Bool ScAreaLinkSaver::IsEqual( const ScAreaLink& rCompare ) const
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
        pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aFileName, &aFilterName, &aSourceArea );
        pLink->Update();
        pLink->SetInCreate( sal_False );
    }
}

// -----------------------------------------------------------------------

ScAreaLinkSaveCollection::ScAreaLinkSaveCollection()
{
}

ScAreaLinkSaveCollection::ScAreaLinkSaveCollection( const ScAreaLinkSaveCollection& rCopy ) :
    ScCollection( rCopy )
{
}

ScAreaLinkSaveCollection::~ScAreaLinkSaveCollection()
{
}

ScDataObject*   ScAreaLinkSaveCollection::Clone() const
{
    return new ScAreaLinkSaveCollection( *this );
}

sal_Bool ScAreaLinkSaveCollection::IsEqual( const ScDocument* pDoc ) const
{
    // IsEqual can be checked in sequence.
    // Neither ref-update nor removing links will change the order.

    sfx2::LinkManager* pLinkManager = const_cast<ScDocument*>(pDoc)->GetLinkManager();
    if (pLinkManager)
    {
        sal_uInt16 nPos = 0;
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nLinkCount = rLinks.Count();
        for (sal_uInt16 i=0; i<nLinkCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            ScAreaLink* pScAreaLink = dynamic_cast< ScAreaLink* >(pBase);

            if (pScAreaLink)
            {
                if ( nPos >= GetCount() || !(*this)[nPos]->IsEqual( *pScAreaLink ) )
                    return sal_False;

                ++nPos;
            }
        }
        if ( nPos < GetCount() )
            return sal_False;           // fewer links in the document than in the save collection
    }

    return sal_True;
}

ScAreaLink* lcl_FindLink( const ::sfx2::SvBaseLinks& rLinks, const ScAreaLinkSaver& rSaver )
{
    sal_uInt16 nLinkCount = rLinks.Count();
    for (sal_uInt16 i=0; i<nLinkCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        ScAreaLink* pScAreaLink = dynamic_cast< ScAreaLink* >(pBase);

        if ( pScAreaLink && rSaver.IsEqualSource( *pScAreaLink ) )
        {
            return pScAreaLink;     // found
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
        sal_uInt16 nSaveCount = GetCount();
        for (sal_uInt16 nPos=0; nPos<nSaveCount; nPos++)
        {
            ScAreaLinkSaver* pSaver = (*this)[nPos];
            ScAreaLink* pLink = lcl_FindLink( rLinks, *pSaver );
            if ( pLink )
                pSaver->WriteToLink( *pLink );          // restore output position
            else
                pSaver->InsertNewLink( pDoc );          // re-insert deleted link
        }
    }
}

// static
ScAreaLinkSaveCollection* ScAreaLinkSaveCollection::CreateFromDoc( const ScDocument* pDoc )
{
    ScAreaLinkSaveCollection* pColl = NULL;

    sfx2::LinkManager* pLinkManager = const_cast<ScDocument*>(pDoc)->GetLinkManager();
    if (pLinkManager)
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nLinkCount = rLinks.Count();
        for (sal_uInt16 i=0; i<nLinkCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            ScAreaLink* pScAreaLink = dynamic_cast< ScAreaLink* >(pBase);

            if (pScAreaLink)
            {
                if (!pColl)
                    pColl = new ScAreaLinkSaveCollection;

                ScAreaLinkSaver* pSaver = new ScAreaLinkSaver( *pScAreaLink );
                if (!pColl->Insert(pSaver))
                    delete pSaver;
            }
        }
    }

    return pColl;
}

