/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: flypos.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:02:57 $
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
#include "precompiled_sw.hxx"



#include "doc.hxx"
#include "node.hxx"
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif


#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#include "flypos.hxx"
#include "frmfmt.hxx"
#include "dcontact.hxx"
#include "dview.hxx"
#include "flyfrm.hxx"
#include "dflyobj.hxx"
#include "ndindex.hxx"



SV_IMPL_OP_PTRARR_SORT( SwPosFlyFrms, SwPosFlyFrmPtr )

SwPosFlyFrm::SwPosFlyFrm( const SwNodeIndex& rIdx, const SwFrmFmt* pFmt,
                            USHORT nArrPos )
    : pFrmFmt( pFmt ), pNdIdx( (SwNodeIndex*) &rIdx )
{
    BOOL bFnd = FALSE;
    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
    if( FLY_PAGE == rAnchor.GetAnchorId() )
        pNdIdx = new SwNodeIndex( rIdx );
    else if( pFmt->GetDoc()->GetRootFrm() )
    {
        SwClientIter aIter( (SwFmt&)*pFmt );
        if( RES_FLYFRMFMT == pFmt->Which() )
        {
            // Schauen, ob es ein SdrObject dafuer gibt
            if( aIter.First( TYPE( SwFlyFrm) ) )
                nOrdNum = ((SwFlyFrm*)aIter())->GetVirtDrawObj()->GetOrdNum(),
                bFnd = TRUE;
        }
        else if( RES_DRAWFRMFMT == pFmt->Which() )
        {
            // Schauen, ob es ein SdrObject dafuer gibt
            if( aIter.First( TYPE(SwDrawContact) ) )
                nOrdNum = ((SwDrawContact*)aIter())->GetMaster()->GetOrdNum(),
                bFnd = TRUE;
        }
    }

    if( !bFnd )
    {
        nOrdNum = pFmt->GetDoc()->GetSpzFrmFmts()->Count();
        nOrdNum += nArrPos;
    }
}

SwPosFlyFrm::~SwPosFlyFrm()
{
    const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
    if( FLY_PAGE == rAnchor.GetAnchorId() )
        delete pNdIdx;
}

BOOL SwPosFlyFrm::operator==( const SwPosFlyFrm& )
{
    return FALSE;   // FlyFrames koennen auf der gleichen Position stehen
}

BOOL SwPosFlyFrm::operator<( const SwPosFlyFrm& rPosFly )
{
    if( pNdIdx->GetIndex() == rPosFly.pNdIdx->GetIndex() )
    {
        // dann entscheidet die Ordnungsnummer!
        return nOrdNum < rPosFly.nOrdNum;
    }
    return pNdIdx->GetIndex() < rPosFly.pNdIdx->GetIndex();
}



