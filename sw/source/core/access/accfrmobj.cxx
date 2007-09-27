 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accfrmobj.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:21:08 $
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



#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _ACCFRMOBJ_HXX
#include <accfrmobj.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif

sal_Bool SwFrmOrObj::IsAccessible( sal_Bool bPagePreview ) const
{
    return ( pFrm && pFrm->IsAccessibleFrm() &&
             ( !pFrm->IsCellFrm() ||
               static_cast<const SwCellFrm *>( pFrm )->GetTabBox()->GetSttNd() != 0 ) &&
             !pFrm->IsInCoveredCell() &&
             ( bPagePreview || !pFrm->IsPageFrm() ) ) ||
           pObj;
}

sal_Bool SwFrmOrObj::IsBoundAsChar() const
{
    // currently only SwFrms are accessible
    if( pFrm )
    {
        return pFrm->IsFlyFrm() &&
               static_cast< const SwFlyFrm *>(pFrm)->IsFlyInCntFrm();
    }
    else
    {
        const SwFrmFmt *pFrmFmt = pObj ? ::FindFrmFmt( pObj ) : 0;
        return pFrmFmt ? static_cast<sal_Bool>(FLY_IN_CNTNT == pFrmFmt->GetAnchor().GetAnchorId())
                       : sal_False;
    }
}

