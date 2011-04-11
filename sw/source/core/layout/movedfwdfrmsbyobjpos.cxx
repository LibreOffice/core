/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <movedfwdfrmsbyobjpos.hxx>
#include <txtfrm.hxx>
#include <rowfrm.hxx>
#include <pagefrm.hxx>
#include <ndtxt.hxx>
#include <switerator.hxx>

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
            SwIterator<SwTxtFrm,SwTxtNode> aFrmIter( *rEntry.first );
            for( SwTxtFrm* pTxtFrm = aFrmIter.First(); pTxtFrm; pTxtFrm = (SwTxtFrm*)aFrmIter.Next() )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
