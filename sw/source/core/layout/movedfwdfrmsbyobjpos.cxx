/*************************************************************************
 *
 *  $RCSfile: movedfwdfrmsbyobjpos.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-22 08:20:18 $
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
            SwClientIter aIter( *const_cast<SwTxtNode*>( rEntry.first ) );
            for( SwTxtFrm* pTxtFrm = (SwTxtFrm*)aIter.First( TYPE(SwTxtFrm) );
                 pTxtFrm;
                 pTxtFrm = (SwTxtFrm*)aIter.Next() )
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

