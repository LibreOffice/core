/*************************************************************************
 *
 *  $RCSfile: areasave.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-30 11:35:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// -----------------------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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
    aDestArea   ( rSource.GetDestArea() )
{
}

ScAreaLinkSaver::ScAreaLinkSaver( const ScAreaLinkSaver& rCopy ) :
    aFileName   ( rCopy.aFileName ),
    aFilterName ( rCopy.aFilterName ),
    aOptions    ( rCopy.aOptions ),
    aSourceArea ( rCopy.aSourceArea ),
    aDestArea   ( rCopy.aDestArea )
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
             aSourceArea == rCompare.GetSource() );
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
    SvxLinkManager* pLinkManager = const_cast<ScDocument*>(pDoc)->GetLinkManager();
    if (pLinkManager)
    {
        USHORT nPos = 0;
        const SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nLinkCount = rLinks.Count();
        for (USHORT i=0; i<nLinkCount; i++)
        {
            SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScAreaLink))
            {
                if ( nPos >= GetCount() || !(*this)[nPos]->IsEqual( *(ScAreaLink*)pBase ) )
                    return FALSE;

                ++nPos;
            }
        }
    }

    return TRUE;
}

void ScAreaLinkSaveCollection::Restore( ScDocument* pDoc ) const
{
    BOOL bError = FALSE;
    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
    if (pLinkManager)
    {
        USHORT nPos = 0;
        const SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nLinkCount = rLinks.Count();
        for (USHORT i=0; i<nLinkCount; i++)
        {
            SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScAreaLink))
            {
                ScAreaLink* pLink = (ScAreaLink*)pBase;
                if ( nPos < GetCount() )
                {
                    ScAreaLinkSaver* pSaver = (*this)[nPos];
                    if ( pSaver->IsEqualSource( *pLink ) )
                        pSaver->WriteToLink( *pLink );
                    else
                        bError = TRUE;
                }
                else
                    bError = TRUE;

                ++nPos;
            }
        }
    }
    DBG_ASSERT( !bError, "ScAreaLinkSaveCollection: wrong links" );
}

// static
ScAreaLinkSaveCollection* ScAreaLinkSaveCollection::CreateFromDoc( const ScDocument* pDoc )
{
    ScAreaLinkSaveCollection* pColl = NULL;

    SvxLinkManager* pLinkManager = const_cast<ScDocument*>(pDoc)->GetLinkManager();
    if (pLinkManager)
    {
        const SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nLinkCount = rLinks.Count();
        for (USHORT i=0; i<nLinkCount; i++)
        {
            SvBaseLink* pBase = *rLinks[i];
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

