/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: areasave.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:25:15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// -----------------------------------------------------------------------



// INCLUDE ---------------------------------------------------------------

#include <svx/linkmgr.hxx>
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

DataObject* ScAreaLinkSaver::Clone() const
{
    return new ScAreaLinkSaver( *this );
}

BOOL ScAreaLinkSaver::IsEqualSource( const ScAreaLink& rCompare ) const
{
    return ( aFileName   == rCompare.GetFile() &&
             aFilterName == rCompare.GetFilter() &&
             aOptions    == rCompare.GetOptions() &&
             aSourceArea == rCompare.GetSource() &&
             nRefresh    == rCompare.GetRefreshDelay() );
}

BOOL ScAreaLinkSaver::IsEqual( const ScAreaLink& rCompare ) const
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

    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
    SfxObjectShell* pObjSh = pDoc->GetDocumentShell();

    if ( pLinkManager && pObjSh )
    {
        ScAreaLink* pLink = new ScAreaLink( pObjSh, aFileName, aFilterName, aOptions,
                                            aSourceArea, aDestArea.aStart, nRefresh );
        pLink->SetInCreate( TRUE );
        pLink->SetDestArea( aDestArea );
        pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aFileName, &aFilterName, &aSourceArea );
        pLink->Update();
        pLink->SetInCreate( FALSE );
    }
}

// -----------------------------------------------------------------------

ScAreaLinkSaveCollection::ScAreaLinkSaveCollection()
{
}

ScAreaLinkSaveCollection::ScAreaLinkSaveCollection( const ScAreaLinkSaveCollection& rCopy ) :
    Collection( rCopy )
{
}

ScAreaLinkSaveCollection::~ScAreaLinkSaveCollection()
{
}

DataObject* ScAreaLinkSaveCollection::Clone() const
{
    return new ScAreaLinkSaveCollection( *this );
}

BOOL ScAreaLinkSaveCollection::IsEqual( const ScDocument* pDoc ) const
{
    // IsEqual can be checked in sequence.
    // Neither ref-update nor removing links will change the order.

    SvxLinkManager* pLinkManager = const_cast<ScDocument*>(pDoc)->GetLinkManager();
    if (pLinkManager)
    {
        USHORT nPos = 0;
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nLinkCount = rLinks.Count();
        for (USHORT i=0; i<nLinkCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScAreaLink))
            {
                if ( nPos >= GetCount() || !(*this)[nPos]->IsEqual( *(ScAreaLink*)pBase ) )
                    return FALSE;

                ++nPos;
            }
        }
        if ( nPos < GetCount() )
            return FALSE;           // fewer links in the document than in the save collection
    }

    return TRUE;
}

ScAreaLink* lcl_FindLink( const ::sfx2::SvBaseLinks& rLinks, const ScAreaLinkSaver& rSaver )
{
    USHORT nLinkCount = rLinks.Count();
    for (USHORT i=0; i<nLinkCount; i++)
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

    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
    if (pLinkManager)
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nCount = GetCount();
        for (USHORT nPos=0; nPos<nCount; nPos++)
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

    SvxLinkManager* pLinkManager = const_cast<ScDocument*>(pDoc)->GetLinkManager();
    if (pLinkManager)
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nLinkCount = rLinks.Count();
        for (USHORT i=0; i<nLinkCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScAreaLink))
            {
                if (!pColl)
                    pColl = new ScAreaLinkSaveCollection;

                ScAreaLinkSaver* pSaver = new ScAreaLinkSaver( *(ScAreaLink*)pBase );
                if (!pColl->Insert(pSaver))
                    delete pSaver;
            }
        }
    }

    return pColl;
}

