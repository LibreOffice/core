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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <accfrmobj.hxx>
#include <dcontact.hxx>
#include <cellfrm.hxx>

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
        return (pFrmFmt)
            ? static_cast<sal_Bool>(FLY_AS_CHAR ==
                    pFrmFmt->GetAnchor().GetAnchorId())
            : sal_False;
    }
}

