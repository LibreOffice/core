/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: movedfwdfrmsbyobjpos.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:04:40 $
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
#ifndef _MOVEDFWDFRMSBYOBJPOS_HXX
#include <movedfwdfrmsbyobjpos.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
// --> OD 2004-10-05 #i26945#
#ifndef _ROWFRM_HXX
#include <rowfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
// <--

SwMovedFwdFrmsByObjPos::SwMovedFwdFrmsByObjPos()
{
}

SwMovedFwdFrmsByObjPos::~SwMovedFwdFrmsByObjPos()
{
    Clear();
}

void SwMovedFwdFrmsByObjPos::Insert( const SwTxtFrm& _rMovedFwdFrmByObjPos,
                                     const sal_uInt32 _nToPageNum )
{
    if ( maMovedFwdFrms.end() ==
         maMovedFwdFrms.find( _rMovedFwdFrmByObjPos.GetTxtNode() ) )
    {
        const NodeMapEntry aEntry( _rMovedFwdFrmByObjPos.GetTxtNode(), _nToPageNum );
        maMovedFwdFrms.insert( aEntry );
    }
}

void SwMovedFwdFrmsByObjPos::Remove( const SwTxtFrm& _rTxtFrm )
{
    maMovedFwdFrms.erase( _rTxtFrm.GetTxtNode() );
};

bool SwMovedFwdFrmsByObjPos::FrmMovedFwdByObjPos( const SwTxtFrm& _rTxtFrm,
                                                  sal_uInt32& _ornToPageNum ) const
{
    NodeMapIter aIter = maMovedFwdFrms.find( _rTxtFrm.GetTxtNode() );
    if ( maMovedFwdFrms.end() != aIter )
    {
        _ornToPageNum = (*aIter).second;
        return true;
    }

    return false;
}

// --> OD 2004-10-05 #i26945#
bool SwMovedFwdFrmsByObjPos::DoesRowContainMovedFwdFrm( const SwRowFrm& _rRowFrm ) const
{
    bool bDoesRowContainMovedFwdFrm( false );

    const sal_uInt32 nPageNumOfRow = _rRowFrm.FindPageFrm()->GetPhyPageNum();

    NodeMapIter aIter = maMovedFwdFrms.begin();
    for ( ; aIter != maMovedFwdFrms.end(); ++aIter )
    {
        const NodeMapEntry& rEntry = *(aIter);
        if ( rEntry.second >= nPageNumOfRow )
        {
            SwClientIter aFrmIter( *const_cast<SwTxtNode*>( rEntry.first ) );
            for( SwTxtFrm* pTxtFrm = (SwTxtFrm*)aFrmIter.First( TYPE(SwTxtFrm) );
                 pTxtFrm;
                 pTxtFrm = (SwTxtFrm*)aFrmIter.Next() )
            {
                // --> OD 2004-12-03 #115759# - assure that found text frame
                // is the first one.
                if ( _rRowFrm.IsAnLower( pTxtFrm ) && !pTxtFrm->GetIndPrev() )
                // <--
                {
                    bDoesRowContainMovedFwdFrm = true;
                    break;
                }
            }
        }
    }

    return bDoesRowContainMovedFwdFrm;
}
// <--

